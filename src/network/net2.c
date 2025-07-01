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

typedef struct {
    char* host;
    char* service;
    uint16 port;
} SocketAddr;

typedef enum {
    CONNECT_SUCCESS = 0,
    CONNECT_FAIL_GENERAL,
    CONNECT_FAIL_SSL_GENERAL,
    CONNECT_FAIL_SSL_VERIFY,
} ConnectResult;

typedef void (*OnConnectFn)(uint32 connect_id, Socket socket, ConnectResult result, void* arg);
typedef bool (*OnReadFnStream)(Socket socket, BufferSlice read_buffer, void* arg);
typedef void (*OnWriteFnStream)(Socket socket, void* arg);
typedef bool (*OnReadFnDatagram)(Socket socket, SocketAddr addr, BufferSlice datagram, void* arg);
typedef void (*OnWriteFnDatagram)(Socket socket, SocketAddr addr, void* arg);

typedef enum {
    NETWORK_PROTOCOL_TCP = 0,
    NETWORK_PROTOCOL_UDP,
    NETWORK_PROTOCOL_TLS,
    NETWORK_PROTOCOL_DTLS,
} NetworkProtocolEnum;

typedef enum {
    COMM_DIRECTION_READ = 0,
    COMM_DIRECTION_WRITE,
    COMM_DIRETION_BOTH,
} CommDirection;

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
    ConnectResult result;
} NetworkConnectCommand;

typedef struct {
    bool do_read;
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

#define SOCKET_WRITE_QUEUE_CAPACITY 10
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
    CommDirection current_direction;
    NetworkReadCommand read_command;
    NetworkWriteCommandQueue write_queue;
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

void store_network_connection(NetworkConnection netconn) {
    NETWORK_CONNECTIONS.connections[NETWORK_CONNECTIONS.length++] = netconn;
}

NetworkConnection* find_network_connection(Socket socket) {
    for (uint32 i = 0; i < NETWORK_CONNECTIONS.length; i++) {
        if (NETWORK_CONNECTIONS.connections[i].conn.socket == socket) {
            return &NETWORK_CONNECTIONS.connections[i];
        }
    }
    return NULL;
}

#pragma region NetworkRequestQueue
typedef struct {
    NetworkProtocolEnum protocol;
    SocketAddr remote_addr;
    uint32 connect_id;
} NetworkConnectRequest;

typedef struct {
    Socket socket;
    void* on_read_fn;
    void* arg;
} NetworkReadRequest;

typedef struct {
    Socket socket;
    BufferSlice buffer;
    void* on_write_fn;
    void* arg;
} NetworkWriteRequest;

typedef struct {
    uint32 connect_length;
    NetworkConnectRequest connect_requests[FD_SETSIZE];
    uint32 read_length;
    NetworkReadRequest read_requests[FD_SETSIZE];
    uint32 write_length;
    NetworkWriteRequest write_requests[FD_SETSIZE];
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
#pragma endregion NetworkRequestQueue

void TCP_try_read(NetworkConnection* netconn, BufferSlice netbuffer) {
    JUST_LOG_TRACE("Read TCP\n");
    Socket socket = netconn->conn.socket;
    NetworkReadCommand read_command = netconn->read_command;

    int32 received_count = recv(socket, netbuffer.bytes, netbuffer.length, 0);
    if (received_count == SOCKET_ERROR) {
        int32 err = WSAGetLastError();
        switch (err) {
        case WSAEWOULDBLOCK:
            // Fine
            break;
        default:
            // TODO: handle err
            JUST_LOG_WARN("read err: %d\n", err);
            break;
        }
    }
    JUST_LOG_TRACE("received count: %d\n", received_count);
    
    BufferSlice read_buffer = buffer_as_slice(netbuffer, 0, received_count);
    netconn->read_command.do_read = !((OnReadFnStream) read_command.on_read_fn)(socket, read_buffer, read_command.arg);
}

void UDP_try_read(NetworkConnection* netconn, BufferSlice netbuffer) {
    JUST_LOG_TRACE("Read UDP\n");
    Socket socket = netconn->conn.socket;
    NetworkReadCommand read_command = netconn->read_command;

    SOCKADDR_IN sockaddr = {0};
    int sockaddr_size = sizeof(sockaddr);
    int32 received_count = recvfrom(socket, netbuffer.bytes, netbuffer.length, 0, (SOCKADDR*) &sockaddr, &sockaddr_size);
    if (received_count == SOCKET_ERROR) {
        int32 err = WSAGetLastError();
        switch (err) {
        case WSAEWOULDBLOCK:
            // Fine
            break;
        case WSAEMSGSIZE:
            // TODO: Partial Read
            break;
        default:
            // TODO: handle err
            JUST_LOG_WARN("read err: %d\n", err);
            break;
        }
    }
    JUST_LOG_TRACE("received count: %d\n", received_count);

    SocketAddr addr = {
        .host = inet_ntoa(sockaddr.sin_addr),
        .port = ntohs(sockaddr.sin_port),
    };

    BufferSlice datagram = buffer_as_slice(netbuffer, 0, received_count);
    netconn->read_command.do_read = !((OnReadFnDatagram) read_command.on_read_fn)(socket, addr, datagram, read_command.arg);
}



