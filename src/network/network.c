#include <winsock2.h>
#include <windows.h>
#include <stdlib.h>
#include <process.h>
#include <stdatomic.h>
#include <stdbool.h>

#include "core.h"
#include "logging.h"
#include "thread/threadsync.h"

HANDLE NETWORK_THREAD;
uint32 NETWORK_THREAD_ID;
SRWLock* NETWORK_THREAD_LOCK;

atomic_bool interrupted = ATOMIC_VAR_INIT(1);
SOCKET interrupt_socket;

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
            JUST_LOG_ERROR("interrupt_socket INVALID_SOCKET\n");
            exit(1);
        }

        unsigned long non_blocking_mode = 1;
        int32 result = ioctlsocket(interrupt_socket, FIONBIO, &non_blocking_mode);
        if (result != NO_ERROR) {
            // TODO: handle err
            JUST_LOG_ERROR("interrupt_socket setup failed\n");
            exit(1);
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
        JUST_LOG_WARN("Interrupt APC failed: %d\n", err);
    }
}

typedef enum {
    SOCKET_TYPE_STREAM = SOCK_STREAM,
    SOCKET_TYPE_DATAGRAM = SOCK_DGRAM,
} SocketTypeEnum;

typedef struct {
    char* host; // string
    uint16 port;
} SocketAddr;

typedef void (*OnConnectFnStream)(SOCKET socket, bool success, void* arg);
typedef bool (*OnReadFnStream)(SOCKET socket, BufferSlice read_buffer, void* arg);
typedef void (*OnWriteFnStream)(SOCKET socket, void* arg);
typedef bool (*OnReadFnDatagram)(SOCKET socket, SocketAddr addr, BufferSlice datagram, void* arg);
typedef void (*OnWriteFnDatagram)(SOCKET socket, SocketAddr addr, void* arg);

typedef struct {
    SOCKET socket;
    void* on_connect_fn;
    void* arg;
    // --
    bool connected;
    bool failed;
} ConnectSocket;

/**
 * ReadSocket handles read until removed using the .should_remove flag
 * return value of the .on_read function determines .should_remove 
 */
typedef struct {
    SocketTypeEnum type;
    SOCKET socket;
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
    SocketTypeEnum type;
    SOCKET socket;
    SocketAddr write_addr; // datagram
    BufferSlice buffer;
    usize offset;
    // --
    void* on_write_fn;
    void* arg;
} WriteSocket;

typedef struct {
    uint32 length;
    ConnectSocket sockets[FD_SETSIZE];
} ConnectSocketList;

typedef struct {
    uint32 length;
    ReadSocket sockets[FD_SETSIZE];
} ReadSocketList;

typedef struct {
    uint32 length;
    WriteSocket sockets[FD_SETSIZE];
} WriteSocketList;

ConnectSocketList CONNECT_SOCKETS = {0};
ReadSocketList READ_SOCKETS = {0};
WriteSocketList WRITE_SOCKETS = {0};
// fd_set EXCEPT_SOCKETS = {0};

ConnectSocketList QUEUE_CONNECT_SOCKETS = {0};
ReadSocketList QUEUE_READ_SOCKETS = {0};
WriteSocketList QUEUE_WRITE_SOCKETS = {0};

void queue_add_connect_socket(SOCKET socket, OnConnectFnStream on_connect, void* arg) {
    QUEUE_CONNECT_SOCKETS.sockets[QUEUE_CONNECT_SOCKETS.length++] = (ConnectSocket) {
        .socket = socket,
        .on_connect_fn = on_connect,
        .arg = arg,
        .connected = false,
        .failed = false,
    };
}
void queue_add_read_socket(SOCKET socket, OnReadFnStream on_read, void* arg) {
    QUEUE_READ_SOCKETS.sockets[QUEUE_READ_SOCKETS.length++] = (ReadSocket) {
        .type = SOCKET_TYPE_STREAM,
        .socket = socket,
        .on_read_fn = on_read,
        .arg = arg,
        .should_remove = false,
    };
}
void queue_add_write_socket(SOCKET socket, BufferSlice buffer, OnWriteFnStream on_write, void* arg) {
    QUEUE_WRITE_SOCKETS.sockets[QUEUE_WRITE_SOCKETS.length++] = (WriteSocket) {
        .type = SOCKET_TYPE_STREAM,
        .socket = socket,
        .write_addr = {0},
        .buffer = buffer,
        .offset = 0,
        .on_write_fn = on_write,
        .arg = arg,
    };
}
void queue_add_read_socket_datagram(SOCKET socket, OnReadFnDatagram on_read, void* arg) {
    QUEUE_READ_SOCKETS.sockets[QUEUE_READ_SOCKETS.length++] = (ReadSocket) {
        .type = SOCKET_TYPE_DATAGRAM,
        .socket = socket,
        .on_read_fn = on_read,
        .arg = arg,
        .should_remove = false,
    };
}
void queue_add_write_socket_datagram(SOCKET socket, SocketAddr addr, BufferSlice datagram, OnWriteFnDatagram on_write, void* arg) {
    QUEUE_WRITE_SOCKETS.sockets[QUEUE_WRITE_SOCKETS.length++] = (WriteSocket) {
        .type = SOCKET_TYPE_DATAGRAM,
        .socket = socket,
        .write_addr = addr,
        .buffer = datagram,
        .offset = 0,
        .on_write_fn = on_write,
        .arg = arg,
    };
}

