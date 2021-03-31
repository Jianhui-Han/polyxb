#include <string.h>

#include <isl/ast.h>
#include <isl/id.h>
#include <isl/ilp.h>
#include <isl/set.h>
#include <isl/space.h>

#include "build.h"
#include "info.h"
#include "node.h"
#include "pipeline.h"
#include "polyxb.h"
#include "scop.h"
#include "utils.h"

const char iter_loop[] = "c%d";
const char iter_tile[] = "t%d";
int idx_outer[4] = {0, 1, 2, 3};
int idx_inner_mm[3][2] = {{0, 2}, {2, 1}, {0, 1}};
int idx_inner_mv[3][2] = {{0, 1}, {1, -1}, {0, -1}};
int idx_inner_pool[2][2] = {{0, 1}, {0, 1}};

/*
 */
static __isl_give isl_ast_node *build_conv_call(isl_ctx *ctx,
    __isl_take isl_ast_expr_list *conv, int is_load,
    struct ast_info *info, void *user)
{
    isl_ast_expr *funct, *body, *temp0, *temp1;
    isl_ast_node *node;
    isl_ast_expr_list *list = isl_ast_expr_list_alloc(ctx, 13);
    isl_ast_expr_list *indices;
    isl_size s;
    struct polyxb_scop *ps = user;
    int i, upper;
    isl_id *name;

    if (ps->options->en_tiling)
        if (info->is_pool)
            for (i=1; i<3; i++)
                list = isl_ast_expr_list_add(list,
                    isl_ast_expr_list_get_at(conv, i));
        else
            for (i=1; i<4; i++)
                list = isl_ast_expr_list_add(list,
                    isl_ast_expr_list_get_at(conv, i));
    else
        if (info->is_pool)
            for (i=0; i<2; i++)
                list = isl_ast_expr_list_add(list,
                    isl_ast_expr_list_get_at(conv, i));
        else
            for (i=0; i<3; i++)
                list = isl_ast_expr_list_add(list,
                    isl_ast_expr_list_get_at(conv, i));

