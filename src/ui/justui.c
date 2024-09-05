#include "string.h"

#include "raylib.h"
#include "raymath.h"

#include "base.h"
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
    strcpy(&button.title, title);
    return button;
}

void button_consume_click(Button* button) {
    button->elem.state.just_clicked = false;
}

void handle_begin_hover_button(Button* button, Vector2 mouse) {
    button->elem.state.hover = true;
}

void handle_end_hover_button(Button* button, Vector2 mouse) {
    button->elem.state.hover = false;
}

void handle_pressed_button(Button* button, Vector2 mouse) {
    button->elem.state.pressed = true;
}

void handle_released_button(Button* button, Vector2 mouse) {
    if (button->elem.state.hover && button->elem.state.pressed) {
        button->elem.state.pressed = false;
        button->elem.state.just_clicked = true;
        button->elem.state.click_point_relative = mouse;
    }
}

void draw_button(Button* button) {
    UIElement* elem = &button->elem;
    Vector2 top_left = find_rectangle_top_left(elem->anchor, elem->position, elem->size);

    Rectangle rect = {
        .x = top_left.x,
        .y = top_left.y,
        .width = elem->size.width,
        .height = elem->size.height,
    };

    Color color = button->style.idle_color;;
    if (elem->disabled) {
        color = button->style.disabled_color;
    }
    else { // if (!elem->disabled)
        if (button->elem.state.pressed) {
            color = button->style.pressed_color;
        }
        else if (button->elem.state.hover) {
            color = button->style.hovered_color;
        }
    }

    DrawRectangleRec(rect, color);
    
    if (button->style.is_bordered) {
        DrawRectangleLinesEx(rect, button->style.border_thick, button->style.border_color);
    }
}

// ----------------

void ui_element_handle_begin_hover(UIElement* elem, Vector2 mouse) {
}

void ui_element_handle_end_hover(UIElement* elem, Vector2 mouse) {
}

void ui_element_handle_pressed(UIElement* elem, Vector2 mouse) {
}

void ui_element_handle_released(UIElement* elem, Vector2 mouse) {
}

void ui_element_draw(UIElement* elem) {
}

// ----------------

// UIElementStore

UIElementStore ui_element_store_new() {
    UIElementStore store = {0};
    store.memory = make_bump_allocator_with_size(256 * sizeof(Button));
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

UIElementId put_ui_element_button(UIElementStore* store, Button button) {
    UIElementId id = { .id = store->count };
    button.elem.id = id;
    
    Button* elem_ptr = bump_alloc(&store->memory, sizeof(Button));
    *elem_ptr = button;

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

    if (store->pressed_element != NULL) {
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            Vector2 relative_mouse = ui_element_relative_point(store->pressed_element, mouse);
            ui_element_handle_released(store->pressed_element, relative_mouse);
        }
    }

    for (uint32 i = 0; i < store->count; i++) {
        UIElement* elem = store->elems[i];
        Vector2 relative_mouse = ui_element_relative_point(store->pressed_element, mouse);
        
        bool elem_hovered = ui_element_hovered(elem, mouse);
        if (!elem->state.hover && elem_hovered) {
            ui_element_handle_begin_hover(elem, relative_mouse);
        }
        else if (elem->state.hover && !elem_hovered) {
            ui_element_handle_end_hover(elem, relative_mouse);
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            store->pressed_element = elem;
            ui_element_handle_pressed(elem, relative_mouse);
        }

        // if (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)) {
        //     ui_element_handle_released(elem, relative_mouse);
        // }
    }
}

void SYSTEM_RENDER_draw_ui_elements(
    UIElementStore* store
) {
    if (!store->active) {
        return;
    }

    for (uint32 i = 0; i < store->count; i++) {
        ui_element_draw(store->elems[i]);
    }
}