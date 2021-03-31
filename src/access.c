#include <string.h>

#include <isl/aff.h>
#include <isl/ctx.h>
#include <isl/point.h>
#include <isl/space.h>

#include "access.h"
#include "utils.h"

const char acc_str_mul1_mv[] = "{[i,j] -> [(i),(j)]}";
const char acc_str_mul2_mv[] = "{[i,j] -> [(j)]}";
const char acc_str_res_mv[] = "{[i,j] -> [(i)]}";
const char acc_str_mul1_mm[] = "{[i,j,k] -> [(i),(k)]}"; 
const char acc_str_mul2_mm[] = "{[i,j,k] -> [(k),(j)]}"; 
const char acc_str_res_mm[] = "{[i,j,k] -> [(i),(j)]}";
const char acc_str_mul1_conv[] =
    "{[i,j,k,m,n,p,q] -> [(n),(k*%ld+p),(m*%ld+q)]}";
const char acc_str_mul2_conv[] = "{[i,j,k,m,n,p,q] -> [(j),(n),(p),(q)]}";
const char acc_str_res_conv[] = "{[i,j,k,m,n,p,q] -> [(j),(k),(m)]}";
const char acc_str_opr_pool[] =
    "{[i,j,k,m,p,q] -> [(i),(j),(k*%ld),(m*%ld)]}";
const char acc_str_res_pool[] = "{[i,j,k,m,p,q] -> [(i),(j),(k),(m)]}";
const char acc_str_call_pool[] =
    "{[i,j,k,m,p,q] -> [(i),(j),(k*%ld+p),(m*%ld+q)]}";
const char acc_str_res_pool_init[] = "{[i,j,k] -> [(i),(j),(k)]}";
const char acc_str_opr_pool_init[] = "{[i,j,k] -> [(i),(j*%ld),(k*%ld)]}";

/* Initialize an access_mac.
 */
void access_mac_init(struct access_mac *acc)
{
    acc->mul1 = NULL;
    acc->mul2 = NULL;
    acc->add = NULL;
    acc->res = NULL;
    acc->strd = NULL;
}

/* Free an access_mac.
 */
void access_mac_free(struct access_mac *acc)
{
    isl_multi_pw_aff_free(acc->mul1);
    isl_multi_pw_aff_free(acc->mul2);
    isl_multi_pw_aff_free(acc->add);
    isl_multi_pw_aff_free(acc->res);
    isl_val_free(acc->strd);
    free(acc);
}

/*
 */
static int mul_acc_is_mv(isl_ctx *ctx,
    isl_multi_pw_aff *mul1, isl_multi_pw_aff *mul2)
{
    isl_multi_pw_aff *acc_mul1, *acc_mul2;
    isl_multi_pw_aff *mir_mul1, *mir_mul2;
    isl_size n;
    int res = 0;

    acc_mul1 = isl_multi_pw_aff_read_from_str(ctx, acc_str_mul1_mv);
    acc_mul2 = isl_multi_pw_aff_read_from_str(ctx, acc_str_mul2_mv);

    mir_mul1 = mpa_cast(isl_multi_pw_aff_copy(mul1), 2, 2);
    mir_mul2 = mpa_cast(isl_multi_pw_aff_copy(mul2), 1, 2);

    if (isl_multi_pw_aff_is_equal(acc_mul1, mir_mul1) &&
        isl_multi_pw_aff_is_equal(acc_mul2, mir_mul2))
        res = 1;

    isl_multi_pw_aff_free(mir_mul1);
    isl_multi_pw_aff_free(mir_mul2);
    isl_multi_pw_aff_free(acc_mul1);
    isl_multi_pw_aff_free(acc_mul2);
    return res;
}

/* Is the access_mac a MAC in MV?
 */
