#include <isl/ast.h>
#include <isl/ast_build.h>
#include <isl/ast_type.h>
#include <isl/id.h>
#include <isl/schedule_node.h>
#include <pet.h>

#include "polyxb.h"
#include "print.h"
#include "utils.h"


/* Names used for the macros that may appear in a printed isl AST.
 */
const char *polyxb_min = "polyxb_min";
const char *polyxb_max = "polyxb_max";
const char *polyxb_fdiv_q = "polyxb_fdiv_q";

/* Set the names of the macros that may appear in a printed isl AST.
 */
__isl_give isl_printer *set_macro_names(__isl_take isl_printer *p)
{
	p = isl_ast_op_type_set_print_name(p, isl_ast_op_min, polyxb_min);
	p = isl_ast_op_type_set_print_name(p, isl_ast_op_max, polyxb_max);
	p = isl_ast_op_type_set_print_name(p, isl_ast_op_fdiv_q, polyxb_fdiv_q);
	return p;
}


/* Structure for keeping track of definitions of some macros.
 */
struct polyxb_macros {
	const char *min;
	const char *max;
};

/* Free the memory allocated by a struct polyxb_macros.
 */
static void polyxb_macros_free(void *user)
{
	free(user);
}

/* Default macro definitions (when GNU extensions are allowed).
 */
struct polyxb_macros polyxb_macros_default = {
	.min = "(x,y)    "
		"({ __typeof__(x) _x = (x); __typeof__(y) _y = (y); "
		"_x < _y ? _x : _y; })",
	.max = "(x,y)    "
		"({ __typeof__(x) _x = (x); __typeof__(y) _y = (y); "
		"_x > _y ? _x : _y; })",
};


/* Name used for the note that keeps track of macro definitions.
 */
static const char *polyxb_macros = "polyxb_macros";


/* Return the polyxb_macros object that holds the currently active
 * macro definitions in "p".
 * If "p" has a note with macro definitions, then return those.
 * Otherwise, return the default macro definitions.
 */
static struct polyxb_macros *get_macros(__isl_keep isl_printer *p)
{
	isl_id *id;
	isl_bool has_macros;
	struct polyxb_macros *macros;

	id = isl_id_alloc(isl_printer_get_ctx(p), polyxb_macros, NULL);
	has_macros = isl_printer_has_note(p, id);
	if (has_macros < 0 || !has_macros) {
		isl_id_free(id);
		if (has_macros < 0)
			return NULL;
		return &polyxb_macros_default;
	}
	id = isl_printer_get_note(p, id);
	macros = isl_id_get_user(id);
	isl_id_free(id);

	return macros;
}


/* Names of notes that keep track of whether min/max
 * macro definitions have already been printed.
 */
static const char *polyxb_max_printed = "polyxb_max_printed";
static const char *polyxb_min_printed = "polyxb_min_printed";


/* Has the macro definition corresponding to "note_name" been printed
 * to "p" before?
 * That is, does "p" have an associated "note_name" note?
 */
static isl_bool printed_before(__isl_keep isl_printer *p, const char *note_name)
{
	isl_ctx *ctx;
	isl_id *id;
	isl_bool printed;

	if (!p)
		return isl_bool_error;

	ctx = isl_printer_get_ctx(p);
	id = isl_id_alloc(ctx, note_name, NULL);
	printed = isl_printer_has_note(p, id);
	isl_id_free(id);

	return printed;
}


/* Keep track of the fact that the macro definition corresponding
 * to "note_name" has been printed to "p" by attaching a note with
 * that name.  The value of the note is of no importance, but it
 * has to be a valid isl_id, so the note identifier is reused
 * as the note.
 */
static __isl_give isl_printer *mark_printed(__isl_take isl_printer *p,
	const char *note_name)
{
	isl_ctx *ctx;
	isl_id *id;

	if (!p)
		return NULL;

	ctx = isl_printer_get_ctx(p);
	id = isl_id_alloc(ctx, note_name, NULL);
	return isl_printer_set_note(p, id, isl_id_copy(id));
}


