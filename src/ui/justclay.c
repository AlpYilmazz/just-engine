#define CLAY_IMPLEMENTATION
#include "clay.h"

#include "base.h"
#include "memory/memory.h"
#include "memory/juststring.h"

#include "justclay.h"

bool reinitialize_clay = false;
void* justclay_arena_memory;

void justclay_handle_errors(Clay_ErrorData errorData) {
    JUST_LOG_WARN("%s\n", errorData.errorText.chars);
    if (errorData.errorType == CLAY_ERROR_TYPE_ELEMENTS_CAPACITY_EXCEEDED) {
        reinitialize_clay = true;
        Clay_SetMaxElementCount(Clay_GetMaxElementCount() * 2);
    }
    else if (errorData.errorType == CLAY_ERROR_TYPE_TEXT_MEASUREMENT_CAPACITY_EXCEEDED) {
        reinitialize_clay = true;
        Clay_SetMaxMeasureTextCacheWordCount(Clay_GetMaxMeasureTextCacheWordCount() * 2);
    }
}

Clay_Dimensions justclay_measure_text(Clay_StringSlice text, Clay_TextElementConfig* config, void* user_data) {
    FontList* font_list = (FontList*) user_data;

    // Measure string size for Font
    Clay_Dimensions textSize = { 0 };

    float32 max_text_width = 0.0f;
    float32 line_text_width = 0;
    int32 max_line_char_count = 0;
    int32 line_char_count = 0;

    float32 text_height = config->fontSize;
    Font font_to_use = font_list->fonts[config->fontId];
    // Font failed to load, likely the fonts are in the wrong place relative to the execution dir.
    // RayLib ships with a default font, so we can continue with that built in one. 
    if (!font_to_use.glyphs) {
        font_to_use = GetFontDefault();
    }

    float32 scale_factor = config->fontSize / (float32)font_to_use.baseSize;

    for (int32 i = 0; i < text.length; ++i, line_char_count++) {
        if (text.chars[i] == '\n') {
            max_text_width = fmax(max_text_width, line_text_width);
            max_line_char_count = CLAY__MAX(max_line_char_count, line_char_count);
            line_text_width = 0;
            line_char_count = 0;
            continue;
        }
        int32 index = text.chars[i] - 32;
        if (font_to_use.glyphs[index].advanceX != 0) line_text_width += font_to_use.glyphs[index].advanceX;
        else line_text_width += (font_to_use.recs[index].width + font_to_use.glyphs[index].offsetX);
    }

    max_text_width = fmax(max_text_width, line_text_width);
    max_line_char_count = CLAY__MAX(max_line_char_count, line_char_count);

    textSize.width = max_text_width * scale_factor + (line_char_count * config->letterSpacing);
    textSize.height = text_height;

    return textSize;
}

void initialize_justclay(FontList* font_list) {
    uint64_t totalMemorySize = Clay_MinMemorySize();

    justclay_arena_memory = std_malloc(totalMemorySize);
    Clay_Arena clayMemory = Clay_CreateArenaWithCapacityAndMemory(totalMemorySize, justclay_arena_memory);

    Clay_Initialize(clayMemory, (Clay_Dimensions) { (float32)GetScreenWidth(), (float32)GetScreenHeight() }, (Clay_ErrorHandler) { justclay_handle_errors, 0 });
    Clay_SetMeasureTextFunction(justclay_measure_text, font_list);
}

void SYSTEM_PRE_PREPARE_reinit_justclay_if_necessary() {
    if (reinitialize_clay) {
        uint64_t totalMemorySize = Clay_MinMemorySize();

        std_free(justclay_arena_memory);
        justclay_arena_memory = std_malloc(totalMemorySize);
        Clay_Arena clayMemory = Clay_CreateArenaWithCapacityAndMemory(totalMemorySize, justclay_arena_memory);

        Clay_Initialize(clayMemory, (Clay_Dimensions) { (float32)GetScreenWidth(), (float32)GetScreenHeight() }, (Clay_ErrorHandler) { justclay_handle_errors, 0 });
        reinitialize_clay = false;
    }
}

void SYSTEM_PRE_PREPARE_justclay_set_state(
    Vector2 mouse_position,
    bool mouse_down
) {
    Clay_Vector2 clay_mouse_position = RAYLIB_VECTOR2_TO_CLAY_VECTOR2(mouse_position);
    Clay_SetPointerState(clay_mouse_position, mouse_down);
    Clay_SetLayoutDimensions((Clay_Dimensions) { (float)GetScreenWidth(), (float)GetScreenHeight() });
}

