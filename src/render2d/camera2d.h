#pragma once

#include "raylib.h"

#include "base.h"

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
