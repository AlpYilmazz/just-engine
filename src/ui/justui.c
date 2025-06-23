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
    return Vector2Subtract(point, find_rectangle_top_left(elem->anchor, elem->position, elem->size));
}

// AreaStyle
// Area

void ui_draw_area(Area* area, Vector2 element_origin) {
    UIElement* elem = &area->elem;
    AreaStyle* style = &area->style;

    Vector2 top_left = Vector2Add(
        element_origin,
        find_rectangle_top_left(elem->anchor, elem->position, elem->size)
    );

    Rectangle rect = {
        .x = top_left.x,
        .y = top_left.y,
        .width = elem->size.width,
        .height = elem->size.height,
    };

    Color color = style->idle_color;
    if (elem->state.on_hover) {
        color = style->hovered_color;
    }

    DrawRectangleRec(rect, color);

    if (style->border.is_bordered) {
        DrawRectangleLinesEx(rect, style->border.thick, style->border.color);
    }
}

void ui_handle_area(Area* area, UIEvent event, UIEventContext context) {
    switch (event) {
    case UIEvent_Draw:
        ui_draw_area(area, context.element_origin);
        break;
    }
}

// ButtonStyle
// Button

void button_consume_click(Button* button) {
    button->elem.state.just_clicked = false;
}

void ui_draw_button(Button* button, Vector2 element_origin) {
    UIElement* elem = &button->elem;
    ButtonStyle* style = &button->style;

    Vector2 top_left = Vector2Add(
        element_origin,
        find_rectangle_top_left(elem->anchor, elem->position, elem->size)
    );
    top_left = Vector2Add(top_left, button->draw_offset);
    Vector2 mid = Vector2Add(top_left, Vector2Scale(elem->size.as_vec, 0.5));

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
        if (elem->state.on_press) {
            color = style->pressed_color;
        }
        else if (elem->state.on_hover) {
            color = style->hovered_color;
        }
    }

    DrawRectangleRec(rect, color);
    
    if (style->border.is_bordered) {
        DrawRectangleLinesEx(rect, style->border.thick, style->border.color);
    }

    // Draw Text [title]
    Vector2 text_size = MeasureTextEx(style->title.font, button->title, style->title.font_size, style->title.spacing);
    Vector2 text_pos = Vector2Subtract(mid, Vector2Scale(text_size, 0.5));

    DrawTextEx(style->title.font, button->title, text_pos, style->title.font_size, style->title.spacing, style->title.color);
}

void ui_handle_button(Button* button, UIEvent event, UIEventContext context) {
    switch (event) {
    case UIEvent_Draw:
        ui_draw_button(button, context.element_origin);
        break;
    }
}

// SelectionBoxStyle
// SelectionBox

void ui_draw_selection_box(SelectionBox* sbox, Vector2 element_origin) {
    UIElement* elem = &sbox->elem;
    SelectionBoxStyle* style = &sbox->style;

    Vector2 top_left = Vector2Add(
        element_origin,
        find_rectangle_top_left(elem->anchor, elem->position, elem->size)
    );
    Vector2 mid = Vector2Add(top_left, Vector2Scale(elem->size.as_vec, 0.5));

    Rectangle rect = {
        .x = top_left.x,
        .y = top_left.y,
        .width = elem->size.width,
        .height = elem->size.height,
    };
    Color color = elem->disabled ? style->disabled_color
        : sbox->selected ? style->selected_color : style->unselected_color;

    DrawRectangleRec(rect, color);
    
    if (style->border.is_bordered) {
        DrawRectangleLinesEx(rect, style->border.thick, style->border.color);
    }

    // Draw Text [title]
    Vector2 text_size = MeasureTextEx(style->title.font, sbox->title, style->title.font_size, style->title.spacing);
    Vector2 text_pos = Vector2Subtract(mid, Vector2Scale(text_size, 0.5));

    DrawTextEx(style->title.font, sbox->title, text_pos, style->title.font_size, style->title.spacing, style->title.color);
}

