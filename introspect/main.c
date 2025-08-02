#include "justengine.h"

// run preprocessor only with PRE_INTROSPECT_PASS defined
// run introspect generator
// run normal compilation without PRE_INTROSPECT_PASS defined

typedef struct {
    uint32* * field_name;
    bool bool_arr  [ 3];
    const char const * const_field      mode_dynarray(count: count);
    long long int lli                   alias(int64);
    unsigned long int a;
    // long unsigned long int b;
    short c;
} StructName;

introspect_with()
typedef struct {
    usize count;
    usize capacity;
    uint32* items   mode_dynarray(count: count);
} InnerTestStruct;

// introspect_with()
typedef struct {
    bool bool_field;
    uint32 uint_field;
    int32 int_field;
    int cint_field alias(int32);
    float32 float_field;
    uint32* ptr_field;
    uint32 arr_field[10];
    char* cstr_field mode_cstr();
    uint32* dynarray_field mode_dynarray(count: uint_field);
    // TestString string_field;
    InnerTestStruct struct_field;
    InnerTestStruct struct_arr_field[3];
} TestStruct;

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

// void assert_all_star(StringView sv) {
//     for (usize i = 0; i < sv.count; i++) {
//         if (sv.str[i] != '*') {
//             PANIC("Weird pointer definition\n");
//         }
//     }
// }

// FieldInfo parse_struct_field(StringView field_def) {
//     usize tokens_count = ARRAY_LENGTH(field_parse_tokens__static);
//     StringToken* field_parse_tokens = string_tokens_from_static(field_parse_tokens__static, tokens_count);
//     StringTokensIter tokens_iter = string_view_iter_tokens(field_def, field_parse_tokens, tokens_count);
//     StringTokenOut token;

//     FieldInfo field_info = {0};
//     FieldExtensions field_ext = {0};

//     FieldParseState state = FieldParse_Begin;
//     FieldExtensionType current_ext = FieldExtensionNone;
//     bool in_array_def = false;

//     while (next_token(&tokens_iter, &token)) {
//         printf("token: %d -> ", token.id);
//         print_string_view(token.token);
//         printf("\n");

//         switch (state) {
//         case FieldParse_Begin:
//             if (token.free_word) {
//                 field_info.type_str = cstr_nclone(token.token.str, token.token.count);
//                 state = FieldParse_AfterType;
//             }
//             switch (token.id) {
//             case Token_const:
//                 break;
//             case Token_unsigned_short:
//             case Token_unsigned_long:
//             case Token_unsigned_long_long:
//             case Token_unsigned_short_int:
//             case Token_unsigned_long_int:
//             case Token_unsigned_long_long_int:
//             case Token_short:
//             case Token_long:
//             case Token_long_long:
//             case Token_short_int:
//             case Token_long_int:
//             case Token_long_long_int:
//                 field_info.type_str = cstr_nclone(token.token.str, token.token.count);
//                 state = FieldParse_AfterType;
//                 break;
//             }
//             break;
//         case FieldParse_AfterType:
//             if (token.free_word) {
//                 field_info.name = cstr_nclone(token.token.str, token.token.count);
//                 state = FieldParse_AfterName;
//             }
//             switch (token.id) {
//             case Token_star:
//                 field_info.is_ptr = true;
//                 field_info.ptr_depth++;
//                 break;
//             }
//             break;
//         case FieldParse_AfterName:
//             if (in_array_def) {
//                 field_info.array_dim++;
//                 if (field_info.count == 0){
//                     field_info.count = 1;
//                 }
//                 uint64 dim;
//                 bool success = sv_parse_uint64(token.token, &dim);
//                 field_info.count *= dim;
//                 field_info.array_dim_counts[field_info.array_dim] = dim;

