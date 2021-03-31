#ifndef NODE_H
#define NODE_H

#include <isl/ctx.h>
#include <isl/set.h>
#include <isl/schedule_node.h>

#include "info.h"

#define MAX_NODES 32

int node_match_structure(__isl_keep isl_schedule_node *node, const char *desc);
__isl_give isl_schedule_node *node_get_subnode(
    __isl_keep isl_schedule_node *node, const char *desc, int pos);

__isl_give isl_basic_set *node_band_get_domain(
    __isl_keep isl_schedule_node *node);
__isl_give isl_basic_set *node_filter_get_domain(
    __isl_keep isl_schedule_node *node);

int node_match_structure_pipeline(__isl_keep isl_schedule_node *node,
    struct ast_info ***infos);

#endif