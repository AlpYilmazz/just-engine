#pragma once

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#define __HEADER_LOGGING
#ifdef __HEADER_LOGGING

typedef enum {
    LOG_LEVEL_ALL = 0,
    LOG_LEVEL_TRACE,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_NONE,
} LogLevel;

void SET_LOG_LEVEL(LogLevel log_level);

void JUST_LOG_TRACE(const char* format, ...);
void JUST_LOG_DEBUG(const char* format, ...);
void JUST_LOG_INFO(const char* format, ...);
void JUST_LOG_WARN(const char* format, ...);
void JUST_LOG_ERROR(const char* format, ...);
void JUST_LOG_PANIC(const char* format, ...);

#define JUST_DEV_MARK() printf("-- [%s] [%d] --\n", __FILE__, __LINE__)

#endif // __HEAEDER_LOGGING

#define __HEADER_CORE
#ifdef __HEADER_CORE

typedef     unsigned char           uint8;
typedef     unsigned short          uint16;
typedef     unsigned int            uint32;
typedef     unsigned long long      uint64;

typedef     char                    int8;
typedef     short                   int16;
typedef     int                     int32;
typedef     long long               int64;

typedef     float                   float32;
typedef     double                  float64;

typedef     uint64                  usize;
typedef     unsigned char           byte;
// typedef     uint8                   bool;

#define branchless_if(cond, on_true, on_false) ( ( ((cond) != 0) * (on_true) ) + ( ((cond) == 0) * (on_false) ) )

#define Option(Type) DeclType_Option_##Type
#define Option_None {0}
#define Option_Some(val) { .is_some = true, .value = val, }
#define DECLARE__Option(Type) typedef struct { bool is_some; Type value; } Option(Type);

DECLARE__Option(uint8);
DECLARE__Option(uint16);
DECLARE__Option(uint32);
DECLARE__Option(uint64);
DECLARE__Option(int8);
DECLARE__Option(int16);
DECLARE__Option(int32);
DECLARE__Option(int64);
DECLARE__Option(float32);
DECLARE__Option(float64);
DECLARE__Option(usize);
DECLARE__Option(byte);
DECLARE__Option(char);

#define STRUCT_ZERO_INIT {0}
#define LAZY_INIT {0}
#define LATER_INIT {0}
#define UNINIT {0}

#define MAX(a, b) ((a >= b) ? a : b)
#define MIN(a, b) ((a <= b) ? a : b)

#define SIGNOF(x) ( (x == 0) ? 0 : ( (x > 0) ? 1 : -1 ) )

#define PANIC(...) { JUST_LOG_PANIC(__VA_ARGS__); exit(EXIT_FAILURE); }
#define UNREACHABLE() { JUST_LOG_PANIC("UNREACHABLE: [%s] [%d]\n", __FILE__, __LINE__); exit(EXIT_FAILURE); }

typedef struct {
    uint32 id;
    uint32 generation;
} EntityId;

static inline EntityId new_entity_id(uint32 id, uint32 generation) {
    return (EntityId) { id, generation };
}

// Memory is owned
typedef struct {
    usize length;
    byte* bytes;
} Buffer;

// Memory is only viewed
typedef Buffer BufferSlice;

typedef struct {
    usize length;
    byte* cursor;
    byte* bytes;
} FillBuffer;

static inline BufferSlice buffer_as_slice(Buffer buffer, usize start, usize length) {
    return (BufferSlice) {
        .bytes = buffer.bytes + start,
        .length = length,
    };
}

static inline BufferSlice buffer_into_slice(Buffer buffer) {
    return buffer_as_slice(buffer, 0, buffer.length);
}

static inline Buffer buffer_clone(Buffer buffer) {
    byte* bytes_clone = malloc(buffer.length);
    memcpy(bytes_clone, buffer.bytes, buffer.length);
    return (Buffer) {
        .length = buffer.length,
        .bytes = bytes_clone,
    };
}

static inline usize filled_length(FillBuffer* buffer) {
    return buffer->cursor - buffer->bytes;
}

#endif // __HEADER_CORE

#define __HEADER_BASE
#ifdef __HEADER_BASE

static const uint32 ALL_SET_32 = 0b11111111111111111111111111111111;

#define RECTANGLE_NICHE ( \
    (Rectangle) {   \
        .x = *(float32*) &ALL_SET_32, \
        .y = *(float32*) &ALL_SET_32, \
        .width = *(float32*) &ALL_SET_32, \
        .height = *(float32*) &ALL_SET_32, \
    }   \
)

#define BYTEWISE_EQUALS(e1, e2, type)   \
    ( bytewise_equals((byte*)e1, (byte*)e2, sizeof(type)) )

static inline bool bytewise_equals(byte* e1, byte* e2, uint32 count) {
    bool is_eq = 1;
    for (uint32 i = 0; i < count; i++) {
        is_eq &= (e1[i] == e2[i]);
    }
    return is_eq;
}

typedef enum {
    Anchor_Top_Left = 0,    // DEFAULT
    Anchor_Top_Right,
    Anchor_Bottom_Left,
    Anchor_Bottom_Right,

    Anchor_Top_Mid,
    Anchor_Bottom_Mid,
    Anchor_Left_Mid,
    Anchor_Right_Mid,

    Anchor_Center,
} AnchorType;

typedef struct {
    Vector2 origin;         // uv coordinate: [0.0, 1.0]
} Anchor;                   // {0} -> origin = {0, 0} -> Anchor_Top_Left

static inline Anchor make_anchor(AnchorType type) {
    switch (type) {
    default:
    case Anchor_Top_Left:
        return (Anchor) { .origin = {0, 0} };
    case Anchor_Top_Right:
        return (Anchor) { .origin = {1, 0} };
    case Anchor_Bottom_Left:
        return (Anchor) { .origin = {0, 1} };
    case Anchor_Bottom_Right:
        return (Anchor) { .origin = {1, 1} };

    case Anchor_Top_Mid:
        return (Anchor) { .origin = {0.5, 0} };
    case Anchor_Bottom_Mid:
        return (Anchor) { .origin = {0.5, 1} };
    case Anchor_Left_Mid:
        return (Anchor) { .origin = {0, 0.5} };
    case Anchor_Right_Mid:
        return (Anchor) { .origin = {1, 0.5} };

    case Anchor_Center:
        return (Anchor) { .origin = {0.5, 0.5} };
    }
    PANIC("Incorrect AnchorType!\n");
}

static inline Anchor make_custom_anchor(Vector2 origin) {
    return (Anchor) { .origin = origin };
}

typedef struct {
    Vector2 translation;
    Vector2 scale;
} SpaceShift;

typedef struct {
    union {
        struct {
            float32 width;
            float32 height;
        };
        Vector2 as_vec;
    };
} RectSize;

typedef struct {
    union {
        struct {
            uint32 width;
            uint32 height;
        };
        struct {
            uint32 x;
            uint32 y;
        };
    };
} URectSize;

static inline Rectangle into_rectangle(Vector2 position, RectSize size) {
    return (Rectangle) {
        .x = position.x,
        .y = position.y,
        .width = size.width,
        .height = size.height,
    };
}

static inline Vector2 find_rectangle_top_left(Anchor anchor, Vector2 position, RectSize size) {
    return Vector2Subtract(
        position,
        Vector2Multiply(anchor.origin, size.as_vec)
    );
}

static inline Vector2 find_rectangle_top_left_rect(Anchor anchor, Rectangle rect) {
    Vector2 position = {rect.x, rect.y};
    Vector2 size = {rect.width, rect.height};
    return Vector2Subtract(
        position,
        Vector2Multiply(anchor.origin, size)
    );
}

static inline Vector2 find_rectangle_position(Anchor anchor, Vector2 top_left, RectSize size) {
    return Vector2Add(
        top_left,
        Vector2Multiply(anchor.origin, size.as_vec)
    );
}

static inline Vector2 find_rectangle_position_rect(Anchor anchor, Rectangle rect) {
    Vector2 top_left = {rect.x, rect.y};
    Vector2 size = {rect.width, rect.height};
    return Vector2Add(
        top_left,
        Vector2Multiply(anchor.origin, size)
    );
}

typedef uint32 BitMask32;

static inline BitMask32 set_bit(BitMask32 mask, uint32 bit) {
    return mask | (1 << bit);
}

static inline BitMask32 unset_bit(BitMask32 mask, uint32 bit) {
    return mask & ((1 << bit) ^ 0xFFFFFFFF);
}

static inline bool check_bit(BitMask32 mask, uint32 bit) {
    return mask & (1 << bit);
}

static inline bool check_overlap(BitMask32 mask1, BitMask32 mask2) {
    return mask1 & mask2;
}

#define PRIMARY_LAYER 1

typedef struct {
    BitMask32 mask;
} Layers;

static inline void set_layer(Layers* layers, uint32 layer) {
    layers->mask = set_bit(layers->mask, layer);
}

static inline void unset_layer(Layers* layers, uint32 layer) {
    layers->mask = unset_bit(layers->mask, layer);
}

static inline bool check_layer(Layers layers, uint32 layer) {
    return check_bit(layers.mask, layer);
}

static inline bool check_layer_overlap(Layers ls1, Layers ls2) {
    return check_overlap(ls1.mask, ls2.mask);
}

static inline Layers on_single_layer(uint32 layer) {
    Layers layers = {0};
    set_layer(&layers, layer);
    return layers;
}

static inline Layers on_primary_layer() {
    return on_single_layer(PRIMARY_LAYER);
}

