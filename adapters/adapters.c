#include "adapters.h"

#include <string.h>
#include <stdio.h>

static const char *file_ext(const char *path) {
    const char *dot = NULL;
    for (const char *p = path; *p; p++)
        if (*p == '.') dot = p;
    return (dot && dot[1]) ? dot + 1 : NULL;
} // to get the .extension

lang_id detect_lang(const char *filepath) {
    const char *ext = file_ext(filepath);
    if (!ext) return UNSUPPORTED_LANG;
    if(strcmp(ext, "py")  == 0) return PYTHON;
    if(strcmp(ext, "js")  == 0) return JS;
    if(strcmp(ext, "mjs") == 0) return JS; // other formats
    if(strcmp(ext, "cjs") == 0) return JS; 
    if(strcmp(ext, "rb")  == 0) return RUBY;
    return UNSUPPORTED_LANG;
}

int lang_adapter(const char *filepath, lang_id lang, ir_result *ir) {
    switch (lang) {
    case PYTHON: return py_adapter(filepath, ir);
    case JS:     return js_adapter(filepath, ir);
    case RUBY:   return rb_adapter(filepath, ir);
    default:
        fprintf(stderr, "No adapter for the unsupported language %d\n", (int)lang);
        return -1;
    }
}