/* Print a macro definition "def" for the macro "name" to "p",
 * unless such a macro definition has been printed to "p" before.
 * "note_name" is used as the name of the note that keeps track
 * of whether this printing has happened.
 */
static __isl_give isl_printer *print_polyxb_macro(__isl_take isl_printer *p,
	const char *name, const char *def, const char *note_name)
{
	isl_bool printed;

	printed = printed_before(p, note_name);
	if (printed < 0)
		return isl_printer_free(p);
	if (printed)
		return p;

	p = isl_printer_start_line(p);
	p = isl_printer_print_str(p, "#define ");
	p = isl_printer_print_str(p, name);
	p = isl_printer_print_str(p, def);
	p = isl_printer_end_line(p);

	p = mark_printed(p, note_name);

	return p;
}


/* Print the currently active macro definition for polyxb_max.
 */
static __isl_give isl_printer *print_max(__isl_take isl_printer *p)
{
	struct polyxb_macros *macros;

	macros = get_macros(p);
	if (!macros)
		return isl_printer_free(p);
	return print_polyxb_macro(p, polyxb_max, macros->max, polyxb_max_printed);
}

/* Print the currently active macro definition for polyxb_min.
 */
static __isl_give isl_printer *print_min(__isl_take isl_printer *p)
{
	struct polyxb_macros *macros;

	macros = get_macros(p);
	if (!macros)
		return isl_printer_free(p);
	return print_polyxb_macro(p, polyxb_min, macros->min, polyxb_min_printed);
}


/* Print a macro definition for "type" to "p".
 * If GNU extensions are allowed, then print a specialized definition
 * for isl_ast_op_min and isl_ast_op_max.
 * Otherwise, use the default isl definition.
 */
__isl_give isl_printer *print_macro_type(enum isl_ast_op_type type,
	__isl_take isl_printer *p)
{
	if (!p)
		return NULL;

	switch (type) {
	case isl_ast_op_max:
		return print_max(p);
	case isl_ast_op_min:
		return print_min(p);
	default:
		return isl_ast_op_type_print_macro(type, p);
	}
}


/* isl_ast_expr_foreach_ast_op_type or isl_ast_node_foreach_ast_op_type
 * callback that prints a macro definition for "type".
 */
static isl_stat print_macro(enum isl_ast_op_type type, void *user)
{
	isl_printer **p = user;

	*p = print_macro_type(type, *p);
	if (!*p)
		return isl_stat_error;

	return isl_stat_ok;
}


/* Print the required macros for "expr".
 */
__isl_give isl_printer *ast_expr_print_macros(
	__isl_keep isl_ast_expr *expr, __isl_take isl_printer *p)
{
	if (isl_ast_expr_foreach_ast_op_type(expr, &print_macro, &p) < 0)
		return isl_printer_free(p);
	return p;
}


/* Print a declaration for an array with element type "base_type" and
 * size "size" to "p".
 */
__isl_give isl_printer *print_declaration_with_size(
	__isl_take isl_printer *p, const char *base_type,
	__isl_keep isl_ast_expr *size)
{
	if (!base_type || !size)
		return isl_printer_free(p);

	p = ast_expr_print_macros(size, p);
	p = isl_printer_start_line(p);
	p = isl_printer_print_str(p, base_type);
	p = isl_printer_print_str(p, " ");
	p = isl_printer_print_ast_expr(p, size);
	p = isl_printer_print_str(p, ";");
	p = isl_printer_end_line(p);

	return p;
}


/* Print a declaration for array "array" to "p", using "build"
 * to simplify any size expressions.
 *
 * The size is computed from the extent of the array and is
 * subsequently converted to an "access expression" by "build".
 */
__isl_give isl_printer *print_declaration_array(__isl_take isl_printer *p,
	struct pet_array *array, __isl_keep isl_ast_build *build)
{
	isl_multi_pw_aff *size;
	isl_ast_expr *expr;

	if (!array)
		return isl_printer_free(p);

	size = size_from_extent(isl_set_copy(array->extent));
	expr = isl_ast_build_access_from_multi_pw_aff(build, size);
	p = print_declaration_with_size(p, array->element_type, expr);
	isl_ast_expr_free(expr);

