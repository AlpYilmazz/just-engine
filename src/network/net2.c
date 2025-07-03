#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdlib.h>
#include <process.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdbool.h>

#include <openssl/ssl.h>

#include "core.h"
#include "logging.h"
#include "thread/threadsync.h"

#define SOCKET_NO_DEFINE
#include "network/net2.h"

typedef SOCKET Socket;

HANDLE NETWORK_THREAD;
uint32 NETWORK_THREAD_ID;
SRWLock* NETWORK_THREAD_LOCK;

#pragma region INTERRUPT
atomic_bool interrupted = ATOMIC_VAR_INIT(1);
Socket interrupt_socket;

bool is_interrupted() {
    return atomic_load(&interrupted);
}

bool maybe_flip_interrupted(bool enter_cond) {
    bool noncond = !enter_cond;
    return atomic_compare_exchange_strong(&interrupted, &enter_cond, noncond);
}

void init_interrupt() {
    if (maybe_flip_interrupted(true)) {
        interrupt_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (interrupt_socket == INVALID_SOCKET) {
            // TODO: handle err
            PANIC("interrupt_socket INVALID_SOCKET\n");
        }

        unsigned long non_blocking_mode = 1;
        int32 result = ioctlsocket(interrupt_socket, FIONBIO, &non_blocking_mode);
        if (result != NO_ERROR) {
            // TODO: handle err
            PANIC("interrupt_socket setup failed\n");
        }
    }
}

void interrupt_network_wait(uint64 _arg) {
    if (maybe_flip_interrupted(false)) {
        closesocket(interrupt_socket);
        JUST_LOG_DEBUG("Interrupted\n");
    }
}

void issue_interrupt_apc() {
    bool success = QueueUserAPC(
        interrupt_network_wait,
        NETWORK_THREAD,
        0
    );
    if (!success) {
        int32 err = GetLastError();
        // TODO: handle err
        JUST_LOG_ERROR("Interrupt APC failed: %d\n", err);
    }
}
#pragma endregion

typedef enum {
    SOCKET_TYPE_STREAM = SOCK_STREAM,
    SOCKET_TYPE_DATAGRAM = SOCK_DGRAM,
} SocketTypeEnum;

typedef enum {
    COMM_DIRECTION_IDLE = 0,
    COMM_DIRECTION_READ,
    COMM_DIRECTION_WRITE,
} CommDirection;

// CommDirection merge_direction(CommDirection base, CommDirection add) {
//     if (add == COMM_DIRECTION_IDLE) {
//         return COMM_DIRECTION_IDLE;
//     }
//     if (base == COMM_DIRECTION_IDLE) {
//         return add;
//     }
//     return (base == add) ? base : COMM_DIRECTION_BOTH;
// }

typedef struct {
    NetworkProtocolEnum protocol;
    SocketAddr remote_addr;
    Socket socket;
    BIO* ssl_bio;
    uint64 fd;
} ConnectionInfo;

typedef struct {
    ConnectionInfo conn;
    CommDirection current_direction;
    uint32 connect_id;
    void* on_connect_fn;
    void* arg;
    // --
    bool done;
    // bool success;
    ConnectResult result;
} NetworkConnectCommand;

typedef struct {
    bool read_active;
    void* on_read_fn;
    void* arg;
} NetworkReadCommand;

#pragma region NetworkWriteCommandQueue
typedef struct {
    SocketAddr write_addr; // datagram
    BufferSlice buffer;
    usize offset;
    // --
    void* on_write_fn;
    void* arg;
} NetworkWriteCommand;

typedef struct {
    uint32 capacity;
    uint32 count;
    NetworkWriteCommand queue[SOCKET_WRITE_QUEUE_CAPACITY];
    uint32 head; // pop position
    uint32 tail; // push position
} NetworkWriteCommandQueue;

NetworkWriteCommandQueue write_queue_new_empty() {
    NetworkWriteCommandQueue queue = {0};
    queue.capacity = SOCKET_WRITE_QUEUE_CAPACITY;
    return queue;
}

bool write_queue_is_full(NetworkWriteCommandQueue* queue) {
    return queue->count == queue->capacity;
}

bool write_queue_is_empty(NetworkWriteCommandQueue* queue) {
    return queue->count == 0;
}

bool write_queue_has_next(NetworkWriteCommandQueue* queue) {
    return queue->count > 0;
}

void write_queue_push_command_unchecked(NetworkWriteCommandQueue* queue, NetworkWriteCommand command) {
    queue->queue[queue->tail] = command;
    queue->tail = (queue->tail + 1) % queue->capacity;
    queue->count++;
}

NetworkWriteCommand write_queue_pop_command_unchecked(NetworkWriteCommandQueue* queue) {
    NetworkWriteCommand command = queue->queue[queue->head];
    queue->count--;
    queue->head = (queue->head + 1) % queue->capacity;
    return command;
}
#pragma endregion NetworkWriteCommandQueue

typedef struct {
    ConnectionInfo conn;
    bool want_read;
    bool want_write;
    NetworkReadCommand read_command;
    NetworkWriteCommandQueue write_queue;
    // --
    bool closed;
} NetworkConnection;

typedef struct {
    uint32 length;
    NetworkConnectCommand commands[FD_SETSIZE];
} ConnectCommandList;

typedef struct {
    uint32 length;
    NetworkConnection connections[FD_SETSIZE];
} NetworkConnectionList;

ConnectCommandList CONNECT_COMMANDS = {0};
NetworkConnectionList NETWORK_CONNECTIONS = {0};

void store_connect_command(NetworkConnectCommand connect_command) {
    CONNECT_COMMANDS.commands[CONNECT_COMMANDS.length++] = connect_command;
}

NetworkConnectCommand* find_connect_command(Socket socket) {
    for (uint32 i = 0; i < CONNECT_COMMANDS.length; i++) {
        if (CONNECT_COMMANDS.commands[i].conn.socket == socket) {
            return &CONNECT_COMMANDS.commands[i];
        }
    }
    return NULL;
}

void store_network_connection(NetworkConnection connection) {
    NETWORK_CONNECTIONS.connections[NETWORK_CONNECTIONS.length++] = connection;
}