    if (is_load) {
        if (info->is_pool)
            funct = isl_ast_expr_from_id(
                isl_id_alloc(ctx, "pool_load_data", NULL));
        else
            funct = isl_ast_expr_from_id(
                isl_id_alloc(ctx, "conv_load_data", NULL));

        if (info->is_pool)
            for (i=info->dims_in-5; i<info->dims_in; i++)
                list = isl_ast_expr_list_add(list,
                    isl_ast_expr_from_val(isl_val_copy(info->ranges[i])));
        else
            for (i=info->dims_in-6; i<info->dims_in; i++)
                list = isl_ast_expr_list_add(list,
                    isl_ast_expr_from_val(isl_val_copy(info->ranges[i])));
        list = isl_ast_expr_list_add(list,
            isl_ast_expr_from_val(
                isl_val_sub(isl_val_copy(info->strd), isl_val_one(ctx))));

        if (info->is_pool)
            temp0 = isl_ast_expr_list_get_at(conv, 3);
        else
            temp0 = isl_ast_expr_list_get_at(conv, 4);
        temp1 = isl_ast_expr_op_get_arg(temp0, 0);
        s = isl_ast_expr_op_get_n_arg(temp1);
        indices = isl_ast_expr_list_alloc(ctx, 4);
        for (i=0; i<info->dims_outer[0]; i++) {
            if (isl_val_get_num_si(info->ranges[i]) == 1)
                indices = isl_ast_expr_list_add(indices,
                    isl_ast_expr_from_val(isl_val_zero(ctx)));
            else
                indices = isl_ast_expr_list_add(indices,
                    isl_ast_expr_op_get_arg(temp1, i+1));
        }
        indices = isl_ast_expr_list_concat(indices,
            ast_expr_list_zeros(ctx, s-info->dims_outer[0])); 
        list = isl_ast_expr_list_add(list,
            isl_ast_expr_address_of(
                isl_ast_expr_access(
                    isl_ast_expr_from_id(isl_id_copy(info->arrays[0]->name)),
                    indices)));
        list = isl_ast_expr_list_add(list, temp0);
        list = isl_ast_expr_list_add(list, isl_ast_expr_op_get_arg(temp1, s-2));
        list = isl_ast_expr_list_add(list, isl_ast_expr_op_get_arg(temp1, s-1));
        isl_ast_expr_free(temp1);

        if (info->is_pool) {
            temp0 = isl_ast_expr_list_get_at(conv, 5);
            upper = info->dims_outer[1];
            name = info->arrays[1]->name;
        }
        else {
            temp0 = isl_ast_expr_list_get_at(conv, 8);
            upper = info->dims_outer[2];
            name = info->arrays[2]->name;
        }
        temp1 = isl_ast_expr_op_get_arg(temp0, 0);
        s = isl_ast_expr_op_get_n_arg(temp1);
        indices = isl_ast_expr_list_alloc(ctx, 4);
        for (i=0; i<upper; i++) {
            if (isl_val_get_num_si(info->ranges[i]) == 1)
                indices = isl_ast_expr_list_add(indices,
                    isl_ast_expr_from_val(isl_val_zero(ctx)));
            else
                indices = isl_ast_expr_list_add(indices,
                    isl_ast_expr_op_get_arg(temp1, i+1));
        }
        indices = isl_ast_expr_list_concat(indices,
            ast_expr_list_zeros(ctx, s-upper));
        list = isl_ast_expr_list_add(list,
            isl_ast_expr_address_of(
                isl_ast_expr_access(
                    isl_ast_expr_from_id(isl_id_copy(name)),
                    indices)));
        list = isl_ast_expr_list_add(list, temp0);
        list = isl_ast_expr_list_add(list, isl_ast_expr_op_get_arg(temp1, s-2));
        list = isl_ast_expr_list_add(list, isl_ast_expr_op_get_arg(temp1, s-1));
        isl_ast_expr_free(temp1);
    }
    else {
        if (info->is_pool) {
            funct = isl_ast_expr_from_id(
                isl_id_alloc(ctx, "pool_store_data", NULL));
            for (i=info->dims_in-5; i<info->dims_in-2; i++)
                list = isl_ast_expr_list_add(list,
                    isl_ast_expr_from_val(isl_val_copy(info->ranges[i])));

            temp0 = isl_ast_expr_list_get_at(conv, 5);
            upper = info->dims_outer[1];
            name = info->arrays[1]->name;
        }
        else {
            funct = isl_ast_expr_from_id(
                isl_id_alloc(ctx, "conv_store_data", NULL));
            for (i=info->dims_in-6; i<info->dims_in-3; i++)
                list = isl_ast_expr_list_add(list,
                    isl_ast_expr_from_val(isl_val_copy(info->ranges[i])));
            
            temp0 = isl_ast_expr_list_get_at(conv, 8);
            upper = info->dims_outer[2];
            name = info->arrays[2]->name;
        }
        
        temp1 = isl_ast_expr_op_get_arg(temp0, 0);
        s = isl_ast_expr_op_get_n_arg(temp1);
        list = isl_ast_expr_list_add(list, temp0);
        indices = isl_ast_expr_list_alloc(ctx, 4);
        for (i=0; i<upper; i++) {
            if (isl_val_get_num_si(info->ranges[i]) == 1)
                indices = isl_ast_expr_list_add(indices,
                    isl_ast_expr_from_val(isl_val_zero(ctx)));
            else
                indices = isl_ast_expr_list_add(indices,
                    isl_ast_expr_op_get_arg(temp1, i+1));
        }
        indices = isl_ast_expr_list_concat(indices,
            ast_expr_list_zeros(ctx, s-upper)); 
        list = isl_ast_expr_list_add(list,
            isl_ast_expr_address_of(
                isl_ast_expr_access(
                    isl_ast_expr_from_id(isl_id_copy(name)),
                    indices)));
        list = isl_ast_expr_list_add(list, isl_ast_expr_op_get_arg(temp1, s-2));
        list = isl_ast_expr_list_add(list, isl_ast_expr_op_get_arg(temp1, s-1));
        isl_ast_expr_free(temp1);
    }

    body = isl_ast_expr_call(funct, list);
    node = isl_ast_node_alloc_user(body);

    isl_ast_expr_list_free(conv);
    return node;
}

/*
 */
