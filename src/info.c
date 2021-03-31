#include <string.h>

#include <isl/ilp.h>

#include "access.h"
#include "info.h"
#include "polyxb.h"
#include "scop.h"

#define INFO_ALLOC info = (struct ast_info*)malloc(sizeof(struct ast_info)); \
    if (!info) { \
        fprintf(stderr, "Memory allocation failed.\n"); \
        exit(0); \
    } \
    ast_info_init(info);

const char name_str_conv[] = "%s_xb";
const char name_str_pool[] = "%s_xb";

/* Initialize the given ast_array object.
 */
static void ast_array_init(struct ast_array *aa)
{
	int i;

	aa->name = NULL;
	aa->n_levels = 0;
	for (i=0; i<4; i++)
		aa->ranges[i] = NULL;
}

/* Free the given ast_array object.
 */
static void ast_array_free(struct ast_array *aa)
{
	int i;

	if(!aa)
		return;

	isl_id_free(aa->name);
	for (i=0; i<aa->n_levels; i++)
		isl_val_free(aa->ranges[i]);
	
	free(aa);
}

/*
 */
void ast_info_init(struct ast_info *info)
{
    int i;

    info->is_conv = 0;
    info->is_pool = 0;
    info->has_init = 0;
    info->has_act = 0;

    info->strd = NULL;
    info->act = NULL;

    for (i=0; i<7; i++)
        info->ranges[i] = NULL;

    for (i=0; i<6; i++) {
        info->sizes[i] = NULL;
        info->sizes_tiled[i] = NULL;
    }
    
    for (i=0; i<3; i++) {
        info->lds[i] = NULL;
        info->names[i] = NULL;
    }

    info->is_pipe = 0;

    info->n_arrays = 0;
	for(i=0; i<3; i++)
		info->arrays[i] = NULL;
}

/*
 */
void ast_info_free(struct ast_info *info)
{
    int i;

    isl_val_free(info->strd);
    isl_id_free(info->act);

    for (i=0; i<7; i++)
        isl_val_free(info->ranges[i]);

    for (i=0; i<6; i++) {
        isl_val_free(info->sizes[i]);
        isl_val_free(info->sizes_tiled[i]);
    }
    
    for (i=0; i<3; i++) {
        isl_val_free(info->lds[i]);
        isl_id_free(info->names[i]);
    }

    for (i=0; i<3; i++)
		ast_array_free(info->arrays[i]);

    free(info);
}

/*
 */
static void get_ast_info_ranges(struct ast_info *info,
    __isl_keep isl_basic_set *domain)
{
    int i;

    info->dims_in = isl_basic_set_n_dim(domain);
    for (i=0; i<info->dims_in; i++) {
        info->ranges[i] = isl_basic_set_dim_max_val(
            isl_basic_set_copy(domain), i);
        info->ranges[i] = isl_val_add_ui(info->ranges[i], 1);
    }
}

static void get_ast_info_names(struct ast_info *info, struct access_mac *acc)
{
    info->names[0] = isl_multi_pw_aff_get_tuple_id(acc->mul1, isl_dim_out);
    info->names[1] = isl_multi_pw_aff_get_tuple_id(acc->mul2, isl_dim_out);
    info->names[2] = isl_multi_pw_aff_get_tuple_id(acc->res, isl_dim_out);
    info->dims_out[0] = isl_multi_pw_aff_dim(acc->mul1, isl_dim_out);
    info->dims_out[1] = isl_multi_pw_aff_dim(acc->mul2, isl_dim_out);
    info->dims_out[2] = isl_multi_pw_aff_dim(acc->res, isl_dim_out);
}

/*
 */
struct ast_info *get_ast_info(isl_ctx *ctx, __isl_keep isl_basic_set *domain,
    struct access_mac *acc, int n_args, int has_init,
    __isl_take isl_id *act, void *user)
{
    struct polyxb_scop *ps = user;
    struct ast_info *info;
    int i, j, temp;

    INFO_ALLOC

    info->n_args = n_args;
    info->n_levels = n_args - 6;
    if (act) {
        info->has_act = 1;
        info->act = act;
        info->n_levels -= 1;
    }
    info->has_init = has_init;

    get_ast_info_ranges(info, domain);
    get_ast_info_names(info, acc);

    for (i=0; i<info->n_levels; i++) {
        info->sizes[i] = isl_val_copy(
            info->ranges[info->dims_in-info->n_levels+i]);
        if (ps->options->en_tiling)
            info->sizes_tiled[i] = isl_val_ceil(
                isl_val_div(
                    isl_val_copy(info->sizes[i]),
                    isl_val_int_from_ui(ctx, ps->options->mm_size[i])));
    }

    if (info->n_levels == 3) {
        info->dims_inner[0] = 2;
        info->dims_inner[1] = 2;
        info->dims_inner[2] = 2;
        info->lds[0] = isl_val_copy(info->ranges[info->dims_in-1]);
        info->lds[1] = isl_val_copy(info->ranges[info->dims_in-2]);
        info->lds[2] = isl_val_copy(info->ranges[info->dims_in-2]);
    }
    else {
        info->dims_inner[0] = 2;
        info->dims_inner[1] = 1;
        info->dims_inner[2] = 1;
        info->lds[0] = isl_val_copy(info->ranges[info->dims_in-1]);
        info->lds[1] = isl_val_one(ctx);
        info->lds[2] = isl_val_one(ctx);
    }
    for (i=0; i<3; i++)
        info->dims_outer[i] = info->dims_out[i] - info->dims_inner[i];

    return info;
}

