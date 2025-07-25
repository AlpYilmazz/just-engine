#include "justengine.h"

#define mode(...) 
#define ext(...)

typedef struct {
    uint32* * field_name;
    bool bool_arr  [ 3];
    const char const * const_field; alias(bool) mode(dynarray, count: count)
    long long int lli; alias(int64)
} StructName;

typedef enum {
    Token_const = 0,
    Token_star,
    Token_semicolon,
    Token_sq_paren_open,
    Token_sq_paren_close,
    Token_long_long_int,
} FieldParseTokens;

typedef enum {
    FieldParse_Begin,
    FieldParse_AfterType,
    FieldParse_CountingPtr,
    FieldParse_AfterName,
    FieldParse_End,
} FieldParseState;

typedef struct {
    // --
    char* _alias;
    // --
    bool mode_cstr;
    // --
    bool mode_dynarray;
    char* dynarray_count_field;
    // --
    bool mode_string;
    char* string_count_field;
    // --
} FieldExtensions;

void assert_all_star(StringView sv) {
    for (usize i = 0; i < sv.count; i++) {
        if (sv.str[i] != '*') {
            PANIC("Weird pointer definition\n");
        }
    }
}

FieldInfo parse_struct_field(StringView field_def) {
    StringToken field_parse_tokens[] = {
        (StringToken) {
            .id = Token_const,
            .token = string_from_cstr("const"),
        },
        (StringToken) {
            .id = Token_star,
            .token = string_from_cstr("*"),
        },
        (StringToken) {
            .id = Token_semicolon,
            .token = string_from_cstr(";"),
        },
        (StringToken) {
            .id = Token_sq_paren_open,
            .token = string_from_cstr("["),
        },
        (StringToken) {
            .id = Token_sq_paren_close,
            .token = string_from_cstr("]"),
        },
        (StringToken) {
            .id = Token_long_long_int,
            .token = string_from_cstr("long long int"),
        },
    };
    StringTokenIter tokens_iters = string_view_iter_tokens(field_def, field_parse_tokens, ARRAY_LENGTH(field_parse_tokens));
    StringTokenOut token;

    FieldInfo field_info = {0};
    FieldExtensions field_ext = {0};
    FieldParseState state = FieldParse_Begin;
    uint32 count = 0;

    while (next_token(&tokens_iters, &token)) {
        printf("token: %d -> ", token.id);
        print_string_view(token.token);
        printf("\n");
    }
    return field_info;

    // bool part_exists[6] = {0};
    // StringView parts[6] = {0}; // (const)? (type) (const)? (*)* (name) (array_def)?
    
    // StringWordsIter words_iter = string_view_iter_words(field_def);
    // StringView word;

    // FieldInfo field_info = {0};
    // FieldExtensions field_ext = {0};
    // FieldParseState state = FieldParse_Begin;
    // uint32 count = 0;
    // while (next_word(&words_iter, &word)) {
    //     switch (state) {
    //     case FieldParse_Begin:
    //         if (svcs_equals(word, "const")) {
    //             if (count == 1) {
    //                 PANIC("Multi const before type\n");
    //             }
    //             count = 1;
    //         }
    //         else {
    //             field_info.type_str = cstr_nclone(word.str, word.count);
    //             state = FieldParse_AfterType;
    //             count = 0;
    //         }
    //         break;
    //     case FieldParse_AfterType:
    //         if (svcs_equals(word, "const")) {
    //             if (count == 1) {
    //                 PANIC("Multi const before type\n");
    //             }
    //             count = 1;
    //         }
    //         else if (word.str[0] == '*') {
    //             assert_all_star(word);
    //             field_info.is_ptr = true;
    //             state = FieldParse_CountingPtr;
    //             count = word.count;
    //         }
    //         else {
    //             field_info.name = cstr_nclone(word.str, word.count);
    //             state = FieldParse_AfterName;
    //             count = 0;
    //         }
    //         break;
    //     case FieldParse_CountingPtr:
    //         if (word.str[0] == '*') {
    //             assert_all_star(word);
    //             count += word.count;
    //         }
    //         else {
    //             field_info.name = cstr_nclone(word.str, word.count);
    //             state = FieldParse_AfterName;
    //             count = 0;
    //         }
    //         break;
    //     case FieldParse_AfterName:
    //         if (word.str[0] == ';') {
    //             state = FieldParse_End;
    //         }
    //         else if (word.str[0] == '[') {
    //             field_info.is_array = true;

    //             StringView array_part = word;
    //             if (array_part.count >= 2 && array_part.str[array_part.count - 1] == ';') {
    //                 array_part = string_view_slice_view(array_part, 0, array_part.count - 1);
    //                 state = FieldParse_End;
    //             }

    //             StringVarDelimIter delim_iter = string_view_iter_delim_var(word);
    //             StringView dim_str;
    //             uint32 array_count = 1;
    //             uint32 dim_count = 0;
    //             while (true) {
    //                 if (!next_item_until_delim(&delim_iter, '[', &dim_str)) {
    //                     break;
    //                 }
    //                 if (!next_item_until_delim(&delim_iter, ']', &dim_str)) {
    //                     PANIC("No closing array bracket.\n");
    //                 }
    //                 dim_count++;
    //                 uint64 dim = sv_parse_int(dim_str);
    //                 array_count += dim;
    //                 field_info.array_dim_counts[dim_count] = dim;
    //             }

    //             field_info.count = array_count;
    //             field_info.array_dim = dim_count;
    //         }
    //         break;
    //     case FieldParse_End:
    //         break;
    //     }
    // }

    // uint32 i = 0;
    // while (next_word(&words_iter, &word)) {
    //     if (svcs_equals(word, "const")) {
    //         part_exists[i] = true;
    //         parts[i] = word;
    //     }
    //     else if () {

    //     }
    //     i++;
    // }
}

int main() {
uint32* * field_name;
    bool bool_arr  [ 3];
    const char const * const_field; alias(bool) mode(dynarray, count: count)
    long long int lli; alias(int64)

    String fields[] = {
        string_from_cstr("uint32* * field_name;"),
        string_from_cstr("bool bool_arr  [ 3];"),
        string_from_cstr("const char const * const_field; alias(bool) mode(dynarray, count: count)"),
        string_from_cstr("long long int lli; alias(int64)"),
    };

    for (uint32 i = 0; i < ARRAY_LENGTH(fields); i++) {
        printf("-----\n");
        parse_struct_field(string_as_view(fields[i]));
        printf("-----\n");
    }
}