int access_is_mv(isl_ctx *ctx, struct access_mac *acc)
{
    isl_multi_pw_aff *acc_res, *mir_res, *temp;
    isl_size n;

    if (!isl_multi_pw_aff_is_equal(acc->res, acc->add))
        return 0;
        
    acc_res = isl_multi_pw_aff_read_from_str(ctx, acc_str_res_mv);
    mir_res = mpa_cast(isl_multi_pw_aff_copy(acc->res), 1, 2);

    if (!isl_multi_pw_aff_is_equal(mir_res, acc_res)) {
        isl_multi_pw_aff_free(mir_res);
        isl_multi_pw_aff_free(acc_res);
        return 0;
    }
    isl_multi_pw_aff_free(mir_res);
    isl_multi_pw_aff_free(acc_res);
        
    if (mul_acc_is_mv(ctx, acc->mul1, acc->mul2))
        return 1;
    if (mul_acc_is_mv(ctx, acc->mul2, acc->mul1)) {
        temp = acc->mul1;
        acc->mul1 = acc->mul2;
        acc->mul2 = temp;
        return 1;
    }

    return 0;
}

static int mul_acc_is_mm(isl_ctx *ctx,
    isl_multi_pw_aff *mul1, isl_multi_pw_aff *mul2)
{
    isl_multi_pw_aff *acc_mul1, *acc_mul2;
    isl_multi_pw_aff *mir_mul1, *mir_mul2;
    int res = 0;

    acc_mul1 = isl_multi_pw_aff_read_from_str(ctx, acc_str_mul1_mm);
    acc_mul2 = isl_multi_pw_aff_read_from_str(ctx, acc_str_mul2_mm);

    mir_mul1 = mpa_cast(isl_multi_pw_aff_copy(mul1), 2, 3);
    mir_mul2 = mpa_cast(isl_multi_pw_aff_copy(mul2), 2, 3);

    if (isl_multi_pw_aff_is_equal(acc_mul1, mir_mul1) &&
        isl_multi_pw_aff_is_equal(acc_mul2, mir_mul2))
        res = 1;
    
    isl_multi_pw_aff_free(mir_mul1);
    isl_multi_pw_aff_free(mir_mul2);
    isl_multi_pw_aff_free(acc_mul1);
    isl_multi_pw_aff_free(acc_mul2);
    return res;
}

/*
 */
int access_is_mm(isl_ctx *ctx, struct access_mac *acc)
{
    isl_multi_pw_aff *acc_res, *mir_res, *temp;
    isl_size n;

    if (!isl_multi_pw_aff_is_equal(acc->res, acc->add))
        return 0;
    
    acc_res = isl_multi_pw_aff_read_from_str(ctx, acc_str_res_mm);
    mir_res = mpa_cast(isl_multi_pw_aff_copy(acc->res), 2, 3);
    
    if (!isl_multi_pw_aff_is_equal(mir_res, acc_res)) {
        isl_multi_pw_aff_free(mir_res);
        isl_multi_pw_aff_free(acc_res);
        return 0;
    }
    isl_multi_pw_aff_free(mir_res);
    isl_multi_pw_aff_free(acc_res);

    if (mul_acc_is_mm(ctx, acc->mul1, acc->mul2))
        return 1;
    if (mul_acc_is_mm(ctx, acc->mul2, acc->mul1)) {
        temp = acc->mul1;
        acc->mul1 = acc->mul2;
        acc->mul2 = temp;
        return 1;
    }

    return 0;
}

/*
 */