#define Vector2_From(val) ((Vector2) {val, val})
#define Vector2_Ones ((Vector2) {1.0, 1.0})
#define Vector2_Unit_X ((Vector2) {1.0, 0.0})
#define Vector2_Unit_Y ((Vector2) {0.0, 1.0})
#define Vector2_Neg_Unit_X ((Vector2) {-1.0, 0.0})
#define Vector2_Neg_Unit_Y ((Vector2) {0.0, -1.0})
#define Vector2_On_X(val) ((Vector2) {val, 0.0})
#define Vector2_On_Y(val) ((Vector2) {0.0, val})
#define Vector2_XX(vec) ((Vector2) {vec.x, vec.x})
#define Vector2_YY(vec) ((Vector2) {vec.y, vec.y})
#define Vector2_YX(vec) ((Vector2) {vec.y, vec.x})

static inline Vector2 vector2_from(float32 val) {
    return (Vector2) {val, val};
}

static inline Vector2 vector2_ones() {
    return (Vector2) {1.0, 1.0};
}

static inline Vector2 vector2_unit_x() {
    return (Vector2) {1.0, 0.0};
}

static inline Vector2 vector2_unit_y() {
    return (Vector2) {0.0, 1.0};
}

static inline Vector2 vector2_neg_unit_x() {
    return (Vector2) {-1.0, 0.0};
}

static inline Vector2 vector2_neg_unit_y() {
    return (Vector2) {0.0, -1.0};
}

static inline Vector2 vector2_on_x(float32 val) {
    return (Vector2) {val, 0.0};
}

static inline Vector2 vector2_on_y(float32 val) {
    return (Vector2) {0.0, val};
}

static inline Vector2 vector2_xx(Vector2 vec) {
    return (Vector2) {vec.x, vec.x};
}

static inline Vector2 vector2_yy(Vector2 vec) {
    return (Vector2) {vec.y, vec.y};
}

static inline Vector2 vector2_yx(Vector2 vec) {
    return (Vector2) {vec.y, vec.x};
}

#endif // __HEADER_BASE

#define __HEADER_MEMORY_MEMORY
#ifdef __HEADER_MEMORY_MEMORY

#define dynarray_clear(arr) \
    do { \
        (arr).count = 0; \
    } while(0)

#define dynarray_free(arr) \
    do { \
        if ((arr).capacity > 0) { \
            (arr).count = 0; \
            free((arr).items); \
        } \
    } while(0)

#define dynarray_free_custom(arr, items_field) \
    do { \
        if ((arr).capacity > 0) { \
            (arr).count = 0; \
            free((arr)items_field); \
        } \
    } while(0)

#define dynarray_push_back(arr, item) \
    do { \
        const uint32 DYNARRAY_INITIAL_CAPACITY = 2; \
        const uint32 DYNARRAY_GROWTH_FACTOR = 2; \
\
        if ((arr).capacity == 0) { \
            (arr).capacity = DYNARRAY_INITIAL_CAPACITY; \
            (arr).items = malloc((arr).capacity * sizeof((item))); \
        } \
        else if ((arr).count == (arr).capacity) { \
            (arr).capacity = DYNARRAY_GROWTH_FACTOR * (arr).capacity; \
            (arr).items = realloc((arr).items, (arr).capacity * sizeof((item))); \
        } \
        \
        (arr).items[(arr).count] = (item); \
        (arr).count++; \
    } while(0)

#define dynarray_push_back_custom(arr, items_field, item) \
    do { \
        const uint32 DYNARRAY_INITIAL_CAPACITY = 2; \
        const uint32 DYNARRAY_GROWTH_FACTOR = 2; \
\
        if ((arr).capacity == 0) { \
            (arr).capacity = DYNARRAY_INITIAL_CAPACITY; \
            (arr)items_field = malloc((arr).capacity * sizeof((item))); \
        } \
        else if ((arr).count == (arr).capacity) { \
            (arr).capacity = DYNARRAY_GROWTH_FACTOR * (arr).capacity; \
            (arr)items_field = realloc((arr)items_field, (arr).capacity * sizeof((item))); \
        } \
        \
        (arr)items_field[(arr).count] = (item); \
        (arr).count++; \
    } while(0)

#define dynarray_clone(dst_arr, src_arr) \
    do { \
        usize size = sizeof(*(dst_arr).items) * (dst_arr).count; \
        (src_arr).count = (dst_arr).count; \
        (src_arr).capacity = (dst_arr).count; \
        (src_arr).items = (dst_arr).items; \
        (src_arr).items = malloc(size); \
        memcpy((src_arr).items, (dst_arr).items, size); \
    } while(0)

#define dynarray_clone_custom(dst_arr, src_arr, items_field) \
    do { \
        usize size = sizeof(*(dst_arr)items_field) * (dst_arr).count; \
        (src_arr).count = (dst_arr).count; \
        (src_arr).capacity = (dst_arr).count; \
        (src_arr)items_field = (dst_arr)items_field; \
        (src_arr)items_field = malloc(size); \
        memcpy((src_arr)items_field, (dst_arr)items_field, size); \
    } while(0)

static inline Buffer* malloc_buffer(usize size) {
    Buffer* buffer = malloc(sizeof(Buffer) + size); // Buffer + [bytes]
    buffer->length = size;
    buffer->bytes = (byte*) (buffer + 1);
    return buffer;
}

static inline FillBuffer* malloc_fillbuffer(usize size) {
    FillBuffer* fillbuffer = malloc(sizeof(FillBuffer) + size); // FillBuffer + [bytes]
    fillbuffer->length = size;
    fillbuffer->bytes = (byte*) (fillbuffer + 1);
    fillbuffer->cursor = fillbuffer->bytes;
    return fillbuffer;
}

// -- Allocation --
typedef struct {
    usize size;
    usize alignment;
} MemoryLayout;

#define layoutof(TYPE) ((MemoryLayout) { .size = sizeof(TYPE), .alignment = _Alignof(TYPE)})

usize addr_align_up(usize addr, usize align);
void* ptr_align_up(void* ptr, usize align);

#define BUMP_ALLOCATOR_DEFAULT_SIZE (10 * 1024)

typedef struct {
    byte* base;
    byte* cursor;
    usize total_size;
} BumpAllocator;

typedef BumpAllocator TemporaryStorage;

BumpAllocator make_bump_allocator_with_size(usize size);
BumpAllocator make_bump_allocator();
void free_bump_allocator(BumpAllocator* bump_allocator);
void reset_bump_allocator(BumpAllocator* bump_allocator);
void* bump_alloc(BumpAllocator* bump_allocator, usize size);
void* bump_alloc_aligned(BumpAllocator* bump_allocator, MemoryLayout layout);

#define ARENA_ALLOCATOR_DEFAULT_REGION_SIZE (10 * 1024)

typedef struct ArenaRegion {
    struct ArenaRegion* next_region;
    usize total_size;
    usize free_size;
    byte* cursor;
    byte base[];
} ArenaRegion;

typedef struct {
    usize region_size;
    ArenaRegion* head_region;
} ArenaAllocator;

ArenaAllocator make_arena_allocator_with_region_size(usize region_size);
ArenaAllocator make_arena_allocator();
void free_arena_allocator(ArenaAllocator* arena_allocator);
void reset_arena_allocator(ArenaAllocator* arena_allocator);
void* arena_alloc(ArenaAllocator* arena_allocator, usize size);
void* arena_alloc_aligned(ArenaAllocator* arena_allocator, MemoryLayout layout);

#endif // __HEADER_MEMORY_MEMORY

#define __HEADER_MEMORY_JUSTSTRING
#ifdef __HEADER_MEMORY_JUSTSTRING

typedef struct {
    usize count;
    usize capacity;
    char* str;
} String;

typedef struct {
    usize count;
    char* str;
    char temp_hold;
} StringView;

typedef struct {
    String first;
    String second;
} StringPair;

typedef struct {
    StringView first;
    StringView second;
} StringViewPair;

typedef struct StringBuilderNode {
    struct StringBuilderNode* next;
    bool auto_free;
    usize count;
    char* str;
} StringBuilderNode;

typedef struct {
    usize total_count;
    StringBuilderNode* head;
    StringBuilderNode* tail;
} StringBuilder;

usize cstr_length(char* cstr);
char* cstr_nclone(char* cstr, usize count);

char* string_as_cstr(String string);
char* string_view_as_cstr_acquire(StringView* string_view);
void string_view_as_cstr_release(StringView* string_view);

String string_new();
String string_from_cstr(char* cstr);
void free_string(String string);

void string_append_cstr(String* string_dst, char* cstr_src);
StringView string_view(String string, usize start, usize count);
StringViewPair string_split_at(String string, usize index);

StringBuilder string_builder_new();
void string_builder_append(StringBuilder* builder, char* cstr);
void string_builder_append_free(StringBuilder* builder, char* cstr);
String build_string(StringBuilder* builder);

#endif // __HEADER_MEMORY_JUSTSTRING

#define __HEADER_THREAD_TASK
#ifdef __HEADER_THREAD_TASK

typedef void TaskArgVoid;

typedef struct {
    unsigned int (*handler)(TaskArgVoid*);
    TaskArgVoid* arg;
} ThreadEntry;

typedef struct {
    void (*handler)(TaskArgVoid*);
    TaskArgVoid* arg;
} Task;

#endif // __HEADER_THREAD_TASK

#define __HEADER_THREAD_THREAD
#ifdef __HEADER_THREAD_THREAD

typedef struct {
    unsigned int id;
    uintptr_t handle;
} Thread;

Thread thread_spawn(ThreadEntry entry);
void end_thread(unsigned int return_code);
void thread_join(Thread thread);

#endif // __HEADER_THREAD_THREAD

#define __HEADER_THREAD_THREADSYNC
#ifdef __HEADER_THREAD_THREADSYNC