NetworkConnection* find_network_connection(Socket socket) {
    for (uint32 i = 0; i < NETWORK_CONNECTIONS.length; i++) {
        if (NETWORK_CONNECTIONS.connections[i].conn.socket == socket) {
            return &NETWORK_CONNECTIONS.connections[i];
        }
    }
    return NULL;
}

#pragma region SSL CONTEXTS
SSL_CTX* TLS_ctx;
SSL_CTX* DTLS_ctx;

bool create_ssl_contexts() {
    {
        TLS_ctx = SSL_CTX_new(TLS_client_method());
        if (TLS_ctx == NULL) {
            JUST_LOG_ERROR("Failed to create the TLS SSL_CTX\n");
            goto FAIL;
        }

        SSL_CTX_set_verify(TLS_ctx, SSL_VERIFY_PEER, NULL);

        /* Use the default trusted certificate store */
        if (!SSL_CTX_set_default_verify_paths(TLS_ctx)) {
            JUST_LOG_ERROR("Failed to set the default trusted certificate store\n");
            goto FAIL;
        }

        if (!SSL_CTX_set_min_proto_version(TLS_ctx, TLS1_2_VERSION)) {
            JUST_LOG_ERROR("Failed to set the minimum TLS protocol version\n");
            goto FAIL;
        }

        SSL_CTX_set_mode(TLS_ctx, SSL_MODE_ENABLE_PARTIAL_WRITE | SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);
    }
    {
        DTLS_ctx = SSL_CTX_new(DTLS_client_method());
        if (DTLS_ctx == NULL) {
            JUST_LOG_ERROR("Failed to create the DTLS SSL_CTX\n");
            goto FAIL;
        }

        SSL_CTX_set_verify(DTLS_ctx, SSL_VERIFY_PEER, NULL);

        /* Use the default trusted certificate store */
        if (!SSL_CTX_set_default_verify_paths(DTLS_ctx)) {
            JUST_LOG_ERROR("Failed to set the default trusted certificate store\n");
            goto FAIL;
        }

        if (!SSL_CTX_set_min_proto_version(DTLS_ctx, DTLS1_2_VERSION)) {
            JUST_LOG_ERROR("Failed to set the minimum DTLS protocol version\n");
            goto FAIL;
        }
        
        SSL_CTX_set_mode(DTLS_ctx, SSL_MODE_ENABLE_PARTIAL_WRITE | SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);
        // SSL_set_mtu(ssl, 1500);
        // SSL_set_mtu(ssl, 0); // Auto-discovery ?
        // SSL_CTX_set_mtu();
        // SSL_CTX_set_dtlsext_mtu(DTLS_ctx, 0); // Auto-discovery
    }

    return true;
    FAIL:
    return false;
}
#pragma endregion SSL CONTEXTS

#pragma region NetworkRequestQueue
typedef struct {
    NetworkProtocolEnum protocol;
    SocketAddr remote_addr;
    uint32 connect_id;
    void* on_connect_fn;
    void* arg;
} NetworkConnectRequest;

typedef struct {
    Socket socket;
    void* on_read_fn;
    void* arg;
} NetworkReadRequest;

typedef struct {
    Socket socket;
    SocketAddr remote_addr;
    BufferSlice buffer;
    void* on_write_fn;
    void* arg;
} NetworkWriteRequest;

typedef struct {
    Socket socket;
    ConnectionCloseEnum close_moment;
    void* on_close_fn;
    void* arg;
} NetworkCloseRequest;

typedef struct {
    uint32 connect_length;
    NetworkConnectRequest connect_requests[FD_SETSIZE];
    uint32 read_length;
    NetworkReadRequest read_requests[FD_SETSIZE];
    uint32 write_length;
    NetworkWriteRequest write_requests[FD_SETSIZE];
    uint32 close_length;
    NetworkCloseRequest close_requests[FD_SETSIZE];
} NetworkRequestQueue;

NetworkRequestQueue REQUEST_QUEUE = {0};

void queue_connect_request(NetworkConnectRequest connect_request) {
    REQUEST_QUEUE.connect_requests[REQUEST_QUEUE.connect_length++] = connect_request;
}
void queue_read_request(NetworkReadRequest read_request) {
    REQUEST_QUEUE.read_requests[REQUEST_QUEUE.read_length++] = read_request;
}
void queue_write_request(NetworkWriteRequest write_request) {
    REQUEST_QUEUE.write_requests[REQUEST_QUEUE.write_length++] = write_request;
}
void queue_close_request(NetworkCloseRequest close_request) {
    REQUEST_QUEUE.close_requests[REQUEST_QUEUE.close_length++] = close_request;
}
#pragma endregion NetworkRequestQueue

Socket make_socket(SocketTypeEnum socket_type) {
    Socket sock;
    switch (socket_type) {
        case SOCKET_TYPE_STREAM:
            sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            break;
        case SOCKET_TYPE_DATAGRAM:
            sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            break;
        default:
            // Unsupported
            JUST_LOG_ERROR("Unsupported Socket Type\n");
            return INVALID_SOCKET;
    }

    if (sock == INVALID_SOCKET) {
        // TODO: handle err
        JUST_LOG_ERROR("Socket creation failed\n");
        return INVALID_SOCKET;
    }

    unsigned long non_blocking_mode = 1;
    int32 result = ioctlsocket(sock, FIONBIO, &non_blocking_mode);
    if (result != NO_ERROR) {
        // TODO: handle err
        JUST_LOG_ERROR("Socket setup failed\n");
        closesocket(sock);
        return INVALID_SOCKET;
    }

    return sock;
}
void bind_socket(Socket socket, SocketAddr addr) {
    SOCKADDR_IN sockaddr = {0};
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_addr.s_addr = inet_addr(addr.host);
    sockaddr.sin_port = htons(addr.port);

    int32 result = bind(socket, (SOCKADDR*) &sockaddr, sizeof(sockaddr));
    if (result == SOCKET_ERROR) {
        int32 err = WSAGetLastError();
        switch (err) {
        default:
            // TODO: handle err
            JUST_LOG_ERROR("Bind failed: %d\n", err);
            break;
        }
    }
}