void ui_handle_selection_box(SelectionBox* sbox, UIEvent event, UIEventContext context) {
    switch (event) {
    case UIEvent_Released:
        sbox->selected ^= 1;
        break;
    case UIEvent_Draw:
        ui_draw_selection_box(sbox, context.element_origin);
        break;
    }
}

// SliderStyle
// Slider

float32 get_slider_value(Slider* slider) {
    return slider->low_value + (slider->high_value - slider->low_value) * slider->cursor;
}

void ui_update_slider(Slider* slider, Vector2 element_origin, Vector2 mouse, float32 delta_time) {
    UIElement* elem = &slider->elem;
    if (elem->state.on_press) {
        Vector2 top_left = Vector2Add(
            element_origin,
            find_rectangle_top_left(elem->anchor, elem->position, elem->size)
        );
        Vector2 global_mouse = Vector2Add(top_left, mouse);

        float32 cursor_radius = elem->size.height/2.0;

        float32 start_x = top_left.x + cursor_radius;
        float32 width = elem->size.width - 2*cursor_radius;

        float32 mouse_x = global_mouse.x - start_x;

        slider->cursor = Clamp(mouse_x / width, 0.0, 1.0);
    }
}

void ui_draw_slider(Slider* slider, Vector2 element_origin) {
    UIElement* elem = &slider->elem;
    SliderStyle* style = &slider->style;

    Vector2 size_vec = elem->size.as_vec;
    Vector2 top_left = Vector2Add(
        element_origin,
        find_rectangle_top_left(elem->anchor, elem->position, elem->size)
    );
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

    // Draw text
    const float32 MARGIN = 10;

    {
        const char* low_value_text = TextFormat("%0.1f", slider->low_value);
        Vector2 low_value_text_size = MeasureTextEx(style->title.font, low_value_text, style->title.font_size, style->title.spacing);
        Vector2 low_value_text_pos = {
            .x = top_left.x - low_value_text_size.x - MARGIN,
            .y = mid.y - low_value_text_size.y/2.0,
        };
        DrawTextEx(style->title.font, low_value_text, low_value_text_pos, style->title.font_size, style->title.spacing, style->title.color);
    }

    {
        const char* high_value_text = TextFormat("%0.1f", slider->high_value);
        Vector2 high_value_text_size = MeasureTextEx(style->title.font, high_value_text, style->title.font_size, style->title.spacing);
        Vector2 high_value_text_pos = {
            .x = bottom_right.x + MARGIN,
            .y = mid.y - high_value_text_size.y/2.0,
        };
        DrawTextEx(style->title.font, high_value_text, high_value_text_pos, style->title.font_size, style->title.spacing, style->title.color);
    }

    {
        const char* cursor_value_text = TextFormat("%0.2f", get_slider_value(slider));
        Vector2 cursor_value_text_size = MeasureTextEx(style->title.font, cursor_value_text, style->title.font_size, style->title.spacing);
        Vector2 cursor_value_text_pos = {
            .x = cursor_pos.x - cursor_value_text_size.x/2.0,
            .y = cursor_pos.y + cursor_radius + MARGIN,
        };
        DrawTextEx(style->title.font, cursor_value_text, cursor_value_text_pos, style->title.font_size, style->title.spacing, style->title.color);
    }

    {
        Vector2 title_size = MeasureTextEx(style->title.font, slider->title, style->title.font_size, style->title.spacing);
        Vector2 title_pos = {
            .x = mid.x - title_size.x/2.0,
            .y = mid.y - title_size.y/2.0 - cursor_radius - 2*MARGIN,
        };
        DrawTextEx(style->title.font, slider->title, title_pos, style->title.font_size, style->title.spacing, style->title.color);
    }
}

void ui_handle_slider(Slider* slider, UIEvent event, UIEventContext context) {
    switch (event) {
    case UIEvent_Update:
        ui_update_slider(slider, context.element_origin, context.mouse, context.delta_time);
        break;
    case UIEvent_Draw:
        ui_draw_slider(slider, context.element_origin);
        break;
    }
}

