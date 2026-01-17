#pragma once

#ifndef PRE_INTROSPECT_PASS

#include "justengine.h"

/**
 * !! IMPORTANT !!
 * IMPORT THIS HEADER AFTER THESE DEFINITONS
 * - C_HttpResponse
*/

static FieldInfo C_HttpResponse__fields[2];

static FieldInfo C_HttpResponse__fields[] = {
	{
		.type = TYPE_uint32, .name = "status_code", .ptr = &(((C_HttpResponse*)(0))->status_code),
	},
	{
		.type = TYPE_char, .name = "body", .ptr = &(((C_HttpResponse*)(0))->body),
		.is_ptr = true, .ptr_depth = 1,
		.is_cstr = true,
	},
};

__IMPL_____generate_print_functions(C_HttpResponse);

#endif