static __isl_give isl_ast_expr_list *build_array_access(isl_ctx *ctx,
    isl_size dim, isl_size outer, isl_size inner, int *idx,
    int bias, struct ast_info *info, void *user)
{
    struct polyxb_scop *ps = user;
    isl_ast_expr_list *list;
    int i, ind=0;
    char name[2];

    if (!ps->options->en_tiling) {
        if (outer > 0) {
            list = isl_ast_expr_list_alloc(ctx, 4);
            for (i=0; i<outer; i++) {
                if (isl_val_get_num_si(info->ranges[i]) == 1) {
                    list = isl_ast_expr_list_add(list,
                        isl_ast_expr_from_val(isl_val_zero(ctx)));
                }
                else {
                    sprintf(name, iter_loop, ind);
                    list = isl_ast_expr_list_add(list,
                        isl_ast_expr_from_id(isl_id_alloc(ctx, name, NULL)));
                    ind++;
                }
            }
            list = isl_ast_expr_list_concat(list,
                ast_expr_list_zeros(ctx, inner));
        }
        else
            list = ast_expr_list_zeros(ctx, dim);
    }
    else {
        if (outer > 0) {
            list = isl_ast_expr_list_alloc(ctx, 4);
            for (i=0; i<outer; i++) {
                if (isl_val_get_num_si(info->ranges[i]) == 1) {
                    list = isl_ast_expr_list_add(list,
                        isl_ast_expr_from_val(isl_val_zero(ctx)));
                }
                else {
                    sprintf(name, iter_loop, ind);
                    if (ps->options->en_pipeline && info->is_pipe)
                        list = isl_ast_expr_list_add(list,
                            isl_ast_expr_sub(
                                isl_ast_expr_from_id(
                                    isl_id_alloc(ctx, name, NULL)), 
                                isl_ast_expr_from_val(
                                    isl_val_int_from_si(ctx,
                                        info->n_stage-1))));
                    else
                        list = isl_ast_expr_list_add(list,
                            isl_ast_expr_from_id(
                                isl_id_alloc(ctx, name, NULL)));
                    ind++;
                }
            }
            list = isl_ast_expr_list_concat(list,
                ast_expr_list_iters(ctx, inner, idx, iter_tile));
        }
        else
            list = ast_expr_list_iters(ctx, inner, idx, iter_tile);
    }

    return list;
}

/*
 */
static __isl_give isl_ast_node *build_block_node_conv(isl_ctx *ctx,
    __isl_take isl_ast_node *body, struct ast_info *info, 
    __isl_take isl_ast_expr_list *par_list, void *user)
{
    isl_ast_node_list *conv_list;
    isl_ast_node *ast_node;
    struct polyxb_scop *ps = user;
    isl_id *temp;
    int i;

    conv_list = isl_ast_node_list_alloc(ctx, 3);
    conv_list = isl_ast_node_list_add(conv_list,
        build_conv_call(ctx, isl_ast_expr_list_copy(par_list), 1, info, ps));
    conv_list = isl_ast_node_list_add(conv_list, body);
    conv_list = isl_ast_node_list_add(conv_list,
        build_conv_call(ctx, isl_ast_expr_list_copy(par_list), 0, info, ps));
    ast_node = isl_ast_node_block_build(ctx, conv_list);

    // Do a swap here for future use during printing.
    for (i=0; i<3; i++) {
        temp = info->names[i];
        info->names[i] = info->arrays[i]->name;
        info->arrays[i]->name = temp;
    }

    isl_ast_expr_list_free(par_list);
    return ast_node;
}

/*
 */
static __isl_give isl_ast_node *build_block_node_pool(isl_ctx *ctx,
    __isl_take isl_ast_node *body, struct ast_info *info,
    __isl_take isl_ast_expr_list *par_list, void *user)
{
    isl_ast_node_list *pool_list;
    isl_ast_node *ast_node;
    struct polyxb_scop *ps = user;
    isl_id *temp;
    int i;

    pool_list = isl_ast_node_list_alloc(ctx, 3);
    pool_list = isl_ast_node_list_add(pool_list,
        build_conv_call(ctx, isl_ast_expr_list_copy(par_list), 1, info, ps));
    pool_list = isl_ast_node_list_add(pool_list, body);
    pool_list = isl_ast_node_list_add(pool_list,
        build_conv_call(ctx, isl_ast_expr_list_copy(par_list), 0, info, ps));
    ast_node = isl_ast_node_block_build(ctx, pool_list);

    // Do a swap here for future use during printing.
    for (i=0; i<2; i++) {
        temp = info->names[i];
        info->names[i] = info->arrays[i]->name;
        info->arrays[i]->name = temp;
    }

    isl_ast_expr_list_free(par_list);
    return ast_node;
}

/* Build the user node with the presence of an activation.
 */
static __isl_give isl_ast_node *build_user_node_act(isl_ctx *ctx,
    __isl_take isl_ast_expr *funct_act,
    __isl_take isl_ast_expr_list *list,
    struct ast_info *info, void *user)
{
    struct polyxb_scop *ps = user;
    isl_ast_expr *funct_then;
    isl_ast_expr *guard;
    isl_ast_node *node, *node_then, *node_else;
    isl_ast_expr_list *list_then, *list_else;
    char iterator[2];

    sprintf(iterator, iter_tile, info->n_levels-1);
    guard = isl_ast_expr_lt(
        isl_ast_expr_from_id(isl_id_alloc(ctx, iterator, NULL)),
        isl_ast_expr_from_val(
            isl_val_copy(info->sizes_tiled[info->n_levels-1])));
    
