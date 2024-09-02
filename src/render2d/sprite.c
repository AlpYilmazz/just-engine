#include <stdlib.h>
#include <assert.h>

#include "raylib.h"
#include "raymath.h"

#include "base.h"

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

        sprite_store->slots = malloc(sprite_store->capacity * sizeof(bool));
        sprite_store->transforms = malloc(sprite_store->capacity * sizeof(SpriteTransform));
        sprite_store->sprites = malloc(sprite_store->capacity * sizeof(Sprite));
    }
    else if (sprite_store->count == sprite_store->capacity) {
        if (sprite_store->free_count > 0) {
            for (uint32 i = 0; i < sprite_store->count; i++) {
                if (!sprite_store->slots[i]) {
                    sprite_store->free_count--;
                    entity_id = i;
                    goto SET_ENTITY_AND_RETURN;
                }
            }
        }

        sprite_store->capacity = GROWTH_FACTOR * sprite_store->capacity;

        sprite_store->slots = realloc(sprite_store->slots, sprite_store->capacity * sizeof(bool));
        sprite_store->transforms = realloc(sprite_store->transforms, sprite_store->capacity * sizeof(SpriteTransform));
        sprite_store->sprites = realloc(sprite_store->sprites, sprite_store->capacity * sizeof(Sprite));
    }

    entity_id = sprite_store->count;
    sprite_store->count++;

    #undef INITIAL_CAPACITY
    #undef GROWTH_FACTOR

    SET_ENTITY_AND_RETURN:
    sprite_store->slots[entity_id] = true;
    sprite_store->sprites[entity_id] = sprite;
    sprite_store->transforms[entity_id] = transform;
    return new_component_id(entity_id);
}

void despawn_sprite(SpriteStore* sprite_store, SpriteComponentId cid) {
    sprite_store->slots[cid.id] = false;
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

// TODO: impl n*logn sort and maybe use pointers to avoid many clones
void render_sprites_z_index_sort(SortedRenderSprites* render_sprites) {
    RenderSprite temp;
    bool no_swap_done = true;
    for (uint32 begin = 0; begin < render_sprites->count - 1; begin++) {
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
        .source = sprite->source,
        .transform = *transform,
        .z_index = sprite->z_index,
    };
}
