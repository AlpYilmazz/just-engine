
#include "justcstd.h"
#include "memory/memory.h"

#include "memory/juststring.h"

// cstr

usize cstr_length(const char* cstr) {
    usize i = 0;
    while (cstr[i] != '\0') i++;
    return i;
}

char* cstr_nclone(const char* cstr, usize count) {
    char* cstr_clone = std_malloc(count + 1);
    std_memcpy(cstr_clone, cstr, count);
    cstr_clone[count] = '\0';
    return cstr_clone;
}

char* cstr_clone(const char* cstr) {
    return cstr_nclone(cstr, cstr_length(cstr));
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
    std_free(string.str);
}

char* string_as_cstr(String string) {
    return string.str;
}

void string_nappend_cstr(String* string, char* cstr, usize count) {
    dynarray_reserve_custom(*string, .str, count + 1);
    std_memcpy(string->str + string->count, cstr, count);
    string->count += count;
    string->str[string->count] = '\0';
}

void string_append_cstr(String* string, char* cstr) {
    string_nappend_cstr(string, cstr, cstr_length(cstr));
}

StringView string_view(String string, usize start, usize count) {
    return (StringView) {
        .count = count,
        .str = string.str + start,
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

void string_builder_nappend(StringBuilder* builder, char* cstr, usize count) {
    struct StringBuilderNode* node = std_malloc(sizeof(StringBuilderNode));
    node->next = NULL;
    node->auto_free = false;
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

void string_builder_nappend_free(StringBuilder* builder, char* cstr, usize count) {
    string_builder_nappend(builder, cstr, count);
    builder->tail->auto_free = true;
}

void string_builder_append(StringBuilder* builder, char* cstr) {
    string_builder_nappend(builder, cstr, cstr_length(cstr));
}

void string_builder_append_free(StringBuilder* builder, char* cstr) {
    string_builder_nappend_free(builder, cstr, cstr_length(cstr));
}

void string_builder_append_string(StringBuilder* builder, String string) {
    string_builder_nappend(builder, string.str, string.count);
}

void string_builder_append_string_free(StringBuilder* builder, String string) {
    string_builder_nappend_free(builder, string.str, string.count);
}

String build_string(StringBuilder* builder) {
    usize total_count = builder->total_count;

    char* str = std_malloc(total_count + 1);
    str[total_count] = '\0';

    char* str_cursor = str;
    StringBuilderNode* node = builder->head;
    while(node != NULL) {
        std_memcpy(str_cursor, node->str, node->count);
        if (node->auto_free) {
            std_free(node->str);
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

