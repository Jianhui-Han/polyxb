#include <limits.h>
#include <stdio.h>
#include <string.h>

#include <isl/id.h>

#include "scop.h"
#include "utils.h"

/* Return a pointer to the final path component of "filename" or
 * to "filename" itself if it does not contain any components.
 */
const char *base_name(const char *filename)
{
	const char *base;

	base = strrchr(filename, '/');
	if (base)
		return ++base;
	else
		return filename;
}

/* Copy the base name of "input" to "name" and return its length.
 * "name" is not NULL terminated.
 *
 * In particular, remove all leading directory components and
 * the final extension, if any.
 */
static int extract_base_name(char *name, const char *input)
{
	const char *base;
	const char *ext;
	int len;

	base = base_name(input);
	ext = strrchr(base, '.');
	len = ext ? ext - base : strlen(base);

	memcpy(name, base, len);

	return len;
}

/* Derive the output file name from the input file name.
 * 'input' is the entire path of the input file. The output
 * is the file name plus the additional extension.
 *
 * We will basically replace everything after the last point
 * with '.polyxb.c'. This means file.c becomes file.polyxb.c
 */
FILE *get_output_file(const char *input, const char *output)
{
    char name[PATH_MAX];
    const char *ext;
    const char marker[] = ".polyxb";
    int len;
    FILE *file;

    len = extract_base_name(name, input);
    strcpy(name + len, marker);
    ext = strrchr(input, '.');
    strcpy(name + len + sizeof(marker) - 1, ext ? ext : ".c");

    if (!output)
        output = name;
    
    file = fopen(output, "w");
    if (!file) {
        fprintf(stderr, "Unable to open '%s' for writing.\n", output);
        return NULL;
    }

    return file;
}

/* Compute the size of a bounding box around the origin and "set",
 * where "set" is assumed to contain only non-negative elements.
 * In particular, compute the maximal value of "set" in each direction
 * and add one.
 */
__isl_give isl_multi_pw_aff *size_from_extent(__isl_take isl_set *set)
{
	int i, n;
	isl_multi_pw_aff *mpa;

	n = isl_set_dim(set, isl_dim_set);
	mpa = isl_multi_pw_aff_zero(isl_set_get_space(set));
	for (i = 0; i < n; ++i) {
		isl_space *space;
		isl_aff *one;
		isl_pw_aff *bound;

		if (!isl_set_dim_has_upper_bound(set, isl_dim_set, i)) {
			const char *name;
			name = isl_set_get_tuple_name(set);
			if (!name)
				name = "";
			fprintf(stderr, "unable to determine extent of '%s' "
				"in dimension %d\n", name, i);
			set = isl_set_free(set);
		}
		bound = isl_set_dim_max(isl_set_copy(set), i);

		space = isl_pw_aff_get_domain_space(bound);
		one = isl_aff_zero_on_domain(isl_local_space_from_space(space));
		one = isl_aff_add_constant_si(one, 1);
		bound = isl_pw_aff_add(bound, isl_pw_aff_from_aff(one));
		mpa = isl_multi_pw_aff_set_pw_aff(mpa, i, bound);
	}
	isl_set_free(set);

	return mpa;
}

/*
 */
__isl_give isl_multi_pw_aff *mpa_cast(__isl_take isl_multi_pw_aff *mpa,
    int no, int ni)
{
    isl_multi_pw_aff *res;
    isl_size n;

    res =  isl_multi_pw_aff_reset_tuple_id(mpa, isl_dim_out);
    res = isl_multi_pw_aff_reset_tuple_id(res, isl_dim_in);
    n = isl_multi_pw_aff_dim(res, isl_dim_out);
    if ((no > 0) && (n > no))
        res = isl_multi_pw_aff_drop_dims(res, isl_dim_out, 0, n-no);
    n = isl_multi_pw_aff_dim(res, isl_dim_in);
    if ((ni > 0) && (n > ni))
        res = isl_multi_pw_aff_drop_dims(res, isl_dim_in, 0, n-ni);

    return res;
}

/*
 */
struct pet_stmt *domain_get_stmt(__isl_keep isl_basic_set *domain, void *user)
{
    struct polyxb_scop *ps = user;

    const char *name;
    isl_id *id;
    struct pet_stmt *stmt;

    name = isl_basic_set_get_tuple_name(domain);
    id = isl_id_alloc(isl_basic_set_get_ctx(domain), name, NULL);
    stmt = find_stmt(ps, id);

    isl_id_free(id);
    return stmt;
}

/*
 */
__isl_give isl_ast_expr_list *ast_expr_list_zeros(isl_ctx *ctx, int dim)
{
	isl_ast_expr_list *list = isl_ast_expr_list_alloc(ctx, dim);
	int i;

	for (i=0; i<dim; i++)
		list = isl_ast_expr_list_add(list,
			isl_ast_expr_from_val(isl_val_zero(ctx)));
	
	return list;
}

/*
 */
__isl_give isl_ast_expr_list *ast_expr_list_iters(isl_ctx *ctx, int dim,
    int *indices, const char *iter)
{
	isl_ast_expr_list *list = isl_ast_expr_list_alloc(ctx, dim);
	int i;
	char name[2];

	for (i=0; i<dim; i++) {		
		sprintf(name, iter, indices[i]);
		list = isl_ast_expr_list_add(list,
			isl_ast_expr_from_id(isl_id_alloc(ctx, name, NULL)));
	}

	return list;
}

/*
 */
__isl_give isl_ast_expr *ast_expr_iter(isl_ctx *ctx, int dim)
{
	isl_ast_expr *expr;
	const char iter[] = "t%d";
	char name[2];

	sprintf(name, iter, dim);
	expr = isl_ast_expr_from_id(isl_id_alloc(ctx, name, NULL));

	return expr;
}
