#ifndef SCHEDULE_H
#define SCHEDULE_H

#include <isl/ctx.h>
#include <isl/schedule.h>

__isl_give isl_schedule *compute_schedule(void *user);

__isl_give isl_schedule_node *tile_band(__isl_take isl_schedule_node *node,
    void *user);

void print_schedule(__isl_keep isl_schedule *schedule);
void print_schedule_node(__isl_keep isl_schedule *schedule);
isl_bool print_node(__isl_keep isl_schedule_node *node, void *user);

#endif