//                 in_array_def = false;
//             }
//             switch (token.id) {
//             case Token_sq_paren_open:
//                 in_array_def = true;
//                 break;
//             case Token_sq_paren_close:
//                 in_array_def = false;
//                 break;
//             case Token_semicolon:
//                 state = FieldParse_End;
//                 break;
//             // --
//             case Token_introspect_extension_alias:
//                 field_ext.ext_alias = true;
//                 expect_token(&tokens_iter, Token_paren_open);
//                 if (next_token(&tokens_iter, &token)) {
//                     if (token.free_word) {
//                         field_ext.type_alias = string_from_view(token.token).cstr;
//                     }
//                     else {
//                         PANIC("Extension syntax error.\n");
//                     }
//                 }
//                 else {
//                     PANIC("Extension syntax error.\n");
//                 }
//                 expect_token(&tokens_iter, Token_paren_close);
//                 break;
//             case Token_introspect_extension_mode_cstr:
//                 field_ext.ext_mode_cstr = true;
//                 expect_token(&tokens_iter, Token_paren_open);
//                 expect_token(&tokens_iter, Token_paren_close);
//                 break;
//             case Token_introspect_extension_mode_dynarray:
//                 field_ext.ext_mode_dynarray = true;
//                 expect_token(&tokens_iter, Token_paren_open);
//                 expect_token(&tokens_iter, Token_introspect_extension_key_count);
//                 expect_token(&tokens_iter, Token_colon);
//                 if (next_token(&tokens_iter, &token)) {
//                     field_ext.dynarray_count_field = string_from_view(token.token).cstr;
//                 }
//                 else {
//                     PANIC("Extension syntax error.\n");
//                 }
//                 expect_token(&tokens_iter, Token_paren_close);
//                 break;
//             case Token_introspect_extension_mode_string:
//                 current_ext = FieldExtension_mode_string;
//                 break;
//             }
//             break;
//         case FieldParse_End:
//             if (current_ext == FieldExtensionNone) {
//                 switch (token.id) {
//                 case Token_introspect_extension_alias:
//                     current_ext = FieldExtension_alias;
//                     break;
//                 case Token_introspect_extension_mode_cstr:
//                     current_ext = FieldExtension_mode_cstr;
//                     break;
//                 case Token_introspect_extension_mode_dynarray:
//                     current_ext = FieldExtension_mode_dynarray;
//                     break;
//                 case Token_introspect_extension_mode_string:
//                     current_ext = FieldExtension_mode_string;
//                     break;
//                 }
//                 if (!next_token(&tokens_iter, &token)) {
//                     PANIC("Extension syntax error.\n");
//                 }
//                 if (token.id != Token_paren_open) {
//                     PANIC("Extension syntax error.\n");
//                 }
//             }
//             else {
//                 switch (current_ext) {
//                 case FieldExtension_alias:
//                     if (!next_token(&tokens_iter, &token)) {
//                         PANIC("Extension syntax error.\n");
//                     }
//                     // field_ext.
//                     break;
//                 case FieldExtension_mode_cstr:
//                     break;
//                 case FieldExtension_mode_dynarray:
//                     break;
//                 case FieldExtension_mode_string:
//                     break;
//                 }
//             }
//             break;
//         }
//     }
//     return field_info;
// }

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

typedef struct {
    char* name;
    Type type;
} TypeNameMapping;

