#pragma once

#include <stdbool.h>

#include "core.h"
#include "thread/threadsync.h"

#define SOCKET_WRITE_QUEUE_CAPACITY 10

typedef uint64 Socket;

typedef enum {
    SOCKET_TYPE_STREAM = 1,
    SOCKET_TYPE_DATAGRAM = 2,
} SocketTypeEnum;

typedef enum {
    CONNECT_SUCCESS = 0,
    CONNECT_FAIL_GENERAL,
} ConnectResult;

typedef struct {
    char* host; // string
    uint16 port;
} SocketAddr;

typedef void (*OnConnectFn)(Socket socket, ConnectResult result, void* arg);
typedef bool (*OnReadFnStream)(Socket socket, BufferSlice read_buffer, void* arg);
typedef void (*OnWriteFnStream)(Socket socket, void* arg);

typedef bool (*OnReadFnDatagram)(Socket socket, SocketAddr addr, BufferSlice datagram, void* arg);
typedef void (*OnWriteFnDatagram)(Socket socket, SocketAddr addr, void* arg);

void init_network_thread();

Socket make_socket(SocketTypeEnum socket_type);
void bind_socket(Socket socket, SocketAddr addr);

void network_connect(Socket socket, SocketAddr addr, OnConnectFn on_connect, void* arg);

void network_start_read_stream(Socket socket, OnReadFnStream on_read, void* arg);
void network_write_stream(Socket socket, BufferSlice buffer, OnWriteFnStream on_write, void* arg);

void network_start_read_datagram(Socket socket, OnReadFnDatagram on_read, void* arg);
void network_write_datagram(Socket socket, SocketAddr addr, BufferSlice datagram, OnWriteFnDatagram on_write, void* arg);

uint16 just_htons(uint16 hostnum);
uint32 just_htonl(uint32 hostnum);
uint64 just_htonll(uint64 hostnum);

uint16 just_ntohs(uint16 netnum);
uint32 just_ntohl(uint32 netnum);
uint64 just_ntohll(uint64 netnum);