/*
 */
struct ast_info *get_ast_info_conv(isl_ctx *ctx,
    __isl_keep isl_basic_set *domain, struct access_mac *acc, int n_args,
    int has_init, __isl_take isl_id *act, void *user)
{
    struct polyxb_scop *ps = user;
    struct ast_info *info;
    isl_id *id;
    int i, j, temp;

    char name_new[64];

    INFO_ALLOC

    info->n_args = n_args;
    info->is_conv = 1;
    info->strd = isl_val_copy(acc->strd);
    if (act) {
        info->has_act = 1;
        info->act = act;
    }
    info->has_init = has_init;

    get_ast_info_ranges(info, domain);
    get_ast_info_names(info, acc);

    if (ps->options->en_conv) {
        info->n_levels = 6;
        for (i=0; i<6; i++)
            info->sizes[i] = isl_val_copy(info->ranges[i+1]);
        info->dims_inner[0] = 3;
        info->dims_inner[1] = 4;
        info->dims_inner[2] = 3;
    }
    else {
        info->n_levels = 3;
        info->sizes[0] = isl_val_mul(
            isl_val_copy(info->ranges[info->dims_in-5]),
            isl_val_copy(info->ranges[info->dims_in-4]));
        info->sizes[1] = isl_val_copy(info->ranges[info->dims_in-6]);
        info->sizes[2] = isl_val_mul(
            isl_val_mul(
                isl_val_copy(info->ranges[info->dims_in-3]),
                isl_val_copy(info->ranges[info->dims_in-2])),
            isl_val_copy(info->ranges[info->dims_in-1]));
        
        if (ps->options->en_tiling)
            for (i=0; i<info->n_levels; i++)
                info->sizes_tiled[i] = isl_val_ceil(
                    isl_val_div(
                        isl_val_copy(info->sizes[i]),
                        isl_val_int_from_ui(ctx, ps->options->mm_size[i])));
        info->lds[0] = isl_val_copy(info->sizes[2]);
        info->lds[1] = isl_val_copy(info->sizes[1]);
        info->lds[2] = isl_val_copy(info->sizes[1]);
        info->dims_out[0] -= 1;
        info->dims_out[1] -= 2;
        info->dims_out[2] -= 1;
        info->dims_inner[0] = 2;
        info->dims_inner[1] = 2;
        info->dims_inner[2] = 2;
    }

    for (i=0; i<3; i++)
        info->dims_outer[i] = info->dims_out[i] - info->dims_inner[i];
    
    info->n_arrays = 3;
    for (i=0; i<3; i++) {
        info->arrays[i] = (struct ast_array*)malloc(sizeof(struct ast_array));
        if (!info->arrays[i]) {
            fprintf(stderr, "Memory allocation failed.\n");
            exit(0);
        }
        ast_array_init(info->arrays[i]);
        info->arrays[i]->name = info->names[i];
        sprintf(name_new, name_str_conv, isl_id_get_name(info->names[i]));
        info->names[i] = isl_id_alloc(ctx, name_new, NULL);
        info->arrays[i]->n_levels = info->dims_out[i];

        temp = 0;
        for (j=0; j<isl_id_list_size(ps->xb_arrays); j++) {
            id = isl_id_list_get_at(ps->xb_arrays, j);
            if (!strcmp(name_new, isl_id_get_name(id)))
                temp = 1;
            isl_id_free(id);
        }
        if (!temp) {
            ps->xb_arrays = isl_id_list_add(ps->xb_arrays,
                isl_id_copy(info->names[i]));
            ps->xb_levels = isl_val_list_add(ps->xb_levels,
                isl_val_int_from_si(ctx, info->arrays[i]->n_levels));
        }
    }

    /* It is either 3 or 2. */
    if (info->dims_out[0] == 3) {
        info->arrays[0]->ranges[0] = isl_val_copy(info->ranges[0]);
        info->arrays[0]->ranges[1] = isl_val_copy(info->sizes[0]);
        info->arrays[0]->ranges[2] = isl_val_copy(info->sizes[2]);
        info->arrays[2]->ranges[0] = isl_val_copy(info->ranges[0]);
        info->arrays[2]->ranges[1] = isl_val_copy(info->sizes[0]);
        info->arrays[2]->ranges[2] = isl_val_copy(info->sizes[1]);
    }
    else {
        info->arrays[0]->ranges[0] = isl_val_copy(info->sizes[0]);
        info->arrays[0]->ranges[1] = isl_val_copy(info->sizes[2]);
        info->arrays[2]->ranges[0] = isl_val_copy(info->sizes[0]);
        info->arrays[2]->ranges[1] = isl_val_copy(info->sizes[1]);
    }

    info->arrays[1]->ranges[0] = isl_val_copy(info->sizes[2]);
    info->arrays[1]->ranges[1] = isl_val_copy(info->sizes[1]);

    return info;
}

