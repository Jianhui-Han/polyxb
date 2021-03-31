#include <string.h>

#include <isl/id.h>
#include <pet.h>

#include "access.h"
#include "stmt.h"

/* Is the statement a MAC operation?
 * If so, return the accesses relations of it; Otherwise, return NULL.
 */
__isl_give struct access_mac *stmt_is_mac(struct pet_stmt *stmt)
{
    struct pet_expr *expr, *arg0, *arg1, *arg10, *arg11;
    struct pet_expr *mult, *mult0, *mult1;
    struct access_mac *acc;
    
    acc = (struct access_mac*)malloc(sizeof(struct access_mac));
    if (!acc) {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(0);
    }
    access_mac_init(acc);

    expr = pet_tree_expr_get_expr(stmt->body);
    if (pet_expr_get_type(expr) != pet_expr_op)
        goto clean;

    arg0 = pet_expr_get_arg(expr, 0);
    if (pet_expr_get_type(arg0) != pet_expr_access)
        goto clean_arg0;
    acc->res = pet_expr_access_get_index(arg0);
    
    if (pet_expr_op_get_type(expr) == pet_op_assign) {
        arg1 = pet_expr_get_arg(expr, 1);
        if ((pet_expr_get_type(arg1) != pet_expr_op) ||
            (pet_expr_op_get_type(arg1) != pet_op_add) ||
            (pet_expr_get_n_arg(expr) != 2))
            goto clean_arg1;
        arg10 = pet_expr_get_arg(arg1, 0);
        arg11 = pet_expr_get_arg(arg1, 1);
        if ((pet_expr_get_type(arg10) == pet_expr_op) &&
            (pet_expr_op_get_type(arg10) == pet_op_mul) &&
            (pet_expr_get_n_arg(arg10) == 2) &&
            (pet_expr_get_type(arg11) == pet_expr_access)) {
            mult = arg10;
            acc->add = pet_expr_access_get_index(arg11);
        }
        else if ((pet_expr_get_type(arg11) == pet_expr_op) &&
            (pet_expr_op_get_type(arg11) == pet_op_mul) &&
            (pet_expr_get_n_arg(arg11) == 2) &&
            (pet_expr_get_type(arg10) == pet_expr_access)) {
            mult = arg11;
            acc->add = pet_expr_access_get_index(arg10);
        }
        else
            goto clean_arg11;
        pet_expr_free(arg11);
        pet_expr_free(arg10);
    }
    else if (pet_expr_op_get_type(expr) == pet_op_add_assign) {
        acc->add = isl_multi_pw_aff_copy(acc->res);
        arg1 = pet_expr_get_arg(expr, 1);
        if ((pet_expr_get_type(arg1) == pet_expr_op) ||
            (pet_expr_op_get_type(arg1) == pet_op_mul) ||
            (pet_expr_get_n_arg(expr) == 2))
            mult = arg1;
        else
            goto clean_arg1;
    }
    else 
        goto clean;
    
    /* Handle the multiplication. */
    mult0 = pet_expr_get_arg(mult, 0);
    mult1 = pet_expr_get_arg(mult, 1);
    if ((pet_expr_get_type(mult0) != pet_expr_access) ||
        (pet_expr_get_type(mult1) != pet_expr_access))
        goto clean_mult1;
    acc->mul1 = pet_expr_access_get_index(mult0);
    acc->mul2 = pet_expr_access_get_index(mult1);

    pet_expr_free(mult1);
    pet_expr_free(mult0);
    pet_expr_free(arg1);
    pet_expr_free(arg0);
    pet_expr_free(expr);
    return acc;

clean_mult1:
    pet_expr_free(mult1);
    pet_expr_free(mult0);
clean_arg11:
    pet_expr_free(arg11);
    pet_expr_free(arg10);
clean_arg1:
    pet_expr_free(arg1);
clean_arg0:
    pet_expr_free(arg0);
clean:
    pet_expr_free(expr);
    access_mac_free(acc);
    return NULL;
}

/*
 */
__isl_give struct access_init_act *stmt_is_init(struct pet_stmt *stmt)
{
    struct pet_expr *expr, *arg0, *arg1;
    struct access_init_act *acc;

    acc = (struct access_init_act*)malloc(sizeof(struct access_init_act));
    if (!acc) {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(0);
    }
    access_init_act_init(acc);

    expr = pet_tree_expr_get_expr(stmt->body);
    if ((pet_expr_get_type(expr) != pet_expr_op) ||
        (pet_expr_get_n_arg(expr) != 2))
        goto clean;
    
    arg0 = pet_expr_get_arg(expr, 0);
    if (pet_expr_get_type(arg0) != pet_expr_access)
        goto clean_arg0;
    acc->res = pet_expr_access_get_index(arg0);

    arg1 = pet_expr_get_arg(expr, 1);
    if (pet_expr_get_type(arg1) != pet_expr_double)
        goto clean_arg1;
    if (strtod(pet_expr_double_get_str(arg1), NULL) != 0.0)
        goto clean_arg1;
    
    pet_expr_free(arg1);
    pet_expr_free(arg0);
    pet_expr_free(expr);
    return acc;

clean_arg1:
    pet_expr_free(arg1);
clean_arg0:
    pet_expr_free(arg0);
clean:
    pet_expr_free(expr);
    access_init_act_free(acc);
    return NULL;
}

/*
 */
