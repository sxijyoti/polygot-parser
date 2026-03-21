#ifndef ADAPTERS_H
#define ADAPTERS_H

#include <tree_sitter/api.h>
#include "ir/ir.h"
#include "graph/graph.h"

typedef enum {
    PYTHON = 0,
    JS = 1,
    RUBY = 2,
    UNSUPPORTED_LANG = -1
} lang_id;

char *adp_read_file(const char *path, unsigned int *len_out);
void adp_node_text(const void *node_ptr, const char *src, char *out, unsigned int out_len);

lang_id detect_lang(const char *fpath);
int lang_adapter(const char *fpath, lang_id lang, ir_result *ir);

int py_adapter(const char *fpath, ir_result *ir);
int js_adapter(const char *fpath, ir_result *ir);
int rb_adapter(const char *fpath, ir_result *ir);


#endif