
#include "introspect.h"

void print_indent(uint32 indent_count, IndentToken indent_token) {
    for (uint32 i = 0; i < indent_count * indent_token.count; i++) {
        printf("%s", indent_token.token);
    }
}

#define __IMPL_____print_functions__stdout(TYPE, format) \
    void TYPE##__print(TYPE var) { \
        printf(format, var); \
    } \
    void TYPE##_ptr__print(TYPE* ptr) { \
        ptr__print(ptr); \
        if (ptr != NULL) { \
            printf(" "); \
            TYPE##__print(*ptr); \
        } \
    } \
    void TYPE##_array__print(TYPE* arr, usize count) { \
        printf("[ "); \
        for (usize i = 0; i < count; i++) { \
            TYPE##__print(arr[i]); \
            if (i != count-1) { \
                printf(", "); \
            } \
        } \
        printf(" ]"); \
    } \
    void TYPE##_array__pretty_print(TYPE* arr, usize count, uint32 indent, IndentToken indent_token) { \
        if (count == 0) { \
            printf("[]"); \
            return; \
        } \
        printf("[\n"); \
        for (usize i = 0; i < count; i++) { \
            print_indent(indent+1, indent_token); \
            TYPE##__print(arr[i]); \
            printf(",\n"); \
        } \
        print_indent(indent, indent_token); \
        printf("]"); \
    } \
    void TYPE##_dynarray__print(TYPE* arr, usize count) { \
        ptr__print(arr); \
        if (arr != NULL) { \
            printf(" "); \
            TYPE##_array__print(arr, count); \
        } \
    } \
    void TYPE##_dynarray__pretty_print(TYPE* arr, usize count, uint32 indent, IndentToken indent_token) { \
        ptr__print(arr); \
        if (arr != NULL) { \
            printf(" "); \
            TYPE##_array__pretty_print(arr, count, indent, indent_token); \
        } \
    } \
    typedef enum { TYPE##__VARIANT__IMPL_____print_functions__stdout = 0 } TYPE##__ENUM__IMPL_____print_functions__stdout

void ptr__print(void* var) {
    if (var == NULL) {
        printf("<NULL>");
    }
    else {
        printf("<0x%p>", var);
    }
}

void ptr_array__print(void** arr, usize count) {
    printf("[ ");
    for (usize i = 0; i < count; i++) {
        ptr__print(arr[i]);
        if (i != count-1) {
            printf(", ");
        }
    }
    printf(" ]");
}
void ptr_array__pretty_print(void** arr, usize count, uint32 indent, IndentToken indent_token) {
    if (count == 0) {
        printf("[]");
        return;
    }
    printf("[\n");
    for (usize i = 0; i < count; i++) {
        print_indent(indent+1, indent_token);
        ptr__print(arr[i]);
        printf(",\n");
    }
    print_indent(indent, indent_token);
    printf("]");
}

__IMPL_____print_functions__stdout(char, "%c");
__IMPL_____print_functions__stdout(byte, "%.2x");
// __IMPL_____print_functions__stdout(bool, "%hhu");
__IMPL_____print_functions__stdout(uint8, "%hhu");
__IMPL_____print_functions__stdout(uint16, "%hu");
__IMPL_____print_functions__stdout(uint32, "%u");
__IMPL_____print_functions__stdout(uint64, "%llu");
__IMPL_____print_functions__stdout(int8, "%hhd");
__IMPL_____print_functions__stdout(int16, "%hd");
__IMPL_____print_functions__stdout(int32, "%d");
__IMPL_____print_functions__stdout(int64, "%lld");
__IMPL_____print_functions__stdout(float32, "%f");
__IMPL_____print_functions__stdout(float64, "%Lf");
__IMPL_____print_functions__stdout(usize, "%llu");

void char_cstr__print(char* cstr) {
    ptr__print(cstr);
    if (cstr != NULL) {
        printf(" ");
        printf("\"");
        for (usize i = 0; cstr[i] != '\0'; i++) {
            printf("%c", cstr[i]);
        }
        printf("\"");
    }
}
void char_string__print(char* str, usize count) {
    ptr__print(str);
    if (str != NULL) {
        printf(" ");
        printf("\"");
        for (usize i = 0; i < count; i++) {
            printf("%c", str[i]);
        }
        printf("\"");
    }
}

void bool__print(bool var) {
    if (var) {
        printf("true");
    }
    else {
        printf("false");
    }
}
void bool_ptr__print(bool* ptr) {
    ptr__print(ptr);
    if (ptr != NULL) {
        printf(" ");
        bool__print(*ptr);
    }
}
void bool_array__print(bool* arr, usize count) {
    printf("[ ");
    for (usize i = 0; i < count; i++) {
        bool__print(arr[i]);
        if (i != count-1) {
            printf(", ");
        }
    }
    printf(" ]");
}
void bool_array__pretty_print(bool* arr, usize count, uint32 indent, IndentToken indent_token) {
    if (count == 0) {
        printf("[]");
        return;
    }
    printf("[\n");
    for (usize i = 0; i < count; i++) {
        print_indent(indent+1, indent_token);
        bool__print(arr[i]);
        printf(",\n");
    }
    print_indent(indent, indent_token);
    printf("]");
}
void bool_dynarray__print(bool* arr, usize count) {
    ptr__print(arr);
    if (arr != NULL) {
        printf(" ");
        bool_array__print(arr, count);
    }
}
void bool_dynarray__pretty_print(bool* arr, usize count, uint32 indent, IndentToken indent_token) {
    ptr__print(arr);
    if (arr != NULL) {
        printf(" ");
        bool_array__pretty_print(arr, count, indent, indent_token);
    }
}

void struct__print(void* var, FieldInfo* fields, uint32 field_count) {
    printf("{ ");
    for (uint32 i = 0; i < field_count; i++) {
        FieldInfo field = fields[i];
        printf(field.name);
        printf(": ");
        introspect_field_print(field, var);
        if (i != field_count-1) {
            printf(", ");
        }
    }
    printf(" }");
}
void struct__pretty_print(void* var, FieldInfo* fields, uint32 field_count, uint32 indent, IndentToken indent_token) {
    printf("{\n");
    for (uint32 i = 0; i < field_count; i++) {
        FieldInfo field = fields[i];
        print_indent(indent+1, indent_token);
        // TODO: think better way
        if (!field.is_discriminated_union) {
            printf(field.name);
            printf(": ");
        }
        introspect_field_pretty_print(field, var, indent+1, indent_token);
        printf(",\n");
    }
    print_indent(indent, indent_token);
    printf("}");
}
void struct_ptr__print(void** ptr, FieldInfo* fields, uint32 field_count) {
    ptr__print(ptr);
    if (ptr != NULL) {
        printf(" ");
        struct__print(*ptr, fields, field_count);
    }
}
void struct_ptr__pretty_print(void** ptr, FieldInfo* fields, uint32 field_count, uint32 indent, IndentToken indent_token) {
    ptr__print(ptr);
    if (ptr != NULL) {
        printf(" ");
        struct__pretty_print(*ptr, fields, field_count, indent, indent_token);
    }
}
void struct_array__print(void* arr, usize count, uint32 struct_size, FieldInfo* fields, uint32 field_count) {
    printf("[ ");
    for (uint32 i = 0; i < count; i++) {
        void* var = (((byte*)arr) + (struct_size * i));
        struct__print(var, fields, field_count);
    }
    printf(" ]");
}
void struct_array__pretty_print(void* arr, usize count, uint32 struct_size, FieldInfo* fields, uint32 field_count, uint32 indent, IndentToken indent_token) {
    if (count == 0) {
        printf("[]");
        return;
    }
    printf("[\n");
    for (uint32 i = 0; i < count; i++) {
        void* var = (((byte*)arr) + (struct_size * i));
        print_indent(indent+1, indent_token);
        struct__pretty_print(var, fields, field_count, indent+1, indent_token);
        printf(",\n");
    }
    print_indent(indent, indent_token);
    printf("]");
}
void struct_dynarray__print(void* arr, usize count, uint32 struct_size, FieldInfo* fields, uint32 field_count) {
    ptr__print(arr);
    if (arr != NULL) {
        printf(" ");
        struct_array__print(arr, count, struct_size, fields, field_count);
    }
}
void struct_dynarray__pretty_print(void* arr, usize count, uint32 struct_size, FieldInfo* fields, uint32 field_count, uint32 indent, IndentToken indent_token) {
    ptr__print(arr);
    if (arr != NULL) {
        printf(" ");
        struct_array__pretty_print(arr, count, struct_size, fields, field_count, indent, indent_token);
    }
}

void union__print(void* struct_var, void* var, FieldInfo self, uint32 variant_index, FieldInfo* variants, uint32 variant_count) {
    FieldInfo variant = variants[variant_index];
    variant.ptr = self.ptr;
    if (self.is_discriminated_union) {
        if (self.is_named_union) {
            printf(self.union_name);
            printf(".");
        }
        printf(variant.name);
        printf(": ");
    }
    introspect_field_print(variant, struct_var);
}
void union__pretty_print(void* struct_var, void* var, FieldInfo self, uint32 variant_index, FieldInfo* variants, uint32 variant_count, uint32 indent, IndentToken indent_token) {
    FieldInfo variant = variants[variant_index];
    variant.ptr = self.ptr;
    if (self.is_discriminated_union) {
        if (self.is_named_union) {
            printf(self.union_name);
            printf(".");
        }
        printf(variant.name);
        printf(": ");
    }
    introspect_field_pretty_print(variant, struct_var, indent, indent_token);
}
void union_ptr__print(void* struct_var, void** ptr, FieldInfo self, uint32 variant_index, FieldInfo* variants, uint32 variant_count) {
    ptr__print(ptr);
    if (ptr != NULL) {
        printf(" ");
        union__print(struct_var, *ptr, self, variant_index, variants, variant_count);
    }
}
void union_ptr__pretty_print(void* struct_var, void** ptr, FieldInfo self, uint32 variant_index, FieldInfo* variants, uint32 variant_count, uint32 indent, IndentToken indent_token) {
    ptr__print(ptr);
    if (ptr != NULL) {
        printf(" ");
        union__pretty_print(struct_var, *ptr, self, variant_index, variants, variant_count, indent, indent_token);
    }
}
void union_array__print(void* struct_var, void* arr, usize count, uint32 union_size, FieldInfo self, uint32 variant_index, FieldInfo* variants, uint32 variant_count) {
    printf("[ ");
    for (uint32 i = 0; i < count; i++) {
        void* var = (((byte*)arr) + (union_size * i));
        union__print(struct_var, var, self, variant_index, variants, variant_count);
    }
    printf(" ]");
}
void union_array__pretty_print(void* struct_var, void* arr, usize count, uint32 union_size, FieldInfo self, uint32 variant_index, FieldInfo* variants, uint32 variant_count, uint32 indent, IndentToken indent_token) {
    if (count == 0) {
        printf("[]");
        return;
    }
    printf("[\n");
    for (uint32 i = 0; i < count; i++) {
        void* var = (((byte*)arr) + (union_size * i));
        print_indent(indent+1, indent_token);
        union__pretty_print(struct_var, var, self, variant_index, variants, variant_count, indent, indent_token);
        printf(",\n");
    }
    print_indent(indent, indent_token);
    printf("]");
}
void union_dynarray__print(void* struct_var, void* arr, usize count, uint32 union_size, FieldInfo self, uint32 variant_index, FieldInfo* variants, uint32 variant_count) {
    ptr__print(arr);
    if (arr != NULL) {
        printf(" ");
        union_array__print(struct_var, arr, count, union_size, self, variant_index, variants, variant_count);
    }
}
void union_dynarray__pretty_print(void* struct_var, void* arr, usize count, uint32 union_size, FieldInfo self, uint32 variant_index, FieldInfo* variants, uint32 variant_count, uint32 indent, IndentToken indent_token) {
    ptr__print(arr);
    if (arr != NULL) {
        printf(" ");
        union_array__pretty_print(struct_var, arr, count, union_size, self, variant_index, variants, variant_count, indent, indent_token);
    }
}

#define field_print(TYPE, field) \
    do { \
        void* field_ptr = (void*)(((usize)var) + ((usize)(field).ptr)); \
        if ((field).is_dynarray) { \
            TYPE** items_ptr = field_ptr; \
            usize* count = (void*)(((usize)var) + ((usize)(field).count_ptr)); \
            TYPE##_dynarray__print(*items_ptr, *count); \
        } \
        else if ((field).is_array) { \
            if ((field).is_ptr) { \
                void** var_ptr = field_ptr; \
                ptr_array__print(var_ptr, (field).count); \
            } \
            else { \
                TYPE* var_ptr = field_ptr; \
                TYPE##_array__print(var_ptr, (field).count); \
            } \
        } \
        else if ((field).is_ptr) { \
            TYPE** var_ptr = field_ptr; \
            TYPE##_ptr__print(*var_ptr); \
        } \
        else { \
            TYPE* var_ptr = field_ptr; \
            TYPE##__print(*var_ptr); \
        } \
    } while (0)

#define field_pretty_print(TYPE, field, ...) \
    do { \
        void* field_ptr = (void*)(((usize)var) + ((usize)(field).ptr)); \
        if ((field).is_dynarray) { \
            TYPE** items_ptr = field_ptr; \
            usize* count = (void*)(((usize)var) + ((usize)(field).count_ptr)); \
            TYPE##_dynarray__pretty_print(*items_ptr, *count, __VA_ARGS__); \
        } \
        else if ((field).is_array) { \
            if ((field).is_ptr) { \
                void** var_ptr = field_ptr; \
                ptr_array__pretty_print(var_ptr, (field).count, __VA_ARGS__); \
            } \
            else { \
                TYPE* var_ptr = field_ptr; \
                TYPE##_array__pretty_print(var_ptr, (field).count, __VA_ARGS__); \
            } \
        } \
        else if ((field).is_ptr) { \
            TYPE** var_ptr = field_ptr; \
            TYPE##_ptr__print(*var_ptr); \
        } \
        else { \
            TYPE* var_ptr = field_ptr; \
            TYPE##__print(*var_ptr); \
        } \
    } while (0)

void introspect_field_print(FieldInfo field, void* var) {
    switch (field.type) {
    case TYPE_void: {
        if (!field.is_ptr) {
            PANIC("TYPE_void has to be a pointer, void field is not allowed.\n");
        }
        void* field_ptr = (void*)(((usize)var) + ((usize)field.ptr));
        void** var_ptr = field_ptr;
        ptr__print(*var_ptr);
        break;
    }
    case TYPE_char: {
        void* field_ptr = (void*)(((usize)var) + ((usize)field.ptr));

        if (field.is_cstr) {
            char** cstr_ptr = field_ptr;
            char_cstr__print(*cstr_ptr);
        }
        else if (field.is_string) {
            char** str_ptr = field_ptr;
            usize* count = (void*)(((usize)var) + ((usize)field.count_ptr));
            char_string__print(*str_ptr, *count);
        }
        else if (field.is_dynarray) {
            char** items_ptr = field_ptr;
            usize* count = (void*)(((usize)var) + ((usize)field.count_ptr));
            char_dynarray__print(*items_ptr, *count);
        }
        else if (field.is_array) {
            if (field.is_ptr) {
                void** var_ptr = field_ptr;
                ptr_array__print(var_ptr, field.count);
            }
            else {
                char* var_ptr = field_ptr;
                char_array__print(var_ptr, field.count);
            }
        }
        else if (field.is_ptr) {
            char** var_ptr = field_ptr;
            char_ptr__print(*var_ptr);
        }
        else {
            char* var_ptr = field_ptr;
            char__print(*var_ptr);
        }
        break;
    }
    case TYPE_bool:
        field_print(bool, field);
        break;
    case TYPE_byte:
        field_print(byte, field);
        break;
    case TYPE_uint8:
        field_print(uint8, field);
        break;
    case TYPE_uint16:
        field_print(uint16, field);
        break;
    case TYPE_uint32:
        field_print(uint32, field);
        break;
    case TYPE_uint64:
        field_print(uint64, field);
        break;
    case TYPE_int8:
        field_print(int8, field);
        break;
    case TYPE_int16:
        field_print(int16, field);
        break;
    case TYPE_int32:
        field_print(int32, field);
        break;
    case TYPE_int64:
        field_print(int64, field);
        break;
    case TYPE_usize:
        // JUST_DEV_MARK();
        void* field_ptr = (void*)(((usize)var) + ((usize)field.ptr));
        usize* p = field_ptr;
        field_print(usize, field);
        break;
    case TYPE_float32:
        field_print(float32, field);
        break;
    case TYPE_float64:
        field_print(float64, field);
        break;
    case TYPE_struct: {
        void* field_ptr = (void*)(((usize)var) + ((usize)field.ptr));
        if ((field).is_dynarray) {
            void** items_ptr = field_ptr;
            usize* count = (void*)(((usize)var) + ((usize)(field).count_ptr));
            struct_dynarray__print(*items_ptr, *count, field.struct_size, field.fields, field.field_count);
        }
        else if ((field).is_array) {
            if (field.is_ptr) {
                void** items_ptr = field_ptr;
                ptr_array__print(items_ptr, field.count);
            }
            else {
                struct_array__print(field_ptr, field.count, field.struct_size, field.fields, field.field_count);
            }
        }
        else if ((field).is_ptr) {
            void** var_ptr = field_ptr;
            struct_ptr__print(var_ptr, field.fields, field.field_count);
        }
        else {
            struct__print(field_ptr, field.fields, field.field_count);
        }
        break;
    }
    case TYPE_union: {
        void* field_ptr = (void*)(((usize)var) + ((usize)field.ptr));
        uint32 variant_index = field.union_header_variant;
        if (field.is_discriminated_union) {
            uint32* discriminant_value = (void*)(((usize)var) + ((usize)(field).discriminant_ptr));
            variant_index = *discriminant_value;
        }
        variant_index = MIN(variant_index, field.variant_count-1);

        if ((field).is_dynarray) {
            void** items_ptr = field_ptr;
            usize* count = (void*)(((usize)var) + ((usize)(field).count_ptr));
            union_dynarray__print(var, *items_ptr, *count, field.union_size, field, variant_index, field.variants, field.variant_count);
        }
        else if ((field).is_array) {
            if (field.is_ptr) {
                void** items_ptr = field_ptr;
                ptr_array__print(items_ptr, field.count);
            }
            else {
                union_array__print(var, field_ptr, field.count, field.union_size, field, variant_index, field.variants, field.variant_count);
            }
        }
        else if ((field).is_ptr) {
            void** var_ptr = field_ptr;
            union_ptr__print(var, var_ptr, field, variant_index, field.variants, field.variant_count);
        }
        else {
            union__print(var, field_ptr, field, variant_index, field.variants, field.variant_count);
        }
        break;
    }
    default:
        UNREACHABLE();
        break;
    }
}

void introspect_field_pretty_print(FieldInfo field, void* var, uint32 indent, IndentToken indent_token) {
    switch (field.type) {
    case TYPE_char: {
        void* field_ptr = (void*)(((usize)var) + ((usize)field.ptr));
        if (field.is_cstr) {
            char** cstr_ptr = field_ptr;
            char_cstr__print(*cstr_ptr);
        }
        else if (field.is_string) {
            char** str_ptr = field_ptr;
            usize* count = (void*)(((usize)var) + ((usize)field.count_ptr));
            char_string__print(*str_ptr, *count);
        }
        else if (field.is_dynarray) {
            char** items_ptr = field_ptr;
            usize* count = (void*)(((usize)var) + ((usize)field.count_ptr));
            char_dynarray__pretty_print(*items_ptr, *count, indent, indent_token);
        }
        else if (field.is_array) {
            if (field.is_ptr) {
                void** var_ptr = field_ptr;
                ptr_array__pretty_print(var_ptr, field.count, indent, indent_token);
            }
            else {
                char* var_ptr = field_ptr;
                char_array__pretty_print(var_ptr, field.count, indent, indent_token);
            }
        }
        else if (field.is_ptr) {
            char** var_ptr = field_ptr;
            char_ptr__print(*var_ptr);
        }
        else {
            char* var_ptr = field_ptr;
            char__print(*var_ptr);
        }
        break;
    }
    case TYPE_bool:
        field_pretty_print(bool, field, indent, indent_token);
        break;
    case TYPE_byte:
        field_pretty_print(byte, field, indent, indent_token);
        break;
    case TYPE_uint8:
        field_pretty_print(uint8, field, indent, indent_token);
        break;
    case TYPE_uint16:
        field_pretty_print(uint16, field, indent, indent_token);
        break;
    case TYPE_uint32:
        field_pretty_print(uint32, field, indent, indent_token);
        break;
    case TYPE_uint64:
        field_pretty_print(uint64, field, indent, indent_token);
        break;
    case TYPE_int8:
        field_pretty_print(int8, field, indent, indent_token);
        break;
    case TYPE_int16:
        field_pretty_print(int16, field, indent, indent_token);
        break;
    case TYPE_int32:
        field_pretty_print(int32, field, indent, indent_token);
        break;
    case TYPE_int64:
        field_pretty_print(int64, field, indent, indent_token);
        break;
    case TYPE_usize:
        void* field_ptr = (void*)(((usize)var) + ((usize)field.ptr));
        usize* p = field_ptr;
        field_pretty_print(usize, field, indent, indent_token);
        break;
    case TYPE_float32:
        field_pretty_print(float32, field, indent, indent_token);
        break;
    case TYPE_float64:
        field_pretty_print(float64, field, indent, indent_token);
        break;
    case TYPE_struct: {
        void* field_ptr = (void*)(((usize)var) + ((usize)field.ptr));
        if ((field).is_dynarray) {
            void** items_ptr = field_ptr;
            usize* count = (void*)(((usize)var) + ((usize)(field).count_ptr));
            struct_dynarray__pretty_print(*items_ptr, *count, field.struct_size, field.fields, field.field_count, indent, indent_token);
        }
        else if ((field).is_array) {
            if (field.is_ptr) {
                void** items_ptr = field_ptr;
                ptr_array__pretty_print(items_ptr, field.count, indent, indent_token);
            }
            else {
                struct_array__pretty_print(field_ptr, field.count, field.struct_size, field.fields, field.field_count, indent, indent_token);
            }
        }
        else if ((field).is_ptr) {
            void** var_ptr = field_ptr;
            struct_ptr__pretty_print(var_ptr, field.fields, field.field_count, indent, indent_token);
        }
        else {
            struct__pretty_print(field_ptr, field.fields, field.field_count, indent, indent_token);
        }
        break;
    }
    case TYPE_union: {
        void* field_ptr = (void*)(((usize)var) + ((usize)field.ptr));
        uint32 variant_index = field.union_header_variant;
        if (field.is_discriminated_union) {
            uint32* discriminant_value = (void*)(((usize)var) + ((usize)(field).discriminant_ptr));
            variant_index = *discriminant_value;
        }
        variant_index = MIN(variant_index, field.variant_count-1);

        if ((field).is_dynarray) {
            void** items_ptr = field_ptr;
            usize* count = (void*)(((usize)var) + ((usize)(field).count_ptr));
            union_dynarray__pretty_print(var, *items_ptr, *count, field.union_size, field, variant_index, field.variants, field.variant_count, indent, indent_token);
        }
        else if ((field).is_array) {
            if (field.is_ptr) {
                void** items_ptr = field_ptr;
                ptr_array__pretty_print(items_ptr, field.count, indent, indent_token);
            }
            else {
                union_array__pretty_print(var, field_ptr, field.count, field.union_size, field, variant_index, field.variants, field.variant_count, indent, indent_token);
            }
        }
        else if ((field).is_ptr) {
            void** var_ptr = field_ptr;
            union_ptr__pretty_print(var, var_ptr, field, variant_index, field.variants, field.variant_count, indent, indent_token);
        }
        else {
            union__pretty_print(var, field_ptr, field, variant_index, field.variants, field.variant_count, indent, indent_token);
        }
        break;
    }
    default:
        UNREACHABLE();
        break;
    }
}
