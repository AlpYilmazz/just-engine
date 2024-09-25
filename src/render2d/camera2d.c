#include "assert.h"

#include "camera2d.h"

void set_primary_camera(SpriteCameraStore* store, SpriteCamera camera) {
    store->cameras[0] = camera;
    store->count = MAX(1, store->count);
}

SpriteCamera* get_primary_camera(SpriteCameraStore* store) {
    return &store->cameras[0]; 
}

void add_camera(SpriteCameraStore* store, SpriteCamera camera) {
    assert(store->count < MAX_CAMERA_COUNT);
    store->cameras[store->count] = camera;
    store->count++;
}