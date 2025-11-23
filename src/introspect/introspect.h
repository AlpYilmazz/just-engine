#pragma once

#include "core.h"

#ifdef PRE_INTROSPECT_PASS
    #define introspect _introspect__just_to_make_sure_no_token_overlap__
    #define introspect_with(...) _introspect_with__just_to_make_sure_no_token_overlap__(__VA_ARGS__)
    #define alias(alias_type) _alias__just_to_make_sure_no_token_overlap__(alias_type)
    #define union_header(...) _union_header__just_to_make_sure_no_token_overlap__(__VA_ARGS__)
    #define mode_discriminated_union(discriminant_field) _mode_discriminated_union__just_to_make_sure_no_token_overlap__(discriminant_field)
    #define mode_cstr(...) _mode_cstr__just_to_make_sure_no_token_overlap__(__VA_ARGS__)
    #define mode_dynarray(count_field) _mode_dynarray__just_to_make_sure_no_token_overlap__(count_field)
    #define mode_string(count_field) _mode_string__just_to_make_sure_no_token_overlap__(count_field)
    #define mode_function_ptr(...) _mode_function_ptr__just_to_make_sure_no_token_overlap__(__VA_ARGS__)
#else
    #define introspect 
    #define introspect_with(...) 
    #define alias(alias_type) 
    #define union_header(...) 
    #define mode_discriminated_union(...) 
    #define mode_cstr(...) 
    #define mode_dynarray(count_field) 
    #define mode_string(count_field) 
    #define mode_function_ptr(...) 

    #define _introspect__just_to_make_sure_no_token_overlap__ 
    #define _introspect_with__just_to_make_sure_no_token_overlap__(...) 
    #define _alias__just_to_make_sure_no_token_overlap__(alias_type) 
    #define _union_header__just_to_make_sure_no_token_overlap__(...) 
    #define _mode_discriminated_union__just_to_make_sure_no_token_overlap__(discriminant_field) 
    #define _mode_cstr__just_to_make_sure_no_token_overlap__(...) 
    #define _mode_dynarray__just_to_make_sure_no_token_overlap__(count_field) 
    #define _mode_string__just_to_make_sure_no_token_overlap__(count_field) 
    #define _mode_function_ptr__just_to_make_sure_no_token_overlap__(...) 
#endif

// TODO
/**
 * TODO:
 * - handle enum types
 * - handle void type (ofc void*, not void)
 * - handle multi layer pointers (i.e. uint32***)
 * - handle multi dimension arrays (i.e. arr[3][5][6])
 * - handle type aliasing with `alias()`
 * - function pointer `function_ptr()`
 */

typedef enum {
    TYPE_void, // TODO
    TYPE_char,
    TYPE_byte,
    TYPE_bool,
    TYPE_uint8,
    TYPE_uint16,
    TYPE_uint32,
    TYPE_uint64,
    TYPE_int8,
    TYPE_int16,
    TYPE_int32,
    TYPE_int64,
    TYPE_usize,
    TYPE_float32,
    TYPE_float64,
    TYPE_struct,
    TYPE_union,
} Type;

typedef struct FieldInfo {
    Type type;
    char* type_str;
    char* name;
    void* ptr;
    // --
    bool is_ptr;
    uint32 ptr_depth;
    // --
    bool is_array;
    usize count; // total array length
    usize array_dim;
    usize array_dim_counts[10]; // max 10 dimensions
    // --
    bool is_cstr;
    // --
    bool is_dynarray;
    bool is_string;
    void* count_ptr;
    // --
    usize struct_size;
    uint32 field_count;
    struct FieldInfo* fields;
    // --
    bool is_named_union;
    char* union_name;
    uint32 union_header_variant;
    // --
    bool is_discriminated_union;
    void* discriminant_ptr;
    // --
    usize union_size;
    uint32 variant_count;
    struct FieldInfo* variants;
    // --
} FieldInfo;

typedef struct {
    char* token;
    uint32 count;
} IndentToken;

#define DEFAULT_INDENT_TOKEN ((IndentToken){ .count = 1, .token = "\t" })
void print_indent(uint32 indent_count, IndentToken indent_token);

#define just_print(Type) Type##__print0
#define just_pretty_print(Type) Type##__pretty_print0
#define just_pretty_print_with(Type) Type##__pretty_print_with0
#define just_array_print(Type) Type##_array__print0
#define just_array_pretty_print(Type) Type##_array__pretty_print0
#define just_array_pretty_print_with(Type) Type##_array__pretty_print_with0

// -----

#define __DECLARE__print_functions__stdout(TYPE) \
    void TYPE##__print(TYPE var); \
    void TYPE##_ptr__print(TYPE* ptr); \
    void TYPE##_array__print(TYPE* arr, usize count); \
    void TYPE##_array__pretty_print(TYPE* arr, usize count, uint32 indent, IndentToken indent_token); \
    void TYPE##_dynarray__print(TYPE* arr, usize count); \
    void TYPE##_dynarray__pretty_print(TYPE* arr, usize count, uint32 indent, IndentToken indent_token); \
