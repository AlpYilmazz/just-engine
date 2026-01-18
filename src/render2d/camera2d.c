
#include "memory/memory.h"

#include "camera2d.h"

void set_primary_camera(SpriteCameraStore* store, SpriteCamera camera) {
    dynarray_reserve_custom(*store, .cameras, 1);
    store->cameras[PRIMARY_CAMERA_ID] = camera;
    store->count = MAX(1, store->count);
}

SpriteCamera* get_primary_camera(SpriteCameraStore* store) {
    return &store->cameras[PRIMARY_CAMERA_ID]; 
}

void add_camera(SpriteCameraStore* store, SpriteCamera camera) {
    dynarray_push_back_custom(*store, .cameras, camera);
}