// ChoiceListStyle
// ChoiceListOption
// ChoiceList

void ui_set_hovered_choise_list(ChoiceList* choice_list, Vector2 mouse) {
    UIElement* elem = &choice_list->elem;
    GridLayout* layout = &choice_list->layout;
    
    choice_list->hovered_option_index = (Option(uint32)) Option_None;

    if (
        mouse.x < layout->box_padding || layout->box_padding + layout->content_box.width < mouse.x
        || mouse.y < layout->box_padding || layout->box_padding + layout->content_box.height < mouse.y
    ) {
        return;
    }
    // Mouse Inside ContentBox

    Vector2 mouse_on_content = {
        .x = mouse.x - layout->box_padding,
        .y = mouse.y - layout->box_padding,
    };

    RectSize cellbox_size = {
        .width = layout->cell_size.width + layout->cell_padding,
        .height = layout->cell_size.height + layout->cell_padding,
    };

    uint32 cell_x = ((uint32)mouse_on_content.x) / (uint32)cellbox_size.width;
    uint32 cell_y = ((uint32)mouse_on_content.y) / (uint32)cellbox_size.height;

    uint32 option_index = (cell_y * layout->cols) + cell_x;
    if (cell_x >= layout->cols || cell_y >= layout->rows || option_index >= choice_list->option_count) {
        return;
    }
    // Cell is an Option

    Vector2 cell_top_left = {
        .x = (cell_x * cellbox_size.width) + layout->box_padding,
        .y = (cell_y * cellbox_size.height) + layout->box_padding,
    };
    Vector2 mouse_on_cell = Vector2Subtract(mouse, cell_top_left);

    if (
        EPSILON <= mouse_on_cell.x && mouse_on_cell.x <= layout->cell_size.width
        && EPSILON <= mouse_on_cell.y && mouse_on_cell.y <= layout->cell_size.height
    ) {
        // Mouse Inside Cell Content
        JUST_LOG_DEBUG("option: %d, cell: %d %d, mouse: %0.2f %0.2f, mouse_on_cell: %0.2f %0.2f\n", option_index, cell_x, cell_y, mouse.x, mouse.y, mouse_on_cell.x, mouse_on_cell.y);
        choice_list->hovered_option_index = (Option(uint32)) Option_Some(option_index);
    }
}

void ui_draw_choice_list(ChoiceList* choice_list, Vector2 element_origin) {
    UIElement* elem = &choice_list->elem;
    ChoiceListStyle* style = &choice_list->style;
    GridLayout* layout = &choice_list->layout;

    Vector2 top_left = Vector2Add(
        element_origin,
        find_rectangle_top_left(elem->anchor, elem->position, elem->size)
    );

    {
        Rectangle bg_rect = {
            .x = top_left.x,
            .y = top_left.y,
            .width = elem->size.width,
            .height = elem->size.height,
        };
        DrawRectangleRec(bg_rect, (Color) {100, 100, 100, 50});
        DrawRectangleLinesEx(bg_rect, 2, (Color) {100, 100, 100, 100});
    }

    layout->next_cell = 0;
    for (uint32 option_index = 0; option_index < choice_list->option_count; option_index++) {
        JUST_LOG_TRACE("option: %d\n", option_index);
        ChoiceListOption* option = &choice_list->options[option_index];

        Rectangle cell_rect = grid_layout_next(layout);
        Vector2 cell_mid = {
            .x = cell_rect.x + cell_rect.width*0.5,
            .y = cell_rect.y + cell_rect.height*0.5,
        };
        JUST_LOG_DEBUG("[%d] cell_rect: {%0.0f %0.0f %0.0f %0.0f}\n", option_index, cell_rect.x, cell_rect.y, cell_rect.width, cell_rect.height);

        Color color = elem->disabled ? style->disabled_color
            : (choice_list->selected_option_id == option->id) ? style->selected_color
            : style->unselected_color;
        // Color color = choice_list->selected_option_id == option->id ? GREEN : RED;
        JUST_LOG_TRACE("option color: {%d %d %d %d}\n", color.r, color.g, color.b, color.a);

        DrawRectangleRec(cell_rect, color);
            
        if (style->border.is_bordered) {
            DrawRectangleLinesEx(cell_rect, style->border.thick, style->border.color);
        }

        // Draw Text [title]
        Vector2 text_size = MeasureTextEx(style->title.font, option->title, style->title.font_size, style->title.spacing);
        Vector2 text_pos = Vector2Subtract(cell_mid, Vector2Scale(text_size, 0.5));

        DrawTextEx(style->title.font, option->title, text_pos, style->title.font_size, style->title.spacing, style->title.color);
    }
}

