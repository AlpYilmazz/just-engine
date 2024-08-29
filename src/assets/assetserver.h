#pragma once

#include "asset.h"
#include "events/events.h"

typedef struct {
    const char const* asset_folder;
} FileAssetServer;

TextureHandle just_engine_asyncio_file_load_image(
    ThreadPool* RES_threadpool,
    Events_TextureAssetEvent* RES_texture_assets_events,
    TextureAssets* RES_texture_assets,
    FileAssetServer* server,
    const char* filepath
);
