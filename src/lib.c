#include <stdio.h>
#include <assert.h>

#include "raylib.h"
#include "raymath.h"

#include "raycimgui.h"

#include "justcstd.h"
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
    RenderTexture screen_target = LoadRenderTexture(init.render2d.render_screen_size.width, init.render2d.render_screen_size.height);
    SetTextureFilter(screen_target.texture, TEXTURE_FILTER_POINT);

    JUST_GLOBAL = (JustEngineGlobalResources) {
        .should_close = false,
        .delta_time = 0.0,
        .screen_size = init.window.size,
        .frame_storage = make_bump_allocator(),
        .threadpool = thread_pool_create(init.threadpool.nthreads, init.threadpool.task_queue_capacity),
        .file_image_server = LATER_INIT,
        .texture_assets = new_texture_assets(),
        .texture_asset_events = TextureAssetEvent__events_create(),
        .render_screen_size = init.render2d.render_screen_size,
        .screen_target = screen_target,
        .clear_color = init.render2d.clear_color,
        .camera_store = STRUCT_ZERO_INIT,
        .sprite_store = STRUCT_ZERO_INIT,
        .ui_store = ui_element_store_new(),
    };

    JUST_GLOBAL.file_image_server = (FileImageServer) {
        .RES_threadpool = JUST_GLOBAL.threadpool,
        .RES_texture_assets = &JUST_GLOBAL.texture_assets,
        .RES_texture_assets_events = &JUST_GLOBAL.texture_asset_events,
        .asset_folder = init.dir.asset_dir,
    };

    set_primary_camera(&JUST_GLOBAL.camera_store, init.render2d.primary_camera);
}

void just_engine_deinit(JustEngineDeinit deinit) {
    thread_pool_shutdown(&JUST_GLOBAL.threadpool, deinit.threadpool.shutdown);
}

static JustChapter* find_chapter(JustChapters* chapters, int32 chapter_id) {
    for (usize i = 0; i < chapters->count; i++) {
        JustChapter* ch = chapters->chapters[i];
        if (ch->chapter_id == chapter_id) {
            return ch;
        }
    }
    return NULL;
}

void just_engine_run(JustChapters chapters, JustEngineInit init, JustEngineDeinit* deinit) {
    #pragma region INITIALIZE

    InitWindow(init.window.size.width, init.window.size.height, init.window.title);
    SetTargetFPS(init.execution.target_fps);
    // if (!IsWindowFullscreen()) {
    //     ToggleFullscreen();
    // }

    just_engine_init(init);

	rligSetup(true);

    if (init.use_network_subsystem) {
        configure_network_system(init.network.config);
        start_network_thread();
    }

    if (init.use_http_client_subsystem) {
        just_http_global_init_default();
    }

    #pragma endregion INITIALIZE
    
    #pragma region RUN

    JustChapter* current_chapter = find_chapter(&chapters, chapters.initial_chapter);
    if (current_chapter->init_fn) {
        current_chapter->init_fn();
    }

    while (true) {
        just_app_run_once(&current_chapter->app);
        if (JUST_GLOBAL.should_close) {
            break;
        }
        if (current_chapter->end) {
            JustChapter* next_chapter = find_chapter(&chapters, current_chapter->transition_id);

            if (current_chapter->deinit_fn) {
                current_chapter->deinit_fn();
            }
            current_chapter->end = false;
            current_chapter->transition_id = 0;

            if (next_chapter->init_fn) {
                next_chapter->init_fn();
            }
            current_chapter = next_chapter;
        }
    }

    #pragma endregion RUN

    #pragma region DEINITIALIZE

    if (deinit != NULL) {
        just_engine_deinit(*deinit);
    }

    #pragma endregion DEINITIALIZE
}

// ---------------------------

void SYSTEM_FRAME_BEGIN_set_delta_time(
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
                .type = AssetEvent_ImageChanged,
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
        // TODO: Push Texture Events
        switch (event.type) {
        case AssetEvent_ImageChanged:
            texture_assets_update_texture_unchecked(RES_texture_assets, event.handle);
            break;
        case AssetEvent_ImageLoaded:
            texture_assets_load_texture_uncheched(RES_texture_assets, event.handle);
            break;
        }
    }

    LOCAL_image_loaded_events_offset = TextureAssetEvent__events_iter_end(&events_iter);
}

void SYSTEM_FRAME_END_swap_event_buffers(
    Events_TextureAssetEvent* RES_texture_asset_events
) {
    TextureAssetEvent__events_swap_buffers(RES_texture_asset_events);
}

void SYSTEM_FRAME_END_reset_temporary_storage(
    BumpAllocator* RES_temporary_storage
) {
    reset_bump_allocator(RES_temporary_storage);
}

// ---------------------------

// -- FRAME_BEGIN --

void JUST_SYSTEM_FRAME_BEGIN_set_delta_time() {
    SYSTEM_FRAME_BEGIN_set_delta_time(
        &JUST_GLOBAL.delta_time
    );
}

void JUST_SYSTEM_FRAME_BEGIN_begin_imgui() {
    rligBegin();
}

// -- INPUT --

