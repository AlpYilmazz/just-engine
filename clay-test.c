#include "justengine.h"

#define FONT_ID_BODY_24 0
#define FONT_ID_BODY_16 1

#define CLAY_GRAY               (Clay_Color) { 43, 41, 51, 255 }
#define CLAY_CONTENT_BG_GRAY    (Clay_Color) { 90, 90, 90, 255 }

void button_on_hover(
    Clay_ElementId elementId,
    Clay_PointerData pointerData,
    intptr_t userData
) {
    char* state = "";
    switch (pointerData.state) {
    case CLAY_POINTER_DATA_PRESSED_THIS_FRAME:
        state = "CLAY_POINTER_DATA_PRESSED_THIS_FRAME";
        break;
    case CLAY_POINTER_DATA_PRESSED:
        state = "CLAY_POINTER_DATA_PRESSED";
        break;
    case CLAY_POINTER_DATA_RELEASED_THIS_FRAME:
        state = "CLAY_POINTER_DATA_RELEASED_THIS_FRAME";
        break;
    case CLAY_POINTER_DATA_RELEASED:
        state = "CLAY_POINTER_DATA_RELEASED";
        break;
    }

    String element_id_str = string_new();
    string_nappend_cstr(&element_id_str, (char*) elementId.stringId.chars, elementId.stringId.length);
    printf("element: %s, state: %s\n", element_id_str.cstr, state);
}

void RenderButton(Clay_String text) {
    CLAY({
        .id = CLAY_SID(text),
        .layout = {
            .padding = CLAY_PADDING_ALL(16),
            .sizing = {
                .width = CLAY_SIZING_GROW(0),
                .height = CLAY_SIZING_FIT(0),
            },
        },
        .backgroundColor = Clay_Hovered() ? RAYLIB_COLOR_TO_CLAY_COLOR(GREEN) : RAYLIB_COLOR_TO_CLAY_COLOR(RAYWHITE),
        .cornerRadius = 10,
    }) {
        Clay_OnHover(button_on_hover, (uintptr_t) NULL);
        CLAY_TEXT(text, CLAY_TEXT_CONFIG({
            .fontId = FONT_ID_BODY_24,
            .fontSize = 24,
            .textColor = RAYLIB_COLOR_TO_CLAY_COLOR(BLACK),
        }));
    }
}

