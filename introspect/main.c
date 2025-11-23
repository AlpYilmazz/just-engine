#include <process.h>

#include "justengine.h"

// run preprocessor only with PRE_INTROSPECT_PASS defined
// run introspect generator
// run normal compilation without PRE_INTROSPECT_PASS defined

typedef enum {
    // --
    Token_typedef,
    Token_struct,
    Token_union,
    // --
    Token_const,
    // --
    Token_unsigned_char,
    Token_unsigned_int,
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
    Token_cr_paren_open,
    Token_cr_paren_close,
    // --
    Token__startblock__extension,
    Token_introspect_extension_alias,
    Token_introspect_extension_union_header,
    Token_introspect_extension_mode_discriminated_union,
    Token_introspect_extension_mode_cstr,
    Token_introspect_extension_mode_dynarray,
    Token_introspect_extension_mode_string,
    // Token_introspect_extension_key_count,
    // Token_introspect_extension_key_discriminant,
    Token__endblock__extension,
    // --
} FieldParseTokens;

StaticStringToken field_parse_tokens__static[] = {
    (StaticStringToken) { .id = Token_typedef, .token = "typedef" },
    (StaticStringToken) { .id = Token_struct, .token = "struct" },
    (StaticStringToken) { .id = Token_union, .token = "union" },

    (StaticStringToken) { .id = Token_const, .token = "const" },

    (StaticStringToken) { .id = Token_unsigned_char, .token = "unsigned char" },
    (StaticStringToken) { .id = Token_unsigned_int, .token = "unsigned int" },

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
    (StaticStringToken) { .id = Token_cr_paren_open, .token = "{" },
    (StaticStringToken) { .id = Token_cr_paren_close, .token = "}" },

    (StaticStringToken) { .id = Token_introspect_extension_alias, .token = "_alias__just_to_make_sure_no_token_overlap__" },
    (StaticStringToken) { .id = Token_introspect_extension_union_header, .token = "_union_header__just_to_make_sure_no_token_overlap__" },
    (StaticStringToken) { .id = Token_introspect_extension_mode_discriminated_union, .token = "_mode_discriminated_union__just_to_make_sure_no_token_overlap__" },
    (StaticStringToken) { .id = Token_introspect_extension_mode_cstr, .token = "_mode_cstr__just_to_make_sure_no_token_overlap__" },
    (StaticStringToken) { .id = Token_introspect_extension_mode_dynarray, .token = "_mode_dynarray__just_to_make_sure_no_token_overlap__" },
    (StaticStringToken) { .id = Token_introspect_extension_mode_string, .token = "_mode_string__just_to_make_sure_no_token_overlap__" },
    // (StaticStringToken) { .id = Token_introspect_extension_key_count, .token = "count" },
    // (StaticStringToken) { .id = Token_introspect_extension_key_discriminant, .token = "discriminant" },
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
    FieldExtension_union_header,
    FieldExtension_mode_cstr,
    FieldExtension_mode_dynarray,
    FieldExtension_mode_string,
    FieldExtension_head,
} FieldExtensionType;

typedef struct {
    // --
    bool ext_alias;
    char* type_alias;
    // --
    bool union_header;
    // --
    bool ext_discriminated_union;
    char* discriminant_field;
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

struct FieldInfoExt_s;

typedef struct {
    FieldInfo self_info;
    usize count;
    usize capacity;
    struct FieldInfoExt_s* variants;
} UnionInfo;

typedef struct FieldInfoExt_s {
    bool is_union;
    union {
        FieldInfo field_info;
        UnionInfo union_info;
    };
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
    if (field->is_union) {
        return TYPE_union;
    }

    char* type_name_cs = field->field_info.type_str;
    if (field->field_ext.ext_alias) {
        type_name_cs = field->field_ext.type_alias;
    }
    String type_name = string_from_cstr(type_name_cs);

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
        // --
        { .name = "int",      .type = TYPE_int32},
        { .name = "float",     .type = TYPE_float32},
        { .name = "double",     .type = TYPE_float64},
        // --
        { .name = "unsigned char",      .type = TYPE_byte},
        { .name = "unsigned int",     .type = TYPE_uint32},
        // --
        { .name = "unsigned short",     .type = TYPE_uint16},
        { .name = "unsigned long",     .type = TYPE_uint32},
        { .name = "unsigned long long",     .type = TYPE_uint64},
        // --
        { .name = "unsigned short int",     .type = TYPE_uint16},
        { .name = "unsigned long int",     .type = TYPE_uint32},
        { .name = "unsigned long long int",     .type = TYPE_uint64},
        // --
        { .name = "short",     .type = TYPE_int16},
        { .name = "long",     .type = TYPE_int32},
        { .name = "long long",     .type = TYPE_int64},
        // --
        { .name = "short int",     .type = TYPE_int16},
        { .name = "long int",     .type = TYPE_int32},
        { .name = "long long int",     .type = TYPE_int64},
        // --
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
    case TYPE_union:
        return "TYPE_union";
    }
    PANIC("Unknown Type\n");
    return "";
}

void write_union_introspect(StringBuilder* GEN, StructInfo* struct_info, UnionInfo* union_info, usize union_index) {
    // TODO
}

