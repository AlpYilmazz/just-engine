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

#include "lib.h"


// TODO:
//  - Game Base Thread
//      - runs game logic
//  - Render Base Thread (Main thread)
//      - runs render logic
//      - gpu bus io
//      - draw calls


JustEngineGlobalResources JUST_GLOBAL = LAZY_INIT;
JustEngineGlobalRenderResources JUST_RENDER_GLOBAL = LAZY_INIT;

void just_engine_init(JustEngineInit init) {
    JUST_GLOBAL = (JustEngineGlobalResources) {
        .delta_time = 0.0,
        .screen_size = init.screen_size,
        .temporary_storage = make_bump_allocator(),
        .threadpool = thread_pool_create(init.threadpool_nthreads, init.threadpool_taskqueuecapacity),
        .file_image_server = LAZY_INIT,
        .texture_assets = new_texture_assets(),
        .texture_asset_events = TextureAssetEvent__events_create(),
        .camera_store = STRUCT_ZERO_INIT,
        .sprite_store = STRUCT_ZERO_INIT,
        .ui_store = ui_element_store_new(),
    };

    JUST_GLOBAL.file_image_server = (FileImageServer) {
        .RES_threadpool = JUST_GLOBAL.threadpool,
        .RES_texture_assets = &JUST_GLOBAL.texture_assets,
        .RES_texture_assets_events = &JUST_GLOBAL.texture_asset_events,
        .asset_folder = init.asset_folder,
    };

    set_primary_camera(&JUST_GLOBAL.camera_store, init.primary_camera);
}

void just_engine_deinit(JustEngineDeinit deinit) {
    thread_pool_shutdown(&JUST_GLOBAL.threadpool, deinit.threadpool_shutdown);
}

// ---------------------------

void SYSTEM_PRE_PREPARE_set_delta_time(
    float32* RES_delta_time
) {
    *RES_delta_time = GetFrameTime();
}

void SYSTEM_POST_UPDATE_camera_visibility(
    SpriteCameraStore* sprite_camera_store,
    SpriteStore* RES_sprite_store
) {
    // TODO
}

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
            TextureAssetEvent__events_send_single(RES_texture_asset_events, event);
        }
    }
}

void SYSTEM_EXTRACT_RENDER_load_textures_for_loaded_or_changed_images(
    TextureAssets* RES_texture_assets,
    Events_TextureAssetEvent* RES_texture_asset_events
) {
    static uint32 LOCAL_image_loaded_events_offset = 0;

    EventsIter_TextureAssetEvent events_iter = TextureAssetEvent__events_begin_iter(RES_texture_asset_events, LOCAL_image_loaded_events_offset);

    while (TextureAssetEvent__events_iter_has_next(&events_iter)) {
        TextureAssetEvent event = TextureAssetEvent__events_iter_read_next(&events_iter);
        switch (event.type) {
        case AssetEvent_Changed:
            texture_assets_update_texture_unchecked(RES_texture_assets, event.handle);
            break;
        case AssetEvent_Loaded:
            texture_assets_load_texture_uncheched(RES_texture_assets, event.handle);
            break;
        }
    }

    LOCAL_image_loaded_events_offset = TextureAssetEvent__events_iter_end(&events_iter);
}

void SYSTEM_FRAME_BOUNDARY_swap_event_buffers(
    Events_TextureAssetEvent* RES_texture_asset_events
) {
    TextureAssetEvent__events_swap_buffers(RES_texture_asset_events);
}

void SYSTEM_FRAME_BOUNDARY_reset_temporary_storage(
    TemporaryStorage* RES_temporary_storage
) {
    reset_bump_allocator(RES_temporary_storage);
}

// ---------------------------

// -- INPUT --

void JUST_SYSTEM_INPUT_handle_input_for_ui_store() {
    SYSTEM_INPUT_handle_input_for_ui_store(
        &JUST_GLOBAL.ui_store
    );
}

// -- PREPARE --
// -- -- PRE_PREPARE --

void JUST_SYSTEM_PRE_PREPARE_set_delta_time() {
    SYSTEM_PRE_PREPARE_set_delta_time(
        &JUST_GLOBAL.delta_time
    );
}

// -- -- PREPARE --
// -- -- POST_PREPARE --

// -- UPDATE --
// -- -- PRE_UPDATE --
// -- -- UPDATE --