void JUST_SYSTEM_INPUT_handle_input_for_ui_store() {
    SYSTEM_INPUT_handle_input_for_ui_store(
        &JUST_GLOBAL.ui_store
    );
}

// -- PREPARE --
// -- -- PRE_PREPARE --
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

void JUST_SYSTEM_RENDER_begin_drawing() {
    BeginTextureMode(JUST_GLOBAL.screen_target);
        ClearBackground(JUST_GLOBAL.clear_color);
}

void JUST_SYSTEM_RENDER_sorted_sprites() {
    SYSTEM_RENDER_sorted_sprites(
        &JUST_GLOBAL.texture_assets,
        &JUST_GLOBAL.camera_store,
        &JUST_RENDER_GLOBAL.prepared_render_sprites
    );
}

void JUST_SYSTEM_RENDER_end_drawing() {
    EndTextureMode();
}

void JUST_SYSTEM_RENDER_SCREEN_begin_drawing() {
    BeginDrawing();
        Texture texture = JUST_GLOBAL.screen_target.texture;
        URectSize screen_size = JUST_GLOBAL.screen_size;
        DrawTexturePro(
            texture,
            (Rectangle) { 0, 0, (float)texture.width, (float)-texture.height },
            (Rectangle) { 0, 0, (float)screen_size.width, (float)screen_size.height },
            (Vector2) { 0, 0 },
            0,
            WHITE
        );
}

void JUST_SYSTEM_RENDER_SCREEN_draw_ui_elements() {
    SYSTEM_RENDER_draw_ui_elements(
        &JUST_GLOBAL.ui_store
    );
}

void JUST_SYSTEM_RENDER_SCREEN_draw_imgui() {
    rligEnd();
}

void JUST_SYSTEM_RENDER_SCREEN_end_drawing() {
    EndDrawing();
}

// -- FRAME_END --

void JUST_SYSTEM_FRAME_END_swap_event_buffers() {
    SYSTEM_FRAME_END_swap_event_buffers(
        &JUST_GLOBAL.texture_asset_events
    );
}

void JUST_SYSTEM_FRAME_END_reset_temporary_storage() {
    SYSTEM_FRAME_END_reset_temporary_storage(
        &JUST_GLOBAL.frame_storage
    );
}

// ---------------------------
#if 0
void JUST_ENGINE_RUN_STAGE(JustEngineSystemStage stage) {
    switch (stage) {
        case STAGE__INPUT:
        JUST_LOG_TRACE("-- JustEngine -- Stage: STAGE__INPUT --\n");
        JUST_SYSTEM_INPUT_handle_input_for_ui_store();
        break;

    case STAGE__PREPARE__PRE_PREPARE:
        JUST_LOG_TRACE("-- JustEngine -- Stage: STAGE__PREPARE__PRE_PREPARE --\n");
        JUST_SYSTEM_PRE_PREPARE_set_delta_time();
        break;
    case STAGE__PREPARE__PREPARE:
        JUST_LOG_TRACE("-- JustEngine -- Stage: STAGE__PREPARE__PREPARE --\n");
        break;
    case STAGE__PREPARE__POST_PREPARE:
        JUST_LOG_TRACE("-- JustEngine -- Stage: STAGE__PREPARE__POST_PREPARE --\n");
        break;

    case STAGE__UPDATE__PRE_UPDATE:
        JUST_LOG_TRACE("-- JustEngine -- Stage: STAGE__UPDATE__PRE_UPDATE --\n");
        break;
    case STAGE__UPDATE__UPDATE:
        JUST_LOG_TRACE("-- JustEngine -- Stage: STAGE__UPDATE__UPDATE --\n");
        JUST_SYSTEM_UPDATE_update_ui_elements();
        break;
    case STAGE__UPDATE__POST_UPDATE:
        JUST_LOG_TRACE("-- JustEngine -- Stage: STAGE__UPDATE__POST_UPDATE --\n");
        JUST_SYSTEM_POST_UPDATE_check_mutated_images();
        JUST_SYSTEM_POST_UPDATE_camera_visibility();
        break;
        
    case STAGE__RENDER__QUEUE_RENDER:
        JUST_LOG_TRACE("-- JustEngine -- Stage: STAGE__RENDER__QUEUE_RENDER --\n");
        break;
    case STAGE__RENDER__EXTRACT_RENDER:
        JUST_LOG_TRACE("-- JustEngine -- Stage: STAGE__RENDER__EXTRACT_RENDER --\n");
        JUST_SYSTEM_EXTRACT_RENDER_load_textures_for_loaded_or_changed_images();
        JUST_SYSTEM_EXTRACT_RENDER_cull_and_sort_sprites();
        break;
    case STAGE__RENDER__RENDER:
        JUST_LOG_TRACE("-- JustEngine -- Stage: STAGE__RENDER__RENDER --\n");
        JUST_SYSTEM_RENDER_sorted_sprites();
        JUST_SYSTEM_RENDER_draw_ui_elements();
        break;
    
    case STAGE__FRAME_BOUNDARY:
        JUST_LOG_TRACE("-- JustEngine -- Stage: STAGE__FRAME_BOUNDARY --\n");
        JUST_SYSTEM_FRAME_BOUNDARY_swap_event_buffers();
        JUST_SYSTEM_FRAME_BOUNDARY_reset_temporary_storage();
        break;
    }
}
#endif
// ---------------------------

