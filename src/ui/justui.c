#include "string.h"

#include "raylib.h"
#include "raymath.h"

#include "base.h"
#include "logging.h"
#include "memory/memory.h"

#include "justui.h"

// UIEvent
// UIEventContext

typedef struct {
    void (*fn)(UIElement* elem, UIEvent event, UIEventContext context);
} UIHandleFunction;

static UIHandleFunction UI_HANDLE_VTABLE[CUSTOM_UI_ELEMENT_SLOT_COUNT] = {0};

void put_ui_handle_vtable_entry(uint32 type_id, void (*handler)(UIElement* elem, UIEvent event, UIEventContext context)) {
    UI_HANDLE_VTABLE[type_id] = (UIHandleFunction) { .fn = handler };
}

// UIElementId
// UIElementType
// UIElementState
// UIElement

bool ui_element_hovered(UIElement* elem, Vector2 mouse) {
    // TODO: maybe precalculate top_left and recalculate when necessary
    Vector2 top_left = find_rectangle_top_left(elem->anchor, elem->position, elem->size);
    return top_left.x <= mouse.x && mouse.x <= top_left.x + elem->size.width
        && top_left.y <= mouse.y && mouse.y <= top_left.y + elem->size.height;
}

Vector2 ui_element_relative_point(UIElement* elem, Vector2 point) {
    return Vector2Subtract(point, elem->anchor.origin);
}

// AreaStyle
// Area

void ui_draw_area(Area* area) {
    UIElement* elem = &area->elem;
    AreaStyle* style = &area->style;

    Vector2 top_left = find_rectangle_top_left(elem->anchor, elem->position, elem->size);

    Rectangle rect = {
        .x = top_left.x,
        .y = top_left.y,
        .width = elem->size.width,
        .height = elem->size.height,
    };

    Color color = style->idle_color;
    if (elem->state.hover) {
        color = style->hovered_color;
    }

    DrawRectangleRec(rect, color);

    if (style->is_bordered) {
        DrawRectangleLinesEx(rect, style->border_thick, style->border_color);
    }
}

void ui_handle_area(Area* area, UIEvent event, UIEventContext context) {
    switch (event) {
    case UIEvent_BeginHover:
        area->elem.state.hover = true;
        break;
    case UIEvent_EndHover:
        area->elem.state.hover = false;
        break;
    case UIEvent_Pressed:
        break;
    case UIEvent_Released:
        break;
    case UIEvent_Update:
        break;
    case UIEvent_Draw:
        ui_draw_area(area);
        break;
    }
}

// ButtonStyle
// Button

void button_consume_click(Button* button) {
    button->elem.state.just_clicked = false;
}

void ui_draw_button(Button* button) {
    UIElement* elem = &button->elem;
    ButtonStyle* style = &button->style;

    Vector2 top_left = find_rectangle_top_left(elem->anchor, elem->position, elem->size);
    Vector2 mid = Vector2Add(top_left, Vector2Scale(rectsize_into_v2(elem->size), 0.5));

    Rectangle rect = {
        .x = top_left.x,
        .y = top_left.y,
        .width = elem->size.width,
        .height = elem->size.height,
    };

    Color color = style->idle_color;
    if (elem->disabled) {
        color = style->disabled_color;
    }
    else { // if (!elem->disabled)
        if (elem->state.pressed) {
            color = style->pressed_color;
        }
        else if (elem->state.hover) {
            color = style->hovered_color;
        }
    }

    DrawRectangleRec(rect, color);
    
    if (style->is_bordered) {
        DrawRectangleLinesEx(rect, style->border_thick, style->border_color);
    }

    // Draw Text [title]
    Vector2 text_size = MeasureTextEx(style->title_font, button->title, style->title_font_size, style->title_spacing);
    Vector2 text_pos = Vector2Subtract(mid, Vector2Scale(text_size, 0.5));

    DrawTextEx(style->title_font, button->title, text_pos, style->title_font_size, style->title_spacing, style->title_color);
}

void ui_handle_button(Button* button, UIEvent event, UIEventContext context) {
    switch (event) {
    case UIEvent_BeginHover:
        button->elem.state.hover = true;
        break;
    case UIEvent_EndHover:
        button->elem.state.hover = false;
        break;
    case UIEvent_Pressed:
        button->elem.state.pressed = true;
        break;
    case UIEvent_Released:
        if (button->elem.state.hover && button->elem.state.pressed) {
            button->elem.state.pressed = false;
            button->elem.state.just_clicked = true;
            button->elem.state.click_point_relative = context.mouse;
        }
        break;
    case UIEvent_Update:
        break;
    case UIEvent_Draw:
        ui_draw_button(button);
        break;
    }
}

