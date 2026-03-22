#include "adapters.h"
#include <string.h>
#include <stdio.h>
#include <tree_sitter/api.h>
#include "../ir/ir.h"

extern const TSLanguage *tree_sitter_ruby(void);

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
        } else if (strcmp(t, "optional_parameter") == 0 ||
                   strcmp(t, "splat_parameter")    == 0 ||
                   strcmp(t, "block_parameter")    == 0) {
            TSNode id = ts_node_named_child(child, 0);
            if (!ts_node_is_null(id) &&
                strcmp(ts_node_type(id), "identifier") == 0)
                node_text(id, src, arg, sizeof(arg));
        }

        if (arg[0] != '\0')
            ir_symbol_add_args(sym, arg);
    }
}

int rb_adapter(const char *fpath, ir_result *ir) {
    uint32_t src_len = 0;
    char *src = read_file(fpath, &src_len);
    if (!src) {
        fprintf(stderr, "Cannot open ruby file: %s\n", fpath);
        return -1;
    }

    TSParser *parser = ts_parser_new();
    ts_parser_set_language(parser, tree_sitter_ruby());
    TSTree *tree = ts_parser_parse_string(parser, NULL, src, src_len);
    TSNode  root = ts_tree_root_node(tree);

    uint32_t err_off;
    TSQueryError err_type;

    static const char func_q[] =
        "(method"
        "  name: (identifier) @name) @def";

    TSQuery *qfunc = ts_query_new(tree_sitter_ruby(), func_q, sizeof(func_q) - 1, &err_off, &err_type);
    if (!qfunc) {
        fprintf(stderr, "Ruby function query error at offset %u (type %d)\n", err_off, (int)err_type);
    } else {
        TSQueryCursor *cur = ts_query_cursor_new();
        ts_query_cursor_exec(cur, qfunc, root);

        TSQueryMatch match;
        while (ts_query_cursor_next_match(cur, &match)) {
            TSNode name_nd   = {0};
            TSNode def_nd    = {0};
            TSNode params_nd = {0};
            int got_name = 0, got_def = 0;

            for (uint16_t c = 0; c < match.capture_count; c++) {
                uint32_t nlen;
                uint32_t idx = match.captures[c].index;
                const char *cap = ts_query_capture_name_for_id(qfunc, idx, &nlen);
                if (strncmp(cap, "name", nlen) == 0) {
                    name_nd = match.captures[c].node;
                    got_name = 1;
                } else if (strncmp(cap, "def", nlen) == 0) {
                    def_nd = match.captures[c].node;
                    got_def = 1;
                }
            }

            if (got_name) {
                char fname[64] = {0};
                node_text(name_nd, src, fname, sizeof(fname));
                int line = (int)ts_node_start_point(name_nd).row + 1;
                ir_symbol *sym = ir_add_symbol(ir, fname, "rb", fpath, line);
                if (got_def) {
                    params_nd = ts_node_child_by_field_name(def_nd, "parameters", 10);
                }
                if (sym && !ts_node_is_null(params_nd))
                    collect_params(params_nd, src, sym);
            }
        }
        ts_query_cursor_delete(cur);
        ts_query_delete(qfunc);
    }

    static const char req_q[] =
        "(call"
        "  method: (identifier) @fn"
        "  arguments: (argument_list"
        "    (string (string_content) @module)))";

    TSQuery *q_req = ts_query_new(tree_sitter_ruby(), req_q, sizeof(req_q) - 1, &err_off, &err_type);
    if (q_req) {
        TSQueryCursor *cur = ts_query_cursor_new();
        ts_query_cursor_exec(cur, q_req, root);
        TSQueryMatch match;
        while (ts_query_cursor_next_match(cur, &match)) {
            char fn_name[32]   = {0};
            char mod_name[256] = {0};
            int  got_fn = 0, got_mod = 0;

            for (uint16_t c = 0; c < match.capture_count; c++) {
                uint32_t    nlen;
                uint32_t    idx = match.captures[c].index;
                const char *cap = ts_query_capture_name_for_id(q_req, idx, &nlen);
                if (strncmp(cap, "fn", nlen) == 0) {
                    node_text(match.captures[c].node, src, fn_name, sizeof(fn_name));
                    got_fn = 1;
                } else if (strncmp(cap, "module", nlen) == 0) {
                    node_text(match.captures[c].node, src, mod_name, sizeof(mod_name));
                    got_mod = 1;
                }
            }

            if (got_fn && got_mod &&
                (strcmp(fn_name, "require")          == 0 ||
                 strcmp(fn_name, "require_relative")  == 0)) {
                ir_add_dependency(ir, fpath, mod_name, "require", "rb");
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