\
    typedef enum { TYPE##__VARIANT__DECLARE__print_functions__stdout = 0 } TYPE##__ENUM__DECLARE__print_functions__stdout

void ptr__print(void* var);

__DECLARE__print_functions__stdout(char);
__DECLARE__print_functions__stdout(byte);
__DECLARE__print_functions__stdout(bool);
__DECLARE__print_functions__stdout(uint8);
__DECLARE__print_functions__stdout(uint16);
__DECLARE__print_functions__stdout(uint32);
__DECLARE__print_functions__stdout(uint64);
__DECLARE__print_functions__stdout(int8);
__DECLARE__print_functions__stdout(int16);
__DECLARE__print_functions__stdout(int32);
__DECLARE__print_functions__stdout(int64);
__DECLARE__print_functions__stdout(float32);
__DECLARE__print_functions__stdout(float64);
__DECLARE__print_functions__stdout(usize);

void char_cstr__print(char* cstr);
void char_string__print(char* str, usize count);

void struct__print(void* var, FieldInfo* fields, uint32 field_count);
void struct__pretty_print(void* var, FieldInfo* fields, uint32 field_count, uint32 indent, IndentToken indent_token);
void struct_ptr__print(void** ptr, FieldInfo* fields, uint32 field_count);
void struct_ptr__pretty_print(void** ptr, FieldInfo* fields, uint32 field_count, uint32 indent, IndentToken indent_token);
void struct_array__print(void* arr, usize count, uint32 struct_size, FieldInfo* fields, uint32 field_count);
void struct_array__pretty_print(void* arr, usize count, uint32 struct_size, FieldInfo* fields, uint32 field_count, uint32 indent, IndentToken indent_token);
void struct_dynarray__print(void* arr, usize count, uint32 struct_size, FieldInfo* fields, uint32 field_count);
void struct_dynarray__pretty_print(void* arr, usize count, uint32 struct_size, FieldInfo* fields, uint32 field_count, uint32 indent, IndentToken indent_token);

void introspect_field_print(FieldInfo field, void* var);
void introspect_field_pretty_print(FieldInfo field, void* var, uint32 indent, IndentToken indent_token);


// -----

#define __IMPL_____generate_print_functions(TYPE) \
    static inline void TYPE##__print(TYPE* var) { \
        struct__print(var, TYPE##__fields, ARRAY_LENGTH(TYPE##__fields)); \
    } \
    static inline void TYPE##__pretty_print_with(TYPE* var, uint32 indent, IndentToken indent_token) { \
        struct__pretty_print(var, TYPE##__fields, ARRAY_LENGTH(TYPE##__fields), indent, indent_token); \
    } \
    static inline void TYPE##__pretty_print(TYPE* var, uint32 indent) { \
        TYPE##__pretty_print_with(var, 0, DEFAULT_INDENT_TOKEN); \
    } \
\
    static inline void TYPE##_array__print(TYPE* var, usize count) { \
        struct_array__print(var, count, sizeof(TYPE), TYPE##__fields, ARRAY_LENGTH(TYPE##__fields)); \
    } \
    static inline void TYPE##_array__pretty_print_with(TYPE* var, usize count, uint32 indent, IndentToken indent_token) { \
        struct_array__pretty_print(var, count, sizeof(TYPE), TYPE##__fields, ARRAY_LENGTH(TYPE##__fields), indent, indent_token); \
    } \
    static inline void TYPE##_array__pretty_print(TYPE* var, usize count, uint32 indent) { \
        TYPE##_array__pretty_print_with(var, count, 0, DEFAULT_INDENT_TOKEN); \
    } \
\
    static inline void TYPE##__print0(TYPE* var) { \
        TYPE##__print(var); \
        printf("\n"); \
    } \
    static inline void TYPE##__pretty_print0(TYPE* var) { \
        TYPE##__pretty_print(var, 0); \
        printf("\n"); \
    } \
    static inline void TYPE##__pretty_print_with0(TYPE* var, IndentToken indent_token) { \
        TYPE##__pretty_print_with(var, 0, indent_token); \
        printf("\n"); \
    } \
\
    static inline void TYPE##_array__print0(TYPE* var, usize count) { \
        TYPE##_array__print(var, count); \
        printf("\n"); \
    } \
    static inline void TYPE##_array__pretty_print0(TYPE* var, usize count) { \
        TYPE##_array__pretty_print(var, count, 0); \
        printf("\n"); \
    } \
    static inline void TYPE##_array__pretty_print_with0(TYPE* var, usize count, IndentToken indent_token) { \
        TYPE##_array__pretty_print_with(var, count, 0, indent_token); \
        printf("\n"); \
    } \
\
    typedef enum { TYPE##__VARIANT__IMPL_____generate_print_functions = 0 } TYPE##__IMPL_____generate_print_functions

// -----