__isl_give struct access_init_act *stmt_is_act(struct pet_stmt *stmt)
{
    struct pet_expr *expr, *arg0, *arg1;
    struct access_init_act *acc;
    const char *name;

    acc = (struct access_init_act*)malloc(sizeof(struct access_init_act));
    if (!acc) {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(0);
    }
    access_init_act_init(acc);

    expr = pet_tree_expr_get_expr(stmt->body);
    if ((pet_expr_get_type(expr) != pet_expr_op) ||
        (pet_expr_get_n_arg(expr) != 2))
        goto clean;
    
    arg0 = pet_expr_get_arg(expr, 0);
    if (pet_expr_get_type(arg0) != pet_expr_access)
        goto clean_arg0;
    acc->res = pet_expr_access_get_index(arg0);

    arg1 = pet_expr_get_arg(expr, 1);
    if (pet_expr_get_type(arg1) != pet_expr_call)
        goto clean_arg1;
    name = pet_expr_call_get_name(arg1);
    if (strcmp(name, "ReLU") && strcmp(name, "tanh") && strcmp(name, "sigmoid"))
        goto clean_arg1;

    pet_expr_free(arg1);
    pet_expr_free(arg0);
    pet_expr_free(expr);
    return acc;
clean_arg1:
    pet_expr_free(arg1);
clean_arg0:
    pet_expr_free(arg0);
clean:
    pet_expr_free(expr);
    access_init_act_free(acc);
    return NULL;
}

/*
 */
__isl_give isl_id *stmt_get_act(struct pet_stmt *stmt)
{
    struct pet_expr *expr, *act;
    const char *name;
    isl_id *res;

    expr = pet_tree_expr_get_expr(stmt->body);

    act = pet_expr_get_arg(expr, 1);
    name = pet_expr_call_get_name(act);
    res = isl_id_alloc(pet_expr_get_ctx(expr), name, NULL);
    
    pet_expr_free(act);
    pet_expr_free(expr);
    return res;
}

/*
 */
__isl_give struct access_init_act *stmt_is_init_pooling(struct pet_stmt *stmt)
{
    struct pet_expr *expr, *arg0, *arg1;
    struct access_init_act *acc;

    acc = (struct access_init_act*)malloc(sizeof(struct access_init_act));
    if (!acc) {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(0);
    }
    access_init_act_init(acc);

    expr = pet_tree_expr_get_expr(stmt->body);
    if ((pet_expr_get_type(expr) != pet_expr_op) ||
        (pet_expr_get_n_arg(expr) != 2))
        goto clean;
    
    arg0 = pet_expr_get_arg(expr, 0);
    if (pet_expr_get_type(arg0) != pet_expr_access)
        goto clean_arg0;
    acc->res = pet_expr_access_get_index(arg0);

    arg1 = pet_expr_get_arg(expr, 1);
    if (pet_expr_get_type(arg1) != pet_expr_access)
        goto clean_arg1;
    acc->opr = pet_expr_access_get_index(arg1);
    
    pet_expr_free(arg1);
    pet_expr_free(arg0);
    pet_expr_free(expr);
    return acc;

clean_arg1:
    pet_expr_free(arg1);
clean_arg0:
    pet_expr_free(arg0);
clean:
    pet_expr_free(expr);
    access_init_act_free(acc);
    return NULL;
}

/*
 */
__isl_give struct access_mac *stmt_is_mac_pooling(struct pet_stmt *stmt)
{
    struct pet_expr *expr, *arg0, *arg1, *arg10, *arg11;
    struct access_mac *acc;
    const char *name;
    isl_multi_pw_aff *acc10, *acc11;

    acc = (struct access_mac*)malloc(sizeof(struct access_mac));
    if (!acc) {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(0);
    }
    access_mac_init(acc);

    expr = pet_tree_expr_get_expr(stmt->body);
    if (pet_expr_get_type(expr) != pet_expr_op)
        goto clean;
    
    arg0 = pet_expr_get_arg(expr, 0);
    if (pet_expr_get_type(arg0) != pet_expr_access)
        goto clean_arg0;
    acc->res = pet_expr_access_get_index(arg0);

    arg1 = pet_expr_get_arg(expr, 1);
    if (pet_expr_get_type(arg1) != pet_expr_call)
        goto clean_arg1;
    name = pet_expr_call_get_name(arg1);
    if (strcmp(name, "max"))
        goto clean_arg1;
    
    arg10 = pet_expr_get_arg(arg1, 0);
    arg11 = pet_expr_get_arg(arg1, 1);
    if ((pet_expr_get_type(arg10) != pet_expr_access) ||
        (pet_expr_get_type(arg11) != pet_expr_access))
        goto clean_arg11;
    acc10 = pet_expr_access_get_index(arg10);
    acc11 = pet_expr_access_get_index(arg11);
    if (isl_multi_pw_aff_is_equal(acc10, acc->res))
        acc->add = isl_multi_pw_aff_copy(acc11);
    else
        acc->add = isl_multi_pw_aff_copy(acc10);

    isl_multi_pw_aff_free(acc11);
    isl_multi_pw_aff_free(acc10);
    pet_expr_free(arg11);
    pet_expr_free(arg10);
    pet_expr_free(arg1);
    pet_expr_free(arg0);
    pet_expr_free(expr);
    return acc;

clean_arg11:
    pet_expr_free(arg11);
    pet_expr_free(arg10);
clean_arg1:
    pet_expr_free(arg1);
clean_arg0:
    pet_expr_free(arg0);
clean:
    pet_expr_free(expr);
    access_mac_free(acc);
    return NULL;
}