	return p;
}


/* Print declarations for the arrays in "scop" that are declared
 * and that are exposed (if exposed == 1) or not exposed (if exposed == 0).
 */
static __isl_give isl_printer *print_declarations(__isl_take isl_printer *p,
	struct polyxb_scop *scop, int exposed)
{
	int i;
	isl_ast_build *build;

	if (!scop)
		return isl_printer_free(p);

	build = isl_ast_build_from_context(isl_set_copy(scop->context));
	for (i = 0; i < scop->pet->n_array; ++i) {
		struct pet_array *array = scop->pet->arrays[i];

		if (!array->declared)
			continue;
		if (array->exposed != exposed)
			continue;

		p = print_declaration_array(p, array, build);
	}
	isl_ast_build_free(build);

	return p;
}


/* Print declarations for the arrays in "scop" that are declared
 * and exposed to the code after the scop.
 */
__isl_give isl_printer *print_exposed_declarations(
	__isl_take isl_printer *p, struct polyxb_scop *scop)
{
	return print_declarations(p, scop, 1);
}

/* Print declarations for the arrays in "scop" that are declared,
 * but not exposed to the code after the scop.
 */
__isl_give isl_printer *print_hidden_declarations(
	__isl_take isl_printer *p, struct polyxb_scop *scop)
{
	return print_declarations(p, scop, 0);
}

__isl_give isl_printer *start_block(__isl_take isl_printer *p)
{
	p = isl_printer_start_line(p);
	p = isl_printer_print_str(p, "{");
	p = isl_printer_end_line(p);
	p = isl_printer_indent(p, 2);
	return p;
}

__isl_give isl_printer *end_block(__isl_take isl_printer *p)
{
	p = isl_printer_indent(p, -2);
	p = isl_printer_start_line(p);
	p = isl_printer_print_str(p, "}");
	p = isl_printer_end_line(p);
	return p;
}


/* Set *depth (initialized to 0 by the caller) to the maximum
 * of the schedule depths of the leaf nodes for which this function is called.
 */
static isl_bool update_depth(__isl_keep isl_schedule_node *node, void *user)
{
	int *depth = user;
	int node_depth;

	if (isl_schedule_node_get_type(node) != isl_schedule_node_leaf)
		return isl_bool_true;
	node_depth = isl_schedule_node_get_schedule_depth(node);
	if (node_depth > *depth)
		*depth = node_depth;

	return isl_bool_false;
}


static void polyxb_stmt_free(void *user)
{
	struct polyxb_stmt *stmt = user;

	if (!stmt)
		return;

	isl_id_to_ast_expr_free(stmt->ref2expr);

	free(stmt);
}


/* Index transformation callback for pet_stmt_build_ast_exprs.
 *
 * "index" expresses the array indices in terms of statement iterators
 * "iterator_map" expresses the statement iterators in terms of
 * AST loop iterators.
 *
 * The result expresses the array indices in terms of
 * AST loop iterators.
 */
static __isl_give isl_multi_pw_aff *pullback_index(
	__isl_take isl_multi_pw_aff *index, __isl_keep isl_id *id, void *user)
{
	isl_pw_multi_aff *iterator_map = user;

	iterator_map = isl_pw_multi_aff_copy(iterator_map);
	return isl_multi_pw_aff_pullback_pw_multi_aff(index, iterator_map);
}

/* Transform the accesses in the statement associated to the domain
 * called by "node" to refer to the AST loop iterators, construct
 * corresponding AST expressions using "build",
 * collect them in a polyxb_stmt and annotate the node with the polyxb_stmt.
 */
