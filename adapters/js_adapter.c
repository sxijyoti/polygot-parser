#include "adapters.h"
#include <string.h>
#include <stdio.h>
#include <tree_sitter/api.h>
#include "../ir/ir.h"

extern const TSLanguage *tree_sitter_javascript(void);

/*
function foo() {}              
const foo = function() {}           
const foo = () => {}                
*/

// same helper for all languages
static char *read_file(const char *path, uint32_t *len_out) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = malloc((size_t)sz + 1);
    if (!buf) { fclose(f); return NULL; }
    fread(buf, 1, (size_t)sz, f);
    buf[sz] = '\0';
    fclose(f);
    if (len_out) *len_out = (uint32_t)sz;
    return buf;
}

static void node_text(TSNode node, const char *src, char *out, size_t cap) {
    uint32_t s = ts_node_start_byte(node);
    uint32_t e = ts_node_end_byte(node);
    uint32_t n = e - s;
    if (n >= (uint32_t)cap) n = (uint32_t)(cap - 1);
    memcpy(out, src + s, n);
    out[n] = '\0';
}

static void collect_params(TSNode params, const char *src, ir_symbol *sym) {
    uint32_t n = ts_node_named_child_count(params);
    for (uint32_t i = 0; i < n; i++) {
        TSNode child = ts_node_named_child(params, i);
        const char *t = ts_node_type(child);
        char arg[IR_ARG_LEN] = {0};

        if (strcmp(t, "identifier") == 0) {
            node_text(child, src, arg, sizeof(arg));
        } else if (strcmp(t, "assignment_pattern") == 0) {
            TSNode left = ts_node_child_by_field_name(child, "left", 4);
            if (!ts_node_is_null(left)){
                node_text(left, src, arg, sizeof(arg));
            }
        } else if (strcmp(t, "rest_element") == 0) {
            TSNode id = ts_node_named_child(child, 0);
            if (!ts_node_is_null(id))
                node_text(id, src, arg, sizeof(arg));
        }

        if (arg[0] != '\0')
            ir_symbol_add_args(sym, arg);
    }
}

