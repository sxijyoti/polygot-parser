#ifndef GRAPH_H
#define GRAPH_H

#include "../ir/ir.h"

#define GRAPH_MAX_EDGES 1024

typedef struct {
    char from[256];
    char to[256];
    char type[24];
    char lang[8];
} grp_edge;

typedef struct {
    grp_edge edges[GRAPH_MAX_EDGES];
    int      edge_count;
} grp;

void grp_init(grp *g);
void grp_builder(grp *g, const ir_result *ir);
void grp_print(const grp *g);

#endif