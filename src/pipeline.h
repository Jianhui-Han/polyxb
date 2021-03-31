#ifndef PIPELINE_H
#define PIPELINE_H

#include <isl/ctx.h>
#include <isl/schedule_node.h>

#include "info.h"

void do_allocate(__isl_keep isl_schedule_node *node,
    struct ast_info **infos, void *user);

__isl_give isl_multi_union_pw_aff *build_pipeline_partial(
    __isl_keep isl_schedule_node *node, void *user);

#endif