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

void grp_builder(grp *g, const ir_result *ir) {
    grp_init(g);
    // dependencies (imports/requires)
    for (int i = 0; i < ir->dep_count; i++) {
        if (g->edge_count >= GRAPH_MAX_EDGES) {
            fprintf(stderr, "Dependency graph edge limit reached\n");
            return;
        }
        const ir_dep *dep = &ir->deps[i];
        grp_edge *edg = &g->edges[g->edge_count++];
        strncpy(edg->from, dep->from_file, sizeof(edg->from)-1);
        strncpy(edg->to, dep->module, sizeof(edg->to)-1);
        edg->from_kind = GRP_NODE_FILE;
        edg->to_kind   = GRP_NODE_MODULE;
        edg->rel_kind  = (strcmp(dep->type, "require") == 0) ? GRP_EDGE_REQUIRE : GRP_EDGE_IMPORT;
        strncpy(edg->lang, dep->lang, sizeof(edg->lang)-1);
    }

    // definitions
    for (int i = 0; i < ir->symbol_count && g->edge_count < GRAPH_MAX_EDGES; i++) {
        const ir_symbol *sym = &ir->symbols[i];
        grp_edge *def = &g->edges[g->edge_count++];
        strncpy(def->from, sym->file, sizeof(def->from)-1);
        strncpy(def->to, sym->name, sizeof(def->to)-1);
        def->from_kind = GRP_NODE_FILE;
        def->to_kind   = GRP_NODE_SYMBOL;
        def->rel_kind  = GRP_EDGE_DEFINE;
        strncpy(def->lang, sym->lang, sizeof(def->lang)-1);

        if (sym->is_exported && g->edge_count < GRAPH_MAX_EDGES) {
            grp_edge *exp = &g->edges[g->edge_count++];
            strncpy(exp->from, sym->file, sizeof(exp->from)-1);
            strncpy(exp->to, sym->name, sizeof(exp->to)-1);
            exp->from_kind = GRP_NODE_FILE;
            exp->to_kind   = GRP_NODE_SYMBOL;
            exp->rel_kind  = GRP_EDGE_EXPORT;
            strncpy(exp->lang, sym->lang, sizeof(exp->lang)-1);
        }
    }

    printf("Graph built with %d edges\n", g->edge_count);
}

void grp_print(const grp *g) {
    printf("Dependency Graph Edges:\n");
    for (int i = 0; i < g->edge_count; i++) {
        grp_edge *edg = &g->edges[i];
        printf("%s --(%d:%s)--> %s\n", edg->from, (int)edg->rel_kind, edg->lang, edg->to);
    }
}