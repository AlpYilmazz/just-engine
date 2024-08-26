#include <stdlib.h>
#include <stdio.h>

#include "raylib.h"

#include "base.h"
#include "asset.h"

const AssetHandle PRIMARY_ASSET_HANDLE = {0};
const TextureHandle PRIMARY_TEXTURE_HANDLE = {0};
const uint8 DEFAULT_IMAGE_DATA[4] = {255, 0, 255, 255};

AssetHandle primary_handle() {
    return PRIMARY_ASSET_HANDLE;
}

AssetHandle new_handle(uint32 id) {
    return (AssetHandle) { id };
}

TextureHandle primary_texture_handle() {
    return PRIMARY_TEXTURE_HANDLE;
}

TextureHandle new_texture_handle(uint32 id) {
    return (TextureHandle) { id };
}

ImageResponse null_image_response() {
    return (ImageResponse) { false, NULL };
}

ImageResponse valid_image_response(Image* image) {
    return (ImageResponse) { true, image };
}

TextureResponse null_texture_response() {
    return (TextureResponse) { false, NULL };
}

TextureResponse valid_texture_response(Texture* texture) {
    return (TextureResponse) { true, texture };
}

// TextureAssets

TextureAssets just_engine_new_texture_assets() {
    TextureAssets texture_assets = {0};

    Image image = {
        .data = (void*)&DEFAULT_IMAGE_DATA,
        .width = 1,
        .height = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
        .mipmaps = 1
    };
    Texture texture = LoadTextureFromImage(image);

    texture_assets.next_slot_available_bump = 1;
    texture_assets.slots[PRIMARY_TEXTURE_HANDLE.id] = true;
    texture_assets.image_ready[PRIMARY_TEXTURE_HANDLE.id] = true;
    texture_assets.texture_ready[PRIMARY_TEXTURE_HANDLE.id] = true;
    texture_assets.images[PRIMARY_TEXTURE_HANDLE.id] = image;
    texture_assets.textures[PRIMARY_TEXTURE_HANDLE.id] = texture;
    
    return texture_assets;
}

TextureHandle just_engine_texture_assets_reserve_texture_slot(TextureAssets* assets) {
    if (assets->next_slot_available_bump < TEXTURE_SLOTS) {
        assets->slots[assets->next_slot_available_bump] = true;
        return new_texture_handle(assets->next_slot_available_bump++);
    }
    for (uint32 i = 0; i < TEXTURE_SLOTS; i++) {
        if (!assets->slots[i]) {
            assets->slots[i] = true;
            return new_texture_handle(i);
        }
    }
    // TODO: no texture slots available could not load texture
    return new_texture_handle(-1);
}

// -- LOAD IMAGE/TEXTURE --

void just_engine_texture_assets_put_image(TextureAssets* assets, TextureHandle handle, Image image) {
    assets->images[handle.id] = image;
    assets->image_ready[handle.id] = true;
}

void just_engine_texture_assets_load_image_unchecked(TextureAssets* assets, TextureHandle handle) {
    assets->images[handle.id] = LoadImageFromTexture(assets->textures[handle.id]);
    assets->image_ready[handle.id] = true;
}

void just_engine_texture_assets_load_texture_uncheched(TextureAssets* assets, TextureHandle handle) {
    assets->textures[handle.id] = LoadTextureFromImage(assets->images[handle.id]);
    assets->texture_ready[handle.id] = true;
}

void just_engine_texture_assets_put_image_and_load_texture(TextureAssets* assets, TextureHandle handle, Image image) {
    just_engine_texture_assets_put_image(assets, handle, image);
    just_engine_texture_assets_load_texture_uncheched(assets, handle);
}

void just_engine_texture_assets_load_texture_then_unload_image(TextureAssets* assets, TextureHandle handle, Image image) {
    assets->images[handle.id] = image;
    just_engine_texture_assets_load_texture_uncheched(assets, handle);
    UnloadImage(image);
    // assets->images[handle.id] = (Image) {0};
}

// -- LOAD IMAGE/TEXTURE -- END --

// -- GET IMAGE

ImageResponse just_engine_texture_assets_get_image(TextureAssets* assets, TextureHandle handle) {
    if (assets->image_ready[handle.id]) {
        return valid_image_response(&assets->images[handle.id]);
    }
    return null_image_response();
}

Image* just_engine_texture_assets_get_image_or_default(TextureAssets* assets, TextureHandle handle) {
    if (assets->image_ready[handle.id]) {
        return &assets->images[handle.id];
    }
    return &assets->images[PRIMARY_TEXTURE_HANDLE.id];
}

Image* just_engine_texture_assets_get_image_unchecked(TextureAssets* assets, TextureHandle handle) {
    return &assets->images[handle.id];
}

ImageResponse just_engine_texture_assets_get_image_mut(TextureAssets* assets, TextureHandle handle) {
    if (assets->image_ready[handle.id]) {
        assets->image_changed[handle.id] = true;
        return valid_image_response(&assets->images[handle.id]);
    }
    return null_image_response();
}

Image* just_engine_texture_assets_get_image_unchecked_mut(TextureAssets* assets, TextureHandle handle) {
    assets->image_changed[handle.id] = true;
    return &assets->images[handle.id];
}

// -- GET IMAGE -- END --

// -- GET TEXTURE

TextureResponse just_engine_texture_assets_get_texture(TextureAssets* assets, TextureHandle handle) {
    if (assets->texture_ready[handle.id]) {
        return valid_texture_response(&assets->textures[handle.id]);
    }
    return null_texture_response();
}

Texture* just_engine_texture_assets_get_texture_or_default(TextureAssets* assets, TextureHandle handle) {
    if (assets->texture_ready[handle.id]) {
        return &assets->textures[handle.id];
    }
    return &assets->textures[PRIMARY_TEXTURE_HANDLE.id];
}

Texture* just_engine_texture_assets_get_texture_unchecked(TextureAssets* assets, TextureHandle handle) {
    return &assets->textures[handle.id];
}

// -- GET TEXTURE -- END --

// -- UNLOAD --

void just_engine_texture_assets_unload_image(TextureAssets* assets, TextureHandle handle) {
    if (!assets->image_ready[handle.id]) {
        return;
    }

    assets->image_ready[handle.id] = false;

    Image image = assets->images[handle.id];
    UnloadImage(image);
    // assets->images[handle.id] = (Image) {0};
}

void just_engine_texture_assets_unload_texture(TextureAssets* assets, TextureHandle handle) {
    if (!assets->texture_ready[handle.id]) {
        return;
    }

    assets->texture_ready[handle.id] = false;

    Texture texture = assets->textures[handle.id];
    UnloadTexture(texture);
    // assets->textures[handle.id] = (Texture) {0};
}

void just_engine_texture_assets_unload_slot(TextureAssets* assets, TextureHandle handle) {
    if (!assets->slots[handle.id]) {
        return;
    }

    assets->slots[handle.id] = false;
    assets->texture_ready[handle.id] = false;

    if (handle.id == assets->next_slot_available_bump - 1) {
        assets->next_slot_available_bump--;
    }

    Image image = assets->images[handle.id];
    Texture texture = assets->textures[handle.id];
    UnloadImage(image);
    UnloadTexture(texture);
    // assets->images[handle.id] = (Image) {0};
    // assets->textures[handle.id] = (Texture) {0};
}

// -- UNLOAD -- END --
