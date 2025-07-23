#include <stdio.h>

#include "raylib.h"

#include "justcstd.h"
#include "base.h"
#include "asset.h"

const uint8 DEFAULT_IMAGE_DATA[4] = {255, 0, 255, 255}; // MAGENTA
const uint8 BLANK_IMAGE_DATA[4] = {0, 0, 0, 0};         // BLANK
const uint8 WHITE_IMAGE_DATA[4] = {255, 255, 255, 255}; // WHITE

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

TextureAssets new_texture_assets() {
    TextureAssets texture_assets = {0};

    Image default_image = {
        .data = (void*)&DEFAULT_IMAGE_DATA,
        .width = 1,
        .height = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
        .mipmaps = 1
    };
    Texture default_texture = LoadTextureFromImage(default_image);

    Image blank_image = {
        .data = (void*)&BLANK_IMAGE_DATA,
        .width = 1,
        .height = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
        .mipmaps = 1
    };
    Texture blank_texture = LoadTextureFromImage(blank_image);

    Image white_image = {
        .data = (void*)&WHITE_IMAGE_DATA,
        .width = 1,
        .height = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
        .mipmaps = 1
    };
    Texture white_texture = LoadTextureFromImage(white_image);

    texture_assets.next_slot_available_bump = 3;

    texture_assets.slots[DEFAULT_TEXTURE_HANDLE_ID] = true;
    texture_assets.image_ready[DEFAULT_TEXTURE_HANDLE_ID] = true;
    texture_assets.texture_ready[DEFAULT_TEXTURE_HANDLE_ID] = true;
    texture_assets.images[DEFAULT_TEXTURE_HANDLE_ID] = default_image;
    texture_assets.textures[DEFAULT_TEXTURE_HANDLE_ID] = default_texture;

    texture_assets.slots[BLANK_TEXTURE_HANDLE_ID] = true;
    texture_assets.image_ready[BLANK_TEXTURE_HANDLE_ID] = true;
    texture_assets.texture_ready[BLANK_TEXTURE_HANDLE_ID] = true;
    texture_assets.images[BLANK_TEXTURE_HANDLE_ID] = blank_image;
    texture_assets.textures[BLANK_TEXTURE_HANDLE_ID] = blank_texture;

    texture_assets.slots[WHITE_TEXTURE_HANDLE_ID] = true;
    texture_assets.image_ready[WHITE_TEXTURE_HANDLE_ID] = true;
    texture_assets.texture_ready[WHITE_TEXTURE_HANDLE_ID] = true;
    texture_assets.images[WHITE_TEXTURE_HANDLE_ID] = white_image;
    texture_assets.textures[WHITE_TEXTURE_HANDLE_ID] = white_texture;
    
    return texture_assets;
}

TextureHandle texture_assets_reserve_texture_slot(TextureAssets* assets) {
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

void texture_assets_put_image(TextureAssets* assets, TextureHandle handle, Image image) {
    assets->images[handle.id] = image;
    assets->image_ready[handle.id] = true;
}

void texture_assets_load_image_unchecked(TextureAssets* assets, TextureHandle handle) {
    assets->images[handle.id] = LoadImageFromTexture(assets->textures[handle.id]);
    assets->image_ready[handle.id] = true;
}

void texture_assets_load_texture_uncheched(TextureAssets* assets, TextureHandle handle) {
    assets->textures[handle.id] = LoadTextureFromImage(assets->images[handle.id]);
    assets->texture_ready[handle.id] = true;
}

void texture_assets_update_texture_unchecked(TextureAssets* assets, TextureHandle handle) {
    UpdateTexture(assets->textures[handle.id], assets->images[handle.id].data);
}

void texture_assets_update_texture_rec_unchecked(TextureAssets* assets, TextureHandle handle, Rectangle rec) {
    UpdateTextureRec(assets->textures[handle.id], rec, assets->images[handle.id].data);
}

void texture_assets_put_image_and_load_texture(TextureAssets* assets, TextureHandle handle, Image image) {
    texture_assets_put_image(assets, handle, image);
    texture_assets_load_texture_uncheched(assets, handle);
}

void texture_assets_load_texture_then_unload_image(TextureAssets* assets, TextureHandle handle, Image image) {
    assets->images[handle.id] = image;
    texture_assets_load_texture_uncheched(assets, handle);
    UnloadImage(image);
    // assets->images[handle.id] = (Image) {0};
}

// -- LOAD IMAGE/TEXTURE -- END --

// -- GET IMAGE

ImageResponse texture_assets_get_image(TextureAssets* assets, TextureHandle handle) {
    if (assets->image_ready[handle.id]) {
        return valid_image_response(&assets->images[handle.id]);
    }
    return null_image_response();
}

Image* texture_assets_get_image_or_default(TextureAssets* assets, TextureHandle handle) {
    if (assets->image_ready[handle.id]) {
        return &assets->images[handle.id];
    }
    return &assets->images[DEFAULT_TEXTURE_HANDLE_ID];
}

Image* texture_assets_get_image_unchecked(TextureAssets* assets, TextureHandle handle) {
    return &assets->images[handle.id];
}

ImageResponse texture_assets_get_image_mut(TextureAssets* assets, TextureHandle handle) {
    if (assets->image_ready[handle.id]) {
        assets->image_changed[handle.id] = true;
        return valid_image_response(&assets->images[handle.id]);
    }
    return null_image_response();
}

Image* texture_assets_get_image_unchecked_mut(TextureAssets* assets, TextureHandle handle) {
    assets->image_changed[handle.id] = true;
    return &assets->images[handle.id];
}

// -- GET IMAGE -- END --

// -- GET TEXTURE

TextureResponse texture_assets_get_texture(TextureAssets* assets, TextureHandle handle) {
    if (assets->texture_ready[handle.id]) {
        return valid_texture_response(&assets->textures[handle.id]);
    }
    return null_texture_response();
}

Texture* texture_assets_get_texture_or_default(TextureAssets* assets, TextureHandle handle) {
    if (assets->texture_ready[handle.id]) {
        return &assets->textures[handle.id];
    }
    return &assets->textures[DEFAULT_TEXTURE_HANDLE_ID];
}

Texture* texture_assets_get_texture_unchecked(TextureAssets* assets, TextureHandle handle) {
    return &assets->textures[handle.id];
}

// -- GET TEXTURE -- END --

// -- UNLOAD --

void texture_assets_unload_image(TextureAssets* assets, TextureHandle handle) {
    if (!assets->image_ready[handle.id]) {
        return;
    }

    assets->image_ready[handle.id] = false;

    Image image = assets->images[handle.id];
    UnloadImage(image);
    // assets->images[handle.id] = (Image) {0};
}

void texture_assets_unload_texture(TextureAssets* assets, TextureHandle handle) {
    if (!assets->texture_ready[handle.id]) {
        return;
    }

    assets->texture_ready[handle.id] = false;

    Texture texture = assets->textures[handle.id];
    UnloadTexture(texture);
    // assets->textures[handle.id] = (Texture) {0};
}

void texture_assets_unload_slot(TextureAssets* assets, TextureHandle handle) {
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
