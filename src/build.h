#ifndef BUILD_H
#define BUILD_H

#include <isl/ctx.h>
#include <isl/schedule_node.h>

__isl_give isl_schedule_node *do_build(
    __isl_take isl_schedule_node *node, void *user);

__isl_give isl_schedule_node *do_build_pipeline(
    __isl_take isl_schedule_node *node, void *user);

#endif