void SYSTEM_POST_PREPARE_justclay_update_scroll_containers(
    Vector2 mouse_wheel_delta,
    float32 delta_time
) {
    Clay_Vector2 clay_mouse_wheel_delta = RAYLIB_VECTOR2_TO_CLAY_VECTOR2(mouse_wheel_delta);
    Clay_UpdateScrollContainers(true, clay_mouse_wheel_delta, delta_time);
}

String TEMP_STRING;

void SYSTEM_RENDER_justclay_ui(
    TextureAssets* RES_TEXTURE_ASSETS,
    FontList RES_FONT_LIST,
    Clay_RenderCommandArray renderCommands
) {
    for (int32 j = 0; j < renderCommands.length; j++) {
        Clay_RenderCommand *renderCommand = Clay_RenderCommandArray_Get(&renderCommands, j);
        Clay_BoundingBox boundingBox = {
            .x = roundf(renderCommand->boundingBox.x),
            .y = roundf(renderCommand->boundingBox.y),
            .width = roundf(renderCommand->boundingBox.width),
            .height = roundf(renderCommand->boundingBox.height),
        };
        
        switch (renderCommand->commandType) {
        case CLAY_RENDER_COMMAND_TYPE_TEXT: {
            Clay_TextRenderData *textData = &renderCommand->renderData.text;
            Font fontToUse = RES_FONT_LIST.fonts[textData->fontId];

            // Raylib uses standard C strings so isn't compatible with cheap slices, we need to clone the string to append null terminator
            clear_string(&TEMP_STRING);
            dynarray_reserve_custom(TEMP_STRING, .str, textData->stringContents.length);
            string_nappend_cstr(&TEMP_STRING, (char*) textData->stringContents.chars, textData->stringContents.length);

            DrawTextEx(
                fontToUse,
                TEMP_STRING.cstr,
                (Vector2){boundingBox.x, boundingBox.y},
                (float)textData->fontSize,
                (float)textData->letterSpacing,
                CLAY_COLOR_TO_RAYLIB_COLOR(textData->textColor)
            );

            break;
        }
        case CLAY_RENDER_COMMAND_TYPE_IMAGE: {
            TextureHandle texture_handle = *(TextureHandle*)renderCommand->renderData.image.imageData;
            Texture* texture = texture_assets_get_texture_or_default(RES_TEXTURE_ASSETS, texture_handle);

            Clay_Color tintColor = renderCommand->renderData.image.backgroundColor;
            if (tintColor.r == 0 && tintColor.g == 0 && tintColor.b == 0 && tintColor.a == 0) {
                tintColor = (Clay_Color) { 255, 255, 255, 255 };
            }
            DrawTexturePro(
                *texture,
                (Rectangle) { 0, 0, texture->width, texture->height },
                CLAY_RECTANGLE_TO_RAYLIB_RECTANGLE(boundingBox),
                (Vector2) {},
                0,
                CLAY_COLOR_TO_RAYLIB_COLOR(tintColor)
            );
            break;
        }
        case CLAY_RENDER_COMMAND_TYPE_SCISSOR_START: {
            BeginScissorMode((int)roundf(boundingBox.x), (int)roundf(boundingBox.y), (int)roundf(boundingBox.width), (int)roundf(boundingBox.height));
            break;
        }
        case CLAY_RENDER_COMMAND_TYPE_SCISSOR_END: {
            EndScissorMode();
            break;
        }
        case CLAY_RENDER_COMMAND_TYPE_RECTANGLE: {
            Clay_RectangleRenderData *config = &renderCommand->renderData.rectangle;
            if (config->cornerRadius.topLeft > 0) {
                float radius = (config->cornerRadius.topLeft * 2) / (float)((boundingBox.width > boundingBox.height) ? boundingBox.height : boundingBox.width);
                DrawRectangleRounded(
                    CLAY_RECTANGLE_TO_RAYLIB_RECTANGLE(boundingBox),
                    radius,
                    8,
                    CLAY_COLOR_TO_RAYLIB_COLOR(config->backgroundColor)
                );
            } else {
                DrawRectangle(boundingBox.x, boundingBox.y, boundingBox.width, boundingBox.height, CLAY_COLOR_TO_RAYLIB_COLOR(config->backgroundColor));
            }
            break;
        }
        case CLAY_RENDER_COMMAND_TYPE_BORDER: {
            Clay_BorderRenderData *config = &renderCommand->renderData.border;
            // Left border
            if (config->width.left > 0) {
                DrawRectangle((int)roundf(boundingBox.x), (int)roundf(boundingBox.y + config->cornerRadius.topLeft), (int)config->width.left, (int)roundf(boundingBox.height - config->cornerRadius.topLeft - config->cornerRadius.bottomLeft), CLAY_COLOR_TO_RAYLIB_COLOR(config->color));
            }
            // Right border
            if (config->width.right > 0) {
                DrawRectangle((int)roundf(boundingBox.x + boundingBox.width - config->width.right), (int)roundf(boundingBox.y + config->cornerRadius.topRight), (int)config->width.right, (int)roundf(boundingBox.height - config->cornerRadius.topRight - config->cornerRadius.bottomRight), CLAY_COLOR_TO_RAYLIB_COLOR(config->color));
            }
            // Top border
            if (config->width.top > 0) {
                DrawRectangle((int)roundf(boundingBox.x + config->cornerRadius.topLeft), (int)roundf(boundingBox.y), (int)roundf(boundingBox.width - config->cornerRadius.topLeft - config->cornerRadius.topRight), (int)config->width.top, CLAY_COLOR_TO_RAYLIB_COLOR(config->color));
            }
            // Bottom border
            if (config->width.bottom > 0) {
                DrawRectangle((int)roundf(boundingBox.x + config->cornerRadius.bottomLeft), (int)roundf(boundingBox.y + boundingBox.height - config->width.bottom), (int)roundf(boundingBox.width - config->cornerRadius.bottomLeft - config->cornerRadius.bottomRight), (int)config->width.bottom, CLAY_COLOR_TO_RAYLIB_COLOR(config->color));
            }
            if (config->cornerRadius.topLeft > 0) {
                DrawRing((Vector2) { roundf(boundingBox.x + config->cornerRadius.topLeft), roundf(boundingBox.y + config->cornerRadius.topLeft) }, roundf(config->cornerRadius.topLeft - config->width.top), config->cornerRadius.topLeft, 180, 270, 10, CLAY_COLOR_TO_RAYLIB_COLOR(config->color));
            }
            if (config->cornerRadius.topRight > 0) {
                DrawRing((Vector2) { roundf(boundingBox.x + boundingBox.width - config->cornerRadius.topRight), roundf(boundingBox.y + config->cornerRadius.topRight) }, roundf(config->cornerRadius.topRight - config->width.top), config->cornerRadius.topRight, 270, 360, 10, CLAY_COLOR_TO_RAYLIB_COLOR(config->color));
            }
            if (config->cornerRadius.bottomLeft > 0) {
                DrawRing((Vector2) { roundf(boundingBox.x + config->cornerRadius.bottomLeft), roundf(boundingBox.y + boundingBox.height - config->cornerRadius.bottomLeft) }, roundf(config->cornerRadius.bottomLeft - config->width.bottom), config->cornerRadius.bottomLeft, 90, 180, 10, CLAY_COLOR_TO_RAYLIB_COLOR(config->color));
            }
            if (config->cornerRadius.bottomRight > 0) {
                DrawRing((Vector2) { roundf(boundingBox.x + boundingBox.width - config->cornerRadius.bottomRight), roundf(boundingBox.y + boundingBox.height - config->cornerRadius.bottomRight) }, roundf(config->cornerRadius.bottomRight - config->width.bottom), config->cornerRadius.bottomRight, 0.1, 90, 10, CLAY_COLOR_TO_RAYLIB_COLOR(config->color));
            }
            break;
        }
        case CLAY_RENDER_COMMAND_TYPE_CUSTOM: {
            Clay_CustomRenderData* config = &renderCommand->renderData.custom;
            ClayCustomElement* custom_element = (ClayCustomElement*) config->customData;
            if (!custom_element) continue;
            switch (custom_element->type) {
            case CLAY_CUSTOM_ELEMENT_CHECKBOX: {
                ClayCustomElement_CheckBox checkbox = custom_element->custom_data.checkbox;

                Color color = RED;
                if (checkbox.active) {
                    color = GREEN;
                }
                DrawRectangle(boundingBox.x, boundingBox.y, boundingBox.width, boundingBox.height, color);
                break;
            }
            default:
                PANIC("Error: unhandled custom element type.\n");
            }
            break;
        }
        default:
            PANIC("Error: unhandled render command.\n");
        }
    }
}