static int mul_acc_is_conv(isl_ctx *ctx,
    isl_multi_pw_aff *mul1, isl_multi_pw_aff *mul2, struct access_mac *acc)
{
    isl_multi_pw_aff *acc_mul1, *acc_mul2;
    isl_multi_pw_aff *mir_mul1, *mir_mul2;
    int res = 0;

    char acc_str_mul1[47];
    isl_space *space;
    isl_point *point;
    isl_pw_aff *pa;
    isl_val *val;
    long stride;
    int i;

    mir_mul1 = mpa_cast(isl_multi_pw_aff_copy(mul1), 3, 7);
    mir_mul2 = mpa_cast(isl_multi_pw_aff_copy(mul2), 4, 7);

    space = isl_space_set_alloc(ctx, 0, 7);
    point = isl_point_zero(space);
    for (i=0; i<7; i++)
        point = isl_point_add_ui(point, isl_dim_set, i, 1);
    pa = isl_multi_pw_aff_get_at(mir_mul1, 2);
    val = isl_pw_aff_eval(pa, point);
    stride = isl_val_get_num_si(val) - 1;
    sprintf(acc_str_mul1, acc_str_mul1_conv, stride, stride);
    acc_mul1 = isl_multi_pw_aff_read_from_str(ctx, acc_str_mul1);
    acc_mul2 = isl_multi_pw_aff_read_from_str(ctx, acc_str_mul2_conv);

    if (isl_multi_pw_aff_is_equal(acc_mul1, mir_mul1) &&
        isl_multi_pw_aff_is_equal(acc_mul2, mir_mul2)) {
        acc->strd = isl_val_copy(val);
        res = 1;
    }
    
    isl_val_free(val);
    isl_multi_pw_aff_free(mir_mul1);
    isl_multi_pw_aff_free(mir_mul2);
    isl_multi_pw_aff_free(acc_mul1);
    isl_multi_pw_aff_free(acc_mul2);
    return res;
}

/*
 */
int access_is_conv(isl_ctx *ctx, struct access_mac *acc)
{
    isl_multi_pw_aff *acc_res, *mir_res, *temp;

    if (!isl_multi_pw_aff_is_equal(acc->res, acc->add))
        return 0;
    
    acc_res = isl_multi_pw_aff_read_from_str(ctx, acc_str_res_conv);
    mir_res = mpa_cast(isl_multi_pw_aff_copy(acc->res), 3, 7);

    if (!isl_multi_pw_aff_is_equal(mir_res, acc_res)) {
        isl_multi_pw_aff_free(mir_res);
        isl_multi_pw_aff_free(acc_res);
        return 0;
    }

    isl_multi_pw_aff_free(mir_res);
    isl_multi_pw_aff_free(acc_res);

    if (mul_acc_is_conv(ctx, acc->mul1, acc->mul2, acc))
        return 1;
    if (mul_acc_is_conv(ctx, acc->mul2, acc->mul1, acc)) {
        temp = acc->mul1;
        acc->mul1 = acc->mul2;
        acc->mul2 = temp;
        return 1;
    }
    
    return 0;
}

/*
 */
void access_init_act_init(struct access_init_act *acc)
{
    acc->res = NULL;
    acc->opr = NULL;
}

/*
 */
void access_init_act_free(struct access_init_act *acc)
{
    isl_multi_pw_aff_free(acc->res);
    isl_multi_pw_aff_free(acc->opr);
    
    free(acc);
}

/*
 */
int access_is_init_act(isl_ctx *ctx, struct access_init_act *acc_init,
    struct access_mac *acc_mac, int diff)
{
    isl_multi_pw_aff *mir_mac, *mir_init;
    isl_size n;
    int res = 0;

    mir_mac = mpa_cast(isl_multi_pw_aff_copy(acc_mac->res), -1, -1);
    n = isl_multi_pw_aff_dim(mir_mac, isl_dim_in);
    if (n > diff)
        mir_mac = isl_multi_pw_aff_drop_dims(mir_mac, isl_dim_in, n-diff, diff);

    mir_init = mpa_cast(isl_multi_pw_aff_copy(acc_init->res), -1, -1);

    if (isl_multi_pw_aff_is_equal(mir_init, mir_mac))
        res = 1;
    
    isl_multi_pw_aff_free(mir_init);
    isl_multi_pw_aff_free(mir_mac);
    return res;
}

/*
 */
