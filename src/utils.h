#ifndef UTILS_H
#define UTILS_H

#include <isl/aff.h>
#include <isl/ast.h>
#include <isl/ctx.h>
#include <isl/list.h>
#include <isl/set.h>

FILE *get_output_file(const char *input, const char *output);

__isl_give isl_multi_pw_aff *size_from_extent(__isl_take isl_set *set);

__isl_give isl_multi_pw_aff *mpa_cast(__isl_take isl_multi_pw_aff *mpa,
    int no, int ni);

struct pet_stmt *domain_get_stmt(__isl_keep isl_basic_set *domain, void *user);

__isl_give isl_ast_expr_list *ast_expr_list_zeros(isl_ctx *ctx, int dim);
__isl_give isl_ast_expr_list *ast_expr_list_iters(isl_ctx *ctx, int dim,
    int *indices, const char *iter);
__isl_give isl_ast_expr *ast_expr_iter(isl_ctx *ctx, int dim);

#endif