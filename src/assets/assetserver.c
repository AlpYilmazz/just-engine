#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "raylib.h"

#include "asset.h"
#include "thread/threadpool.h"
#include "logging.h"

#include "assetserver.h"

// NetworkAssetServer


// FileAssetServer

// -- LOAD IMAGE AND TEXTURE --

typedef struct {
    Events_TextureAssetEvent* RES_texture_assets_events;
    TextureAssets* RES_texture_assets;
    TextureHandle handle;
    char* filepath;
} AsyncioFileLoadImageTaskArg;

void asyncio_file_load_image_task(TaskArgVoid* arg) {
    AsyncioFileLoadImageTaskArg* this_arg = arg;

    Image image = LoadImage(this_arg->filepath);
    if (image.data == NULL) {
        JUST_LOG_ERROR("ERROR: [ASYNCIO][FILE][IMAGE] File could not be loaded: %s\n", this_arg->filepath);
        goto CLEANUP;
    }

    just_engine_texture_assets_put_image(
        this_arg->RES_texture_assets,
        this_arg->handle,
        image
    );

    TextureAssetEvent event = {
        .handle = this_arg->handle,
        .type = AssetEvent_Loaded,
        .consumed = false,
    };
    just_engine_events_texture_asset_event_send_single(
        this_arg->RES_texture_assets_events,
        event
    );

    CLEANUP:
    free(this_arg->filepath);
    free(this_arg);
}

TextureHandle just_engine_asyncio_file_load_image(
    ThreadPool* RES_threadpool,
    Events_TextureAssetEvent* RES_texture_assets_events,
    TextureAssets* RES_texture_assets,
    FileAssetServer* server,
    const char* filepath
) {
    TextureHandle handle = just_engine_texture_assets_reserve_texture_slot(RES_texture_assets);

    uint32 path_len =
        strlen(server->asset_folder)// "assets"
        + 1                             // '/'
        + strlen(filepath)              // "image.png"
        + 1;                            // '\0'
    char* path = malloc(path_len * sizeof(char));
    sprintf(path, "%s/%s", server->asset_folder, filepath);
    path[path_len-1] = '\0'; // does sprintf add this

    AsyncioFileLoadImageTaskArg* arg = malloc(sizeof(AsyncioFileLoadImageTaskArg));
    *arg = (AsyncioFileLoadImageTaskArg) {
        .RES_texture_assets_events = RES_texture_assets_events,
        .RES_texture_assets = RES_texture_assets,
        .handle = handle,
        .filepath = path,
    };

    Task asyncio_task = {
        .handler = asyncio_file_load_image_task,
        .arg = arg
    };
    thread_pool_add_task(RES_threadpool, asyncio_task);

    return handle;
}

// -- LOAD IMAGE AND TEXTURE -- END --