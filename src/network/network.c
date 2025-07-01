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
    NETWORK_PROTOCOL_TCP = 0,
    NETWORK_PROTOCOL_UDP,
    NETWORK_PROTOCOL_TLS,
    NETWORK_PROTOCOL_DTLS,
} NetworkProtocolEnum;

typedef enum {
    CONNECT_SUCCESS = 0,
    CONNECT_FAIL_GENERAL,
    CONNECT_FAIL_SSL_GENERAL,
    CONNECT_FAIL_SSL_VERIFY,
} ConnectResult;

typedef enum {
    COMM_DIRECTION_READ = 0,
    COMM_DIRECTION_WRITE,
    COMM_DIRETION_BOTH,
} CommDirection;

typedef struct {
    char* host;
    char* service;
    uint16 port;
} SocketAddr;

typedef void (*OnConnectFn)(uint32 connect_id, Socket socket, ConnectResult result, void* arg);
typedef bool (*OnReadFnStream)(Socket socket, BufferSlice read_buffer, void* arg);
typedef void (*OnWriteFnStream)(Socket socket, void* arg);
typedef bool (*OnReadFnDatagram)(Socket socket, SocketAddr addr, BufferSlice datagram, void* arg);
typedef void (*OnWriteFnDatagram)(Socket socket, SocketAddr addr, void* arg);

#pragma region WriteCommandQueue
typedef struct {
    SocketAddr write_addr; // datagram
    BufferSlice buffer;
    usize offset;
    // --
    void* on_write_fn;
    void* arg;
} WriteCommand;

#define SOCKET_WRITE_QUEUE_CAPACITY 10
typedef struct {
    uint32 capacity;
    uint32 count;
    WriteCommand queue[SOCKET_WRITE_QUEUE_CAPACITY];
    uint32 head; // pop position
    uint32 tail; // push position
} WriteCommandQueue;

WriteCommandQueue write_queue_new_empty() {
    WriteCommandQueue queue = {0};
    queue.capacity = SOCKET_WRITE_QUEUE_CAPACITY;
    return queue;
}

bool write_queue_is_full(WriteCommandQueue* queue) {
    return queue->count == queue->capacity;
}

bool write_queue_is_empty(WriteCommandQueue* queue) {
    return queue->count == 0;
}

bool write_queue_has_next(WriteCommandQueue* queue) {
    return queue->count > 0;
}

void write_queue_push_command_unchecked(WriteCommandQueue* queue, WriteCommand command) {
    queue->queue[queue->tail] = command;
    queue->tail = (queue->tail + 1) % queue->capacity;
    queue->count++;
}

WriteCommand write_queue_pop_command_unchecked(WriteCommandQueue* queue) {
    WriteCommand command = queue->queue[queue->head];
    queue->count--;
    queue->head = (queue->head + 1) % queue->capacity;
    return command;
}
#pragma endregion

typedef struct {
    NetworkProtocolEnum protocol;
    SocketAddr remote_addr;
    Socket socket;
    BIO* ssl_bio; // protocol: TLS | DTLS
    CommDirection current_direction;
} ConnectionState;

uint64 get_connection_fd(ConnectionState* conn) {
    switch (conn->protocol) {
    case NETWORK_PROTOCOL_TCP:
    case NETWORK_PROTOCOL_UDP:
        return conn->socket;
    case NETWORK_PROTOCOL_TLS:
    case NETWORK_PROTOCOL_DTLS:
        return BIO_get_fd(conn->ssl_bio, NULL); // TODO: can return zero/negative on error
    }
    UNREACHABLE();
}

typedef struct {
    ConnectionState conn;
    uint32 connect_id;
    void* on_connect_fn;
    void* arg;
    // --
    bool done;
    ConnectResult result;
} ConnectSocket;

// typedef struct {
//     ConnectSocket connect_socket;
//     bool handshake_want_write; // want_write: true, want_read: false
// } SSLHandshakeSocket;

/**
 * ReadSocket handles read until removed using the .should_remove flag
 * return value of the .on_read function determines .should_remove 
 */
typedef struct {
    ConnectionState conn;
    void* on_read_fn;
    void* arg;
    // --
    bool should_remove;
} ReadSocket;

/**
 * WriteSocket handles write once
 * if you want to write again you should create again
 */
typedef struct {
    ConnectionState conn;
    WriteCommandQueue write_queue;
} WriteSocket;

typedef struct {
    uint32 length;
    ConnectSocket sockets[FD_SETSIZE];
} ConnectSocketList;

// typedef struct {
//     uint32 length;
//     SSLHandshakeSocket sockets[FD_SETSIZE];
// } SSLHandshakeSocketList;

typedef struct {
    uint32 length;
    ReadSocket sockets[FD_SETSIZE];
} ReadSocketList;

typedef struct {
    uint32 length;
    WriteSocket sockets[FD_SETSIZE];
} WriteSocketList;

