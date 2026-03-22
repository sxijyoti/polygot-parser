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

// C API 

polyparser_result *polyparser_parse_file(const char *fpath) {
    lang_id lang = detect_lang(fpath);
    if (lang == UNSUPPORTED_LANG) {
        fprintf(stderr, "Unsupported extension: %s\n", fpath);
        return NULL;
    }
    polyparser_result *res = result_new();
    if (!res) return NULL;

    if (lang_adapter(fpath, lang, &res->ir) != 0) {
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
        lang_id lang = detect_lang(fp);
        if (lang == UNSUPPORTED_LANG) {
            fprintf(stderr, "Unsupported extension: %s\n", fp);
            continue;
        }
        if (lang_adapter(fp, lang, &res->ir) != 0)
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
    if (stat(fpath, &st) == 0 && S_ISDIR(st.st_mode)) continue;

    lang_id lang = detect_lang(fpath);
    if (lang != UNSUPPORTED_LANG)
        lang_adapter(fpath, lang, &res->ir);
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
