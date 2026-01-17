#include "base.h"
#include "memory/memory.h"
#include "thread/threadpool.h"
#include "assets/asset.h"
#include "assets/assetserver.h"
#include "network/net2.h"
#include "network/httpclient.h"
#include "render2d/camera2d.h"
#include "render2d/sprite.h"
#include "ui/justui.h"
#include "execution/execution.h"

typedef struct {
    // --------
    struct {
        URectSize size;
        const char* title;
    } window;
    // --------
    struct {
        uint32 target_fps;
    } execution;
    // --------
    struct {
        uint32 nthreads;
        uint32 task_queue_capacity;
    } threadpool;
    // --------
    struct {
        const char* asset_dir;
    } dir;
    // --------
    struct {
        URectSize render_screen_size; // 640x360
        Color clear_color;
        SpriteCamera primary_camera;
    } render2d;
    // --------
    bool use_network_subsystem;
    struct {
        NetworkConfig config;
    } network;
    // --------
    bool use_http_client_subsystem;
    // --------
} JustEngineInit;

typedef struct {
    // --------
    struct {
        ThreadPoolShutdown shutdown;
    } threadpool;
    // --------
} JustEngineDeinit;

typedef struct {
    // --------
    bool should_close;
    // --------
    float32 delta_time;
    // --------
    URectSize screen_size;
    BumpAllocator frame_storage;
    ThreadPool* threadpool;
    // -- Image/Texture
    FileImageServer file_image_server;
    TextureAssets texture_assets;
    Events_TextureAssetEvent texture_asset_events;
    // -- Render Begin
    URectSize render_screen_size;
    RenderTexture screen_target;
    Color clear_color;
    // -- Render2D
    SpriteCameraStore camera_store;
    SpriteStore sprite_store;
    // -- UI
    UIElementStore ui_store;
    // --------
} JustEngineGlobalResources;

typedef struct {
    // --------
    // -- Render2D
    PreparedRenderSprites prepared_render_sprites;
    // --------
} JustEngineGlobalRenderResources;

extern JustEngineGlobalResources JUST_GLOBAL;
extern JustEngineGlobalRenderResources JUST_RENDER_GLOBAL;

void just_engine_init(JustEngineInit init);
void just_engine_deinit(JustEngineDeinit deinit);
void just_engine_run(JustChapters chapters, JustEngineInit init, JustEngineDeinit* deinit);

// ---------------------------

/**
 * -- STAGES --
 * 
 * INITIALIZE
 * ----------
 * 
 * FRAME_BEGIN
 * 
 * INPUT
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
 * FRAME_END
 * 
 */

// ---------------------------

void SYSTEM_FRAME_BEGIN_set_delta_time(
    float32* RES_delta_time
);

void SYSTEM_POST_UPDATE_camera_visibility(
    SpriteCameraStore* sprite_camera_store,
    SpriteStore* RES_sprite_store
);

void SYSTEM_POST_UPDATE_check_mutated_images(
    TextureAssets* RES_texture_assets,
    Events_TextureAssetEvent* RES_texture_asset_events
);

void SYSTEM_EXTRACT_RENDER_load_textures_for_loaded_or_changed_images(
    TextureAssets* RES_texture_assets,
    Events_TextureAssetEvent* RES_texture_asset_events
);

void SYSTEM_FRAME_END_swap_event_buffers(
    Events_TextureAssetEvent* RES_texture_asset_events
);

void SYSTEM_FRAME_END_reset_temporary_storage(
    TemporaryStorage* RES_temporary_storage
);

// ---------------------------

// -- FRAME_BEGIN --

void JUST_SYSTEM_FRAME_BEGIN_set_delta_time();
void JUST_SYSTEM_FRAME_BEGIN_begin_imgui();

// -- INPUT --

void JUST_SYSTEM_INPUT_handle_input_for_ui_store();

// -- PREPARE --
// -- -- PRE_PREPARE --
// -- -- PREPARE --
// -- -- POST_PREPARE --

// -- UPDATE --
// -- -- PRE_UPDATE --
// -- -- UPDATE --

void JUST_SYSTEM_UPDATE_update_ui_elements();

// -- -- POST_UPDATE --

void JUST_SYSTEM_POST_UPDATE_check_mutated_images();
void JUST_SYSTEM_POST_UPDATE_camera_visibility();

// -- RENDER --
// -- -- QUEUE_RENDER --
// -- -- EXTRACT_RENDER --

void JUST_SYSTEM_EXTRACT_RENDER_load_textures_for_loaded_or_changed_images();
void JUST_SYSTEM_EXTRACT_RENDER_cull_and_sort_sprites();

// -- -- RENDER --

void JUST_SYSTEM_RENDER_begin_drawing();
void JUST_SYSTEM_RENDER_sorted_sprites();
void JUST_SYSTEM_RENDER_end_drawing();

void JUST_SYSTEM_RENDER_SCREEN_begin_drawing();
void JUST_SYSTEM_RENDER_SCREEN_draw_ui_elements();
void JUST_SYSTEM_RENDER_SCREEN_draw_imgui();
void JUST_SYSTEM_RENDER_SCREEN_end_drawing();

// -- FRAME_END --

void JUST_SYSTEM_FRAME_END_swap_event_buffers();
void JUST_SYSTEM_FRAME_END_reset_temporary_storage();

// ---------------------------

void APP_BUILDER_ADD__JUST_ENGINE_CORE_SYSTEMS(JustAppBuilder* app_builder);
void APP_ADD__JUST_ENGINE_CORE_SYSTEMS();
