#include <assert.h>

#include "raylib.h"
#include "raymath.h"

#include "justcstd.h"
#include "base.h"
#include "logging.h"
#include "memory/memory.h"

#include "sprite.h"

SpriteEntityId spawn_sprite(
    SpriteStore* sprite_store,
    SpriteTransform transform,
    Sprite sprite
) {
    usize sprite_id;

    #define INITIAL_CAPACITY 100
    #define GROWTH_FACTOR 2

    usize old_capacity = sprite_store->capacity;

    if (sprite_store->capacity == 0) {
        sprite_store->capacity = INITIAL_CAPACITY;

        sprite_store->slot_occupied = std_malloc(sprite_store->capacity * sizeof(bool));
        sprite_store->generations = std_malloc(sprite_store->capacity * sizeof(usize));
        sprite_store->transforms = std_malloc(sprite_store->capacity * sizeof(SpriteTransform));
        sprite_store->sprites = std_malloc(sprite_store->capacity * sizeof(Sprite));
    }
    else if (sprite_store->count == sprite_store->capacity) {
        if (sprite_store->free_count > 0) {
            for (usize i = 0; i < sprite_store->count; i++) {
                if (!sprite_store->slot_occupied[i]) {
                    sprite_store->free_count--;
                    sprite_id = i;
                    goto SET_ENTITY_AND_RETURN;
                }
            }
        }

        sprite_store->capacity = GROWTH_FACTOR * sprite_store->capacity;

        sprite_store->slot_occupied = std_realloc(sprite_store->slot_occupied, sprite_store->capacity * sizeof(bool));
        sprite_store->generations = std_realloc(sprite_store->generations, sprite_store->capacity * sizeof(usize));
        sprite_store->transforms = std_realloc(sprite_store->transforms, sprite_store->capacity * sizeof(SpriteTransform));
        sprite_store->sprites = std_realloc(sprite_store->sprites, sprite_store->capacity * sizeof(Sprite));
    }

    for (usize i = old_capacity; i < sprite_store->capacity; i++) {
        sprite_store->slot_occupied[i] = false;
        sprite_store->generations[i] = 0;
    }

    sprite_id = sprite_store->count;
    sprite_store->count++;

    #undef INITIAL_CAPACITY
    #undef GROWTH_FACTOR

    SET_ENTITY_AND_RETURN:
    sprite_store->slot_occupied[sprite_id] = true;
    const usize generation = sprite_store->generations[sprite_id];
    sprite_store->sprites[sprite_id] = sprite;
    sprite_store->transforms[sprite_id] = transform;
    return new_entity_id(sprite_id, generation);
}

void despawn_sprite(SpriteStore* sprite_store, SpriteEntityId sprite_id) {
    sprite_store->generations[sprite_id.id]++;
    sprite_store->slot_occupied[sprite_id.id] = false;
    sprite_store->free_count++;
}

bool sprite_is_valid(SpriteStore* sprite_store, SpriteEntityId sprite_id) {
    return sprite_id.id < sprite_store->count
        && sprite_store->slot_occupied[sprite_id.id]
        && sprite_id.generation == sprite_store->generations[sprite_id.id];
}

void destroy_sprite_store(SpriteStore* sprite_store) {
    std_free(sprite_store->slot_occupied);
    std_free(sprite_store->generations);
    std_free(sprite_store->transforms);
    std_free(sprite_store->sprites);
}

// TODO: unused
bool render_sprites_check_validity(SpriteStore* sprite_store, SortedRenderSprites render_sprites, usize sprite_count) {
    // count check
    if (render_sprites.count != sprite_count) {
        return false;
    }

    // generation check
    for (usize i = 0; i < render_sprites.count; i++) {
        bool valid = sprite_is_valid(sprite_store, render_sprites.sprites[i-1].sprite_entity);
        if (!valid) {
            return false;
        }
    }

    // sorted check
    for (usize i = 1; i < render_sprites.count; i++) {
        uint32 z_index_0 = sprite_store->sprites[render_sprites.sprites[i-1].sprite_entity.id].z_index;
        uint32 z_index_1 = sprite_store->sprites[render_sprites.sprites[i].sprite_entity.id].z_index;
        if (z_index_0 > z_index_1) {
            return false;
        }
    }

    return true;
}

// TODO: impl n*logn sort
void render_sprites_z_index_sort(SortedRenderSprites* render_sprites) {
    if (render_sprites->count == 0) {
        return;
    }

    RenderSprite temp;
    bool no_swap_done = true;
    for (usize begin = 0; begin < render_sprites->count - 1; begin++) {
        no_swap_done = true;
        for (usize i = render_sprites->count - 1; i > begin; i--) {
            if (render_sprites->sprites[i].z_index < render_sprites->sprites[i-1].z_index) {
                // swap
                temp = render_sprites->sprites[i];
                render_sprites->sprites[i] = render_sprites->sprites[i-1];
                render_sprites->sprites[i-1] = temp;
                no_swap_done = false;
            }
            if (i == begin+1) {
                break;
            }
        }
        if (no_swap_done) {
            break;
        }
    }
}

RenderSprite extract_render_sprite(SpriteStore* sprite_store, SpriteEntityId sprite_entity) {
    Sprite* sprite = &sprite_store->sprites[sprite_entity.id];
    return (RenderSprite) {
        .sprite_entity = sprite_entity,
        .z_index = sprite->z_index,
    };
}

static void camera_insert_sorted(CameraSortElem* arr, usize count, CameraSortElem value) {
    for (usize i = 0; i < count; i++) {
        if (arr[i].sort_index > value.sort_index) {
            for (usize j = count+1; j > i; j--) {
                arr[j] = arr[j-1];
            }
            arr[i] = value;
            return;
        }
    }
    arr[count] = value;
}

