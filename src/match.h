#ifndef MATCH_H
#define MATCH_H

#include <isl/ctx.h>
#include <isl/schedule.h>
#include <isl/schedule_node.h>

#include "scop.h"

__isl_give isl_schedule_node *match_mv_bare(
    __isl_take isl_schedule_node *node, void *user);
__isl_give isl_schedule_node *match_mv_init(
    __isl_take isl_schedule_node *node, void *user);
__isl_give isl_schedule_node *match_mm_bare(
    __isl_take isl_schedule_node *node, void *user);
__isl_give isl_schedule_node *match_mm_init(
    __isl_take isl_schedule_node *node, void *user);
__isl_give isl_schedule_node *match_mm_act(
    __isl_take isl_schedule_node *node, void *user);
__isl_give isl_schedule_node *match_mm_init_act(
    __isl_take isl_schedule_node *node, void *user);
__isl_give isl_schedule_node *match_conv_bare(
    __isl_take isl_schedule_node *node, void *user);
__isl_give isl_schedule_node *match_conv_init(
    __isl_take isl_schedule_node *node, void *user);
__isl_give isl_schedule_node *match_conv_act(
    __isl_take isl_schedule_node *node, void *user);
__isl_give isl_schedule_node *match_conv_init_act(
    __isl_take isl_schedule_node *node, void *user);

__isl_give isl_schedule_node *match_pooling(
    __isl_take isl_schedule_node *node, void *user);

#endif