// this is main entry point
int js_adapter(const char *fpath, ir_result *ir) {
    uint32_t src_len = 0;
    char *src = read_file(fpath, &src_len);
    if (!src) {
        fprintf(stderr, "Cannot open javascript file: %s\n", fpath);
        return -1;
    }

    TSParser *parser = ts_parser_new();
    ts_parser_set_language(parser, tree_sitter_javascript());
    TSTree *tree = ts_parser_parse_string(parser, NULL, src, src_len);
    TSNode  root = ts_tree_root_node(tree);

    uint32_t err_off;
    TSQueryError err_type;

    static const char func_q[] =
        "[(function_declaration"
        "   name: (identifier) @name"
        "   parameters: (formal_parameters) @params)"
        " (variable_declarator"
        "   name: (identifier) @name"
        "   value: (function_expression"
        "     parameters: (formal_parameters) @params))"
        " (variable_declarator"
        "   name: (identifier) @name"
        "   value: (arrow_function"
        "     parameters: (formal_parameters) @params))]";

    TSQuery *qfunc = ts_query_new(tree_sitter_javascript(), func_q, sizeof(func_q) - 1, &err_off, &err_type);
    if (!qfunc) {
        fprintf(stderr, "Javascript function query error at offset %u (type %d)\n", err_off, (int)err_type);
    } else {
        TSQueryCursor *cur = ts_query_cursor_new();
        ts_query_cursor_exec(cur, qfunc, root);

        TSQueryMatch match;
        while (ts_query_cursor_next_match(cur, &match)) {
            TSNode name_nd   = {0};
            TSNode params_nd = {0};
            int got_name  = 0, got_params = 0;

            for (uint16_t c = 0; c < match.capture_count; c++) {
                uint32_t nlen;
                uint32_t idx = match.captures[c].index;
                const char *cap = ts_query_capture_name_for_id(qfunc, idx, &nlen);
                if (strncmp(cap, "name",   nlen) == 0) {
                    name_nd  = match.captures[c].node; got_name   = 1;
                } else if (strncmp(cap, "params", nlen) == 0) {
                    params_nd = match.captures[c].node; got_params = 1;
                }
            }

            if (got_name) {
                char fname[64] = {0};
                node_text(name_nd, src, fname, sizeof(fname));
                int line = (int)ts_node_start_point(name_nd).row + 1;
                ir_symbol *sym = ir_add_symbol(ir, fname, IR_SYMBOL_FUNCTION, "js", fpath, line);
                if (sym && got_params)
                    collect_params(params_nd, src, sym);
            }
        }
        ts_query_cursor_delete(cur);
        ts_query_delete(qfunc);
    }

    static const char class_q[] =
        "(class_declaration"
        "  name: (identifier) @name)";

    TSQuery *qclass = ts_query_new(tree_sitter_javascript(), class_q, sizeof(class_q) - 1, &err_off, &err_type);
    if (!qclass) {
        fprintf(stderr, "Javascript class query error at offset %u (type %d)\n", err_off, (int)err_type);
    } else {
        TSQueryCursor *cur = ts_query_cursor_new();
        ts_query_cursor_exec(cur, qclass, root);
        TSQueryMatch match;
        while (ts_query_cursor_next_match(cur, &match)) {
            for (uint16_t c = 0; c < match.capture_count; c++) {
                char cname[64] = {0};
                node_text(match.captures[c].node, src, cname, sizeof(cname));
                int line = (int)ts_node_start_point(match.captures[c].node).row + 1;
                ir_add_symbol(ir, cname, IR_SYMBOL_CLASS, "js", fpath, line);
            }
        }
        ts_query_cursor_delete(cur);
        ts_query_delete(qclass);
    }

    static const char obj_q[] =
        "(variable_declarator"
        "  name: (identifier) @name"
        "  value: (object))";

    TSQuery *qobj = ts_query_new(tree_sitter_javascript(), obj_q, sizeof(obj_q) - 1, &err_off, &err_type);
    if (!qobj) {
        fprintf(stderr, "Javascript object query error at offset %u (type %d)\n", err_off, (int)err_type);
    } else {
        TSQueryCursor *cur = ts_query_cursor_new();
        ts_query_cursor_exec(cur, qobj, root);
        TSQueryMatch match;
        while (ts_query_cursor_next_match(cur, &match)) {
            for (uint16_t c = 0; c < match.capture_count; c++) {
                char oname[64] = {0};
                node_text(match.captures[c].node, src, oname, sizeof(oname));
                int line = (int)ts_node_start_point(match.captures[c].node).row + 1;
                ir_add_symbol(ir, oname, IR_SYMBOL_OBJECT, "js", fpath, line);
            }
        }
        ts_query_cursor_delete(cur);
        ts_query_delete(qobj);
    }

    static const char esm_q[] =
        "(import_statement"
        "  source: (string (string_fragment) @module))";

    TSQuery *q_esm = ts_query_new(tree_sitter_javascript(), esm_q, sizeof(esm_q) - 1, &err_off, &err_type);
    if (q_esm) {
        TSQueryCursor *cur = ts_query_cursor_new();
        ts_query_cursor_exec(cur, q_esm, root);
        TSQueryMatch match;
        while (ts_query_cursor_next_match(cur, &match)) {
            for (uint16_t c = 0; c < match.capture_count; c++) {
                char mod[256] = {0};
                node_text(match.captures[c].node, src, mod, sizeof(mod));
                char resolved[256] = {0};
                const char *target = mod;
                if (resolve_module_path(fpath, mod, JS, resolved, sizeof(resolved)))
                    target = resolved;
                ir_add_dependency(ir, fpath, target, "import", "js");
            }
        }
        ts_query_cursor_delete(cur);
        ts_query_delete(q_esm);
    }

    static const char req_q[] =
        "(call_expression"
        "  function: (identifier) @fn"
        "  arguments: (arguments (string (string_fragment) @module)))"; 
    TSQuery *q_req = ts_query_new(tree_sitter_javascript(), req_q, sizeof(req_q) - 1, &err_off, &err_type);
    if (q_req) {
        TSQueryCursor *cur = ts_query_cursor_new();
        ts_query_cursor_exec(cur, q_req, root);
        TSQueryMatch match;
        while (ts_query_cursor_next_match(cur, &match)) {
            int got_fn = 0;
            for (uint16_t c = 0; c < match.capture_count; c++) {
                uint32_t nlen;
                uint32_t idx = match.captures[c].index;
                const char *cap = ts_query_capture_name_for_id(q_req, idx, &nlen);
                if (strncmp(cap, "fn", nlen) == 0) {
                    char fn[32] = {0};
                    node_text(match.captures[c].node, src, fn, sizeof(fn));
                    if (strcmp(fn, "require") == 0)                        got_fn = 1;
                } else if (strncmp(cap, "module", nlen) == 0 && got_fn) {
                    char mod[256] = {0};
                    node_text(match.captures[c].node, src, mod, sizeof(mod));
                    char resolved[256] = {0};
                    const char *target = mod;
                    if (resolve_module_path(fpath, mod, JS, resolved, sizeof(resolved)))
                        target = resolved;
                    ir_add_dependency(ir, fpath, target, "require", "js");
                }
            }
        }
        ts_query_cursor_delete(cur);
        ts_query_delete(q_req);
    }

    ts_tree_delete(tree);
    ts_parser_delete(parser);
    free(src);
    return 0;
}