    if (info->n_levels == 3)
        funct_then = isl_ast_expr_from_id(
            isl_id_alloc(ctx, "xbblas_mm_bare", NULL));
    else
        funct_then = isl_ast_expr_from_id(
            isl_id_alloc(ctx, "xbblas_mv_bare", NULL));

    if (info->has_init)
        list_then = isl_ast_expr_list_add(isl_ast_expr_list_copy(list),
            isl_ast_expr_eq(
                isl_ast_expr_from_id(isl_id_alloc(ctx, iterator, NULL)),
                isl_ast_expr_from_val(isl_val_zero(ctx))));
    else
        list_then = isl_ast_expr_list_add(isl_ast_expr_list_copy(list),
            isl_ast_expr_from_val(isl_val_zero(ctx)));
    node_then = isl_ast_node_alloc_user(
        isl_ast_expr_call(funct_then, list_then));
 
    list_else = isl_ast_expr_list_add(list,
        isl_ast_expr_from_id(isl_id_copy(info->act)));
    if (info->has_init)
        list_else = isl_ast_expr_list_add(list_else,
            isl_ast_expr_eq(
                isl_ast_expr_from_id(isl_id_alloc(ctx, iterator, NULL)),
                isl_ast_expr_from_val(isl_val_zero(ctx))));
    else
        list_else = isl_ast_expr_list_add(list_else,
            isl_ast_expr_from_val(isl_val_zero(ctx)));
    node_else = isl_ast_node_alloc_user(
        isl_ast_expr_call(funct_act, list_else));
    
    node = isl_ast_node_if_build(ctx, guard, node_then, node_else);

    return node;
}

/* Build the expression that do spatial mapping.
 */
static __isl_give isl_ast_expr *build_spatial_mapping(isl_ctx *ctx,
    struct ast_info *info, void *user)
{
    struct polyxb_scop *ps = user;
    isl_ast_expr *expr;
    char iterator[2];
    isl_id *id;
    int i, n;

    for (i=0; i<info->n_levels; i++) {
        sprintf(iterator, iter_tile, i);
        id = isl_id_alloc(ctx, iterator, NULL);
        if (i == 0)
            expr = isl_ast_expr_from_id(id);
        else
            expr = isl_ast_expr_add(
                isl_ast_expr_mul(expr,
                    isl_ast_expr_from_val(
                        isl_val_copy(info->sizes_tiled[i]))),
                isl_ast_expr_from_id(id));
    }

    if (ps->options->en_pipeline && info->is_pipe) {
        expr = isl_ast_expr_pdiv_r(expr, isl_ast_expr_from_val(
            isl_val_int_from_ui(ctx, info->n_pes)));
        expr = isl_ast_expr_add(expr, isl_ast_expr_from_val(
            isl_val_int_from_ui(ctx, info->a_base)));
    }
    else
        expr = isl_ast_expr_pdiv_r(expr, isl_ast_expr_from_val(
            isl_val_int_from_ui(ctx, ps->options->num_pes)));

    return expr;
}

/* Build the ast_node for marker node.
 */
