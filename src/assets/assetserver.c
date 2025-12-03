#include <stdio.h>

#include "raylib.h"

#include "justcstd.h"
#include "logging.h"
#include "memory/juststring.h"
#include "thread/threadpool.h"
#include "asset.h"

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

void asyncio_file_load_image_task(TaskExecutorContext* context, TaskArgVoid* arg) {
    AsyncioFileLoadImageTaskArg* this_arg = arg;

    Image image = LoadImage(this_arg->filepath);
    if (image.data == NULL) {
        JUST_LOG_ERROR("ERROR: [ASYNCIO][FILE][IMAGE] File could not be loaded: %s\n", this_arg->filepath);
        goto CLEANUP;
    }

    texture_assets_put_image(
        this_arg->RES_texture_assets,
        this_arg->handle,
        image
    );

    TextureAssetEvent event = {
        .handle = this_arg->handle,
        .type = AssetEvent_Loaded,
        .consumed = false,
    };
    TextureAssetEvent__events_send_single(
        this_arg->RES_texture_assets_events,
        event
    );

    CLEANUP:
    std_free(this_arg->filepath);
    std_free(this_arg);
}

TextureHandle asyncio_file_load_image(
    FileImageServer* server,
    const char* filepath
) {
    TextureHandle handle = texture_assets_reserve_texture_slot(server->RES_texture_assets);

    uint32 path_len =
        cstr_length(server->asset_folder)// "assets"
        + 1                             // '/'
        + cstr_length(filepath)              // "image.png"
        + 1;                            // '\0'
    char* path = std_malloc(path_len * sizeof(char));
    sprintf(path, "%s/%s", server->asset_folder, filepath);
    path[path_len-1] = '\0'; // does sprintf add this

    AsyncioFileLoadImageTaskArg* arg = std_malloc(sizeof(AsyncioFileLoadImageTaskArg));
    *arg = (AsyncioFileLoadImageTaskArg) {
        .RES_texture_assets_events = server->RES_texture_assets_events,
        .RES_texture_assets = server->RES_texture_assets,
        .handle = handle,
        .filepath = path,
    };

    Task asyncio_task = {
        .handler = asyncio_file_load_image_task,
        .arg = arg
    };
    thread_pool_add_task(server->RES_threadpool, asyncio_task);

    return handle;
}

// -- LOAD IMAGE AND TEXTURE -- END --