static __isl_give isl_ast_node *at_each_domain(__isl_take isl_ast_node *node,
	__isl_keep isl_ast_build *build, void *user)
{
	struct polyxb_scop *scop = user;
	isl_ast_expr *expr, *arg;
	isl_ctx *ctx;
	isl_id *id;
	isl_map *map;
	isl_pw_multi_aff *iterator_map;
	struct polyxb_stmt *stmt;

	ctx = isl_ast_node_get_ctx(node);
	stmt = isl_calloc_type(ctx, struct polyxb_stmt);
	if (!stmt)
		goto error;

	expr = isl_ast_node_user_get_expr(node);
	arg = isl_ast_expr_get_op_arg(expr, 0);
	isl_ast_expr_free(expr);
	id = isl_ast_expr_get_id(arg);
	isl_ast_expr_free(arg);
	stmt->stmt = find_stmt(scop, id);
	isl_id_free(id);
	if (!stmt->stmt)
		goto error;

	map = isl_map_from_union_map(isl_ast_build_get_schedule(build));
	map = isl_map_reverse(map);
	iterator_map = isl_pw_multi_aff_from_map(map);
	stmt->ref2expr = pet_stmt_build_ast_exprs(stmt->stmt, build,
				        &pullback_index, iterator_map, NULL, NULL);
	isl_pw_multi_aff_free(iterator_map);

	id = isl_id_alloc(isl_ast_node_get_ctx(node), NULL, stmt);
	id = isl_id_set_free_user(id, &polyxb_stmt_free);
	return isl_ast_node_set_annotation(node, id);
error:
	polyxb_stmt_free(stmt);
	return isl_ast_node_free(node);
}

/* Print a user statement in the generated AST.
 * The polyxb_stmt has been attached to the node in at_each_domain.
 */
static __isl_give isl_printer *print_user(__isl_take isl_printer *p,
	__isl_take isl_ast_print_options *print_options,
	__isl_keep isl_ast_node *node, void *user)
{
	struct polyxb_stmt *stmt;
	isl_id *id;

	id = isl_ast_node_get_annotation(node);
	stmt = isl_id_get_user(id);
	isl_id_free(id);

	p = pet_stmt_print_body(stmt->stmt, p, stmt->ref2expr);

	isl_ast_print_options_free(print_options);
	return p;
}


/* Print a for node.
 *
 * Depending on how the node is annotated, we either print a normal
 * for node or an openmp parallel for node.
 */
static __isl_give isl_printer *print_for(__isl_take isl_printer *p,
	__isl_take isl_ast_print_options *print_options,
	__isl_keep isl_ast_node *node, void *user)
{
	isl_id *id;
	id = isl_ast_node_get_annotation(node);

    p = isl_ast_node_for_print(node, p, print_options);

	isl_id_free(id);

	return p;
}


/* isl_id_to_ast_expr_foreach callback that prints the required
 * macro definitions for "val".
 */
static isl_stat print_expr_macros(__isl_take isl_id *key,
	__isl_take isl_ast_expr *val, void *user)
{
	isl_printer **p = user;

	*p = ast_expr_print_macros(val, *p);
	isl_id_free(key);
	isl_ast_expr_free(val);

	if (!*p)
		return isl_stat_error;
	return isl_stat_ok;
}


/* Print the required macro definitions for the body of a statement in which
 * the access expressions are replaced by the isl_ast_expr objects
 * in "ref2expr".
 */
__isl_give isl_printer *print_body_macros(__isl_take isl_printer *p,
	__isl_keep isl_id_to_ast_expr *ref2expr)
{
	if (isl_id_to_ast_expr_foreach(ref2expr, &print_expr_macros, &p) < 0)
		return isl_printer_free(p);
	return p;
}


/* This function is called for each node in an AST.
 * In case of a user node, print the macro definitions required
 * for printing the AST expressions in the annotation, if any.
 * For other nodes, return true such that descendants are also
 * visited.
 *
 * In particular, print the macro definitions needed for the substitutions
 * of the original user statements.
 */
static isl_bool at_node(__isl_keep isl_ast_node *node, void *user)
{
	struct polyxb_stmt *stmt;
	isl_id *id;
	isl_printer **p = user;

	if (isl_ast_node_get_type(node) != isl_ast_node_user)
		return isl_bool_true;

	id = isl_ast_node_get_annotation(node);
	stmt = isl_id_get_user(id);
	isl_id_free(id);

	if (!stmt)
		return isl_bool_error;

	*p = print_body_macros(*p, stmt->ref2expr);
	if (!*p)
		return isl_bool_error;

	return isl_bool_false;
}


