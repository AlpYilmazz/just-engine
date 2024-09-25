#include <stdlib.h>
#include <assert.h>

#include "raylib.h"
#include "raymath.h"

#include "base.h"
#include "logging.h"

#include "sprite.h"

SpriteComponentId spawn_sprite(
    SpriteStore* sprite_store,
    SpriteTransform transform,
    Sprite sprite
) {
    uint32 entity_id;

    #define INITIAL_CAPACITY 100
    #define GROWTH_FACTOR 2

    if (sprite_store->capacity == 0) {
        sprite_store->capacity = INITIAL_CAPACITY;

        sprite_store->slot_occupied = malloc(sprite_store->capacity * sizeof(bool));
        sprite_store->transforms = malloc(sprite_store->capacity * sizeof(SpriteTransform));
        sprite_store->sprites = malloc(sprite_store->capacity * sizeof(Sprite));
    }
    else if (sprite_store->count == sprite_store->capacity) {
        if (sprite_store->free_count > 0) {
            for (uint32 i = 0; i < sprite_store->count; i++) {
                if (!sprite_store->slot_occupied[i]) {
                    sprite_store->free_count--;
                    entity_id = i;
                    goto SET_ENTITY_AND_RETURN;
                }
            }
        }

        sprite_store->capacity = GROWTH_FACTOR * sprite_store->capacity;

        sprite_store->slot_occupied = realloc(sprite_store->slot_occupied, sprite_store->capacity * sizeof(bool));
        sprite_store->transforms = realloc(sprite_store->transforms, sprite_store->capacity * sizeof(SpriteTransform));
        sprite_store->sprites = realloc(sprite_store->sprites, sprite_store->capacity * sizeof(Sprite));
    }

    entity_id = sprite_store->count;
    sprite_store->count++;

    #undef INITIAL_CAPACITY
    #undef GROWTH_FACTOR

    SET_ENTITY_AND_RETURN:
    sprite_store->slot_occupied[entity_id] = true;
    sprite_store->sprites[entity_id] = sprite;
    sprite_store->transforms[entity_id] = transform;
    return new_component_id(entity_id);
}

void despawn_sprite(SpriteStore* sprite_store, SpriteComponentId cid) {
    sprite_store->slot_occupied[cid.id] = false;
    sprite_store->free_count++;
}

void render_sprites_reserve_total(SortedRenderSprites* render_sprites, uint32 capacity) {
    if (render_sprites->capacity < capacity) {
        // TODO: init with initial capacity instead of checking if uninit
        if (render_sprites->capacity > 0) {
            free(render_sprites->sprites);
        }
        render_sprites->sprites = malloc(capacity * sizeof(RenderSprite));
    }
}

void render_sprites_push_back_unchecked(SortedRenderSprites* render_sprites, RenderSprite sprite) {
    render_sprites->sprites[render_sprites->count] = sprite;
    render_sprites->count++;
}

void render_sprites_push_back(SortedRenderSprites* render_sprites, RenderSprite render_sprite) {
    #define INITIAL_CAPACITY 100
    #define GROWTH_FACTOR 2

    if (render_sprites->capacity == 0) {
        render_sprites->capacity = INITIAL_CAPACITY;
        render_sprites->sprites = malloc(render_sprites->capacity * sizeof(RenderSprite));
    }
    else if (render_sprites->count == render_sprites->capacity) {
        render_sprites->capacity = GROWTH_FACTOR * render_sprites->capacity;
        render_sprites->sprites = realloc(render_sprites->sprites, render_sprites->capacity * sizeof(RenderSprite));
    }

    render_sprites->sprites[render_sprites->count] = render_sprite;
    render_sprites->count++;

    #undef INITIAL_CAPACITY
    #undef GROWTH_FACTOR
}

// TODO: impl n*logn sort and maybe use pointers to avoid many clones
void render_sprites_z_index_sort(SortedRenderSprites* render_sprites) {
    if (render_sprites->count == 0) {
        return;
    }

    RenderSprite temp;
    bool no_swap_done = true;
    for (uint32 begin = 0; begin < render_sprites->count - 1; begin++) {
        no_swap_done = true;
        for (uint32 i = render_sprites->count - 1; i > begin; i--) {
            if (render_sprites->sprites[i].z_index < render_sprites->sprites[i-1].z_index) {
                // swap
                temp = render_sprites->sprites[i];
                render_sprites->sprites[i] = render_sprites->sprites[i-1];
                render_sprites->sprites[i-1] = temp;
                no_swap_done = false;
            }
        }
        if (no_swap_done) {
            break;
        }
    }
}