// SelectionBoxStyle
// SelectionBox

void ui_draw_selection_box(SelectionBox* sbox) {
    UIElement* elem = &sbox->elem;
    SelectionBoxStyle* style = &sbox->style;

    Vector2 top_left = find_rectangle_top_left(elem->anchor, elem->position, elem->size);
    Vector2 mid = Vector2Add(top_left, Vector2Scale(rectsize_into_v2(elem->size), 0.5));

    Rectangle rect = {
        .x = top_left.x,
        .y = top_left.y,
        .width = elem->size.width,
        .height = elem->size.height,
    };
    Color color = elem->disabled ? style->disabled_color
        : sbox->selected ? style->selected_color : style->unselected_color;

    DrawRectangleRec(rect, color);
    
    if (style->is_bordered) {
        DrawRectangleLinesEx(rect, style->border_thick, style->border_color);
    }

    // Draw Text [title]
    Vector2 text_size = MeasureTextEx(style->title_font, sbox->title, style->title_font_size, style->title_spacing);
    Vector2 text_pos = Vector2Subtract(mid, Vector2Scale(text_size, 0.5));

    DrawTextEx(style->title_font, sbox->title, text_pos, style->title_font_size, style->title_spacing, style->title_color);
}

void ui_handle_selection_box(SelectionBox* sbox, UIEvent event, UIEventContext context) {
    switch (event) {
    case UIEvent_BeginHover:
        sbox->elem.state.hover = true;
        break;
    case UIEvent_EndHover:
        sbox->elem.state.hover = false;
        break;
    case UIEvent_Pressed:
        sbox->elem.state.pressed = true;
        break;
    case UIEvent_Released:
        if (sbox->elem.state.hover && sbox->elem.state.pressed) {
            sbox->selected ^= 1;
            sbox->elem.state.pressed = false;
            sbox->elem.state.just_clicked = true;
            sbox->elem.state.click_point_relative = context.mouse;
        }
        break;
    case UIEvent_Update:
        break;
    case UIEvent_Draw:
        ui_draw_selection_box(sbox);
        break;
    }
}

// SliderStyle
// Slider

float32 get_slider_value(Slider* slider) {
    return slider->low_value + (slider->high_value - slider->low_value) * slider->cursor;
}

void ui_update_slider(Slider* slider, Vector2 mouse, float32 delta_time) {
    UIElement* elem = &slider->elem;
    if (elem->state.pressed) {
        Vector2 top_left = find_rectangle_top_left(elem->anchor, elem->position, elem->size);
        Vector2 global_mouse = Vector2Add(elem->anchor.origin, mouse);

        float32 cursor_radius = elem->size.height/2.0;

        float32 start_x = top_left.x + cursor_radius;
        float32 width = elem->size.width - 2*cursor_radius;

        float32 mouse_x = global_mouse.x - start_x;

        slider->cursor = Clamp(mouse_x / width, 0.0, 1.0);
    }
}

void ui_draw_slider(Slider* slider) {
    UIElement* elem = &slider->elem;
    SliderStyle* style = &slider->style;

    Vector2 size_vec = rectsize_into_v2(elem->size);
    Vector2 top_left = find_rectangle_top_left(elem->anchor, elem->position, elem->size);
    Vector2 bottom_right = Vector2Add(top_left, size_vec);
    Vector2 mid = Vector2Add(top_left, Vector2Scale(size_vec, 0.5));

    float32 cursor_radius = elem->size.height/2.0;

    Vector2 start_pos = { top_left.x + cursor_radius, mid.y };
    Vector2 end_pos = { bottom_right.x - cursor_radius, mid.y };
    DrawLineEx(start_pos, end_pos, 2, style->line_color);

    Vector2 cursor_pos = {
        .x = start_pos.x + (end_pos.x - start_pos.x) * slider->cursor,
        .y = mid.y,
    };
    DrawCircleV(cursor_pos, cursor_radius, style->cursor_color);
}

void ui_handle_slider(Slider* slider, UIEvent event, UIEventContext context) {
    switch (event) {
    case UIEvent_BeginHover:
        slider->elem.state.hover = true;
        break;
    case UIEvent_EndHover:
        slider->elem.state.hover = false;
        break;
    case UIEvent_Pressed:
        slider->elem.state.pressed = true;
        break;
    case UIEvent_Released:
        slider->elem.state.pressed = false;
        break;
    case UIEvent_Update:
        ui_update_slider(slider, context.mouse, context.delta_time);
        break;
    case UIEvent_Draw:
        ui_draw_slider(slider);
        break;
    }
}

// ----------------

