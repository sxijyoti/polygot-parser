#ifndef MC_EXPORT_H
#define MC_EXPORT_H

#include "../ir/ir.h"
#include "../graph/graph.h"
#include <stdio.h>


char *mc_export_json(const ir_result *ir, const grp *g);
void mc_export_json_fp(const ir_result *ir, const grp *g, FILE *fp);

#endif