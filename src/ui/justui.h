#pragma once

#include "raylib.h"

#include "base.h"
#include "memory/memory.h"

typedef struct {
    uint32 id;
} UIElementId;

typedef enum {
    UIElementType_Area,
    UIElementType_Layout,
    UIElementType_TextInput,
    UIElementType_Selection,
    UIElementType_Button,
} UIElementType;

typedef struct {
    // bool idle;
    bool hover;
    bool pressed;
    bool just_clicked;
    Vector2 click_point_relative;
} UIElementState;

/**
 * Specific UI Elements have to start with UIElement field
 */
typedef struct {
    UIElementId id;
    UIElementType type;
    UIElementState state;
    Anchor anchor;
    Vector2 position;
    RectSize size;
    bool disabled;
} UIElement;

bool ui_element_hovered(UIElement* elem, Vector2 mouse);
Vector2 ui_element_relative_point(UIElement* elem, Vector2 point);

typedef struct {
    Color idle_color;
    Color hovered_color;
    Color pressed_color;
    Color disabled_color;
    bool is_bordered;
    float32 border_thick;
    Color border_color;
} ButtonStyle;

typedef struct {
    UIElement elem;
    ButtonStyle style;
    char title[20];
} Button;

Button button_new(
    UIElement elem,
    ButtonStyle style,
    char* title
);

void button_consume_click(Button* button);

void handle_begin_hover_button(Button* button, Vector2 mouse);
void handle_end_hover_button(Button* button, Vector2 mouse);
void handle_pressed_button(Button* button, Vector2 mouse);
void handle_released_button(Button* button, Vector2 mouse);
void draw_button(Button* button);

// ----------------

void ui_element_handle_begin_hover(UIElement* elem, Vector2 mouse);
void ui_element_handle_end_hover(UIElement* elem, Vector2 mouse);
void ui_element_handle_pressed(UIElement* elem, Vector2 mouse);
void ui_element_handle_released(UIElement* elem, Vector2 mouse);
void ui_element_draw(UIElement* elem);

// ----------------

typedef struct {
    BumpAllocator memory;
    uint32 count;
    UIElement* elems[100];
    UIElement* pressed_element;
    bool active;
} UIElementStore;

UIElementStore ui_element_store_new();
UIElementStore ui_element_store_new_active();
void ui_element_store_drop(UIElementStore* store);

UIElementId put_ui_element_button(UIElementStore* store, Button button);

void* get_ui_element_unchecked(UIElementStore* store, UIElementId elem_id);
UIElement* get_ui_element(UIElementStore* store, UIElementId elem_id);

Button* get_ui_element_button(UIElementStore* store, UIElementId elem_id);

void SYSTEM_PRE_UPDATE_handle_input_for_ui_store(
    UIElementStore* store
);

void SYSTEM_RENDER_draw_ui_elements(
    UIElementStore* store
);
