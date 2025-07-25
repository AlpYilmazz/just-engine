
#include "justcstd.h"
#include "memory/memory.h"

#include "memory/juststring.h"

// char

bool char_is_eof(char ch) {
    return ch == '\0';
}

bool char_is_whitespace(char ch) {
    return ch == ' '
        || ch == '\t'
        || ch == '\r'
        || ch == '\n'
        || ch == '\v'
        || ch == '\f';
}

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

String string_from_cstr(const char* cstr) {
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

bool ss_equals(String s1, String s2) {
    if (s1.count != s2.count) {
        return false;
    }
    return std_memcmp(s1.str, s2.str, s1.count) == 0;
}
bool scs_equals(String s, char* cs) {
    usize cs_count = cstr_length(cs);
    if (s.count != cs_count) {
        return false;
    }
    return std_memcmp(s.str, cs, s.count) == 0;
}
bool ssv_equals(String s, StringView sv) {
    if (s.count != sv.count) {
        return false;
    }
    return std_memcmp(s.str, sv.str, s.count) == 0;
}
bool svcs_equals(StringView sv, char* cs) {
    usize cs_count = cstr_length(cs);
    if (sv.count != cs_count) {
        return false;
    }
    return std_memcmp(sv.str, cs, sv.count) == 0;
}

uint64 sv_parse_int(StringView sv) {
    uint64 num = 0;
    uint64 factor = 1;
    for (usize i = sv.count-1; i >= 0; i--) {
        uint64 digit = sv.str[i] - '0';
        num += digit * factor;
        factor *= 10;
    }
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

StringView string_as_view(String string) {
    return (StringView) {
        .count = string.count,
        .str = string.str,
    };
}

StringView string_slice_view(String string, usize start, usize count) {
    return (StringView) {
        .count = count,
        .str = string.str + start,
    };
}

StringView string_view_slice_view(StringView string_view, usize start, usize count) {
    return (StringView) {
        .count = count,
        .str = string_view.str + start,
    };
}

StringViewPair string_split_at(String string, usize index) {
    return (StringViewPair) {
        .first = string_slice_view(string, 0, index),
        .second = string_slice_view(string, index, string.count - index),
    };
}

StringViewPair string_view_split_at(StringView string_view, usize index) {
    return (StringViewPair) {
        .first = string_view_slice_view(string_view, 0, index),
        .second = string_view_slice_view(string_view, index, string_view.count - index),
    };
}

StringView string_view_trim(StringView string_view) {
    usize word_start_inc = 0;
    usize word_end_exc = 0;
    bool in_word = false;
    bool word_found = false;

    usize i = 0;
    for (i = 0; i < string_view.count; i++) {
        char ch = string_view.str[i];
        if (in_word) {
            if (char_is_whitespace(ch)) {
                word_end_exc = i;
                in_word = false;
                word_found = true;
                break;
            }
        }
        else { // if (!in_word) {
            if (!char_is_whitespace(ch)) {
                word_start_inc = i;
                in_word = true;
            }
        }
    }
    if (in_word) {
        word_end_exc = i;
        word_found = true;
    }

    // printf("trim: %d, %d, %d, %d\n", in_word, word_found, word_start_inc, word_end_exc);

    if (word_found) {
        return string_view_slice_view(string_view, word_start_inc, word_end_exc - word_start_inc);
    }
    return (StringView) {0};
}

void print_string_view(StringView string_view) {
    for (usize i = 0; i < string_view.count; i++) {
        printf("%c", string_view.str[i]);
    }
}

void print_string(String string) {
    print_string_view(string_as_view(string));
}

// String Iterators

StringWordsIter string_view_iter_words(StringView string_view) {
    return (StringWordsIter) {
        .cursor = string_view,
    };
}

StringWordsIter string_iter_words(String string) {
    return string_view_iter_words(string_as_view(string));
}

bool next_word(StringWordsIter* words_iter, StringView* word_out) {
    for (
        ;
        words_iter->cursor.count > 0 && char_is_whitespace(words_iter->cursor.str[0]);
        words_iter->cursor.count--, words_iter->cursor.str++
    );

    if (words_iter->cursor.count == 0) {
        return false;
    }

    usize i;
    for (
        i = 0;
        i < words_iter->cursor.count && !char_is_whitespace(words_iter->cursor.str[i]);
        i++
    );

    *word_out = string_view_slice_view(words_iter->cursor, 0, i);

    return true;
}

StringVarDelimIter string_view_iter_delim_var(StringView string_view) {
    return (StringVarDelimIter) {
        .cursor = string_view,
    };
}

StringVarDelimIter string_iter_delim_var(String string) {
    return string_view_iter_delim_var(string_as_view(string));
}

bool next_item_until_delim(StringVarDelimIter* delim_iter, char delim, StringView* item_out) {
    if (delim_iter->cursor.count == 0) {
        return false;
    }

    usize i;
    for (
        i = 0;
        i < delim_iter->cursor.count && delim_iter->cursor.str[i] != delim;
        i++
    );

    *item_out = string_view_slice_view(delim_iter->cursor, 0, i);

    return true;
}

StringToken* string_tokens_from_static(StaticStringToken* static_tokens, usize count) {
    StringToken* tokens = std_malloc(sizeof(StringToken) * count);
    for (usize i = 0; i < count; i++) {
        tokens[i] = (StringToken) {
            .id = static_tokens[i].id,
            .token = string_from_cstr(static_tokens[i].token),
        };
    }
    return tokens;
}

StringTokenIter string_view_iter_tokens(StringView string_view, StringToken* tokens, usize token_count) {
    return (StringTokenIter) {
        .cursor = string_view,
        .token_count = token_count,
        .tokens = tokens,
    };
}

StringTokenIter string_iter_tokens(String string, StringToken* tokens, usize token_count) {
    return string_view_iter_tokens(string_as_view(string), tokens, token_count);
}

bool next_token(StringTokenIter* tokens_iter, StringTokenOut* token_out) {
    StringView start = tokens_iter->cursor;

    usize word_start_inc = 0;
    usize word_end_exc = 0;
    bool in_word = false;
    bool word_found = false;
    
    while (tokens_iter->cursor.count > 0) {
        char ch = *tokens_iter->cursor.str;
        usize i = (usize)tokens_iter->cursor.str - (usize)start.str;
        if (in_word) {
            if (char_is_whitespace(ch)) {
                word_end_exc = i;
                in_word = false;
                *token_out = (StringTokenOut) {
                    .id = 0,
                    .free_word = true,
                    .token = string_view_slice_view(start, word_start_inc, word_end_exc - word_start_inc),
                };
                return true;
            }
        }
        else { // if (!in_word) {
            if (!char_is_whitespace(ch)) {
                word_start_inc = i;
                in_word = true;
            }
        }

        for (uint32 token_i = 0; token_i < tokens_iter->token_count; token_i++) {
            StringToken token = tokens_iter->tokens[token_i];
            usize token_len = token.token.count;
            if (token_len <= tokens_iter->cursor.count) {
                StringViewPair split = string_view_split_at(tokens_iter->cursor, token_len);
                // printf("check: ");
                // print_string(token.token);
                // printf(" - ");
                // print_string_view(split.first);
                // printf("\n");
                if (ssv_equals(token.token, split.first)) {
                    StringView prefix = string_view_slice_view(start, 0, start.count - tokens_iter->cursor.count);
                    StringView trimmed = string_view_trim(prefix);
                    // print_string(token.token);
                    // printf("\n");
                    // print_string_view(prefix);
                    // printf("\n");
                    // print_string_view(trimmed);
                    // printf("\n");
                    if (trimmed.count > 0) {
                        *token_out = (StringTokenOut) {
                            .id = 0,
                            .free_word = true,
                            .token = trimmed,
                        };
                        return true;
                    }

                    tokens_iter->cursor = split.second;
                    *token_out = (StringTokenOut) {
                        .id = token.id,
                        .free_word = false,
                        .token = string_as_view(token.token),
                    };
                    return true;
                }
            }
        }

        tokens_iter->cursor.count--;
        tokens_iter->cursor.str++;
    }

    usize i = (usize)tokens_iter->cursor.str - (usize)start.str;
    if (in_word) {
        word_end_exc = i;
        *token_out = (StringTokenOut) {
            .id = 0,
            .free_word = true,
            .token = string_view_slice_view(start, word_start_inc, word_end_exc - word_start_inc),
        };
        return true;
    }

    return false;
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

