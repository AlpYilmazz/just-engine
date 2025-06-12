#pragma once

#include "raylib.h"

#include "base.h"

#define TEXTURE_SLOTS 100
#define DEFAULT_TEXTURE_HANDLE_ID   0
#define BLANK_TEXTURE_HANDLE_ID     1
#define WHITE_TEXTURE_HANDLE_ID     2

typedef struct {
    uint32 id;
} TextureHandle;

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
    bool image_changed[TEXTURE_SLOTS];
    Image images[TEXTURE_SLOTS];
    Texture textures[TEXTURE_SLOTS];
} TextureAssets;

TextureAssets new_texture_assets();

TextureHandle texture_assets_reserve_texture_slot(TextureAssets* assets);

void texture_assets_put_image(TextureAssets* assets, TextureHandle handle, Image image);
void texture_assets_load_image_unchecked(TextureAssets* assets, TextureHandle handle);
void texture_assets_load_texture_uncheched(TextureAssets* assets, TextureHandle handle);
void texture_assets_update_texture_unchecked(TextureAssets* assets, TextureHandle handle);
void texture_assets_update_texture_rec_unchecked(TextureAssets* assets, TextureHandle handle, Rectangle rec);
void texture_assets_put_image_and_load_texture(TextureAssets* assets, TextureHandle handle, Image image);
void texture_assets_load_texture_then_unload_image(TextureAssets* assets, TextureHandle handle, Image image);

ImageResponse texture_assets_get_image(TextureAssets* assets, TextureHandle handle);
Image* texture_assets_get_image_or_default(TextureAssets* assets, TextureHandle handle);
Image* texture_assets_get_image_unchecked(TextureAssets* assets, TextureHandle handle);
ImageResponse texture_assets_get_image_mut(TextureAssets* assets, TextureHandle handle);
Image* texture_assets_get_image_unchecked_mut(TextureAssets* assets, TextureHandle handle);

TextureResponse texture_assets_get_texture(TextureAssets* assets, TextureHandle handle);
Texture* texture_assets_get_texture_or_default(TextureAssets* assets, TextureHandle handle);
Texture* texture_assets_get_texture_unchecked(TextureAssets* assets, TextureHandle handle);

void texture_assets_unload_image(TextureAssets* assets, TextureHandle handle);
void texture_assets_unload_texture(TextureAssets* assets, TextureHandle handle);
void texture_assets_unload_slot(TextureAssets* assets, TextureHandle handle);