RenderSprite extract_render_sprite(SpriteTransform* transform, Sprite* sprite) {
    return (RenderSprite) {
        .texture = sprite->texture,
        .tint = sprite->tint,
        .use_custom_source = sprite->use_custom_source,
        .source = sprite->source,
        .transform = *transform,
        .z_index = sprite->z_index,
    };
}

static void insert_sorted(CameraSortElem* arr, uint32 count, CameraSortElem value) {
    for (uint32 i = 0; i < count; i++) {
        if (arr[i].sort_index > value.sort_index) {
            for (uint32 j = count+1; j > i; j--) {
                arr[j] = arr[j-1];
            }
            arr[i] = value;
        }
    }
    arr[count] = value;
} 

void SYSTEM_EXTRACT_RENDER_cull_and_sort_sprites(
    SpriteCameraStore* sprite_camera_store,
    SpriteStore* sprite_store,
    PreparedRenderSprites* prepared_render_sprites
) {
    prepared_render_sprites->camera_count = sprite_camera_store->count;
    JUST_LOG_DEBUG("camera count: %d\n", sprite_camera_store->count);

    SpriteTransform* transform;
    Sprite* sprite;
    for (uint32 camera_i = 0; camera_i < sprite_camera_store->count; camera_i++) {
        SpriteCamera* sprite_camera = &sprite_camera_store->cameras[camera_i];
        SortedRenderSprites* render_sprites = &prepared_render_sprites->render_sprites[camera_i];

        JUST_LOG_DEBUG("camera %d\n", camera_i);

        insert_sorted(
            prepared_render_sprites->camera_sort,
            camera_i,
            (CameraSortElem) {
                .camera_index = camera_i,
                .sort_index =  sprite_camera->sort_index,
            }
        );
        JUST_LOG_DEBUG("camera_inserted_for_sort\n");

        for (uint32 i = 0; i < sprite_store->count; i++) {
            transform = &sprite_store->transforms[i];
            sprite = &sprite_store->sprites[i];

            JUST_LOG_DEBUG("check sprite %d\n", i);
            if (sprite->visible && sprite->camera_visible) {
                if (
                    (!sprite->use_layer_system && camera_i == PRIMARY_CAMERA_ID)
                    || (sprite->use_layer_system && layers_overlap(sprite->layers, sprite_camera->layers))
                ) {
                    JUST_LOG_DEBUG("prepared sprite %d\n", i);
                    render_sprites_push_back(
                        render_sprites,
                        extract_render_sprite(transform, sprite)
                    );
                }
            }
        }
        JUST_LOG_DEBUG("start render_sprites_z_index_sort\n");
        JUST_LOG_DEBUG("render_sprites: %d, %d\n", render_sprites->count, render_sprites->capacity);
        render_sprites_z_index_sort(render_sprites);
        JUST_LOG_DEBUG("end render_sprites_z_index_sort\n");
    }
}

void SYSTEM_RENDER_sorted_sprites(
    TextureAssets* RES_texture_assets,
    SpriteCameraStore* sprite_camera_store,
    PreparedRenderSprites* prepared_render_sprites
) {
    RenderSprite sprite;
    Texture* texture;
    SpriteTransform transform;
    Rectangle source;
    Vector2 size;
    Rectangle destination;
    Vector2 origin;
    float32 rotation;

    for (uint32 ci = 0; ci < sprite_camera_store->count; ci++) {
        uint32 camera_i = prepared_render_sprites->camera_sort[ci].camera_index;

        SpriteCamera* sprite_camera = &sprite_camera_store->cameras[camera_i];
        SortedRenderSprites* render_sprites = &prepared_render_sprites->render_sprites[camera_i];

        BeginMode2D(sprite_camera->camera);

        for (uint32 i = 0; i < render_sprites->count; i++) {
            sprite = render_sprites->sprites[i];
            texture = just_engine_texture_assets_get_texture_or_default(RES_texture_assets, sprite.texture);
            transform = sprite.transform;
            source = sprite.use_custom_source
                ? sprite.source
                : (Rectangle) {0, 0, texture->width, texture->height};
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
                source,
                destination,
                origin,
                rotation,
                sprite.tint
            );
        }
        render_sprites->count = 0; // RESET

        EndMode2D();
    }
}