void ui_handle_choice_list(ChoiceList* choice_list, UIEvent event, UIEventContext context) {
    switch (event) {
    case UIEvent_BeginHover:
    case UIEvent_StayHover:
        ui_set_hovered_choise_list(choice_list, context.mouse);
        break;
    case UIEvent_EndHover:
        choice_list->hovered_option_index = (Option(uint32)) Option_None;
        break;
    case UIEvent_Released:
        if (choice_list->hovered_option_index.is_some) {
            choice_list->selected_option_id = choice_list->options[choice_list->hovered_option_index.value].id;
        }
        break;
    case UIEvent_Draw:
        ui_draw_choice_list(choice_list, context.element_origin);
        break;
    }
}

// ----------------

Area make_ui_area(Area area) {
    area.elem.type = UIElementType_Area;
    area.elem.state = (UIElementState) {0};
    return area;
}

Button make_ui_button(Button button) {
    button.elem.type = UIElementType_Button;
    button.elem.state = (UIElementState) {0};
    return button;
}

SelectionBox make_ui_selection_box(SelectionBox selection_box) {
    selection_box.elem.type = UIElementType_SelectionBox;
    selection_box.elem.state = (UIElementState) {0};
    return selection_box;
}

Slider make_ui_slider(Slider slider) {
    slider.elem.type = UIElementType_Slider;
    slider.elem.state = (UIElementState) {0};
    return slider;
}

ChoiceList make_ui_choice_list(ChoiceList choice_list) {
    choice_list.elem.type = UIElementType_ChoiceList;
    choice_list.elem.state = (UIElementState) {0};
    choice_list.layout.box = into_rectangle(choice_list.elem.position, choice_list.elem.size);
    choice_list.layout = make_grid_layout(choice_list.layout);
    choice_list.hovered_option_index = (Option(uint32)) Option_None;
    return choice_list;
}

Panel make_ui_panel(Panel panel) {
    panel.elem.type = UIElementType_Panel;
    panel.elem.state = (UIElementState) {0};
    return panel;
}

// ----------------

void ui_draw_panel(Panel* panel, Vector2 element_origin) {
    UIElement* elem = &panel->elem;

    Vector2 top_left = Vector2Add(
        element_origin,
        find_rectangle_top_left(elem->anchor, elem->position, elem->size)
    );

    Rectangle rect = {
        .x = top_left.x,
        .y = top_left.y,
        .width = elem->size.width,
        .height = elem->size.height,
    };
    DrawRectangleRec(rect, (Color) {100, 100, 100, 50});
    DrawRectangleLinesEx(rect, 2, (Color) {100, 100, 100, 100});
}

void ui_handle_element(UIElement* elem, UIEvent event, UIEventContext context);