void handle_queued_sockets() {
    for (uint32 i = 0; i < QUEUE_CONNECT_SOCKETS.length; i++) {
        CONNECT_SOCKETS.sockets[CONNECT_SOCKETS.length++] = QUEUE_CONNECT_SOCKETS.sockets[i];
    }
    for (uint32 i = 0; i < QUEUE_READ_SOCKETS.length; i++) {
        READ_SOCKETS.sockets[READ_SOCKETS.length++] = QUEUE_READ_SOCKETS.sockets[i];
    }
    for (uint32 i = 0; i < QUEUE_WRITE_SOCKETS.length; i++) {
        WRITE_SOCKETS.sockets[WRITE_SOCKETS.length++] = QUEUE_WRITE_SOCKETS.sockets[i];
    }
    QUEUE_CONNECT_SOCKETS.length = 0;
    QUEUE_READ_SOCKETS.length = 0;
    QUEUE_WRITE_SOCKETS.length = 0;
}

ConnectSocket* get_as_connect_socket(SOCKET socket) {
    for (uint32 i = 0; i < CONNECT_SOCKETS.length; i++) {
        ConnectSocket* connect_socket = &CONNECT_SOCKETS.sockets[i];
        if (socket == connect_socket->socket) {
            return connect_socket;
        }
    }
    return NULL;
}

ReadSocket* get_as_read_socket(SOCKET socket) {
    for (uint32 i = 0; i < READ_SOCKETS.length; i++) {
        ReadSocket* read_socket = &READ_SOCKETS.sockets[i];
        if (socket == read_socket->socket) {
            return read_socket;
        }
    }
    return NULL;
}

WriteSocket* get_as_write_socket(SOCKET socket) {
    for (uint32 i = 0; i < WRITE_SOCKETS.length; i++) {
        WriteSocket* write_socket = &WRITE_SOCKETS.sockets[i];
        if (socket == write_socket->socket) {
            return write_socket;
        }
    }
    return NULL;
}

void cleanup_connect_sockets() {
    for (uint32 i = 0; i < CONNECT_SOCKETS.length; i++) {
        ConnectSocket connect_socket = CONNECT_SOCKETS.sockets[i];
        if (connect_socket.connected || connect_socket.failed) {
            // connection done or failed
            if (connect_socket.on_connect_fn != NULL) {
                ((OnConnectFnStream) connect_socket.on_connect_fn)(connect_socket.socket, connect_socket.connected, connect_socket.arg);
            }
            // swap remove
            CONNECT_SOCKETS.sockets[i] = CONNECT_SOCKETS.sockets[CONNECT_SOCKETS.length];
            CONNECT_SOCKETS.length--;
            i--;
        }
    }
}

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
        if (write_socket.offset == write_socket.buffer.length) {
            // write complete
            if (write_socket.on_write_fn != NULL) {
                switch (write_socket.type) {
                case SOCKET_TYPE_STREAM:
                    ((OnWriteFnStream) write_socket.on_write_fn)(write_socket.socket, write_socket.arg);
                    break;
                
                case SOCKET_TYPE_DATAGRAM:
                    ((OnWriteFnDatagram) write_socket.on_write_fn)(write_socket.socket, write_socket.write_addr, write_socket.arg);
                    break;
                }
            }
            // swap remove
            WRITE_SOCKETS.sockets[i] = WRITE_SOCKETS.sockets[WRITE_SOCKETS.length];
            WRITE_SOCKETS.length--;
            i--;
        }
    }
}

