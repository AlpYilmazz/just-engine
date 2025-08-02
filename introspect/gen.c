FieldInfo InnerTestStruct__fields[3];

FieldInfo InnerTestStruct__fields[] = {
	{
		.type = TYPE_usize, .name = "count", .ptr = &(((InnerTestStruct*)(0))->count),
	},
	{
		.type = TYPE_usize, .name = "capacity", .ptr = &(((InnerTestStruct*)(0))->capacity),
	},
	{
		.type = TYPE_uint32, .name = "items", .ptr = &(((InnerTestStruct*)(0))->items),
		.is_ptr = true, .ptr_depth = 1,
		.is_dynarray = true, .count_ptr = &(((InnerTestStruct*)(0))->count),
	},
};

