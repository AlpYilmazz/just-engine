#include <windows.h>
#include <stdlib.h>
#include <process.h>
#include <stdatomic.h>
#include <stdbool.h>

#include "core.h"
#include "logging.h"
#include "thread/threadsync.h"

SRWLock* NETWORK_THREAD_LOCK;

atomic_bool interrupted = ATOMIC_VAR_INIT(1);
SOCKET interrupt_socket;

bool is_interrupted() {
    return atomic_load(&interrupted);
}

void set_interrupted(bool value) {
    atomic_store(&interrupted, value ? 1 : 0);
}

void init_interrupt() {
    if (is_interrupted()) {
        set_interrupted(false);

        interrupt_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (interrupt_socket == INVALID_SOCKET) {
            // TODO: handle err
            JUST_LOG_DEBUG("interrupt_socket INVALID_SOCKET\n");
            exit(1);
        }

        unsigned long non_blocking_mode = 1;
        int32 result = ioctlsocket(interrupt_socket, FIONBIO, &non_blocking_mode);
        if (result != NO_ERROR) {
            // TODO: handle err
            JUST_LOG_DEBUG("interrupt_socket setup failed\n");
            exit(1);
        }

        // int yes = 1;
        // int setsockopt_result = setsockopt(interrupt_socket, SOL_SOCKET, SO_REUSEADDR, (char*) &yes, sizeof(yes));
        // if (setsockopt_result == SOCKET_ERROR) {
        //     JUST_LOG_DEBUG("setsockopt error: %d\n", WSAGetLastError());
        //     exit(1);
        // }

        // SOCKADDR_IN sockaddr = {0};
        // sockaddr.sin_family = AF_INET;
        // sockaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        // sockaddr.sin_port = htons(0);

        // int bind_result = bind(interrupt_socket, (SOCKADDR*) &sockaddr, sizeof(sockaddr));
        // if (bind_result == SOCKET_ERROR) {
        //     JUST_LOG_DEBUG("bind error: %d\n", WSAGetLastError());
        //     exit(1);
        // }
    }
}

void interrupt_network_wait(uint64 _arg) {
    if (!is_interrupted()) {
        set_interrupted(true);
        closesocket(interrupt_socket);
        JUST_LOG_DEBUG("Interrupted\n");
    }
}

typedef void (*OnConnectFn)(SOCKET socket, void* arg);
typedef bool (*OnReadFn)(SOCKET socket, BufferSlice read_buffer, void* arg);
typedef void (*OnWriteFn)(SOCKET socket, void* arg);

typedef struct {
    SOCKET socket;
    OnConnectFn on_connect;
    void* arg;
    // --
    bool is_connected;
} ConnectSocket;

/**
 * ReadSocket handles read until removed using the .should_remove flag
 * return value of the .on_read function determines .should_remove 
 */
typedef struct {
    SOCKET socket;
    OnReadFn on_read;
    void* arg;
    // --
    bool should_remove;
} ReadSocket;

/**
 * WriteSocket handles write once
 * if you want to write again you should create again
 */