ConnectSocketList CONNECT_SOCKETS = {0};
// SSLHandshakeSocketList SSLHANDSHAKE_SOCKETS = {0};
ReadSocketList READ_SOCKETS = {0};
WriteSocketList WRITE_SOCKETS = {0};

ConnectSocket* get_as_connect_socket(Socket socket) {
    for (uint32 i = 0; i < CONNECT_SOCKETS.length; i++) {
        ConnectSocket* connect_socket = &CONNECT_SOCKETS.sockets[i];
        if (socket == connect_socket->conn.socket) {
            return connect_socket;
        }
    }
    return NULL;
}

// ConnectSocket* get_as_sslhandshake_socket(Socket socket) {
//     for (uint32 i = 0; i < SSLHANDSHAKE_SOCKETS.length; i++) {
//         SSLHandshakeSocket* sslhandshake_socket = &SSLHANDSHAKE_SOCKETS.sockets[i];
//         if (socket == sslhandshake_socket->connect_socket.socket) {
//             return sslhandshake_socket;
//         }
//     }
//     return NULL;
// }

ReadSocket* get_as_read_socket(Socket socket) {
    for (uint32 i = 0; i < READ_SOCKETS.length; i++) {
        ReadSocket* read_socket = &READ_SOCKETS.sockets[i];
        if (socket == read_socket->conn.socket) {
            return read_socket;
        }
    }
    return NULL;
}

WriteSocket* get_as_write_socket(Socket socket) {
    for (uint32 i = 0; i < WRITE_SOCKETS.length; i++) {
        WriteSocket* write_socket = &WRITE_SOCKETS.sockets[i];
        if (socket == write_socket->conn.socket) {
            return write_socket;
        }
    }
    return NULL;
}

typedef struct {
    Socket socket;
    BIO* ssl_bio;
} SSLConnection;

typedef struct {
    uint32 length;
    SSLConnection connections[FD_SETSIZE];
} SSLConnections;

SSLConnections SSL_CONNECTIONS = {0};

void store_ssl_connection(Socket socket, BIO* ssl_bio) {
    SSL_CONNECTIONS.connections[SSL_CONNECTIONS.length++] = (SSLConnection) {
        .socket = socket,
        .ssl_bio = ssl_bio,
    };
}

BIO* find_ssl_connection(Socket socket) {
    for (uint32 i = 0; i < SSL_CONNECTIONS.length; i++) {
        if (SSL_CONNECTIONS.connections[i].socket == socket) {
            return SSL_CONNECTIONS.connections[i].ssl_bio;
        }
    }
    return NULL;
}

ConnectSocketList QUEUE_CONNECT_SOCKETS = {0};
ReadSocketList QUEUE_READ_SOCKETS = {0};
WriteSocketList QUEUE_WRITE_SOCKETS = {0};

void queue_add_connect_socket(ConnectionState conn, uint32 connect_id, void* on_connect_fn, void* arg) {
    srw_lock_acquire_exclusive(NETWORK_THREAD_LOCK);

    if (QUEUE_CONNECT_SOCKETS.length < FD_SETSIZE) {
        QUEUE_CONNECT_SOCKETS.sockets[QUEUE_CONNECT_SOCKETS.length++] = (ConnectSocket) {
            .conn = conn,
            .connect_id = connect_id,
            .on_connect_fn = on_connect_fn,
            .arg = arg,
            .done = false,
            .result = CONNECT_SUCCESS,
        };
    }

    srw_lock_release_exclusive(NETWORK_THREAD_LOCK);
}
void queue_add_read_socket(ConnectionState conn, void* on_read_fn, void* arg) {
    srw_lock_acquire_exclusive(NETWORK_THREAD_LOCK);

    if (QUEUE_CONNECT_SOCKETS.length < FD_SETSIZE) {
        QUEUE_READ_SOCKETS.sockets[QUEUE_READ_SOCKETS.length++] = (ReadSocket) {
            .conn = conn,
            .arg = arg,
            .should_remove = false,
        };
    }

    srw_lock_release_exclusive(NETWORK_THREAD_LOCK);
}
void queue_add_write_socket(ConnectionState conn, BufferSlice buffer, void* on_write_fn, void* arg) {
    srw_lock_acquire_exclusive(NETWORK_THREAD_LOCK);
    
    if (QUEUE_CONNECT_SOCKETS.length < FD_SETSIZE) {
        WriteCommand command = {
            .write_addr = conn.remote_addr,
            .buffer = buffer,
            .offset = 0,
            .on_write_fn = on_write_fn,
            .arg = arg,
        };
        for (uint32 i = 0; i < QUEUE_WRITE_SOCKETS.length; i++) {
            WriteSocket* write_socket = &QUEUE_WRITE_SOCKETS.sockets[i];
            if (conn.socket == write_socket->conn.socket) {
                if (!write_queue_is_full(&write_socket->write_queue)) {
                    write_queue_push_command_unchecked(&write_socket->write_queue, command);
                }
                return;
            }
        }
        // no queue, create new

        WriteCommandQueue queue = write_queue_new_empty();
        write_queue_push_command_unchecked(&queue, command);
        QUEUE_WRITE_SOCKETS.sockets[QUEUE_WRITE_SOCKETS.length++] = (WriteSocket) {
            .conn = conn,
            .write_queue = queue,
        };
    }

    srw_lock_release_exclusive(NETWORK_THREAD_LOCK);
}
// void queue_add_read_socket_datagram(Socket socket, OnReadFnDatagram on_read, void* arg) {
//     QUEUE_READ_SOCKETS.sockets[QUEUE_READ_SOCKETS.length++] = (ReadSocket) {
//         .type = SOCKET_TYPE_DATAGRAM,
//         .socket = socket,
//         .on_read_fn = on_read,
//         .arg = arg,
//         .should_remove = false,
//     };
// }
// void queue_add_write_socket_datagram(Socket socket, SocketAddr addr, BufferSlice datagram, OnWriteFnDatagram on_write, void* arg) {
//     WriteCommand command = {
//         .write_addr = addr,
//         .buffer = datagram,
//         .offset = 0,
//         .on_write_fn = on_write,
//         .arg = arg,
//     };
//     for (uint32 i = 0; i < QUEUE_WRITE_SOCKETS.length; i++) {
//         WriteSocket* write_socket = &QUEUE_WRITE_SOCKETS.sockets[i];
//         if (socket == write_socket->socket) {
//             write_queue_push_command_unchecked(&write_socket->write_queue, command);
//             return;
//         }
//     }