NetworkConnectCommand TCP_init_connect(NetworkConnectRequest connect_request) {
    bool done = false;
    ConnectResult connect_result = CONNECT_SUCCESS;

    Socket socket = make_socket(SOCKET_TYPE_STREAM);
    if (socket == INVALID_SOCKET) {
        done = true;
        connect_result = CONNECT_FAIL_GENERAL;
    }

    return (NetworkConnectCommand) {
        .conn = (ConnectionInfo) {
            .protocol = connect_request.protocol,
            .remote_addr = connect_request.remote_addr,
            .socket = socket,
            .ssl_bio = NULL,
            .fd = socket,
        },
        .current_direction = COMM_DIRECTION_IDLE,
        .connect_id = connect_request.connect_id,
        .on_connect_fn = connect_request.on_connect_fn,
        .arg = connect_request.arg,
        .done = done,
        .result = connect_result,
    };
}
void TCP_try_connect(NetworkConnectCommand* connect_command) {
    Socket socket = connect_command->conn.socket;

    SOCKADDR_IN sockaddr = {0};
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_addr.s_addr = inet_addr(connect_command->conn.remote_addr.host);
    sockaddr.sin_port = htons(connect_command->conn.remote_addr.port);

    int32 result = connect(socket, (SOCKADDR*) &sockaddr, sizeof(sockaddr));
    if (result == SOCKET_ERROR) {
        int32 err = WSAGetLastError();
        switch (err) {
        case WSAEWOULDBLOCK:
            // Fine
            JUST_LOG_INFO("Connection started\n");
            connect_command->current_direction = COMM_DIRECTION_WRITE;
            break;
        default:
            // TODO: handle err
            JUST_LOG_ERROR("Connection failed: %d\n", err);
            connect_command->done = true;
            connect_command->result = CONNECT_FAIL_GENERAL;
            break;
        }
        return;
    }

    connect_command->done = (result == 0); // result == 0 -> success
    connect_command->result = CONNECT_SUCCESS;
}
void TCP_try_read(NetworkConnection* connection, BufferSlice netbuffer) {
    JUST_LOG_TRACE("Read TCP\n");
    Socket socket = connection->conn.socket;
    NetworkReadCommand read_command = connection->read_command;

    int32 received_count = recv(socket, netbuffer.bytes, netbuffer.length, 0);
    if (received_count == SOCKET_ERROR) {
        int32 err = WSAGetLastError();
        switch (err) {
        case WSAEWOULDBLOCK:
            // Fine
            connection->want_read |= true;
            break;
        default:
            // TODO: handle err
            JUST_LOG_WARN("read err: %d\n", err);
            break;
        }
        return;
    }
    JUST_LOG_TRACE("received count: %d\n", received_count);
    
    BufferSlice read_buffer = buffer_as_slice(netbuffer, 0, received_count);
    ReadContext context = {
        .socket = socket,
        .remote_addr = connection->conn.remote_addr,
    };
    connection->read_command.read_active = ((OnReadFn) read_command.on_read_fn)(context, read_buffer, read_command.arg);
}
void TCP_try_write(NetworkConnection* connection) {
    JUST_LOG_TRACE("Write TCP\n");
    Socket socket = connection->conn.socket;
    NetworkWriteCommand* this_write = &connection->write_queue.queue[connection->write_queue.head];
                
    int32 sent_count = send(
        socket,
        this_write->buffer.bytes + this_write->offset,
        this_write->buffer.length - this_write->offset,
        0
    );
    if (sent_count == SOCKET_ERROR) {
        int32 err = WSAGetLastError();
        switch (err) {
        case WSAEWOULDBLOCK:
            // Fine
            connection->want_write |= true;
            break;
        default:
            // TODO: handle err
            JUST_LOG_WARN("write err: %d\n", err);
            break;
        }
        return;
    }

    this_write->offset += sent_count;
}
void TCP_free_connection(NetworkConnection* connection) {
    closesocket(connection->conn.socket);
}

NetworkConnectCommand UDP_init_connect(NetworkConnectRequest connect_request) {
    bool done = false;
    ConnectResult connect_result = CONNECT_SUCCESS;

    Socket socket = make_socket(SOCKET_TYPE_DATAGRAM);
    if (socket == INVALID_SOCKET) {
        done = true;
        connect_result = CONNECT_FAIL_GENERAL;
    }

    return (NetworkConnectCommand) {
        .conn = (ConnectionInfo) {
            .protocol = connect_request.protocol,
            .remote_addr = connect_request.remote_addr,
            .socket = socket,
            .ssl_bio = NULL,
            .fd = socket,
        },
        .current_direction = COMM_DIRECTION_IDLE,
        .connect_id = connect_request.connect_id,
        .on_connect_fn = connect_request.on_connect_fn,
        .arg = connect_request.arg,
        .done = done,
        .result = connect_result,
    };
}
void UDP_try_connect(NetworkConnectCommand* connect_command) {
    connect_command->current_direction = COMM_DIRECTION_WRITE;
}
void UDP_try_read(NetworkConnection* connection, BufferSlice netbuffer) {
    JUST_LOG_TRACE("Read UDP\n");
    Socket socket = connection->conn.socket;
    NetworkReadCommand read_command = connection->read_command;

    SOCKADDR_IN sockaddr = {0};
    int sockaddr_size = sizeof(sockaddr);
    int32 received_count = recvfrom(socket, netbuffer.bytes, netbuffer.length, 0, (SOCKADDR*) &sockaddr, &sockaddr_size);
    if (received_count == SOCKET_ERROR) {
        int32 err = WSAGetLastError();
        switch (err) {
        case WSAEWOULDBLOCK:
            // Fine
            connection->want_read |= true;
            break;
        case WSAEMSGSIZE:
            // TODO: Partial Read
            break;
        default:
            // TODO: handle err
            JUST_LOG_WARN("read err: %d\n", err);
            break;
        }
        return;
    }
    JUST_LOG_TRACE("received count: %d\n", received_count);

    SocketAddr addr = {
        .host = inet_ntoa(sockaddr.sin_addr),
        .port = ntohs(sockaddr.sin_port),
    };

    BufferSlice datagram = buffer_as_slice(netbuffer, 0, received_count);
    ReadContext context = {
        .socket = socket,
        .remote_addr = addr,
    };
    connection->read_command.read_active = ((OnReadFn) read_command.on_read_fn)(context, datagram, read_command.arg);
}
void UDP_try_write(NetworkConnection* connection) {
    JUST_LOG_TRACE("Write UDP\n");
    Socket socket = connection->conn.socket;
    NetworkWriteCommand* this_write = &connection->write_queue.queue[connection->write_queue.head];

    SOCKADDR_IN sockaddr = {0};
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_addr.s_addr = inet_addr(this_write->write_addr.host);
    sockaddr.sin_port = htons(this_write->write_addr.port);

    int32 sent_count = sendto(
        socket,
        this_write->buffer.bytes + this_write->offset,
        this_write->buffer.length - this_write->offset,
        0,
        (SOCKADDR*) &sockaddr,
        sizeof(sockaddr)
    );
    if (sent_count == SOCKET_ERROR) {
        int32 err = WSAGetLastError();
        switch (err) {
        case WSAEWOULDBLOCK:
            // Fine
            connection->want_write |= true;
            break;
        case WSAEMSGSIZE:
            // TODO: Partial Write
            // no data is transmitted
            this_write->offset = this_write->buffer.length;
            break;
        default:
            // TODO: handle err
            JUST_LOG_WARN("write err: %d\n", err);
            JUST_LOG_WARN("write_addr.host: %s\n", this_write->write_addr.host);
            JUST_LOG_WARN("write_addr.port: %d\n", this_write->write_addr.port);
            break;
        }
        return;
    }
    
    this_write->offset += sent_count;
}
void UDP_free_connection(NetworkConnection* connection) {
    closesocket(connection->conn.socket);
}

