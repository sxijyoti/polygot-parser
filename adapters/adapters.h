#ifndef ADAPTERS_H
#define ADAPTERS_H

#include <tree_sitter/api.h>
#include "../ir/ir.h"
#include "../graph/graph.h"

typedef enum {
    PYTHON = 0,
    JS = 1,
    RUBY = 2,
    UNSUPPORTED_LANG = -1
} lang_id;

lang_id detect_lang(const char *fpath);
int lang_adapter(const char *fpath, lang_id lang, ir_result *ir);

int py_adapter(const char *fpath, ir_result *ir);
int js_adapter(const char *fpath, ir_result *ir);
int rb_adapter(const char *fpath, ir_result *ir);

int resolve_module_path(const char *from_file, const char *module, lang_id lang, char *out, size_t out_cap);


#endif