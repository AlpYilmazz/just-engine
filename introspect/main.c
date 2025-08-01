#include "justengine.h"

#ifdef PRE_INTROSPECT_PASS
    #define mode(...) _mode(__VA_ARGS__)
#else
    #define mode(...) 
    #define _mode(...) 
#endif

// run preprocessor only with PRE_INTROSPECT_PASS defined
// run introspect generator
// run normal compilation without PRE_INTROSPECT_PASS defined

introspect()
typedef struct {
    uint32* * field_name;
    bool bool_arr  [ 3];
    const char const * const_field      alias(bool) mode_dynarray(count: count) mode(count: count);
    long long int lli                   alias(int64);
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
    Token_introspect_extension_key_count,
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
    (StaticStringToken) { .id = Token_introspect_extension_key_count, .token = "count" },
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
    bool ext_alias;
    char* type_alias;
    // --
    bool ext_mode_cstr;
    // --
    bool ext_mode_dynarray;
    char* dynarray_count_field;
    // --
    bool ext_mode_string;
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
    StringTokensIter tokens_iter = string_view_iter_tokens(field_def, field_parse_tokens, tokens_count);
    StringTokenOut token;

    FieldInfo field_info = {0};
    FieldExtensions field_ext = {0};

    FieldParseState state = FieldParse_Begin;
    FieldExtensionType current_ext = FieldExtensionNone;
    bool in_array_def = false;

    while (next_token(&tokens_iter, &token)) {
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
                uint64 dim;
                bool success = sv_parse_uint64(token.token, &dim);
                field_info.count *= dim;
                field_info.array_dim_counts[field_info.array_dim] = dim;

                in_array_def = false;
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
            // --
            case Token_introspect_extension_alias:
                field_ext.ext_alias = true;
                expect_token(&tokens_iter, Token_paren_open);
                if (next_token(&tokens_iter, &token)) {
                    if (token.free_word) {
                        field_ext.type_alias = string_from_view(token.token).cstr;
                    }
                    else {
                        PANIC("Extension syntax error.\n");
                    }
                }
                else {
                    PANIC("Extension syntax error.\n");
                }
                expect_token(&tokens_iter, Token_paren_close);
                break;
            case Token_introspect_extension_mode_cstr:
                field_ext.ext_mode_cstr = true;
                expect_token(&tokens_iter, Token_paren_open);
                expect_token(&tokens_iter, Token_paren_close);
                break;
            case Token_introspect_extension_mode_dynarray:
                field_ext.ext_mode_dynarray = true;
                expect_token(&tokens_iter, Token_paren_open);
                expect_token(&tokens_iter, Token_introspect_extension_key_count);
                expect_token(&tokens_iter, Token_colon);
                if (next_token(&tokens_iter, &token)) {
                    field_ext.dynarray_count_field = string_from_view(token.token).cstr;
                }
                else {
                    PANIC("Extension syntax error.\n");
                }
                expect_token(&tokens_iter, Token_paren_close);
                break;
            case Token_introspect_extension_mode_string:
                current_ext = FieldExtension_mode_string;
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
                if (!next_token(&tokens_iter, &token)) {
                    PANIC("Extension syntax error.\n");
                }
                if (token.id != Token_paren_open) {
                    PANIC("Extension syntax error.\n");
                }
            }
            else {
                switch (current_ext) {
                case FieldExtension_alias:
                    if (!next_token(&tokens_iter, &token)) {
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

#include <process.h>

typedef struct {
    FieldInfo field_info;
    FieldExtensions field_ext;
} FieldInfoExt;

typedef struct {
    String type_name;
    usize count;
    usize capacity;
    FieldInfoExt* fields;
} StructInfo;

typedef struct {
    usize count;
    usize capacity;
    StructInfo* structs;
} IntrospectedStructs;

IntrospectedStructs INTROSPECTED_STRUCTS = {0};

bool already_introspected(String type_name) {
    for (usize i = 0; i < INTROSPECTED_STRUCTS.count; i++) {
        if (ss_equals(INTROSPECTED_STRUCTS.structs[i].type_name, type_name)) {
            return true;
        }
    }
    return false;
}

void generate_introspect_for_struct(String struct_def) {
    usize curly_paren_open;
    usize curly_paren_close;
    for (usize i = 0; i < struct_def.count; i++) {
        if (struct_def.str[i] == '{') {
            curly_paren_open = i;
            break;
        }
    }
    for (int64 i = struct_def.count-1; i >= 0; i--) {
        if (struct_def.str[i] == '}') {
            curly_paren_close = i;
            break;
        }
    }

    if (curly_paren_open >= curly_paren_close) {
        PANIC("Syntax error: curly paren\n");
    }
    
    StructInfo struct_info = {0};
    struct_info.type_name = string_from_view(string_view_trim(string_slice_view(struct_def, curly_paren_close + 1, struct_def.count-1 - curly_paren_close - 1)));

    if (already_introspected(struct_info.type_name)) {
        JUST_LOG_INFO("Introspection already generated for type: %s.\n", struct_info.type_name.cstr);
        return;
    }

    StringView struct_fields = string_slice_view(struct_def, curly_paren_open + 1, curly_paren_close - curly_paren_open - 1);

    usize tokens_count = ARRAY_LENGTH(field_parse_tokens__static);
    StringToken* field_parse_tokens = string_tokens_from_static(field_parse_tokens__static, tokens_count);
    StringTokensIter tokens_iter = string_view_iter_tokens(struct_fields, field_parse_tokens, tokens_count);
    StringTokenOut token;

    FieldParseState state = FieldParse_Begin;
    FieldInfo field_info = {0};
    FieldExtensions field_ext = {0};
    bool in_array_def = false;

    while (next_token(&tokens_iter, &token)) {
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
                uint64 dim;
                bool success = sv_parse_uint64(token.token, &dim);
                field_info.count *= dim;
                field_info.array_dim_counts[field_info.array_dim] = dim;

                in_array_def = false;
            }
            switch (token.id) {
            case Token_sq_paren_open:
                in_array_def = true;
                break;
            case Token_sq_paren_close:
                in_array_def = false;
                break;
            case Token_semicolon:
                FieldInfoExt field_info_ext = {
                    .field_info = field_info,
                    .field_ext = field_ext,
                };
                dynarray_push_back_custom(struct_info, .fields, field_info_ext);
                
                state = FieldParse_Begin;
                field_info = (FieldInfo) {0};
                field_ext = (FieldExtensions) {0};
                in_array_def = false;

                break;
            // -- extensions --
            case Token_introspect_extension_alias:
                field_ext.ext_alias = true;
                expect_token(&tokens_iter, Token_paren_open);
                if (next_token(&tokens_iter, &token)) {
                    if (token.free_word) {
                        field_ext.type_alias = string_from_view(token.token).cstr;
                    }
                    else {
                        PANIC("Extension syntax error.\n");
                    }
                }
                else {
                    PANIC("Extension syntax error.\n");
                }
                expect_token(&tokens_iter, Token_paren_close);
                break;
            case Token_introspect_extension_mode_cstr:
                field_ext.ext_mode_cstr = true;
                expect_token(&tokens_iter, Token_paren_open);
                expect_token(&tokens_iter, Token_paren_close);
                break;
            case Token_introspect_extension_mode_dynarray:
                field_ext.ext_mode_dynarray = true;
                expect_token(&tokens_iter, Token_paren_open);
                expect_token(&tokens_iter, Token_introspect_extension_key_count);
                expect_token(&tokens_iter, Token_colon);
                if (next_token(&tokens_iter, &token)) {
                    field_ext.dynarray_count_field = string_from_view(token.token).cstr;
                }
                else {
                    PANIC("Extension syntax error.\n");
                }
                expect_token(&tokens_iter, Token_paren_close);
                break;
            case Token_introspect_extension_mode_string:
                field_ext.ext_mode_string = true;
                expect_token(&tokens_iter, Token_paren_open);
                expect_token(&tokens_iter, Token_introspect_extension_key_count);
                expect_token(&tokens_iter, Token_colon);
                if (next_token(&tokens_iter, &token)) {
                    field_ext.string_count_field = string_from_view(token.token).cstr;
                }
                else {
                    PANIC("Extension syntax error.\n");
                }
                expect_token(&tokens_iter, Token_paren_close);
                break;
            }
        }
    }

    free_tokens_iter(&tokens_iter);
}

typedef enum {
    STATE_STRUCT_BEGIN = 0,
    STATE_CURLY_PAREN_OPEN_RECEIVED,
    STATE_CURLY_PAREN_CLOSE_RECEIVED,
} ParseStructDefState;

bool generate_introspect(String int_filename) {
    FILE* file = fopen(int_filename.cstr, "r");
    if (file == NULL) {
        JUST_LOG_ERROR("Error opening file");
        return false;
    }

    String cmd_introspect = string_from_cstr("_introspect ");

    uint32 i = 0;
    bool gen_introspect = false;
    
    String struct_def_str = string_with_capacity(100);
    ParseStructDefState state = STATE_STRUCT_BEGIN;

    char ch;
    while ((ch = fgetc(file)) != EOF) {
        if (gen_introspect) {
            string_push_char(&struct_def_str, ch);
            switch (state) {
            case STATE_STRUCT_BEGIN:
                if (ch == '{') {
                    state = STATE_CURLY_PAREN_OPEN_RECEIVED;
                }
                break;
            case STATE_CURLY_PAREN_OPEN_RECEIVED:
                if (ch == '}') {
                    state = STATE_CURLY_PAREN_CLOSE_RECEIVED;
                }
                break;
            case STATE_CURLY_PAREN_CLOSE_RECEIVED:
                if (ch == ';') {
                    generate_introspect_for_struct(struct_def_str);
                    clear_string(&struct_def_str);
                    gen_introspect = false;
                    state = STATE_STRUCT_BEGIN;
                }
                break;
            }
        }
        else {
            if (ch == cmd_introspect.str[i]) {
                i++;
            }
            else {
                i = 0;
            }
            if (i == cmd_introspect.count) {
                i = 0;
                gen_introspect = true;
            }
        }
    }
}

int main() {
    String INTROSPECT_FILE_SUFFIX_EXT = string_from_cstr(".int");
    // new_string_merged(filename, INTROSPECT_FILE_SUFFIX_EXT);

    String filenames[] = {
        string_from_cstr("introspect/main.c"),
        string_from_cstr("introspect/main.c"),
        string_from_cstr("introspect/main.c"),
    };
    #define FILE_COUNT ARRAY_LENGTH(filenames)

    String int_filenames[FILE_COUNT] = {
        string_from_cstr("introspect/main.c.int"),
        string_from_cstr("introspect/main.c.int"),
        string_from_cstr("introspect/main.c.int"),
    };
    usize process_ids[FILE_COUNT] = {0};

    for (usize i = 0; i < FILE_COUNT; i++) {
        String filename = filenames[i];
        String int_filename = int_filenames[i];

        usize spawn_result = _spawnlp(
            _P_NOWAIT,
            "gcc",
            "gcc",
            "-Ijustengine/include",
            "-IC:/dev/vendor/openssl-3.5.0/include",
            "-DPRE_INTROSPECT_PASS",
            "-E",
            "-P",
            filename.cstr,
            "-o",
            int_filename.cstr,
            NULL
        );
        if (spawn_result == -1) {
            PANIC("Failed to create process\n");
        }

        process_ids[i] = spawn_result;
    }

    bool success = true;
    for (usize i = 0; i < FILE_COUNT; i++) {
        int32 exit_status;
        usize wait_result = _cwait(&exit_status, process_ids[i], _WAIT_CHILD);
        if (wait_result == -1) {
            PANIC("Failed to wait process\n");
        }
        if (exit_status != 0) {
            PANIC("Proces exited with non-zero status\n");
        }

        String int_filename = int_filenames[i];
        if (success) {
            success &= generate_introspect(int_filename);
        }
    }

    if (!success) {
        PANIC("Introspect failed\n");
    }

    return 0;

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
    
    return 0;
}