static __isl_give isl_ast_node *build_mark_node(
    __isl_keep isl_schedule_node *node, void *user)
{
    isl_id *id = isl_schedule_node_mark_get_id(node);
    const char *name = isl_id_get_name(id);
    struct ast_info *info = isl_id_get_user(id);
    isl_ctx *ctx = isl_schedule_node_get_ctx(node);
    struct polyxb_scop *ps = user;

    isl_ast_expr *funct = isl_ast_expr_from_id(isl_id_alloc(ctx, name, NULL));
    isl_ast_expr_list *list = isl_ast_expr_list_alloc(ctx, info->n_args);
    int i, bias=0;

    isl_ast_expr *body;
    isl_ast_node *ast_node;
    isl_ast_expr_list *sublist;

    char iterator[2];

    if (info->is_pipe)
        bias = info->n_stage;

    if (!ps->options->en_tiling) {
        for (i=0; i<info->n_levels; i++)
            list = isl_ast_expr_list_add(list,
                isl_ast_expr_from_val(isl_val_copy(info->sizes[i])));
        
        if (info->is_pool) {
            for (int i=0; i<2; i++) {
                list = isl_ast_expr_list_add(list, isl_ast_expr_address_of(
                    isl_ast_expr_access(
                        isl_ast_expr_from_id(isl_id_copy(info->names[i])),
                        build_array_access(ctx, info->dims_out[i],
                            info->dims_outer[i], info->dims_inner[i],
                            idx_inner_pool[i], bias, info, ps))));
                list = isl_ast_expr_list_add(list,
                    isl_ast_expr_from_val(isl_val_copy(info->lds[i])));
            }
            body = isl_ast_expr_call(funct, isl_ast_expr_list_copy(list));
            ast_node = isl_ast_node_alloc_user(body);
            ast_node = build_block_node_pool(ctx, ast_node, info, list, ps);
        }

        else if (ps->options->en_conv) {
            for (i=0; i<3; i++)
                if (info->n_levels == 3) {
                    list = isl_ast_expr_list_add(list, isl_ast_expr_address_of(
                        isl_ast_expr_access(
                            isl_ast_expr_from_id(isl_id_copy(info->names[i])),
                            build_array_access(ctx, info->dims_out[i],
                                info->dims_outer[i], info->dims_inner[i],
                                idx_inner_mm[i], bias, info, ps))));
                }
                else {
                    list = isl_ast_expr_list_add(list, isl_ast_expr_address_of(
                        isl_ast_expr_access(
                            isl_ast_expr_from_id(isl_id_copy(info->names[i])),
                            build_array_access(ctx, info->dims_out[i],
                                info->dims_outer[i], info->dims_inner[i],
                                idx_inner_mv[i], bias, info, ps))));
                }
            if (info->has_act)
                list = isl_ast_expr_list_add(list, 
                    isl_ast_expr_from_id(isl_id_copy(info->act)));
            body = isl_ast_expr_call(funct, list);
            ast_node = isl_ast_node_alloc_user(body);
        }
        
        else {
            for (i=0; i<3; i++) {
                if (info->n_levels == 3) {
                    list = isl_ast_expr_list_add(list, isl_ast_expr_address_of(
                        isl_ast_expr_access(
                            isl_ast_expr_from_id(isl_id_copy(info->names[i])),
                            build_array_access(ctx, info->dims_out[i],
                                info->dims_outer[i], info->dims_inner[i],
                                idx_inner_mm[i], bias, info, ps))));
                    list = isl_ast_expr_list_add(list,
                            isl_ast_expr_from_val(isl_val_copy(info->lds[i])));
                }
                else {
                    list = isl_ast_expr_list_add(list, isl_ast_expr_address_of(
                        isl_ast_expr_access(
                            isl_ast_expr_from_id(isl_id_copy(info->names[i])),
                            build_array_access(ctx, info->dims_out[i],
                                info->dims_outer[i], info->dims_inner[i],
                                idx_inner_mv[i], bias, info, ps))));
                    if (i == 0)
                        list = isl_ast_expr_list_add(list,
                            isl_ast_expr_from_val(isl_val_copy(info->lds[i])));
                }
            }
            if (info->has_act)
                list = isl_ast_expr_list_add(list, 
                    isl_ast_expr_from_id(isl_id_copy(info->act)));
            
            if (info->is_conv) {
                body = isl_ast_expr_call(funct, isl_ast_expr_list_copy(list));
                ast_node = isl_ast_node_alloc_user(body);
                ast_node = build_block_node_conv(ctx, ast_node, info, list, ps);
            }
            else {
                body = isl_ast_expr_call(funct, list);
                ast_node = isl_ast_node_alloc_user(body);
            }
        }

        ast_info_free(info);
    }

    else {
        list = isl_ast_expr_list_add(list,
            build_spatial_mapping(ctx, info, ps));

        for (i=0; i<info->n_levels; i++) {
            sublist = isl_ast_expr_list_alloc(ctx, 2);
            sublist = isl_ast_expr_list_add(sublist,
                isl_ast_expr_from_val(
                    isl_val_int_from_ui(ctx, ps->options->mm_size[i])));
            sublist = isl_ast_expr_list_add(sublist,
                isl_ast_expr_sub(
                    isl_ast_expr_from_val(isl_val_copy(info->sizes[i])),
                    ast_expr_iter(ctx, i)));
            list = isl_ast_expr_list_add(list,
                isl_ast_expr_call(
                    isl_ast_expr_from_id(isl_id_alloc(ctx, "min", NULL)),
                    sublist));
        }

        if (info->is_pool) {
            for (i=0; i<2; i++) {
                list = isl_ast_expr_list_add(list, isl_ast_expr_address_of(
                    isl_ast_expr_access(
                        isl_ast_expr_from_id(isl_id_copy(info->names[i])),
                            build_array_access(ctx, info->dims_out[i],
                                info->dims_outer[i], info->dims_inner[i],
                                idx_inner_pool[i], bias, info, ps))));
                list = isl_ast_expr_list_add(list,
                    isl_ast_expr_from_val(isl_val_copy(info->lds[i])));
            }
        }
        else {

        for (i=0; i<3; i++) {
            if (info->n_levels == 3) {
                list = isl_ast_expr_list_add(list, isl_ast_expr_address_of(
                    isl_ast_expr_access(
                        isl_ast_expr_from_id(isl_id_copy(info->names[i])),
                            build_array_access(ctx, info->dims_out[i],
                                info->dims_outer[i], info->dims_inner[i],
                                idx_inner_mm[i], bias, info, ps))));
                list = isl_ast_expr_list_add(list,
                    isl_ast_expr_from_val(isl_val_copy(info->lds[i])));
            }
            else {
                list = isl_ast_expr_list_add(list, isl_ast_expr_address_of(
                    isl_ast_expr_access(
                        isl_ast_expr_from_id(isl_id_copy(info->names[i])),
                            build_array_access(ctx, info->dims_out[i],
                                info->dims_outer[i], info->dims_inner[i],
                                idx_inner_mv[i], bias, info, ps))));
                if (i == 0)
                    list = isl_ast_expr_list_add(list,
                        isl_ast_expr_from_val(isl_val_copy(info->lds[i])));
            }
        }

        }
             
        if (info->has_act) {
            if (info->is_conv || info->is_pool)
                ast_node = build_user_node_act(ctx, funct,
                    isl_ast_expr_list_copy(list), info, ps);
            else
                ast_node = build_user_node_act(ctx, funct, list, info, ps);
        }
        else {
            if (info->has_init) {
                sprintf(iterator, iter_tile, info->n_levels-1);
                list = isl_ast_expr_list_add(list,
                    isl_ast_expr_eq(
                        isl_ast_expr_from_id(isl_id_alloc(ctx, iterator, NULL)),
                        isl_ast_expr_from_val(isl_val_zero(ctx))));
            }
            else
                list = isl_ast_expr_list_add(list,
                    isl_ast_expr_from_val(isl_val_zero(ctx)));
            if (info->is_conv || info->is_pool)
                body = isl_ast_expr_call(funct, isl_ast_expr_list_copy(list));
            else
                body = isl_ast_expr_call(funct, list);
            ast_node = isl_ast_node_alloc_user(body);
        }

        if (info->is_conv)
            ast_node = build_block_node_conv(ctx, ast_node, info, list, ps);
        else if (info->is_pool)
            ast_node = build_block_node_pool(ctx, ast_node, info, list, ps);
    }

    isl_id_free(id);
    return ast_node;
}

