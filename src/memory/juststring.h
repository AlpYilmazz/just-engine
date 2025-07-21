#pragma once

#include "core.h"

typedef struct {
    usize count;
    usize capacity;
    char* str;
} String;

typedef struct {
    usize count;
    char* str;
    char temp_hold;
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

usize cstr_length(char* cstr);
char* cstr_nclone(char* cstr, usize count);

char* string_as_cstr(String string);
char* string_view_as_cstr_acquire(StringView* string_view);
void string_view_as_cstr_release(StringView* string_view);

String string_new();
String string_from_cstr(char* cstr);
void free_string(String string);

void string_append_cstr(String* string_dst, char* cstr_src);
StringView string_view(String string, usize start, usize count);
StringViewPair string_split_at(String string, usize index);

StringBuilder string_builder_new();
void string_builder_append(StringBuilder* builder, char* cstr);
void string_builder_append_free(StringBuilder* builder, char* cstr);
String build_string(StringBuilder* builder);