// Sync objects are opeque
// because windows.h and raylib.h
// has name collisions

typedef void SRWLock;

SRWLock* alloc_create_srw_lock();
void free_srw_lock(SRWLock* lock);
void srw_lock_acquire_exclusive(SRWLock* lock);
void srw_lock_acquire_shared(SRWLock* lock);
void srw_lock_release_exclusive(SRWLock* lock);
void srw_lock_release_shared(SRWLock* lock);

#define SRW_LOCK_EXCLUSIVE_ZONE(SRW_LOCK, CodeBlock) \
    do {\
    srw_lock_acquire_exclusive(SRW_LOCK);\
        do { CodeBlock; } while(0);\
    srw_lock_release_exclusive(SRW_LOCK);\
    } while (0)

#define SRW_LOCK_SHARED_ZONE(SRW_LOCK, CodeBlock) \
    do {\
    srw_lock_acquire_shared(SRW_LOCK);\
        do { CodeBlock; } while(0);\
    srw_lock_release_shared(SRW_LOCK);\
    } while (0)

#endif // __HEADER_THREAD_THREADSYNC

#define __HEADER_THREAD_THREADPOOL
#ifdef __HEADER_THREAD_THREADPOOL

typedef enum {
    // NO_SHUTDOWN = 0
    THREADPOOL_IMMEDIATE_SHUTDOWN = 1,
    THREADPOOL_GRACEFULL_SHUTDOWN = 2,
} ThreadPoolShutdown;

// ThreadPool is opeque
// because windows.h and raylib.h
// has name collisions
// and ThreadPool includes windows structs
typedef void ThreadPool;

ThreadPool* thread_pool_create(unsigned int n_threads, unsigned int task_queue_capacity);
bool thread_pool_add_task(ThreadPool* pool, Task task);
void thread_pool_shutdown(ThreadPool* pool, ThreadPoolShutdown shutdown);

void example_async_task_print_int_arg(TaskArgVoid* arg);

#endif // __HEADER_THREAD_THREADPOOL

#define __HEADER_NETWORK_NETWORK
#ifdef __HEADER_NETWORK_NETWORK

#define SOCKET_WRITE_QUEUE_CAPACITY 10

typedef uint64 Socket;

typedef struct {
    char* bind;
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

#endif // __HEADER_NETWORK_NETWORK

#define __HEADER_ASSET_ASSET
#ifdef __HEADER_ASSET_ASSET

#define TEXTURE_SLOTS 100
#define DEFAULT_TEXTURE_HANDLE_ID   0
#define BLANK_TEXTURE_HANDLE_ID     1
#define WHITE_TEXTURE_HANDLE_ID     2

typedef struct {
    uint32 id;
} TextureHandle;

TextureHandle new_texture_handle(uint32 id);

typedef struct {
    bool exists;
    Image* texture;
} ImageResponse;

typedef struct {
    bool exists;
    Texture* texture;
} TextureResponse;

typedef struct {
    uint32 next_slot_available_bump;
    bool slots[TEXTURE_SLOTS];
    bool image_ready[TEXTURE_SLOTS];
    bool texture_ready[TEXTURE_SLOTS];
    bool image_changed[TEXTURE_SLOTS];
    Image images[TEXTURE_SLOTS];
    Texture textures[TEXTURE_SLOTS];
} TextureAssets;

TextureAssets new_texture_assets();

TextureHandle texture_assets_reserve_texture_slot(TextureAssets* assets);

void texture_assets_put_image(TextureAssets* assets, TextureHandle handle, Image image);
void texture_assets_load_image_unchecked(TextureAssets* assets, TextureHandle handle);
void texture_assets_load_texture_uncheched(TextureAssets* assets, TextureHandle handle);
void texture_assets_update_texture_unchecked(TextureAssets* assets, TextureHandle handle);
void texture_assets_update_texture_rec_unchecked(TextureAssets* assets, TextureHandle handle, Rectangle rec);
void texture_assets_put_image_and_load_texture(TextureAssets* assets, TextureHandle handle, Image image);
void texture_assets_load_texture_then_unload_image(TextureAssets* assets, TextureHandle handle, Image image);

ImageResponse texture_assets_get_image(TextureAssets* assets, TextureHandle handle);
Image* texture_assets_get_image_or_default(TextureAssets* assets, TextureHandle handle);
Image* texture_assets_get_image_unchecked(TextureAssets* assets, TextureHandle handle);
ImageResponse texture_assets_get_image_mut(TextureAssets* assets, TextureHandle handle);
Image* texture_assets_get_image_unchecked_mut(TextureAssets* assets, TextureHandle handle);

TextureResponse texture_assets_get_texture(TextureAssets* assets, TextureHandle handle);
Texture* texture_assets_get_texture_or_default(TextureAssets* assets, TextureHandle handle);
Texture* texture_assets_get_texture_unchecked(TextureAssets* assets, TextureHandle handle);

void texture_assets_unload_image(TextureAssets* assets, TextureHandle handle);
void texture_assets_unload_texture(TextureAssets* assets, TextureHandle handle);
void texture_assets_unload_slot(TextureAssets* assets, TextureHandle handle);

#endif // __HEADER_ASSET_ASSET

#define __HEADER_EVENTS_DECLMACRO
#ifdef __HEADER_EVENTS_DECLMACRO

/**
 * Use matching versions of DECLARE and DEFINE_IMPL macros
 * Both ACCESS_SINGLE_THREADED or ACCESS_MULTI_THREADED
 * 
 * Event types have these constraints:
 * - Should define field "bool consumed"
 */

#define EventBuffer(TYPE_EVENT) EventBuffer_##TYPE_EVENT
#define Events(TYPE_EVENT) Events_##TYPE_EVENT
#define EventsIter(TYPE_EVENT) EventsIter_##TYPE_EVENT

#define events_create(TYPE_EVENT) TYPE_EVENT##__events_create
#define events_send_single(TYPE_EVENT) TYPE_EVENT##__events_send_single
#define events_send_batch(TYPE_EVENT) TYPE_EVENT##__events_send_batch
#define events_swap_buffers(TYPE_EVENT) TYPE_EVENT##__events_swap_buffers

/**
 * SHOULD ALWAYS BE PAIRED WITH A `events_iter_end` CALL TO RELEASE THE LOCK
 */
#define events_begin_iter(TYPE_EVENT) TYPE_EVENT##__events_begin_iter
/**
 * SHOULD ALWAYS BE PAIRED WITH A `events_iter_end` CALL TO RELEASE THE LOCK
 */
#define events_begin_iter_all(TYPE_EVENT) TYPE_EVENT##__events_begin_iter_all
/**
 * 
 * @return offset for next frame
 * 
 */
#define events_iter_end(TYPE_EVENT) TYPE_EVENT##__events_iter_end
#define events_iter_has_next(TYPE_EVENT) TYPE_EVENT##__events_iter_has_next
#define events_iter_read_next(TYPE_EVENT) TYPE_EVENT##__events_iter_read_next
#define events_iter_consume_next(TYPE_EVENT) TYPE_EVENT##__events_iter_consume_next
#define events_iter_maybe_consume_next(TYPE_EVENT) TYPE_EVENT##__events_iter_maybe_consume_next

// -------------------------------------------------------------------------------------------------------------------

#define __DECLARE__EVENT_SYSTEM__ACCESS_SINGLE_THREADED(TYPE_EVENT) \
\
    typedef struct {\
        usize count;\
        usize capacity;\
        TYPE_EVENT* items;\
    } EventBuffer_##TYPE_EVENT;\
\
    typedef struct {\
        EventBuffer_##TYPE_EVENT event_buffers[2];\
        uint8 this_frame_ind;\
    } Events_##TYPE_EVENT;\
\
    Events_##TYPE_EVENT TYPE_EVENT##__events_create();\
    void TYPE_EVENT##__events_send_single(Events_##TYPE_EVENT* events, TYPE_EVENT event);\
    void TYPE_EVENT##__events_send_batch(Events_##TYPE_EVENT* events, TYPE_EVENT* event_list, usize count);\
    void TYPE_EVENT##__events_swap_buffers(Events_##TYPE_EVENT* events);\
\
    typedef struct {\
        usize index;\
        Events_##TYPE_EVENT* events;\
    } EventsIter_##TYPE_EVENT;\
\
    EventsIter_##TYPE_EVENT TYPE_EVENT##__events_begin_iter(Events_##TYPE_EVENT* events, usize offset);\
    EventsIter_##TYPE_EVENT TYPE_EVENT##__events_begin_iter_all(Events_##TYPE_EVENT* events);\
    usize TYPE_EVENT##__events_iter_end(EventsIter_##TYPE_EVENT* iter);\
    bool TYPE_EVENT##__events_iter_has_next(EventsIter_##TYPE_EVENT* iter);\
    TYPE_EVENT TYPE_EVENT##__events_iter_read_next(EventsIter_##TYPE_EVENT* iter);\
    TYPE_EVENT TYPE_EVENT##__events_iter_consume_next(EventsIter_##TYPE_EVENT* iter);\
    TYPE_EVENT TYPE_EVENT##__events_iter_maybe_consume_next(EventsIter_##TYPE_EVENT* iter, bool** set_consumed);

// -------------------------------------------------------------------------------------------------------------------

#define __DECLARE__EVENT_SYSTEM__ACCESS_MULTI_THREADED(TYPE_EVENT) \
\
    typedef struct {\
        usize count;\
        usize capacity;\
        TYPE_EVENT* items;\
    } EventBuffer_##TYPE_EVENT;\
\
    typedef struct {\
        SRWLock* rw_lock;\
        EventBuffer_##TYPE_EVENT event_buffers[2];\
        uint8 this_frame_ind;\
    } Events_##TYPE_EVENT;\
\
    Events_##TYPE_EVENT TYPE_EVENT##__events_create();\
    void TYPE_EVENT##__events_send_single(Events_##TYPE_EVENT* events, TYPE_EVENT event);\
    void TYPE_EVENT##__events_send_batch(Events_##TYPE_EVENT* events, TYPE_EVENT* event_list, usize count);\
    void TYPE_EVENT##__events_swap_buffers(Events_##TYPE_EVENT* events);\
\
    typedef struct {\
        usize index;\
        Events_##TYPE_EVENT* events;\
    } EventsIter_##TYPE_EVENT;\
\
    EventsIter_##TYPE_EVENT TYPE_EVENT##__events_begin_iter(Events_##TYPE_EVENT* events, usize offset);\
    EventsIter_##TYPE_EVENT TYPE_EVENT##__events_begin_iter_all(Events_##TYPE_EVENT* events);\
    usize TYPE_EVENT##__events_iter_end(EventsIter_##TYPE_EVENT* iter);\
    bool TYPE_EVENT##__events_iter_has_next(EventsIter_##TYPE_EVENT* iter);\
    TYPE_EVENT TYPE_EVENT##__events_iter_read_next(EventsIter_##TYPE_EVENT* iter);\
    TYPE_EVENT TYPE_EVENT##__events_iter_consume_next(EventsIter_##TYPE_EVENT* iter);\
    TYPE_EVENT TYPE_EVENT##__events_iter_maybe_consume_next(EventsIter_##TYPE_EVENT* iter, bool** set_consumed);

// -------------------------------------------------------------------------------------------------------------------

#define __IMPL_____EVENT_SYSTEM__ACCESS_SINGLE_THREADED(TYPE_EVENT) \
\
    void TYPE_EVENT##__event_buffer_push_back(EventBuffer_##TYPE_EVENT* buffer, TYPE_EVENT item) {\
        const uint32 INITIAL_CAPACITY = 32;\
        const uint32 GROWTH_FACTOR = 2;\
\
        if (buffer->capacity == 0) {\
            buffer->capacity = INITIAL_CAPACITY;\
            buffer->items = malloc(buffer->capacity * sizeof(*&item));\
        }\
        else if (buffer->count == buffer->capacity) {\
            buffer->capacity = GROWTH_FACTOR * buffer->capacity;\
            buffer->items = realloc(buffer->items, buffer->capacity * sizeof(*&item));\
        }\
\
        buffer->items[buffer->count] = item;\
        buffer->count++;\
\
    }\
