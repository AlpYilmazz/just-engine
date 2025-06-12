#pragma once

#include "asset.h"
#include "events/events.h"

typedef struct {
    ThreadPool* RES_threadpool;
    Events_TextureAssetEvent* RES_texture_assets_events;
    TextureAssets* RES_texture_assets;
    const char const* asset_folder;
} FileImageServer;

TextureHandle asyncio_file_load_image(
    FileImageServer* server,
    const char* filepath
);
