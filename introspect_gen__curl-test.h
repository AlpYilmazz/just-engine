#pragma once

#ifndef PRE_INTROSPECT_PASS

#include "justengine.h"

/**
 * !! IMPORTANT !!
 * IMPORT THIS HEADER AFTER THESE DEFINITONS
 * - WriteFnArg
*/

static FieldInfo WriteFnArg__fields[1];

static FieldInfo WriteFnArg__fields[] = {
	{
		.type = TYPE_struct, .name = "response_body", .ptr = &(((WriteFnArg*)(0))->response_body),
		.is_ptr = true, .ptr_depth = 1,
		.struct_size = sizeof(String), .field_count = ARRAY_LENGTH(String__fields), .fields = String__fields,
	},
};


__IMPL_____generate_print_functions(WriteFnArg);

#endif

