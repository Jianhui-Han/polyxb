#ifndef INFO_H
#define INFO_H

#include <isl/ctx.h>
#include <isl/id.h>
#include <isl/set.h>
#include <isl/val.h>

#include "access.h"

struct ast_array {
	isl_id *name;
	int n_levels;
	isl_val *ranges[4];
};

struct ast_info {
    isl_size n_args;

    int is_conv;
    int is_pool;
    isl_val *strd;

    int has_init;

    int has_act;
    isl_id *act;
    
    isl_size dims_in;
    isl_val *ranges[7];
    
    isl_size n_levels;
    isl_val *sizes[6];
    isl_val *sizes_tiled[6];
    
    isl_id *names[3];
    isl_val *lds[3];
    isl_size dims_out[3];
    isl_size dims_inner[3];
    isl_size dims_outer[3];

    int is_pipe;
    int n_pes;
    int a_base;
    int n_stage;

    int n_arrays;
	struct ast_array *arrays[3];
};

void ast_info_init(struct ast_info *info);
void ast_info_free(struct ast_info *info);

struct ast_info *get_ast_info(isl_ctx *ctx, __isl_keep isl_basic_set *domain,
    struct access_mac *acc, int n_args, int has_init, __isl_take isl_id *act,
    void *user);

struct ast_info *get_ast_info_conv(isl_ctx *ctx,
    __isl_keep isl_basic_set *domain, struct access_mac *acc, int n_args,
    int has_init, __isl_take isl_id *n_act, void *user);

struct ast_info *get_ast_info_pool(isl_ctx *ctx,
    __isl_keep isl_basic_set *domain, struct access_mac *acc, int n_args,
    void *user);

#endif