void ui_handle_panel(Panel* panel, UIEvent event, UIEventContext panel_context) {
    JUST_LOG_TRACE("Panel: %d\n", event);
    if (!panel->open) {
        JUST_LOG_TRACE("Panel closed\n");
        return;
    }
    JUST_LOG_TRACE("Panel open\n");

    // JUST_LOG_INFO("\tpanel mouse: %0.2f %0.2f\n", panel_context.mouse.x, panel_context.mouse.y);

    Vector2 panel_top_left = Vector2Add(
        panel_context.element_origin,
        find_rectangle_top_left(panel->elem.anchor, panel->elem.position, panel->elem.size)
    );

    switch (event) {
    case UIEvent_BeginHover:
    case UIEvent_StayHover:
    case UIEvent_EndHover:
    case UIEvent_Pressed:
    case UIEvent_Released:
        for (uint32 i = 0; i < panel->store.count; i++) {
            UIElement* elem = panel->store.elems[i];
            elem->state.just_begin_hover = false;
            elem->state.just_end_hover = false;
            elem->state.just_pressed = false;
            elem->state.just_clicked = false;
        }
        break;
    }

    switch (event) {
    case UIEvent_BeginHover:
    case UIEvent_StayHover:
        // Hover Check
        UIElement* hover_candidate_elem = NULL;
        for (uint32 i = 0; i < panel->store.count; i++) {
            UIElement* elem = panel->store.elems[i];

            bool elem_hovered = ui_element_hovered(elem, panel_context.mouse);
            if (elem_hovered) {
                // BeginHover OR StayHover Candidates
                if (hover_candidate_elem == NULL || elem->layer > hover_candidate_elem->layer) {
                    // JUST_LOG_INFO("Hover Candidate: %d\n", elem->id);
                    hover_candidate_elem = elem;
                }
            }
        }
        for (uint32 i = 0; i < panel->store.count; i++) {
            UIElement* elem = panel->store.elems[i];

            bool elem_hovered = ui_element_hovered(elem, panel_context.mouse);
            if (elem->state.on_hover && (!elem_hovered || elem != hover_candidate_elem)) {
                // EndHover
                JUST_LOG_INFO("EndHover: %d\n", elem->id);
                UIEventContext context = {
                    .mouse = ui_element_relative_point(elem, panel_context.mouse),
                    .element_origin = panel_top_left,
                };
                elem->state.just_end_hover = true;
                elem->state.on_hover = false;
                ui_handle_element(elem, UIEvent_EndHover, context);
            }
        }
        if (hover_candidate_elem != NULL) {
            UIElement* elem = hover_candidate_elem;
            // JUST_LOG_INFO("Hover Result: %d\n", elem->id);

            UIEventContext context = {
                .mouse = ui_element_relative_point(elem, panel_context.mouse),
                .element_origin = panel_top_left,
            };

            if (!elem->state.on_hover) {
                JUST_LOG_INFO("BeginHover: %d\n", elem->id);
                elem->state.just_begin_hover = true;
                elem->state.on_hover = true;
                ui_handle_element(elem, UIEvent_BeginHover, context);
            }
            else { // if (elem->state.on_hover)
                // JUST_LOG_INFO("StayHover: %d\n", elem->id);
                ui_handle_element(elem, UIEvent_StayHover, context);
            }
        }
        // Hover Check -- END --

        // for (uint32 i = 0; i < panel->store.count; i++) {
        //     UIElement* elem = panel->store.elems[i];

        //     UIEventContext context = {
        //         .mouse = ui_element_relative_point(elem, panel_context.mouse),
        //         .element_origin = panel_top_left,
        //     };
        //     // JUST_LOG_INFO("\t\telem mouse: %0.2f %0.2f\n", context.mouse.x, context.mouse.y);
            
        //     bool elem_hovered = ui_element_hovered(elem, panel_context.mouse);
        //     if (!elem->state.on_hover && elem_hovered) {
        //         elem->state.just_begin_hover = true;
        //         elem->state.on_hover = true;
        //         ui_handle_element(elem, UIEvent_BeginHover, context);
        //     }
        //     else if (elem->state.on_hover && elem_hovered) {
        //         ui_handle_element(elem, UIEvent_StayHover, context);
        //     }
        //     else if (elem->state.on_hover && !elem_hovered) {
        //         elem->state.just_end_hover = true;
        //         elem->state.on_hover = false;
        //         ui_handle_element(elem, UIEvent_EndHover, context);
        //     }
        // }
        break;
    case UIEvent_EndHover:
        for (uint32 i = 0; i < panel->store.count; i++) {
            UIElement* elem = panel->store.elems[i];

            if (elem->state.on_hover) {
                JUST_LOG_INFO("EndHover: %d\n", elem->id);
                UIEventContext context = {
                    .mouse = ui_element_relative_point(elem, panel_context.mouse),
                    .element_origin = panel_top_left,
                };
                elem->state.just_end_hover = true;
                elem->state.on_hover = false;
                ui_handle_element(elem, UIEvent_EndHover, context);
            }
        }
        break;
    case UIEvent_Pressed:
        for (uint32 i = 0; i < panel->store.count; i++) {
            UIElement* elem = panel->store.elems[i];
            
            if (elem->state.on_hover) {
                JUST_LOG_INFO("Pressed: %d\n", elem->id);
                UIEventContext context = {
                    .mouse = ui_element_relative_point(elem, panel_context.mouse),
                    .element_origin = panel_top_left,
                };
                panel->store.pressed_element = elem;
                JUST_LOG_INFO("pressed_element: %p -> %d\n", panel->store.pressed_element, elem->id);
                elem->state.just_pressed = true;
                elem->state.on_press = true;
                ui_handle_element(elem, UIEvent_Pressed, context);
            }
        }
        break;
    case UIEvent_Released:
        JUST_LOG_INFO("Panel Released\n");
        for (uint32 i = 0; i < panel->store.count; i++) {
            UIElement* elem = panel->store.elems[i];

            JUST_LOG_INFO("Released: pressed_element: %p\n", panel->store.pressed_element);
            if (elem == panel->store.pressed_element) {
                JUST_LOG_INFO("I am pressed_element: %d\n", elem->id);
                if (elem->state.on_hover && elem->state.on_press) {
                    JUST_LOG_INFO("Released: %d\n", elem->id);
                    UIEventContext context = {
                        .mouse = ui_element_relative_point(elem, panel_context.mouse),
                        .element_origin = panel_top_left,
                    };
                    elem->state.just_clicked = true;
                    elem->state.click_point_relative = context.mouse;
                    ui_handle_element(elem, UIEvent_Released, context);
                }
                panel->store.pressed_element = NULL;
            }
            elem->state.on_press = false;
        }
        break;
    case UIEvent_Update:
        JUST_LOG_TRACE("Panel Elem Count: %d\n", panel->store.count);
        for (uint32 i = 0; i < panel->store.count; i++) {
            JUST_LOG_TRACE("Update Elem: %d\n", i);
            UIEventContext context = {
                .mouse = ui_element_relative_point(panel->store.elems[i], panel_context.mouse),
                .element_origin = panel_top_left,
            };
            ui_handle_element(panel->store.elems[i], UIEvent_Update, context);
        }
        break;
    case UIEvent_Draw:
        ui_draw_panel(panel, panel_context.element_origin);
        for (uint32 i = 0; i < panel->store.count; i++) {
            UIElement* elem = panel->store.elems[panel->store.layer_sort[i].index];
            UIEventContext context = {
                .element_origin = panel_top_left,
            };
            ui_handle_element(elem, UIEvent_Draw, context);
        }
        break;
    case UIEvent_DropElement:
        ui_element_store_drop(&panel->store);
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
    case UIElementType_ChoiceList:
        ui_handle_choice_list((void*)elem, event, context);
        break;
    case UIElementType_Panel:
        ui_handle_panel((void*)elem, event, context);
        break;
    default:
        UI_HANDLE_VTABLE[elem->type].fn(elem, event, context);
        break;
    }
}

