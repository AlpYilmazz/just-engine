#pragma once

#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#define __HEADER_BASE
#ifdef __HEADER_BASE

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

// typedef     uint8                   bool;

#define MAX(a, b) ((a >= b) ? a : b)
#define MIN(a, b) ((a <= b) ? a : b)

#endif // __HEADER_BASE

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

#define __HEADER_ASSET_ASSET
#ifdef __HEADER_ASSET_ASSET

#define TEXTURE_SLOTS 100

typedef struct {
    uint32 id;
} AssetHandle;

AssetHandle primary_handle();
AssetHandle new_handle(uint32 id);

typedef struct {
    uint32 id;
} TextureHandle;

TextureHandle primary_texture_handle();
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

TextureAssets just_engine_new_texture_assets();

TextureHandle just_engine_texture_assets_reserve_texture_slot(TextureAssets* assets);

void just_engine_texture_assets_put_image(TextureAssets* assets, TextureHandle handle, Image image);
void just_engine_texture_assets_load_image_unchecked(TextureAssets* assets, TextureHandle handle);
void just_engine_texture_assets_load_texture_uncheched(TextureAssets* assets, TextureHandle handle);
void just_engine_texture_assets_put_image_and_load_texture(TextureAssets* assets, TextureHandle handle, Image image);
void just_engine_texture_assets_load_texture_then_unload_image(TextureAssets* assets, TextureHandle handle, Image image);

ImageResponse just_engine_texture_assets_get_image(TextureAssets* assets, TextureHandle handle);
Image* just_engine_texture_assets_get_image_or_default(TextureAssets* assets, TextureHandle handle);
Image* just_engine_texture_assets_get_image_unchecked(TextureAssets* assets, TextureHandle handle);

TextureResponse just_engine_texture_assets_get_texture(TextureAssets* assets, TextureHandle handle);
Texture* just_engine_texture_assets_get_texture_or_default(TextureAssets* assets, TextureHandle handle);
Texture* just_engine_texture_assets_get_texture_unchecked(TextureAssets* assets, TextureHandle handle);

void just_engine_texture_assets_unload_image(TextureAssets* assets, TextureHandle handle);
void just_engine_texture_assets_unload_texture(TextureAssets* assets, TextureHandle handle);
void just_engine_texture_assets_unload_slot(TextureAssets* assets, TextureHandle handle);

#endif // __HEADER_ASSET_ASSET

#define __HEADER_ASSET_ASSETSERVER
#ifdef __HEADER_ASSET_ASSETSERVER

typedef struct {
    const char const* asset_folder;
} FileAssetServer;

TextureHandle just_engine_asyncio_file_load_image(
    TextureAssets* texture_assets,
    FileAssetServer* server,
    const char* filepath
);

#endif // __HEADER_ASSET_ASSETSERVER

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
    uint32 count;
    uint32 capacity;
    TextureAssetEvent* items;
} EventBuffer_TextureAssetEvent;

typedef struct {
    SRWLock* rw_lock;
    EventBuffer_TextureAssetEvent event_buffers[2];
    uint8 this_frame_ind;
} Events_TextureAssetEvent;

void just_engine_events_texture_asset_event_send_single(Events_TextureAssetEvent* events, TextureAssetEvent event);
void just_engine_events_texture_asset_event_send_batch(Events_TextureAssetEvent* events, TextureAssetEvent* event_list, uint32 count);
void just_engine_events_texture_asset_event_swap_buffers(Events_TextureAssetEvent* events);

typedef struct {
    uint32 index;
    Events_TextureAssetEvent* events;
} EventsIter_TextureAssetEvent;

EventsIter_TextureAssetEvent just_engine_events_iter_texture_asset_events_begin_iter(Events_TextureAssetEvent* events, uint32 offset);
EventsIter_TextureAssetEvent just_engine_events_iter_texture_asset_events_begin_iter_all(Events_TextureAssetEvent* events);
/**
 * 
 * @return offset for next frame
 */
uint32 just_engine_events_iter_texture_asset_events_end_iter(EventsIter_TextureAssetEvent* iter);
bool just_engine_events_iter_texture_asset_events_has_next(EventsIter_TextureAssetEvent* iter);
TextureAssetEvent just_engine_events_iter_texture_asset_events_read_next(EventsIter_TextureAssetEvent* iter);
TextureAssetEvent just_engine_events_iter_texture_asset_events_consume_next(EventsIter_TextureAssetEvent* iter);

#endif // __HEADER_EVENTS_EVENTS

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

// TODO: RectangleCollider: arbitrarily rotated rectangle

float32 just_engine_collider_dist_circle_circle(CircleCollider c1, CircleCollider c2);

bool just_engine_check_collision_line_line(LineSegmentCollider l1, LineSegmentCollider l2);
bool just_engine_check_collision_circle_circle(CircleCollider c1, CircleCollider c2);
bool just_engine_check_collision_aabb_aabb(AABBCollider r1, AABBCollider r2);

bool just_engine_check_rayhit_circle(Ray2 ray, CircleCollider c1, float32 max_dist);
bool just_engine_check_rayhit_aabb(Ray2 ray, AABBCollider c1, float32 max_dist);

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
