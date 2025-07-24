
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

String string_with_capacity(usize capacity) {
    String s = string_new();
    dynarray_reserve_custom(s, .str, capacity);
    return s;
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

bool string_equals(String s1, String s2) {
    if (s1.count != s2.count) {
        return false;
    }

    for (uint32 i = 0; i < s1.count; i++) {
        if (s1.str[i] != s2.str[i]) {
            return false;
        }
    }
    return true;
}

static inline void string_nappend_cstr_cap_unchecked(String* string, char* cstr, usize count) {
    std_memcpy(string->str + string->count, cstr, count);
    string->count += count;
    string->str[string->count] = '\0';
}

void string_nappend_cstr(String* string, char* cstr, usize count) {
    dynarray_reserve_custom(*string, .str, count + 1);
    string_nappend_cstr_cap_unchecked(string, cstr, count);
}

void string_append_cstr(String* string, char* cstr) {
    string_nappend_cstr(string, cstr, cstr_length(cstr));
}

String new_string_merged(String s1, String s2) {
    String s = string_with_capacity(s1.count + s2.count + 1);
    string_nappend_cstr_cap_unchecked(&s, s1.str, s1.count);
    string_nappend_cstr_cap_unchecked(&s, s2.str, s2.count);
    return s;
}

StringView string_slice_view(String string, usize start, usize count) {
    return (StringView) {
        .count = count,
        .str = string.str + start,
    };
}

StringViewPair string_split_at(String string, usize index) {
    return (StringViewPair) {
        .first = string_slice_view(string, 0, index),
        .second = string_slice_view(string, index, string.count - index),
    };
}

// StringBuilder

StringBuilder string_builder_new() {
    return (StringBuilder) {0};
}

String build_string(StringBuilder* builder) {
    usize total_count = builder->total_count;

    char* str = std_malloc(total_count + 1);
    str[total_count] = '\0';

    char* str_cursor = str;
    StringBuilderNode* node = builder->head;
    StringBuilderNode* next;
    while(node != NULL) {
        std_memcpy(str_cursor, node->str, node->count);
        if (node->auto_free) {
            std_free(node->str);
        }
        str_cursor += node->count;
        next = node->next;
        std_free(node);
        node = next;
    }

    *builder = string_builder_new();

    return (String) {
        .count = total_count,
        .capacity = total_count,
        .str = str,
    };
}

void string_builder_nappend_cstr(StringBuilder* builder, char* cstr, usize count) {
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

void string_builder_nappend_cstr_owned(StringBuilder* builder, char* cstr, usize count) {
    string_builder_nappend_cstr(builder, cstr, count);
    builder->tail->auto_free = true;
}

void string_builder_append_cstr(StringBuilder* builder, char* cstr) {
    string_builder_nappend_cstr(builder, cstr, cstr_length(cstr));
}

void string_builder_append_cstr_owned(StringBuilder* builder, char* cstr) {
    string_builder_nappend_cstr_owned(builder, cstr, cstr_length(cstr));
}

void string_builder_append_string(StringBuilder* builder, String string) {
    string_builder_nappend_cstr(builder, string.str, string.count);
}

void string_builder_append_string_owned(StringBuilder* builder, String string) {
    string_builder_nappend_cstr_owned(builder, string.str, string.count);
}

