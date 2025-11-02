#pragma once

#include "justcstd.h"
#include "core.h"
#include "memory/memory.h"

bool char_is_eof(char ch);
bool char_is_whitespace(char ch);

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

typedef struct {
    usize count;
    usize capacity;
    String* items;
} StringList;

typedef struct {
    usize count;
    usize capacity;
    StringView* items;
} StringViewList;

String string_new();
String string_with_capacity(usize capacity);
String string_from_cstr(const char* cstr);
String string_from_view(StringView string_view);
String clone_string(String string);
void clear_string(String* string);
void free_string(String string);

bool ss_equals(String s1, String s2);
bool scs_equals(String s, char* cs);
bool ssv_equals(String s, StringView sv);
bool svcs_equals(StringView sv, char* cs);

bool sv_parse_uint64(StringView sv, uint64* out);

#define string_view_use_as_cstr(string_view_in, cstr_use, CodeBlock) \
    do { \
        char string_view_use_as_cstr__temp_hold = (string_view_in).str[(string_view_in).count]; \
        (string_view_in).str[(string_view_in).count] = '\0'; \
        cstr_use = (string_view_in).str; \
        CodeBlock; \
        (string_view_in).str[(string_view_in).count] = string_view_use_as_cstr__temp_hold; \
    } while (0)

void string_push_char(String* string, char ch);

void string_nappend_cstr(String* string, char* cstr, usize count);
void string_append_cstr(String* string, char* cstr);
String new_string_merged(String s1, String s2);

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

StringView string_as_view(String string);
StringView string_slice_view(String string, usize start, usize count);
StringView string_view_slice_view(StringView string_view, usize start, usize count);
StringViewPair string_split_at(String string, usize index);
StringViewPair string_view_split_at(StringView string_view, usize index);
StringView string_view_trim(StringView string_view);

void print_string_view(StringView string_view);
void println_string_view(StringView string_view);
void print_string(String string);
void println_string(String string);

typedef struct {
    StringView cursor;
} StringWordsIter;

StringWordsIter string_view_iter_words(StringView string_view);
StringWordsIter string_iter_words(String string);
bool next_word(StringWordsIter* words_iter, StringView* word_out);

typedef struct {
    StringView cursor;
} StringVarDelimIter;

StringVarDelimIter string_view_iter_delim_var(StringView string_view);
StringVarDelimIter string_iter_delim_var(String string);
bool next_item_until_delim(StringVarDelimIter* delim_iter, char delim, StringView* item_out);

typedef struct {
    uint32 id;
    String token;
} StringToken;

typedef struct {
    uint32 id;
    const char* token;
} StaticStringToken;

StringToken* string_tokens_from_static(StaticStringToken* static_tokens, usize count);

typedef struct {
    int64 id;
    bool free_word;
    StringView token;
} StringTokenOut;

typedef struct {
    StringView cursor;
    usize token_count;
    StringToken* tokens;
} StringTokensIter;

StringTokensIter string_view_iter_tokens(StringView string_view, StringToken* tokens, usize token_count);
StringTokensIter string_iter_tokens(String string, StringToken* tokens, usize token_count);
void free_tokens_iter(StringTokensIter* tokens_iter);
bool next_token(StringTokensIter* tokens_iter, StringTokenOut* token_out);
#define expect_token(tokens_iter_ptr, expected_token_id) \
    do { \
        StringTokenOut expect_token__token; \
        bool expect_token__result = next_token((tokens_iter_ptr), &expect_token__token); \
        if (!expect_token__result) { \
            PANIC("Token expect failed, no token\n"); \
        } \
        if ((expected_token_id) != expect_token__token.id) { \
            PANIC("Token expect failed, expected: %d, result: %d\n", (expected_token_id), expect_token__token.id); \
        } \
    } while (0)

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

StringBuilder string_builder_new();
String build_string(StringBuilder* builder);
void string_builder_nappend_cstr(StringBuilder* builder, char* cstr, usize count);
void string_builder_nappend_cstr_owned(StringBuilder* builder, char* cstr, usize count);
void string_builder_append_cstr(StringBuilder* builder, char* cstr);
void string_builder_append_cstr_owned(StringBuilder* builder, char* cstr);
void string_builder_append_string(StringBuilder* builder, String string);
void string_builder_append_string_owned(StringBuilder* builder, String string);

#define string_builder_append_format(string_builder_ptr, format, ...) \
    do { \
        String string_builder_append_format__string = string_new(); \
        string_append_format(string_builder_append_format__string, format, __VA_ARGS__); \
        string_builder_append_string_owned(string_builder_ptr, string_builder_append_format__string); \
    } while(0)

#define string_builder_hinted_append_format(string_builder_ptr, count_hint, format, ...) \
    do { \
        String string_builder_append_format__string = string_new(); \
        string_hinted_append_format(string_builder_append_format__string, count_hint, format, __VA_ARGS__); \
        string_builder_append_string_owned(string_builder_ptr, string_builder_append_format__string); \
    } while(0)

// -