//     WriteCommandQueue queue = write_queue_new_empty();
//     write_queue_push_command_unchecked(&queue, command);
//     QUEUE_WRITE_SOCKETS.sockets[QUEUE_WRITE_SOCKETS.length++] = (WriteSocket) {
//         .type = SOCKET_TYPE_DATAGRAM,
//         .socket = socket,
//         .write_queue = queue,
//     };
// }

void handle_queued_sockets() {
    for (uint32 i = 0; i < QUEUE_CONNECT_SOCKETS.length; i++) {
        CONNECT_SOCKETS.sockets[CONNECT_SOCKETS.length++] = QUEUE_CONNECT_SOCKETS.sockets[i];
    }
    for (uint32 i = 0; i < QUEUE_READ_SOCKETS.length; i++) {
        READ_SOCKETS.sockets[READ_SOCKETS.length++] = QUEUE_READ_SOCKETS.sockets[i];
    }
    for (uint32 i = 0; i < QUEUE_WRITE_SOCKETS.length; i++) {
        WriteSocket queued_write_socket = QUEUE_WRITE_SOCKETS.sockets[i];
        WriteSocket* write_socket = get_as_write_socket(QUEUE_WRITE_SOCKETS.sockets[i].conn.socket);
        if (write_socket != NULL) {
            // Add to current queue
            while (write_queue_has_next(&queued_write_socket.write_queue) && !write_queue_is_full(&write_socket->write_queue)) {
                WriteCommand write_command = write_queue_pop_command_unchecked(&queued_write_socket.write_queue);
                write_queue_push_command_unchecked(&write_socket->write_queue, write_command);
            }
        }
        else {
            // Create new write queue for socket
            WRITE_SOCKETS.sockets[WRITE_SOCKETS.length++] = QUEUE_WRITE_SOCKETS.sockets[i];
        }
    }
    QUEUE_CONNECT_SOCKETS.length = 0;
    QUEUE_READ_SOCKETS.length = 0;
    QUEUE_WRITE_SOCKETS.length = 0;
}