NetworkConnectCommand TLS_init_connect(NetworkConnectRequest connect_request) {
    bool done = false;
    ConnectResult connect_result = CONNECT_SUCCESS;

    BIO *bio = BIO_new_ssl_connect(TLS_ctx);
    if (!bio) {
        JUST_LOG_ERROR("Failed BIO_new_ssl_connect\n");
        done = true;
        connect_result = CONNECT_FAIL_GENERAL;
        goto END;
    }

    // BIO_set_conn_hostname(bio, "example.com:443");
    BIO_set_conn_hostname(bio, connect_request.remote_addr.host);
    BIO_set_conn_port(bio, connect_request.remote_addr.service);
    
    SSL *ssl = NULL;
    BIO_get_ssl(bio, &ssl);
    SSL_set_tlsext_host_name(ssl, connect_request.remote_addr.host);

    BIO_set_close(bio, BIO_CLOSE);
    BIO_set_nbio(bio, 1);

    END:
    return (NetworkConnectCommand) {
        .conn = (ConnectionInfo) {
            .protocol = connect_request.protocol,
            .remote_addr = connect_request.remote_addr,
            .socket = INVALID_SOCKET,
            .ssl_bio = bio,
            .fd = INVALID_SOCKET,
        },
        .current_direction = COMM_DIRECTION_IDLE,
        .connect_id = connect_request.connect_id,
        .on_connect_fn = connect_request.on_connect_fn,
        .arg = connect_request.arg,
        .done = done,
        .result = connect_result,
    };
}
void TLS_try_connect(NetworkConnectCommand* connect_command) {
    BIO *bio = connect_command->conn.ssl_bio;

    int32 result = BIO_do_connect(bio);

    int32 fd = BIO_get_fd(bio, NULL);
    // if (fd < 0) {
    //     PANIC("FD should not be negative\n"); // TODO: Maybe not ???
    // }
    connect_command->conn.socket = fd;
    connect_command->conn.fd = fd;

    CommDirection comm_direction;
    if (result > 0) {
        connect_command->done = true;
        connect_command->result = CONNECT_SUCCESS;
    }
    else if (BIO_should_retry(bio)) {
        if (BIO_should_read(bio)) {
            comm_direction = COMM_DIRECTION_READ;
        }
        else if (BIO_should_write(bio)) {
            comm_direction = COMM_DIRECTION_WRITE;
        }
        else {
            // TODO: should not happen, what to do
            UNREACHABLE();
        }
    }
    else {
        connect_command->done = true;
        connect_command->result = CONNECT_FAIL_GENERAL;
    }
}
void TLS_try_read(NetworkConnection* connection, BufferSlice netbuffer) {
    JUST_LOG_TRACE("Read TLS\n");
    Socket socket = connection->conn.socket;
    BIO* bio = connection->conn.ssl_bio;
    NetworkReadCommand read_command = connection->read_command;

    uint64 received_count;
    int32 success = BIO_read_ex(bio, netbuffer.bytes, netbuffer.length, &received_count);
    
    if (!success) {
        if (BIO_should_retry(bio)) {
            if (BIO_should_read(bio)) {
                connection->want_read = true;
            }
            else if (BIO_should_write(bio)) {
                connection->want_write = true;
            }
            else {
                // TODO: should not happen, what to do
                UNREACHABLE();
            }
        }
        else if (BIO_eof(bio)) {
            // TODO: connection closed
        }
        else {
            // TODO: handle err
        }
        return;
    }
    JUST_LOG_TRACE("received count: %d\n", received_count);
    
    BufferSlice read_buffer = buffer_as_slice(netbuffer, 0, received_count);
    ReadContext context = {
        .socket = socket,
        .remote_addr = connection->conn.remote_addr,
    };
    connection->read_command.read_active = ((OnReadFn) read_command.on_read_fn)(context, read_buffer, read_command.arg);
}
void TLS_try_write(NetworkConnection* connection) {
    JUST_LOG_TRACE("Write TLS\n");
    Socket socket = connection->conn.socket;
    BIO* bio = connection->conn.ssl_bio;
    NetworkWriteCommand* this_write = &connection->write_queue.queue[connection->write_queue.head];

    uint64 sent_count;
    int32 success = BIO_write_ex(
        bio,
        this_write->buffer.bytes + this_write->offset,
        this_write->buffer.length - this_write->offset,
        &sent_count
    );

    if (!success) {
        if (BIO_should_retry(bio)) {
            if (BIO_should_read(bio)) {
                connection->want_read = true;
            }
            else if (BIO_should_write(bio)) {
                connection->want_write = true;
            }
            else {
                // TODO: should not happen, what to do
                UNREACHABLE();
            }
        }
        else if (BIO_eof(bio)) {
            // TODO: connection closed
        }
        else {
            // TODO: handle err
        }
        return;
    }

    this_write->offset += sent_count;
}
void TLS_free_connection(NetworkConnection* connection) {
    BIO_free_all(connection->conn.ssl_bio);
}

