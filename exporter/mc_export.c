#include "mc_export.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

typedef struct sbuf {
    char *buf;
    size_t len;
    size_t cap;
} sbuf;

static int sb_reserve(sbuf *s, size_t extra) {
    if (!s) return 0;
    size_t need = s->len + extra + 1;
    if (need <= s->cap) return 1;

    size_t ncap = (s->cap == 0) ? 256 : s->cap;
    while (ncap < need) ncap *= 2;

    char *p = (char *)realloc(s->buf, ncap);
    if (!p) return 0;
    s->buf = p;
    s->cap = ncap;
    return 1;
}

static int sb_putc(sbuf *s, char c) {
    if (!sb_reserve(s, 1)) return 0;
    s->buf[s->len++] = c;
    s->buf[s->len] = '\0';
    return 1;
}

static int sb_puts(sbuf *s, const char *txt) {
    if (!txt) txt = "";
    size_t n = strlen(txt);
    if (!sb_reserve(s, n)) return 0;
    memcpy(s->buf + s->len, txt, n);
    s->len += n;
    s->buf[s->len] = '\0';
    return 1;
}

static int sb_printf(sbuf *s, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    va_list ap2;
    va_copy(ap2, ap);
    int n = vsnprintf(NULL, 0, fmt, ap2);
    va_end(ap2);

    if (n < 0) {
        va_end(ap);
        return 0;
    }
    if (!sb_reserve(s, (size_t)n)) {
        va_end(ap);
        return 0;
    }

    int w = vsnprintf(s->buf + s->len, s->cap - s->len, fmt, ap);
    va_end(ap);
    if (w < 0) return 0;

    s->len += (size_t)w;
    return 1;
}

static int json_esp_append(sbuf *s, const char *str) {
    if (!sb_putc(s, '"')) return 0;
    if (!str) str = "";

    for (const unsigned char *p = (const unsigned char *)str; *p; ++p) {
        unsigned char c = *p;
        switch (c) {
            case '\"': if (!sb_puts(s, "\\\"")) return 0; break;
            case '\\': if (!sb_puts(s, "\\\\")) return 0; break;
            case '\b': if (!sb_puts(s, "\\b")) return 0; break;
            case '\f': if (!sb_puts(s, "\\f")) return 0; break;
            case '\n': if (!sb_puts(s, "\\n")) return 0; break;
            case '\r': if (!sb_puts(s, "\\r")) return 0; break;
            case '\t': if (!sb_puts(s, "\\t")) return 0; break;
            default:
                if (c < 0x20) {
                    if (!sb_printf(s, "\\u%04x", (unsigned)c)) return 0;
                } else {
                    if (!sb_putc(s, (char)c)) return 0;
                }
        }
    }

    return sb_putc(s, '"');
}

static int sb_indent(sbuf *s, int level) {
    for (int i = 0; i < level; ++i) {
        if (!sb_puts(s, "  ")) return 0;
    }
    return 1;
}

static int sb_newline_indent(sbuf *s, int level) {
    if (!sb_putc(s, '\n')) return 0;
    return sb_indent(s, level);
}

static int emit_symbols_for_kind(sbuf *s, const ir_result *ir, const char *lang, ir_symbol_kind kind, const char *label, int indent) {
    if (!sb_putc(s, '"')) return 0;
    if (!sb_puts(s, label)) return 0;
    if (!sb_puts(s, "\": [")) return 0;

    int first_symbol = 1;
    for (size_t i = 0; i < ir->symbol_count; ++i) {
        const ir_symbol *sym = &ir->symbols[i];
        if (strcmp(sym->lang, lang) != 0 || sym->kind != kind) continue;

        if (!first_symbol && !sb_putc(s, ',')) return 0;
        if (!sb_newline_indent(s, indent + 1)) return 0;
        first_symbol = 0;

        if (!sb_putc(s, '{')) return 0;
        if (!sb_puts(s, "\"name\": ")) return 0;
        if (!json_esp_append(s, sym->name)) return 0;
        if (!sb_puts(s, ", \"exported\": ")) return 0;
        if (!sb_puts(s, sym->is_exported ? "true" : "false")) return 0;
        if (!sb_puts(s, ", \"args\": [")) return 0;

        for (int a = 0; a < sym->args_count; ++a) {
            if (a > 0 && !sb_puts(s, ", ")) return 0;
            if (!json_esp_append(s, sym->args[a])) return 0;
        }

        if (!sb_puts(s, "]}")) return 0;
    }

    if (!first_symbol && !sb_newline_indent(s, indent)) return 0;
    return sb_putc(s, ']');
}

static int emit_symbol_groups_for_lang(sbuf *s, const ir_result *ir, const char *lang, int indent) {
    if (!emit_symbols_for_kind(s, ir, lang, IR_SYMBOL_FUNCTION, "functions", indent)) return 0;
    if (!sb_putc(s, ',')) return 0;
    if (!sb_newline_indent(s, indent)) return 0;
    if (!emit_symbols_for_kind(s, ir, lang, IR_SYMBOL_CLASS, "classes", indent)) return 0;
    if (!sb_putc(s, ',')) return 0;
    if (!sb_newline_indent(s, indent)) return 0;
    if (!emit_symbols_for_kind(s, ir, lang, IR_SYMBOL_OBJECT, "objects", indent)) return 0;
    return 1;
}

static int lang_seen(char **langs, size_t n, const char *lang) {
    for (size_t i = 0; i < n; ++i) {
        if (strcmp(langs[i], lang) == 0) return 1;
    }
    return 0;
}

