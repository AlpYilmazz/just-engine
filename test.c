#include "justengine.h"

#define introspect(...) 
#define alias(...) 

introspect(mode_dynarray, count: count, items: items)
typedef struct {
    usize count;
    usize capacity;
    uint32* items;
} InnerTestStruct;

introspect(mode_normal)
typedef struct {
    bool bool_field;
    uint32 uint_field;
    int32 int_field;
    alias(int32) int cint_field;
    float32 float_field;
    uint32* ptr_field;
    uint32 arr_field[10];
    InnerTestStruct struct_field1;
    InnerTestStruct struct_field2;
} TestStruct;

typedef enum {
    TYPE_bool,
    TYPE_uint32,
    TYPE_int32,
    TYPE_usize,
    TYPE_float32,
    TYPE_struct,
    TYPE_TestStruct,
    TYPE_InnerTestStruct,
} Type;

typedef struct FieldInfo {
    Type type;
    char* name;
    void* ptr;
    // --
    bool is_ptr;
    // --
    bool is_array;
    usize count;
    // --
    bool is_dynarray;
    void* count_ptr;
    // --
    bool is_struct;
    uint32 field_count;
    struct FieldInfo* fields;
    // --
} FieldInfo;

#define ARRAY_LENGTH(arr) (sizeof((arr)) / sizeof((arr)[0]))

FieldInfo InnerTestStruct__fields[3];
FieldInfo TestStruct__fields[9];

FieldInfo TestStruct__fields[] = {
    { .type = TYPE_bool, .name = "bool_field", .ptr = &(((TestStruct*)(0))->bool_field) },
    { .type = TYPE_uint32, .name = "uint_field", .ptr = &(((TestStruct*)(0))->uint_field) },
    { .type = TYPE_int32, .name = "int_field", .ptr = &(((TestStruct*)(0))->int_field) },
    { .type = TYPE_int32, .name = "cint_field", .ptr = &(((TestStruct*)(0))->cint_field) },
    { .type = TYPE_float32, .name = "float_field", .ptr = &(((TestStruct*)(0))->float_field) },
    { .type = TYPE_uint32, .name = "ptr_field", .ptr = &(((TestStruct*)(0))->ptr_field), .is_ptr = true },
    { .type = TYPE_uint32, .name = "arr_field", .ptr = &(((TestStruct*)(0))->arr_field), .is_array = true, .count = 10 },
    { .type = TYPE_InnerTestStruct, .name = "struct_field1", .ptr = &(((TestStruct*)(0))->struct_field1) },
    {
        .type = TYPE_struct, .name = "struct_field2", .ptr = &(((TestStruct*)(0))->struct_field2),
        .is_struct = true, .field_count = ARRAY_LENGTH(InnerTestStruct__fields), .fields = InnerTestStruct__fields
    },
};

FieldInfo InnerTestStruct__fields[] = {
    { .type = TYPE_usize, .name = "count", .ptr = &(((InnerTestStruct*)(0))->count) },
    { .type = TYPE_usize, .name = "capacity", .ptr = &(((InnerTestStruct*)(0))->capacity) },
    // { .type = TYPE_uint32, .name = "items", .ptr = &(((InnerTestStruct*)(0))->items), .is_ptr = true },
    { .type = TYPE_uint32, .name = "items", .ptr = &(((InnerTestStruct*)(0))->items), .is_dynarray = true, .count_ptr = &(((InnerTestStruct*)(0))->count) },
};

typedef struct {
    char* token;
    uint32 count;
} IndentToken;

static inline IndentToken default_indent_token() {
    return (IndentToken) {
        .token = "\t",
        .count = 1,
    };
}

void print_indent(uint32 indent_count, IndentToken indent_token) {
    for (uint32 i = 0; i < indent_count * indent_token.count; i++) {
        printf(indent_token.token);
    }
}

void introspect_field_print(FieldInfo field, void* var);
void introspect_field_pretty_print(FieldInfo field, void* var, uint32 indent, IndentToken indent_token);

void bool__print(bool* var) {
    if (*var) {
        printf("true");
    }
    else {
        printf("false");
    }
}