\
    void TYPE_EVENT##__event_buffer_push_back_batch(EventBuffer_##TYPE_EVENT* buffer, TYPE_EVENT* items, usize count) {\
        const uint32 INITIAL_CAPACITY = 32;\
        const uint32 GROWTH_FACTOR = 2;\
\
        if (buffer->capacity == 0) {\
            buffer->capacity = __max(INITIAL_CAPACITY, count);\
            buffer->items = malloc(buffer->capacity * sizeof(*items));\
        }\
        else if (buffer->count + count > buffer->capacity) {\
            buffer->capacity = __max(GROWTH_FACTOR * buffer->capacity, buffer->count + count);\
            buffer->items = realloc(buffer->items, buffer->capacity * sizeof(*items));\
        }\
\
        memcpy(buffer->items + buffer->count, items, count * sizeof(*items));\
        buffer->count += count;\
\
    }\
\
    void TYPE_EVENT##__event_buffer_clear(EventBuffer_##TYPE_EVENT* buffer) {\
        buffer->count = 0;\
    }\
\
    Events_##TYPE_EVENT TYPE_EVENT##__events_create() {\
        Events_##TYPE_EVENT events = {0};\
        return events;\
    }\
\
    void TYPE_EVENT##__events_send_single(Events_##TYPE_EVENT* events, TYPE_EVENT event) {\
        TYPE_EVENT##__event_buffer_push_back(&events->event_buffers[events->this_frame_ind], event);\
    }\
\
    void TYPE_EVENT##__events_send_batch(Events_##TYPE_EVENT* events, TYPE_EVENT* event_list, usize count) {\
        TYPE_EVENT##__event_buffer_push_back_batch(&events->event_buffers[events->this_frame_ind], event_list, count);\
    }\
\
    void TYPE_EVENT##__events_swap_buffers(Events_##TYPE_EVENT* events) {\
        events->this_frame_ind = !events->this_frame_ind;\
        TYPE_EVENT##__event_buffer_clear(&events->event_buffers[events->this_frame_ind]);\
    }\
\
    EventsIter_##TYPE_EVENT TYPE_EVENT##__events_begin_iter(Events_##TYPE_EVENT* events, usize offset) {\
        return (EventsIter_##TYPE_EVENT) {\
            .index = offset,\
            .events = events,\
        };\
    }\
\
    EventsIter_##TYPE_EVENT TYPE_EVENT##__events_begin_iter_all(Events_##TYPE_EVENT* events) {\
        return TYPE_EVENT##__events_begin_iter(events, 0);\
    }\
\
    usize TYPE_EVENT##__events_iter_end(EventsIter_##TYPE_EVENT* iter) {\
        usize count = iter->events->event_buffers[iter->events->this_frame_ind].count;\
        return count;\
    }\
\
    bool TYPE_EVENT##__events_iter_has_next(EventsIter_##TYPE_EVENT* iter) {\
            return iter->index < iter->events->event_buffers[0].count + iter->events->event_buffers[1].count;\
    }\
\
    TYPE_EVENT TYPE_EVENT##__events_iter_read_next(EventsIter_##TYPE_EVENT* iter) {\
        EventBuffer_##TYPE_EVENT events_this_frame = iter->events->event_buffers[iter->events->this_frame_ind];\
        EventBuffer_##TYPE_EVENT events_last_frame = iter->events->event_buffers[!iter->events->this_frame_ind];\
\
        usize index = iter->index;\
        iter->index++;\
\
        if (index < events_this_frame.count) {\
            return events_this_frame.items[index];\
        }\
        return events_last_frame.items[index - events_this_frame.count];\
    }\
\
    TYPE_EVENT TYPE_EVENT##__events_iter_consume_next(EventsIter_##TYPE_EVENT* iter) {\
        EventBuffer_##TYPE_EVENT events_this_frame = iter->events->event_buffers[iter->events->this_frame_ind];\
        EventBuffer_##TYPE_EVENT events_last_frame = iter->events->event_buffers[!iter->events->this_frame_ind];\
\
        usize index = iter->index;\
        iter->index++;\
\
        TYPE_EVENT* event;\
        if (index < events_this_frame.count) {\
            event = &events_this_frame.items[index];\
        }\
        else {\
            event = &events_last_frame.items[index - events_this_frame.count];\
        }\
\
        event->consumed = true;\
        return *event;\
    }\
\
    TYPE_EVENT TYPE_EVENT##__events_iter_maybe_consume_next(EventsIter_##TYPE_EVENT* iter, bool** set_consumed) {\
        EventBuffer_##TYPE_EVENT events_this_frame = iter->events->event_buffers[iter->events->this_frame_ind];\
        EventBuffer_##TYPE_EVENT events_last_frame = iter->events->event_buffers[!iter->events->this_frame_ind];\
\
        usize index = iter->index;\
        iter->index++;\
\
        TYPE_EVENT* event;\
        if (index < events_this_frame.count) {\
            event = &events_this_frame.items[index];\
        }\
        else {\
            event = &events_last_frame.items[index - events_this_frame.count];\
        }\
\
        *set_consumed = &event->consumed;\
        return *event;\
    }

// -------------------------------------------------------------------------------------------------------------------

#define __IMPL_____EVENT_SYSTEM__ACCESS_MULTI_THREADED(TYPE_EVENT) \
\
    void TYPE_EVENT##__event_buffer_push_back(EventBuffer_##TYPE_EVENT* buffer, TYPE_EVENT item) {\
        const uint32 INITIAL_CAPACITY = 32;\
        const uint32 GROWTH_FACTOR = 2;\
\
        if (buffer->capacity == 0) {\
            buffer->capacity = INITIAL_CAPACITY;\
            buffer->items = malloc(buffer->capacity * sizeof(*&item));\
        }\
        else if (buffer->count == buffer->capacity) {\
            buffer->capacity = GROWTH_FACTOR * buffer->capacity;\
            buffer->items = realloc(buffer->items, buffer->capacity * sizeof(*&item));\
        }\
\
        buffer->items[buffer->count] = item;\
        buffer->count++;\
\
    }\
\
    void TYPE_EVENT##__event_buffer_push_back_batch(EventBuffer_##TYPE_EVENT* buffer, TYPE_EVENT* items, usize count) {\
        const uint32 INITIAL_CAPACITY = 32;\
        const uint32 GROWTH_FACTOR = 2;\
\
        if (buffer->capacity == 0) {\
            buffer->capacity = __max(INITIAL_CAPACITY, count);\
            buffer->items = malloc(buffer->capacity * sizeof(*items));\
        }\
        else if (buffer->count + count > buffer->capacity) {\
            buffer->capacity = __max(GROWTH_FACTOR * buffer->capacity, buffer->count + count);\
            buffer->items = realloc(buffer->items, buffer->capacity * sizeof(*items));\
        }\
\
        memcpy(buffer->items + buffer->count, items, count * sizeof(*items));\
        buffer->count += count;\
\
    }\
\
    void TYPE_EVENT##__event_buffer_clear(EventBuffer_##TYPE_EVENT* buffer) {\
        buffer->count = 0;\
    }\