void cleanup_connect_sockets() {
    for (uint32 i = 0; i < CONNECT_SOCKETS.length; i++) {
        ConnectSocket connect_socket = CONNECT_SOCKETS.sockets[i];
        if (connect_socket.done) {
            // connection done or failed
            switch (connect_socket.conn.protocol) {
            // SSL
            case NETWORK_PROTOCOL_TLS:
            case NETWORK_PROTOCOL_DTLS:
                // connect_socket.done = false;

                // SSL* ssl = SSL_new(TLS_ctx);
                // if (ssl == NULL) {
                //     JUST_LOG_ERROR("Failed to create the SSL object\n");
                //     connect_socket.done = true;
                //     connect_socket.result = CONNECT_FAIL_SSL_GENERAL;
                //     goto FAIL;
                // }

                // BIO* bio = BIO_new(BIO_s_socket());
                // if (bio == NULL) {
                //     JUST_LOG_ERROR("Failed BIO_new\n");
                //     connect_socket.done = true;
                //     connect_socket.result = CONNECT_FAIL_SSL_GENERAL;
                //     goto FAIL;
                // }
                // BIO_set_fd(bio, socket, BIO_CLOSE);

                // SSL_set_bio(ssl, bio, bio);

                // if (!SSL_set_tlsext_host_name(ssl, connect_socket.addr.host)) {
                //     JUST_LOG_ERROR("Failed to set the SNI hostname\n");
                //     connect_socket.done = true;
                //     connect_socket.result = CONNECT_FAIL_SSL_GENERAL;
                //     goto FAIL;
                // }
                // if (!SSL_set1_host(ssl, connect_socket.addr.host)) {
                //     JUST_LOG_ERROR("Failed to set the certificate verification hostname\n");
                //     connect_socket.done = true;
                //     connect_socket.result = CONNECT_FAIL_SSL_GENERAL;
                //     goto FAIL;
                // }
                // connect_socket.ssl = ssl;

                // int32 ssl_result = SSL_connect(connect_socket.ssl);
                // if (ssl_result == 1) {
                //     connect_socket.done = true;
                // }
                // else {
                //     switch (SSL_get_error(connect_socket.ssl, ssl_result)) {
                //     case SSL_ERROR_WANT_READ:
                //         SSLHANDSHAKE_SOCKETS.sockets[SSLHANDSHAKE_SOCKETS.length++] = (SSLHandshakeSocket) {
                //             .connect_socket = connect_socket,
                //             .handshake_want_write = false,
                //         };
                //         break;
                //     case SSL_ERROR_WANT_WRITE:
                //         SSLHANDSHAKE_SOCKETS.sockets[SSLHANDSHAKE_SOCKETS.length++] = (SSLHandshakeSocket) {
                //             .connect_socket = connect_socket,
                //             .handshake_want_write = true,
                //         };
                //         break;
                //     case SSL_ERROR_ZERO_RETURN:
                //     case SSL_ERROR_SYSCALL:
                //     case SSL_ERROR_SSL:
                //         connect_socket.done = true;
                //         connect_socket.result = CONNECT_FAIL_SSL_GENERAL;
                //         int64 ssl_verify_result = SSL_get_verify_result(connect_socket.ssl);
                //         if (ssl_verify_result != X509_V_OK) {
                //             JUST_LOG_ERROR(
                //                 "Verify error: %s\n",
                //                 X509_verify_cert_error_string(ssl_verify_result)
                //             );
                //             connect_socket.result = CONNECT_FAIL_SSL_VERIFY;
                //         }
                //         break;
                //     }
                // }
                // FAIL:
                // break;
                
                store_ssl_connection(connect_socket.conn.socket, connect_socket.conn.ssl_bio);
            // NO-SSL
            case NETWORK_PROTOCOL_TCP:
            case NETWORK_PROTOCOL_UDP:
                if (connect_socket.on_connect_fn != NULL) {
                    ((OnConnectFn) connect_socket.on_connect_fn)(connect_socket.connect_id, connect_socket.conn.socket, connect_socket.result, connect_socket.arg);
                }
                break;
            }
            // swap remove
            CONNECT_SOCKETS.sockets[i] = CONNECT_SOCKETS.sockets[CONNECT_SOCKETS.length];
            CONNECT_SOCKETS.length--;
            i--;
        }
    }
}

// void cleanup_sslhandshake_sockets() {
//     for (uint32 i = 0; i < SSLHANDSHAKE_SOCKETS.length; i++) {
//         SSLHandshakeSocket sslhandshake_socket = SSLHANDSHAKE_SOCKETS.sockets[i];
//         if (sslhandshake_socket.connect_socket.done) {
//             // connection done or failed
//             ConnectSocket connect_socket = sslhandshake_socket.connect_socket;
//             store_ssl_connection(connect_socket.socket, connect_socket.ssl);
//             if (connect_socket.on_connect_fn != NULL) {
//                 ((OnConnectFn) connect_socket.on_connect_fn)(connect_socket.connect_id, connect_socket.socket, connect_socket.result, connect_socket.arg);
//             }
//         }
//     }
// }

void cleanup_read_sockets() {
    for (uint32 i = 0; i < READ_SOCKETS.length; i++) {
        ReadSocket read_socket = READ_SOCKETS.sockets[i];
        if (read_socket.should_remove) {
            // read complete
            // swap remove
            READ_SOCKETS.sockets[i] = READ_SOCKETS.sockets[READ_SOCKETS.length];
            READ_SOCKETS.length--;
            i--;
        }
    }
}

void cleanup_write_sockets() {
    for (uint32 i = 0; i < WRITE_SOCKETS.length; i++) {
        WriteSocket write_socket = WRITE_SOCKETS.sockets[i];

        WriteCommand this_write = write_socket.write_queue.queue[write_socket.write_queue.head];
        if (this_write.offset == this_write.buffer.length) {
            // write complete
            if (this_write.on_write_fn != NULL) {
                switch (write_socket.protocol) {
                // STREAM
                case NETWORK_PROTOCOL_TCP:
                case NETWORK_PROTOCOL_TLS:
                    ((OnWriteFnStream) this_write.on_write_fn)(write_socket.socket, this_write.arg);
                    break;
                // DATAGRAM
                case NETWORK_PROTOCOL_UDP:
                case NETWORK_PROTOCOL_DTLS:
                    ((OnWriteFnDatagram) this_write.on_write_fn)(write_socket.socket, this_write.write_addr, this_write.arg);
                    break;
                }
            }
            write_queue_pop_command_unchecked(&write_socket.write_queue);
            if (write_queue_is_empty(&write_socket.write_queue)) {
                // swap remove
                WRITE_SOCKETS.sockets[i] = WRITE_SOCKETS.sockets[WRITE_SOCKETS.length];
                WRITE_SOCKETS.length--;
                i--;
            }
        }
    }
}

