#ifndef SCOP_H
#define SCOP_H

#include <isl/id.h>
#include <isl/set.h>
#include <isl/union_map.h>
#include <pet.h>

struct polyxb_scop {
    struct polyxb_options *options;

    isl_set *context;
    isl_union_set *domain;
    isl_union_set *call;
	isl_union_map *tagged_reads;
	isl_union_map *reads;
	isl_union_map *live_in;
	isl_union_map *tagged_may_writes;
	isl_union_map *may_writes;
	isl_union_map *tagged_must_writes;
	isl_union_map *must_writes;
	isl_union_map *live_out;
	isl_union_map *tagged_must_kills;
	isl_union_map *must_kills;

	isl_union_pw_multi_aff *tagger;

	isl_union_map *independence;

	isl_union_map *dep_flow;
	isl_union_map *tagged_dep_flow;
	isl_union_map *dep_false;
	isl_union_map *dep_forced;
	isl_union_map *dep_order;
	isl_union_map *tagged_dep_order;
	isl_schedule *schedule;

    isl_id_to_ast_expr *names;

    struct pet_scop *pet;

	isl_id_list *xb_arrays;
	isl_val_list *xb_levels;
};

struct polyxb_scop *polyxb_scop_from_pet_scop(struct pet_scop *scop,
    struct polyxb_options *options);

void *polyxb_scop_free(struct polyxb_scop *ps);

int polyxb_scop_any_hidden_declarations(struct polyxb_scop *scop);

__isl_give isl_id_list *polyxb_scop_generate_names(struct polyxb_scop *scop,
	int n, const char *prefix);

struct pet_stmt *find_stmt(struct polyxb_scop *scop, __isl_keep isl_id *id);

#endif