#ifndef POLYXB_H
#define POLYXB_H

#include <isl/arg.h>
#include <isl/ctx.h>
#include <isl/printer.h>

#include "scop.h"

/* PolyXB options. */
struct polyxb_options {
    struct isl_options *isl;
    char *ctx;
    int en_tiling;
    int mm_size[3];
    int en_pipeline;
    int num_pes;
    int en_conv;
};

ISL_ARG_DECL(polyxb_options, struct polyxb_options, polyxb_options_args)

/* Overall options. */
struct options {
    struct pet_options *pet;
    struct polyxb_options *polyxb;
    char *input;
    char *output;
};

ISL_ARG_DECL(options, struct options, options_args)

/* Container for data passed to transform */
struct polyxb_transform_data {
	struct polyxb_options *options;
	__isl_give isl_printer *(*transform)(__isl_take isl_printer *p,
		struct polyxb_scop *scop, void *user);
	void *user;
};

#endif