NetworkConnectCommand DTLS_init_connect(NetworkConnectRequest connect_request) {
    bool done = false;
    ConnectResult connect_result = CONNECT_SUCCESS;

    BIO *bio = BIO_new_ssl_connect(DTLS_ctx);
    if (!bio) {
        JUST_LOG_ERROR("Failed BIO_new_ssl_connect\n");
        done = true;
        connect_result = CONNECT_FAIL_GENERAL;
        goto END;
    }

    // BIO_set_conn_hostname(bio, "example.com:443");
    BIO_set_conn_hostname(bio, connect_request.remote_addr.host);
    BIO_set_conn_port(bio, connect_request.remote_addr.service);
    
    SSL *ssl = NULL;
    BIO_get_ssl(bio, &ssl);
    SSL_set_tlsext_host_name(ssl, connect_request.remote_addr.host);

    BIO_set_close(bio, BIO_CLOSE);
    BIO_set_nbio(bio, 1);

    END:
    return (NetworkConnectCommand) {
        .conn = (ConnectionInfo) {
            .protocol = connect_request.protocol,
            .remote_addr = connect_request.remote_addr,
            .socket = INVALID_SOCKET,
            .ssl_bio = bio,
            .fd = INVALID_SOCKET,
        },
        .current_direction = COMM_DIRECTION_IDLE,
        .connect_id = connect_request.connect_id,
        .on_connect_fn = connect_request.on_connect_fn,
        .arg = connect_request.arg,
        .done = done,
        .result = connect_result,
    };
}
void DTLS_try_connect(NetworkConnectCommand* connect_command) {
    BIO *bio = connect_command->conn.ssl_bio;

    int32 result = BIO_do_connect(bio);

    int32 fd = BIO_get_fd(bio, NULL);
    // if (fd < 0) {
    //     PANIC("FD should not be negative\n"); // TODO: Maybe not ???
    // }
    connect_command->conn.socket = fd;
    connect_command->conn.fd = fd;

    CommDirection comm_direction;
    if (result > 0) {
        connect_command->done = true;
        connect_command->result = CONNECT_SUCCESS;
    }
    else if (BIO_should_retry(bio)) {
        if (BIO_should_read(bio)) {
            comm_direction = COMM_DIRECTION_READ;
        }
        else if (BIO_should_write(bio)) {
            comm_direction = COMM_DIRECTION_WRITE;
        }
        else {
            // TODO: should not happen, what to do
            UNREACHABLE();
        }
    }
    else {
        connect_command->done = true;
        connect_command->result = CONNECT_FAIL_GENERAL;
    }
}
void DTLS_try_read(NetworkConnection* connection, BufferSlice netbuffer) {
    JUST_LOG_TRACE("Read DTLS\n");
    Socket socket = connection->conn.socket;
    BIO* bio = connection->conn.ssl_bio;
    NetworkReadCommand read_command = connection->read_command;

    uint64 received_count;
    int32 success = BIO_read_ex(bio, netbuffer.bytes, netbuffer.length, &received_count);
    
    if (!success) {
        if (BIO_should_retry(bio)) {
            if (BIO_should_read(bio)) {
                connection->want_read = true;
            }
            else if (BIO_should_write(bio)) {
                connection->want_write = true;
            }
            else {
                // TODO: should not happen, what to do
                UNREACHABLE();
            }
        }
        else if (BIO_eof(bio)) {
            // TODO: connection closed
        }
        else {
            // TODO: handle err
        }
        return;
    }
    JUST_LOG_TRACE("received count: %d\n", received_count);
    
    BufferSlice read_buffer = buffer_as_slice(netbuffer, 0, received_count);
    ReadContext context = {
        .socket = socket,
        .remote_addr = connection->conn.remote_addr,
    };
    connection->read_command.read_active = ((OnReadFn) read_command.on_read_fn)(context, read_buffer, read_command.arg);
}
void DTLS_try_write(NetworkConnection* connection) {
    JUST_LOG_TRACE("Write DTLS\n");
    Socket socket = connection->conn.socket;
    BIO* bio = connection->conn.ssl_bio;
    NetworkWriteCommand* this_write = &connection->write_queue.queue[connection->write_queue.head];

    uint64 sent_count;
    int32 success = BIO_write_ex(
        bio,
        this_write->buffer.bytes + this_write->offset,
        this_write->buffer.length - this_write->offset,
        &sent_count
    );

    if (!success) {
        if (BIO_should_retry(bio)) {
            if (BIO_should_read(bio)) {
                connection->want_read = true;
            }
            else if (BIO_should_write(bio)) {
                connection->want_write = true;
            }
            else {
                // TODO: should not happen, what to do
                UNREACHABLE();
            }
        }
        else if (BIO_eof(bio)) {
            // TODO: connection closed
        }
        else {
            // TODO: handle err
        }
        return;
    }

    this_write->offset += sent_count;
}
void DTLS_free_connection(NetworkConnection* connection) {
    BIO_free_all(connection->conn.ssl_bio);
}