void handle_read(fd_set* read_sockets, BufferSlice buffer) {
    JUST_LOG_TRACE("handle_read: %d\n", read_sockets->fd_count);
    int32 received_count;
    for (uint32 i = 0; i < read_sockets->fd_count; i++) {
        Socket socket = read_sockets->fd_array[i];
        if (socket == interrupt_socket) continue;

        ReadSocket* read_socket = get_as_read_socket(socket);
        SSLHandshakeSocket* sslhandshake_socket = (read_socket == NULL) ? get_as_sslhandshake_socket(socket) : NULL;

        if (read_socket != NULL) {
            switch (read_socket->type) {
            case SOCKET_TYPE_STREAM:
                JUST_LOG_TRACE("Read Stream Socket\n");

                received_count = recv(socket, buffer.bytes, buffer.length, 0);
                if (received_count == SOCKET_ERROR) {
                    int32 err = WSAGetLastError();
                    switch (err) {
                    case WSAEWOULDBLOCK:
                        // Fine
                        continue;
                    default:
                        // TODO: handle err
                        JUST_LOG_WARN("read err: %d\n", err);
                        continue;
                    }
                }
                JUST_LOG_TRACE("received count: %d\n", received_count);
                
                BufferSlice read_buffer = buffer_as_slice(buffer, 0, received_count);
                read_socket->should_remove = ((OnReadFnStream) read_socket->on_read_fn)(socket, read_buffer, read_socket->arg);

                break;
            
            case SOCKET_TYPE_DATAGRAM:
                JUST_LOG_TRACE("Read Datagram Socket\n");

                SOCKADDR_IN sockaddr = {0};
                int sockaddr_size = sizeof(sockaddr);
                received_count = recvfrom(socket, buffer.bytes, buffer.length, 0, (SOCKADDR*) &sockaddr, &sockaddr_size);
                if (received_count == SOCKET_ERROR) {
                    int32 err = WSAGetLastError();
                    switch (err) {
                    case WSAEWOULDBLOCK:
                        // Fine
                        continue;
                    case WSAEMSGSIZE:
                        // TODO: Partial Read
                        break;
                    default:
                        // TODO: handle err
                        JUST_LOG_WARN("read err: %d\n", err);
                        continue;
                    }
                }
                JUST_LOG_TRACE("received count: %d\n", received_count);

                SocketAddr addr = {
                    .host = inet_ntoa(sockaddr.sin_addr),
                    .port = ntohs(sockaddr.sin_port),
                };

                BufferSlice datagram = buffer_as_slice(buffer, 0, received_count);
                read_socket->should_remove = ((OnReadFnDatagram) read_socket->on_read_fn)(socket, addr, datagram, read_socket->arg);

                break;
            }
        }
        // SSLHandshake
        else if (sslhandshake_socket != NULL) {
            // TODO: progress SSL_connect
        }
    }
}

void handle_write(fd_set* write_sockets) {
    JUST_LOG_TRACE("handle_write: %d\n", write_sockets->fd_count);
    int32 sent_count;
    for (uint32 i = 0; i < write_sockets->fd_count; i++) {
        Socket socket = write_sockets->fd_array[i];
        if (socket == interrupt_socket) continue;

        WriteSocket* write_socket = get_as_write_socket(socket);
        ConnectSocket* connect_socket = (write_socket == NULL) ? get_as_connect_socket(socket) : NULL;
        SSLHandshakeSocket* sslhandshake_socket = (write_socket == NULL && connect_socket == NULL) ? get_as_sslhandshake_socket(socket) : NULL;

        // Write
        if (write_socket != NULL) {
            WriteCommand* this_write = &write_socket->write_queue.queue[write_socket->write_queue.head];

            switch (write_socket->type) {
            case SOCKET_TYPE_STREAM:
                JUST_LOG_TRACE("Write Stream Socket\n");
                
                sent_count = send(
                    write_socket->socket,
                    this_write->buffer.bytes + this_write->offset,
                    this_write->buffer.length - this_write->offset,
                    0
                );
                if (sent_count == SOCKET_ERROR) {
                    int32 err = WSAGetLastError();
                    switch (err) {
                    case WSAEWOULDBLOCK:
                        // Fine
                        continue;
                    default:
                        // TODO: handle err
                        JUST_LOG_WARN("write err: %d\n", err);
                        continue;
                    }
                }
        
                this_write->offset += sent_count;

                break;
            
            case SOCKET_TYPE_DATAGRAM:
                JUST_LOG_TRACE("Write Datagram Socket\n");

                SOCKADDR_IN sockaddr = {0};
                sockaddr.sin_family = AF_INET;
                sockaddr.sin_addr.s_addr = inet_addr(this_write->write_addr.host);
                sockaddr.sin_port = htons(this_write->write_addr.port);

                sent_count = sendto(
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
                        continue;
                    case WSAEMSGSIZE:
                        // TODO: Partial Write
                        // no data is transmitted
                        this_write->offset = this_write->buffer.length;
                        continue;
                    default:
                        // TODO: handle err
                        JUST_LOG_WARN("write err: %d\n", err);
                        JUST_LOG_WARN("write_addr.host: %s\n", this_write->write_addr.host);
                        JUST_LOG_WARN("write_addr.port: %d\n", this_write->write_addr.port);
                        continue;
                    }
                }
                
                this_write->offset += sent_count;

                break;
            }
        }
        // Connection
        else if (connect_socket != NULL) {
            connect_socket->done = true;
            connect_socket->result = CONNECT_SUCCESS;
        }
        // SSLHandshake
        else if (sslhandshake_socket != NULL) {
            // TODO: progress SSL_connect
        }
    }
}