void handle_read(fd_set* read_sockets, BufferSlice buffer) {
    JUST_LOG_TRACE("handle_read: %d\n", read_sockets->fd_count);
    int32 received_count;
    for (uint32 i = 0; i < read_sockets->fd_count; i++) {
        SOCKET socket = read_sockets->fd_array[i];
        if (socket == interrupt_socket) continue;

        ReadSocket* read_socket = get_as_read_socket(socket);

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
                JUST_LOG_DEBUG("received count: %d\n", received_count);
                
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
                JUST_LOG_DEBUG("received count: %d\n", received_count);

                SocketAddr addr = {
                    .host = inet_ntoa(sockaddr.sin_addr),
                    .port = ntohs(sockaddr.sin_port),
                };

                BufferSlice datagram = buffer_as_slice(buffer, 0, received_count);
                read_socket->should_remove = ((OnReadFnDatagram) read_socket->on_read_fn)(socket, addr, datagram, read_socket->arg);

                break;
            }
        }
    }
}

void handle_write(fd_set* write_sockets) {
    JUST_LOG_TRACE("handle_write\n");
    int32 sent_count;
    for (uint32 i = 0; i < write_sockets->fd_count; i++) {
        SOCKET socket = write_sockets->fd_array[i];
        if (socket == interrupt_socket) continue;

        WriteSocket* write_socket = get_as_write_socket(socket);

        // Write
        if (write_socket != NULL) {
            switch (write_socket->type) {
            case SOCKET_TYPE_STREAM:
                JUST_LOG_TRACE("Write Stream Socket\n");
                
                sent_count = send(
                    write_socket->socket,
                    write_socket->buffer.bytes + write_socket->offset,
                    write_socket->buffer.length - write_socket->offset,
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
        
                write_socket->offset += sent_count;

                break;
            
            case SOCKET_TYPE_DATAGRAM:
                JUST_LOG_TRACE("Write Datagram Socket\n");

                SOCKADDR_IN sockaddr = {0};
                sockaddr.sin_family = AF_INET;
                sockaddr.sin_addr.s_addr = inet_addr(write_socket->write_addr.host);
                sockaddr.sin_port = htons(write_socket->write_addr.port);

                sent_count = sendto(
                    socket,
                    write_socket->buffer.bytes + write_socket->offset,
                    write_socket->buffer.length - write_socket->offset,
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
                        write_socket->offset = write_socket->buffer.length;
                        continue;
                    default:
                        // TODO: handle err
                        JUST_LOG_WARN("write err: %d\n", err);
                        continue;
                    }
                }
                
                write_socket->offset += sent_count;

                break;
            }
        }
        // Connection
        else {
            ConnectSocket* connect_socket = get_as_connect_socket(socket);
            if (connect_socket != NULL) {
                connect_socket->connected = true;
            }
        }
    }
}

