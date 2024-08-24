#pragma once

#include "asset.h"

typedef struct {
    const char const* asset_folder;
} FileAssetServer;

TextureHandle just_engine_asyncio_file_load_image(
    TextureAssets* texture_assets,
    FileAssetServer* server,
    const char* filepath
);
