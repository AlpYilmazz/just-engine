
#include "memory/memory.h"

#include "memory/juststring.h"

// cstr

usize cstr_length(char* cstr) {
    usize i = 0;
    while (cstr[i] != '\0') i++;
    return i;
}

char* cstr_nclone(char* cstr, usize count) {
    char* cstr_clone = malloc(count + 1);
    memcpy(cstr_clone, cstr, count);
    cstr_clone[count] = '\0';
    return cstr_clone;
}

char* string_as_cstr(String string) {
    return string.str;
}

char* string_view_as_cstr_acquire(StringView* string_view) {
    string_view->temp_hold = string_view->str[string_view->count];
    string_view->str[string_view->count] = '\0';
    return string_view->str;
}

void string_view_as_cstr_release(StringView* string_view) {
    string_view->str[string_view->count] = string_view->temp_hold;
}

// String

String string_new() {
    return (String) {0};
}

String string_from_cstr(char* cstr) {
    usize count = cstr_length(cstr);
    return (String) {
        .count = count,
        .capacity = count,
        .str = cstr_nclone(cstr, count),
    };
}

void free_string(String string) {
    free(string.str);
}

void string_append_cstr(String* string_dst, char* cstr_src) {
    usize count = cstr_length(cstr_src);
    dynarray_reserve_custom(*string_dst, .str, count+1);
    memcpy(string_dst->str + string_dst->count, cstr_src, count);
    string_dst->count += count;
    string_dst->str[string_dst->count] = '\0';
}

StringView string_view(String string, usize start, usize count) {
    return (StringView) {
        .count = count,
        .str = string.str + start,
        .temp_hold = '\0',
    };
}

StringViewPair string_split_at(String string, usize index) {
    return (StringViewPair) {
        .first = string_view(string, 0, index),
        .second = string_view(string, index, string.count - index),
    };
}

// StringBuilder

StringBuilder string_builder_new() {
    return (StringBuilder) {0};
}

void string_builder_append(StringBuilder* builder, char* cstr) {
    usize count = cstr_length(cstr);

    struct StringBuilderNode* node = malloc(sizeof(StringBuilderNode));
    node->next = NULL;
    node->count = count;
    node->str = cstr;

    builder->total_count += count;
    if (builder->head != NULL) {
        builder->tail->next = node;
    }
    else {
        builder->head = node;
    }
    builder->tail = node;
}

void string_builder_append_free(StringBuilder* builder, char* cstr) {
    string_builder_append(builder, cstr);
    builder->tail->auto_free = true;
}

String build_string(StringBuilder* builder) {
    usize total_count = builder->total_count;

    char* str = malloc(total_count + 1);
    str[total_count] = '\0';

    char* str_cursor = str;
    StringBuilderNode* node = builder->head;
    while(node != NULL) {
        memcpy(str_cursor, node->str, node->count);
        if (node->auto_free) {
            free(node->str);
        }
        str_cursor += node->count;
        node = node->next;
    }

    *builder = string_builder_new();

    return (String) {
        .count = total_count,
        .capacity = total_count,
        .str = str,
    };
}

