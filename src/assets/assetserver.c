#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "raylib.h"

#include "threadpool/threadpool.h"

#include "asset.h"

#include "assetserver.h"

// NetworkAssetServer


// FileAssetServer

// -- LOAD IMAGE AND TEXTURE --

typedef struct {
    TextureAssets* texture_assets;
    TextureHandle handle;
    char* filepath;
} AsyncioFileLoadImageTaskArg;

void asyncio_file_load_image_task(TaskArgVoid* arg) {
    AsyncioFileLoadImageTaskArg* this_arg = arg;

    Image image = LoadImage(this_arg->filepath);
    just_engine_texture_assets_put_image(
        this_arg->texture_assets,
        this_arg->handle,
        image
    );

    free(this_arg->filepath);
    free(this_arg);
}

TextureHandle just_engine_asyncio_file_load_image(
    TextureAssets* texture_assets,
    FileAssetServer* server,
    const char* filepath
) {
    TextureHandle handle = just_engine_texture_assets_reserve_texture_slot(texture_assets);

    int path_len =
        strlen(server->asset_folder)    // "assets"
        + 1                             // '/'
        + strlen(filepath)              // "image.png"
        + 1;                            // '\0'
    char* path = malloc(path_len * sizeof(char));
    sprintf(path, "%s/%s", server->asset_folder, filepath);
    path[path_len-1] = '\0'; // does sprintf add this

    AsyncioFileLoadImageTaskArg* arg = malloc(sizeof(AsyncioFileLoadImageTaskArg));
    *arg = (AsyncioFileLoadImageTaskArg) {
        .texture_assets = texture_assets,
        .handle = handle,
        .filepath = path,
    };

    Task asyncio_task = {
        .handler = asyncio_file_load_image_task,
        .arg = arg
    };

    return handle;
}

// -- LOAD IMAGE AND TEXTURE -- END --