\
    Events_##TYPE_EVENT TYPE_EVENT##__events_create() {\
        Events_##TYPE_EVENT events = {0};\
        events.rw_lock = alloc_create_srw_lock();\
        return events;\
    }\
\
    void TYPE_EVENT##__events_send_single(Events_##TYPE_EVENT* events, TYPE_EVENT event) {\
        srw_lock_acquire_exclusive(events->rw_lock);\
            TYPE_EVENT##__event_buffer_push_back(&events->event_buffers[events->this_frame_ind], event);\
        srw_lock_release_exclusive(events->rw_lock);\
    }\
\
    void TYPE_EVENT##__events_send_batch(Events_##TYPE_EVENT* events, TYPE_EVENT* event_list, usize count) {\
        srw_lock_acquire_exclusive(events->rw_lock);\
            TYPE_EVENT##__event_buffer_push_back_batch(&events->event_buffers[events->this_frame_ind], event_list, count);\
        srw_lock_release_exclusive(events->rw_lock);\
    }\
\
    void TYPE_EVENT##__events_swap_buffers(Events_##TYPE_EVENT* events) {\
        srw_lock_acquire_exclusive(events->rw_lock);\
            events->this_frame_ind = !events->this_frame_ind;\
            TYPE_EVENT##__event_buffer_clear(&events->event_buffers[events->this_frame_ind]);\
        srw_lock_release_exclusive(events->rw_lock);\
    }\
\
    EventsIter_##TYPE_EVENT TYPE_EVENT##__events_begin_iter(Events_##TYPE_EVENT* events, usize offset) {\
        srw_lock_acquire_shared(events->rw_lock);\
        return (EventsIter_##TYPE_EVENT) {\
            .index = offset,\
            .events = events,\
        };\
    }\
\
    EventsIter_##TYPE_EVENT TYPE_EVENT##__events_begin_iter_all(Events_##TYPE_EVENT* events) {\
        return TYPE_EVENT##__events_begin_iter(events, 0);\
    }\
\
    usize TYPE_EVENT##__events_iter_end(EventsIter_##TYPE_EVENT* iter) {\
        usize count = iter->events->event_buffers[iter->events->this_frame_ind].count;\
        srw_lock_release_shared(iter->events->rw_lock);\
        return count;\
    }\
\
    bool TYPE_EVENT##__events_iter_has_next(EventsIter_##TYPE_EVENT* iter) {\
            return iter->index < iter->events->event_buffers[0].count + iter->events->event_buffers[1].count;\
    }\
\
    TYPE_EVENT TYPE_EVENT##__events_iter_read_next(EventsIter_##TYPE_EVENT* iter) {\
        EventBuffer_##TYPE_EVENT events_this_frame = iter->events->event_buffers[iter->events->this_frame_ind];\
        EventBuffer_##TYPE_EVENT events_last_frame = iter->events->event_buffers[!iter->events->this_frame_ind];\
\
        usize index = iter->index;\
        iter->index++;\
\
        if (index < events_this_frame.count) {\
            return events_this_frame.items[index];\
        }\
        return events_last_frame.items[index - events_this_frame.count];\
    }\
\
    TYPE_EVENT TYPE_EVENT##__events_iter_consume_next(EventsIter_##TYPE_EVENT* iter) {\
        EventBuffer_##TYPE_EVENT events_this_frame = iter->events->event_buffers[iter->events->this_frame_ind];\
        EventBuffer_##TYPE_EVENT events_last_frame = iter->events->event_buffers[!iter->events->this_frame_ind];\
\
        usize index = iter->index;\
        iter->index++;\
\
        TYPE_EVENT* event;\
        if (index < events_this_frame.count) {\
            event = &events_this_frame.items[index];\
        }\
        else {\
            event = &events_last_frame.items[index - events_this_frame.count];\
        }\
\
        event->consumed = true;\
        return *event;\
    }\
\
    TYPE_EVENT TYPE_EVENT##__events_iter_maybe_consume_next(EventsIter_##TYPE_EVENT* iter, bool** set_consumed) {\
        EventBuffer_##TYPE_EVENT events_this_frame = iter->events->event_buffers[iter->events->this_frame_ind];\
        EventBuffer_##TYPE_EVENT events_last_frame = iter->events->event_buffers[!iter->events->this_frame_ind];\
\
        usize index = iter->index;\
        iter->index++;\
\
        TYPE_EVENT* event;\
        if (index < events_this_frame.count) {\
            event = &events_this_frame.items[index];\
        }\
        else {\
            event = &events_last_frame.items[index - events_this_frame.count];\
        }\
\
        *set_consumed = &event->consumed;\
        return *event;\
    }

// -------------------------------------------------------------------------------------------------------------------

#endif // __HEADER_EVENTS_DECLMACRO

#define __HEADER_EVENTS_EVENTS
#ifdef __HEADER_EVENTS_EVENTS

typedef enum {
    AssetEvent_Loaded,
    AssetEvent_Changed,
    AssetEvent_Unloaded,
} AssetEventType;

typedef struct {
    TextureHandle handle;
    AssetEventType type;
    bool consumed;
} TextureAssetEvent;

typedef struct {
    usize count;
    usize capacity;
    TextureAssetEvent* items;
} EventBuffer_TextureAssetEvent;

typedef struct {
    SRWLock* rw_lock;
    EventBuffer_TextureAssetEvent event_buffers[2];
    uint8 this_frame_ind;
} Events_TextureAssetEvent;

Events_TextureAssetEvent TextureAssetEvent__events_create();
void TextureAssetEvent__events_send_single(Events_TextureAssetEvent* events, TextureAssetEvent event);
void TextureAssetEvent__events_send_batch(Events_TextureAssetEvent* events, TextureAssetEvent* event_list, usize count);
void TextureAssetEvent__events_swap_buffers(Events_TextureAssetEvent* events);

typedef struct {
    usize index;
    Events_TextureAssetEvent* events;
} EventsIter_TextureAssetEvent;

/**
 * SHOULD ALWAYS BE PAIRED WITH A `events_iter_end` CALL TO RELEASE THE LOCK
 */
EventsIter_TextureAssetEvent TextureAssetEvent__events_begin_iter(Events_TextureAssetEvent* events, usize offset);
/**
 * SHOULD ALWAYS BE PAIRED WITH A `events_iter_end` CALL TO RELEASE THE LOCK
 */
EventsIter_TextureAssetEvent TextureAssetEvent__events_begin_iter_all(Events_TextureAssetEvent* events);
/**
 * 
 * @return offset for next frame
 */
usize TextureAssetEvent__events_iter_end(EventsIter_TextureAssetEvent* iter);
bool TextureAssetEvent__events_iter_has_next(EventsIter_TextureAssetEvent* iter);
TextureAssetEvent TextureAssetEvent__events_iter_read_next(EventsIter_TextureAssetEvent* iter);
TextureAssetEvent TextureAssetEvent__events_iter_consume_next(EventsIter_TextureAssetEvent* iter);
TextureAssetEvent TextureAssetEvent__events_iter_maybe_consume_next(EventsIter_TextureAssetEvent* iter, bool** set_consumed);

#endif // __HEADER_EVENTS_EVENTS

#define __HEADER_ASSET_ASSETSERVER
#ifdef __HEADER_ASSET_ASSETSERVER

typedef struct {
    ThreadPool* RES_threadpool;
    TextureAssets* RES_texture_assets;
    Events_TextureAssetEvent* RES_texture_assets_events;
    const char const* asset_folder;
} FileImageServer;

TextureHandle asyncio_file_load_image(
    FileImageServer* server,
    const char* filepath
);

#endif // __HEADER_ASSET_ASSETSERVER

#define __HEADER_INPUT_INPUT
#ifdef __HEADER_INPUT_INPUT

// same MAX definitions as in raylib
#define MAX_KEYBOARD_KEYS 512
// #define MAX_GAMEPADS 4
#define MAX_GAMEPAD_BUTTONS 32
#define MAX_GAMEPAD_AXES 10

#define MAX_CONTROLS 512

typedef enum {
    KEY_STATE_NULL = 0,
    KEY_STATE_RELEASED = 1,
    KEY_STATE_PRESSED = 2,
    KEY_STATE_REPEATED = 3,
} KeyState;

typedef struct {
    KeyState keys[MAX_KEYBOARD_KEYS];
} KeyInputs;

bool key_just_pressed(KeyInputs* key_inputs, uint32 key);
bool key_is_repeated(KeyInputs* key_inputs, uint32 key);
bool key_is_released(KeyInputs* key_inputs, uint32 key);
bool key_is_up(KeyInputs* key_inputs, uint32 key);
bool key_is_down(KeyInputs* key_inputs, uint32 key);

void update_key_state(KeyInputs* key_inputs, uint32 key, bool state);
void set_key_repeated(KeyInputs* key_inputs, uint32 key);

typedef struct {
    KeyState buttons[MAX_GAMEPAD_BUTTONS];
    float32 axis_values[MAX_GAMEPAD_AXES];
    float32 prev_axis_values[MAX_GAMEPAD_AXES];
} GamepadInputs;

bool gamepad_button_just_pressed(GamepadInputs* gamepad_inputs, uint32 button);
bool gamepad_button_is_repeated(GamepadInputs* gamepad_inputs, uint32 button);
bool gamepad_button_is_released(GamepadInputs* gamepad_inputs, uint32 button);
bool gamepad_button_is_up(GamepadInputs* gamepad_inputs, uint32 button);
bool gamepad_button_is_down(GamepadInputs* gamepad_inputs, uint32 button);
float32 gamepad_axis_value(GamepadInputs* gamepad_inputs, uint32 axis);
float32 gamepad_axis_delta(GamepadInputs* gamepad_inputs, uint32 axis);

