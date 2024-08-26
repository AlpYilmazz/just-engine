#include <stdlib.h>
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
static uint32 LOCAL_image_loaded_events_offset = 0;
/**
 * EXTRACT_RENDER
 */
void SYSTEM_load_textures_for_loaded_or_changed_images(
    TextureAssets* RES_texture_assets,
    Events_TextureAssetEvent* RES_texture_asset_events
) {
    EventsIter_TextureAssetEvent events_iter = just_engine_events_iter_texture_asset_events_begin_iter(RES_texture_asset_events, LOCAL_image_loaded_events_offset);

    while (just_engine_events_iter_texture_asset_events_has_next(&events_iter)) {
        TextureAssetEvent event = just_engine_events_iter_texture_asset_events_read_next(&events_iter);
        switch (event.type) {
        case AssetEvent_Changed:
            just_engine_texture_assets_unload_texture(RES_texture_assets, event.handle);
            // fallthrough
        case AssetEvent_Loaded:
            just_engine_texture_assets_load_texture_uncheched(RES_texture_assets, event.handle);
        }
    }

    LOCAL_image_loaded_events_offset = just_engine_events_iter_texture_asset_events_end_iter(&events_iter);
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
    SpriteTransform transform;
    uint32 z_index;
    // -- render end
    bool visible;
    bool camera_visible;
} Sprite;

typedef struct {
    uint32 count;
    uint32 capacity;
    Sprite* sprites;
} SpriteList;

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

void queue_sprite_for_render(SpriteList* sprite_list, Sprite sprite) {
    #define INITIAL_CAPACITY 32
    #define GROWTH_FACTOR 2

    if (sprite_list->capacity == 0) {
        sprite_list->capacity = INITIAL_CAPACITY;
        sprite_list->sprites = malloc(sprite_list->capacity * sizeof(*&sprite));
    }
    else if (sprite_list->count == sprite_list->capacity) {
        sprite_list->capacity = GROWTH_FACTOR * sprite_list->capacity;
        sprite_list->sprites = realloc(sprite_list->sprites, sprite_list->capacity * sizeof(*&sprite));
    }

    sprite_list->sprites[sprite_list->count] = sprite;
    sprite_list->count++;

    #undef INITIAL_CAPACITY
    #undef GROWTH_FACTOR
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

RenderSprite extract_render_sprite(Sprite* sprite) {
    return (RenderSprite) {
        .texture = sprite->texture,
        .tint = sprite->tint,
        .source = sprite->source,
        .transform = sprite->transform,
        .z_index = sprite->z_index,
    };
}

void SYSTEM_cull_and_sort_sprites(
    SpriteList* sprite_list,
    SortedRenderSprites* render_sprites
) {
    render_sprites_reserve_total(render_sprites, sprite_list->count);
    
    Sprite* sprite;
    for (uint32 i = 0; i < sprite_list->count; i++) {
        sprite = &sprite_list->sprites[i];
        if (sprite->visible && sprite->camera_visible) {
            render_sprites_push_back_unchecked(
                render_sprites,
                extract_render_sprite(sprite)
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
