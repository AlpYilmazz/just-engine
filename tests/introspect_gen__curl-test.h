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

static FieldInfo MyString__fields[4];
static FieldInfo MyResult__fields[2];
static FieldInfo TestIntro__fields[5];

static FieldInfo MyString__union_0__variants[] = {
	{
		.type = TYPE_char, .name = "str", .ptr = 0,
		.is_ptr = true, .ptr_depth = 1,
		.is_string = true, .count_ptr = &(((MyString*)(0))->count),
	},
	{
		.type = TYPE_char, .name = "cstr", .ptr = 0,
		.is_ptr = true, .ptr_depth = 1,
		.is_cstr = true,
	},
};

static FieldInfo MyString__fields[] = {
	{
		.type = TYPE_uint32, .name = "use_variant", .ptr = &(((MyString*)(0))->use_variant),
	},
	{
		.type = TYPE_usize, .name = "count", .ptr = &(((MyString*)(0))->count),
	},
	{
		.type = TYPE_usize, .name = "capacity", .ptr = &(((MyString*)(0))->capacity),
	},
	{
		.type = TYPE_union, .name = "str", .ptr = &(((MyString*)(0))->str),
		.is_named_union = false, .union_name = "(null)", .union_header_variant = 0,
		.is_discriminated_union = true, .discriminant_ptr = &(((MyString*)(0))->use_variant),
		.union_size = sizeof(union { char* str _mode_string__just_to_make_sure_no_token_overlap__(count); char* cstr _mode_cstr__just_to_make_sure_no_token_overlap__(); }), .variant_count = ARRAY_LENGTH(MyString__union_0__variants), .variants = MyString__union_0__variants,
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
		.type = TYPE_union, .name = "value.ok", .ptr = &(((MyResult*)(0))->value.ok),
		.is_named_union = true, .union_name = "value", .union_header_variant = 0,
		.is_discriminated_union = true, .discriminant_ptr = &(((MyResult*)(0))->is_ok),
		.union_size = sizeof(union { usize ok; char* err _mode_cstr__just_to_make_sure_no_token_overlap__(); }), .variant_count = ARRAY_LENGTH(MyResult__union_0__variants), .variants = MyResult__union_0__variants,
	},
};

static FieldInfo TestIntro__fields[] = {
	{
		.type = TYPE_usize, .name = "usize_ptr", .ptr = &(((TestIntro*)(0))->usize_ptr),
		.is_ptr = true, .ptr_depth = 1,
	},
	{
		.type = TYPE_struct, .name = "body_s", .ptr = &(((TestIntro*)(0))->body_s),
		.struct_size = sizeof(MyString), .field_count = ARRAY_LENGTH(MyString__fields), .fields = MyString__fields,
	},
	{
		.type = TYPE_struct, .name = "body_p", .ptr = &(((TestIntro*)(0))->body_p),
		.is_ptr = true, .ptr_depth = 1,
		.struct_size = sizeof(MyString), .field_count = ARRAY_LENGTH(MyString__fields), .fields = MyString__fields,
	},
	{
		.type = TYPE_struct, .name = "result_1", .ptr = &(((TestIntro*)(0))->result_1),
		.struct_size = sizeof(MyResult), .field_count = ARRAY_LENGTH(MyResult__fields), .fields = MyResult__fields,
	},
	{
		.type = TYPE_struct, .name = "result_2", .ptr = &(((TestIntro*)(0))->result_2),
		.struct_size = sizeof(MyResult), .field_count = ARRAY_LENGTH(MyResult__fields), .fields = MyResult__fields,
	},
};


__IMPL_____generate_print_functions(MyString);
__IMPL_____generate_print_functions(MyResult);
__IMPL_____generate_print_functions(TestIntro);

#endif

