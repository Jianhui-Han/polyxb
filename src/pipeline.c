#include <math.h>

#include <isl/union_map.h>

#include "info.h"
#include "node.h"
#include "pipeline.h"
#include "polyxb.h"
#include "scop.h"

/* Allocate the PE resources among all the matched nodes.
 */
void do_allocate(__isl_keep isl_schedule_node *node,
    struct ast_info **infos, void *user)
{
    struct polyxb_scop *ps = user;
    isl_ctx *ctx = isl_schedule_node_get_ctx(node);
    isl_val *val;
    isl_schedule_node *child = isl_schedule_node_get_child(node, 0);
    int n = isl_schedule_node_n_children(child);
    int *n_tile = (int*)malloc(sizeof(int)*n);
    int *reuse = (int*)malloc(sizeof(int)*n);
    int *degree = (int*)malloc(sizeof(int)*n);
    int *latency = (int*)malloc(sizeof(int)*n);
    int *maximum = (int*)malloc(sizeof(int)*n);
    int i, m, c, incr, used, base;
    int minimum = 0;

    for (i=0; i<n; i++) {
        val = isl_val_mul(
            isl_val_copy(infos[i]->sizes_tiled[1]),
            isl_val_copy(infos[i]->sizes_tiled[2]));
        n_tile[i] = isl_val_get_num_si(val);
        isl_val_free(val);
        reuse[i] = isl_val_get_num_si(infos[i]->sizes_tiled[0]);
    }

    for (i=0; i<n; i++)
        minimum += n_tile[i];
    if (minimum > ps->options->num_pes) {
        fprintf(stderr, "Number of PEs less than the minimum requirement. Mapping failed.\n");
        exit(0);
    }

    for (i=0; i<n; i++)
        degree[i] = 1;
    
    while (1) {
        for (i=0; i<n; i++)
            latency[i] = (int)ceil(((double)reuse[i])/degree[i]);

        m = 0;
        for (i=0; i<n; i++)
            if (latency[i] > m)
                m = latency[i];
        c = 0;
        for (i=0; i<n; i++)
            if (latency[i] == m)
                maximum[c++] = i;
        
        incr = 0;
        for (i=0; i<c; i++)
            incr += n_tile[maximum[i]];

        used = 0;
        for (i=0; i<n; i++)
            used += degree[i] * n_tile[i];
        
        if (used + incr > ps->options->num_pes) {
            base = 0;
            for (i=0; i<n; i++) {
                infos[i]->is_pipe = 1;
                infos[i]->n_stage = i+1;
                infos[i]->n_pes = degree[i] * n_tile[i];
                infos[i]->a_base = base;
                base += infos[i]->n_pes;
            }
            break;
        }

        for (i=0; i<c; i++)
            degree[maximum[i]] += 1;
    }

    free(maximum);
    free(latency);
    free(degree);
    free(reuse);
    free(n_tile);

    isl_schedule_node_free(child);
}

/*
 */
__isl_give isl_multi_union_pw_aff *build_pipeline_partial(
    __isl_keep isl_schedule_node *node, void *user)
{
    isl_union_map *umap;
    isl_multi_union_pw_aff *mupa;
    isl_schedule_node *child, *grand;
    isl_size n = isl_schedule_node_n_children(node);
    int i;

    for (i=0; i<n; i++) {
        child = isl_schedule_node_get_child(node, i);
        grand = isl_schedule_node_child(child, 0);
        if (i == 0)
            umap = isl_schedule_node_band_get_partial_schedule_union_map(
                grand);
        else
            umap = isl_union_map_union(umap,
                isl_schedule_node_band_get_partial_schedule_union_map(grand));
        isl_schedule_node_free(grand);
    }

    mupa = isl_multi_union_pw_aff_from_union_map(umap);

    return mupa;
}