void handle_except(fd_set* except_sockets) {
    JUST_LOG_TRACE("handle_write\n");
    int32 sent_count;
    for (uint32 i = 0; i < except_sockets->fd_count; i++) {
        SOCKET socket = except_sockets->fd_array[i];
        if (socket == interrupt_socket) continue;

        ConnectSocket* connect_socket = get_as_connect_socket(socket);

        if (connect_socket != NULL) {
            // TODO: call connect once more to get error code
            connect_socket->failed = true;
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
        
        srw_lock_acquire_shared(NETWORK_THREAD_LOCK);

        init_interrupt();
        FD_SET(interrupt_socket, &read_sockets);

        for (uint32 i = 0; i < READ_SOCKETS.length; i++) {
            FD_SET(READ_SOCKETS.sockets[i].socket, &read_sockets);
        }
        for (uint32 i = 0; i < WRITE_SOCKETS.length; i++) {
            FD_SET(WRITE_SOCKETS.sockets[i].socket, &write_sockets);
        }
        for (uint32 i = 0; i < CONNECT_SOCKETS.length; i++) {
            FD_SET(CONNECT_SOCKETS.sockets[i].socket, &write_sockets); // success
            FD_SET(CONNECT_SOCKETS.sockets[i].socket, &except_sockets); // fail
        }
        
        srw_lock_release_shared(NETWORK_THREAD_LOCK);

        JUST_LOG_TRACE("START: select\n");

        int ready_count = select(
            0,
            &read_sockets,
            &write_sockets,
            &except_sockets,
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

        srw_lock_acquire_exclusive(NETWORK_THREAD_LOCK);

        JUST_LOG_TRACE("START: handle ready sockets\n");

        handle_read(&read_sockets, NETWORK_BUFFER);
        handle_write(&write_sockets);
        handle_except(&except_sockets);
        
        JUST_LOG_TRACE("START: cleanup\n");

        cleanup_connect_sockets();
        cleanup_read_sockets();
        cleanup_write_sockets();
        
        JUST_LOG_TRACE("START: handle socket queue\n");

        handle_queued_sockets();

        JUST_LOG_TRACE("END: ---\n");
        
        srw_lock_release_exclusive(NETWORK_THREAD_LOCK);
    }
}

// DWORD WINAPI thread_pool_worker_thread(LPVOID lpParam);
uint32 __stdcall network_thread_main(void* thread_param) {
    spin_network_worker();

    // Cleanup
    free_srw_lock(NETWORK_THREAD_LOCK);

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
        JUST_LOG_ERROR("WSAStartup function failed with error: %d\n", result);
        exit(1);
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

SOCKET make_socket(SocketTypeEnum socket_type) {
    SOCKET sock;
    switch (socket_type) {
        case SOCKET_TYPE_STREAM:
            sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            break;
        case SOCKET_TYPE_DATAGRAM:
            sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            break;
        default:
            // Unsupported
            JUST_LOG_WARN("Unsupported Socket Type\n");
            return INVALID_SOCKET;
    }

    if (sock == INVALID_SOCKET) {
        // TODO: handle err
        JUST_LOG_WARN("Socket creation failed\n");
        return INVALID_SOCKET;
    }

    unsigned long non_blocking_mode = 1;
    int32 result = ioctlsocket(sock, FIONBIO, &non_blocking_mode);
    if (result != NO_ERROR) {
        // TODO: handle err
        JUST_LOG_WARN("Socket setup failed\n");
        closesocket(sock);
        return INVALID_SOCKET;
    }

    return sock;
}

void network_connect(SOCKET socket, SocketAddr addr, OnConnectFnStream on_connect, void* arg) {
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
        default:
            // TODO: handle err
            JUST_LOG_DEBUG("Connection failed: %d\n", err);
        }
    }

    uint32 current_thread_id = GetCurrentThreadId();
    if (current_thread_id == NETWORK_THREAD_ID) {
        // Called within network thread
        // probably from on_connect, on_read or on_write
        queue_add_connect_socket(socket, on_connect, arg);
    }
    else {
        // Called from another thread
        SRW_LOCK_EXCLUSIVE_ZONE(NETWORK_THREAD_LOCK, {
            queue_add_connect_socket(socket, on_connect, arg);
        });
        issue_interrupt_apc();
    }
}

void network_start_read_stream(SOCKET socket, OnReadFnStream on_read, void* arg) {
    uint32 current_thread_id = GetCurrentThreadId();
    if (current_thread_id == NETWORK_THREAD_ID) {
        // Called within network thread
        // probably from on_connect, on_read or on_write
        queue_add_read_socket(socket, on_read, arg);
    }
    else {
        // Called from another thread
        SRW_LOCK_EXCLUSIVE_ZONE(NETWORK_THREAD_LOCK, {
            queue_add_read_socket(socket, on_read, arg);
        });
        issue_interrupt_apc();
    }
}

void network_write_stream(SOCKET socket, BufferSlice buffer, OnWriteFnStream on_write, void* arg) {
    uint32 current_thread_id = GetCurrentThreadId();
    if (current_thread_id == NETWORK_THREAD_ID) {
        // Called within network thread
        // probably from on_connect, on_read or on_write
        queue_add_write_socket(socket, buffer, on_write, arg);
    }
    else {
        // Called from another thread
        SRW_LOCK_EXCLUSIVE_ZONE(NETWORK_THREAD_LOCK, {
            queue_add_write_socket(socket, buffer, on_write, arg);
        });
        issue_interrupt_apc();
    }
}

void network_start_read_datagram(SOCKET socket, OnReadFnDatagram on_read, void* arg) {
    uint32 current_thread_id = GetCurrentThreadId();
    if (current_thread_id == NETWORK_THREAD_ID) {
        // Called within network thread
        // probably from on_connect, on_read or on_write
        queue_add_read_socket_datagram(socket, on_read, arg);
    }
    else {
        // Called from another thread
        SRW_LOCK_EXCLUSIVE_ZONE(NETWORK_THREAD_LOCK, {
            queue_add_read_socket_datagram(socket, on_read, arg);
        });
        issue_interrupt_apc();
    }
}

void network_write_datagram(SOCKET socket, SocketAddr addr, BufferSlice datagram, OnWriteFnDatagram on_write, void* arg) {
    uint32 current_thread_id = GetCurrentThreadId();
    if (current_thread_id == NETWORK_THREAD_ID) {
        // Called within network thread
        // probably from on_connect, on_read or on_write
        queue_add_write_socket_datagram(socket, addr, datagram, on_write, arg);
    }
    else {
        // Called from another thread
        SRW_LOCK_EXCLUSIVE_ZONE(NETWORK_THREAD_LOCK, {
            queue_add_write_socket_datagram(socket, addr, datagram, on_write, arg);
        });
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