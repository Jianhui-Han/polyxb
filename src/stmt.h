#ifndef STMT_H 
#define STMT_H

#include "access.h"

__isl_give struct access_mac *stmt_is_mac(struct pet_stmt *stmt);
__isl_give struct access_init_act *stmt_is_init(struct pet_stmt *stmt);
__isl_give struct access_init_act *stmt_is_act(struct pet_stmt *stmt);

__isl_give isl_id *stmt_get_act(struct pet_stmt *stmt);

__isl_give struct access_init_act *stmt_is_init_pooling(struct pet_stmt *stmt);
__isl_give struct access_mac *stmt_is_mac_pooling(struct pet_stmt *stmt);
#endif