NetworkConnectCommand network_init_connect(NetworkConnectRequest connect_request) {
    switch (connect_request.protocol) {
    case NETWORK_PROTOCOL_TCP:
        return TCP_init_connect(connect_request);
    case NETWORK_PROTOCOL_UDP:
        return UDP_init_connect(connect_request);
    case NETWORK_PROTOCOL_TLS:
        return TLS_init_connect(connect_request);
    case NETWORK_PROTOCOL_DTLS:
        return DTLS_init_connect(connect_request);
    default:
        UNREACHABLE();
    }
}
void network_try_connect(NetworkConnectCommand* connect_command) {
    if (connect_command->done) {
        return;
    }

    switch (connect_command->conn.protocol) {
    case NETWORK_PROTOCOL_TCP:
        TCP_try_connect(connect_command);
        break;
    case NETWORK_PROTOCOL_UDP:
        UDP_try_connect(connect_command);
        break;
    case NETWORK_PROTOCOL_TLS:
        TLS_try_connect(connect_command);
        break;
    case NETWORK_PROTOCOL_DTLS:
        DTLS_try_connect(connect_command);
        break;
    default:
        UNREACHABLE();
    }
}
void network_try_read(NetworkConnection* connection, BufferSlice netbuffer) {
    if (connection->closed || !connection->read_command.read_active) {
        return;
    }

    switch (connection->conn.protocol) {
    case NETWORK_PROTOCOL_TCP:
        TCP_try_read(connection, netbuffer);
        break;
    case NETWORK_PROTOCOL_UDP:
        UDP_try_read(connection, netbuffer);
        break;
    case NETWORK_PROTOCOL_TLS:
        TLS_try_read(connection, netbuffer);
        break;
    case NETWORK_PROTOCOL_DTLS:
        DTLS_try_read(connection, netbuffer);
        break;
    default:
        UNREACHABLE();
    }
}
void network_try_write(NetworkConnection* connection) {
    if (connection->closed || write_queue_is_empty(&connection->write_queue)) {
        return;
    }

    switch (connection->conn.protocol) {
    case NETWORK_PROTOCOL_TCP:
        TCP_try_write(connection);
        break;
    case NETWORK_PROTOCOL_UDP:
        UDP_try_write(connection);
        break;
    case NETWORK_PROTOCOL_TLS:
        TLS_try_write(connection);
        break;
    case NETWORK_PROTOCOL_DTLS:
        DTLS_try_write(connection);
        break;
    default:
        UNREACHABLE();
    }
}
void network_free_connection(NetworkConnection* connection) {
    switch (connection->conn.protocol) {
    case NETWORK_PROTOCOL_TCP:
        TCP_free_connection(connection);
        break;
    case NETWORK_PROTOCOL_UDP:
        UDP_free_connection(connection);
        break;
    case NETWORK_PROTOCOL_TLS:
        TLS_free_connection(connection);
        break;
    case NETWORK_PROTOCOL_DTLS:
        DTLS_free_connection(connection);
        break;
    default:
        UNREACHABLE();
    }
}

void handle_queued_immediate_close_request() {
    for (uint32 i = 0; i < REQUEST_QUEUE.close_length; i++) {
        NetworkCloseRequest close_request = REQUEST_QUEUE.close_requests[i];
        if (close_request.close_moment == CONNECTION_CLOSE_IMMEDIATE) {
            NetworkConnection* connection = find_network_connection(close_request.socket);
            if (connection != NULL) {
                connection->closed = true;
            }
            else {
                // TODO: handle no connection, or not ???
            }
        }
    }
}

void initiate_queued_requests(BufferSlice netbuffer) {
    for (uint32 i = 0; i < REQUEST_QUEUE.connect_length; i++) {
        NetworkConnectRequest connect_request = REQUEST_QUEUE.connect_requests[i];
        NetworkConnectCommand connect_command = network_init_connect(connect_request);
        network_try_connect(&connect_command);
        store_connect_command(connect_command);
    }

    for (uint32 i = 0; i < REQUEST_QUEUE.read_length; i++) {
        NetworkReadRequest read_request = REQUEST_QUEUE.read_requests[i];
        NetworkConnection* connection = find_network_connection(read_request.socket);
        if (connection != NULL) {
            connection->read_command = (NetworkReadCommand) {
                .read_active = true,
                .on_read_fn = read_request.on_read_fn,
                .arg = read_request.arg,
            };
            network_try_read(connection, netbuffer);
        }
        else {
            // TODO: handle no connection
        }
    }

    for (uint32 i = 0; i < REQUEST_QUEUE.write_length; i++) {
        NetworkWriteRequest write_request = REQUEST_QUEUE.write_requests[i];
        NetworkConnection* connection = find_network_connection(write_request.socket);
        if (connection != NULL) {
            NetworkWriteCommand write_command = {
                .write_addr = write_request.remote_addr,
                .buffer = write_request.buffer,
                .offset = 0,
                .on_write_fn = write_request.on_write_fn,
                .arg = write_request.arg,
            };
            if (write_queue_is_empty(&connection->write_queue)) {
                write_queue_push_command_unchecked(&connection->write_queue, write_command);
                network_try_write(connection);
            }
            else if (!write_queue_is_full(&connection->write_queue)) {
                write_queue_push_command_unchecked(&connection->write_queue, write_command);
            }
            else {
                // TODO: handle full write queue
            }
        }
        else {
            // TODO: handle no connection
        }
    }

    for (uint32 i = 0; i < REQUEST_QUEUE.close_length; i++) {
        NetworkCloseRequest close_request = REQUEST_QUEUE.close_requests[i];
        if (close_request.close_moment == CONNECTION_CLOSE_GRACEFULL) {
            NetworkConnection* connection = find_network_connection(close_request.socket);
            if (connection != NULL) {
                connection->closed = true;
            }
            else {
                // TODO: handle no connection, or not ???
            }
        }
    }

    REQUEST_QUEUE.connect_length = 0;
    REQUEST_QUEUE.read_length = 0;
    REQUEST_QUEUE.write_length = 0;
    REQUEST_QUEUE.close_length = 0;
}