static __isl_give isl_ast_node *build_config_node(isl_ctx *ctx,
    struct ast_info *info, void *user)
{
    isl_ast_expr *funct, *body;
    isl_ast_node *node, *node_load, *node_malloc;
    isl_ast_expr_list *expr_list;
    isl_ast_node_list *node_list = isl_ast_node_list_alloc(ctx, 4);
    struct polyxb_scop *ps = user;
    int i, j;

    for (i=0; i<3-info->is_pool; i++) {
        if (info->arrays[i]->n_levels == 2) {
            funct = isl_ast_expr_from_id(
                isl_id_alloc(ctx, "xbblas_malloc_2d", NULL));
            expr_list = isl_ast_expr_list_alloc(ctx, 3);
            expr_list = isl_ast_expr_list_add(expr_list,
                isl_ast_expr_address_of(isl_ast_expr_access(
                    isl_ast_expr_from_id(isl_id_copy(info->names[i])),
                    isl_ast_expr_list_alloc(ctx, 1))));
            for (j=0; j<2; j++)
                expr_list = isl_ast_expr_list_add(expr_list,
                    isl_ast_expr_from_val(
                        isl_val_copy(info->arrays[i]->ranges[j])));
        }
        else {
            funct = isl_ast_expr_from_id(
                isl_id_alloc(ctx, "xbblas_malloc_3d", NULL));
            expr_list = isl_ast_expr_list_alloc(ctx, 4);
            expr_list = isl_ast_expr_list_add(expr_list,
                isl_ast_expr_address_of(isl_ast_expr_access(
                    isl_ast_expr_from_id(isl_id_copy(info->names[i])),
                    isl_ast_expr_list_alloc(ctx, 1))));
            for (j=0; j<3; j++)
                expr_list = isl_ast_expr_list_add(expr_list,
                    isl_ast_expr_from_val(
                        isl_val_copy(info->arrays[i]->ranges[j])));
        } 
        body = isl_ast_expr_call(funct, expr_list);
        node_malloc = isl_ast_node_alloc_user(body);
        node_list = isl_ast_node_list_add(node_list, node_malloc);
    }
    
