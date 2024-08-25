#pragma once

#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

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


#define TEXTURE_SLOTS 100

typedef struct {
    int id;
} AssetHandle;

AssetHandle primary_handle();
AssetHandle new_handle(int id);

typedef struct {
    int id;
} TextureHandle;

TextureHandle primary_texture_handle();
TextureHandle new_texture_handle(int id);

typedef struct {
    bool exists;
    Image* texture;
} ImageResponse;

typedef struct {
    bool exists;
    Texture* texture;
} TextureResponse;

typedef struct {
    int next_slot_available_bump;
    bool slots[TEXTURE_SLOTS];
    bool image_ready[TEXTURE_SLOTS];
    bool texture_ready[TEXTURE_SLOTS];
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
    EventBuffer_TextureAssetEvent event_buffers[2];
    uint8 this_frame_ind;
} Events_TextureAssetEvent;

uint32 just_engine_events_texture_asset_event_this_frame_buffer_count(Events_TextureAssetEvent* events);
void just_engine_events_texture_asset_event_send_single(Events_TextureAssetEvent* events, TextureAssetEvent event);
void just_engine_events_texture_asset_event_send_batch(Events_TextureAssetEvent* events, TextureAssetEvent* event_list, uint32 count);
void just_engine_events_texture_asset_event_swap_buffers(Events_TextureAssetEvent* events);

typedef struct {
    uint32 index;
    Events_TextureAssetEvent* events;
} EventsIter_TextureAssetEvent;

EventsIter_TextureAssetEvent just_engine_events_iter_texture_asset_events_new(Events_TextureAssetEvent* events, uint32 offset);
EventsIter_TextureAssetEvent just_engine_events_iter_texture_asset_events_new_from_start(Events_TextureAssetEvent* events);
bool just_engine_events_iter_texture_asset_events_has_next(EventsIter_TextureAssetEvent* iter);
TextureAssetEvent just_engine_events_iter_texture_asset_events_read_next(EventsIter_TextureAssetEvent* iter);
TextureAssetEvent just_engine_events_iter_texture_asset_events_consume_next(EventsIter_TextureAssetEvent* iter);




typedef struct {
    Vector2 position;
    Vector2 direction;
} Ray2;

typedef struct {
    Ray2 line;
    float32 length;
} LineSegment2;

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

float32 just_engine_collider_dist_circle_circle(CircleCollider c1, CircleCollider c2);
bool just_engine_check_collision_circle_circle(CircleCollider c1, CircleCollider c2);
bool just_engine_check_collision_aabb_aabb(AABBCollider r1, AABBCollider r2);

bool just_engine_check_rayhit_circle(Ray2 ray, CircleCollider c1, float32 max_dist);