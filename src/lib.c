#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "raylib.h"
#include "raymath.h"

#include "base.h"
#include "logging.h"
#include "memory/memory.h"
#include "assets/asset.h"
#include "events/events.h"
#include "render2d/sprite.h"


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

void SYSTEM_POST_UPDATE_check_mutated_images(
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

void SYSTEM_EXTRACT_RENDER_load_textures_for_loaded_or_changed_images(
    TextureAssets* RES_texture_assets,
    Events_TextureAssetEvent* RES_texture_asset_events
) {
    static uint32 LOCAL_image_loaded_events_offset = 0;

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

void SYSTEM_FRAME_BOUNDARY_swap_event_buffers(
    Events_TextureAssetEvent* RES_texture_asset_events
) {
    just_engine_events_texture_asset_event_swap_buffers(RES_texture_asset_events);
}

void SYSTEM_FRAME_BOUNDARY_reset_temporary_storage(
    TemporaryStorage* RES_temporary_storage
) {
    reset_bump_allocator(RES_temporary_storage);
}

void SYSTEM_EXTRACT_RENDER_cull_and_sort_sprites(
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

void SYSTEM_RENDER_sorted_sprites(
    TextureAssets* RES_texture_assets,
    SortedRenderSprites* render_sprites
) {
    RenderSprite sprite;
    Texture* texture;
    SpriteTransform transform;
    Rectangle source;
    Vector2 size;
    Rectangle destination;
    Vector2 origin;
    float32 rotation;
    for (uint32 i = 0; i < render_sprites->count; i++) {
        sprite = render_sprites->sprites[i];
        texture = just_engine_texture_assets_get_texture_or_default(RES_texture_assets, sprite.texture);
        transform = sprite.transform;
        source = BYTEWISE_EQUALS(&sprite.source, &RECTANGLE_NICHE, Rectangle)
            ? (Rectangle) {0, 0, texture->width, texture->height}
            : sprite.source;
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
}