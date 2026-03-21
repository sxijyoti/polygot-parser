/*
 * adapters/adapters.c
 *
 * Language detection from file extension and dispatch to the correct
 * per-language adapter.  The per-language adapters (python_adapter.c,
 * js_adapter.c, ruby_adapter.c) include their own file-reading helpers
 * so this file stays minimal.
 */
#include "adapters.h"
#include <string.h>
#include <stdio.h>

/* Return a pointer to the file extension, or NULL if none. */
static const char *file_ext(const char *path) {
    const char *dot = NULL;
    for (const char *p = path; *p; p++)
        if (*p == '.') dot = p;
    return (dot && dot[1]) ? dot + 1 : NULL;
}

lang_id adapter_detect_lang(const char *filepath) {
    const char *ext = file_ext(filepath);
    if (!ext) return LANG_UNKNOWN;
    if (strcmp(ext, "py")  == 0) return LANG_PYTHON;
    if (strcmp(ext, "js")  == 0) return LANG_JS;
    if (strcmp(ext, "mjs") == 0) return LANG_JS;
    if (strcmp(ext, "cjs") == 0) return LANG_JS;
    if (strcmp(ext, "rb")  == 0) return LANG_RUBY;
    return LANG_UNKNOWN;
}

int adapter_run(const char *filepath, lang_id lang, ir_result *ir) {
    switch (lang) {
    case LANG_PYTHON: return python_adapt(filepath, ir);
    case LANG_JS:     return js_adapt    (filepath, ir);
    case LANG_RUBY:   return ruby_adapt  (filepath, ir);
    default:
        fprintf(stderr, "[adapter] no adapter for lang %d\n", (int)lang);
        return -1;
    }
}
