#include "adapters.h"

#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <limits.h>
#include <stdlib.h>

static int file_exists(const char *path) {
    struct stat st;
    return path && stat(path, &st) == 0 && S_ISREG(st.st_mode);
}

static void safe_copy(char *dst, size_t cap, const char *src) {
    if (!dst || cap == 0) return;
    if (!src) {
        dst[0] = '\0';
        return;
    }
    strncpy(dst, src, cap - 1);
    dst[cap - 1] = '\0';
}

static void canonicalize_if_exists(char *path, size_t cap) {
    if (!path || path[0] == '\0') return;
#ifndef _WIN32
    char resolved[PATH_MAX];
    if (realpath(path, resolved) != NULL) {
        safe_copy(path, cap, resolved);
    }
#endif
}

static int try_with_ext(const char *base, const char *ext, char *out, size_t cap) {
    if (!base || !ext) return 0;
    if (snprintf(out, cap, "%s%s", base, ext) >= (int)cap) return 0;
    return file_exists(out);
}

static int resolve_js(const char *base_dir, const char *module, char *out, size_t cap) {
    if (!module) return 0;
    const char *exts[] = {".js", ".mjs", ".cjs"};
    char candidate[512];

    if (snprintf(candidate, sizeof(candidate), "%s/%s", base_dir, module) >= (int)sizeof(candidate))
        return 0;

    if (file_exists(candidate)) {
        strncpy(out, candidate, cap - 1); out[cap-1] = '\0';
        return 1;
    }

    // try extensions
    for (size_t i = 0; i < sizeof(exts)/sizeof(exts[0]); i++) {
        if (try_with_ext(candidate, exts[i], out, cap)) return 1;
    }

    // if module points to dir then try index.*
    struct stat st;
    if (stat(candidate, &st) == 0 && S_ISDIR(st.st_mode)) {
        for (size_t i = 0; i < sizeof(exts)/sizeof(exts[0]); i++) {
            if (snprintf(out, cap, "%s/index%s", candidate, exts[i]) < (int)cap && file_exists(out))
                return 1;
        }
    }
    return 0;
}

static int resolve_py(const char *base_dir, const char *module, char *out, size_t cap) {
    if (!module) return 0;
    char path[512];
    // dotted to path
    snprintf(path, sizeof(path), "%s", module);
    for (char *p = path; *p; ++p) if (*p == '.') *p = '/';
    char candidate[512];
    if (snprintf(candidate, sizeof(candidate), "%s/%s.py", base_dir, path) >= (int)sizeof(candidate))
        return 0;
    if (file_exists(candidate)) {
        strncpy(out, candidate, cap - 1); out[cap-1] = '\0';
        return 1;
    }
    return 0;
}

static int resolve_rb(const char *base_dir, const char *module, char *out, size_t cap) {
    if (!module) return 0;
    char candidate[512];
    if (snprintf(candidate, sizeof(candidate), "%s/%s", base_dir, module) >= (int)sizeof(candidate))
        return 0;
    if (file_exists(candidate)) {
        strncpy(out, candidate, cap - 1); out[cap-1] = '\0';
        return 1;
    }
    if (try_with_ext(candidate, ".rb", out, cap)) return 1;
    return 0;
}

int resolve_module_path(const char *from_file, const char *module, lang_id lang, char *out, size_t out_cap) {
    if (!from_file || !module || !out || out_cap == 0) return 0;
    const char *slash = strrchr(from_file, '/');
    char base_dir[512];
    if (slash) {
        size_t n = (size_t)(slash - from_file);
        if (n >= sizeof(base_dir)) n = sizeof(base_dir) - 1;
        memcpy(base_dir, from_file, n);
        base_dir[n] = '\0';
    } else {
        strcpy(base_dir, ".");
    }

    // only attempt resolution for relative like modules
    int is_relative = (module[0] == '.' || strchr(module, '/') != NULL);
    if (!is_relative) return 0;

    int check = 0;
    switch (lang) {
    case JS:
        check = resolve_js(base_dir, module, out, out_cap);
        break;
    case PYTHON:
        check = resolve_py(base_dir, module, out, out_cap);
        break;
    case RUBY:
        check = resolve_rb(base_dir, module, out, out_cap);
        break;
    default:
        check = 0;
        break;
    }

    if (check) {
        canonicalize_if_exists(out, out_cap);
    }

    return check;
}

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