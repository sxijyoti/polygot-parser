#ifndef GRAPH_H
#define GRAPH_H

#include "../ir/ir.h"

#define GRAPH_MAX_EDGES 2048

typedef enum {
    GRP_NODE_FILE = 0,
    GRP_NODE_SYMBOL = 1,
    GRP_NODE_MODULE = 2
} grp_node_kind;

typedef enum {
    GRP_EDGE_IMPORT = 0,
    GRP_EDGE_REQUIRE = 1,
    GRP_EDGE_DEFINE = 2,
    GRP_EDGE_EXPORT = 3
} grp_edge_kind;

typedef struct {
    char from[256];
    char to[256];
    grp_node_kind from_kind;
    grp_node_kind to_kind;
    grp_edge_kind rel_kind;
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