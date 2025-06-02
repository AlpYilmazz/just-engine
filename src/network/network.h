#pragma once

#include <stdbool.h>

#include "core.h"
#include "thread/threadsync.h"

typedef uint64 SOCKET;

typedef void (*OnConnectFn)(SOCKET socket, void* arg);
typedef bool (*OnReadFn)(SOCKET socket, BufferSlice read_buffer, void* arg);
typedef void (*OnWriteFn)(SOCKET socket, void* arg);

void init_network_thread();

typedef enum {
    SOCKET_TYPE_TCP = 0,
    SOCKET_TYPE_UDP,
} SocketTypeEnum;

SOCKET make_socket(SocketTypeEnum socket_type);

typedef struct {
    char* host; // string
    uint16 port;
} SocketAddr;

void network_connect(SOCKET socket, SocketAddr addr, OnConnectFn on_connect, void* arg);
void network_start_read(SOCKET socket, OnReadFn on_read, void* arg);
void network_write_buffer(SOCKET socket, BufferSlice buffer, OnWriteFn on_write, void* arg);

uint16 just_htons(uint16 hostnum);
uint32 just_htonl(uint32 hostnum);
uint64 just_htonll(uint64 hostnum);

uint16 just_ntohs(uint16 netnum);
uint32 just_ntohl(uint32 netnum);
uint64 just_ntohll(uint64 netnum);