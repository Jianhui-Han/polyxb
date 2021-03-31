#include <isl/constraint.h>
#include <isl/ctx.h>
#include <isl/printer.h>
#include <isl/schedule.h>
#include <isl/schedule_node.h>
#include <isl/union_map.h>
#include <isl/union_set.h>

#include "polyxb.h"
#include "schedule.h"

#include <isl/set.h>


static __isl_give isl_schedule_constraints *construct_schedule_constraints(
	struct polyxb_scop *ps)
{
	isl_schedule_constraints *sc;
	isl_union_map *validity, *coincidence;

	sc = isl_schedule_constraints_on_domain(isl_union_set_copy(ps->domain));
	
    validity = isl_union_map_copy(ps->dep_flow);
    validity = isl_union_map_union(validity, isl_union_map_copy(ps->dep_false));
    coincidence = isl_union_map_copy(validity);

    sc = isl_schedule_constraints_set_coincidence(sc, coincidence);
	sc = isl_schedule_constraints_set_validity(sc, validity);
	sc = isl_schedule_constraints_set_proximity(sc,
                isl_union_map_copy(ps->dep_flow));

	return sc;
}


__isl_give isl_schedule *compute_schedule(void *user)
{
    struct polyxb_scop *ps = user;
    if (!ps)
        return NULL;

    isl_schedule_constraints *sc;
    isl_schedule *schedule;

    sc = construct_schedule_constraints(ps);
    schedule = isl_schedule_constraints_compute_schedule(sc);
    return schedule;
}

/* The job is done by calling `isl_schedule_node_band_tile`.
 */
__isl_give isl_schedule_node *tile_band(
    __isl_take isl_schedule_node *node, void *user)
{
    return NULL;
}

/* Print the given schedule.
 */
void print_schedule(__isl_keep isl_schedule *schedule)
{
    isl_ctx *ctx = isl_schedule_get_ctx(schedule);
    isl_printer *p = isl_printer_to_file(ctx, stderr);

    p = isl_printer_set_yaml_style(p, ISL_YAML_STYLE_BLOCK);
    p = isl_printer_print_schedule(p, schedule);
    p = isl_printer_print_str(p, "\n");

    isl_printer_free(p);
}

isl_bool print_node(__isl_keep isl_schedule_node *node, void *user)
{
    isl_ctx *ctx = isl_schedule_node_get_ctx(node);
    isl_printer *p = isl_printer_to_file(ctx, stderr);
    p = isl_printer_set_yaml_style(p, ISL_YAML_STYLE_BLOCK);
    p = isl_printer_print_str(p, "\n");
    p = isl_printer_print_schedule_node(p, node);
    p = isl_printer_print_union_set(p, isl_schedule_node_get_domain(node));
    p = isl_printer_print_str(p, "\n");
    isl_printer_free(p);
    return isl_bool_true;
}

/*
 */
void print_schedule_node(__isl_keep isl_schedule *schedule)
{
    isl_schedule_foreach_schedule_node_top_down(schedule, &print_node, NULL);
}