// ----------------

// UIElementStore

UIElementStore ui_element_store_new_with_count_hint(uint32 count_hint) {
    UIElementStore store = {0};

    uint32 elems_count = count_hint;

    MemoryLayout layer_sort_array_layout = array_layoutof(ElementSort, elems_count);
    MemoryLayout elems_array_layout = array_layoutof(UIElement*, elems_count);

    uint32 elem_store_mem_size =
        layer_sort_array_layout.size
        + elems_array_layout.size
        + (10 * elems_count * sizeof(UIElement));

    store.memory = make_bump_allocator_with_size(elem_store_mem_size);

    store.layer_sort = bump_alloc_aligned(&store.memory, layer_sort_array_layout);
    store.elems = bump_alloc_aligned(&store.memory, elems_array_layout);
    
    // store.layer_sort = bump_alloc(&store.memory, layer_sort_array_layout.size);
    // store.elems = bump_alloc(&store.memory, elems_array_layout.size);
    
    store.memory_reset_cursor = store.memory.cursor;
    return store;
}

UIElementStore ui_element_store_new() {
    return ui_element_store_new_with_count_hint(100);
}

UIElementStore ui_element_store_new_active_with_count_hint(uint32 count_hint) {
    UIElementStore store = ui_element_store_new_with_count_hint(count_hint);
    store.active = true;
    return store;
}