void APP_BUILDER_ADD__JUST_ENGINE_CORE_SYSTEMS(JustAppBuilder* app_builder) {
    int32 STAGE; 
    
    // =====
    STAGE = CORE_STAGE__FRAME_BEGIN;
    {
        just_app_builder_add_system_with(app_builder, STAGE, fn_into_system(JUST_SYSTEM_FRAME_BEGIN_set_delta_time), (SystemConstraint) { .run_first = true });
        just_app_builder_add_system(app_builder, STAGE, fn_into_system(JUST_SYSTEM_FRAME_BEGIN_begin_imgui));
    }

    // =====
    STAGE = CORE_STAGE__INPUT;
    {
        just_app_builder_add_system(app_builder, STAGE, fn_into_system(JUST_SYSTEM_INPUT_handle_input_for_ui_store));
    }

    // =====
    STAGE = CORE_STAGE__PREPARE__PRE_PREPARE;

    STAGE = CORE_STAGE__PREPARE__PREPARE;

    STAGE = CORE_STAGE__PREPARE__POST_PREPARE;

    // =====
    STAGE = CORE_STAGE__UPDATE__PRE_UPDATE;

    STAGE = CORE_STAGE__UPDATE__UPDATE;
    {
        just_app_builder_add_system(app_builder, STAGE, fn_into_system(JUST_SYSTEM_UPDATE_update_ui_elements));
    }

    STAGE = CORE_STAGE__UPDATE__POST_UPDATE;
    {
        just_app_builder_add_system(app_builder, STAGE, fn_into_system(JUST_SYSTEM_POST_UPDATE_check_mutated_images));
        just_app_builder_add_system(app_builder, STAGE, fn_into_system(JUST_SYSTEM_POST_UPDATE_camera_visibility));
    }
    
    // =====
    STAGE = CORE_STAGE__RENDER__QUEUE_RENDER;

    STAGE = CORE_STAGE__RENDER__EXTRACT_RENDER;
    {
        just_app_builder_add_system(app_builder, STAGE, fn_into_system(JUST_SYSTEM_EXTRACT_RENDER_load_textures_for_loaded_or_changed_images));
        just_app_builder_add_system(app_builder, STAGE, fn_into_system(JUST_SYSTEM_EXTRACT_RENDER_cull_and_sort_sprites));
    }

    STAGE = CORE_STAGE__RENDER__RENDER;
    {
        just_app_builder_add_system_with(app_builder, STAGE, fn_into_system(JUST_SYSTEM_RENDER_begin_drawing), (SystemConstraint) { .run_first = true });
        just_app_builder_add_system_with(app_builder, STAGE,
            fn_into_system(JUST_SYSTEM_RENDER_sorted_sprites),
            (SystemConstraint) {
                .run_after = {
                    .count = 1,
                    .systems = { fn_into_system(JUST_SYSTEM_RENDER_begin_drawing) },
                },
            }
        );
        just_app_builder_add_system_with(app_builder, STAGE, fn_into_system(JUST_SYSTEM_RENDER_end_drawing), (SystemConstraint) { .run_last = true });
    }

    STAGE = CORE_STAGE__RENDER__RENDER_SCREEN;
    {
        just_app_builder_add_system_with(app_builder, STAGE, fn_into_system(JUST_SYSTEM_RENDER_SCREEN_begin_drawing), (SystemConstraint) { .run_first = true });
        just_app_builder_add_system_with(app_builder, STAGE,
            fn_into_system(JUST_SYSTEM_RENDER_SCREEN_draw_ui_elements),
            (SystemConstraint) {
                .run_after = {
                    .count = 1,
                    .systems = { fn_into_system(JUST_SYSTEM_RENDER_SCREEN_begin_drawing) },
                },
            }
        );
        just_app_builder_add_system_with(app_builder, STAGE,
            fn_into_system(JUST_SYSTEM_RENDER_SCREEN_draw_imgui),
            (SystemConstraint) {
                .run_after = {
                    .count = 1,
                    .systems = { fn_into_system(JUST_SYSTEM_RENDER_SCREEN_draw_ui_elements) },
                },
            }
        );
        just_app_builder_add_system_with(app_builder, STAGE, fn_into_system(JUST_SYSTEM_RENDER_SCREEN_end_drawing), (SystemConstraint) { .run_last = true });
    }
    
    // =====
    STAGE = CORE_STAGE__FRAME_END;
    {
        just_app_builder_add_system(app_builder, STAGE, fn_into_system(JUST_SYSTEM_FRAME_END_swap_event_buffers));
        just_app_builder_add_system(app_builder, STAGE, fn_into_system(JUST_SYSTEM_FRAME_END_reset_temporary_storage));
    }
}

void APP_ADD__JUST_ENGINE_CORE_SYSTEMS() {
    APP_BUILDER_ADD__JUST_ENGINE_CORE_SYSTEMS(GLOBAL_APP_BUILDER());
}