/* Print the required macros for the CPU AST "node" to "p",
 * including those needed for the user statements inside the AST.
 */
static __isl_give isl_printer *print_macros_node(__isl_take isl_printer *p,
	__isl_keep isl_ast_node *node)
{
	if (isl_ast_node_foreach_descendant_top_down(node, &at_node, &p) < 0)
		return isl_printer_free(p);
    if (isl_ast_node_foreach_ast_op_type(node, &print_macro, &p) < 0)
		return isl_printer_free(p);
	return p;
}

/* Print the ast_node inside mark_node, which should be built by PolyXB
 * instead of being generated from the schedule tree.
 * Maybe too much printing details are exposed here.
 */
static __isl_give isl_printer *print_mark_node(__isl_take isl_printer *p,
	__isl_keep isl_ast_print_options *print_options,
	__isl_keep isl_ast_node *node, void *user)
{
	isl_id *id;
	const char *name;
	const char *type;
	isl_ast_node *body;
	isl_ast_expr *expr;
	isl_ast_node_list *list;
	int i;

	if (isl_ast_node_get_type(node) == isl_ast_node_for) {
		type = isl_options_get_ast_iterator_type(isl_printer_get_ctx(p));
		expr = isl_ast_node_for_get_iterator(node);
		id = isl_ast_expr_get_id(expr);
		isl_ast_expr_free(expr);
		name = isl_id_get_name(id);
		isl_id_free(id);
		p = isl_printer_start_line(p);
		p = isl_printer_print_str(p, "for (");
		p = isl_printer_print_str(p, type);
		p = isl_printer_print_str(p, " ");
		p = isl_printer_print_str(p, name);
		p = isl_printer_print_str(p, " = ");
		expr = isl_ast_node_for_get_init(node);
		p = isl_printer_print_ast_expr(p, expr);
		isl_ast_expr_free(expr);
		p = isl_printer_print_str(p, "; ");
		expr = isl_ast_node_for_get_cond(node);
		p = isl_printer_print_ast_expr(p, expr);
		isl_ast_expr_free(expr);
		p = isl_printer_print_str(p, "; ");
		p = isl_printer_print_str(p, name);
		p = isl_printer_print_str(p, " += ");
		expr = isl_ast_node_for_get_inc(node);
		p = isl_printer_print_ast_expr(p, expr);
		isl_ast_expr_free(expr);
		p = isl_printer_print_str(p, ")");
		p = isl_printer_print_str(p, " {");
		p = isl_printer_end_line(p);
		p = isl_printer_indent(p, 2);
		body = isl_ast_node_for_get_body(node);
		p = print_mark_node(p, print_options, body, NULL);
		isl_ast_node_free(body);
		p = isl_printer_indent(p, -2);
		p = isl_printer_start_line(p);
		p = isl_printer_print_str(p, "}");
		p = isl_printer_end_line(p);
	}
	else if (isl_ast_node_get_type(node) == isl_ast_node_if) {
		expr = isl_ast_node_if_get_cond(node);
		p = isl_printer_start_line(p);
		p = isl_printer_print_str(p, "if (");
		p = isl_printer_print_ast_expr(p, expr);
		p = isl_printer_print_str(p, ") {");
		p = isl_printer_end_line(p);
		p = isl_printer_indent(p, 2);
		isl_ast_expr_free(expr);
		body = isl_ast_node_if_get_then(node);
		p = print_mark_node(p, print_options, body, NULL);
		isl_ast_node_free(body);
		p = isl_printer_indent(p, -2);
		p = isl_printer_start_line(p);
		p = isl_printer_print_str(p, "}");
		p = isl_printer_end_line(p);
		if (isl_ast_node_if_has_else_node(node)) {
			p = isl_printer_start_line(p);
			p = isl_printer_print_str(p, "else {");
			p = isl_printer_end_line(p);
			p = isl_printer_indent(p, 2);
			body = isl_ast_node_if_get_else(node);
			p = print_mark_node(p, print_options, body, NULL);
			isl_ast_node_free(body);
			p = isl_printer_indent(p, -2);
			p = isl_printer_start_line(p);
			p = isl_printer_print_str(p, "}");
			p = isl_printer_end_line(p);
		}
	}
	else if (isl_ast_node_get_type(node) == isl_ast_node_block) {
		list = isl_ast_node_block_get_children(node);
		for (i=0; i<isl_ast_node_list_size(list); i++) {
			body = isl_ast_node_list_get_at(list, i);
			p = print_mark_node(p, print_options, body, NULL);
			isl_ast_node_free(body);
		}
		isl_ast_node_list_free(list);
	}
	else {
		expr = isl_ast_node_user_get_expr(node);
		p = isl_printer_start_line(p);
		p = isl_printer_print_ast_expr(p, expr);
		p = isl_printer_print_str(p, ";");
		p = isl_printer_end_line(p);
		isl_ast_expr_free(expr);
	}

	return p;
}