UIElementStore ui_element_store_new_active() {
    UIElementStore store = ui_element_store_new();
    store.active = true;
    return store;
}

void ui_element_store_drop_elements(UIElementStore* store) {
    for (uint32 i = 0; i < store->count; i++) {
        UIEventContext context = {0};
        ui_handle_element(store->elems[i], UIEvent_DropElement, context);
    }
}

void ui_element_store_drop(UIElementStore* store) {
    ui_element_store_drop_elements(store);
    free_bump_allocator(&store->memory);
}

void ui_element_store_clear(UIElementStore* store) {
    ui_element_store_drop_elements(store);
    store->memory.cursor = store->memory_reset_cursor;
    store->count = 0;
}

void* get_ui_element_unchecked(UIElementStore* store, UIElementId elem_id) {
    return (void*) store->elems[elem_id.id];
}

UIElement* get_ui_element(UIElementStore* store, UIElementId elem_id) {
    return store->elems[elem_id.id];
}

// ----------------

static uint32 insert_sorted(ElementSort* arr, uint32 count, ElementSort value) {
    for (uint32 i = 0; i < count; i++) {
        if (arr[i].layer > value.layer) {
            for (uint32 j = count+1; j > i; j--) {
                arr[j] = arr[j-1];
            }
            arr[i] = value;
            return i;
        }
    }
    arr[count] = value;
    return count;
} 

UIElementId put_ui_element(UIElementStore* store, UIElement* elem, MemoryLayout layout) {
    UIElementId id = { .id = store->count };
    
    UIElement* elem_ptr = bump_alloc_aligned(&store->memory, layout);
    // UIElement* elem_ptr = bump_alloc(&store->memory, layout.size);
    memcpy(elem_ptr, elem, layout.size);
    elem_ptr->id = id;

    store->elems[store->count] = (void*)elem_ptr;
    uint32 sort_order = insert_sorted(
        store->layer_sort,
        store->count,
        (ElementSort) {
            .layer = elem_ptr->layer,
            .index = id.id,
        }
    );
    store->count++;

    JUST_LOG_INFO("Id: %d, Count: %d, Sort: %d, Size: %d\n", id.id, store->count, sort_order, layout.size);
    return id;
}

UIElementId put_ui_element_area(UIElementStore* store, Area area) {
    return put_ui_element(store, (void*)&area, layoutof(Area));
}

UIElementId put_ui_element_button(UIElementStore* store, Button button) {
    return put_ui_element(store, (void*)&button, layoutof(Button));
}

UIElementId put_ui_element_selection_box(UIElementStore* store, SelectionBox sbox) {
    return put_ui_element(store, (void*)&sbox, layoutof(SelectionBox));
}

UIElementId put_ui_element_slider(UIElementStore* store, Slider slider) {
    return put_ui_element(store, (void*)&slider, layoutof(Slider));
}

UIElementId put_ui_element_choice_list(UIElementStore* store, ChoiceList choice_list) {
    return put_ui_element(store, (void*)&choice_list, layoutof(ChoiceList));
}

UIElementId put_ui_element_panel(UIElementStore* store, Panel panel) {
    return put_ui_element(store, (void*)&panel, layoutof(Panel));
}

// ----------------