    if (info->is_conv) {
        funct = isl_ast_expr_from_id(isl_id_alloc(ctx, "conv_load_filter", NULL));
        expr_list = isl_ast_expr_list_alloc(ctx, 6);
        expr_list = isl_ast_expr_list_add(expr_list,
            isl_ast_expr_address_of(isl_ast_expr_access(
                isl_ast_expr_from_id(isl_id_copy(info->arrays[1]->name)),
                ast_expr_list_zeros(ctx, 4))));
        expr_list = isl_ast_expr_list_add(expr_list,
            isl_ast_expr_from_val(isl_val_copy(info->ranges[info->dims_in-6])));
        expr_list = isl_ast_expr_list_add(expr_list,
            isl_ast_expr_from_val(isl_val_copy(info->ranges[info->dims_in-3])));
        expr_list = isl_ast_expr_list_add(expr_list,
            isl_ast_expr_from_val(isl_val_copy(info->ranges[info->dims_in-2])));
        expr_list = isl_ast_expr_list_add(expr_list,
            isl_ast_expr_from_val(isl_val_copy(info->ranges[info->dims_in-1])));
        expr_list = isl_ast_expr_list_add(expr_list,
            isl_ast_expr_address_of(isl_ast_expr_access(
                isl_ast_expr_from_id(isl_id_copy(info->names[1])),
                ast_expr_list_zeros(ctx, 2))));
        body = isl_ast_expr_call(funct, expr_list);
        node_load = isl_ast_node_alloc_user(body);
        node_list = isl_ast_node_list_add(node_list, node_load);
    }

    node = isl_ast_node_block_build(ctx, node_list);
    return node;
}

/* Build the ast_node for marker node.
 * This function is called when tiling is needed.
 */
static __isl_give isl_ast_node *build_for_node(
    __isl_keep isl_schedule_node *node, void *user, int depth)
{
    isl_id *id = isl_schedule_node_mark_get_id(node);
    const char *name = isl_id_get_name(id);
    struct ast_info *info = isl_id_get_user(id);
    isl_ctx *ctx = isl_schedule_node_get_ctx(node);
    struct polyxb_scop *ps = user;

    isl_ast_expr *iter, *init, *cond, *inc;
    isl_ast_node *ast_node, *body;
    char iterator[2];

    if ((depth == info->n_levels) || (depth == -1)) {
        ast_node = build_mark_node(node, ps);
        ast_info_free(info);
    }
    else {
        sprintf(iterator, iter_tile, depth);
        iter = isl_ast_expr_from_id(isl_id_alloc(ctx, iterator, NULL));
        init = isl_ast_expr_from_val(isl_val_zero(ctx));
        cond = isl_ast_expr_le(
            isl_ast_expr_from_id(isl_id_alloc(ctx, iterator, NULL)),
            isl_ast_expr_from_val(isl_val_sub(
                isl_val_copy(info->sizes[depth]), isl_val_one(ctx))));
        inc = isl_ast_expr_from_val(
            isl_val_int_from_ui(ctx, ps->options->mm_size[depth]));
        body = build_for_node(node, ps, depth+1);

        ast_node = isl_ast_node_for_build(ctx, iter, init, cond, inc, body);
    }

    isl_id_free(id);
    return ast_node;
}

/*
 */