/* Print a mark node.
 * The ast_expr is constructed and attached to the mark node during
 * the inserting of that mark node.
 */
static __isl_give isl_printer *print_mark(__isl_take isl_printer *p,
	__isl_take isl_ast_print_options *print_options,
	__isl_keep isl_ast_node *node, void *user)
{
	isl_ast_node *unode = user;

	p = print_mark_node(p, print_options, unode, NULL);

	isl_ast_node_free(unode);
	isl_ast_print_options_free(print_options);
	return p;
}

/*
 */
static __isl_give isl_printer *print_polyxb_arrays(
	__isl_take isl_printer *p, struct polyxb_scop *scop)
{
	isl_ctx *ctx = isl_printer_get_ctx(p);
	isl_id *id;
	isl_val *val;
	int i, j;

	for (i=0; i<isl_id_list_size(scop->xb_arrays); i++) {
		p = isl_printer_start_line(p);
		p = isl_printer_print_str(p, "double ");
		val = isl_val_list_get_at(scop->xb_levels, i);
		for (j=0; j<isl_val_get_num_si(val); j++)
			p = isl_printer_print_str(p, "*");
		isl_val_free(val);
		id = isl_id_list_get_at(scop->xb_arrays, i);
		p = isl_printer_print_id(p, id);
		isl_id_free(id);
		p = isl_printer_print_str(p, ";");
		p = isl_printer_end_line(p);
	}

	return p;
}

/* Code generate the scop 'scop' using "schedule"
 * and print the corresponding C code to 'p'.
 */
__isl_give isl_printer *print_scop(struct polyxb_scop *scop,
	__isl_take isl_schedule *schedule, __isl_take isl_printer *p,
	struct polyxb_options *options)
{
	isl_ctx *ctx = isl_printer_get_ctx(p);
	isl_ast_build *build;
	isl_ast_print_options *print_options;
	isl_ast_node *tree;
	isl_id_list *iterators;
	struct ast_build_userinfo build_info;
	int depth;

	depth = 0;
	if (isl_schedule_foreach_schedule_node_top_down(schedule, &update_depth,
	        &depth) < 0)
		goto error;

	build = isl_ast_build_alloc(ctx);
	iterators = polyxb_scop_generate_names(scop, depth, "c");
	build = isl_ast_build_set_iterators(build, iterators);
	build = isl_ast_build_set_at_each_domain(build, &at_each_domain, scop);

	tree = isl_ast_build_node_from_schedule(build, schedule);
	isl_ast_build_free(build);

	print_options = isl_ast_print_options_alloc(ctx);
	print_options = isl_ast_print_options_set_print_user(print_options,
							&print_user, NULL);
	print_options = isl_ast_print_options_set_print_for(print_options,
							&print_for, NULL);
	print_options = isl_ast_print_options_set_print_mark(print_options,
							&print_mark, NULL);

	p = print_macros_node(p, tree);
	p = print_polyxb_arrays(p, scop);
	p = isl_ast_node_print(tree, p, print_options);
	isl_ast_node_free(tree);
	return p;
error:
	isl_schedule_free(schedule);
	isl_printer_free(p);
	return NULL;
}