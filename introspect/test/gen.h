#pragma once

#include "justengine.h"

/**
 * !! IMPORTANT !!
 * IMPORT THIS HEADER AFTER THESE DEFINITONS
 * - TestString
 * - InnerTestStruct
 * - TestStruct
 * - TestStruct_DynArray
*/

static FieldInfo TestString__fields[3];
static FieldInfo InnerTestStruct__fields[3];
static FieldInfo TestStruct__fields[12];
static FieldInfo TestStruct_DynArray__fields[3];

static FieldInfo TestString__fields[] = {
	{
		.type = TYPE_usize, .name = "count", .ptr = &(((TestString*)(0))->count),
	},
	{
		.type = TYPE_usize, .name = "capacity", .ptr = &(((TestString*)(0))->capacity),
	},
	{
		.type = TYPE_char, .name = "str", .ptr = &(((TestString*)(0))->str),
		.is_ptr = true, .ptr_depth = 1,
	},
};

static FieldInfo InnerTestStruct__fields[] = {
	{
		.type = TYPE_usize, .name = "count", .ptr = &(((InnerTestStruct*)(0))->count),
	},
	{
		.type = TYPE_usize, .name = "capacity", .ptr = &(((InnerTestStruct*)(0))->capacity),
	},
	{
		.type = TYPE_uint32, .name = "items", .ptr = &(((InnerTestStruct*)(0))->items),
		.is_ptr = true, .ptr_depth = 1,
	},
};

static FieldInfo TestStruct__fields[] = {
	{
		.type = TYPE_bool, .name = "bool_field", .ptr = &(((TestStruct*)(0))->bool_field),
	},
	{
		.type = TYPE_uint32, .name = "uint_field", .ptr = &(((TestStruct*)(0))->uint_field),
	},
	{
		.type = TYPE_int32, .name = "int_field", .ptr = &(((TestStruct*)(0))->int_field),
	},
	{
		.type = TYPE_int32, .name = "cint_field", .ptr = &(((TestStruct*)(0))->cint_field),
	},
	{
		.type = TYPE_float32, .name = "float_field", .ptr = &(((TestStruct*)(0))->float_field),
	},
	{
		.type = TYPE_uint32, .name = "ptr_field", .ptr = &(((TestStruct*)(0))->ptr_field),
		.is_ptr = true, .ptr_depth = 1,
	},
	{
		.type = TYPE_uint32, .name = "arr_field", .ptr = &(((TestStruct*)(0))->arr_field),
		.is_array = true, .count = 10, .array_dim = 1, .array_dim_counts = {10},
	},
	{
		.type = TYPE_char, .name = "cstr_field", .ptr = &(((TestStruct*)(0))->cstr_field),
		.is_ptr = true, .ptr_depth = 1,
		.is_cstr = true,
	},
	{
		.type = TYPE_uint32, .name = "dynarray_field", .ptr = &(((TestStruct*)(0))->dynarray_field),
		.is_ptr = true, .ptr_depth = 1,
		.is_dynarray = true, .count_ptr = &(((TestStruct*)(0))->uint_field),
	},
	{
		.type = TYPE_struct, .name = "string_field", .ptr = &(((TestStruct*)(0))->string_field),
		.struct_size = sizeof(TestString), .field_count = ARRAY_LENGTH(TestString__fields), .fields = TestString__fields,
	},
	{
		.type = TYPE_struct, .name = "struct_field", .ptr = &(((TestStruct*)(0))->struct_field),
		.struct_size = sizeof(InnerTestStruct), .field_count = ARRAY_LENGTH(InnerTestStruct__fields), .fields = InnerTestStruct__fields,
	},
	{
		.type = TYPE_struct, .name = "struct_arr_field", .ptr = &(((TestStruct*)(0))->struct_arr_field),
		.is_array = true, .count = 3, .array_dim = 1, .array_dim_counts = {3},
		.struct_size = sizeof(InnerTestStruct), .field_count = ARRAY_LENGTH(InnerTestStruct__fields), .fields = InnerTestStruct__fields,
	},
};

static FieldInfo TestStruct_DynArray__fields[] = {
	{
		.type = TYPE_usize, .name = "count", .ptr = &(((TestStruct_DynArray*)(0))->count),
	},
	{
		.type = TYPE_usize, .name = "capacity", .ptr = &(((TestStruct_DynArray*)(0))->capacity),
	},
	{
		.type = TYPE_struct, .name = "items", .ptr = &(((TestStruct_DynArray*)(0))->items),
		.is_ptr = true, .ptr_depth = 1,
		.is_dynarray = true, .count_ptr = &(((TestStruct_DynArray*)(0))->count),
		.struct_size = sizeof(TestStruct), .field_count = ARRAY_LENGTH(TestStruct__fields), .fields = TestStruct__fields,
	},
};


__IMPL_____generate_print_functions(TestString);
__IMPL_____generate_print_functions(InnerTestStruct);
__IMPL_____generate_print_functions(TestStruct);
__IMPL_____generate_print_functions(TestStruct_DynArray);