void SYSTEM_EXTRACT_RENDER_cull_and_sort_sprites(
    SpriteCameraStore* sprite_camera_store,
    SpriteStore* sprite_store,
    PreparedRenderSprites* prepared_render_sprites
) {
    dynarray_reserve_custom_2(*prepared_render_sprites, .camera_render_order, .render_sprites, sprite_camera_store->count);
    prepared_render_sprites->count = sprite_camera_store->count;

    for (usize camera_i = 0; camera_i < sprite_camera_store->count; camera_i++) {
        SpriteCamera* sprite_camera = &sprite_camera_store->cameras[camera_i];
        SortedRenderSprites* render_sprites = &prepared_render_sprites->render_sprites[camera_i];
        *render_sprites = (SortedRenderSprites){0};

        camera_insert_sorted(
            prepared_render_sprites->camera_render_order,
            camera_i,
            (CameraSortElem) {
                .camera_index = camera_i,
                .sort_index =  sprite_camera->sort_index,
            }
        );

        for (uint32 i = 0; i < sprite_store->count; i++) {
            SpriteEntityId sprite_entity = {
                .id = i,
                .generation = sprite_store->generations[i],
            };
            
            bool valid = sprite_is_valid(sprite_store, sprite_entity);
            if (valid) {
                Sprite* sprite = &sprite_store->sprites[i];
                if (sprite->visible && sprite->camera_visible) {
                    if (
                        (!sprite->use_layer_system && camera_i == PRIMARY_CAMERA_ID)
                        || (sprite->use_layer_system && check_layer_overlap(sprite->layers, sprite_camera->layers))
                    ) {
                        RenderSprite render_sprite = extract_render_sprite(sprite_store, sprite_entity);
                        dynarray_push_back_custom(*render_sprites, .sprites, render_sprite);
                    }
                }
            }
        }
        render_sprites_z_index_sort(render_sprites);
    }
}

static SpriteCamera* RENDER2D_CURRENT_CAMERA = NULL;

void render2d_begin_camera_render(SpriteCamera* sprite_camera) {
    RENDER2D_CURRENT_CAMERA = sprite_camera;

    switch (RENDER2D_CURRENT_CAMERA->target.type) {
    case RENDER_TARGET_WINDOW:
        BeginDrawing();
        break;
    case RENDER_TARGET_TEXTURE:
        BeginTextureMode(RENDER2D_CURRENT_CAMERA->target.texture_target.texture);
        break;
    default:
        PANIC("Unsupported RenderTargetType.");
    }
    ClearBackground(RENDER2D_CURRENT_CAMERA->target.clear_color);
    BeginMode2D(RENDER2D_CURRENT_CAMERA->camera);
}

void render2d_end_camera_render() {
    EndMode2D();
    switch (RENDER2D_CURRENT_CAMERA->target.type) {
    case RENDER_TARGET_WINDOW:
        EndDrawing();
        break;
    case RENDER_TARGET_TEXTURE:
        EndTextureMode();
        break;
    default:
        PANIC("Unsupported RenderTargetType.");
    }

    RENDER2D_CURRENT_CAMERA = NULL;
}

void render2d_render_sprites(
    TextureAssets* RES_texture_assets,
    SpriteStore* RES_sprite_store,
    SortedRenderSprites* render_sprites
) {
    RenderSprite render_sprite;
    Sprite sprite;
    SpriteTransform transform;
    Texture* texture;
    Rectangle source;
    Rectangle source_flip;
    Vector2 size;
    Rectangle destination;
    Vector2 origin;
    float32 rotation;

    for (usize i = 0; i < render_sprites->count; i++) {
        render_sprite = render_sprites->sprites[i];
        sprite = RES_sprite_store->sprites[render_sprite.sprite_entity.id];
        transform = RES_sprite_store->transforms[render_sprite.sprite_entity.id];

        texture = texture_assets_get_texture_or_default(RES_texture_assets, sprite.texture);
        source = sprite.use_custom_source
            ? sprite.source
            : (Rectangle) {0, 0, texture->width, texture->height};
        source_flip = source;
        source_flip.width *= sprite.flip_x ? -1 : 1;
        source_flip.height *= sprite.flip_y ? -1 : 1;
        size = transform.use_source_size
            ? (Vector2) {source.width, source.height}
            : transform.size;
        size = Vector2Multiply(size, transform.scale);
        destination = (Rectangle) {
            .x = transform.position.x,
            .y = transform.position.y,
            .width = size.x,
            .height = size.y,
        };
        origin = Vector2Multiply(transform.anchor.origin, size);
        rotation = transform.rotation * transform.rway;

        DrawTexturePro(
            *texture,
            source_flip,
            destination,
            origin,
            rotation,
            sprite.tint
        );
    }
    // render_sprites->count = 0; // RESET
}

void SYSTEM_RENDER_render2d_render_sprites(
    TextureAssets* RES_texture_assets,
    SpriteStore* RES_sprite_store,
    SpriteCameraStore* RES_sprite_camera_store,
    PreparedRenderSprites* RENDER_RES_prepared_render_sprites
) {
    for (uint32 ci = 0; ci < RENDER_RES_prepared_render_sprites->count; ci++) {
        uint32 camera_i = RENDER_RES_prepared_render_sprites->camera_render_order[ci].camera_index;

        SpriteCamera* sprite_camera = &RES_sprite_camera_store->cameras[camera_i];
        SortedRenderSprites* render_sprites = &RENDER_RES_prepared_render_sprites->render_sprites[camera_i];

        render2d_begin_camera_render(sprite_camera);
            render2d_render_sprites(RES_texture_assets, RES_sprite_store, render_sprites);
        render2d_end_camera_render();
    }
}