#include "justengine.h"

#include "introspect/test/depexample.h"
#include "introspect/test/example.h"
#include "introspect/test/gen.h"

#define COUNT 10
void reset_test_tweens(Tween_Vector2* tweens) {
    float32 pathlen = 900;
    float32 seplen = 900.0 / (COUNT-1);

    tweens[0] = (Tween_Vector2) {
        .state = new_tween_state(TWEEN_ONCE, animation_curve_eased(ease_cubic_in), 2),
    };
    tweens[1] = (Tween_Vector2) {
        .state = new_tween_state(TWEEN_REPEAT_STARTOVER, animation_curve_linear(), 4),
    };
    tweens[2] = (Tween_Vector2) {
        .state = new_tween_state(TWEEN_REPEAT_STARTOVER, animation_curve_step(0.5), 2),
    };
    tweens[3] = (Tween_Vector2) {
        .state = new_tween_state(TWEEN_REPEAT_STARTOVER, animation_curve_eased(ease_elastic_out), 2),
    };
    tweens[4] = (Tween_Vector2) {
        .state = new_tween_state(TWEEN_REPEAT_MIRRORED, animation_curve_linear(), 4),
    };
    tweens[5] = (Tween_Vector2) {
        .state = new_tween_state(TWEEN_REPEAT_MIRRORED, animation_curve_eased(ease_quadratic_out), 4),
    };
    tweens[6] = (Tween_Vector2) {
        .state = new_tween_state(TWEEN_REPEAT_MIRRORED, animation_curve_eased(ease_cubic_out), 4),
    };
    tweens[7] = (Tween_Vector2) {
        .state = new_tween_state(TWEEN_REPEAT_MIRRORED, animation_curve_eased(ease_quartic_out), 4),
    };
    tweens[8] = (Tween_Vector2) {
        .state = new_tween_state(TWEEN_REPEAT_MIRRORED, animation_curve_step(0.5), 2),
    };
    tweens[9] = (Tween_Vector2) {
        .state = new_tween_state(TWEEN_REPEAT_MIRRORED, animation_curve_eased(ease_elastic_out), 2),
    };

    Vector2 start_i = { 50, 50 };
    Vector2 end_i = { 50, 50 + pathlen };
    for (uint32 i = 0; i < COUNT; i++) {
        tweens[i].limits.start = start_i;
        tweens[i].limits.end = end_i;
        start_i.x += seplen;
        end_i.x += seplen;
    }
}

#define SEQ_COUNT 4
void reset_test_tween_sequence(TweenSequence_Vector2* tween_seq) {
    tween_seq->state.elapsed = 0;
    tween_seq->state.direction = 1;
    tween_seq->state.section = 0;
    dynarray_clear(tween_seq->state.sections);
    dynarray_clear(tween_seq->limits_list);

    uint32 i;
    float32 pathlen = 800;
    i = 0;
    {
        TweenSequenceStateSection section = {
            .curve = animation_curve_linear(),
            .duration = 2,
        };
        TweenLimits_Vector2 limits = {
            .start = {100, 100},
            .end = {100 + pathlen, 100},
        };  
        dynarray_push_back(tween_seq->state.sections, section);
        dynarray_push_back(tween_seq->limits_list, limits);
    }
    i = 1;
    {
        Vector2 prev_end = tween_seq->limits_list.items[i-1].end;
        TweenSequenceStateSection section = {
            .curve = animation_curve_eased(ease_cubic_out),
            .duration = 2,
        };
        TweenLimits_Vector2 limits = {
            .start = prev_end,
            .end = {prev_end.x, prev_end.y + pathlen},
        };  
        dynarray_push_back(tween_seq->state.sections, section);
        dynarray_push_back(tween_seq->limits_list, limits);
    }
    i = 2;
    {
        Vector2 prev_end = tween_seq->limits_list.items[i-1].end;
        TweenSequenceStateSection section = {
            .curve = animation_curve_delay(),
            .duration = 2,
        };
        TweenLimits_Vector2 limits = {
            .start = prev_end,
            .end = prev_end,
        };  
        dynarray_push_back(tween_seq->state.sections, section);
        dynarray_push_back(tween_seq->limits_list, limits);
    }
    i = 3;
    {
        Vector2 prev_end = tween_seq->limits_list.items[i-1].end;
        TweenSequenceStateSection section = {
            .curve = animation_curve_eased(ease_elastic_in_out),
            .duration = 2,
        };
        TweenLimits_Vector2 limits = {
            .start = prev_end,
            .end = {prev_end.x - pathlen, prev_end.y},
        };  
        dynarray_push_back(tween_seq->state.sections, section);
        dynarray_push_back(tween_seq->limits_list, limits);
    }
}