void cleanup_connect_commands() {
    for (uint32 i = 0; i < CONNECT_COMMANDS.length; i++) {
        NetworkConnectCommand connect_command = CONNECT_COMMANDS.commands[i];
        if (connect_command.done) {
            if (connect_command.on_connect_fn != NULL) {
                ((OnConnectFn) connect_command.on_connect_fn)(connect_command.connect_id, connect_command.conn.socket, connect_command.result, connect_command.arg);
            }
            if (connect_command.result == CONNECT_SUCCESS) {
                NetworkConnection connection = {
                    .conn = connect_command.conn,
                    .want_read = false,
                    .want_write = false,
                    .read_command = {0},
                    .write_queue = write_queue_new_empty(),
                    .closed = false,
                };
                store_network_connection(connection);
            }
            // swap remove
            CONNECT_COMMANDS.commands[i] = CONNECT_COMMANDS.commands[CONNECT_COMMANDS.length];
            CONNECT_COMMANDS.length--;
            i--;
        }
    }
}

void cleanup_connection_commands() {
    for (uint32 i = 0; i < NETWORK_CONNECTIONS.length; i++) {
        NetworkConnection* connection = &NETWORK_CONNECTIONS.connections[i];
        if (connection->closed) {
            network_free_connection(connection);
            // swap remove
            NETWORK_CONNECTIONS.connections[i] = NETWORK_CONNECTIONS.connections[NETWORK_CONNECTIONS.length];
            NETWORK_CONNECTIONS.length--;
            i--;
        }
    }
    for (uint32 i = 0; i < NETWORK_CONNECTIONS.length; i++) {
        NetworkConnection* connection = &NETWORK_CONNECTIONS.connections[i];
        if (!write_queue_is_empty(&connection->write_queue)) {
            NetworkWriteCommand this_write = connection->write_queue.queue[connection->write_queue.head];
            if (this_write.offset == this_write.buffer.length) {
                // write complete
                if (this_write.on_write_fn != NULL) {

                    SocketAddr remote_addr;
                    switch (connection->conn.protocol) {
                    case NETWORK_PROTOCOL_TCP:
                    case NETWORK_PROTOCOL_TLS:
                    case NETWORK_PROTOCOL_DTLS:
                        remote_addr = connection->conn.remote_addr;
                        break;
                    case NETWORK_PROTOCOL_UDP:
                        remote_addr = this_write.write_addr;
                        break;
                    }
                    
                    WriteContext context = {
                        .socket = connection->conn.socket,
                        .remote_addr = remote_addr,
                    };
                    ((OnWriteFn) this_write.on_write_fn)(context, this_write.arg);
                }
                write_queue_pop_command_unchecked(&connection->write_queue);
            }
        }
    }
}

void handle_read(fd_set* read_sockets, BufferSlice netbuffer) {
    JUST_LOG_TRACE("handle_read: %d\n", read_sockets->fd_count);
    for (uint32 i = 0; i < read_sockets->fd_count; i++) {
        Socket socket = read_sockets->fd_array[i];
        if (socket == interrupt_socket) continue;

        NetworkConnection* connection = find_network_connection(socket);
        if (connection != NULL) {
            network_try_read(connection, netbuffer);
        }
        else {
            NetworkConnectCommand* connect_command = find_connect_command(socket);
            if (connect_command != NULL) {
                network_try_connect(connect_command);
            }
        }
    }
}

void handle_write(fd_set* write_sockets) {
    JUST_LOG_TRACE("handle_write: %d\n", write_sockets->fd_count);
    for (uint32 i = 0; i < write_sockets->fd_count; i++) {
        Socket socket = write_sockets->fd_array[i];
        if (socket == interrupt_socket) continue;

        NetworkConnection* connection = find_network_connection(socket);
        if (connection != NULL) {
            network_try_write(connection);
        }
        else {
            NetworkConnectCommand* connect_command = find_connect_command(socket);
            if (connect_command != NULL) {
                network_try_connect(connect_command);
            }
        }
    }
}

void handle_except(fd_set* except_sockets) {
    JUST_LOG_TRACE("handle_except: %d\n", except_sockets->fd_count);
    for (uint32 i = 0; i < except_sockets->fd_count; i++) {
        Socket socket = except_sockets->fd_array[i];
        if (socket == interrupt_socket) continue;

        NetworkConnectCommand* connect_command = find_connect_command(socket);
        if (connect_command != NULL) {
            network_try_connect(connect_command);
        }
    }
}

void spin_network_worker(Buffer NETWORK_BUFFER) {
    fd_set read_sockets = {0};
    fd_set write_sockets = {0};
    fd_set except_sockets = {0};

    while (1) {
        FD_ZERO(&read_sockets);
        FD_ZERO(&write_sockets);
        FD_ZERO(&except_sockets);

        init_interrupt();
        FD_SET(interrupt_socket, &read_sockets);

        for (uint32 i = 0; i < CONNECT_COMMANDS.length; i++) {
            NetworkConnectCommand* connect_command = &NETWORK_CONNECTIONS.connections[i];
            switch (connect_command->current_direction) {
            case COMM_DIRECTION_READ:
                FD_SET(connect_command->conn.fd, &read_sockets);
                break;
            case COMM_DIRECTION_WRITE:
                FD_SET(connect_command->conn.fd, &write_sockets);
                break;
            }
            FD_SET(connect_command->conn.fd, &except_sockets); // fail ???
        }

        for (uint32 i = 0; i < NETWORK_CONNECTIONS.length; i++) {
            NetworkConnection* connection = &NETWORK_CONNECTIONS.connections[i];
            uint64 fd = connection->conn.fd;
            if (connection->want_read && connection->read_command.read_active) {
                FD_SET(connection->conn.fd, &read_sockets);
            }
            if (connection->want_write) {
                FD_SET(connection->conn.fd, &write_sockets);
            }
            NETWORK_CONNECTIONS.connections[i].want_read = false;
            NETWORK_CONNECTIONS.connections[i].want_write = false;
        }

        JUST_LOG_TRACE("read_sockets: %d\n", read_sockets.fd_count);
        JUST_LOG_TRACE("write_sockets: %d\n", write_sockets.fd_count);
        JUST_LOG_TRACE("except_sockets: %d\n", except_sockets.fd_count);

        JUST_LOG_TRACE("START: select\n");

        int ready_count = select(
            0,
            read_sockets.fd_count == 0 ? NULL : &read_sockets,
            write_sockets.fd_count == 0 ? NULL : &write_sockets,
            except_sockets.fd_count == 0 ? NULL : &except_sockets,
            NULL
        );

        JUST_LOG_TRACE("END: select\n");
    
        if (ready_count == SOCKET_ERROR) {
            int err = WSAGetLastError();
            JUST_LOG_WARN("select error: %d\n", err);
            continue;
        }

        if (is_interrupted()) {
            JUST_LOG_TRACE("select was interrupted\n");
        }

        JUST_LOG_TRACE("START: handle immediate close\n");

        handle_queued_immediate_close_request();

        JUST_LOG_TRACE("START: handle ready sockets\n");

        handle_read(&read_sockets, NETWORK_BUFFER);
        handle_write(&write_sockets);
        handle_except(&except_sockets);
        
        JUST_LOG_TRACE("START: handle socket queue\n");

        initiate_queued_requests(NETWORK_BUFFER);
        
        JUST_LOG_TRACE("START: cleanup\n");

        cleanup_connect_commands();
        cleanup_connection_commands();

        JUST_LOG_TRACE("END: ---\n");
    }
}

