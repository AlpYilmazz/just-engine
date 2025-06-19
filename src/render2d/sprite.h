#pragma once

#include "raylib.h"

#include "base.h"
#include "assets/asset.h"

#include "camera2d.h"

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