static int call_acc_is_pool(isl_ctx *ctx, isl_multi_pw_aff *call,
    struct access_mac *acc)
{
    isl_multi_pw_aff *acc_call, *mir_call;
    int res = 0;

    char acc_str_call[47];
    isl_space *space;
    isl_point *point;
    isl_pw_aff *pa;
    isl_val *val;
    long stride;
    int i;

    mir_call = mpa_cast(isl_multi_pw_aff_copy(call), 4, 6);

    space = isl_space_set_alloc(ctx, 0, 6);
    point = isl_point_zero(space);
    for (i=0; i<6; i++)
        point = isl_point_add_ui(point, isl_dim_set, i, 1);
    pa = isl_multi_pw_aff_get_at(mir_call, 3);
    val = isl_pw_aff_eval(pa, point);
    stride = isl_val_get_num_si(val) - 1;
    sprintf(acc_str_call, acc_str_call_pool, stride, stride);
    acc_call = isl_multi_pw_aff_read_from_str(ctx, acc_str_call);

    if (isl_multi_pw_aff_is_equal(mir_call, acc_call)) {
        acc->strd = isl_val_copy(val);
        res = 1;
    }

    isl_val_free(val);
    isl_multi_pw_aff_free(mir_call);
    isl_multi_pw_aff_free(acc_call);
    return res;
}

/*
 */
int access_is_pool(isl_ctx *ctx, struct access_mac *acc)
{
    isl_multi_pw_aff *acc_res, *mir_res;

    acc_res = isl_multi_pw_aff_read_from_str(ctx, acc_str_res_pool);
    mir_res = mpa_cast(isl_multi_pw_aff_copy(acc->res), 4, 6);

    if (!isl_multi_pw_aff_is_equal(mir_res, acc_res)) {
        isl_multi_pw_aff_free(mir_res);
        isl_multi_pw_aff_free(acc_res);
        return 0;
    }

    isl_multi_pw_aff_free(mir_res);
    isl_multi_pw_aff_free(acc_res);

    if (call_acc_is_pool(ctx, acc->add, acc))
        return 1;

    return 0;
}

int access_is_init_pooling(isl_ctx *ctx, struct access_init_act *acc)
{
    isl_multi_pw_aff *acc_res, *acc_opr;
    isl_multi_pw_aff *mir_res, *mir_opr;
    int res = 0;

    char acc_str_opr[47];
    isl_space *space;
    isl_point *point;
    isl_pw_aff *pa;
    isl_val *val;
    long stride;
    int i;
    isl_size n;

    mir_res = mpa_cast(isl_multi_pw_aff_copy(acc->res), 3, 3);
    mir_opr = mpa_cast(isl_multi_pw_aff_copy(acc->opr), 3, 3);

    n = isl_multi_pw_aff_dim(mir_opr, isl_dim_in);
    space = isl_space_set_alloc(ctx, 0, n);
    point = isl_point_zero(space);
    for (i=0; i<n; i++)
        point = isl_point_add_ui(point, isl_dim_set, i, 1);
    pa = isl_multi_pw_aff_get_at(mir_opr, n-1);
    val = isl_pw_aff_eval(pa, point);
    stride = isl_val_get_num_si(val);
    sprintf(acc_str_opr, acc_str_opr_pool_init, stride, stride);
    acc_res = isl_multi_pw_aff_read_from_str(ctx, acc_str_res_pool_init);
    acc_opr = isl_multi_pw_aff_read_from_str(ctx, acc_str_opr);

    if (isl_multi_pw_aff_is_equal(acc_res, mir_res) &&
        isl_multi_pw_aff_is_equal(acc_opr, mir_opr))
        res = 1;
    
    isl_val_free(val);
    isl_multi_pw_aff_free(mir_res);
    isl_multi_pw_aff_free(mir_opr);
    isl_multi_pw_aff_free(acc_res);
    isl_multi_pw_aff_free(acc_opr);
    return res;
}