/*
 */
struct ast_info *get_ast_info_pool(isl_ctx *ctx,
    __isl_keep isl_basic_set *domain, struct access_mac *acc, int n_args,
    void *user)
{
    struct polyxb_scop *ps = user;
    struct ast_info *info;
    isl_id *id;
    int i, j, temp;

    char name_new[64];

    INFO_ALLOC

    info->n_args = n_args;
    info->strd = isl_val_copy(acc->strd);
    info->is_pool = 1;
    
    get_ast_info_ranges(info, domain);
    info->names[0] = isl_multi_pw_aff_get_tuple_id(acc->add, isl_dim_out);
    info->names[1] = isl_multi_pw_aff_get_tuple_id(acc->res, isl_dim_out);
    info->dims_out[0] = isl_multi_pw_aff_dim(acc->add, isl_dim_out) - 1;
    info->dims_out[1] = isl_multi_pw_aff_dim(acc->res, isl_dim_out) - 1;

    info->n_levels = 2;
    info->sizes[0] = isl_val_mul(
        isl_val_copy(info->ranges[info->dims_in-4]),
        isl_val_copy(info->ranges[info->dims_in-3]));
    info->sizes[1] = isl_val_copy(info->ranges[info->dims_in-5]);
    info->sizes[2] = isl_val_mul(
        isl_val_mul(
            isl_val_copy(info->ranges[info->dims_in-2]),
            isl_val_copy(info->ranges[info->dims_in-1])),
        isl_val_copy(info->ranges[info->dims_in-5]));

    if (ps->options->en_tiling)
        for (i=0; i<info->n_levels; i++)
            info->sizes_tiled[i] = isl_val_ceil(
                isl_val_div(
                    isl_val_copy(info->sizes[i]),
                    isl_val_int_from_ui(ctx, ps->options->mm_size[i])));
    info->lds[0] = isl_val_mul(
        isl_val_mul(
            isl_val_copy(info->ranges[info->dims_in-2]),
            isl_val_copy(info->ranges[info->dims_in-1])),
        isl_val_copy(info->ranges[info->dims_in-5]));
    info->lds[1] = isl_val_copy(info->ranges[info->dims_in-5]);
    info->dims_inner[0] = 2;
    info->dims_inner[1] = 2;
    info->dims_outer[0] = info->dims_out[0] - info->dims_inner[0];
    info->dims_outer[1] = info->dims_out[1] - info->dims_inner[1];

    info->n_arrays = 2;
    for (i=0; i<2; i++) {
        info->arrays[i] = (struct ast_array*)malloc(sizeof(struct ast_array));
        if (!info->arrays[i]) {
            fprintf(stderr, "Memory allocation failed.\n");
            exit(0);
        }
        ast_array_init(info->arrays[i]);
        info->arrays[i]->name = info->names[i];
        sprintf(name_new, name_str_conv, isl_id_get_name(info->names[i]));
        info->names[i] = isl_id_alloc(ctx, name_new, NULL);
        info->arrays[i]->n_levels = info->dims_out[i];

        temp = 0;
        for (j=0; j<isl_id_list_size(ps->xb_arrays); j++) {
            id = isl_id_list_get_at(ps->xb_arrays, j);
            if (!strcmp(name_new, isl_id_get_name(id)))
                temp = 1;
            isl_id_free(id);
        }
        if (!temp) {
            ps->xb_arrays = isl_id_list_add(ps->xb_arrays,
                isl_id_copy(info->names[i]));
            ps->xb_levels = isl_val_list_add(ps->xb_levels,
                isl_val_int_from_si(ctx, info->arrays[i]->n_levels));
        }
    }
    
    if (info->dims_out[0] == 3) {
        info->arrays[0]->ranges[0] = isl_val_copy(info->ranges[0]);
        info->arrays[0]->ranges[1] = isl_val_copy(info->sizes[0]);
        info->arrays[0]->ranges[2] = isl_val_copy(info->sizes[2]);
        info->arrays[1]->ranges[0] = isl_val_copy(info->ranges[0]);
        info->arrays[1]->ranges[1] = isl_val_copy(info->sizes[0]);
        info->arrays[1]->ranges[2] = isl_val_copy(info->sizes[1]);
    }
    else {
        info->arrays[0]->ranges[0] = isl_val_copy(info->sizes[0]);
        info->arrays[0]->ranges[1] = isl_val_copy(info->sizes[2]);
        info->arrays[1]->ranges[0] = isl_val_copy(info->sizes[0]);
        info->arrays[1]->ranges[1] = isl_val_copy(info->sizes[1]);
    }

    return info;
}