void handle_except(fd_set* except_sockets) {
    JUST_LOG_TRACE("handle_except: %d\n", except_sockets->fd_count);
    int32 sent_count;
    for (uint32 i = 0; i < except_sockets->fd_count; i++) {
        Socket socket = except_sockets->fd_array[i];
        if (socket == interrupt_socket) continue;

        ConnectSocket* connect_socket = get_as_connect_socket(socket);

        if (connect_socket != NULL) {
            // TODO: call connect once more to get error code
            connect_socket->done = true;
            connect_socket->result = CONNECT_FAIL_GENERAL;
        }
    }
}

void spin_network_worker() {
    #define NETWORK_BUFFER_LEN 10000
    byte* NETWORK_BUFFER_MEMORY = malloc(NETWORK_BUFFER_LEN);
    Buffer NETWORK_BUFFER = {
        .bytes = NETWORK_BUFFER_MEMORY,
        .length = NETWORK_BUFFER_LEN,
    };
    #undef NETWORK_BUFFER_LEN

    fd_set read_sockets = {0};
    fd_set write_sockets = {0};
    fd_set except_sockets = {0};

    while (1) {
        FD_ZERO(&read_sockets);
        FD_ZERO(&write_sockets);
        FD_ZERO(&except_sockets);

        init_interrupt();
        FD_SET(interrupt_socket, &read_sockets);

        for (uint32 i = 0; i < READ_SOCKETS.length; i++) {
            uint64 fd = get_connection_fd(&READ_SOCKETS.sockets[i].conn);
            switch (READ_SOCKETS.sockets[i].conn.current_direction) {
            case COMM_DIRECTION_READ:
                FD_SET(fd, &read_sockets);
                break;
            case COMM_DIRECTION_WRITE:
                FD_SET(fd, &write_sockets);
                break;
            }
        }
        for (uint32 i = 0; i < WRITE_SOCKETS.length; i++) {
            uint64 fd = get_connection_fd(&WRITE_SOCKETS.sockets[i].conn);
            switch (WRITE_SOCKETS.sockets[i].conn.current_direction) {
            case COMM_DIRECTION_READ:
                FD_SET(fd, &read_sockets);
                break;
            case COMM_DIRECTION_WRITE:
                FD_SET(fd, &write_sockets);
                break;
            }
        }
        for (uint32 i = 0; i < CONNECT_SOCKETS.length; i++) {
            uint64 fd = get_connection_fd(&CONNECT_SOCKETS.sockets[i].conn);
            switch (CONNECT_SOCKETS.sockets[i].conn.current_direction) {
                case COMM_DIRECTION_READ:
                FD_SET(fd, &read_sockets); // success
                break;
                case COMM_DIRECTION_WRITE:
                FD_SET(fd, &write_sockets); // success
                break;
            }
            FD_SET(fd, &except_sockets); // fail
        }
        // for (uint32 i = 0; i < SSLHANDSHAKE_SOCKETS.length; i++) {
        //     int32 ssl_fd = SSL_get_fd(SSLHANDSHAKE_SOCKETS.sockets[i].connect_socket.ssl);
        //     if (SSLHANDSHAKE_SOCKETS.sockets[i].handshake_want_write) {
        //         FD_SET(ssl_fd, &write_sockets);
        //     }
        //     else {
        //         FD_SET(ssl_fd, &read_sockets);
        //     }
        // }

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

        JUST_LOG_TRACE("START: handle ready sockets\n");

        handle_read(&read_sockets, NETWORK_BUFFER);
        handle_write(&write_sockets);
        handle_except(&except_sockets);
        
        JUST_LOG_TRACE("START: cleanup\n");

        cleanup_connect_sockets();
        cleanup_sslhandshake_sockets();
        cleanup_read_sockets();
        cleanup_write_sockets();
        
        JUST_LOG_TRACE("START: handle socket queue\n");

        handle_queued_sockets();

        JUST_LOG_TRACE("END: ---\n");
    }
}