void SYSTEM_INPUT_handle_input_for_ui_store(
    UIElementStore* store
) {
    if (!store->active) {
        return;
    }

    Vector2 mouse = GetMousePosition();
    bool mouse_left_pressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    bool mouse_left_released = IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
    // JUST_LOG_INFO("mouse: %0.2f %0.2f\n", mouse.x, mouse.y);

    // Reset
    for (uint32 i = 0; i < store->count; i++) {
        UIElement* elem = store->elems[i];
        elem->state.just_begin_hover = false;
        elem->state.just_end_hover = false;
        elem->state.just_pressed = false;
        elem->state.just_clicked = false;
    }

    // Hover Check
    UIElement* hover_candidate_elem = NULL;
    for (uint32 i = 0; i < store->count; i++) {
        UIElement* elem = store->elems[i];

        bool elem_hovered = ui_element_hovered(elem, mouse);
        if (elem_hovered) {
            // BeginHover OR StayHover Candidates
            if (hover_candidate_elem == NULL || elem->layer > hover_candidate_elem->layer) {
                hover_candidate_elem = elem;
            }
        }
    }
    for (uint32 i = 0; i < store->count; i++) {
        UIElement* elem = store->elems[i];

        bool elem_hovered = ui_element_hovered(elem, mouse);
        if (elem->state.on_hover && (!elem_hovered || elem != hover_candidate_elem)) {
            // EndHover
            UIEventContext context = {
                .mouse = ui_element_relative_point(elem, mouse),
                .element_origin = {0, 0},
            };
            elem->state.just_end_hover = true;
            elem->state.on_hover = false;
            ui_handle_element(elem, UIEvent_EndHover, context);
        }
    }
    if (hover_candidate_elem != NULL) {
        UIElement* elem = hover_candidate_elem;

        UIEventContext context = {
            .mouse = ui_element_relative_point(elem, mouse),
            .element_origin = {0, 0},
        };

        if (!elem->state.on_hover) {
            elem->state.just_begin_hover = true;
            elem->state.on_hover = true;
            ui_handle_element(elem, UIEvent_BeginHover, context);
        }
        else if (elem->state.on_hover) {
            ui_handle_element(elem, UIEvent_StayHover, context);
        }
    }
    // Hover Check -- END --

    // Pressed/Released Check
    for (uint32 i = 0; i < store->count; i++) {
        UIElement* elem = store->elems[i];

        UIEventContext context = {
            .mouse = ui_element_relative_point(elem, mouse),
            .element_origin = {0, 0},
        };

        if (elem->state.on_hover && mouse_left_pressed) {
            store->pressed_element = elem;
            elem->state.just_pressed = true;
            elem->state.on_press = true;
            ui_handle_element(elem, UIEvent_Pressed, context);
        }

        if (mouse_left_released) {
            if (elem == store->pressed_element) {
                if (elem->state.on_hover && elem->state.on_press) {
                    elem->state.just_clicked = true;
                    elem->state.click_point_relative = context.mouse;
                    ui_handle_element(elem, UIEvent_Released, context);
                }
                store->pressed_element = NULL;
            }
            elem->state.on_press = false;
        }
    }
    // Pressed/Released Check -- END --
}

void SYSTEM_UPDATE_update_ui_elements(
    UIElementStore* store,
    float32 delta_time
) {
    if (!store->active) {
        return;
    }

    Vector2 mouse = GetMousePosition();

    JUST_LOG_TRACE("Store Elem Count: %d\n", store->count);
    for (uint32 i = 0; i < store->count; i++) {
        JUST_LOG_TRACE("Store Update Elem: %d\n", i);
        UIElement* elem = store->elems[i];

        UIEventContext context = {
            .delta_time = delta_time,
            .mouse = ui_element_relative_point(elem, mouse),
            .element_origin = {0, 0},
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

    UIEventContext context = {
        .element_origin = {0, 0},
    };
    
    for (uint32 i = 0; i < store->count; i++) {
        UIElement* elem = store->elems[store->layer_sort[i].index];
        ui_handle_element(elem, UIEvent_Draw, context);
    }
}