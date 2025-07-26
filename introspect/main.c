#include "justengine.h"

#ifdef PRE_INTROSPECT_PASS
    #define mode() _mode()
#else
    #define mode() 
    #define _mode() 
#endif

// run preprocessor only with PRE_INTROSPECT_PASS defined
// run introspect generator
// run normal compilation without PRE_INTROSPECT_PASS defined

typedef struct {
    uint32* * field_name;
    bool bool_arr  [ 3];
    const char const * const_field; alias(bool) mode_dynarray(count: count) mode()
    long long int lli; alias(int64)
    unsigned long int a;
    // long unsigned long int b;
    short c;
} StructName;

typedef enum {
    // --
    Token_const = 0,
    // --
    Token_unsigned_short,
    Token_unsigned_long,
    Token_unsigned_long_long,
    // --
    Token_unsigned_short_int,
    Token_unsigned_long_int,
    Token_unsigned_long_long_int,
    // --
    Token_short,
    Token_long,
    Token_long_long,
    // --
    Token_short_int,
    Token_long_int,
    Token_long_long_int,
    // --
    Token_star,
    Token_comma,
    Token_colon,
    Token_semicolon,
    // --
    Token_paren_open,
    Token_paren_close,
    Token_sq_paren_open,
    Token_sq_paren_close,
    // --
    Token_introspect_extension_alias,
    Token_introspect_extension_mode_cstr,
    Token_introspect_extension_mode_dynarray,
    Token_introspect_extension_mode_string,
    // --
} FieldParseTokens;

StaticStringToken field_parse_tokens__static[] = {
    (StaticStringToken) { .id = Token_const, .token = "const" },

    (StaticStringToken) { .id = Token_unsigned_short, .token = "unsigned short" },
    (StaticStringToken) { .id = Token_unsigned_long, .token = "unsigned long" },
    (StaticStringToken) { .id = Token_unsigned_long_long, .token = "unsigned long long" },

    (StaticStringToken) { .id = Token_unsigned_short_int, .token = "unsigned short int" },
    (StaticStringToken) { .id = Token_unsigned_long_int, .token = "unsigned long int" },
    (StaticStringToken) { .id = Token_unsigned_long_long_int, .token = "unsigned long long int" },

    (StaticStringToken) { .id = Token_short, .token = "short" },
    (StaticStringToken) { .id = Token_long, .token = "long" },
    (StaticStringToken) { .id = Token_long_long, .token = "long long" },

    (StaticStringToken) { .id = Token_short_int, .token = "short int" },
    (StaticStringToken) { .id = Token_long_int, .token = "long int" },
    (StaticStringToken) { .id = Token_long_long_int, .token = "long long int" },

    (StaticStringToken) { .id = Token_star, .token = "*" },
    (StaticStringToken) { .id = Token_comma, .token = "," },
    (StaticStringToken) { .id = Token_colon, .token = ":" },
    (StaticStringToken) { .id = Token_semicolon, .token = ";" },
    (StaticStringToken) { .id = Token_paren_open, .token = "(" },
    (StaticStringToken) { .id = Token_paren_close, .token = ")" },
    (StaticStringToken) { .id = Token_sq_paren_open, .token = "[" },
    (StaticStringToken) { .id = Token_sq_paren_close, .token = "]" },

    (StaticStringToken) { .id = Token_introspect_extension_alias, .token = "_alias" },
    (StaticStringToken) { .id = Token_introspect_extension_mode_cstr, .token = "_cstr" },
    (StaticStringToken) { .id = Token_introspect_extension_mode_dynarray, .token = "_dynarray" },
    (StaticStringToken) { .id = Token_introspect_extension_mode_string, .token = "_string" },
};

typedef enum {
    FieldParse_Begin,
    FieldParse_AfterType,
    FieldParse_AfterName,
    FieldParse_End,
} FieldParseState;

typedef enum {
    FieldExtensionNone,
    FieldExtension_alias,
    FieldExtension_mode_cstr,
    FieldExtension_mode_dynarray,
    FieldExtension_mode_string,
} FieldExtensionType;

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
    usize tokens_count = ARRAY_LENGTH(field_parse_tokens__static);
    StringToken* field_parse_tokens = string_tokens_from_static(field_parse_tokens__static, tokens_count);
    StringTokenIter tokens_iters = string_view_iter_tokens(field_def, field_parse_tokens, tokens_count);
    StringTokenOut token;

    FieldInfo field_info = {0};
    FieldExtensions field_ext = {0};

    FieldParseState state = FieldParse_Begin;
    FieldExtensionType current_ext = FieldExtensionNone;
    bool in_array_def = false;

    while (next_token(&tokens_iters, &token)) {
        printf("token: %d -> ", token.id);
        print_string_view(token.token);
        printf("\n");

        switch (state) {
        case FieldParse_Begin:
            if (token.free_word) {
                field_info.type_str = cstr_nclone(token.token.str, token.token.count);
                state = FieldParse_AfterType;
            }
            switch (token.id) {
            case Token_const:
                break;
            case Token_unsigned_short:
            case Token_unsigned_long:
            case Token_unsigned_long_long:
            case Token_unsigned_short_int:
            case Token_unsigned_long_int:
            case Token_unsigned_long_long_int:
            case Token_short:
            case Token_long:
            case Token_long_long:
            case Token_short_int:
            case Token_long_int:
            case Token_long_long_int:
                field_info.type_str = cstr_nclone(token.token.str, token.token.count);
                state = FieldParse_AfterType;
                break;
            }
            break;
        case FieldParse_AfterType:
            if (token.free_word) {
                field_info.name = cstr_nclone(token.token.str, token.token.count);
                state = FieldParse_AfterName;
            }
            switch (token.id) {
            case Token_star:
                field_info.is_ptr = true;
                field_info.ref_depth++;
                break;
            }
            break;
        case FieldParse_AfterName:
            if (in_array_def) {
                field_info.array_dim++;
                if (field_info.count == 0){
                    field_info.count = 1;
                }
                uint64 dim = sv_parse_int(token.token);
                field_info.count *= dim;
                field_info.array_dim_counts[field_info.array_dim] = dim;
            }
            switch (token.id) {
            case Token_sq_paren_open:
                in_array_def = true;
                break;
            case Token_sq_paren_close:
                in_array_def = false;
                break;
            case Token_semicolon:
                state = FieldParse_End;
                break;
            }
            break;
        case FieldParse_End:
            if (current_ext == FieldExtensionNone) {
                switch (token.id) {
                case Token_introspect_extension_alias:
                    current_ext = FieldExtension_alias;
                    break;
                case Token_introspect_extension_mode_cstr:
                    current_ext = FieldExtension_mode_cstr;
                    break;
                case Token_introspect_extension_mode_dynarray:
                    current_ext = FieldExtension_mode_dynarray;
                    break;
                case Token_introspect_extension_mode_string:
                    current_ext = FieldExtension_mode_string;
                    break;
                }
                if (!next_token(&tokens_iters, &token)) {
                    PANIC("Extension syntax error.\n");
                }
                if (token.id != Token_paren_open) {
                    PANIC("Extension syntax error.\n");
                }
            }
            else {
                switch (current_ext) {
                case FieldExtension_alias:
                    if (!next_token(&tokens_iters, &token)) {
                        PANIC("Extension syntax error.\n");
                    }
                    // field_ext.
                    break;
                case FieldExtension_mode_cstr:
                    break;
                case FieldExtension_mode_dynarray:
                    break;
                case FieldExtension_mode_string:
                    break;
                }
            }
            break;
        }
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