__isl_give isl_ast_node *build_pipeline_node(
    __isl_keep isl_schedule_node *node, void *user)
{
    isl_ctx *ctx = isl_schedule_node_get_ctx(node);
    struct polyxb_scop *ps = user;
    isl_ast_node *ast_node, *for_node, *if_node;
    isl_ast_node_list *list, *list_for;
    isl_size n;
    isl_schedule_node *child, *grand, *ggrand;
    int i;

    isl_ast_expr *guard, *iter, *init, *cond, *inc;
    isl_basic_set *bset;
    isl_id *id_info;
    struct ast_info *info;
    int pos;
    isl_val *val;

    bset = node_band_get_domain(node);

    child = isl_schedule_node_get_child(node, 0);
    n = isl_schedule_node_n_children(child);
    list = isl_ast_node_list_alloc(ctx, n+1);

    for (i=0; i<n; i++) {
        grand = isl_schedule_node_get_child(child, i);
        ggrand = isl_schedule_node_get_child(grand, 0);
        id_info = isl_schedule_node_mark_get_id(ggrand);
        info = isl_id_get_user(id_info);

        if (i == 0) {
            if (info->is_conv || info->is_pool)
                pos = isl_basic_set_n_dim(bset) - info->n_levels - 4;
            else
                pos = isl_basic_set_n_dim(bset) - info->n_levels - 1;
            val = isl_basic_set_dim_max_val(bset, pos);
            val = isl_val_add_ui(val, 1);
        }

        if (info->is_conv || info->is_pool) {
            list_for = isl_ast_node_list_alloc(ctx, 2);
            list_for = isl_ast_node_list_add(list_for,
                build_config_node(ctx, info, ps));
            list_for = isl_ast_node_list_add(list_for,
                build_for_node(ggrand, ps, 0));
            for_node = isl_ast_node_block_build(ctx, list_for);
        }
        else {
            for_node = build_for_node(ggrand, ps, 0);
        }
        isl_id_free(id_info);
    
        guard = isl_ast_expr_and(
            isl_ast_expr_ge(isl_ast_expr_from_id(isl_id_alloc(ctx, "c0", NULL)),
                isl_ast_expr_from_val(isl_val_int_from_ui(ctx, i))), 
            isl_ast_expr_le(isl_ast_expr_from_id(isl_id_alloc(ctx, "c0", NULL)),
                isl_ast_expr_from_val(isl_val_sub(
                    isl_val_add(isl_val_copy(val),
                        isl_val_int_from_ui(ctx, i)),
                    isl_val_one(ctx)))));
        if_node = isl_ast_node_if_build(ctx, guard, for_node, NULL);
        list = isl_ast_node_list_add(list, if_node);
    
        isl_schedule_node_free(ggrand);
        isl_schedule_node_free(grand);
    }
    isl_schedule_node_free(child);

    list = isl_ast_node_list_add(list,
        isl_ast_node_alloc_user(
            isl_ast_expr_from_id(isl_id_alloc(ctx, "fence()", NULL))));
    ast_node = isl_ast_node_block_build(ctx, list);

    iter = isl_ast_expr_from_id(isl_id_alloc(ctx, "c0", NULL));
    init = isl_ast_expr_from_val(isl_val_zero(ctx));
    cond = isl_ast_expr_le(
        isl_ast_expr_from_id(isl_id_alloc(ctx, "c0", NULL)),
            isl_ast_expr_from_val(isl_val_add(
                isl_val_sub(val, isl_val_one(ctx)),
                isl_val_int_from_si(ctx, n-1))));
    inc = isl_ast_expr_from_val(isl_val_one(ctx));
    ast_node = isl_ast_node_for_build(ctx, iter, init, cond, inc, ast_node);
    return ast_node;
}

/*
 */
__isl_give isl_schedule_node *do_build(
    __isl_take isl_schedule_node *node, void *user)
{
    struct polyxb_scop *ps = user;
    isl_ctx *ctx = isl_schedule_node_get_ctx(node);

    isl_ast_node *ast_node;
    isl_schedule_node *mark_node;

    isl_id *id, *id_info;

    struct ast_info *info;
    isl_ast_node_list *node_list;

    if (isl_schedule_node_get_type(node) != isl_schedule_node_mark)
        return node;
    
    id = isl_schedule_node_mark_get_id(node);
    if (!strcmp(isl_id_get_name(id), "Generated pipeline.")) {
        isl_id_free(id);
        return node;
    }
    isl_id_free(id);

    if (!ps->options->en_tiling)
        ast_node = build_mark_node(node, ps);
    else {
        id_info = isl_schedule_node_mark_get_id(node);
        info = isl_id_get_user(id_info);
        if (info->is_conv || info->is_pool) {
            node_list = isl_ast_node_list_alloc(ctx, 2);
            node_list = isl_ast_node_list_add(node_list,
                build_config_node(ctx, info, ps));
            node_list = isl_ast_node_list_add(node_list,
                build_for_node(node, ps, 0));
            ast_node = isl_ast_node_block_build(ctx, node_list);
        }
        else
            ast_node = build_for_node(node, ps, 0);
        isl_id_free(id_info);
    }
    mark_node = isl_schedule_node_insert_mark(node,
        isl_id_alloc(ctx, "Generated function calls.", ast_node));
    
    return mark_node;
}

/*
 */
__isl_give isl_schedule_node *do_build_pipeline(
    __isl_take isl_schedule_node *node, void *user)
{
    struct polyxb_scop *ps = user;
    isl_ctx *ctx = isl_schedule_node_get_ctx(node);

    struct ast_info **infos;

    isl_ast_node *ast_node;
    isl_schedule_node *mark_node;

    if (node_match_structure_pipeline(node, &infos)) {
        do_allocate(node, infos, ps);
        ast_node = build_pipeline_node(node, ps);
        node = isl_schedule_node_cut(node);
        mark_node = isl_schedule_node_insert_mark(node,
            isl_id_alloc(ctx, "Generated pipeline.", ast_node));

        return mark_node;
    }

    return node;
}