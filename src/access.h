#ifndef ACCESS_H
#define ACCESS_H

#include <isl/aff.h>

struct access_mac {
    isl_multi_pw_aff *mul1;
    isl_multi_pw_aff *mul2;
    isl_multi_pw_aff *add;
    isl_multi_pw_aff *res;
    isl_val *strd;
};

void access_mac_init(struct access_mac *acc);
void access_mac_free(struct access_mac *acc);

int access_is_mv(isl_ctx *ctx, struct access_mac *acc);
int access_is_mm(isl_ctx *ctx, struct access_mac *acc);
int access_is_conv(isl_ctx *ctx, struct access_mac *acc);

struct access_init_act {
    isl_multi_pw_aff *res;
    isl_multi_pw_aff *opr;
};

void access_init_act_init(struct access_init_act *acc);
void access_init_act_free(struct access_init_act *acc);

int access_is_init_act(isl_ctx *ctx, struct access_init_act *acc_init,
    struct access_mac *acc_mac, int diff);

int access_is_pool(isl_ctx *ctx, struct access_mac *acc);
int access_is_init_pooling(isl_ctx *ctx, struct access_init_act *acc);

#endif