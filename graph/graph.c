#include "graph.h"
#include "../ir/ir.h"
#include <stdio.h>
#include <string.h>

/*
void grp_init(grp *g);
void grp_builder(grp *g, const ir_result *ir);
void grp_print(grp *g);
*/

void grp_init(grp *g) {
    g->edge_count = 0;
}

static void edge_copy(char *dst, size_t cap, const char *src) {
    if (!dst || cap == 0) return;
    if (!src) {
        dst[0] = '\0';
        return;
    }
    strncpy(dst, src, cap - 1);
    dst[cap - 1] = '\0';
}

static void grp_add_edge(grp *g, const char *from, grp_node_kind from_kind, const char *to, grp_node_kind to_kind, grp_edge_kind rel_kind, const char *lang) {
    if (g->edge_count >= GRAPH_MAX_EDGES) return;
    grp_edge *edge = &g->edges[g->edge_count++];
    memset(edge, 0, sizeof(*edge));
    edge_copy(edge->from, sizeof(edge->from), from);
    edge_copy(edge->to, sizeof(edge->to), to);
    edge->from_kind = from_kind;
    edge->to_kind = to_kind;
    edge->rel_kind = rel_kind;
    edge_copy(edge->lang, sizeof(edge->lang), lang);
}

void grp_builder(grp *g, const ir_result *ir) {
    grp_init(g);
    // dependencies (imports/requires)
    for (int i = 0; i < ir->dep_count; i++) {
        if (g->edge_count >= GRAPH_MAX_EDGES) {
            fprintf(stderr, "Dependency graph edge limit reached\n");
            return;
        }
        const ir_dep *dep = &ir->deps[i];
        grp_add_edge(
            g,
            dep->from_file,
            GRP_NODE_FILE,
            dep->module,
            GRP_NODE_MODULE,
            (strcmp(dep->type, "require") == 0) ? GRP_EDGE_REQUIRE : GRP_EDGE_IMPORT,
            dep->lang
        );
    }

    // definitions
    for (int i = 0; i < ir->symbol_count && g->edge_count < GRAPH_MAX_EDGES; i++) {
        const ir_symbol *sym = &ir->symbols[i];
        grp_add_edge(g, sym->file, GRP_NODE_FILE, sym->name, GRP_NODE_SYMBOL, GRP_EDGE_DEFINE, sym->lang);

        if (sym->is_exported && g->edge_count < GRAPH_MAX_EDGES) {
            grp_add_edge(g, sym->file, GRP_NODE_FILE, sym->name, GRP_NODE_SYMBOL, GRP_EDGE_EXPORT, sym->lang);
        }

        if (sym->kind == IR_SYMBOL_FUNCTION && sym->owner[0] != '\0' && g->edge_count < GRAPH_MAX_EDGES) {
            grp_add_edge(g, sym->name, GRP_NODE_SYMBOL, sym->owner, GRP_NODE_SYMBOL, GRP_EDGE_MEMBER_OF, sym->lang);
        }
    }

    printf("Graph built with %d edges\n", g->edge_count);
}

void grp_print(const grp *g) {
    printf("Dependency Graph Edges:\n");
    for (int i = 0; i < g->edge_count; i++) {
        const grp_edge *edg = &g->edges[i];
        printf("%s --(%d:%s)--> %s\n", edg->from, (int)edg->rel_kind, edg->lang, edg->to);
    }
}