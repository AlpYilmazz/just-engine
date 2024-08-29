#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "raylib.h"
#include "raymath.h"

#include "base.h"
#include "assets/asset.h"
#include "events/events.h"


// TODO:
//  - Game Base Thread
//      - runs game logic
//  - Render Base Thread (Main thread)
//      - runs render logic
//      - gpu bus io
//      - draw calls

/**
 * -- STAGES --
 * 
 * PREPARE
 *      PRE_PREPARE
 *      PREAPRE
 *      POST_PREPARE
 * 
 * UPDATE
 *      PRE_UPDATE
 *      UPDATE
 *      POST_UPDATE
 * 
 * RENDER
 *      QUEUE_RENDER
 *      EXTRACT_RENDER
 *      RENDER
 * 
 * FRAME_BOUNDARY
 * 
 */

/**
 * POST_UPDATE
 */
void SYSTEM_check_mutated_images(
    TextureAssets* RES_texture_assets,
    Events_TextureAssetEvent* RES_texture_asset_events
) {
    TextureAssetEvent event;
    for (uint32 i = 0; i < TEXTURE_SLOTS; i++) {
        if (RES_texture_assets->image_changed[i]) {
            RES_texture_assets->image_changed[i] = false;
            event = (TextureAssetEvent) {
                .handle = new_texture_handle(i),
                .type = AssetEvent_Changed,
                .consumed = false,
            };
            just_engine_events_texture_asset_event_send_single(RES_texture_asset_events, event);
        }
    }
}

// --------------------------------------------------
/**
 * EXTRACT_RENDER
 */
void SYSTEM_load_textures_for_loaded_or_changed_images(
    TextureAssets* RES_texture_assets,
    Events_TextureAssetEvent* RES_texture_asset_events
) {
    static uint32 LOCAL_image_loaded_events_offset = 0;

    printf("LOCAL_image_loaded_events_offset: %d\n", LOCAL_image_loaded_events_offset);
    EventsIter_TextureAssetEvent events_iter = just_engine_events_iter_texture_asset_events_begin_iter(RES_texture_asset_events, LOCAL_image_loaded_events_offset);
    printf("1\n");

    while (just_engine_events_iter_texture_asset_events_has_next(&events_iter)) {
        printf("__iter\n");
        TextureAssetEvent event = just_engine_events_iter_texture_asset_events_read_next(&events_iter);
        switch (event.type) {
        case AssetEvent_Changed:
            just_engine_texture_assets_unload_texture(RES_texture_assets, event.handle);
            // fallthrough
        case AssetEvent_Loaded:
            just_engine_texture_assets_load_texture_uncheched(RES_texture_assets, event.handle);
        }
    }
    printf("2\n");

    LOCAL_image_loaded_events_offset = just_engine_events_iter_texture_asset_events_end_iter(&events_iter);
    
    printf("3\n");
}

/**
 * FRAME_BOUNDARY
 */
void SYSTEM_swap_event_buffers(
    Events_TextureAssetEvent* RES_texture_asset_events
) {
    just_engine_events_texture_asset_event_swap_buffers(RES_texture_asset_events);
}

typedef enum {
    Anchor_Top_Left, // DEFAULT
    Anchor_Top_Right,
    Anchor_Bottom_Left,
    Anchor_Bottom_Right,

    Anchor_Top_Mid,
    Anchor_Bottom_Mid,
    Anchor_Left_Mid,
    Anchor_Right_Mid,

    Anchor_Center,

    Anchor_Custom_UV,       // uv coordinate: [0.0, 1.0]
} AnchorType;

typedef struct {
    AnchorType type;
    Vector2 origin;         // uv coordinate: [0.0, 1.0]
} Anchor;

typedef enum {
    Rotation_CW = 1,
    Rotation_CCW = -1,
} RotationWay;

