#pragma once

#include "raylib.h"

#include "base.h"
#include "assets/asset.h"

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
    TextureHandle texture;
    Color tint;
    Rectangle source;
    SpriteTransform transform;
    uint32 z_index;
} RenderSprite;

typedef struct {
    uint32 count;
    uint32 capacity;
    RenderSprite* sprites;
} SortedRenderSprites;

typedef struct {
    // -- render start
    TextureHandle texture;
    Color tint;
    Rectangle source;
    uint32 z_index;
    // -- render end
    bool visible;
    bool camera_visible;
} Sprite;

typedef struct {
    uint32 count;
    uint32 capacity;
    uint32 free_count;
    bool* slots;
    SpriteTransform* transforms;
    Sprite* sprites;
} SpriteStore;

typedef ComponentId SpriteComponentId;

SpriteComponentId spawn_sprite(
    SpriteStore* sprite_store,
    SpriteTransform transform,
    Sprite sprite
);
void despawn_sprite(SpriteStore* sprite_store, SpriteComponentId id);

Vector2 calculate_relative_origin_from_anchor(Anchor anchor, Vector2 size);

void render_sprites_reserve_total(SortedRenderSprites* render_sprites, uint32 capacity);
void render_sprites_push_back_unchecked(SortedRenderSprites* render_sprites, RenderSprite sprite);

// TODO: impl n*logn sort and maybe use pointers to avoid many clones
void render_sprites_z_index_sort(SortedRenderSprites* render_sprites);
RenderSprite extract_render_sprite(SpriteTransform* transform, Sprite* sprite);