#pragma once

#include "raylib.h"

#include "base.h"
#include "memory/memory.h"

#include "uilayout.h"

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
    UIElementType_ChoiceList,
    UIElementType_Panel,
} UIElementType;

typedef struct {
    // bool idle;
    bool on_hover;
    bool on_press;
    //
    bool just_begin_hover;
    bool just_end_hover;
    bool just_pressed;
    //
    bool just_clicked;
    Vector2 click_point_relative;
} UIElementState;

// TODO: maybe utilize bitmask
// typedef enum {
//     ON_HOVER,
//     ON_PRESS,
//     //
//     JUST_BEGIN_HOVER,
//     JUST_END_HOVER,
//     JUST_PRESSED,
//     //
//     JUST_CLICKED,
// } UIElementStateEnum;

/**
 * Specific UI Elements have to start with UIElement field
 */
typedef struct {
    UIElementId id;
    UIElementType type;
    UIElementState state;
    uint32 layer;
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
    uint32 layer;
    uint32 index;
} ElementSort;

typedef struct {
    BumpAllocator memory;
    byte* memory_reset_cursor;
    uint32 count;
    UIElement** elems;
    ElementSort* layer_sort;
    UIElement* pressed_element;
    bool active;
} UIElementStore;

// ----------------
typedef struct {
    bool is_bordered;
    float32 thick;
    Color color;
} UIBorderStyle;

typedef struct {
    Font font;
    float32 font_size;
    float32 spacing;
    Color color;
} UITextStyle;
// ----------------

typedef struct {
    Color idle_color;
    Color hovered_color;
    UIBorderStyle border;
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
    UIBorderStyle border;
    UITextStyle title;
} ButtonStyle;

typedef struct {
    UIElement elem;
    ButtonStyle style;
    Vector2 draw_offset;
    char title[20];
} Button;

void button_consume_click(Button* button);

typedef struct {
    Color selected_color;
    Color unselected_color;
    Color disabled_color;
    UIBorderStyle border;
    UITextStyle title;
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
    UIBorderStyle border;
    UITextStyle title;
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
    Color selected_color;
    Color unselected_color;
    Color disabled_color;
    UIBorderStyle border;
    UITextStyle title;
} ChoiceListStyle;

typedef struct {
    uint32 id;
    char title[20];
} ChoiceListOption;

typedef struct {
    UIElement elem;
    ChoiceListStyle style;
    GridLayout layout;
    Option(uint32) hovered_option_index;
    uint32 selected_option_id;
    uint32 option_count;
    ChoiceListOption options[20];
} ChoiceList;

typedef struct {
    UIElement elem;
    UIElementStore store;
    bool open;
} Panel;

// ----------------
Area make_ui_area(Area area);
Button make_ui_button(Button button);
SelectionBox make_ui_selection_box(SelectionBox selection_box);
Slider make_ui_slider(Slider slider);
ChoiceList make_ui_choice_list(ChoiceList choice_list);
Panel make_ui_panel(Panel panel);
// ----------------

UIElementStore ui_element_store_new_with_count_hint(uint32 count_hint);
UIElementStore ui_element_store_new();
UIElementStore ui_element_store_new_active_with_count_hint(uint32 count_hint);
UIElementStore ui_element_store_new_active();
void ui_element_store_drop_elements(UIElementStore* store);
void ui_element_store_drop(UIElementStore* store);
void ui_element_store_clear(UIElementStore* store);

void* get_ui_element_unchecked(UIElementStore* store, UIElementId elem_id);
UIElement* get_ui_element(UIElementStore* store, UIElementId elem_id);

// ----------------
UIElementId put_ui_element(UIElementStore* store, UIElement* elem, MemoryLayout layout);

UIElementId put_ui_element_area(UIElementStore* store, Area area);
UIElementId put_ui_element_button(UIElementStore* store, Button button);
UIElementId put_ui_element_selection_box(UIElementStore* store, SelectionBox sbox);
UIElementId put_ui_element_slider(UIElementStore* store, Slider slider);
UIElementId put_ui_element_choice_list(UIElementStore* store, ChoiceList choice_list);
UIElementId put_ui_element_panel(UIElementStore* store, Panel panel);
// ----------------

void SYSTEM_INPUT_handle_input_for_ui_store(
    UIElementStore* store
);

void SYSTEM_UPDATE_update_ui_elements(
    UIElementStore* store,
    float32 delta_time
);

void SYSTEM_RENDER_draw_ui_elements(
    UIElementStore* store
);
