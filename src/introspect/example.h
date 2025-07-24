#pragma once

#include "introspect/introspect.h"

#include "depexample.h"

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
    char* cstr_field;
    TestString string_field;
    InnerTestStruct struct_field;
    InnerTestStruct struct_arr_field[3];
} TestStruct;

/**
 * ABOVE CODE SHOULD GENERATE BELOW CODE
 * THEN THE GENERATED CODE 
 */

#include "introspect/introspect.h"

// configure dependency includes
#include "depexample.h"

#pragma region FieldInfo Arrays

FieldInfo InnerTestStruct__fields[3];
FieldInfo TestStruct__fields[10];

FieldInfo InnerTestStruct__fields[] = {
    { .type = TYPE_usize, .name = "count", .ptr = &(((InnerTestStruct*)(0))->count) },
    { .type = TYPE_usize, .name = "capacity", .ptr = &(((InnerTestStruct*)(0))->capacity) },
    // { .type = TYPE_uint32, .name = "items", .ptr = &(((InnerTestStruct*)(0))->items), .is_ptr = true },
    { .type = TYPE_uint32, .name = "items", .ptr = &(((InnerTestStruct*)(0))->items), .is_dynarray = true, .count_ptr = &(((InnerTestStruct*)(0))->count) },
};


FieldInfo TestStruct__fields[] = {
    { .type = TYPE_bool, .name = "bool_field", .ptr = &(((TestStruct*)(0))->bool_field) },
    { .type = TYPE_uint32, .name = "uint_field", .ptr = &(((TestStruct*)(0))->uint_field) },
    { .type = TYPE_int32, .name = "int_field", .ptr = &(((TestStruct*)(0))->int_field) },
    { .type = TYPE_int32, .name = "cint_field", .ptr = &(((TestStruct*)(0))->cint_field) },
    { .type = TYPE_float32, .name = "float_field", .ptr = &(((TestStruct*)(0))->float_field) },
    { .type = TYPE_uint32, .name = "ptr_field", .ptr = &(((TestStruct*)(0))->ptr_field), .is_ptr = true },
    { .type = TYPE_uint32, .name = "arr_field", .ptr = &(((TestStruct*)(0))->arr_field), .is_array = true, .count = 10 },
    { .type = TYPE_char, .name = "cstr_field", .ptr = &(((TestStruct*)(0))->cstr_field), .is_cstr = true },
    {
        .type = TYPE_struct, .name = "string_field", .ptr = &(((TestStruct*)(0))->string_field),
        .struct_size = sizeof(((TestStruct*)(0))->string_field), .field_count = ARRAY_LENGTH(TestString__fields), .fields = TestString__fields
    },
    {
        .type = TYPE_struct, .name = "struct_field", .ptr = &(((TestStruct*)(0))->struct_field),
        .struct_size = sizeof(((TestStruct*)(0))->struct_field), .field_count = ARRAY_LENGTH(InnerTestStruct__fields), .fields = InnerTestStruct__fields
    },
    {
        .type = TYPE_struct, .name = "struct_arr_field", .ptr = &(((TestStruct*)(0))->struct_arr_field),
        .is_array = true, .count = 3,
        .struct_size = sizeof(((TestStruct*)(0))->struct_arr_field), .field_count = ARRAY_LENGTH(InnerTestStruct__fields), .fields = InnerTestStruct__fields
    },
};

#pragma endregion FieldInfo Arrays

// -----

#pragma region Print Functions

__IMPL_____generate_print_functions(TestStruct);

// static inline void TestStruct__print(TestStruct* var) {
//     struct__print(var, TestStruct__fields, ARRAY_LENGTH(TestStruct__fields));
// }
// static inline void TestStruct__pretty_print_with(TestStruct* var, uint32 indent, IndentToken indent_token) {
//     struct__pretty_print(var, TestStruct__fields, ARRAY_LENGTH(TestStruct__fields), indent, indent_token);
// }
// static inline void TestStruct__pretty_print(TestStruct* var, uint32 indent) {
//     TestStruct__pretty_print_with(var, 0, default_indent_token());
// }

// static inline void TestStruct_array__print(TestStruct* var, usize count) {
//     struct_array__print(var, count, sizeof(TestStruct), TestStruct__fields, ARRAY_LENGTH(TestStruct__fields));
// }
// static inline void TestStruct_array__pretty_print_with(TestStruct* var, usize count, uint32 indent, IndentToken indent_token) {
//     struct_array__pretty_print(var, count, sizeof(TestStruct), TestStruct__fields, ARRAY_LENGTH(TestStruct__fields), indent, indent_token);
// }
// static inline void TestStruct_array__pretty_print(TestStruct* var, usize count, uint32 indent) {
//     TestStruct_array__pretty_print_with(var, count, 0, default_indent_token());
// }

// static inline void TestStruct__print0(TestStruct* var) {
//     TestStruct__print(var);
//     printf("\n");
// }
// static inline void TestStruct__pretty_print0(TestStruct* var) {
//     TestStruct__pretty_print(var, 0);
//     printf("\n");
// }
// static inline void TestStruct__pretty_print_with0(TestStruct* var, IndentToken indent_token) {
//     TestStruct__pretty_print_with(var, 0, indent_token);
//     printf("\n");
// }

// static inline void TestStruct_array__print0(TestStruct* var, usize count) {
//     TestStruct_array__print(var, count);
//     printf("\n");
// }
// static inline void TestStruct_array__pretty_print0(TestStruct* var, usize count) {
//     TestStruct_array__pretty_print(var, count, 0);
//     printf("\n");
// }
// static inline void TestStruct_array__pretty_print_with0(TestStruct* var, usize count, IndentToken indent_token) {
//     TestStruct_array__pretty_print_with(var, count, 0, indent_token);
//     printf("\n");
// }

#pragma endregion Print Functions