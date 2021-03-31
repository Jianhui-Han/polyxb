#ifndef TACTICS_H
#define TACTICS_H

#include <isl/ctx.h>
#include <isl/schedule.h>

#include "polyxb.h"

__isl_give isl_schedule *do_tactics(__isl_take isl_schedule *schedule,
    struct polyxb_scop *ps);

#endif