static int emit_lang_object(sbuf *s, const ir_result *ir, int indent) {
    if (!sb_puts(s, "\"languages\": {")) return 0;

    char **langs = NULL;
    size_t nlangs = 0;
    size_t cap = 0;

    for (size_t i = 0; i < ir->symbol_count; ++i) {
        const char *lang = ir->symbols[i].lang;
        if (!lang || !*lang) continue;
        if (lang_seen(langs, nlangs, lang)) continue;

        if (nlangs == cap) {
            size_t ncap = (cap == 0) ? 4 : cap * 2;
            char **tmp = (char **)realloc(langs, ncap * sizeof(char *));
            if (!tmp) {
                for (size_t k = 0; k < nlangs; ++k) free(langs[k]);
                free(langs);
                return 0;
            }
            langs = tmp;
            cap = ncap;
        }

        langs[nlangs] = strdup(lang);
        if (!langs[nlangs]) {
            for (size_t k = 0; k < nlangs; ++k) free(langs[k]);
            free(langs);
            return 0;
        }
        nlangs++;
    }

    for (size_t i = 0; i < nlangs; ++i) {
        if (i > 0 && !sb_putc(s, ',')) goto fail;
        if (!sb_newline_indent(s, indent + 1)) goto fail;

        if (!json_esp_append(s, langs[i])) goto fail;
        if (!sb_puts(s, ": {")) goto fail;
        if (!sb_newline_indent(s, indent + 2)) goto fail;
        if (!emit_symbol_groups_for_lang(s, ir, langs[i], indent + 2)) goto fail;
        if (!sb_newline_indent(s, indent + 1)) goto fail;
        if (!sb_putc(s, '}')) goto fail;
    }

    for (size_t i = 0; i < nlangs; ++i) free(langs[i]);
    free(langs);

    if (nlangs > 0 && !sb_newline_indent(s, indent)) return 0;
    return sb_putc(s, '}');

fail:
    for (size_t i = 0; i < nlangs; ++i) free(langs[i]);
    free(langs);
    return 0;
}

static const char *edge_kind_str(grp_edge_kind k) {
    switch (k) {
    case GRP_EDGE_IMPORT:  return "import";
    case GRP_EDGE_REQUIRE: return "require";
    case GRP_EDGE_DEFINE:  return "define";
    case GRP_EDGE_EXPORT:  return "export";
    case GRP_EDGE_MEMBER_OF: return "member_of";
    default: return "unknown";
    }
}

static const char *node_kind_str(grp_node_kind k) {
    switch (k) {
    case GRP_NODE_FILE:   return "file";
    case GRP_NODE_SYMBOL: return "symbol";
    case GRP_NODE_MODULE: return "module";
    default: return "unknown";
    }
}

static int emit_graph_object(sbuf *s, const grp *g, int indent) {
    if (!sb_puts(s, "\"graph\": {")) return 0;
    if (!sb_newline_indent(s, indent + 1)) return 0;
    if (!sb_puts(s, "\"edges\": [")) return 0;

    if (g) {
        for (size_t i = 0; i < g->edge_count; ++i) {
            if (i > 0 && !sb_putc(s, ',')) return 0;
            if (!sb_newline_indent(s, indent + 2)) return 0;

            if (!sb_putc(s, '{')) return 0;
            if (!sb_puts(s, "\"from\": ")) return 0;
            if (!json_esp_append(s, g->edges[i].from)) return 0;
            if (!sb_puts(s, ", \"from_kind\": ")) return 0;
            if (!json_esp_append(s, node_kind_str(g->edges[i].from_kind))) return 0;
            if (!sb_puts(s, ", \"to\": ")) return 0;
            if (!json_esp_append(s, g->edges[i].to)) return 0;
            if (!sb_puts(s, ", \"to_kind\": ")) return 0;
            if (!json_esp_append(s, node_kind_str(g->edges[i].to_kind))) return 0;
            if (!sb_puts(s, ", \"type\": ")) return 0;
            if (!json_esp_append(s, edge_kind_str(g->edges[i].rel_kind))) return 0;
            if (!sb_puts(s, ", \"lang\": ")) return 0;
            if (!json_esp_append(s, g->edges[i].lang)) return 0;
            if (!sb_putc(s, '}')) return 0;
        }
    }

    if (g && g->edge_count > 0 && !sb_newline_indent(s, indent + 1)) return 0;
    if (!sb_puts(s, "]")) return 0;
    if (!sb_newline_indent(s, indent)) return 0;
    if (!sb_putc(s, '}')) return 0;
    return 1;
}

char *mc_export_json(const ir_result *ir, const grp *g) {
    if (!ir) return NULL;

    sbuf s = {0};

    if (!sb_putc(&s, '{')) goto fail;
    if (!sb_newline_indent(&s, 1)) goto fail;
    if (!emit_lang_object(&s, ir, 1)) goto fail;
    if (!sb_putc(&s, ',')) goto fail;
    if (!sb_newline_indent(&s, 1)) goto fail;
    if (!emit_graph_object(&s, g, 1)) goto fail;
    if (!sb_newline_indent(&s, 0)) goto fail;
    if (!sb_putc(&s, '}')) goto fail;

    return s.buf;

fail:
    free(s.buf);
    return NULL;
}

void mc_export_json_fp(const ir_result *ir, const grp *g, FILE *fp) {
    if (!fp) return;
    char *json = mc_export_json(ir, g);
    if (!json) return;
    fputs(json, fp);
    free(json);
}