int main() {
    SET_LOG_LEVEL(LOG_LEVEL_ERROR);
    // SET_LOG_LEVEL(LOG_LEVEL_WARN);
    // SET_LOG_LEVEL(LOG_LEVEL_TRACE);

    InitWindow(1000, 1000, "Test");
    SetTargetFPS(60);

    Vector2 rect_origin_center = {25, 25};

    // -----

    Tween_Vector2 tweens[COUNT] = {0};
    reset_test_tweens(tweens);

    Rectangle rects[COUNT] = {0};
    for (uint32 i = 0; i < COUNT; i++) {
        rects[i].width = 50;
        rects[i].height = 50;
    }

    // -----

    TweenSequence_Vector2 tween_seq = {
        .state = new_tween_sequence_state(TWEEN_REPEAT_MIRRORED),
        .limits_list = {0},
    };
    reset_test_tween_sequence(&tween_seq);

    Rectangle rect = {
        .width = 50,
        .height = 50,
    };

    // -----

    uint32 val = 15;
    uint32 items[4] = {1, 2, 3, 4};
    TestStruct test_struct = {
        .bool_field = true,
        .uint_field = 14,
        .int_field = -2020,
        .cint_field = -150,
        .float_field = 17.899,
        .ptr_field = &val,
        .arr_field = {0},
        .cstr_field = "This is a cstr.",
        .string_field = (TestString) {
            .count = 10,
            .capacity = 15,
            .str = "This is a|string but cut off.",
        },
        .struct_field = (InnerTestStruct) {
            .count = 2,
            .capacity = 4,
            .items = items,
        },
        .struct_arr_field = {
            (InnerTestStruct) {
                .count = 1,
                .capacity = 4,
                .items = items,
            },
            (InnerTestStruct) {
                .count = 2,
                .capacity = 4,
                .items = items,
            },
            (InnerTestStruct) {
                .count = 3,
                .capacity = 4,
                .items = items,
            },
        }
    };
    TestStruct test_struct_arr[3] = {
        test_struct,
        test_struct,
        test_struct,
    };
    TestStruct_DynArray test_struct_dynarr = {0};
    dynarray_push_back(test_struct_dynarr, test_struct);
    dynarray_push_back(test_struct_dynarr, test_struct);
    dynarray_push_back(test_struct_dynarr, test_struct);

    while (!WindowShouldClose()) {
        float32 delta_time = GetFrameTime();

        if (IsKeyPressed(KEY_SPACE)) {
            reset_test_tweens(tweens);
            reset_test_tween_sequence(&tween_seq);
            printf("---\n");
            just_print(TestStruct)(&test_struct);
            printf("---\n");
            just_pretty_print(TestStruct)(&test_struct);
            printf("---\n");
            just_pretty_print_with(TestStruct)(&test_struct, (IndentToken) {.token = " ", .count = 4});
            printf("---\n");
            just_array_pretty_print_with(TestStruct)(test_struct_arr, 3, (IndentToken) {.token = " ", .count = 2});
            printf("---\n");
            just_pretty_print_with(TestStruct_DynArray)(&test_struct_dynarr, (IndentToken) {.token = " ", .count = 2});
            printf("---\n");
        }
        if (IsKeyPressed(KEY_P)) {
            StringBuilder builder = string_builder_new();

            String string1 = string_new();
            string_append_format(string1, "line1: %d, %d\n", 1, 2);
            string_append_format(string1, "[again] line1: %d, %d\n", 1, 2);
            string_hinted_append_format(string1, 12 + 3, "line2: %d, %d\n", 2, 3);
            string_builder_append_string_owned(&builder, string1);

            String string2 = string_new();
            string_append_cstr(&string2, "This is a cstr.");
            string_builder_append_string_owned(&builder, string2);

            String result_string = build_string(&builder);

            printf("-----\n");
            for (uint32 i = 0; i < result_string.count + 1; i++) {
                char c = result_string.str[i];
                if (c == '\0') {
                    printf("\\0");
                }
                else if (c == '\n') {
                    printf("\\n");
                }
                else {
                    printf("%c", c);
                }
            }
            printf("\n");
            printf("-----\n");
            printf("[len: %d]\n\"%s\"\n", result_string.count, result_string.cstr);
            printf("-----\n");
        }

        for (uint32 i = 0; i < COUNT; i++) {
            Vector2 pos = tween_tick(Vector2)(&tweens[i], delta_time);
            rects[i].x = pos.x;
            rects[i].y = pos.y;
        }

        Vector2 pos = tween_sequence_tick(Vector2)(&tween_seq, delta_time);
        rect.x = pos.x;
        rect.y = pos.y;

        BeginDrawing();
        ClearBackground(RAYWHITE);

        for (uint32 i = 0; i < COUNT; i++) {
            DrawRectanglePro(rects[i], rect_origin_center, 0, RED);
        }
        DrawRectanglePro(rect, rect_origin_center, 0, BLUE);

        EndDrawing();
    }


    return 0;
}