typedef struct {
    SOCKET socket;
    BufferSlice buffer;
    usize offset;
    // --
    OnWriteFn on_write;
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

void queue_add_connect_socket(SOCKET socket, OnConnectFn on_connect, void* arg) {
    QUEUE_CONNECT_SOCKETS.sockets[QUEUE_CONNECT_SOCKETS.length++] = (ConnectSocket) {
        .socket = socket,
        .on_connect = on_connect,
        .arg = arg,
        .is_connected = false,
    };
}
void queue_add_read_socket(SOCKET socket, OnReadFn on_read, void* arg) {
    QUEUE_READ_SOCKETS.sockets[QUEUE_READ_SOCKETS.length++] = (ReadSocket) {
        .socket = socket,
        .on_read = on_read,
        .arg = arg,
        .should_remove = false,
    };
}
void queue_add_write_socket(SOCKET socket, BufferSlice buffer, OnWriteFn on_write, void* arg) {
    QUEUE_WRITE_SOCKETS.sockets[QUEUE_WRITE_SOCKETS.length++] = (WriteSocket) {
        .socket = socket,
        .buffer = buffer,
        .offset = 0,
        .on_write = on_write,
        .arg = arg,
    };
}

void add_connect_socket(SOCKET socket, OnConnectFn on_connect, void* arg) {
    CONNECT_SOCKETS.sockets[CONNECT_SOCKETS.length++] = (ConnectSocket) {
        .socket = socket,
        .on_connect = on_connect,
        .arg = arg,
        .is_connected = false,
    };
}
void add_read_socket(SOCKET socket, OnReadFn on_read, void* arg) {
    READ_SOCKETS.sockets[READ_SOCKETS.length++] = (ReadSocket) {
        .socket = socket,
        .on_read = on_read,
        .arg = arg,
        .should_remove = false,
    };
}
void add_write_socket(SOCKET socket, BufferSlice buffer, OnWriteFn on_write, void* arg) {
    WRITE_SOCKETS.sockets[WRITE_SOCKETS.length++] = (WriteSocket) {
        .socket = socket,
        .buffer = buffer,
        .offset = 0,
        .on_write = on_write,
        .arg = arg,
    };
}

void handle_queued_sockets() {
    for (uint32 i = 0; i < QUEUE_CONNECT_SOCKETS.length; i++) {
        ConnectSocket* queued_socket = &QUEUE_CONNECT_SOCKETS.sockets[i];
        add_connect_socket(queued_socket->socket, queued_socket->on_connect, queued_socket->arg);
    }
    for (uint32 i = 0; i < QUEUE_READ_SOCKETS.length; i++) {
        ReadSocket* queued_socket = &QUEUE_READ_SOCKETS.sockets[i];
        add_read_socket(queued_socket->socket, queued_socket->on_read, queued_socket->arg);
    }
    for (uint32 i = 0; i < QUEUE_WRITE_SOCKETS.length; i++) {
        WriteSocket* queued_socket = &QUEUE_WRITE_SOCKETS.sockets[i];
        add_write_socket(queued_socket->socket, queued_socket->buffer, queued_socket->on_write, queued_socket->arg);
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
        if (connect_socket.is_connected) {
            // connection done
            if (connect_socket.on_connect != NULL) {
                connect_socket.on_connect(connect_socket.socket, connect_socket.arg);
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
            if (write_socket.on_write != NULL) {
                write_socket.on_write(write_socket.socket, write_socket.arg);
            }
            // swap remove
            WRITE_SOCKETS.sockets[i] = WRITE_SOCKETS.sockets[WRITE_SOCKETS.length];
            WRITE_SOCKETS.length--;
            i--;
        }
    }
}

void handle_read(fd_set* read_sockets, BufferSlice buffer) {
    JUST_LOG_DEBUG("handle_read: %d\n", read_sockets->fd_count);
    int32 received_count;
    for (uint32 i = 0; i < read_sockets->fd_count; i++) {
        SOCKET socket = read_sockets->fd_array[i];
        JUST_LOG_DEBUG("1\n");
        if (socket == interrupt_socket) continue;
        JUST_LOG_DEBUG("2\n");

        ReadSocket* read_socket = get_as_read_socket(socket);
        JUST_LOG_DEBUG("3\n");

        if (read_socket != NULL) {
            JUST_LOG_DEBUG("4\n");
            received_count = recv(socket, buffer.bytes, buffer.length, 0);
            if (received_count == SOCKET_ERROR) {
                int32 err = WSAGetLastError();
                if (err == WSAEWOULDBLOCK) {
                    continue;
                }
                // TODO: handle err
                JUST_LOG_DEBUG("read err: %d\n", err);
                continue;
            }
            JUST_LOG_DEBUG("received count: %d\n", received_count);
            
            BufferSlice read_buffer = buffer_as_slice(buffer, 0, received_count);
            JUST_LOG_DEBUG("---- buffer.bytes: %p\n", buffer.bytes);
            JUST_LOG_DEBUG("---- buffer.length: %d\n", buffer.length);
            JUST_LOG_DEBUG("---- read_buffer.bytes: %p\n", read_buffer.bytes);
            JUST_LOG_DEBUG("---- read_buffer.length: %d\n", read_buffer.length);
            bool should_remove = read_socket->on_read(socket, read_buffer, read_socket->arg);
            read_socket->should_remove = should_remove;
        }
    }
}

void handle_write(fd_set* write_sockets) {
    JUST_LOG_DEBUG("handle_write\n");
    int32 sent_count;
    for (uint32 i = 0; i < write_sockets->fd_count; i++) {
        SOCKET socket = write_sockets->fd_array[i];
        if (socket == interrupt_socket) continue;

        WriteSocket* write_socket = get_as_write_socket(socket);

        // Write
        if (write_socket != NULL) {
            sent_count = send(
                write_socket->socket,
                write_socket->buffer.bytes + write_socket->offset,
                write_socket->buffer.length - write_socket->offset,
                0
            );
            if (sent_count == SOCKET_ERROR) {
                int32 err = WSAGetLastError();
                if (err == WSAEWOULDBLOCK) {
                    continue;
                }
                // TODO: handle err
                continue;
            }
    
            write_socket->offset += sent_count;
        }
        // Connection
        else {
            ConnectSocket* connect_socket = get_as_connect_socket(socket);
            connect_socket->is_connected = true;
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
        // FD_SET(interrupt_socket, &write_sockets);
        FD_SET(interrupt_socket, &except_sockets);

        for (uint32 i = 0; i < READ_SOCKETS.length; i++) {
            FD_SET(READ_SOCKETS.sockets[i].socket, &read_sockets);
        }
        for (uint32 i = 0; i < WRITE_SOCKETS.length; i++) {
            FD_SET(WRITE_SOCKETS.sockets[i].socket, &write_sockets);
        }
        for (uint32 i = 0; i < CONNECT_SOCKETS.length; i++) {
            FD_SET(CONNECT_SOCKETS.sockets[i].socket, &write_sockets);
        }
        // fd_set_extend(&EXCEPT_SOCKETS, &except_sockets);
        
        srw_lock_release_shared(NETWORK_THREAD_LOCK);

        JUST_LOG_DEBUG("select started\n");
        int ready_count = select(
            0,
            &read_sockets,
            &write_sockets,
            &except_sockets,
            NULL
        );
        JUST_LOG_DEBUG("select ended\n");
    
        if (ready_count == SOCKET_ERROR) {
            int err = WSAGetLastError();
            JUST_LOG_DEBUG("select error: %d\n", err);
            continue;
        }

        srw_lock_acquire_exclusive(NETWORK_THREAD_LOCK);

        if (!is_interrupted()) {
            JUST_LOG_DEBUG("handling ready sockets\n");
            handle_read(&read_sockets, NETWORK_BUFFER);
            handle_write(&write_sockets);
            // handle_except(&except_sockets);
        }
        else {
            JUST_LOG_DEBUG("select was interrupted\n");
        }

        JUST_LOG_DEBUG("cleanup started\n");

        cleanup_connect_sockets();
        cleanup_read_sockets();
        cleanup_write_sockets();
        
        JUST_LOG_DEBUG("cleanup ended\n");

        handle_queued_sockets();

        srw_lock_release_exclusive(NETWORK_THREAD_LOCK);
    }

    #undef NETWORK_BUFFER_LEN
}

HANDLE NETWORK_THREAD;
uint32 NETWORK_THREAD_ID;

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

#include <stdio.h>

void init_network_thread() {
    //----------------------
    // Initialize Winsock
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR) {
        printf("WSAStartup function failed with error: %d\n", iResult);
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

typedef enum {
    SOCKET_TYPE_TCP = 0,
    SOCKET_TYPE_UDP,
} SocketTypeEnum;

SOCKET make_socket(SocketTypeEnum socket_type) {
    SOCKET new_socket;
    switch (socket_type) {
        case SOCKET_TYPE_TCP:
            new_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            break;
        case SOCKET_TYPE_UDP:
            new_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            break;
        default:
            // Unsupported
            JUST_LOG_DEBUG("Unsupported Socket Type\n");
            return INVALID_SOCKET;
    }

    if (new_socket == INVALID_SOCKET) {
        // TODO: handle err
        JUST_LOG_DEBUG("Socket creation failed\n");
        return INVALID_SOCKET;
    }

    unsigned long non_blocking_mode = 1;
    int32 result = ioctlsocket(new_socket, FIONBIO, &non_blocking_mode);
    if (result != NO_ERROR) {
        // TODO: handle err
        JUST_LOG_DEBUG("Socket setup failed\n");
        return INVALID_SOCKET;
    }

    return new_socket;
}

typedef struct {
    char* host; // string
    uint16 port;
} SocketAddr;

void network_connect(SOCKET socket, SocketAddr addr, OnConnectFn on_connect, void* arg) {
    SOCKADDR_IN sockaddr = {0};
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_addr.s_addr = inet_addr(addr.host);
    sockaddr.sin_port = htons(addr.port);

    int32 result = connect(socket, (SOCKADDR*) &sockaddr, sizeof(sockaddr));
    if (result == SOCKET_ERROR) {
        int32 err = WSAGetLastError();
        if (err == WSAEWOULDBLOCK) {
            // Good
            JUST_LOG_INFO("Connection started\n");
        }
        else {
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
        srw_lock_acquire_exclusive(NETWORK_THREAD_LOCK);

        add_connect_socket(socket, on_connect, arg);

        bool success = QueueUserAPC(
            interrupt_network_wait,
            NETWORK_THREAD,
            0
        );
        if (!success) {
            int32 err = GetLastError();
            // TODO: handle err
            JUST_LOG_DEBUG("Interrupt APC failed: %d\n", err);
        }

        srw_lock_release_exclusive(NETWORK_THREAD_LOCK);
    }
}

void network_start_read(SOCKET socket, OnReadFn on_read, void* arg) {
    uint32 current_thread_id = GetCurrentThreadId();
    if (current_thread_id == NETWORK_THREAD_ID) {
        // Called within network thread
        // probably from on_connect, on_read or on_write
        queue_add_read_socket(socket, on_read, arg);
    }
    else {
        // Called from another thread
        srw_lock_acquire_exclusive(NETWORK_THREAD_LOCK);
    
        add_read_socket(socket, on_read, arg);
    
        bool success = QueueUserAPC(
            interrupt_network_wait,
            NETWORK_THREAD,
            0
        );
        if (!success) {
            int32 err = GetLastError();
            // TODO: handle err
            JUST_LOG_DEBUG("Interrupt APC failed: %d\n", err);
        }
    
        srw_lock_release_exclusive(NETWORK_THREAD_LOCK);
    }
}

void network_write_buffer(SOCKET socket, BufferSlice buffer, OnWriteFn on_write, void* arg) {
    uint32 current_thread_id = GetCurrentThreadId();
    if (current_thread_id == NETWORK_THREAD_ID) {
        // Called within network thread
        // probably from on_connect, on_read or on_write
        queue_add_write_socket(socket, buffer, on_write, arg);
    }
    else {
        // Called from another thread
        srw_lock_acquire_exclusive(NETWORK_THREAD_LOCK);

        add_write_socket(socket, buffer, on_write, arg);

        bool success = QueueUserAPC(
            interrupt_network_wait,
            NETWORK_THREAD,
            0
        );
        if (!success) {
            int32 err = GetLastError();
            // TODO: handle err
            JUST_LOG_DEBUG("Interrupt APC failed: %d\n", err);
        }

        srw_lock_release_exclusive(NETWORK_THREAD_LOCK);
    }
}

/**
 * return 0 (false) for big endian, 1 (true) for little endian.
*/
bool check_system_endianness() {
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
        ? _byteswap_uint64(hostnum) // system little -> need swap
        : hostnum; // system big -> no swap
}

uint16 just_ntohs(uint16 netnum) {
    return ntohs(netnum);
}
uint32 just_ntohl(uint32 netnum) {
    return ntohl(netnum);
}
uint64 just_ntohll(uint64 netnum) {
    return check_system_endianness()
        ? _byteswap_uint64(netnum) // system little -> need swap
        : netnum; // system big -> no swap
}