// DWORD WINAPI thread_pool_worker_thread(LPVOID lpParam);
uint32 __stdcall network_thread_main(void* thread_param) {
    #define NETWORK_BUFFER_LEN 10000
    byte* NETWORK_BUFFER_MEMORY = malloc(NETWORK_BUFFER_LEN);
    Buffer NETWORK_BUFFER = {
        .bytes = NETWORK_BUFFER_MEMORY,
        .length = NETWORK_BUFFER_LEN,
    };
    #undef NETWORK_BUFFER_LEN

    spin_network_worker(NETWORK_BUFFER);
    // TODO: spins infinite, should I gracefull terminate ???

    // Cleanup
    free_srw_lock(NETWORK_THREAD_LOCK);
    free(NETWORK_BUFFER_MEMORY);

    _endthreadex(0);
}

/**
 * BELOW IS THE PUBLIC API
 * CALLED FROM ANOTHER THREAD
 * TO INTERRACT WITH THE NETWORK THREAD
 */

void init_network_thread() {
    // Initialize Winsock
    WSADATA wsa_data;
    int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (result != NO_ERROR) {
        PANIC("WSAStartup function failed with error: %d\n", result);
    }

    if (!create_ssl_contexts()) {
        PANIC("Could not create ssl context objects\n");
    }

    NETWORK_THREAD_LOCK = alloc_create_srw_lock();
    NETWORK_THREAD = (HANDLE) _beginthreadex( // NATIVE CODE
        NULL,                       // default security attributes
        0,                          // use default stack size  
        network_thread_main,        // thread function name
        NULL,                       // argument to thread function 
        0,                          // use default creation flags 
        &NETWORK_THREAD_ID          // returns the thread identifier 
    );
}

void network_connect(SocketAddr remote_addr, NetworkProtocolEnum protocol, uint32 connect_id, OnConnectFn on_connect, void* arg) {
    NetworkConnectRequest request = {
        .protocol = protocol,
        .connect_id = connect_id,
        .remote_addr = remote_addr,
        .on_connect_fn = on_connect,
        .arg = arg,
    };
    queue_connect_request(request);

    if (GetCurrentThreadId() != NETWORK_THREAD_ID) {
        issue_interrupt_apc();
    }
}

void network_start_read(Socket socket, OnReadFn on_read, void* arg) {
    NetworkReadRequest request = {
        .socket = socket,
        .on_read_fn = on_read,
        .arg = arg,
    };
    queue_read_request(request);

    if (GetCurrentThreadId() != NETWORK_THREAD_ID) {
        issue_interrupt_apc();
    }
}

void network_write_buffer(Socket socket, BufferSlice buffer, OnWriteFn on_write, void* arg) {
    NetworkWriteRequest request = {
        .socket = socket,
        .remote_addr = {0},
        .buffer = buffer,
        .on_write_fn = on_write,
        .arg = arg,
    };
    queue_write_request(request);

    if (GetCurrentThreadId() != NETWORK_THREAD_ID) {
        issue_interrupt_apc();
    }
}

void network_write_buffer_to(Socket socket, SocketAddr remote_addr, BufferSlice buffer, OnWriteFn on_write, void* arg) {
    NetworkWriteRequest request = {
        .socket = socket,
        .remote_addr = remote_addr,
        .buffer = buffer,
        .on_write_fn = on_write,
        .arg = arg,
    };
    queue_write_request(request);

    if (GetCurrentThreadId() != NETWORK_THREAD_ID) {
        issue_interrupt_apc();
    }
}

void network_close_connection(Socket socket, ConnectionCloseEnum close, OnCloseFn on_close, void* arg) {
    NetworkCloseRequest request = {
        .socket = socket,
        .close_moment = close,
        .on_close_fn = on_close,
        .arg = arg,
    };
    queue_close_request(request);

    if (GetCurrentThreadId() != NETWORK_THREAD_ID) {
        issue_interrupt_apc();
    }
}

/**
 * return 0 (false) for big endian, 1 (true) for little endian.
*/
static inline bool check_system_endianness() {
    volatile static uint32 val = 1;
    return (*((uint8*)(&val)));
}

uint16 just_htons(uint16 hostnum) {
    return htons(hostnum);
}
uint32 just_htonl(uint32 hostnum) {
    return htonl((unsigned long) hostnum);
}
uint64 just_htonll(uint64 hostnum) {
    return check_system_endianness()
        ? _byteswap_uint64(hostnum) // system little -> needs swap
        : hostnum;                  // system big    -> no swap
}

uint16 just_ntohs(uint16 netnum) {
    return ntohs(netnum);
}
uint32 just_ntohl(uint32 netnum) {
    return ntohl(netnum);
}
uint64 just_ntohll(uint64 netnum) {
    return check_system_endianness()
        ? _byteswap_uint64(netnum)  // system little -> needs swap
        : netnum;                   // system big    -> no swap
}