void update_gamepad_button_state(GamepadInputs* gamepad_inputs, uint32 button, bool state);
void update_gamepad_axis_value(GamepadInputs* gamepad_inputs, uint32 axis, float32 value);

typedef struct {
    uint32 map[MAX_CONTROLS];       // input -> control
    uint32 invmap[MAX_CONTROLS];    // control -> input
} ControlsMap;

void controls_map_set_control(ControlsMap* controls, uint32 input, uint32 control);
uint32 controls_map_get_input(ControlsMap* controls, uint32 control);
uint32 controls_map_get_control(ControlsMap* controls, uint32 input);

#endif // __HEADER_INPUT_INPUT

#define __HEADER_ANIMATION_TIMER
#ifdef __HEADER_ANIMATION_TIMER

typedef enum {
    Timer_Repeating = 0,
    Timer_NonRepeating,
} TimerMode;

typedef struct {
    TimerMode mode;
    float32 time_setup;
    float32 time_elapsed;
    bool finished;
} Timer;

Timer new_timer(float32 setup_secs, TimerMode mode);
void reset_timer(Timer* timer);
void tick_timer(Timer* timer, float32 delta_time);
bool timer_is_finished(Timer* timer);

typedef struct {
    TimerMode mode;
    uint32 checkpoint_count;
    float32* checkpoints;
    uint32 current_index;
    float32 time_elapsed;
    bool pulsed;
    bool finished;
} SequenceTimer;

SequenceTimer new_sequence_timer(float32* ref_checkpoints, uint32 count, TimerMode mode);
SequenceTimer new_sequence_timer_evenly_spaced(float32 time_between, uint32 count, TimerMode mode);
void reset_sequence_timer(SequenceTimer* timer);
void tick_sequence_timer(SequenceTimer* timer, float delta_time);
bool sequence_timer_has_pulsed(SequenceTimer* timer);
bool sequence_timer_is_finished(SequenceTimer* timer);

typedef struct {
    TimerMode mode;
    uint32 step_count;
    uint32 current_step;
    bool finished;
} StepTimer;

StepTimer new_step_timer(uint32 count, TimerMode mode);
void reset_step_timer(StepTimer* timer);
void tick_step_timer(StepTimer* timer);
bool step_timer_is_finished(StepTimer* timer);

#endif // __HEADER_ANIMATION_TIMER

#define __HEADER_ANIMATION_TWEEN
#ifdef __HEADER_ANIMATION_TWEEN

typedef float32 (*EaseFunction)(float32 p);

float32 ease_quadratic_in(float32 p);
float32 ease_quadratic_out(float32 p);
float32 ease_quadratic_in_out(float32 p);

float32 ease_cubic_in(float32 p);
float32 ease_cubic_out(float32 p);
float32 ease_cubic_in_out(float32 p);

float32 ease_quartic_in(float32 p);
float32 ease_quartic_out(float32 p);
float32 ease_quartic_in_out(float32 p);

float32 ease_quintic_in(float32 p);
float32 ease_quintic_out(float32 p);
float32 ease_quintic_in_out(float32 p);

float32 ease_sine_in(float32 p);
float32 ease_sine_out(float32 p);
float32 ease_sine_in_out(float32 p);

float32 ease_circular_in(float32 p);
float32 ease_circular_out(float32 p);
float32 ease_circular_in_out(float32 p);

float32 ease_exponential_in(float32 p);
float32 ease_exponential_out(float32 p);
float32 ease_exponential_in_out(float32 p);

float32 ease_elastic_in(float32 p);
float32 ease_elastic_out(float32 p);
float32 ease_elastic_in_out(float32 p);

float32 ease_back_in(float32 p);
float32 ease_back_out(float32 p);
float32 ease_back_in_out(float32 p);

float32 ease_bounce_in(float32 p);
float32 ease_bounce_out(float32 p);
float32 ease_bounce_in_out(float32 p);

typedef enum {
    ANIMATION_CURVE_LINEAR = 0,
    ANIMATION_CURVE_DELAY,
    ANIMATION_CURVE_STEP,
    ANIMATION_CURVE_EASED,
} AnimationCurveType;

typedef struct {
    AnimationCurveType type;
    union {
        float32 step_cutoff;
        EaseFunction ease_function;
    };
} AnimationCurve;

AnimationCurve animation_curve_linear();
AnimationCurve animation_curve_delay();
AnimationCurve animation_curve_step(float32 step_cutoff);
AnimationCurve animation_curve_eased(EaseFunction ease_function);

float32 eval_animation_curve(AnimationCurve curve, float32 progress);

typedef enum {
    TWEEN_ONCE = 0,
    TWEEN_REPEAT_STARTOVER,
    TWEEN_REPEAT_MIRRORED,
    // TODO: TWEEN_REPEAT_CURVEMIRRORED
} TweenMode;

typedef struct {
    TweenMode mode;
    AnimationCurve curve;
    float32 duration;
    // --
    float32 elapsed;
    int32 direction; // -1 | 1
} TweenState;

TweenState new_tween_state(TweenMode mode, AnimationCurve curve, float32 duration);
float32 tween_state_tick(TweenState* tween, float32 delta_time);

typedef struct {
    AnimationCurve curve;
    float32 duration;
} TweenSequenceStateSection;

typedef struct {
    usize count;
    usize capacity;
    TweenSequenceStateSection* items;
} TweenSequenceStateSectionList;

typedef struct {
    TweenMode mode;
    usize section;
    TweenSequenceStateSectionList sections;
    // --
    float32 elapsed;
    int32 direction; // -1 | 1
} TweenSequenceState;

TweenSequenceState new_tween_sequence_state(TweenMode mode);

typedef struct {
    usize section;
    float32 progress_out;
} TweenSequenceTickOut;

TweenSequenceTickOut tween_sequence_state_tick(TweenSequenceState* tween, float32 delta_time);

typedef struct {
    Vector2 start;
    Vector2 end;
} TweenLimits_Vector2;

typedef struct {
    usize count;
    usize capacity;
    TweenLimits_Vector2* items;
} TweenLimitsList_Vector2;

typedef struct {
    TweenState state;
    TweenLimits_Vector2 limits;
} Tween_Vector2;

typedef struct {
    TweenSequenceState state;
    TweenLimitsList_Vector2 limits_list;
} TweenSequence_Vector2;

Vector2 vector2_interpolate(Vector2 start, Vector2 end, float32 progress);
Vector2 Vector2__tween_tick(Tween_Vector2* tween, float32 delta_time);
Vector2 Vector2__tween_sequence_tick(TweenSequence_Vector2* tween, float32 delta_time);

// -------------------------------------------------------------------------------------------------------------------

#define Tween(TYPE) Tween_##TYPE
#define TweenSequence(TYPE) TweenSequence_##TYPE
#define tween_tick(TYPE) TYPE##__tween_tick
#define tween_sequence_tick(TYPE) TYPE##__tween_sequence_tick

// -------------------------------------------------------------------------------------------------------------------

#define __DECLARE__TWEEN(TYPE) \
\
    typedef struct { \
        TYPE start; \
        TYPE end; \
    } TweenLimits_##TYPE; \
\
    typedef struct { \
        usize count; \
        usize capacity; \
        TweenLimits_##TYPE* items; \
    } TweenLimitsList_##TYPE; \
\
    typedef struct { \
        TweenState state; \
        TweenLimits_##TYPE limits; \
    } Tween_##TYPE; \
\
    typedef struct { \
        TweenSequenceState state; \
        TweenLimitsList_##TYPE limits_list; \
    } TweenSequence_##TYPE; \
\
    TYPE TYPE##__tween_tick(Tween_##TYPE* tween, float32 delta_time); \
    TYPE TYPE##__tween_sequence_tick(TweenSequence_##TYPE* tween, float32 delta_time);

// -------------------------------------------------------------------------------------------------------------------

#define __IMPL_____TWEEN(TYPE, InterpolateFn) \
\
    TYPE TYPE##__tween_tick(Tween_##TYPE* tween, float32 delta_time) { \
        float32 progress_out = tween_state_tick(&tween->state, delta_time); \
        return (InterpolateFn)(tween->limits.start, tween->limits.end, progress_out); \
    } \
    TYPE TYPE##__tween_sequence_tick(Tween_##TYPE* tween, float32 delta_time) { \
        TweenSequenceTickOut tick_out = tween_sequence_state_tick(&tween->state, delta_time); \
        TweenLimits_##TYPE limits = tween->limits_list.items[tick_out.section]; \
        return (InterpolateFn)(limits.start, limits.end, tick_out.progress_out); \
    }

// -------------------------------------------------------------------------------------------------------------------

#endif // __HEADER_ANIMATION_TWEEN

#define __HEADER_ANIMATION_ANIMATION
#ifdef __HEADER_ANIMATION_ANIMATION

typedef struct {
    // -- Setup --
    URectSize texture_offset;
    URectSize sprite_size;
    uint32 rows;
    uint32 cols;
    uint32 frame_count;
    // -- State --
    StepTimer timer;
    uint32 current_frame;
    bool finished;
} SpriteSheetAnimationState;

SpriteSheetAnimationState new_animation_state_row(
    URectSize sprite_size,
    uint32 frame_count
);
SpriteSheetAnimationState new_animation_state_grid(
    URectSize sprite_size,
    uint32 rows,
    uint32 cols,
    uint32 frame_count
);
void reset_animation_state(SpriteSheetAnimationState* state);
void tick_animation_state(SpriteSheetAnimationState* state);
void tick_back_animation_state(SpriteSheetAnimationState* state);
Rectangle animation_state_current_frame(SpriteSheetAnimationState* state);
bool animation_is_finished(SpriteSheetAnimationState* state);

