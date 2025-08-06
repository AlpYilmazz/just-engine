#pragma once

#include <math.h>

#include "clay.h"

#include "assets/asset.h"

#define RAYLIB_VECTOR2_TO_CLAY_VECTOR2(vector) (Clay_Vector2) { .x = (vector).x, .y = (vector).y }
#define CLAY_RECTANGLE_TO_RAYLIB_RECTANGLE(rectangle) (Rectangle) { .x = (rectangle).x, .y = (rectangle).y, .width = (rectangle).width, .height = (rectangle).height }
#define CLAY_COLOR_TO_RAYLIB_COLOR(color) (Color) { .r = (unsigned char)roundf((color).r), .g = (unsigned char)roundf((color).g), .b = (unsigned char)roundf((color).b), .a = (unsigned char)roundf((color).a) }
#define RAYLIB_COLOR_TO_CLAY_COLOR(color) (Clay_Color) { .r = (float)(color).r, .g = (float)(color).g, .b = (float)(color).b, .a = (float)((color).a) }

Clay_String string_to_clay_string(String string);
String clay_string_to_string(Clay_String clay_string);

typedef enum {
    CLAY_CUSTOM_ELEMENT_CHECKBOX,
} ClayCustomElementType;

typedef struct {
    bool active;
} ClayCustomElement_CheckBox;

typedef struct {
    ClayCustomElementType type;
    union {
        ClayCustomElement_CheckBox checkbox;
    } custom_data;
} ClayCustomElement;

typedef struct {
    usize count;
    usize capacity;
    Font* fonts;
} FontList;

void initialize_justclay(FontList* font_list);

// -- SYSTEM --

void SYSTEM_PRE_PREPARE_reinit_justclay_if_necessary();

void SYSTEM_PRE_PREPARE_justclay_set_state(
    Vector2 mouse_position,
    bool mouse_down
);

void SYSTEM_POST_PREPARE_justclay_update_scroll_containers(
    Vector2 mouse_wheel_delta,
    float32 delta_time
);

void SYSTEM_RENDER_justclay_ui(
    TextureAssets* RES_TEXTURE_ASSETS,
    FontList RES_FONT_LIST,
    Clay_RenderCommandArray renderCommands
);