void uint32__print(uint32* var) {
    printf("%u", *var);
}
void uint32_ptr__print(uint32** ptr) {
    printf("<0x%p>(%u)", *ptr, **ptr);
}
void uint32_array__print(uint32* arr, usize count) {
    printf("[ ");
    for (uint32 i = 0; i < count; i++) {
        uint32__print(&arr[i]);
        if (i != count-1) {
            printf(", ");
        }
    }
    printf(" ]");
}
void uint32_array__pretty_print(uint32* arr, usize count, uint32 indent, IndentToken indent_token) {
    if (count == 0) {
        printf("[]");
        return;
    }
    printf("[\n");
    for (uint32 i = 0; i < count; i++) {
        print_indent(indent+1, indent_token);
        uint32__print(&arr[i]);
        printf(",\n");
    }
    print_indent(indent, indent_token);
    printf("]");
}

void int32__print(int32* var) {
    printf("%d", *var);
}

void usize__print(usize* var) {
    printf("%llu", *var);
}

void float32__print(float32* var) {
    printf("%0.2f", *var);
}

void ptr__print(void** var) {
    printf("<0x%p>", *var);
}

void struct__print(void* var, FieldInfo* fields, uint32 count) {
    printf("{ ");
    for (uint32 i = 0; i < count; i++) {
        FieldInfo field = fields[i];
        printf(field.name);
        printf(": ");
        introspect_field_print(field, var);
        if (i != count-1) {
            printf(", ");
        }
    }
    printf(" }");
}

void struct__pretty_print(void* var, FieldInfo* fields, uint32 count, uint32 indent, IndentToken indent_token) {
    printf("{\n");
    for (uint32 i = 0; i < count; i++) {
        FieldInfo field = fields[i];
        print_indent(indent+1, indent_token);
        printf(field.name);
        printf(": ");
        introspect_field_pretty_print(field, var, indent+1, indent_token);
        printf(",\n");
    }
    print_indent(indent, indent_token);
    printf("}");
}

#define just_print(Type) Type##__print0
#define just_pretty_print(Type) Type##__pretty_print0
#define just_pretty_print_with(Type) Type##__pretty_print_with0

void TestStruct__print(TestStruct* var) {
    struct__print(var, TestStruct__fields, ARRAY_LENGTH(TestStruct__fields));
}
void TestStruct__pretty_print_with(TestStruct* var, uint32 indent, IndentToken indent_token) {
    struct__pretty_print(var, TestStruct__fields, ARRAY_LENGTH(TestStruct__fields), indent, indent_token);
}
void TestStruct__pretty_print(TestStruct* var, uint32 indent) {
    TestStruct__pretty_print_with(var, 0, default_indent_token());
}
void TestStruct__print0(TestStruct* var) {
    TestStruct__print(var);
    printf("\n");
}
void TestStruct__pretty_print0(TestStruct* var) {
    TestStruct__pretty_print(var, 0);
    printf("\n");
}
void TestStruct__pretty_print_with0(TestStruct* var, IndentToken indent_token) {
    TestStruct__pretty_print_with(var, 0, indent_token);
    printf("\n");
}

void InnerTestStruct__print(InnerTestStruct* var) {
    struct__print(var, InnerTestStruct__fields, ARRAY_LENGTH(InnerTestStruct__fields));
    // printf("{ ");
    // printf("count: "); usize__print(&var->count); printf(", ");
    // printf("capacity: "); usize__print(&var->capacity); printf(", ");
    // printf("items: "); printf("[0x%p]", var->items); uint32_array__print(var->items, var->count);
    // printf(" }");
}
void InnerTestStruct__pretty_print_with(InnerTestStruct* var, uint32 indent, IndentToken indent_token) {
    struct__pretty_print(var, InnerTestStruct__fields, ARRAY_LENGTH(InnerTestStruct__fields), indent, indent_token);
}
void InnerTestStruct__pretty_print(InnerTestStruct* var, uint32 indent) {
    InnerTestStruct__pretty_print_with(var, 0, default_indent_token());
}
void InnerTestStruct__print0(InnerTestStruct* var) {
    InnerTestStruct__print(var);
    printf("\n");
}
void InnerTestStruct__pretty_print0(InnerTestStruct* var) {
    InnerTestStruct__pretty_print(var, 0);
    printf("\n");
}
void InnerTestStruct__pretty_print_with0(InnerTestStruct* var, IndentToken indent_token) {
    InnerTestStruct__pretty_print_with(var, 0, indent_token);
    printf("\n");
}

