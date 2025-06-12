#pragma once

#include <stdbool.h>

#include "core.h"
#include "thread/threadsync.h"

#define SOCKET_WRITE_QUEUE_CAPACITY 10

typedef uint64 SOCKET;

typedef enum {
    SOCKET_TYPE_STREAM = 1,
    SOCKET_TYPE_DATAGRAM = 2,
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

void init_network_thread();

SOCKET make_socket(SocketTypeEnum socket_type);
void bind_socket(SOCKET socket, SocketAddr addr);

void network_connect(SOCKET socket, SocketAddr addr, OnConnectFnStream on_connect, void* arg);
void network_start_read_stream(SOCKET socket, OnReadFnStream on_read, void* arg);
void network_write_stream(SOCKET socket, BufferSlice buffer, OnWriteFnStream on_write, void* arg);

void network_start_read_datagram(SOCKET socket, OnReadFnDatagram on_read, void* arg);
void network_write_datagram(SOCKET socket, SocketAddr addr, BufferSlice datagram, OnWriteFnDatagram on_write, void* arg);

uint16 just_htons(uint16 hostnum);
uint32 just_htonl(uint32 hostnum);
uint64 just_htonll(uint64 hostnum);

uint16 just_ntohs(uint16 netnum);
uint32 just_ntohl(uint32 netnum);
uint64 just_ntohll(uint64 netnum);