Type type_of_field(FieldInfoExt* field) {
    char* type_name_cs = field->field_info.type_str;
    if (field->field_ext.ext_alias) {
        type_name_cs = field->field_ext.type_alias;
    }
    String type_name = string_from_cstr(type_name_cs);

    // const char* type_map[][2] = {
    //     { "void",       "TYPE_void"},
    //     { "char",       "TYPE_char"},
    //     { "byte",       "TYPE_byte"},
    //     { "bool",       "TYPE_bool"},
    //     { "_Bool",      "TYPE_bool"},
    //     { "uint8",      "TYPE_uint8"},
    //     { "uint16",     "TYPE_uint16"},
    //     { "uint32",     "TYPE_uint32"},
    //     { "uint64",     "TYPE_uint64"},
    //     { "int8",       "TYPE_int8"},
    //     { "int16",      "TYPE_int16"},
    //     { "int32",      "TYPE_int32"},
    //     { "int64",      "TYPE_int64"},
    //     { "usize",      "TYPE_usize"},
    //     { "float32",    "TYPE_float32"},
    //     { "float64",    "TYPE_float64"},
    //     // TYPE_struct
    // };
    TypeNameMapping type_map[] = {
        { .name = "void",       .type = TYPE_void},
        { .name = "char",       .type = TYPE_char},
        { .name = "byte",       .type = TYPE_byte},
        { .name = "bool",       .type = TYPE_bool},
        { .name = "_Bool",      .type = TYPE_bool},
        { .name = "uint8",      .type = TYPE_uint8},
        { .name = "uint16",     .type = TYPE_uint16},
        { .name = "uint32",     .type = TYPE_uint32},
        { .name = "uint64",     .type = TYPE_uint64},
        { .name = "int8",       .type = TYPE_int8},
        { .name = "int16",      .type = TYPE_int16},
        { .name = "int32",      .type = TYPE_int32},
        { .name = "int64",      .type = TYPE_int64},
        { .name = "usize",      .type = TYPE_usize},
        { .name = "float32",    .type = TYPE_float32},
        { .name = "float64",    .type = TYPE_float64},
        // TYPE_struct
    };

    for (uint32 i = 0; i < ARRAY_LENGTH(type_map); i++) {
        if (scs_equals(type_name, type_map[i].name)) {
            return type_map[i].type;
        }
    }
    return TYPE_struct;
}

char* type_to_type_str(Type type) {
    switch (type) {
    case TYPE_void:
        return "TYPE_void";
    case TYPE_char:
        return "TYPE_char";
    case TYPE_byte:
        return "TYPE_byte";
    case TYPE_bool:
        return "TYPE_bool";
    case TYPE_uint8:
        return "TYPE_uint8";
    case TYPE_uint16:
        return "TYPE_uint16";
    case TYPE_uint32:
        return "TYPE_uint32";
    case TYPE_uint64:
        return "TYPE_uint64";
    case TYPE_int8:
        return "TYPE_int8";
    case TYPE_int16:
        return "TYPE_int16";
    case TYPE_int32:
        return "TYPE_int32";
    case TYPE_int64:
        return "TYPE_int64";
    case TYPE_usize:
        return "TYPE_usize";
    case TYPE_float32:
        return "TYPE_float32";
    case TYPE_float64:
        return "TYPE_float64";
    case TYPE_struct:
        return "TYPE_struct";
    }
    PANIC("Unknown Type\n");
    return "";
}

void write_introspect(StringBuilder* GEN, StructInfo* struct_info) {
    string_builder_append_format(GEN, "FieldInfo %s__fields[] = {\n", struct_info->type_name.cstr);

    for (usize i = 0; i < struct_info->count; i++) {
        FieldInfoExt* field = &struct_info->fields[i];
        Type field_type_enum = type_of_field(field);
        char* field_type = type_to_type_str(field_type_enum);

        string_builder_append_format(GEN,
            "\t{\n\t\t.type = %s, .name = \"%s\", .ptr = &(((%s*)(0))->%s),\n",
            field_type,
            field->field_info.name,
            struct_info->type_name.cstr,
            field->field_info.name
        );

        if (field->field_info.is_ptr) {
            string_builder_append_format(GEN,
                "\t\t.is_ptr = true, .ptr_depth = %u,\n",
                field->field_info.ptr_depth
            );
        }

        if (field->field_info.is_array) {
            JUST_DEV_MARK();
            JUST_LOG_INFO("%llu\n", field->field_info.array_dim);
            string_builder_append_format(GEN,
                "\t\t.is_array = true, .count = %llu, .array_dim = %llu, .array_dim_counts = {",
                field->field_info.count,
                field->field_info.array_dim
            );
            for (usize dim_i = 0; dim_i < field->field_info.array_dim; dim_i++) {
                string_builder_append_format(GEN,
                    "%llu%s",
                    field->field_info.array_dim_counts[dim_i],
                    (dim_i != field->field_info.array_dim - 1) ? ", " : ""
                );
            }
            string_builder_append_cstr(GEN, "},\n");
        }

        if (field->field_ext.ext_mode_cstr) {
            string_builder_append_cstr(GEN, "\t\t.is_cstr = true,\n");
        }

        if (field->field_ext.ext_mode_dynarray) {
            string_builder_append_format(GEN,
                "\t\t.is_dynarray = true, .count_ptr = &(((%s*)(0))->%s),\n",
                struct_info->type_name.cstr,
                field->field_ext.dynarray_count_field
            );
        }

        if (field->field_ext.ext_mode_string) {
            string_builder_append_format(GEN,
                "\t\t.is_string = true, .count_ptr = &(((%s*)(0))->%s),\n",
                struct_info->type_name.cstr,
                field->field_ext.string_count_field
            );
        }

        if (field_type_enum == TYPE_struct) {
            string_builder_append_format(GEN,
                "\t\t.struct_size = sizeof(%s), .field_count = ARRAY_LENGTH(%s__fields), .fields = %s__fields,\n",
                field->field_info.type_str,
                field->field_info.type_str,
                field->field_info.type_str
            );
        }

        string_builder_append_cstr(GEN, "\t},\n");
    }
    string_builder_append_cstr(GEN, "};\n\n");
}

