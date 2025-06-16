#include "base.h"
#include "memory/memory.h"
#include "thread/threadpool.h"
#include "assets/asset.h"
#include "assets/assetserver.h"
#include "render2d/camera2d.h"
#include "render2d/sprite.h"
#include "ui/justui.h"

typedef struct {
    URectSize screen_size;
    uint32 threadpool_nthreads;
    uint32 threadpool_taskqueuecapacity;
    const char* asset_folder;
    SpriteCamera primary_camera;
} JustEngineInit;

typedef struct {
    ThreadPoolShutdown threadpool_shutdown;
} JustEngineDeinit;

typedef struct {
    float32 delta_time;
    // --------
    URectSize screen_size;
    BumpAllocator temporary_storage;
    ThreadPool* threadpool;
    // -- Image/Texture
    FileImageServer file_image_server;
    TextureAssets texture_assets;
    Events_TextureAssetEvent texture_asset_events;
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

// ---------------------------

/**
 * -- STAGES --
 * 
 * INITIALIZE
 * ----------
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
 * FRAME_BOUNDARY
 * 
 */

typedef enum {
    STAGE__INPUT,
    //
    STAGE__PREPARE__PRE_PREPARE,
    STAGE__PREPARE__PREPARE,
    STAGE__PREPARE__POST_PREPARE,
    //
    STAGE__UPDATE__PRE_UPDATE,
    STAGE__UPDATE__UPDATE,
    STAGE__UPDATE__POST_UPDATE,
    //
    STAGE__RENDER__QUEUE_RENDER,
    STAGE__RENDER__EXTRACT_RENDER,
    STAGE__RENDER__RENDER,
    //
    STAGE__FRAME_BOUNDARY
} JustEngineSystemStage;

// ---------------------------

void SYSTEM_PRE_PREPARE_set_delta_time(
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

void SYSTEM_FRAME_BOUNDARY_swap_event_buffers(
    Events_TextureAssetEvent* RES_texture_asset_events
);

void SYSTEM_FRAME_BOUNDARY_reset_temporary_storage(
    TemporaryStorage* RES_temporary_storage
);

// ---------------------------

// -- INPUT --

void JUST_SYSTEM_INPUT_handle_input_for_ui_store();

// -- PREPARE --
// -- -- PRE_PREPARE --

void JUST_SYSTEM_PRE_PREPARE_set_delta_time();

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

void JUST_SYSTEM_RENDER_sorted_sprites();
void JUST_SYSTEM_RENDER_draw_ui_elements();

// -- FRAME_BOUNDARY --

void JUST_SYSTEM_FRAME_BOUNDARY_swap_event_buffers();
void JUST_SYSTEM_FRAME_BOUNDARY_reset_temporary_storage();

// ---------------------------

void JUST_ENGINE_RUN_STAGE(JustEngineSystemStage stage);
