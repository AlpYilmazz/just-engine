#pragma once

#include "raylib.h"

#include "base.h"
#include "assets/asset.h"

#include "camera2d.h"

typedef EntityId SpriteEntityId;

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

// typedef struct {
//     TextureHandle texture;
//     Color tint;
//     bool use_custom_source;
//     Rectangle source;
//     bool flip_x;
//     bool flip_y;
//     SpriteTransform transform;
//     uint32 z_index;
// } RenderSprite;

typedef struct {
    SpriteEntityId sprite_entity;
    uint32 z_index;
} RenderSprite;

typedef struct {
    usize count;
    usize capacity;
    RenderSprite* sprites;
} SortedRenderSprites;

typedef struct {
    uint32 camera_index;
    uint32 sort_index;
} CameraSortElem;

typedef struct {
    usize count; // camera_count
    usize capacity;
    CameraSortElem* camera_render_order;
    SortedRenderSprites* render_sprites; // out of order, render based on .camera_render_order
} PreparedRenderSprites;

typedef struct {
    usize count;
    usize capacity;
    usize free_count;
    bool* slot_occupied;
    usize* generations;
    SpriteTransform* transforms;
    Sprite* sprites;
} SpriteStore;

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

void SYSTEM_RENDER_render2d_render_sprites(
    TextureAssets* RES_texture_assets,
    SpriteStore* RES_sprite_store,
    SpriteCameraStore* RES_sprite_camera_store,
    PreparedRenderSprites* RENDER_RES_prepared_render_sprites
);
