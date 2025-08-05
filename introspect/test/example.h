#pragma once

#include "justengine.h"

#include "depexample.h"

introspect
typedef struct {
    usize count;
    usize capacity;
    uint32* items;
} InnerTestStruct;

introspect
typedef struct {
    bool bool_field;
    uint32 uint_field;
    int32 int_field;
    int cint_field alias(int32);
    float32 float_field;
    uint32* ptr_field;
    uint32 arr_field[10];
    char* cstr_field mode_cstr();
    uint32* dynarray_field mode_dynarray(count: uint_field);
    TestString string_field;
    InnerTestStruct struct_field;
    InnerTestStruct struct_arr_field[3];
} TestStruct;

// --

#define DEFINE_DYNARRAY(Type) \
    introspect typedef struct { \
        usize count; \
        usize capacity; \
        Type* items mode_dynarray(count: count); \
    } Type##_DynArray;

// --

DEFINE_DYNARRAY(TestStruct)