void ui_handle_element(UIElement* elem, UIEvent event, UIEventContext context) {
    switch (elem->type) {
    case UIElementType_Area:
        ui_handle_area((void*)elem, event, context);
        break;
    case UIElementType_Button:
        ui_handle_button((void*)elem, event, context);
        break;
    case UIElementType_SelectionBox:
        ui_handle_selection_box((void*)elem, event, context);
        break;
    case UIElementType_Slider:
        ui_handle_slider((void*)elem, event, context);
        break;
    default:
        UI_HANDLE_VTABLE[elem->type].fn(elem, event, context);
        break;
    }
}

// ----------------

// UIElementStore

UIElementStore ui_element_store_new() {
    UIElementStore store = {0};
    store.memory = make_bump_allocator_with_size(512 * sizeof(UIElement));
    return store;
}

UIElementStore ui_element_store_new_active() {
    UIElementStore store = ui_element_store_new();
    store.active = true;
    return store;
}

void ui_element_store_drop(UIElementStore* store) {
    free_bump_allocator(&store->memory);
}

void ui_element_store_clear(UIElementStore* store) {
    reset_bump_allocator(&store->memory);
    store->count = 0;
}

void* get_ui_element_unchecked(UIElementStore* store, UIElementId elem_id) {
    return (void*) store->elems[elem_id.id];
}

UIElement* get_ui_element(UIElementStore* store, UIElementId elem_id) {
    return store->elems[elem_id.id];
}

// ----------------

UIElementId put_ui_element(UIElementStore* store, UIElement* elem, uint32 size) {
    UIElementId id = { .id = store->count };
    
    UIElement* elem_ptr = bump_alloc(&store->memory, size);
    memcpy(elem_ptr, elem, size);
    elem_ptr->id = id;

    store->elems[store->count] = (void*)elem_ptr;
    store->count++;

    return id;
}

UIElementId put_ui_element_area(UIElementStore* store, Area area) {
    return put_ui_element(store, (void*)&area, sizeof(Area));
}

UIElementId put_ui_element_button(UIElementStore* store, Button button) {
    return put_ui_element(store, (void*)&button, sizeof(Button));
}

UIElementId put_ui_element_selection_box(UIElementStore* store, SelectionBox sbox) {
    return put_ui_element(store, (void*)&sbox, sizeof(SelectionBox));
}

UIElementId put_ui_element_slider(UIElementStore* store, Slider slider) {
    return put_ui_element(store, (void*)&slider, sizeof(Slider));
}

// ----------------

void SYSTEM_PRE_UPDATE_handle_input_for_ui_store(
    UIElementStore* store
) {
    if (!store->active) {
        return;
    }

    Vector2 mouse = GetMousePosition();

    // if (store->pressed_element != NULL) {
    //     if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
    //         JUST_LOG_DEBUG("0\n");
    //         Vector2 relative_mouse = ui_element_relative_point(store->pressed_element, mouse);
    //         ui_handle_released_element(store->pressed_element, relative_mouse);
    //     }
    // }

    UIEventContext context = {0};
    for (uint32 i = 0; i < store->count; i++) {
        UIElement* elem = store->elems[i];

        UIEventContext context = {
            .mouse = ui_element_relative_point(elem, mouse),
        };

        elem->state.just_clicked = false;
        
        bool elem_hovered = ui_element_hovered(elem, mouse);
        if (!elem->state.hover && elem_hovered) {
            ui_handle_element(elem, UIEvent_BeginHover, context);
        }
        else if (elem->state.hover && !elem_hovered) {
            ui_handle_element(elem, UIEvent_EndHover, context);
        }

        if (elem_hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            store->pressed_element = elem;
            ui_handle_element(elem, UIEvent_Pressed, context);
        }

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            if (elem == store->pressed_element) {
                ui_handle_element(elem, UIEvent_Released, context);
            }
            elem->state.pressed = false;
        }
    }
}

void SYSTEM_UPDATE_update_ui_elements(
    UIElementStore* store,
    float32 delta_time
) {
    if (!store->active) {
        return;
    }

    Vector2 mouse = GetMousePosition();

    for (uint32 i = 0; i < store->count; i++) {
        UIElement* elem = store->elems[i];

        UIEventContext context = {
            .delta_time = delta_time,
            .mouse = ui_element_relative_point(elem, mouse),
        };

        ui_handle_element(elem, UIEvent_Update, context);
    }
}

void SYSTEM_RENDER_draw_ui_elements(
    UIElementStore* store
) {
    if (!store->active) {
        return;
    }

    for (uint32 i = 0; i < store->count; i++) {
        UIEventContext context = {0};
        ui_handle_element(store->elems[i], UIEvent_Draw, context);
    }
}