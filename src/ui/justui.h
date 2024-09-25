#pragma once

#include "raylib.h"

#include "base.h"
#include "memory/memory.h"

#define CUSTOM_UI_ELEMENT_SLOT_COUNT 100

typedef struct {
    uint32 id;
} UIElementId;

/**
 * Variants [0, CUSTOM_UI_ELEMENT_SLOT_COUNT) are reserved for custom elements
 */
typedef enum {
    UIElementType_Area = CUSTOM_UI_ELEMENT_SLOT_COUNT,
    UIElementType_Button,
    UIElementType_SelectionBox,
    UIElementType_Slider,
    UIElementType_Panel,
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

// ----------------
typedef enum {
    UIEvent_BeginHover,
    UIEvent_StayHover,
    UIEvent_EndHover,
    UIEvent_Pressed,
    UIEvent_Released,
    UIEvent_Update,
    UIEvent_Draw,
    UIEvent_DropElement,
} UIEvent;

typedef struct {
    float32 delta_time;
    Vector2 mouse;      // relative to element top-left
    Vector2 element_origin;
} UIEventContext;

void put_ui_handle_vtable_entry(uint32 type_id, void (*handler)(UIElement* elem, UIEvent event, UIEventContext context));
// ----------------

typedef struct {
    BumpAllocator memory;
    uint32 count;
    UIElement** elems;
    UIElement* pressed_element;
    bool active;
} UIElementStore;

typedef struct {
    Color idle_color;
    Color hovered_color;
    bool is_bordered;
    float32 border_thick;
    Color border_color;
} AreaStyle;

typedef struct {
    UIElement elem;
    AreaStyle style;
} Area;

typedef struct {
    Color idle_color;
    Color hovered_color;
    Color pressed_color;
    Color disabled_color;
    //
    bool is_bordered;
    float32 border_thick;
    Color border_color;
    //
    Font title_font;
    float32 title_font_size;
    float32 title_spacing;
    Color title_color;
} ButtonStyle;

typedef struct {
    UIElement elem;
    ButtonStyle style;
    char title[20];
} Button;

void button_consume_click(Button* button);

typedef struct {
    Color selected_color;
    Color unselected_color;
    Color disabled_color;
    //
    bool is_bordered;
    float32 border_thick;
    Color border_color;
    //
    Font title_font;
    float32 title_font_size;
    float32 title_spacing;
    Color title_color;
} SelectionBoxStyle;

typedef struct {
    UIElement elem;
    SelectionBoxStyle style;
    char title[20];
    bool selected;
} SelectionBox;

typedef struct {
    Color line_color;
    Color cursor_color;
    //
    Font title_font;
    float32 title_font_size;
    float32 title_spacing;
    Color title_color;
} SliderStyle;

typedef struct {
    UIElement elem;
    SliderStyle style;
    char title[20];
    float32 low_value;
    float32 high_value;
    float32 cursor; // [0, 1]
} Slider;

float32 get_slider_value(Slider* slider);

typedef struct {
    UIElement elem;
    UIElementStore store;
    bool open;
} Panel;

UIElementStore ui_element_store_new();
UIElementStore ui_element_store_new_active();
void ui_element_store_drop_elements(UIElementStore* store);
void ui_element_store_drop(UIElementStore* store);

void* get_ui_element_unchecked(UIElementStore* store, UIElementId elem_id);
UIElement* get_ui_element(UIElementStore* store, UIElementId elem_id);

// ----------------
UIElementId put_ui_element(UIElementStore* store, UIElement* elem, uint32 size);

UIElementId put_ui_element_area(UIElementStore* store, Area area);
UIElementId put_ui_element_button(UIElementStore* store, Button button);
UIElementId put_ui_element_selection_box(UIElementStore* store, SelectionBox sbox);
UIElementId put_ui_element_slider(UIElementStore* store, Slider slider);
UIElementId put_ui_element_panel(UIElementStore* store, Panel panel);
// ----------------

void SYSTEM_PRE_UPDATE_handle_input_for_ui_store(
    UIElementStore* store
);

void SYSTEM_UPDATE_update_ui_elements(
    UIElementStore* store,
    float32 delta_time
);

void SYSTEM_RENDER_draw_ui_elements(
    UIElementStore* store
);
