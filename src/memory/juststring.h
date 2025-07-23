#pragma once

#include "justcstd.h"
#include "core.h"

usize cstr_length(const char* cstr);
char* cstr_nclone(const char* cstr, usize count);
char* cstr_clone(const char* cstr);

#define cstr_alloc_format(cstr_out, format, ...) \
    do { \
        int32 cstr_alloc_format__count = std_snprintf(NULL, 0, format, __VA_ARGS__); \
        (cstr_out) = std_malloc(cstr_alloc_format__count + 1); \
        std_snprintf((cstr_out), cstr_alloc_format__count + 1, format, __VA_ARGS__); \
    } while (0)

#define cstr_hinted_alloc_format(cstr_out, count_hint, format, ...) \
    do { \
        usize assert__count_hint = count_hint; \
        (cstr_out) = std_malloc(count_hint + 1); \
        int32 cstr_hinted_alloc_format__count = std_snprintf((cstr_out), count_hint + 1, format, __VA_ARGS__); \
        if (cstr_hinted_alloc_format__count > count_hint) { \
            std_free((cstr_out)); \
            (cstr_out) = std_malloc(cstr_hinted_alloc_format__count + 1); \
            std_snprintf((cstr_out), cstr_hinted_alloc_format__count + 1, format, __VA_ARGS__); \
        } \
    } while (0)

typedef struct {
    usize count;
    usize capacity;
    union {
        char* str;
        char* cstr;
    };
} String;

typedef struct {
    usize count;
    char* str;
} StringView;

typedef struct {
    String first;
    String second;
} StringPair;

typedef struct {
    StringView first;
    StringView second;
} StringViewPair;

typedef struct StringBuilderNode {
    struct StringBuilderNode* next;
    bool auto_free;
    usize count;
    char* str;
} StringBuilderNode;

typedef struct {
    usize total_count;
    StringBuilderNode* head;
    StringBuilderNode* tail;
} StringBuilder;

String string_new();
String string_from_cstr(char* cstr);
void free_string(String string);

char* string_as_cstr(String string);

#define string_view_use_as_cstr(string_view_in, cstr_use, CodeBlock) \
    do { \
        char string_view_use_as_cstr__temp_hold = (string_view_in).str[(string_view_in).count]; \
        (string_view_in).str[(string_view_in).count] = '\0'; \
        cstr_use = (string_view_in).str; \
        CodeBlock; \
        (string_view_in).str[(string_view_in).count] = string_view_use_as_cstr__temp_hold; \
    } while (0)

void string_nappend_cstr(String* string, char* cstr, usize count);
void string_append_cstr(String* string, char* cstr);

#define string_append_format(string, format, ...) \
    do { \
        int32 string_append_format__count = std_snprintf(NULL, 0, format, __VA_ARGS__); \
        dynarray_reserve_custom((string), .str, string_append_format__count + 1); \
        std_snprintf((string).str + (string).count, string_append_format__count + 1, format, __VA_ARGS__); \
        (string).count += string_append_format__count; \
    } while (0)

#define string_hinted_append_format(string, count_hint, format, ...) \
    do { \
        usize assert__count_hint = count_hint; \
        dynarray_reserve_custom((string), .str, count_hint + 1); \
        int32 string_hinted_append_format__count = std_snprintf((string).str + (string).count, count_hint + 1, format, __VA_ARGS__); \
        if (string_hinted_append_format__count > count_hint) { \
            dynarray_reserve_custom((string), .str, string_hinted_append_format__count + 1); \
            std_snprintf((string).str + (string).count, string_hinted_append_format__count + 1, format, __VA_ARGS__); \
        } \
        (string).count += string_hinted_append_format__count; \
    } while (0)

StringView string_view(String string, usize start, usize count);
StringViewPair string_split_at(String string, usize index);

StringBuilder string_builder_new();
void string_builder_nappend(StringBuilder* builder, char* cstr, usize count);
void string_builder_nappend_free(StringBuilder* builder, char* cstr, usize count);
void string_builder_append(StringBuilder* builder, char* cstr);
void string_builder_append_free(StringBuilder* builder, char* cstr);
void string_builder_append_string(StringBuilder* builder, String string);
void string_builder_append_string_free(StringBuilder* builder, String string);
String build_string(StringBuilder* builder);