typedef struct {
    Anchor anchor;
    Vector2 position;       // position of the anchor
    Vector2 size;
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
    // SpriteTransform transform;
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

typedef struct {
    uint32 id;
} EntityId;

EntityId new_entity_id(uint32 id) {
    return (EntityId) { id };
}

EntityId spawn_sprite(
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
    return new_entity_id(entity_id);
}

void despawn_sprite(SpriteStore* sprite_store, EntityId entity) {
    sprite_store->slots[entity.id] = false;
    sprite_store->free_count++;
}

Vector2 calculate_relative_origin_from_anchor(Anchor anchor, Vector2 size) {
    switch (anchor.type) {
    default:
    case Anchor_Top_Left:
        return (Vector2) {
            .x = 0.0,
            .y = 0.0,
        };
    case Anchor_Top_Right:
        return (Vector2) {
            .x = size.x,
            .y = 0.0,
        };
    case Anchor_Bottom_Left:
        return (Vector2) {
            .x = 0.0,
            .y = size.y,
        };
    case Anchor_Bottom_Right:
        return (Vector2) {
            .x = size.x,
            .y = size.y,
        };

    case Anchor_Top_Mid:
        return (Vector2) {
            .x = size.x/2.0,
            .y = 0.0,
        };
    case Anchor_Bottom_Mid:
        return (Vector2) {
            .x = size.x/2.0,
            .y = size.y,
        };
    case Anchor_Left_Mid:
        return (Vector2) {
            .x = 0.0,
            .y = size.y/2.0,
        };
    case Anchor_Right_Mid:
        return (Vector2) {
            .x = size.x,
            .y = size.y/2.0,
        };

    case Anchor_Center:
        return (Vector2) {
            .x = size.x/2.0,
            .y = size.y/2.0,
        };

    case Anchor_Custom_UV:
        return (Vector2) {
            .x = anchor.origin.x * size.x,
            .y = anchor.origin.y * size.y,
        };
    }
    assert(false); // Non-exhaustive switch
    return (Vector2) {0};
}

// void queue_sprite_for_render(SpriteStore* sprite_list, Sprite sprite) {
//     #define INITIAL_CAPACITY 32
//     #define GROWTH_FACTOR 2

//     if (sprite_list->capacity == 0) {
//         sprite_list->capacity = INITIAL_CAPACITY;
//         sprite_list->sprites = malloc(sprite_list->capacity * sizeof(*&sprite));
//     }
//     else if (sprite_list->count == sprite_list->capacity) {
//         sprite_list->capacity = GROWTH_FACTOR * sprite_list->capacity;
//         sprite_list->sprites = realloc(sprite_list->sprites, sprite_list->capacity * sizeof(*&sprite));
//     }

//     sprite_list->sprites[sprite_list->count] = sprite;
//     sprite_list->count++;

//     #undef INITIAL_CAPACITY
//     #undef GROWTH_FACTOR
// }

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
    for (uint32 begin = 0; begin < render_sprites->count - 1; begin++) {
        for (uint32 i = render_sprites->count - 1; i > begin; i--) {
            if (render_sprites->sprites[i].z_index < render_sprites->sprites[i-1].z_index) {
                // swap
                temp = render_sprites->sprites[i];
                render_sprites->sprites[i] = render_sprites->sprites[i-1];
                render_sprites->sprites[i-1] = temp;
            }
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

void SYSTEM_cull_and_sort_sprites(
    SpriteStore* sprite_store,
    SortedRenderSprites* render_sprites
) {
    render_sprites_reserve_total(render_sprites, sprite_store->count);
    
    SpriteTransform* transform;
    Sprite* sprite;
    for (uint32 i = 0; i < sprite_store->count; i++) {
        transform = &sprite_store->transforms[i];
        sprite = &sprite_store->sprites[i];

        if (sprite->visible && sprite->camera_visible) {
            render_sprites_push_back_unchecked(
                render_sprites,
                extract_render_sprite(transform, sprite)
            );
        }
    }

    render_sprites_z_index_sort(render_sprites);
}

void RENDER_sorted_sprites(
    TextureAssets* RES_texture_assets,
    SortedRenderSprites* render_sprites
) {
    RenderSprite sprite;
    Texture* texture;
    SpriteTransform transform;
    Rectangle destination;
    Vector2 origin;
    float32 rotation;
    for (uint32 i = 0; i < render_sprites->count; i++) {
        sprite = render_sprites->sprites[i];
        texture = just_engine_texture_assets_get_texture_or_default(RES_texture_assets, sprite.texture);
        transform = sprite.transform;
        destination = (Rectangle) {
            .x = transform.position.x,
            .y = transform.position.y,
            .width = transform.size.x,
            .height = transform.size.y,
        };
        origin = calculate_relative_origin_from_anchor(transform.anchor, transform.size);
        rotation = transform.rotation * transform.rway;

        DrawTexturePro(
            *texture,
            sprite.source,
            destination,
            origin,
            rotation,
            sprite.tint
        );
    }
    render_sprites->count = 0; // RESET
}


// Rectangle calculate_rectangle_from_anchor(Anchor anchor, Vector2 position, Vector2 size) {
//     switch (anchor.type) {
//     default:
//     case Anchor_Top_Left:
//         return (Rectangle) {
//             .x = position.x,
//             .y = position.y,
//             .width = size.x,
//             .height = size.y,
//         };
//     case Anchor_Center:
//         return (Rectangle) {
//             .x = position.x - size.x/2.0,
//             .y = position.y - size.y/2.0,
//             .width = size.x,
//             .height = size.y,
//         };
//     case Anchor_Custom_UV:
//         return (Rectangle) {
//             .x = position.x - (anchor.origin.x * size.x),
//             .y = position.y - (anchor.origin.y * size.y),
//             .width = size.x,
//             .height = size.y,
//         };
//     }
//     assert(false); // Non-exhaustive switch
//     return (Rectangle) {0};
// }