#endif // __HEADER_ANIMATION_ANIMATION

#define __HEADER_PHYSICS_COLLISION
#ifdef __HEADER_PHYSICS_COLLISION

typedef struct {
    Vector2 position;
    Vector2 direction;
} Ray2;

typedef struct {
    Vector2 start;
    Vector2 end;
} LineSegmentCollider;

typedef struct {
    Vector2 center;
    float32 radius;
} CircleCollider;

typedef struct {
    float32 x_left;
    float32 x_right;
    float32 y_top;
    float32 y_bottom;
} AABBCollider;

typedef struct {
    uint32 count; // collider count
    uint32 capacity;
    AABBCollider bounding_box;
    AABBCollider* colliders;
} AABBColliderSet;

// TODO: FreeRectangleCollider: arbitrarily rotated rectangle

AABBColliderSet clone_aabb_collider_set(AABBColliderSet* set);

static inline AABBCollider shift_aabb(SpaceShift shift, AABBCollider aabb) {
    return (AABBCollider) {
        .x_left = shift.translation.x + (aabb.x_left * shift.scale.x),
        .x_right = shift.translation.x + (aabb.x_right * shift.scale.x),
        .y_top = shift.translation.y + (aabb.y_top * shift.scale.y),
        .y_bottom = shift.translation.y + (aabb.y_bottom * shift.scale.y),
    };
}

float32 collider_dist_circle_circle(CircleCollider c1, CircleCollider c2);
bool check_point_inside_aabb(AABBCollider a1, Vector2 p);

bool check_collision_line_line(LineSegmentCollider l1, LineSegmentCollider l2);
bool check_collision_line_circle(LineSegmentCollider l1, CircleCollider c2);
bool check_collision_line_aabb(LineSegmentCollider l1, AABBCollider a2);

bool check_collision_circle_circle(CircleCollider c1, CircleCollider c2);
bool check_collision_circle_aabb(CircleCollider c1, AABBCollider a2);

bool check_collision_aabb_aabb(AABBCollider a1, AABBCollider a2);
bool check_shifted_collision_aabb_aabb(SpaceShift o1, AABBCollider a1, SpaceShift o2, AABBCollider a2);

bool check_rayhit_circle(Ray2 ray, CircleCollider c1, float32 max_dist);
bool check_rayhit_aabb(Ray2 ray, AABBCollider a1, float32 max_dist);

bool check_collision_aabb_collider_sets(AABBColliderSet* s1, AABBColliderSet* s2);
bool check_shifted_collision_aabb_collider_sets(SpaceShift o1, AABBColliderSet* s1, SpaceShift o2, AABBColliderSet* s2);
void recalculate_bounding_box(AABBColliderSet* set);

#endif // __HEADER_PHYSICS_COLLISION

#define __HEADER_SHAPES_SHAPES
#ifdef __HEADER_SHAPES_SHAPES

typedef struct {
    float32 head_radius;
    Vector2 base;
    Vector2 direction; // normalized
    float32 length;
} Arrow;

Vector2 arrow_get_head(Arrow arrow);
void arrow_draw(Arrow arrow, float32 thick, Color color);

#endif // __HEADER_SHAPES_SHAPES

#define __HEADER_UI_UILAYOUT
#ifdef __HEADER_UI_UILAYOUT

typedef struct {
    Rectangle box;
    float32 box_padding;
    float32 row_padding;
    uint32 rows;
    //
    Rectangle content_box;
    RectSize row_size;
    uint32 next_row;
} RowLayout;

RowLayout make_row_layout(RowLayout layout);
Rectangle row_layout_next_n(RowLayout* layout, uint32 count);
Rectangle row_layout_next(RowLayout* layout);

typedef struct {
    Rectangle box;
    float32 box_padding;
    float32 col_padding;
    uint32 cols;
    //
    Rectangle content_box;
    RectSize col_size;
    uint32 next_col;
} ColumnLayout;

ColumnLayout make_column_layout(ColumnLayout layout);
Rectangle column_layout_next_n(ColumnLayout* layout, uint32 count);
Rectangle column_layout_next(ColumnLayout* layout);

typedef enum {
    Grid_RowMajor,
    Grid_ColumnMajor,
} GridMajor;

typedef struct {
    Rectangle box;
    float32 box_padding;
    float32 cell_padding;
    GridMajor major;
    uint32 rows;
    uint32 cols;
    //
    Rectangle content_box;
    RectSize cell_size;
    uint32 next_cell;
} GridLayout;

GridLayout make_grid_layout(GridLayout layout);
Rectangle grid_layout_next_n(GridLayout* layout, uint32 count);
Rectangle grid_layout_next(GridLayout* layout);

#endif // __HEADER_UI_UILAYOUT

#define __HEADER_UI_JUSTUI
#ifdef __HEADER_UI_JUSTUI

#define CUSTOM_UI_ELEMENT_SLOT_COUNT 100

typedef struct {
    uint32 id;
} UIElementId;

/**
 * Variants [0, CUSTOM_UI_ELEMENT_SLOT_COUNT) are reserved for custom elements
 */
typedef enum {
    UIElementType_Area = CUSTOM_UI_ELEMENT_SLOT_COUNT,
    UIElementType_Button,
    UIElementType_SelectionBox,
    UIElementType_Slider,
    UIElementType_ChoiceList,
    UIElementType_Panel,
} UIElementType;

typedef struct {
    // bool idle;
    bool on_hover;
    bool on_press;
    //
    bool just_begin_hover;
    bool just_end_hover;
    bool just_pressed;
    //
    bool just_clicked;
    Vector2 click_point_relative;
} UIElementState;

// TODO: maybe utilize bitmask
// typedef enum {
//     ON_HOVER,
//     ON_PRESS,
//     //
//     JUST_BEGIN_HOVER,
//     JUST_END_HOVER,
//     JUST_PRESSED,
//     //
//     JUST_CLICKED,
// } UIElementStateEnum;

/**
 * Specific UI Elements have to start with UIElement field
 */
typedef struct {
    UIElementId id;
    UIElementType type;
    UIElementState state;
    uint32 layer;
    Anchor anchor;
    Vector2 position;
    RectSize size;
    bool disabled;
} UIElement;

// ----------------
typedef enum {
    UIEvent_BeginHover,
    UIEvent_StayHover,
    UIEvent_EndHover,
    UIEvent_Pressed,
    UIEvent_Released,
    UIEvent_Update,
    UIEvent_Draw,
    UIEvent_DropElement,
} UIEvent;

typedef struct {
    float32 delta_time;
    Vector2 mouse;      // relative to element top-left
    Vector2 element_origin;
} UIEventContext;

void put_ui_handle_vtable_entry(uint32 type_id, void (*handler)(UIElement* elem, UIEvent event, UIEventContext context));
// ----------------

typedef struct {
    uint32 layer;
    uint32 index;
} ElementSort;

typedef struct {
    BumpAllocator memory;
    byte* memory_reset_cursor;
    uint32 count;
    UIElement** elems;
    ElementSort* layer_sort;
    UIElement* pressed_element;
    bool active;
} UIElementStore;

// ----------------
typedef struct {
    bool is_bordered;
    float32 thick;
    Color color;
} UIBorderStyle;

typedef struct {
    Font font;
    float32 font_size;
    float32 spacing;
    Color color;
} UITextStyle;
// ----------------

typedef struct {
    Color idle_color;
    Color hovered_color;
    UIBorderStyle border;
} AreaStyle;

typedef struct {
    UIElement elem;
    AreaStyle style;
} Area;

typedef struct {
    Color idle_color;
    Color hovered_color;
    Color pressed_color;
    Color disabled_color;
    UIBorderStyle border;
    UITextStyle title;
} ButtonStyle;

typedef struct {
    UIElement elem;
    ButtonStyle style;
    Vector2 draw_offset;
    char title[20];
} Button;

void button_consume_click(Button* button);

typedef struct {
    Color selected_color;
    Color unselected_color;
    Color disabled_color;
    UIBorderStyle border;
    UITextStyle title;
} SelectionBoxStyle;

typedef struct {
    UIElement elem;
    SelectionBoxStyle style;
    char title[20];
    bool selected;
} SelectionBox;

typedef struct {
    Color line_color;
    Color cursor_color;
    UIBorderStyle border;
    UITextStyle title;
} SliderStyle;

typedef struct {
    UIElement elem;
    SliderStyle style;
    char title[20];
    float32 low_value;
    float32 high_value;
    float32 cursor; // [0, 1]
} Slider;

float32 get_slider_value(Slider* slider);

typedef struct {
    Color selected_color;
    Color unselected_color;
    Color disabled_color;
    UIBorderStyle border;
    UITextStyle title;
} ChoiceListStyle;

typedef struct {
    uint32 id;
    char title[20];
} ChoiceListOption;

typedef struct {
    UIElement elem;
    ChoiceListStyle style;
    GridLayout layout;
    Option(uint32) hovered_option_index;
    uint32 selected_option_id;
    uint32 option_count;
    ChoiceListOption options[20];
} ChoiceList;

typedef struct {
    UIElement elem;
    UIElementStore store;
    bool open;
} Panel;

// ----------------
Area make_ui_area(Area area);
Button make_ui_button(Button button);
SelectionBox make_ui_selection_box(SelectionBox selection_box);
Slider make_ui_slider(Slider slider);
ChoiceList make_ui_choice_list(ChoiceList choice_list);
Panel make_ui_panel(Panel panel);
// ----------------

