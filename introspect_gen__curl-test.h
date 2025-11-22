#pragma once

#ifndef PRE_INTROSPECT_PASS

#include "justengine.h"

/**
 * !! IMPORTANT !!
 * IMPORT THIS HEADER AFTER THESE DEFINITONS
 * - MyString
 * - MyResult
 * - TestIntro
*/

static FieldInfo MyString__fields[3];
static FieldInfo MyResult__fields[2];
static FieldInfo TestIntro__fields[2];

static FieldInfo MyString__union_0__variants[] = {
	{
		.type = TYPE_char, .name = "str", .ptr = 0,
		.is_ptr = true, .ptr_depth = 1,
	},
	{
		.type = TYPE_char, .name = "cstr", .ptr = 0,
		.is_ptr = true, .ptr_depth = 1,
	},
};

static FieldInfo MyString__fields[] = {
	{
		.type = TYPE_usize, .name = "count", .ptr = &(((MyString*)(0))->count),
	},
	{
		.type = TYPE_usize, .name = "capacity", .ptr = &(((MyString*)(0))->capacity),
	},
	{
		.type = TYPE_union, .name = "str", .ptr = &(((MyString*)(0))->str),
		.union_header_variant = 0,
		.union_size = sizeof(union { char* str; char* cstr; }), .field_count = ARRAY_LENGTH(MyString__union_0__variants), .fields = MyString__union_0__variants,
	},
};

static FieldInfo MyResult__union_0__variants[] = {
	{
		.type = TYPE_usize, .name = "ok", .ptr = 0,
	},
	{
		.type = TYPE_char, .name = "err", .ptr = 0,
		.is_ptr = true, .ptr_depth = 1,
		.is_cstr = true,
	},
};

static FieldInfo MyResult__fields[] = {
	{
		.type = TYPE_uint32, .name = "is_ok", .ptr = &(((MyResult*)(0))->is_ok),
	},
	{
		.type = TYPE_union, .name = "value", .ptr = &(((MyResult*)(0))->value),
		.union_header_variant = 0,
		.is_discriminated_union = true, .discriminant_ptr = &(((MyResult*)(0))->is_ok),
		.union_size = sizeof(union { usize ok; char* err _mode_cstr__just_to_make_sure_no_token_overlap__(); }), .field_count = ARRAY_LENGTH(MyResult__union_0__variants), .fields = MyResult__union_0__variants,
	},
};

static FieldInfo TestIntro__fields[] = {
	{
		.type = TYPE_struct, .name = "body", .ptr = &(((TestIntro*)(0))->body),
		.is_ptr = true, .ptr_depth = 1,
		.struct_size = sizeof(MyString), .field_count = ARRAY_LENGTH(MyString__fields), .fields = MyString__fields,
	},
	{
		.type = TYPE_struct, .name = "result", .ptr = &(((TestIntro*)(0))->result),
		.struct_size = sizeof(MyResult), .field_count = ARRAY_LENGTH(MyResult__fields), .fields = MyResult__fields,
	},
};


__IMPL_____generate_print_functions(MyString);
__IMPL_____generate_print_functions(MyResult);
__IMPL_____generate_print_functions(TestIntro);

#endif