// DWORD WINAPI thread_pool_worker_thread(LPVOID lpParam);
uint32 __stdcall network_thread_main(void* thread_param) {
    spin_network_worker();

    // Cleanup
    free_srw_lock(NETWORK_THREAD_LOCK);

    _endthreadex(0);
}

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
    }

    return true;
    FAIL:
    return false;
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

bool network_tcp_connect(uint32 connect_id, SocketAddr addr, OnConnectFn on_connect, void* arg) {
    Socket socket = make_socket(SOCKET_TYPE_STREAM);
    if (socket == INVALID_SOCKET) {
        goto FAIL;
    }

    SOCKADDR_IN sockaddr = {0};
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_addr.s_addr = inet_addr(addr.host);
    sockaddr.sin_port = htons(addr.port);

    int32 result = connect(socket, (SOCKADDR*) &sockaddr, sizeof(sockaddr));
    if (result == SOCKET_ERROR) {
        int32 err = WSAGetLastError();
        switch (err) {
        case WSAEWOULDBLOCK:
            // Fine
            JUST_LOG_INFO("Connection started\n");
            break;
        default:
            // TODO: handle err
            JUST_LOG_ERROR("Connection failed: %d\n", err);
            break;
        }
    }
    
    ConnectionState conn = {
        .protocol = NETWORK_PROTOCOL_TCP,
        .remote_addr = addr,
        .socket = socket,
        .ssl_bio = NULL,
        .current_direction = COMM_DIRECTION_WRITE,
    };
    queue_add_connect_socket(conn, connect_id, on_connect, arg);

    if (GetCurrentThreadId() != NETWORK_THREAD_ID) {
        // Called from another thread
        issue_interrupt_apc();
    }
    // else {
        // Called within network thread
        // probably from on_connect, on_read or on_write
    // }

    return true;
    FAIL:
    return false;
}

bool network_tls_connect(uint32 connect_id, SocketAddr addr, OnConnectFn on_connect, void* arg) {
    // Socket socket;

    // char port_str[10] = {0};
    // sprintf(port_str, "%d", addr.port);

    // BIO_ADDRINFO *res;
    // const BIO_ADDRINFO *ai = NULL;

    // if (!BIO_lookup_ex(addr.host, port_str, BIO_LOOKUP_CLIENT, AF_INET, SOCK_STREAM, IPPROTO_TCP, &res)) {
    //     JUST_LOG_ERROR("Failed BIO_lookup_ex\n");
    //     goto FAIL;
    // }

    // for (ai = res; ai != NULL; ai = BIO_ADDRINFO_next(ai)) {
    //     socket = BIO_socket(BIO_ADDRINFO_family(ai), SOCK_STREAM, IPPROTO_TCP, 0);
    //     if (socket == -1) {
    //         continue;
    //     }
    //     if (!BIO_socket_nbio(sock, 1)) {
    //         sock = -1;
    //         continue;
    //     }

    //     BIO_do_connect()
    //     if (!BIO_connect(sock, BIO_ADDRINFO_address(ai), BIO_SOCK_NODELAY)) {
    //         BIO_closesocket(sock);
    //         sock = -1;
    //         continue;
    //     }

    //     /* We have a connected socket so break out of the loop */
    //     break;
    // }

    // /* Free the address information resources we allocated earlier */
    // BIO_ADDRINFO_free(res);

    // struct hostent* hostaddr = gethostbyname(addr.host);
    // if (hostaddr == NULL) {
    //     JUST_LOG_ERROR("Failed gethostbyname\n");
    //     goto FAIL;
    // }

    // SOCKADDR_IN sockaddr = {0};
    // sockaddr.sin_family = AF_INET;
    // memcpy(&sockaddr.sin_addr.s_addr, hostaddr->h_addr, hostaddr->h_length);
    // sockaddr.sin_port = htons(addr.port);

    // int32 result = connect(socket, (SOCKADDR*) &sockaddr, sizeof(sockaddr));
    // if (result == SOCKET_ERROR) {
    //     int32 err = WSAGetLastError();
    //     switch (err) {
    //     case WSAEWOULDBLOCK:
    //         // Fine
    //         JUST_LOG_INFO("Connection started\n");
    //         break;
    //     default:
    //         // TODO: handle err
    //         JUST_LOG_ERROR("Connection failed: %d\n", err);
    //         break;
    //     }
    // }

    Socket socket;
    CommDirection comm_direction;

    BIO *bio = BIO_new_ssl_connect(TLS_ctx);
    if (!bio) {
        JUST_LOG_ERROR("Failed BIO_new_ssl_connect\n");
        goto FAIL;
    }

    BIO_set_conn_hostname(bio, "example.com:443");
    
    SSL *ssl = NULL;
    BIO_get_ssl(bio, &ssl);
    SSL_set_tlsext_host_name(ssl, "example.com");

    BIO_set_nbio(bio, 1);

    int32 result = BIO_do_connect(bio);
    socket = BIO_get_fd(bio, NULL);
    if (result > 0) {
        comm_direction = COMM_DIRETION_BOTH;
    } else if (BIO_should_retry(bio)) {
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
        // TODO: what to do
        UNREACHABLE();
    }
    
    ConnectionState conn = {
        .protocol = NETWORK_PROTOCOL_TLS,
        .remote_addr = addr,
        .socket = socket,
        .ssl_bio = bio,
        .current_direction = comm_direction,
    };
    queue_add_connect_socket(conn, connect_id, on_connect, arg);

    if (GetCurrentThreadId() != NETWORK_THREAD_ID) {
        issue_interrupt_apc();
    }

    return true;
    FAIL:
    return false;
}

