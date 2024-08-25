#pragma once

#include "raylib.h"

#include "base.h"

#define TEXTURE_SLOTS 100

typedef struct {
    uint32 id;
} AssetHandle;

AssetHandle primary_handle();
AssetHandle new_handle(uint32 id);

typedef struct {
    uint32 id;
} TextureHandle;

TextureHandle primary_texture_handle();
TextureHandle new_texture_handle(uint32 id);

typedef struct {
    bool exists;
    Image* texture;
} ImageResponse;

typedef struct {
    bool exists;
    Texture* texture;
} TextureResponse;

typedef struct {
    uint32 next_slot_available_bump;
    bool slots[TEXTURE_SLOTS];
    bool image_ready[TEXTURE_SLOTS];
    bool texture_ready[TEXTURE_SLOTS];
    Image images[TEXTURE_SLOTS];
    Texture textures[TEXTURE_SLOTS];
} TextureAssets;

TextureAssets just_engine_new_texture_assets();

TextureHandle just_engine_texture_assets_reserve_texture_slot(TextureAssets* assets);

void just_engine_texture_assets_put_image(TextureAssets* assets, TextureHandle handle, Image image);
void just_engine_texture_assets_load_image_unchecked(TextureAssets* assets, TextureHandle handle);
void just_engine_texture_assets_load_texture_uncheched(TextureAssets* assets, TextureHandle handle);
void just_engine_texture_assets_put_image_and_load_texture(TextureAssets* assets, TextureHandle handle, Image image);
void just_engine_texture_assets_load_texture_then_unload_image(TextureAssets* assets, TextureHandle handle, Image image);

ImageResponse just_engine_texture_assets_get_image(TextureAssets* assets, TextureHandle handle);
Image* just_engine_texture_assets_get_image_or_default(TextureAssets* assets, TextureHandle handle);
Image* just_engine_texture_assets_get_image_unchecked(TextureAssets* assets, TextureHandle handle);

TextureResponse just_engine_texture_assets_get_texture(TextureAssets* assets, TextureHandle handle);
Texture* just_engine_texture_assets_get_texture_or_default(TextureAssets* assets, TextureHandle handle);
Texture* just_engine_texture_assets_get_texture_unchecked(TextureAssets* assets, TextureHandle handle);

void just_engine_texture_assets_unload_image(TextureAssets* assets, TextureHandle handle);
void just_engine_texture_assets_unload_texture(TextureAssets* assets, TextureHandle handle);
void just_engine_texture_assets_unload_slot(TextureAssets* assets, TextureHandle handle);
