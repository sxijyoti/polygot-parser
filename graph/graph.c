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
    for (int i = 0; i < ir->dep_count; i++) {
        if (g->edge_count >= GRAPH_MAX_EDGES) {
            fprintf(stderr, "Dependency graph edge limit reached\n");
            return;
        }
        ir_dep *dep = &ir->deps[i];
        grp_edge *edg = &g->edges[g->edge_count++];
        strncpy(edg->from, dep->from_file, sizeof(edg->from)-1);
        strncpy(edg->to, dep->module, sizeof(edg->to)-1);
        strncpy(edg->type, dep->type, sizeof(edg->type)-1);
        strncpy(edg->lang, dep->lang, sizeof(edg->lang)-1);
    }
    printf("Dependency graph built with %d edges\n", g->edge_count);
}

void grp_print(const grp *g) {
    printf("Dependency Graph Edges:\n");
    for (int i = 0; i < g->edge_count; i++) {
        grp_edge *edg = &g->edges[i];
        printf("%s --(%s:%s)--> %s\n", edg->from, edg->lang, edg->type, edg->to);
    }
}