String gen_introspect_file() {
    StringBuilder GEN = string_builder_new();

    for (usize i = 0; i < INTROSPECTED_STRUCTS.count; i++) {
        StructInfo* struct_info = &INTROSPECTED_STRUCTS.structs[i];
        string_builder_append_format(&GEN, "FieldInfo %s__fields[%llu];\n", struct_info->type_name.cstr, struct_info->count);
    }
    string_builder_append_cstr(&GEN, "\n");

    for (usize i = 0; i < INTROSPECTED_STRUCTS.count; i++) {
        StructInfo* struct_info = &INTROSPECTED_STRUCTS.structs[i];
        write_introspect(&GEN, struct_info);
    }

    return build_string(&GEN);
}

void generate_introspect_for_struct(String struct_def) {
    // printf("\n");
    // print_string(struct_def);
    // printf("\n");
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
    // printf("\n");
    // JUST_LOG_INFO("%llu -> %llu [%llu/%llu]\n", curly_paren_open, curly_paren_close, struct_def.count, struct_def.count-1 - curly_paren_close - 1);
    
    StructInfo struct_info = {0};
    // StringView s1 = string_slice_view(struct_def, curly_paren_close + 1, struct_def.count-1 - curly_paren_close - 1);
    // StringView s2 = string_view_trim(s1);
    // String s = string_from_view(s2);
    // JUST_LOG_INFO("%p -> %p, %p\n", struct_def.str, s1.str, s1.str);
    // for (usize i = 0; i < s1.count; i++) {
    //     printf("%c", s1.str[i]);
    // }
    // printf("\n---\n");
    // print_string_view(s1);
    // printf("\n---\n");
    // print_string_view(s2);
    // printf("\n---\n");
    // print_string(s);
    // printf("\n---\n");
    struct_info.type_name = string_from_view(string_view_trim(string_slice_view(struct_def, curly_paren_close + 1, struct_def.count-1 - curly_paren_close - 1)));

    if (already_introspected(struct_info.type_name)) {
        JUST_LOG_INFO("Introspection already generated for type: %s.\n", struct_info.type_name.cstr);
        return;
    }

    StringView struct_fields = string_slice_view(struct_def, curly_paren_open + 1, curly_paren_close - curly_paren_open - 1);

    // print_string(struct_info.type_name);
    // print_string_view(struct_fields);

    usize tokens_count = ARRAY_LENGTH(field_parse_tokens__static);
    StringToken* field_parse_tokens = string_tokens_from_static(field_parse_tokens__static, tokens_count);
    StringTokensIter tokens_iter = string_view_iter_tokens(struct_fields, field_parse_tokens, tokens_count);
    StringTokenOut token;

    FieldParseState state = FieldParse_Begin;
    FieldInfo field_info = {0};
    FieldExtensions field_ext = {0};
    bool in_array_def = false;

    usize i = 0;
    while (next_token(&tokens_iter, &token)) {
        printf("[%llu] %d: \"", i++, token.id);
        // for (usize c = 0; c < token.token.count; c++) {
        //     printf("%c", token.token.str[c]);
        // }
        print_string_view(token.token);
        printf("\"\n");
        switch (state) {
        case FieldParse_Begin:
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
                default:
                    field_info.type_str = cstr_nclone(token.token.str, token.token.count);
                    state = FieldParse_AfterType;
                    break;
                }
            break;
        case FieldParse_AfterType:
            switch (token.id) {
            case Token_star:
                field_info.is_ptr = true;
                field_info.ptr_depth++;
                break;
            default:
                field_info.name = cstr_nclone(token.token.str, token.token.count);
                state = FieldParse_AfterName;
                break;
            }
            break;
        case FieldParse_AfterName:
            if (in_array_def) {
                if (field_info.count == 0){
                    field_info.count = 1;
                }
                uint64 dim;
                bool success = sv_parse_uint64(token.token, &dim);
                JUST_LOG_DEBUG("dim: %lld\n", dim);
                field_info.count *= dim;
                field_info.array_dim_counts[field_info.array_dim++] = dim;

                in_array_def = false;
            }
            switch (token.id) {
            case Token_sq_paren_open:
                in_array_def = true;
                field_info.is_array = true;
                break;
            case Token_sq_paren_close:
                in_array_def = false;
                break;
            case Token_semicolon:
                printf("%s %s\n", field_info.type_str, field_info.name);
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

    dynarray_push_back_custom(INTROSPECTED_STRUCTS, .structs, struct_info);

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
        JUST_LOG_ERROR("Error opening file\n");
        return false;
    }

    String cmd_introspect = string_from_cstr("_introspect");
    string_append_cstr(&cmd_introspect, "_with()");

    uint32 i = 0;
    bool gen_introspect = false;
    
    String struct_def_str = string_with_capacity(100);
    ParseStructDefState state = STATE_STRUCT_BEGIN;

    char ch;
    while ((ch = fgetc(file)) != EOF) {
        if (gen_introspect) {
            // printf("%c", ch);
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
                    // printf("\n---\n");
                    // print_string(struct_def_str);
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
        // string_from_cstr("introspect/main.c"),
        // string_from_cstr("introspect/main.c"),
    };
    #define FILE_COUNT ARRAY_LENGTH(filenames)

    String int_filenames[FILE_COUNT] = {
        string_from_cstr("introspect/main.c.int"),
        // string_from_cstr("introspect/main.c.int"),
        // string_from_cstr("introspect/main.c.int"),
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
    JUST_DEV_MARK();

    bool success = true;
    for (usize i = 0; i < FILE_COUNT; i++) {
        int32 exit_status;
        JUST_DEV_MARK();
        usize wait_result = _cwait(&exit_status, process_ids[i], _WAIT_CHILD);
        JUST_DEV_MARK();
        if (wait_result == -1) {
            PANIC("Failed to wait process\n");
        }
        if (exit_status != 0) {
            PANIC("Proces exited with non-zero status\n");
        }
        JUST_DEV_MARK();

        String int_filename = int_filenames[i];
        if (success) {
            JUST_DEV_MARK();
            success &= generate_introspect(int_filename);
        }
        JUST_DEV_MARK();
    }

    JUST_DEV_MARK();

    if (!success) {
        PANIC("Introspect failed\n");
    }

    JUST_DEV_MARK();

    String int_file_content = gen_introspect_file();
    printf("\n-----------------\n");
    print_string(int_file_content);
    printf("\n-----------------\n");
    FILE* file = fopen("introspect/gen.c", "w+");
    if (file == NULL) {
        PANIC("Error opening file\n");
    }
    fputs(int_file_content.cstr, file);
    fclose(file);

    return 0;

    // String fields[] = {
    //     string_from_cstr("uint32* * field_name;"),
    //     string_from_cstr("bool bool_arr  [ 3];"),
    //     string_from_cstr("const char const * const_field; alias(bool) mode(dynarray, count: count)"),
    //     string_from_cstr("long long int lli; alias(int64)"),
    // };

    // for (uint32 i = 0; i < ARRAY_LENGTH(fields); i++) {
    //     printf("-----\n");
    //     parse_struct_field(string_as_view(fields[i]));
    //     printf("-----\n");
    // }
    
    // return 0;
}