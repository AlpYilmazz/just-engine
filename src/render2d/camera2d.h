#pragma once

#include "raylib.h"

#include "base.h"

#define PRIMARY_CAMERA_ID 0

typedef enum {
    RENDER_TARGET_WINDOW = 0,
    RENDER_TARGET_TEXTURE,
} RenderTargetType;

// Unused in raylib which supports single window
typedef struct {
    usize window_id;
} RenderTargetWindow;

typedef struct {
    URectSize texture_size;
    RenderTexture texture;
} RenderTargetTexture;

typedef struct {
    RenderTargetType type;
    union {
        RenderTargetWindow window_target;
        RenderTargetTexture texture_target;
    };
    // -- Common
    Color clear_color;
} RenderTarget;

typedef struct {
    Camera2D camera;
    RenderTarget target;
    Layers layers;
    uint32 sort_index;
} SpriteCamera;

typedef struct {
    usize count;
    usize capacity;
    SpriteCamera* cameras;
} SpriteCameraStore;

void set_primary_camera(SpriteCameraStore* store, SpriteCamera camera);
SpriteCamera* get_primary_camera(SpriteCameraStore* store);
void add_camera(SpriteCameraStore* store, SpriteCamera camera);
