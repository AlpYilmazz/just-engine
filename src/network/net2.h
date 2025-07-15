#pragma once

#include "core.h"

#define SOCKET_WRITE_QUEUE_CAPACITY 10

#ifndef SOCKET_NO_DEFINE
typedef uint64 Socket;
#endif

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

typedef enum {
    NETWORK_PROTOCOL_TCP = 0,
    NETWORK_PROTOCOL_UDP,
    NETWORK_PROTOCOL_TLS,
    NETWORK_PROTOCOL_DTLS,
} NetworkProtocolEnum;

typedef enum {
    SERVER_STOP_ONLY = 0,
    SERVER_STOP_CLOSE_CONNECTIONS,
} ServerStopEnum;

typedef enum {
    CONNECTION_CLOSE_IMMEDIATE = 0,
    CONNECTION_CLOSE_GRACEFULL,
} ConnectionCloseEnum;

typedef struct {
    Socket socket;
    SocketAddr remote_addr;
} ReadContext;

typedef struct {
    Socket socket;
    SocketAddr remote_addr;
} WriteContext;

typedef void (*OnAcceptFn)(uint32 server_id, Socket socket, void* arg);
typedef void (*OnStopFn)(uint32 server_id, Socket server_socket, void* arg);

typedef void (*OnConnectFn)(uint32 connect_id, Socket socket, ConnectResult result, void* arg);
typedef void (*OnCloseFn)(Socket socket, void* arg);

typedef bool (*OnReadFn)(ReadContext context, BufferSlice read_buffer, void* arg); // -> do_continue
typedef void (*OnWriteFn)(WriteContext context, void* arg);

typedef struct {
    bool configure_server;
    bool configure_tls;
    // --
    char* server_cert_file;
    char* server_key_file;
} NetworkConfig;

void configure_network_system(NetworkConfig config);
void start_network_thread();

void network_start_server(SocketAddr bind_addr, NetworkProtocolEnum protocol, uint32 server_id, OnAcceptFn on_accept, void* arg);
void network_stop_server(uint32 server_id, ServerStopEnum stop_kind, OnStopFn on_stop, void* arg);

void network_connect(SocketAddr remote_addr, NetworkProtocolEnum protocol, uint32 connect_id, OnConnectFn on_connect, void* arg);
void network_close_connection(Socket socket, ConnectionCloseEnum close_kind, OnCloseFn on_close, void* arg);

void network_start_read(Socket socket, OnReadFn on_read, void* arg);
void network_write_buffer(Socket socket, BufferSlice buffer, OnWriteFn on_write, void* arg);
void network_write_buffer_to(Socket socket, SocketAddr remote_addr, BufferSlice buffer, OnWriteFn on_write, void* arg);

uint16 just_htons(uint16 hostnum);
uint32 just_htonl(uint32 hostnum);
uint64 just_htonll(uint64 hostnum);

uint16 just_ntohs(uint16 netnum);
uint32 just_ntohl(uint32 netnum);
uint64 just_ntohll(uint64 netnum);