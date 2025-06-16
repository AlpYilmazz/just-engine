#pragma once

#include "asset.h"
#include "events/events.h"

typedef struct {
    ThreadPool* RES_threadpool;
    TextureAssets* RES_texture_assets;
    Events_TextureAssetEvent* RES_texture_assets_events;
    const char const* asset_folder;
} FileImageServer;

TextureHandle asyncio_file_load_image(
    FileImageServer* server,
    const char* filepath
);