void write_introspect_fieldinfo_array(StringBuilder* GEN, String name, String base_struct_type_name, StructInfo* struct_info, bool is_union) {
    string_builder_append_format(GEN, "static FieldInfo %s[] = {\n", name.cstr);

    usize union_i = 0;
    for (usize i = 0; i < struct_info->count; i++) {
        FieldInfoExt* field = &struct_info->fields[i];
        Type field_type_enum = type_of_field(field);
        char* field_type = type_to_type_str(field_type_enum);

        String field_offset_str = string_new();
        if (is_union) {
            field_offset_str = string_from_cstr("0");
        }
        else {
            string_hinted_append_format(field_offset_str, 50,
                "&(((%s*)(0))->%s)",
                struct_info->type_name.cstr,
                field->field_info.name
            );
        }

        string_builder_append_format(GEN,
            "\t{\n\t\t.type = %s, .name = \"%s\", .ptr = %s,\n",
            field_type,
            field->field_info.name,
            field_offset_str.cstr
        );

        if (field->field_info.is_ptr) {
            string_builder_append_format(GEN,
                "\t\t.is_ptr = true, .ptr_depth = %u,\n",
                field->field_info.ptr_depth
            );
        }

        if (field->field_info.is_array) {
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
                // struct_info->type_name.cstr,
                base_struct_type_name.cstr,
                field->field_ext.dynarray_count_field
            );
        }

        if (field->field_ext.ext_mode_string) {
            string_builder_append_format(GEN,
                "\t\t.is_string = true, .count_ptr = &(((%s*)(0))->%s),\n",
                // struct_info->type_name.cstr,
                base_struct_type_name.cstr,
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

        if (field_type_enum == TYPE_union) {
            string_builder_append_format(GEN,
                "\t\t.is_named_union = %s, .union_name = \"%s\", .union_header_variant = %u,\n",
                field->field_info.is_named_union ? "true" : "false",
                field->field_info.union_name,
                field->field_info.union_header_variant
            );
            if (field->field_ext.ext_discriminated_union) {
                string_builder_append_format(GEN,
                    "\t\t.is_discriminated_union = true, .discriminant_ptr = &(((%s*)(0))->%s),\n",
                    struct_info->type_name.cstr,
                    field->field_ext.discriminant_field
                );
            }
            string_builder_append_format(GEN,
                "\t\t.union_size = sizeof(%s), .variant_count = ARRAY_LENGTH(%s__union_%d__variants), .variants = %s__union_%d__variants,\n",
                field->field_info.type_str,
                struct_info->type_name.cstr,
                union_i,
                struct_info->type_name.cstr,
                union_i
            );
            union_i++;
        }

        string_builder_append_cstr(GEN, "\t},\n");
    }
    string_builder_append_cstr(GEN, "};\n\n");
}

void write_introspect(StringBuilder* GEN, StructInfo* struct_info) {
    String array_name = string_new();
    usize union_i = 0;
    for (usize i = 0; i < struct_info->count; i++) {
        FieldInfoExt* field = &struct_info->fields[i];
        if (field->is_union) {
            UnionInfo* union_info = &field->union_info;
            StructInfo union_as_struct_info = {
                .count = union_info->count,
                .capacity = union_info->capacity,
                .fields = union_info->variants,
            };
            string_hinted_append_format(array_name, 50, "%s__union_%d__variants", struct_info->type_name.cstr, union_i);
            write_introspect_fieldinfo_array(GEN, array_name, struct_info->type_name, &union_as_struct_info, true);
            clear_string(&array_name);
            union_i++;
        }
    }

    string_hinted_append_format(array_name, 50, "%s__fields", struct_info->type_name.cstr);
    write_introspect_fieldinfo_array(GEN, array_name, struct_info->type_name, struct_info, false);
    return;

    // string_builder_append_format(GEN, "static FieldInfo %s__fields[] = {\n", struct_info->type_name.cstr);

    // for (usize i = 0; i < struct_info->count; i++) {
    //     FieldInfoExt* field = &struct_info->fields[i];
    //     Type field_type_enum = type_of_field(field);
    //     char* field_type = type_to_type_str(field_type_enum);

    //     string_builder_append_format(GEN,
    //         "\t{\n\t\t.type = %s, .name = \"%s\", .ptr = &(((%s*)(0))->%s),\n",
    //         field_type,
    //         field->field_info.name,
    //         struct_info->type_name.cstr,
    //         field->field_info.name
    //     );

    //     if (field->field_info.is_ptr) {
    //         string_builder_append_format(GEN,
    //             "\t\t.is_ptr = true, .ptr_depth = %u,\n",
    //             field->field_info.ptr_depth
    //         );
    //     }

    //     if (field->field_info.is_array) {
    //         string_builder_append_format(GEN,
    //             "\t\t.is_array = true, .count = %llu, .array_dim = %llu, .array_dim_counts = {",
    //             field->field_info.count,
    //             field->field_info.array_dim
    //         );
    //         for (usize dim_i = 0; dim_i < field->field_info.array_dim; dim_i++) {
    //             string_builder_append_format(GEN,
    //                 "%llu%s",
    //                 field->field_info.array_dim_counts[dim_i],
    //                 (dim_i != field->field_info.array_dim - 1) ? ", " : ""
    //             );
    //         }
    //         string_builder_append_cstr(GEN, "},\n");
    //     }

    //     if (field->field_ext.ext_mode_cstr) {
    //         string_builder_append_cstr(GEN, "\t\t.is_cstr = true,\n");
    //     }

    //     if (field->field_ext.ext_mode_dynarray) {
    //         string_builder_append_format(GEN,
    //             "\t\t.is_dynarray = true, .count_ptr = &(((%s*)(0))->%s),\n",
    //             struct_info->type_name.cstr,
    //             field->field_ext.dynarray_count_field
    //         );
    //     }

    //     if (field->field_ext.ext_mode_string) {
    //         string_builder_append_format(GEN,
    //             "\t\t.is_string = true, .count_ptr = &(((%s*)(0))->%s),\n",
    //             struct_info->type_name.cstr,
    //             field->field_ext.string_count_field
    //         );
    //     }

    //     if (field_type_enum == TYPE_struct) {
    //         string_builder_append_format(GEN,
    //             "\t\t.struct_size = sizeof(%s), .field_count = ARRAY_LENGTH(%s__fields), .fields = %s__fields,\n",
    //             field->field_info.type_str,
    //             field->field_info.type_str,
    //             field->field_info.type_str
    //         );
    //     }

    //     string_builder_append_cstr(GEN, "\t},\n");
    // }
    // string_builder_append_cstr(GEN, "};\n\n");
}

String gen_introspect_file() {
    StringBuilder GEN = string_builder_new();

    string_builder_append_cstr(&GEN, "#pragma once\n\n");
    string_builder_append_cstr(&GEN, "#ifndef PRE_INTROSPECT_PASS\n\n");
    string_builder_append_cstr(&GEN, "#include \"justengine.h\"\n\n");

    string_builder_append_cstr(
        &GEN,
        "/**\n"
        " * !! IMPORTANT !!\n"
        " * IMPORT THIS HEADER AFTER THESE DEFINITONS\n"
    );
    for (usize i = 0; i < INTROSPECTED_STRUCTS.count; i++) {
        StructInfo* struct_info = &INTROSPECTED_STRUCTS.structs[i];
        string_builder_append_format(
            &GEN,
            " * - %s\n",
            struct_info->type_name.cstr
        );
    }
    string_builder_append_cstr(
        &GEN,
        "*/\n\n"
    );

    for (usize i = 0; i < INTROSPECTED_STRUCTS.count; i++) {
        StructInfo* struct_info = &INTROSPECTED_STRUCTS.structs[i];
        string_builder_append_format(&GEN, "static FieldInfo %s__fields[%llu];\n", struct_info->type_name.cstr, struct_info->count);
    }
    string_builder_append_cstr(&GEN, "\n");

    for (usize i = 0; i < INTROSPECTED_STRUCTS.count; i++) {
        StructInfo* struct_info = &INTROSPECTED_STRUCTS.structs[i];
        write_introspect(&GEN, struct_info);
    }
    string_builder_append_cstr(&GEN, "\n");

    for (usize i = 0; i < INTROSPECTED_STRUCTS.count; i++) {
        StructInfo* struct_info = &INTROSPECTED_STRUCTS.structs[i];
        string_builder_append_format(&GEN, "__IMPL_____generate_print_functions(%s);\n", struct_info->type_name.cstr);
    }
    string_builder_append_cstr(&GEN, "\n");
    
    string_builder_append_cstr(&GEN, "#endif\n\n");

    return build_string(&GEN);
}

// typedef struct {
//     bool a;
//     union {
//         bool b;
//         union {
//             bool c;
//             int d;
//         };
//     } union(lead: b);
// } asd;

// void f(asd sd) {
//     sd.d;
// }

// FieldInfoExt parse_field_from_state_2(StringTokensIter* tokens_iter, StringTokenOut token, FieldParseState start_state);
FieldInfoExt parse_field_2(StringTokensIter* tokens_iter, StringTokenOut token);
UnionInfo parse_union_2(StringTokensIter* tokens_iter, StringTokenOut token);
StructInfo parse_struct_2(StringTokensIter* tokens_iter, StringTokenOut token);

void generate_introspect_for_struct(StringView struct_def) {
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
    struct_info.type_name = string_from_view(string_view_trimmed(string_view_slice_view(struct_def, curly_paren_close + 1, struct_def.count-1 - curly_paren_close - 1)));

    if (already_introspected(struct_info.type_name)) {
        JUST_LOG_INFO("Introspection already generated for type: %s.\n", struct_info.type_name.cstr);
        return;
    }

    // StringView struct_fields = string_slice_view(struct_def, curly_paren_open + 1, curly_paren_close - curly_paren_open - 1);

    usize tokens_count = ARRAY_LENGTH(field_parse_tokens__static);
    StringToken* field_parse_tokens = string_tokens_from_static(field_parse_tokens__static, tokens_count);
    StringTokensIter tokens_iter = string_view_iter_tokens(struct_def, field_parse_tokens, tokens_count);
    StringTokenOut token;
    
    peek_token(&tokens_iter, &token);
    println_string_view(token.token);
    expect_token(&tokens_iter, Token_typedef);
    if (next_token(&tokens_iter, &token)) {
        StructInfo struct_info = parse_struct_2(&tokens_iter, token);
        dynarray_push_back_custom(INTROSPECTED_STRUCTS, .structs, struct_info);
    }
    else {
        PANIC("Typedef parse error.\n");
    }
}

struct s {
    union us {
        const bool a;
        int i;
    };
};
void _f() {
    struct s sv;
    union us usv;
    sizeof(struct s);
    sizeof(union us);
}

FieldInfoExt parse_field_2(StringTokensIter* tokens_iter, StringTokenOut token) {
    bool is_union = false;
    FieldInfo field_info = {0};
    UnionInfo union_info = {0};
    FieldExtensions field_ext = {0};

    FieldParseState state = FieldParse_Begin;
    bool in_array_def = false;

    do {
        switch (state) {
        case FieldParse_Begin:
            switch (token.id) {
                case Token_const:
                    break;
                // case Token_struct: // TODO:
                case Token_union:
                    is_union = true;
                    union_info = parse_union_2(tokens_iter, token);

                    field_info.type_str = union_info.self_info.type_str;

                    usize union_header = 0;
                    for (usize i = 0; i < union_info.count; i++) {
                        FieldInfoExt* variant = &union_info.variants[i];
                        if (variant->field_ext.union_header) {
                            union_header = i;
                        }
                    }
                    field_info.union_header_variant = union_header;
                    field_info.name = union_info.variants[union_header].field_info.name;

                    state = FieldParse_AfterType;
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
                if (token.free_word) {
                    if (is_union) {
                        field_info.is_named_union = true;

                        String fname = string_new();
                        string_nappend_cstr(&fname, token.token.str, token.token.count);
                        string_push_char(&fname, '.');

                        string_append_cstr(&fname, field_info.name);
                        std_free(field_info.name);
                        println_string(fname);
                        field_info.name = fname.cstr;

                        field_info.union_name = cstr_nclone(token.token.str, token.token.count);
                    }
                    else {
                        field_info.name = cstr_nclone(token.token.str, token.token.count);
                    }
                    state = FieldParse_AfterName;
                }
                else if (is_union) {
                    state = FieldParse_AfterName;
                    goto CASE_FieldParse_AfterName;
                }
                // else if (Token__startblock__extension < token.id && token.id < Token__endblock__extension) {
                //     field_info.name = cstr_nclone(token.token.str, token.token.count);
                //     state = FieldParse_AfterName;
                // }
                else {
                    JUST_LOG_DEBUG("token.id: %d\n", token.id);
                    JUST_LOG_DEBUG("token.free_word: %d\n", token.free_word);
                    JUST_LOG_DEBUG("token.token: "); println_string_view(token.token);
                    PANIC("Field parse error.\n");
                }
                break;
            }
            break;
        case FieldParse_AfterName:
            CASE_FieldParse_AfterName:
            if (in_array_def) {
                if (field_info.count == 0){
                    field_info.count = 1;
                }
                uint64 dim;
                if (!sv_parse_uint64(token.token, &dim)) {
                    PANIC("Array dim syntax error\n");
                }
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
                goto END_FIELD_PARSE;
                // FieldInfoExt field_info_ext = {
                //     .field_info = field_info,
                //     .field_ext = field_ext,
                // };
                // return field_info_ext;
                // dynarray_push_back_custom(struct_info, .fields, field_info_ext);
                
                // state = FieldParse_Begin;
                // field_info = (FieldInfo) {0};
                // field_ext = (FieldExtensions) {0};
                // in_array_def = false;

                // break;
            // -- extensions --
            case Token_introspect_extension_alias:
                field_ext.ext_alias = true;
                expect_token(tokens_iter, Token_paren_open);
                if (next_token(tokens_iter, &token)) {
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
                expect_token(tokens_iter, Token_paren_close);
                break;
            case Token_introspect_extension_union_header:
                field_ext.union_header = true;
                expect_token(tokens_iter, Token_paren_open);
                expect_token(tokens_iter, Token_paren_close);
                break;
            case Token_introspect_extension_mode_discriminated_union:
                field_ext.ext_discriminated_union = true;
                expect_token(tokens_iter, Token_paren_open);
                // expect_token_cstr(tokens_iter, "discriminant");
                // expect_token(tokens_iter, Token_colon);
                if (next_token(tokens_iter, &token)) {
                    field_ext.discriminant_field = string_from_view(token.token).cstr;
                }
                else {
                    PANIC("Extension syntax error.\n");
                }
                expect_token(tokens_iter, Token_paren_close);
                break;
            case Token_introspect_extension_mode_cstr:
                field_ext.ext_mode_cstr = true;
                expect_token(tokens_iter, Token_paren_open);
                expect_token(tokens_iter, Token_paren_close);
                break;
            case Token_introspect_extension_mode_dynarray:
                field_ext.ext_mode_dynarray = true;
                expect_token(tokens_iter, Token_paren_open);
                // expect_token_cstr(tokens_iter, "count");
                // expect_token(tokens_iter, Token_colon);
                if (next_token(tokens_iter, &token)) {
                    field_ext.dynarray_count_field = string_from_view(token.token).cstr;
                }
                else {
                    PANIC("Extension syntax error.\n");
                }
                expect_token(tokens_iter, Token_paren_close);
                break;
            case Token_introspect_extension_mode_string:
                field_ext.ext_mode_string = true;
                expect_token(tokens_iter, Token_paren_open);
                // expect_token_cstr(tokens_iter, "count");
                // expect_token(tokens_iter, Token_colon);
                if (next_token(tokens_iter, &token)) {
                    field_ext.string_count_field = string_from_view(token.token).cstr;
                }
                else {
                    PANIC("Extension syntax error.\n");
                }
                expect_token(tokens_iter, Token_paren_close);
                break;
            }
        }
    } while (next_token(tokens_iter, &token));
    END_FIELD_PARSE:

    FieldInfoExt field = {
        .is_union = is_union,
        .field_ext = field_ext,
    };
    if (is_union) {
        union_info.self_info = field_info;
        field.union_info = union_info;
    }
    else {
        field.field_info = field_info;
    }

    return field;
}

UnionInfo parse_union_2(StringTokensIter* tokens_iter, StringTokenOut token) {
    UnionInfo union_info = {0};

    StringView start = tokens_iter->cursor;

    ASSERT(token.id == Token_union);
    while (next_token(tokens_iter, &token)) {
        if (token.free_word) {
            // union struct name
            continue;
        }
        switch (token.id) {
        case Token_cr_paren_open:
            goto START_PARSE_UNION_FIELDS;
        default:
            PANIC("Union parse error.\n");
        }
    }

    START_PARSE_UNION_FIELDS:
    while(next_token(tokens_iter, &token)) {
        switch (token.id) {
        case Token_cr_paren_close:
            goto END_PARSE_UNION_FIELDS;

        // -- Field Start
        // case Token_struct: // TODO:
        // case Token_union: // TODO:
        case Token_const:
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
            FieldInfoExt field = parse_field_2(tokens_iter, token);
            dynarray_push_back_custom(union_info, .variants, field);
            break;
        default:
            if (token.free_word) {
                FieldInfoExt field = parse_field_2(tokens_iter, token);
                dynarray_push_back_custom(union_info, .variants, field);
            }
            else {
                string_view_use_as_cstr(token.token, char* token_cstr, ({
                    PANIC("Union parse error. Unexpected token: %d, %s\n", token.id, token_cstr);
                }));
            }
            break;
        }
        // -- Field End
    };
    END_PARSE_UNION_FIELDS:

    StringView end = tokens_iter->cursor;
    StringView traversed = string_view_slice_view(start, 0, start.count - end.count);

    String type_str = string_from_cstr("union");

    bool in_whitespace = false;
    for (usize i = 0; i < traversed.count; i++) {
        char ch = traversed.str[i];
        bool ch_is_whitespace = char_is_whitespace(ch);

        if (!in_whitespace && !ch_is_whitespace) {
            string_push_char(&type_str, ch);
        }
        else if (!in_whitespace && ch_is_whitespace) {
            string_push_char(&type_str, ' ');
            in_whitespace = true;
        }
        else if (in_whitespace && !ch_is_whitespace) {
            string_push_char(&type_str, ch);
            in_whitespace = false;
        }
        else { // if (in_whitespace && ch_is_whitespace)
            continue;
        }
    }
    JUST_LOG_INFO("union type_str: %s\n", type_str.cstr);
    StringView s1 = string_as_view(type_str);
    StringView s2 = string_view_trimmed(s1);
    String s3 = string_from_view(s2);
    println_string_view(s1);
    println_string_view(s2);
    println_string(s3);
    String type_str_trimmed = string_from_view(string_view_trimmed(string_as_view(type_str)));
    free_string(type_str);

    JUST_LOG_INFO("union type_str: %s\n", type_str_trimmed.cstr);

    union_info.self_info.type_str = type_str_trimmed.cstr;

    return union_info;
}

StructInfo parse_struct_2(StringTokensIter* tokens_iter, StringTokenOut token) {
    StructInfo struct_info = {0};

    JUST_LOG_DEBUG("token.id: %d\n", token.id);
    ASSERT(token.id == Token_struct);
    while (next_token(tokens_iter, &token)) {
        if (token.free_word) {
            // struct name
            struct_info.type_name = string_from_view(token.token);
            continue;
        }
        switch (token.id) {
        case Token_cr_paren_open:
            goto START_PARSE_STRUCT_FIELDS;
        default:
            PANIC("Struct parse error.\n");
        }
    }

    START_PARSE_STRUCT_FIELDS:
    while(next_token(tokens_iter, &token)) {
        switch (token.id) {
        case Token_cr_paren_close:
            goto END_PARSE_STRUCT_FIELDS;

        // -- Field Start
        // case Token_struct: // TODO:
        case Token_union:
            // FieldInfoExt union_field = parse_union_2(tokens_iter, token);
            // dynarray_push_back_custom(struct_info, .fields, union_field);
            // break;
        case Token_const:
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
            FieldInfoExt field = parse_field_2(tokens_iter, token);
            dynarray_push_back_custom(struct_info, .fields, field);
            break;
        default:
            if (token.free_word) {
                FieldInfoExt field = parse_field_2(tokens_iter, token);
                dynarray_push_back_custom(struct_info, .fields, field);
            }
            else {
                string_view_use_as_cstr(token.token, char* token_cstr, ({
                    PANIC("Struct parse error. Unexpected token: %d, %s\n", token.id, token_cstr);
                }));
            }
            break;
        }
        // -- Field End
    };
    END_PARSE_STRUCT_FIELDS:

    while (next_token(tokens_iter, &token)) {
        if (token.free_word) {
            // type name
            struct_info.type_name = string_from_view(token.token);
            continue;
        }
        switch (token.id) {
        case Token_semicolon:
            if (struct_info.type_name.count == 0) {
                PANIC("Struct parse error. No name defined.\n");
            }
            goto END_PARSE_STRUCT;
        default:
            PANIC("Struct parse error.\n");
        }
    }

    END_PARSE_STRUCT:
    return struct_info;
}

// FieldInfoExt parse_field(StringTokensIter* tokens_iter, FieldParseState start_state);
// FieldInfoExt parse_union(StringTokensIter* tokens_iter);

// FieldInfoExt parse_field(StringTokensIter* tokens_iter, FieldParseState start_state) {
//     FieldParseState state = start_state; // FieldParse_Begin
//     StringTokenOut token;

//     bool is_union = false;
//     FieldInfo field_info = {0};
//     UnionInfo union_info = {0};
//     FieldExtensions field_ext = {0};
//     bool in_array_def = false;

//     FieldInfoExt field_info_ext;

//     while (next_token(tokens_iter, &token)) {
//         if (Token__startblock__extension < token.id && token.id < Token__endblock__extension) {
//             state = FieldParse_AfterName;
//         }
//         switch (state) {
//         case FieldParse_Begin:
//             switch (token.id) {
//                 case Token_const:
//                     break;
//                 case Token_union:
//                     field_info_ext = parse_union(tokens_iter);
//                     goto END_FIELD_RETURN;
//                 case Token_unsigned_short:
//                 case Token_unsigned_long:
//                 case Token_unsigned_long_long:
//                 case Token_unsigned_short_int:
//                 case Token_unsigned_long_int:
//                 case Token_unsigned_long_long_int:
//                 case Token_short:
//                 case Token_long:
//                 case Token_long_long:
//                 case Token_short_int:
//                 case Token_long_int:
//                 case Token_long_long_int:
//                 default:
//                     field_info.type_str = cstr_nclone(token.token.str, token.token.count);
//                     state = FieldParse_AfterType;
//                     break;
//                 }
//             break;
//         case FieldParse_AfterType:
//             switch (token.id) {
//             case Token_star:
//                 field_info.is_ptr = true;
//                 field_info.ptr_depth++;
//                 break;
//             default:
//                 field_info.name = cstr_nclone(token.token.str, token.token.count);
//                 state = FieldParse_AfterName;
//                 break;
//             }
//             break;
//         case FieldParse_AfterName:
//             if (in_array_def) {
//                 if (field_info.count == 0){
//                     field_info.count = 1;
//                 }
//                 uint64 dim;
//                 bool success = sv_parse_uint64(token.token, &dim);
//                 if (!success) {
//                     PANIC("Array dim syntax error\n");
//                 }
//                 field_info.count *= dim;
//                 field_info.array_dim_counts[field_info.array_dim++] = dim;

//                 in_array_def = false;
//             }
//             switch (token.id) {
//             case Token_sq_paren_open:
//                 in_array_def = true;
//                 field_info.is_array = true;
//                 break;
//             case Token_sq_paren_close:
//                 in_array_def = false;
//                 break;
//             case Token_semicolon:
//                 goto END_FIELD;
//                 // FieldInfoExt field_info_ext = {
//                 //     .field_info = field_info,
//                 //     .field_ext = field_ext,
//                 // };
//                 // return field_info_ext;
//                 // dynarray_push_back_custom(struct_info, .fields, field_info_ext);
                
//                 // state = FieldParse_Begin;
//                 // field_info = (FieldInfo) {0};
//                 // field_ext = (FieldExtensions) {0};
//                 // in_array_def = false;

//                 // break;
//             // -- extensions --
//             case Token_introspect_extension_alias:
//                 field_ext.ext_alias = true;
//                 expect_token(tokens_iter, Token_paren_open);
//                 if (next_token(tokens_iter, &token)) {
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
//                 expect_token(tokens_iter, Token_paren_close);
//                 break;
//             case Token_introspect_extension_union_header:
//                 field_ext.union_header = true;
//                 expect_token(tokens_iter, Token_paren_open);
//                 expect_token(tokens_iter, Token_paren_close);
//                 break;
//             case Token_introspect_extension_mode_discriminated_union:
//                 field_ext.ext_discriminated_union = true;
//                 expect_token(tokens_iter, Token_paren_open);
//                 expect_token(tokens_iter, Token_introspect_extension_key_discriminant);
//                 expect_token(tokens_iter, Token_colon);
//                 if (next_token(tokens_iter, &token)) {
//                     field_ext.discriminant_field = string_from_view(token.token).cstr;
//                 }
//                 else {
//                     PANIC("Extension syntax error.\n");
//                 }
//                 expect_token(tokens_iter, Token_paren_close);
//                 break;
//             case Token_introspect_extension_mode_cstr:
//                 field_ext.ext_mode_cstr = true;
//                 expect_token(tokens_iter, Token_paren_open);
//                 expect_token(tokens_iter, Token_paren_close);
//                 break;
//             case Token_introspect_extension_mode_dynarray:
//                 field_ext.ext_mode_dynarray = true;
//                 expect_token(tokens_iter, Token_paren_open);
//                 expect_token(tokens_iter, Token_introspect_extension_key_count);
//                 expect_token(tokens_iter, Token_colon);
//                 if (next_token(tokens_iter, &token)) {
//                     field_ext.dynarray_count_field = string_from_view(token.token).cstr;
//                 }
//                 else {
//                     PANIC("Extension syntax error.\n");
//                 }
//                 expect_token(tokens_iter, Token_paren_close);
//                 break;
//             case Token_introspect_extension_mode_string:
//                 field_ext.ext_mode_string = true;
//                 expect_token(tokens_iter, Token_paren_open);
//                 expect_token(tokens_iter, Token_introspect_extension_key_count);
//                 expect_token(tokens_iter, Token_colon);
//                 if (next_token(tokens_iter, &token)) {
//                     field_ext.string_count_field = string_from_view(token.token).cstr;
//                 }
//                 else {
//                     PANIC("Extension syntax error.\n");
//                 }
//                 expect_token(tokens_iter, Token_paren_close);
//                 break;
//             }
//         }
//     }

//     END_FIELD:
//     field_info_ext.is_union = is_union;
//     field_info_ext.field_ext = field_ext;
//     if (is_union) {
//         field_info_ext.union_info = union_info;
//     }
//     else {
//         field_info_ext.field_info = field_info;
//     }

//     END_FIELD_RETURN:
//     return field_info_ext;
// }

// FieldInfoExt parse_union(StringTokensIter* tokens_iter) {
//     UnionInfo union_info = {0};
//     FieldExtensions field_ext = {0};

//     StringTokenOut token;

//     expect_token(tokens_iter, Token_cr_paren_open);
//     while (peek_token(tokens_iter, &token)) {
//         if (token.id == Token_cr_paren_close) {
//             break;
//         }
//         else {
//             FieldInfoExt field_info_ext = parse_field(tokens_iter, FieldParse_Begin);
//             dynarray_push_back_custom(union_info, .variants, field_info_ext);
//         }
//     }
//     expect_token(tokens_iter, Token_cr_paren_close);

//     if (!peek_token(tokens_iter, &token)) {
//         PANIC("Union parse error\n");
//     }
//     if (token.id == Token_semicolon) {
//         // return union_info;
//     }
//     else {
//         FieldInfoExt self_info_ext = parse_field(tokens_iter, FieldParse_AfterType);
//         union_info.self_info = self_info_ext.field_info;
//         field_ext = self_info_ext.field_ext;
//     }

//     return (FieldInfoExt) {
//         .is_union = true,
//         .union_info = union_info,
//         .field_ext = field_ext,
//     };
// }

// void generate_introspect_for_struct(String struct_def) {
//     usize curly_paren_open;
//     usize curly_paren_close;
//     for (usize i = 0; i < struct_def.count; i++) {
//         if (struct_def.str[i] == '{') {
//             curly_paren_open = i;
//             break;
//         }
//     }
//     for (int64 i = struct_def.count-1; i >= 0; i--) {
//         if (struct_def.str[i] == '}') {
//             curly_paren_close = i;
//             break;
//         }
//     }

//     if (curly_paren_open >= curly_paren_close) {
//         PANIC("Syntax error: curly paren\n");
//     }
    
//     StructInfo struct_info = {0};
//     struct_info.type_name = string_from_view(string_view_trimmed(string_slice_view(struct_def, curly_paren_close + 1, struct_def.count-1 - curly_paren_close - 1)));

//     if (already_introspected(struct_info.type_name)) {
//         JUST_LOG_INFO("Introspection already generated for type: %s.\n", struct_info.type_name.cstr);
//         return;
//     }

//     StringView struct_fields = string_slice_view(struct_def, curly_paren_open + 1, curly_paren_close - curly_paren_open - 1);

//     usize tokens_count = ARRAY_LENGTH(field_parse_tokens__static);
//     StringToken* field_parse_tokens = string_tokens_from_static(field_parse_tokens__static, tokens_count);
//     StringTokensIter tokens_iter = string_view_iter_tokens(struct_fields, field_parse_tokens, tokens_count);
//     StringTokenOut token;

//     FieldParseState state = FieldParse_Begin;
//     FieldInfo field_info = {0};
//     FieldExtensions field_ext = {0};
//     bool in_array_def = false;

//     // usize i = 0;
//     while (next_token(&tokens_iter, &token)) {
//         switch (state) {
//         case FieldParse_Begin:
//             switch (token.id) {
//                 case Token_const:
//                     break;
//                 case Token_unsigned_short:
//                 case Token_unsigned_long:
//                 case Token_unsigned_long_long:
//                 case Token_unsigned_short_int:
//                 case Token_unsigned_long_int:
//                 case Token_unsigned_long_long_int:
//                 case Token_short:
//                 case Token_long:
//                 case Token_long_long:
//                 case Token_short_int:
//                 case Token_long_int:
//                 case Token_long_long_int:
//                 default:
//                     field_info.type_str = cstr_nclone(token.token.str, token.token.count);
//                     state = FieldParse_AfterType;
//                     break;
//                 }
//             break;
//         case FieldParse_AfterType:
//             switch (token.id) {
//             case Token_star:
//                 field_info.is_ptr = true;
//                 field_info.ptr_depth++;
//                 break;
//             default:
//                 field_info.name = cstr_nclone(token.token.str, token.token.count);
//                 state = FieldParse_AfterName;
//                 break;
//             }
//             break;
//         case FieldParse_AfterName:
//             if (in_array_def) {
//                 if (field_info.count == 0){
//                     field_info.count = 1;
//                 }
//                 uint64 dim;
//                 bool success = sv_parse_uint64(token.token, &dim);
//                 if (!success) {
//                     PANIC("Array dim syntax error\n");
//                 }
//                 field_info.count *= dim;
//                 field_info.array_dim_counts[field_info.array_dim++] = dim;

//                 in_array_def = false;
//             }
//             switch (token.id) {
//             case Token_sq_paren_open:
//                 in_array_def = true;
//                 field_info.is_array = true;
//                 break;
//             case Token_sq_paren_close:
//                 in_array_def = false;
//                 break;
//             case Token_semicolon:
//                 FieldInfoExt field_info_ext = {
//                     .field_info = field_info,
//                     .field_ext = field_ext,
//                 };
//                 dynarray_push_back_custom(struct_info, .fields, field_info_ext);
                
//                 state = FieldParse_Begin;
//                 field_info = (FieldInfo) {0};
//                 field_ext = (FieldExtensions) {0};
//                 in_array_def = false;

//                 break;
//             // -- extensions --
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
//                 field_ext.ext_mode_string = true;
//                 expect_token(&tokens_iter, Token_paren_open);
//                 expect_token(&tokens_iter, Token_introspect_extension_key_count);
//                 expect_token(&tokens_iter, Token_colon);
//                 if (next_token(&tokens_iter, &token)) {
//                     field_ext.string_count_field = string_from_view(token.token).cstr;
//                 }
//                 else {
//                     PANIC("Extension syntax error.\n");
//                 }
//                 expect_token(&tokens_iter, Token_paren_close);
//                 break;
//             }
//         }
//     }

//     dynarray_push_back_custom(INTROSPECTED_STRUCTS, .structs, struct_info);

//     free_tokens_iter(&tokens_iter);
// }

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
    string_append_cstr(&cmd_introspect, "__just_to_make_sure_no_token_overlap__");

    uint32 i = 0;
    bool gen_introspect = false;
    
    String struct_def_str = string_with_capacity(100);
    ParseStructDefState state = STATE_STRUCT_BEGIN;

    char ch;
    int32 paren_state = 0;
    while ((ch = fgetc(file)) != EOF) {
        if (gen_introspect) {
            string_push_char(&struct_def_str, ch);
            switch (state) {
            case STATE_STRUCT_BEGIN:
                if (ch == '{') {
                    paren_state++;
                    state = STATE_CURLY_PAREN_OPEN_RECEIVED;
                }
                break;
            case STATE_CURLY_PAREN_OPEN_RECEIVED:
                if (ch == '{') {
                    paren_state++;
                }
                else if (ch == '}') {
                    paren_state--;
                    if (paren_state == 0) {
                        state = STATE_CURLY_PAREN_CLOSE_RECEIVED;
                    }
                    else if (paren_state < 0) {
                        PANIC("Curly Paren Error\n");
                    }
                }
                break;
            case STATE_CURLY_PAREN_CLOSE_RECEIVED:
                if (ch == ';') {
                    StringView struct_def_view = string_as_view(struct_def_str);
                    generate_introspect_for_struct(struct_def_view);
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

    fclose(file);
    return true;
}

#include <dirent.h>
#include <sys/stat.h>

typedef struct {
    String full_path;
    String filename;
} FileEntry;

typedef struct {
    usize count;
    usize capacity;
    FileEntry* items;
} FileEntryList;

String SCANROOT = {0};
StringList EXCLUDE_FILES = {0};
StringList EXCLUDE_DIRS = {0};

void add_as_excluded_file(String filepath) {
    String excluded_filepath = string_new();
    string_append_format(excluded_filepath, "%s/%s", SCANROOT, filepath);
    dynarray_push_back(EXCLUDE_FILES, excluded_filepath);
}

bool is_file_excluded(String filepath) {
    for (usize i = 0; i < EXCLUDE_FILES.count; i++) {
        if (ss_equals(EXCLUDE_FILES.items[i], filepath)) {
            return true;
        }
    }
    return false;
}

void add_as_excluded_dir(String dirpath) {
    String excluded_dirpath = string_new();
    string_append_format(excluded_dirpath, "%s/%s", SCANROOT, dirpath);
    dynarray_push_back(EXCLUDE_FILES, dirpath);
}

bool is_dir_excluded(String dirpath) {
    for (usize i = 0; i < EXCLUDE_DIRS.count; i++) {
        if (ss_equals(EXCLUDE_DIRS.items[i], dirpath)) {
            return true;
        }
    }
    return false;
}

void files_in_directory_recursive(String path, FileEntryList* file_entries) {
    DIR* dir = opendir(path.cstr);
    if (!dir) {
        PANIC("Error opening dir: \"%s\"\n", path.cstr);
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (std_strcmp(entry->d_name, ".") == 0 || std_strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        String full_path = string_with_capacity(256);
        string_append_format(full_path, "%s/%s", path.cstr, entry->d_name);

        struct stat path_stat;
        if (stat(full_path.cstr, &path_stat) != 0) {
            PANIC("stat failed for: \"%s\"\n", full_path.cstr);
        }

        if (S_ISDIR(path_stat.st_mode)) {
            if (!is_dir_excluded(full_path)) {
                files_in_directory_recursive(full_path, file_entries);
            }
            free_string(full_path);
        }
        else {
            if (is_file_excluded(full_path)) {
                free_string(full_path);
            }
            else {
                FileEntry file_entry = {
                    .full_path = full_path,
                    .filename = string_from_cstr(entry->d_name),
                };
                dynarray_push_back(*file_entries, file_entry);
            }
        }
    }

    closedir(dir);
}

void list_file_entries(String path, FileEntryList* file_entries) {
    struct stat path_stat;
    if (stat(path.cstr, &path_stat) != 0) {
        PANIC("stat failed for: \"%s\"\n", path.cstr);
    }

    if (S_ISDIR(path_stat.st_mode)) {
        files_in_directory_recursive(path, file_entries);
    }
    else {
        int i_last_slash = -1;
        for (int i = 0; i < path.count; i++) {
            char ch = path.str[i];
            if (ch == '/' || ch == '\\') {
                i_last_slash = i;
            }
        }
        int start = i_last_slash + 1;
        String filename = string_from_view(string_slice_view(path, start, path.count - start));

        FileEntry file_entry = {
            .full_path = clone_string(path),
            .filename = filename,
        };
        dynarray_push_back(*file_entries, file_entry);
    }
}

typedef struct {
    usize count;
    usize capacity;
    usize* items;
} ProcessIds;

int main(int argc, char* argv[]) {
    if (argc <= 2) {
        PANIC("Please input root directory for scanning and output file path\n");
    }
    
    String scanroot_path = string_from_cstr(argv[1]);
    String genoutput_path = string_from_cstr(argv[2]);

    StringList include_paths = {0};
    dynarray_reserve(include_paths, argc - 3);
    for (uint32 i = 3; i < argc; i++) {
        String include_path = string_from_cstr(argv[i]);
        dynarray_push_back(include_paths, include_path);
    }

    SCANROOT = clone_string(scanroot_path);
    add_as_excluded_file(genoutput_path);

    FileEntryList file_entries = {0};
    dynarray_reserve(file_entries, 10);
    list_file_entries(scanroot_path, &file_entries);
    free_string(scanroot_path);

    usize FILE_COUNT = file_entries.count;

    String INTROSPECT_FILE_SUFFIX_EXT = string_from_cstr("int");
    String TEMPDIR_ROOT_PATH = string_from_cstr("introspect_temp");
    {
        struct stat st = {0};
        if (stat(TEMPDIR_ROOT_PATH.cstr, &st) == -1) {
            mkdir(TEMPDIR_ROOT_PATH.cstr);
        }
    }

    StringList int_filepaths = {0};
    dynarray_reserve(int_filepaths, FILE_COUNT);
    for (usize i = 0; i < FILE_COUNT; i++) {
        String filename = file_entries.items[i].filename;
        String int_filepath = string_new();
        string_append_format(int_filepath, "%s/%s.%s", TEMPDIR_ROOT_PATH.cstr, filename.cstr, INTROSPECT_FILE_SUFFIX_EXT.cstr);
        dynarray_push_back(int_filepaths, int_filepath);
    }

    ProcessIds process_ids = {0};
    dynarray_reserve(process_ids, FILE_COUNT);
    process_ids.count = FILE_COUNT;

    bool spawn_success = true;
    usize PROCESS_COUNT = 0;
    for (usize i = 0; i < FILE_COUNT; i++) {
        String filepath = file_entries.items[i].full_path;
        String int_filepath = int_filepaths.items[i];

        if (spawn_success) {
            char** args_list = std_malloc(sizeof(char*) * (9 + 10 + include_paths.count));
            usize arg_i = 0;
    
            args_list[arg_i++] = "gcc";
            for (usize inc_i = 0; inc_i < include_paths.count; inc_i++) {
                args_list[arg_i++] = include_paths.items[inc_i].cstr;
            }
            args_list[arg_i++] = "-DPRE_INTROSPECT_PASS";
            args_list[arg_i++] = "-w";
            args_list[arg_i++] = "-E";
            args_list[arg_i++] = "-P";
            args_list[arg_i++] = filepath.cstr;
            args_list[arg_i++] = "-o";
            args_list[arg_i++] = int_filepath.cstr;
            args_list[arg_i++] = NULL;

            usize spawn_result = _spawnvp(
                _P_NOWAIT,
                "gcc",
                (const char* const*) args_list
            );
            if (spawn_result == -1) {
                JUST_LOG_ERROR("Failed to create preprocessor process for \"%s\"\n", filepath);
                spawn_success = false;
            }
            std_free(args_list);
    
            process_ids.items[i] = spawn_result;
            PROCESS_COUNT++;
        }
    }

    bool process_success = true;
    bool introspect_success = true;
    for (usize i = 0; i < PROCESS_COUNT; i++) {
        usize process_id = process_ids.items[i];
        String filepath = file_entries.items[i].full_path;

        int32 exit_status;
        usize wait_result = _cwait(&exit_status, process_ids.items[i], _WAIT_CHILD);
        if (wait_result == -1) {
            JUST_LOG_ERROR("Failed to wait process %llu, \"%s\"\n", process_id, filepath);
            process_success = false;
        }
        if (exit_status != 0) {
            JUST_LOG_ERROR("Process %llu exited with non-zero status, \"%s\"\n", process_id, filepath);
            process_success = false;
        }

        String int_filepath = int_filepaths.items[i];
        if (spawn_success && process_success && introspect_success) {
            introspect_success &= generate_introspect(int_filepath);
        }
    }

    for (usize i = 0; i < FILE_COUNT; i++) {
        String int_filepath = int_filepaths.items[i];
        remove(int_filepath.cstr);
    }
    rmdir(TEMPDIR_ROOT_PATH.cstr);

    if (!process_success || !introspect_success) {
        PANIC("Introspect failed\n");
    }

    String int_file_content = gen_introspect_file();
    FILE* file = fopen(genoutput_path.cstr, "w+");
    if (file == NULL) {
        PANIC("Error opening output file\n");
    }
    fputs(int_file_content.cstr, file);
    fclose(file);

    return 0;
}