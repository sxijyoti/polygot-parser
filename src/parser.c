#include "parser.h"

#include "../ir/ir.h"
#include "../graph/graph.h"
#include "../adapters/adapters.h"
#include "../exporter/mc_export.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <limits.h>

struct polyparser_result {
    ir_result ir;
    grp       graph;
};

static polyparser_result *result_new(void) {
    polyparser_result *res = calloc(1, sizeof(*res));
    if (!res) return NULL;
    ir_init(&res->ir);
    grp_init(&res->graph);
    return res;
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

static const char *canonical_path_or_input(const char *in, char *out, size_t cap) {
    if (!in || !out || cap == 0) return in;
#ifndef _WIN32
    char resolved[PATH_MAX];
    if (realpath(in, resolved) != NULL) {
        safe_copy(out, cap, resolved);
        return out;
    }
#endif
    safe_copy(out, cap, in);
    return out;
}

// C API 

polyparser_result *polyparser_parse_file(const char *fpath) {
    lang_id lang = detect_lang(fpath);
    if (lang == UNSUPPORTED_LANG) {
        fprintf(stderr, "Unsupported extension: %s\n", fpath);
        return NULL;
    }
    polyparser_result *res = result_new();
    if (!res) return NULL;

    char canonical_fpath[PATH_MAX];
    const char *input_path = canonical_path_or_input(fpath, canonical_fpath, sizeof(canonical_fpath));

    if (lang_adapter(input_path, lang, &res->ir) != 0) {
        free(res);
        return NULL;
    }
    grp_builder(&res->graph, &res->ir);
    return res;
}

polyparser_result *polyparser_parse_files(const char **fpaths, int count){
    if (!fpaths || count <= 0) return NULL;

    polyparser_result *res = result_new();
    if (!res) return NULL;

    for (int i = 0; i < count; i++) {
        const char *fp = fpaths[i];
        char canonical_fp[PATH_MAX];
        const char *input_path = canonical_path_or_input(fp, canonical_fp, sizeof(canonical_fp));
        lang_id lang = detect_lang(fp);
        if (lang == UNSUPPORTED_LANG) {
            fprintf(stderr, "Unsupported extension: %s\n", fp);
            continue;
        }
        if (lang_adapter(input_path, lang, &res->ir) != 0)
            fprintf(stderr, "Failed to parse: %s\n", fp);
    }

    grp_builder(&res->graph, &res->ir);
    return res;
}


polyparser_result *polyparser_parse_dir(const char *dirpath) {
    polyparser_result *res = result_new();
    if (!res) return NULL;

DIR *dir = opendir(dirpath);
if (!dir) {
        fprintf(stderr, "Cannot open directory: %s\n", dirpath);
        free(res); return NULL;
    }
struct dirent *ent;
while ((ent = readdir(dir)) != NULL) {
    if (ent->d_name[0] == '.') continue;
    char fpath[512];
    snprintf(fpath, sizeof(fpath), "%s/%s", dirpath, ent->d_name);

    struct stat st;
    if (stat(fpath, &st) != 0) continue;

    if (S_ISDIR(st.st_mode)) {
        // recurse into subdirectories
        polyparser_result *sub = polyparser_parse_dir(fpath);
        if (sub) {
            // merge IR and deps
            for (int i = 0; i < sub->ir.symbol_count && res->ir.symbol_count < IR_MAX_SYMBOLS; i++)
                res->ir.symbols[res->ir.symbol_count++] = sub->ir.symbols[i];
            for (int i = 0; i < sub->ir.dep_count && res->ir.dep_count < IR_MAX_DEPS; i++)
                res->ir.deps[res->ir.dep_count++] = sub->ir.deps[i];
            polyparser_free_res(sub);
        }
        continue;
    }

    lang_id lang = detect_lang(fpath);
    if (lang != UNSUPPORTED_LANG) {
        char canonical_fp[PATH_MAX];
        const char *input_path = canonical_path_or_input(fpath, canonical_fp, sizeof(canonical_fp));
        lang_adapter(input_path, lang, &res->ir);
    }
    }
closedir(dir);

grp_builder(&res->graph, &res->ir);
return res;
}

char *polyparser_to_json(const polyparser_result *res) {
    if (!res) return NULL;
    return mc_export_json(&res->ir, &res->graph);
}

void polyparser_free_json(char *json) {
    free(json);
}

void polyparser_free_res(polyparser_result *res) {
    free(res);
}
