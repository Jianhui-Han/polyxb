#include <isl/id.h>
#include <isl/schedule.h>
#include <isl/schedule_node.h>

#include "build.h"
#include "match.h"
#include "pipeline.h"
#include "tactics.h"

__isl_give isl_schedule *do_tactics(__isl_take isl_schedule *schedule,
    struct polyxb_scop *ps)
{
    schedule = isl_schedule_map_schedule_node_bottom_up(schedule,
        &match_conv_init_act, ps);
    schedule = isl_schedule_map_schedule_node_bottom_up(schedule,
        &match_conv_init, ps);
    schedule = isl_schedule_map_schedule_node_bottom_up(schedule,
        &match_conv_act, ps);
    schedule = isl_schedule_map_schedule_node_bottom_up(schedule,
        &match_conv_bare, ps);
    
    schedule = isl_schedule_map_schedule_node_bottom_up(schedule,
        &match_pooling, ps);

    schedule = isl_schedule_map_schedule_node_bottom_up(schedule,
        &match_mm_init_act, ps);
    schedule = isl_schedule_map_schedule_node_bottom_up(schedule,
        &match_mm_init, ps);
    schedule = isl_schedule_map_schedule_node_bottom_up(schedule,
        &match_mm_act, ps);
    schedule = isl_schedule_map_schedule_node_bottom_up(schedule,
        &match_mm_bare, ps);

    schedule = isl_schedule_map_schedule_node_bottom_up(schedule,
        &match_mv_init, ps);
    schedule = isl_schedule_map_schedule_node_bottom_up(schedule,
        &match_mv_bare, ps);

    if (ps->options->en_pipeline)
        schedule = isl_schedule_map_schedule_node_bottom_up(schedule,
            &do_build_pipeline, ps);
    schedule = isl_schedule_map_schedule_node_bottom_up(schedule,
        &do_build, ps);

    return schedule;
}