UIElementStore ui_element_store_new_with_count_hint(uint32 count_hint);
UIElementStore ui_element_store_new();
UIElementStore ui_element_store_new_active_with_count_hint(uint32 count_hint);
UIElementStore ui_element_store_new_active();
void ui_element_store_drop_elements(UIElementStore* store);
void ui_element_store_drop(UIElementStore* store);
void ui_element_store_clear(UIElementStore* store);

void* get_ui_element_unchecked(UIElementStore* store, UIElementId elem_id);
UIElement* get_ui_element(UIElementStore* store, UIElementId elem_id);

// ----------------
UIElementId put_ui_element(UIElementStore* store, UIElement* elem, MemoryLayout layout);

UIElementId put_ui_element_area(UIElementStore* store, Area area);
UIElementId put_ui_element_button(UIElementStore* store, Button button);
UIElementId put_ui_element_selection_box(UIElementStore* store, SelectionBox sbox);
UIElementId put_ui_element_slider(UIElementStore* store, Slider slider);
UIElementId put_ui_element_choice_list(UIElementStore* store, ChoiceList choice_list);
UIElementId put_ui_element_panel(UIElementStore* store, Panel panel);
// ----------------

void SYSTEM_INPUT_handle_input_for_ui_store(
    UIElementStore* store
);

void SYSTEM_UPDATE_update_ui_elements(
    UIElementStore* store,
    float32 delta_time
);

void SYSTEM_RENDER_draw_ui_elements(
    UIElementStore* store
);


#endif // __HEADER_UI_JUSTUI

#define __HEADER_RENDER2D_CAMERA2D
#ifdef __HEADER_RENDER2D_CAMERA2D

#define MAX_CAMERA_COUNT 10
#define PRIMARY_CAMERA_ID 0

typedef struct {
    Camera2D camera;
    Layers layers;
    uint32 sort_index;
} SpriteCamera;

typedef struct {
    uint32 count;
    SpriteCamera cameras[MAX_CAMERA_COUNT];
} SpriteCameraStore;

void set_primary_camera(SpriteCameraStore* store, SpriteCamera camera);
SpriteCamera* get_primary_camera(SpriteCameraStore* store);
void add_camera(SpriteCameraStore* store, SpriteCamera camera);

#endif // __HEADER_RENDER2D_CAMERA2D

#define __HEADER_RENDER2D_SPRITE
#ifdef __HEADER_RENDER2D_SPRITE

typedef enum {
    Rotation_CW = 1,
    Rotation_CCW = -1,
} RotationWay;

typedef struct {
    Anchor anchor;
    Vector2 position;       // position of the anchor
    bool use_source_size;
    Vector2 size;
    Vector2 scale;
    float32 rotation;       // rotation around its anchor
    RotationWay rway;
} SpriteTransform;

typedef struct {
    // -- render start
    TextureHandle texture;
    Color tint;
    bool use_custom_source;
    Rectangle source;
    bool flip_x;
    bool flip_y;
    uint32 z_index;
    // -- render end
    bool use_layer_system; // otherwise renders on the primary camera by default
    Layers layers;
    bool visible;
    bool camera_visible;
} Sprite;

typedef struct {
    TextureHandle texture;
    Color tint;
    bool use_custom_source;
    Rectangle source;
    bool flip_x;
    bool flip_y;
    SpriteTransform transform;
    uint32 z_index;
} RenderSprite;

typedef struct {
    uint32 count;
    uint32 capacity;
    RenderSprite* sprites;
} SortedRenderSprites;

typedef struct {
    uint32 camera_index;
    uint32 sort_index;
} CameraSortElem;

typedef struct {
    uint32 camera_count;
    CameraSortElem camera_sort[MAX_CAMERA_COUNT];
    SortedRenderSprites render_sprites[MAX_CAMERA_COUNT];
} PreparedRenderSprites;

typedef struct {
    uint32 count;
    uint32 capacity;
    uint32 free_count;
    bool* slot_occupied;
    uint32* generations;
    SpriteTransform* transforms;
    Sprite* sprites;
} SpriteStore;

typedef EntityId SpriteEntityId;

SpriteEntityId spawn_sprite(
    SpriteStore* sprite_store,
    SpriteTransform transform,
    Sprite sprite
);
void despawn_sprite(SpriteStore* sprite_store, SpriteEntityId sprite_id);
bool sprite_is_valid(SpriteStore* sprite_store, SpriteEntityId sprite_id);

void destroy_sprite_store(SpriteStore* sprite_store);

void SYSTEM_EXTRACT_RENDER_cull_and_sort_sprites(
    SpriteCameraStore* sprite_camera_store,
    SpriteStore* sprite_store,
    PreparedRenderSprites* prepared_render_sprites
);

void SYSTEM_RENDER_sorted_sprites(
    TextureAssets* RES_texture_assets,
    SpriteCameraStore* sprite_camera_store,
    PreparedRenderSprites* prepared_render_sprites
);

#endif // __HEADER_RENDER2D_SPRITE

#define __HEADER_LIB
#ifdef __HEADER_LIB

typedef struct {
    URectSize screen_size;
    uint32 threadpool_nthreads;
    uint32 threadpool_taskqueuecapacity;
    const char* asset_folder;
    SpriteCamera primary_camera;
} JustEngineInit;

typedef struct {
    ThreadPoolShutdown threadpool_shutdown;
} JustEngineDeinit;

typedef struct {
    float32 delta_time;
    // --------
    URectSize screen_size;
    BumpAllocator temporary_storage;
    ThreadPool* threadpool;
    // -- Image/Texture
    FileImageServer file_image_server;
    TextureAssets texture_assets;
    Events_TextureAssetEvent texture_asset_events;
    // -- Render2D
    SpriteCameraStore camera_store;
    SpriteStore sprite_store;
    // -- UI
    UIElementStore ui_store;
    // --------
} JustEngineGlobalResources;

typedef struct {
    // --------
    // -- Render2D
    PreparedRenderSprites prepared_render_sprites;
    // --------
} JustEngineGlobalRenderResources;

extern JustEngineGlobalResources JUST_GLOBAL;
extern JustEngineGlobalRenderResources JUST_RENDER_GLOBAL;

void just_engine_init(JustEngineInit init);
void just_engine_deinit(JustEngineDeinit deinit);

// ---------------------------

/**
 * -- STAGES --
 * 
 * INITIALIZE
 * ----------
 * 
 * INPUT
 * 
 * PREPARE
 *      PRE_PREPARE
 *      PREAPRE
 *      POST_PREPARE
 * 
 * UPDATE
 *      PRE_UPDATE
 *      UPDATE
 *      POST_UPDATE
 * 
 * RENDER
 *      QUEUE_RENDER
 *      EXTRACT_RENDER
 *      RENDER
 * 
 * FRAME_BOUNDARY
 * 
 */

typedef enum {
    STAGE__INPUT,
    //
    STAGE__PREPARE__PRE_PREPARE,
    STAGE__PREPARE__PREPARE,
    STAGE__PREPARE__POST_PREPARE,
    //
    STAGE__UPDATE__PRE_UPDATE,
    STAGE__UPDATE__UPDATE,
    STAGE__UPDATE__POST_UPDATE,
    //
    STAGE__RENDER__QUEUE_RENDER,
    STAGE__RENDER__EXTRACT_RENDER,
    STAGE__RENDER__RENDER,
    //
    STAGE__FRAME_BOUNDARY
} JustEngineSystemStage;

// ---------------------------

void SYSTEM_PRE_PREPARE_set_delta_time(
    float32* RES_delta_time
);

void SYSTEM_POST_UPDATE_camera_visibility(
    SpriteCameraStore* sprite_camera_store,
    SpriteStore* RES_sprite_store
);

void SYSTEM_POST_UPDATE_check_mutated_images(
    TextureAssets* RES_texture_assets,
    Events_TextureAssetEvent* RES_texture_asset_events
);

void SYSTEM_EXTRACT_RENDER_load_textures_for_loaded_or_changed_images(
    TextureAssets* RES_texture_assets,
    Events_TextureAssetEvent* RES_texture_asset_events
);

void SYSTEM_FRAME_BOUNDARY_swap_event_buffers(
    Events_TextureAssetEvent* RES_texture_asset_events
);

void SYSTEM_FRAME_BOUNDARY_reset_temporary_storage(
    TemporaryStorage* RES_temporary_storage
);

// ---------------------------

// -- INPUT --

void JUST_SYSTEM_INPUT_handle_input_for_ui_store();

// -- PREPARE --
// -- -- PRE_PREPARE --

void JUST_SYSTEM_PRE_PREPARE_set_delta_time();

// -- -- PREPARE --
// -- -- POST_PREPARE --

// -- UPDATE --
// -- -- PRE_UPDATE --
// -- -- UPDATE --

void JUST_SYSTEM_UPDATE_update_ui_elements();

// -- -- POST_UPDATE --

void JUST_SYSTEM_POST_UPDATE_check_mutated_images();
void JUST_SYSTEM_POST_UPDATE_camera_visibility();

// -- RENDER --
// -- -- QUEUE_RENDER --
// -- -- EXTRACT_RENDER --

void JUST_SYSTEM_EXTRACT_RENDER_load_textures_for_loaded_or_changed_images();
void JUST_SYSTEM_EXTRACT_RENDER_cull_and_sort_sprites();

// -- -- RENDER --

void JUST_SYSTEM_RENDER_sorted_sprites();
void JUST_SYSTEM_RENDER_draw_ui_elements();

// -- FRAME_BOUNDARY --

void JUST_SYSTEM_FRAME_BOUNDARY_swap_event_buffers();
void JUST_SYSTEM_FRAME_BOUNDARY_reset_temporary_storage();

// ---------------------------

void JUST_ENGINE_RUN_STAGE(JustEngineSystemStage stage);

#endif // __HEADER_LIB