void network_tcp_start_read_stream(Socket socket, OnReadFnStream on_read, void* arg) {
    ConnectionState conn = {
        .protocol = NETWORK_PROTOCOL_TCP,
        .remote_addr = {0},
        .socket = socket,
        .ssl_bio = NULL,
        .current_direction = COMM_DIRECTION_READ,
    };
    queue_add_read_socket(conn, on_read, arg);
    if (GetCurrentThreadId() != NETWORK_THREAD_ID) {
        issue_interrupt_apc();
    }
}

void network_tcp_write_stream(Socket socket, BufferSlice buffer, OnWriteFnStream on_write, void* arg) {
    ConnectionState conn = {
        .protocol = NETWORK_PROTOCOL_TCP,
        .remote_addr = {0},
        .socket = socket,
        .ssl_bio = NULL,
        .current_direction = COMM_DIRECTION_WRITE,
    };
    queue_add_write_socket(conn, buffer, on_write, arg);
    if (GetCurrentThreadId() != NETWORK_THREAD_ID) {
        issue_interrupt_apc();
    }
}

void network_udp_start_read_datagram(Socket socket, OnReadFnDatagram on_read, void* arg) {
    ConnectionState conn = {
        .protocol = NETWORK_PROTOCOL_UDP,
        .remote_addr = {0},
        .socket = socket,
        .ssl_bio = NULL,
        .current_direction = COMM_DIRECTION_READ,
    };
    queue_add_read_socket(conn, on_read, arg);
    if (GetCurrentThreadId() != NETWORK_THREAD_ID) {
        issue_interrupt_apc();
    }
}

void network_udp_write_datagram(Socket socket, SocketAddr addr, BufferSlice datagram, OnWriteFnDatagram on_write, void* arg) {
    ConnectionState conn = {
        .protocol = NETWORK_PROTOCOL_UDP,
        .remote_addr = addr,
        .socket = socket,
        .ssl_bio = NULL,
        .current_direction = COMM_DIRECTION_WRITE,
    };
    queue_add_write_socket(conn, datagram, on_write, arg);
    if (GetCurrentThreadId() != NETWORK_THREAD_ID) {
        issue_interrupt_apc();
    }
}

bool network_tls_start_read_stream(Socket socket, OnReadFnStream on_read, void* arg) {
    BIO* ssl_bio = find_ssl_connection(socket);
    if (ssl_bio == NULL) {
        JUST_LOG_ERROR("No SSL Connection found on socket, wait for connection\n");
        goto FAIL;
    }

    // TODO: should I start read here
    ConnectionState conn = {
        .protocol = NETWORK_PROTOCOL_TLS,
        .remote_addr = {0},
        .socket = socket,
        .ssl_bio = ssl_bio,
        .current_direction = COMM_DIRECTION_READ,
    };
    queue_add_read_socket(conn, on_read, arg);
    if (GetCurrentThreadId() != NETWORK_THREAD_ID) {
        issue_interrupt_apc();
    }
    
    return true;
    FAIL:
    return false;
}

bool network_tls_write_stream(Socket socket, BufferSlice buffer, OnWriteFnStream on_write, void* arg) {
    BIO* ssl_bio = find_ssl_connection(socket);
    if (ssl_bio == NULL) {
        JUST_LOG_ERROR("No SSL Connection found on socket, wait for connection\n");
        goto FAIL;
    }

    ConnectionState conn = {
        .protocol = NETWORK_PROTOCOL_TLS,
        .remote_addr = {0},
        .socket = socket,
        .ssl_bio = ssl_bio,
        .current_direction = COMM_DIRECTION_WRITE,
    };
    queue_add_write_socket(conn, buffer, on_write, arg);
    if (GetCurrentThreadId() != NETWORK_THREAD_ID) {
        issue_interrupt_apc();
    }
    
    return true;
    FAIL:
    return false;
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