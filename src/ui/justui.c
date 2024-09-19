#include "string.h"

#include "raylib.h"
#include "raymath.h"

#include "base.h"
#include "logging.h"
#include "memory/memory.h"

#include "justui.h"

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

// ButtonStyle
// Button

Button button_new(
    UIElement elem,
    ButtonStyle style,
    char* title
) {
    elem.type = UIElementType_Button;
    Button button =  {
        .elem = elem,
        .style = style,
        .title = {0},
    };
    strcpy(&button.title[0], title);
    return button;
}

void button_consume_click(Button* button) {
    button->elem.state.just_clicked = false;
}

void ui_handle_begin_hover_button(Button* button, Vector2 mouse) {
    button->elem.state.hover = true;
}

void ui_handle_end_hover_button(Button* button, Vector2 mouse) {
    button->elem.state.hover = false;
}

void ui_handle_pressed_button(Button* button, Vector2 mouse) {
    button->elem.state.pressed = true;
}

void ui_handle_released_button(Button* button, Vector2 mouse) {
    if (button->elem.state.hover && button->elem.state.pressed) {
        button->elem.state.just_clicked = true;
        button->elem.state.click_point_relative = mouse;
    }
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

// AreaStyle
// Area

void ui_handle_begin_hover_area(Area* area, Vector2 mouse) {
    area->elem.state.hover = true;
}

void ui_handle_end_hover_area(Area* area, Vector2 mouse) {
    area->elem.state.hover = false;
}

void ui_handle_pressed_area(Area* area, Vector2 mouse) {
    return;
}

void ui_handle_released_area(Area* area, Vector2 mouse) {
    return;
}

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

// ----------------

void ui_handle_begin_hover_element(UIElement* elem, Vector2 mouse) {
    switch (elem->type) {
    case UIElementType_Area:
        ui_handle_begin_hover_area((void*)elem, mouse);
        break;
    case UIElementType_Button:
        ui_handle_begin_hover_button((void*)elem, mouse);
        break;
    default:
        break;
    }
}

void ui_handle_end_hover_element(UIElement* elem, Vector2 mouse) {
    switch (elem->type) {
    case UIElementType_Area:
        ui_handle_end_hover_area((void*)elem, mouse);
        break;
    case UIElementType_Button:
        ui_handle_end_hover_button((void*)elem, mouse);
        break;
    default:
        break;
    }
}

void ui_handle_pressed_element(UIElement* elem, Vector2 mouse) {
    switch (elem->type) {
    case UIElementType_Area:
        ui_handle_pressed_area((void*)elem, mouse);
        break;
    case UIElementType_Button:
        ui_handle_pressed_button((void*)elem, mouse);
        break;
    default:
        break;
    }
}

void ui_handle_released_element(UIElement* elem, Vector2 mouse) {
    switch (elem->type) {
    case UIElementType_Area:
        ui_handle_released_area((void*)elem, mouse);
        break;
    case UIElementType_Button:
        ui_handle_released_button((void*)elem, mouse);
        break;
    default:
        break;
    }
}

void ui_draw_element(UIElement* elem) {
    switch (elem->type) {
    case UIElementType_Area:
        ui_draw_area((void*)elem);
        break;
    case UIElementType_Button:
        ui_draw_button((void*)elem);
        break;
    default:
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

UIElementId put_ui_element_button(UIElementStore* store, Button button) {
    UIElementId id = { .id = store->count };
    button.elem.id = id;
    
    Button* elem_ptr = bump_alloc(&store->memory, sizeof(Button));
    *elem_ptr = button;

    store->elems[store->count] = (void*)elem_ptr;
    store->count++;

    return id;
}

UIElementId put_ui_element_area(UIElementStore* store, Area area) {
    UIElementId id = { .id = store->count };
    area.elem.id = id;
    
    Area* elem_ptr = bump_alloc(&store->memory, sizeof(Area));
    *elem_ptr = area;

    store->elems[store->count] = (void*)elem_ptr;
    store->count++;

    return id;
}

void* get_ui_element_unchecked(UIElementStore* store, UIElementId elem_id) {
    return (void*) store->elems[elem_id.id];
}

UIElement* get_ui_element(UIElementStore* store, UIElementId elem_id) {
    return store->elems[elem_id.id];
}

Button* get_ui_element_button(UIElementStore* store, UIElementId elem_id) {
    return (Button*) store->elems[elem_id.id];
}

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

    JUST_LOG_DEBUG("Count: %d\n", store->count);

    for (uint32 i = 0; i < store->count; i++) {
        UIElement* elem = store->elems[i];
        Vector2 relative_mouse = ui_element_relative_point(elem, mouse);

        elem->state.just_clicked = 0;
        
        bool elem_hovered = ui_element_hovered(elem, mouse);
        if (!elem->state.hover && elem_hovered) {
            JUST_LOG_DEBUG("1\n");
            ui_handle_begin_hover_element(elem, relative_mouse);
        }
        else if (elem->state.hover && !elem_hovered) {
            JUST_LOG_DEBUG("2\n");
            ui_handle_end_hover_element(elem, relative_mouse);
        }

        if (elem_hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            JUST_LOG_DEBUG("3\n");
            store->pressed_element = elem;
            ui_handle_pressed_element(elem, relative_mouse);
        }

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            JUST_LOG_DEBUG("4\n");
            if (elem == store->pressed_element) {
                ui_handle_released_element(store->pressed_element, relative_mouse);
            }
            elem->state.pressed = false;
        }
    }
}

void SYSTEM_RENDER_draw_ui_elements(
    UIElementStore* store
) {
    if (!store->active) {
        return;
    }

    for (uint32 i = 0; i < store->count; i++) {
        ui_draw_element(store->elems[i]);
    }
}