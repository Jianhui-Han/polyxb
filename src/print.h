#ifndef PRINT_H
#define PRINT_H

#include <isl/ctx.h>
#include <isl/printer.h>

#include "polyxb.h"

/* Information used while building the ast.
 */
struct ast_build_userinfo {
	/* The current polyxb scop. */
	struct polyxb_scop *scop;

	/* Are we currently in a parallel for loop? */
	int in_parallel_for;
};

/* Representation of a statement inside a generated AST.
 *
 * "stmt" refers to the original statement.
 * "ref2expr" maps the reference identifier of each access in
 * the statement to an AST expression that should be printed
 * at the place of the access.
 */
struct polyxb_stmt {
	struct pet_stmt *stmt;
	isl_id_to_ast_expr *ref2expr;
};

__isl_give isl_printer *set_macro_names(__isl_take isl_printer *p);
__isl_give isl_printer *print_exposed_declarations(
	__isl_take isl_printer *p, struct polyxb_scop *scop);
__isl_give isl_printer *print_hidden_declarations(
	__isl_take isl_printer *p, struct polyxb_scop *scop);
__isl_give isl_printer *start_block(__isl_take isl_printer *p);
__isl_give isl_printer *end_block(__isl_take isl_printer *p);
__isl_give isl_printer *print_scop(struct polyxb_scop *scop,
	__isl_take isl_schedule *schedule, __isl_take isl_printer *p,
	struct polyxb_options *options);

#endif