void JUST_SYSTEM_UPDATE_update_ui_elements() {
    SYSTEM_UPDATE_update_ui_elements(
        &JUST_GLOBAL.ui_store,
        JUST_GLOBAL.delta_time
    );
}

// -- -- POST_UPDATE --

void JUST_SYSTEM_POST_UPDATE_check_mutated_images() {
    SYSTEM_POST_UPDATE_check_mutated_images(
        &JUST_GLOBAL.texture_assets,
        &JUST_GLOBAL.texture_asset_events
    );
}

void JUST_SYSTEM_POST_UPDATE_camera_visibility() {
    SYSTEM_POST_UPDATE_camera_visibility(
        &JUST_GLOBAL.camera_store,
        &JUST_GLOBAL.sprite_store
    );
}

// -- RENDER --
// -- -- QUEUE_RENDER --
// -- -- EXTRACT_RENDER --

void JUST_SYSTEM_EXTRACT_RENDER_load_textures_for_loaded_or_changed_images() {
    SYSTEM_EXTRACT_RENDER_load_textures_for_loaded_or_changed_images(
        &JUST_GLOBAL.texture_assets,
        &JUST_GLOBAL.texture_asset_events
    );
}

void JUST_SYSTEM_EXTRACT_RENDER_cull_and_sort_sprites() {
    SYSTEM_EXTRACT_RENDER_cull_and_sort_sprites(
        &JUST_GLOBAL.camera_store,
        &JUST_GLOBAL.sprite_store,
        &JUST_RENDER_GLOBAL.prepared_render_sprites
    );
}

// -- -- RENDER --

void JUST_SYSTEM_RENDER_sorted_sprites() {
    SYSTEM_RENDER_sorted_sprites(
        &JUST_GLOBAL.texture_assets,
        &JUST_GLOBAL.camera_store,
        &JUST_RENDER_GLOBAL.prepared_render_sprites
    );
}

void JUST_SYSTEM_RENDER_draw_ui_elements() {
    SYSTEM_RENDER_draw_ui_elements(
        &JUST_GLOBAL.ui_store
    );
}

// -- FRAME_BOUNDARY --

void JUST_SYSTEM_FRAME_BOUNDARY_swap_event_buffers() {
    SYSTEM_FRAME_BOUNDARY_swap_event_buffers(
        &JUST_GLOBAL.texture_asset_events
    );
}

void JUST_SYSTEM_FRAME_BOUNDARY_reset_temporary_storage() {
    SYSTEM_FRAME_BOUNDARY_reset_temporary_storage(
        &JUST_GLOBAL.temporary_storage
    );
}

// ---------------------------

void JUST_ENGINE_RUN_STAGE(JustEngineSystemStage stage) {
    switch (stage) {
    case STAGE__INPUT:
        JUST_SYSTEM_INPUT_handle_input_for_ui_store();
        break;

    case STAGE__PREPARE__PRE_PREPARE:
        JUST_SYSTEM_PRE_PREPARE_set_delta_time();
        break;
    case STAGE__PREPARE__PREPARE:
        break;
    case STAGE__PREPARE__POST_PREPARE:
        break;

    case STAGE__UPDATE__PRE_UPDATE:
        break;
    case STAGE__UPDATE__UPDATE:
        JUST_SYSTEM_UPDATE_update_ui_elements();
        break;
    case STAGE__UPDATE__POST_UPDATE:
        JUST_SYSTEM_POST_UPDATE_check_mutated_images();
        JUST_SYSTEM_POST_UPDATE_camera_visibility();
        break;
        
    case STAGE__RENDER__QUEUE_RENDER:
        break;
    case STAGE__RENDER__EXTRACT_RENDER:
        JUST_SYSTEM_EXTRACT_RENDER_load_textures_for_loaded_or_changed_images();
        JUST_SYSTEM_EXTRACT_RENDER_cull_and_sort_sprites();
        break;
    case STAGE__RENDER__RENDER:
        JUST_SYSTEM_RENDER_sorted_sprites();
        JUST_SYSTEM_RENDER_draw_ui_elements();
        break;
    
    case STAGE__FRAME_BOUNDARY:
        JUST_SYSTEM_FRAME_BOUNDARY_swap_event_buffers();
        JUST_SYSTEM_FRAME_BOUNDARY_reset_temporary_storage();
        break;
    }
}