void introspect_field_print(FieldInfo field, void* var) {
    void* field_ptr = (void*)(((usize)var) + ((usize)field.ptr));
    switch (field.type) {
    case TYPE_bool:
        bool__print(field_ptr);
        break;
    case TYPE_uint32:
        if (field.is_ptr) {
            uint32_ptr__print(field_ptr);
        }
        else if (field.is_array) {
            uint32_array__print(field_ptr, field.count);
        }
        else if (field.is_dynarray) {
            uint32** items_ptr = field_ptr;
            usize* count = (void*)(((usize)var) + ((usize)field.count_ptr));
            ptr__print((void**)items_ptr);
            uint32_array__print(*items_ptr, *count);
        }
        else {
            uint32__print(field_ptr);
        }
        break;
    case TYPE_int32:
        // if (field.is_ptr) {
        //     int32_ptr__print(field_ptr);
        // }
        // else if (field.is_array) {
        //     int32_array__print(field_ptr, field.count);
        // }
        // else {
        //     int32__print(field_ptr);
        // }
        int32__print(field_ptr);
        break;
    case TYPE_usize:
        usize__print(field_ptr);
        break;
    case TYPE_float32:
        float32__print(field_ptr);
        break;
    case TYPE_struct:
        struct__print(field_ptr, field.fields, field.field_count);
        break;
    case TYPE_TestStruct:
        TestStruct__print(field_ptr);
        break;
    case TYPE_InnerTestStruct:
        InnerTestStruct__print(field_ptr);
        break;
    default:
        break;
    }
}

void introspect_field_pretty_print(FieldInfo field, void* var, uint32 indent, IndentToken indent_token) {
    void* field_ptr = (void*)(((usize)var) + ((usize)field.ptr));
    switch (field.type) {
    case TYPE_bool:
        bool__print(field_ptr);
        break;
    case TYPE_uint32:
        if (field.is_ptr) {
            uint32_ptr__print(field_ptr);
        }
        else if (field.is_array) {
            uint32_array__pretty_print(field_ptr, field.count, indent, indent_token);
        }
        else if (field.is_dynarray) {
            uint32** items_ptr = field_ptr;
            usize* count = (void*)(((usize)var) + ((usize)field.count_ptr));
            ptr__print((void**)items_ptr);
            uint32_array__pretty_print(*items_ptr, *count, indent, indent_token);
        }
        else {
            uint32__print(field_ptr);
        }
        break;
    case TYPE_int32:
        // if (field.is_ptr) {
        //     int32_ptr__print(field_ptr);
        // }
        // else if (field.is_array) {
        //     int32_array__print(field_ptr, field.count);
        // }
        // else {
        //     int32__print(field_ptr);
        // }
        int32__print(field_ptr);
        break;
    case TYPE_usize:
        usize__print(field_ptr);
        break;
    case TYPE_float32:
        float32__print(field_ptr);
        break;
    case TYPE_struct:
        struct__pretty_print(field_ptr, field.fields, field.field_count, indent, indent_token);
        break;
    case TYPE_TestStruct:
        TestStruct__pretty_print_with(field_ptr, indent, indent_token);
        break;
    case TYPE_InnerTestStruct:
        InnerTestStruct__pretty_print_with(field_ptr, indent, indent_token);
        break;
    default:
        break;
    }
}

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
        .struct_field1 = (InnerTestStruct) {
            .count = 2,
            .capacity = 4,
            .items = items,
        },
        .struct_field2 = (InnerTestStruct) {
            .count = 4,
            .capacity = 4,
            .items = items,
        },
    };

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
        }
        if (IsKeyPressed(KEY_P)) {
            StringBuilder builder = string_builder_new();

            String string1 = string_new();
            string_append_format(string1, "line1: %d, %d\n", 1, 2);
            string_append_format(string1, "[again] line1: %d, %d\n", 1, 2);
            string_hinted_append_format(string1, 12 + 3, "line2: %d, %d\n", 2, 3);
            string_builder_append_string(&builder, string1);

            String string2 = string_new();
            string_append_cstr(&string2, "This is a cstr.");
            string_builder_append_string(&builder, string2);

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