int main() {
    SET_LOG_LEVEL(LOG_LEVEL_ERROR);
    // SET_LOG_LEVEL(LOG_LEVEL_WARN);
    // SET_LOG_LEVEL(LOG_LEVEL_TRACE);

    InitWindow(1000, 1000, "Clay");
    SetTargetFPS(60);
    
    TextureAssets RES_TEXTURE_ASSETS = new_texture_assets();
    Events(TextureAssetEvent) RES_TEXTURE_ASSET_EVENTS = events_create(TextureAssetEvent)();

    ThreadPool* RES_THREAD_POOL = thread_pool_create(4, 100);
    FileImageServer RES_FILE_IMAGE_SERVER = {
        .RES_threadpool = RES_THREAD_POOL,
        .RES_texture_assets = &RES_TEXTURE_ASSETS,
        .RES_texture_assets_events = &RES_TEXTURE_ASSET_EVENTS,
        .asset_folder = "./test-assets",
    };

    FontList RES_FONT_LIST = {0};
    dynarray_reserve_custom(RES_FONT_LIST, .fonts, 2);
    {
        Font font_body_24 = LoadFontEx("./test-assets/Roboto-Regular.ttf", 48, 0, 400);
        SetTextureFilter(font_body_24.texture, TEXTURE_FILTER_BILINEAR);
        dynarray_push_back_custom(RES_FONT_LIST, .fonts, font_body_24);
    }

    {
        Font font_body_16 = LoadFontEx("./test-assets/Roboto-Regular.ttf", 32, 0, 400);
        SetTextureFilter(font_body_16.texture, TEXTURE_FILTER_BILINEAR);
        dynarray_push_back_custom(RES_FONT_LIST, .fonts, font_body_16);
    }

    initialize_justclay(&RES_FONT_LIST);

    while (!WindowShouldClose()) {
        float32 delta_time = GetFrameTime();

        bool mouse_down = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
        Vector2 mouse_position = GetMousePosition();
        Vector2 mouse_wheel_delta = GetMouseWheelMoveV();

        SYSTEM_PRE_PREPARE_reinit_justclay_if_necessary();
        SYSTEM_PRE_PREPARE_justclay_set_state(mouse_position, mouse_down);

        SYSTEM_POST_PREPARE_justclay_update_scroll_containers(mouse_wheel_delta, delta_time);

        Clay_BeginLayout();

        Clay_Sizing layoutExpand = {
            .width = CLAY_SIZING_GROW(0),
            .height = CLAY_SIZING_GROW(0)
        };

        Clay_Color contentBackgroundColor = { 90, 90, 90, 255 };

        // Build UI here
        CLAY({
            .id = CLAY_ID("OuterContainer"),
            .backgroundColor = CLAY_GRAY,
            .layout = {
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
                .sizing = layoutExpand,
                .padding = CLAY_PADDING_ALL(16),
                .childGap = 16
            },
        }) {
            CLAY({
                .id = CLAY_ID("ButtonArea"),
                .backgroundColor = CLAY_CONTENT_BG_GRAY,
                .layout = {
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    .sizing = {
                        .width = CLAY_SIZING_PERCENT(0.2),
                        .height = CLAY_SIZING_FIT(0),
                    },
                    .padding = CLAY_PADDING_ALL(16),
                    .childGap = 16
                },
            }) {
                CLAY({
                    .id = CLAY_ID("ButtonAreaFloat"),
                    .backgroundColor = CLAY_CONTENT_BG_GRAY,
                    .layout = {
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                        .sizing = {
                            .width = CLAY_SIZING_FIXED(300),
                            .height = CLAY_SIZING_FIT(0),
                        },
                        .padding = CLAY_PADDING_ALL(16),
                        .childGap = 16
                    },
                    .floating = {
                        .attachTo = CLAY_ATTACH_TO_PARENT,
                        .attachPoints = {
                            .parent = CLAY_ATTACH_POINT_CENTER_TOP
                        },
                        .zIndex = 2,
                    },
                }) {
                    RenderButton(CLAY_STRING("NewFloat"));
                    RenderButton(CLAY_STRING("OpenFloat"));
                    RenderButton(CLAY_STRING("CloseFloat"));
                };
                CLAY({
                    .id = CLAY_ID("ButtonAreaFloat2"),
                    .backgroundColor = CLAY_CONTENT_BG_GRAY,
                    .layout = {
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                        .sizing = {
                            .width = CLAY_SIZING_FIXED(300),
                            .height = CLAY_SIZING_FIT(0),
                        },
                        .padding = CLAY_PADDING_ALL(16),
                        .childGap = 16
                    },
                    .floating = {
                        .attachTo = CLAY_ATTACH_TO_PARENT,
                        .attachPoints = {
                            .parent = CLAY_ATTACH_POINT_RIGHT_TOP
                        },
                        .zIndex = 1,
                    },
                }) {
                    RenderButton(CLAY_STRING("NewFloat2"));
                    RenderButton(CLAY_STRING("OpenFloat2"));
                    RenderButton(CLAY_STRING("CloseFloat2"));
                };

                RenderButton(CLAY_STRING("New"));
                RenderButton(CLAY_STRING("Open"));
                RenderButton(CLAY_STRING("Close"));
            };
        };

        Clay_RenderCommandArray clay_render_commands = Clay_EndLayout();

        BeginDrawing();
        ClearBackground(RAYWHITE);

        SYSTEM_RENDER_justclay_ui(
            &RES_TEXTURE_ASSETS,
            RES_FONT_LIST,
            clay_render_commands
        );

        EndDrawing();
    }

    return 0;
}