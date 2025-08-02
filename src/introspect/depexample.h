#pragma once

#include "introspect/introspect.h"

introspect
typedef struct {
    usize count;
    usize capacity;
    char* str;
} TestString;

/**
 * ABOVE CODE SHOULD GENERATE BELOW CODE
 * THEN THE GENERATED CODE 
 */

FieldInfo TestString__fields[3];

FieldInfo TestString__fields[] = {
    { .type = TYPE_usize, .name = "count", .ptr = &(((TestString*)(0))->count) },
    { .type = TYPE_usize, .name = "capacity", .ptr = &(((TestString*)(0))->capacity) },
    { .type = TYPE_char, .name = "str", .ptr = &(((TestString*)(0))->str), .is_string = true, .count_ptr = &(((TestString*)(0))->count) },
};

__IMPL_____generate_print_functions(TestString);
