#include <isl/ctx.h>
#include <isl/id.h>
#include <isl/schedule_node.h>
#include <isl/set.h>

#include "access.h"
#include "info.h"
#include "match.h"
#include "node.h"
#include "polyxb.h"
#include "scop.h"
#include "stmt.h"
#include "utils.h"

#if DEBUG
#define HEADER_DEBUG_INFO(FUNC) fprintf(stderr, "Matching: %s: ", FUNC);
#define TAILER_DEBUG_INFO_FOUND fprintf(stderr, "Found\n");
#define TAILER_DEBUG_INFO_NOT_FOUND fprintf(stderr, "Not found\n");
#else
#define HEADER_DEBUG_INFO(FUNC) ;
#define TAILER_DEBUG_INFO_FOUND ;
#define TAILER_DEBUG_INFO_NOT_FOUND ;
#endif

#define HEADER_COMMON(FUNC) HEADER_DEBUG_INFO(FUNC) \
    struct polyxb_scop *ps = user; \
    isl_ctx *ctx = isl_schedule_node_get_ctx(node); \
    isl_basic_set *domain; \
    struct pet_stmt *stmt; \
    isl_schedule_node *mark_node = NULL; \
    struct ast_info *info;

#define MATCH_HEADER_BARE(FUNC) HEADER_COMMON(FUNC) \
    struct access_mac *acc_mac;

#define MATCH_HEADER_INIT(FUNC) MATCH_HEADER_BARE(FUNC) \
    struct access_init_act *acc_init; \
    isl_schedule_node *filter;

#define MATCH_HEADER_ACT(FUNC) MATCH_HEADER_BARE(FUNC) \
    struct access_init_act *acc_act; \
    isl_schedule_node *filter; \
    isl_id *act_funct;

#define MATCH_HEADER_INIT_ACT(FUNC) MATCH_HEADER_BARE(FUNC) \
    struct access_init_act *acc_init; \
    struct access_init_act *acc_act; \
    isl_schedule_node *filter; \
    isl_id *act_funct;

#define CL_ACC1_BARE ;
#define CL_ACC1_INIT access_init_act_free(acc_init);
#define CL_ACC1_ACT access_init_act_free(acc_act);
#define CL_ACC1_INIT_ACT access_init_act_free(acc_init); \
    access_init_act_free(acc_act);

#define CL_ACC2_BARE ;
#define CL_ACC2_INIT \
clean_acc_init: \
    access_init_act_free(acc_init);
#define CL_ACC2_ACT \
clean_acc_act: \
    access_init_act_free(acc_act); \
    isl_id_free(act_funct);
#define CL_ACC2_INIT_ACT \
clean_acc_act: \
    access_init_act_free(acc_act); \
clean_acc_init: \
    access_init_act_free(acc_init); \
    isl_id_free(act_funct);

#define MATCH_TAILER_COMMON(CL_ACC1, CL_ACC2) \
    access_mac_free(acc_mac); \
    CL_ACC1 \
    isl_basic_set_free(domain); \
    TAILER_DEBUG_INFO_FOUND \
    return mark_node; \
clean_acc_mac: \
    access_mac_free(acc_mac); \
    CL_ACC2 \
clean_domain: \
    isl_basic_set_free(domain); \
clean: \
    TAILER_DEBUG_INFO_NOT_FOUND \
    return node;

/*
 */
__isl_give isl_schedule_node *match_mv_bare(
    __isl_take isl_schedule_node *node, void *user)
{
    MATCH_HEADER_BARE("mv_bare")

    /* Check structure of the node. */
    if (!node_match_structure(node, "b-b-l"))
        goto clean;

    /* Check operator type. */
    domain = node_band_get_domain(node);
    stmt = domain_get_stmt(domain, ps);
    acc_mac = stmt_is_mac(stmt);
    if (!acc_mac)
        goto clean_domain;

    /* Check access relation. */
    if (!access_is_mv(ctx, acc_mac))
        goto clean_acc_mac;
    
    /* Prepare data for build function. */
    info = get_ast_info(ctx, domain, acc_mac, 8, 0, NULL, ps);
    mark_node = isl_schedule_node_insert_mark(node,
        isl_id_alloc(ctx, "xbblas_mv_bare", info));
    
    MATCH_TAILER_COMMON(CL_ACC1_BARE, CL_ACC2_BARE)
}

/*
 */
__isl_give isl_schedule_node *match_mv_init(
    __isl_take isl_schedule_node *node, void *user)
{
    MATCH_HEADER_INIT("mv_init")

    /* Check structure of the node. */
    if (!node_match_structure(node, "b-s2-f-l-f-b-l"))
        goto clean;

    /* Check operator type. */
    filter = node_get_subnode(node, "b-s2-f-l-f-b-l", 2);
    domain = node_filter_get_domain(filter);
    isl_schedule_node_free(filter);
    stmt = domain_get_stmt(domain, ps);
    acc_init = stmt_is_init(stmt);
    if (!acc_init)
        goto clean_domain;

    isl_basic_set_free(domain);
    filter = node_get_subnode(node, "b-s2-f-l-f-b-l", 4);
    domain = node_filter_get_domain(filter);
    isl_schedule_node_free(filter);
    stmt = domain_get_stmt(domain, ps);
    acc_mac = stmt_is_mac(stmt);
    if (!acc_mac)
        goto clean_acc_init;
    
    /* Check access relation. */
    if (!access_is_mv(ctx, acc_mac))
        goto clean_acc_mac;
    if (!access_is_init_act(ctx, acc_init, acc_mac, 1))
        goto clean_acc_mac;
    
    /* Prepare data for build function. */
    info = get_ast_info(ctx, domain, acc_mac, 8, 1, NULL, ps);
    mark_node = isl_schedule_node_insert_mark(node,
        isl_id_alloc(ctx, "xbblas_mv_bare", info));

    MATCH_TAILER_COMMON(CL_ACC1_INIT, CL_ACC2_INIT)
}

/*
 */
__isl_give isl_schedule_node *match_mm_bare(
    __isl_take isl_schedule_node *node, void *user)
{
    MATCH_HEADER_BARE("mm_bare")

    /* Check structure of the node. */
    if (!node_match_structure(node, "b-b-b-l"))
        goto clean;

    /* Check operator type. */
    domain = node_band_get_domain(node);
    stmt = domain_get_stmt(domain, ps);
    acc_mac = stmt_is_mac(stmt);
    if (!acc_mac)
        goto clean_domain;

    /* Check access relation. */
    if (!access_is_mm(ctx, acc_mac))
        goto clean_acc_mac;
    
    /* Prepare data for build function. */
    info = get_ast_info(ctx, domain, acc_mac, 9, 0, NULL, ps);
    mark_node = isl_schedule_node_insert_mark(node,
        isl_id_alloc(ctx, "xbblas_mm_bare", info));

    MATCH_TAILER_COMMON(CL_ACC1_BARE, CL_ACC2_BARE)
}

/*
 */
__isl_give isl_schedule_node *match_mm_init(
    __isl_take isl_schedule_node *node, void *user)
{
    MATCH_HEADER_INIT("mm_init")

    /* Check structure of the node. */
    if (!node_match_structure(node, "b-b-s2-f-l-f-b-l"))
        goto clean;
    
    /* Check operator type. */
    filter = node_get_subnode(node, "b-b-s2-f-l-f-b-l", 3);
    domain = node_filter_get_domain(filter);
    isl_schedule_node_free(filter);
    stmt = domain_get_stmt(domain, ps);
    acc_init = stmt_is_init(stmt);
    if (!acc_init)
        goto clean_domain;
    
    isl_basic_set_free(domain);
    filter = node_get_subnode(node, "b-b-s2-f-l-f-b-l", 5);
    domain = node_filter_get_domain(filter);
    isl_schedule_node_free(filter);
    stmt = domain_get_stmt(domain, ps);
    acc_mac = stmt_is_mac(stmt);
    if (!acc_mac)
        goto clean_acc_init;
    
    /* Check access relation. */
    if (!access_is_mm(ctx, acc_mac))
        goto clean_acc_mac;
    if (!access_is_init_act(ctx, acc_init, acc_mac, 1))
        goto clean_acc_mac;
    
    /* Prepare data for build function. */
    info = get_ast_info(ctx, domain, acc_mac, 9, 1, NULL, ps);
    mark_node = isl_schedule_node_insert_mark(node,
        isl_id_alloc(ctx, "xbblas_mm_bare", info));

    MATCH_TAILER_COMMON(CL_ACC1_INIT, CL_ACC2_INIT)
}

/*
 */
__isl_give isl_schedule_node *match_mm_act(
    __isl_take isl_schedule_node *node, void *user)
{
    MATCH_HEADER_ACT("mm_act")

    /* Check structure of the node. */
    if (!node_match_structure(node, "b-b-s2-f-b-l-f-l"))
        goto clean;
    
    /* Check operator type. */
    filter = node_get_subnode(node, "b-b-s2-f-b-l-f-l", 6);
    domain = node_filter_get_domain(filter);
    isl_schedule_node_free(filter);
    stmt = domain_get_stmt(domain, ps);
    acc_act = stmt_is_act(stmt);
    if (!acc_act)
        goto clean_domain;
    act_funct = stmt_get_act(stmt);
    
    isl_basic_set_free(domain);
    filter = node_get_subnode(node, "b-b-s2-f-b-l-f-l", 3);
    domain = node_filter_get_domain(filter);
    isl_schedule_node_free(filter);
    stmt = domain_get_stmt(domain, ps);
    acc_mac = stmt_is_mac(stmt);
    if (!acc_mac)
        goto clean_acc_act;
    
    /* Check access relation. */
    if (!access_is_mm(ctx, acc_mac))
        goto clean_acc_mac;
    if (!access_is_init_act(ctx, acc_act, acc_mac, 1))
        goto clean_acc_mac;

    /* Prepare data for build function. */
    info = get_ast_info(ctx, domain, acc_mac, 10, 0, act_funct, ps);
    mark_node = isl_schedule_node_insert_mark(node,
        isl_id_alloc(ctx, "xbblas_mm_act", info));

    MATCH_TAILER_COMMON(CL_ACC1_ACT, CL_ACC2_ACT);
}

/*
 */
__isl_give isl_schedule_node *match_mm_init_act(
    __isl_take isl_schedule_node *node, void *user)
{
    MATCH_HEADER_INIT_ACT("mm_init_act")

    /* Check structure of the node. */
    if (!node_match_structure(node, "b-b-s3-f-l-f-b-l-f-l"))
        goto clean;
    
    /* Check operator type. */
    filter = node_get_subnode(node, "b-b-s3-f-l-f-b-l-f-l", 3);
    domain = node_filter_get_domain(filter);
    isl_schedule_node_free(filter);
    stmt = domain_get_stmt(domain, ps);
    acc_init = stmt_is_init(stmt);
    if (!acc_init)
        goto clean_domain;
    
    isl_basic_set_free(domain);
    filter = node_get_subnode(node, "b-b-s3-f-l-f-b-l-f-l", 8);
    domain = node_filter_get_domain(filter);
    isl_schedule_node_free(filter);
    stmt = domain_get_stmt(domain, ps);
    acc_act = stmt_is_act(stmt);
    if (!acc_act)
        goto clean_acc_init;
    act_funct = stmt_get_act(stmt);
    
    isl_basic_set_free(domain);
    filter = node_get_subnode(node, "b-b-s3-f-l-f-b-l-f-l", 5);
    domain = node_filter_get_domain(filter);
    isl_schedule_node_free(filter);
    stmt = domain_get_stmt(domain, ps);
    acc_mac = stmt_is_mac(stmt);
    if (!acc_mac)
        goto clean_acc_act;
    
    /* Check access relation. */
    if (!access_is_mm(ctx, acc_mac))
        goto clean_acc_mac;
    if (!access_is_init_act(ctx, acc_init, acc_mac, 1))
        goto clean_acc_mac;
    if (!access_is_init_act(ctx, acc_act, acc_mac, 1))
        goto clean_acc_mac;
    
    /* Prepare data for build function. */
    info = get_ast_info(ctx, domain, acc_mac, 10, 1, act_funct, ps);
    mark_node = isl_schedule_node_insert_mark(node,
        isl_id_alloc(ctx, "xbblas_mm_act", info));

    MATCH_TAILER_COMMON(CL_ACC1_INIT_ACT, CL_ACC2_INIT_ACT)
}

/*
 */
__isl_give isl_schedule_node *match_conv_bare(
    __isl_take isl_schedule_node *node, void *user)
{
    MATCH_HEADER_BARE("conv_bare")

    /* Check structure of the node. */
    if (!node_match_structure(node, "b-b-b-b-b-b-l"))
        goto clean;

    /* Check operator type. */
    domain = node_band_get_domain(node);
    stmt = domain_get_stmt(domain, ps);
    acc_mac = stmt_is_mac(stmt);
    if (!acc_mac)
        goto clean_domain;
    
    /* Check access relation. */
    if (!access_is_conv(ctx, acc_mac))
        goto clean_acc_mac;
    
    /* Prepare data for build function. */
    info = get_ast_info_conv(ctx, domain, acc_mac, 13, 0, NULL, ps);
    if (ps->options->en_conv)
        mark_node = isl_schedule_node_insert_mark(node,
            isl_id_alloc(ctx, "xbblas_conv_bare", info));
    else
        mark_node = isl_schedule_node_insert_mark(node,
            isl_id_alloc(ctx, "xbblas_mm_bare", info));

    MATCH_TAILER_COMMON(CL_ACC1_BARE, CL_ACC2_BARE)
}

__isl_give isl_schedule_node *match_conv_init(
    __isl_take isl_schedule_node *node, void *user)
{
    MATCH_HEADER_INIT("conv_init")

    /* Check structure of the node. */
    if (!node_match_structure(node, "b-b-b-s2-f-l-f-b-b-b-l"))
        goto clean;

    /* Check operator type. */
    filter = node_get_subnode(node, "b-b-b-s2-f-l-f-b-b-b-l", 4);
    domain = node_filter_get_domain(filter);
    isl_schedule_node_free(filter);
    stmt = domain_get_stmt(domain, ps);
    acc_init = stmt_is_init(stmt);
    if (!acc_init)
        goto clean_domain;
    
    isl_basic_set_free(domain);
    filter = node_get_subnode(node, "b-b-b-s2-f-l-f-b-b-b-l", 6);
    domain = node_filter_get_domain(filter);
    isl_schedule_node_free(filter);
    stmt = domain_get_stmt(domain, ps);
    acc_mac = stmt_is_mac(stmt);
    if (!acc_mac)
        goto clean_acc_init;
    
    /* Check access relation. */
    if (!access_is_conv(ctx, acc_mac))
        goto clean_acc_mac;
    if (!access_is_init_act(ctx, acc_init, acc_mac, 3))
        goto clean_acc_mac;
    
    /* Prepare data for build function. */
    info = get_ast_info_conv(ctx, domain, acc_mac, 13, 1, NULL, ps);
    if (ps->options->en_conv)
        mark_node = isl_schedule_node_insert_mark(node,
            isl_id_alloc(ctx, "xbblas_conv_bare", info));
    else
        mark_node = isl_schedule_node_insert_mark(node,
            isl_id_alloc(ctx, "xbblas_mm_bare", info));

    MATCH_TAILER_COMMON(CL_ACC1_INIT, CL_ACC2_INIT)
}

__isl_give isl_schedule_node *match_conv_act(
    __isl_take isl_schedule_node *node, void *user)
{
    MATCH_HEADER_ACT("conv_act")

    /* Check structure of the node. */
    if (!node_match_structure(node, "b-b-b-s2-f-b-b-b-l-f-l"))
        goto clean;
    
    /* Check operator type. */
    filter = node_get_subnode(node, "b-b-b-s2-f-b-b-b-l-f-l", 9);
    domain = node_filter_get_domain(filter);
    isl_schedule_node_free(filter);
    stmt = domain_get_stmt(domain, ps);
    acc_act = stmt_is_act(stmt);
    if (!acc_act)
        goto clean_domain;
    act_funct = stmt_get_act(stmt);
    
    isl_basic_set_free(domain);
    filter = node_get_subnode(node, "b-b-b-s2-f-b-b-b-l-f-l", 4);
    domain = node_filter_get_domain(filter);
    isl_schedule_node_free(filter);
    stmt = domain_get_stmt(domain, ps);
    acc_mac = stmt_is_mac(stmt);
    if (!acc_mac)
        goto clean_acc_act;
    
    /* Check access relation. */
    if (!access_is_conv(ctx, acc_mac))
        goto clean_acc_mac;
    if (!access_is_init_act(ctx, acc_act, acc_mac, 3))
        goto clean_acc_mac;
    
    /* Prepare data for build function. */
    info = get_ast_info_conv(ctx, domain, acc_mac, 14, 0, act_funct, ps);
    if (ps->options->en_conv)
        mark_node = isl_schedule_node_insert_mark(node,
            isl_id_alloc(ctx, "xbblas_conv_act", info));
    else
        mark_node = isl_schedule_node_insert_mark(node,
            isl_id_alloc(ctx, "xbblas_mm_act", info));

    MATCH_TAILER_COMMON(CL_ACC1_ACT, CL_ACC2_ACT);

}

__isl_give isl_schedule_node *match_conv_init_act(
    __isl_take isl_schedule_node *node, void *user)
{
    MATCH_HEADER_INIT_ACT("conv_init_act")

    /* Check structure of the node. */
    if (!node_match_structure(node, "b-b-b-s3-f-l-f-b-b-b-l-f-l"))
        goto clean;
    
    /* Check operator type. */
    filter = node_get_subnode(node, "b-b-b-s3-f-l-f-b-b-b-l-f-l", 4);
    domain = node_filter_get_domain(filter);
    isl_schedule_node_free(filter);
    stmt = domain_get_stmt(domain, ps);
    acc_init = stmt_is_init(stmt);
    if (!acc_init)
        goto clean_domain;
    
    isl_basic_set_free(domain);
    filter = node_get_subnode(node, "b-b-b-s3-f-l-f-b-b-b-l-f-l", 11);
    domain = node_filter_get_domain(filter);
    isl_schedule_node_free(filter);
    stmt = domain_get_stmt(domain, ps);
    acc_act = stmt_is_act(stmt);
    if (!acc_act)
        goto clean_acc_init;
    act_funct = stmt_get_act(stmt);
    
    isl_basic_set_free(domain);
    filter = node_get_subnode(node, "b-b-b-s3-f-l-f-b-b-b-l-f-l", 6);
    domain = node_filter_get_domain(filter);
    isl_schedule_node_free(filter);
    stmt = domain_get_stmt(domain, ps);
    acc_mac = stmt_is_mac(stmt);
    if (!acc_mac)
        goto clean_acc_act;
    
    /* Check access relation. */
    if (!access_is_conv(ctx, acc_mac))
        goto clean_acc_mac;
    if (!access_is_init_act(ctx, acc_init, acc_mac, 3))
        goto clean_acc_mac;
    if (!access_is_init_act(ctx, acc_act, acc_mac, 3))
        goto clean_acc_mac;
    
    /* Prepare data for build function. */
    info = get_ast_info_conv(ctx, domain, acc_mac, 14, 1, act_funct, ps);
    if (ps->options->en_conv)
        mark_node = isl_schedule_node_insert_mark(node,
            isl_id_alloc(ctx, "xbblas_conv_act", info));
    else
        mark_node = isl_schedule_node_insert_mark(node,
            isl_id_alloc(ctx, "xbblas_mm_act", info));

    MATCH_TAILER_COMMON(CL_ACC1_INIT_ACT, CL_ACC2_INIT_ACT)
}

__isl_give isl_schedule_node *match_pooling(
    __isl_take isl_schedule_node *node, void *user)
{
    MATCH_HEADER_INIT_ACT("pooling")

    /* Check structure of the node. */
    if (!node_match_structure(node, "b-b-b-s2-f-l-f-b-b-l"))
        goto clean;
    
    /* Check operator type. */
    filter = node_get_subnode(node, "b-b-b-s2-f-l-f-b-b-l", 4);
    domain = node_filter_get_domain(filter);
    isl_schedule_node_free(filter);
    stmt = domain_get_stmt(domain, ps);
    acc_init = stmt_is_init_pooling(stmt);
    if (!acc_init)
        goto clean_domain;
    
    isl_basic_set_free(domain);
    filter = node_get_subnode(node, "b-b-b-s2-f-l-f-b-b-l", 6);
    domain = node_filter_get_domain(filter);
    isl_schedule_node_free(filter);
    stmt = domain_get_stmt(domain, ps);
    acc_mac = stmt_is_mac_pooling(stmt);
    if (!acc_mac)
        goto clean_acc_init;
    
    /* Check access relation. */
    if (!access_is_pool(ctx, acc_mac))
        goto clean_acc_mac;
    if (!access_is_init_pooling(ctx, acc_init))
        goto clean_acc_mac;
    
    /* Prepare data for build function. */
    info = get_ast_info_pool(ctx, domain, acc_mac, 8, ps);
    mark_node = isl_schedule_node_insert_mark(node,
        isl_id_alloc(ctx, "xbblas_pooling", info));
    
    MATCH_TAILER_COMMON(CL_ACC1_INIT, CL_ACC2_INIT)
}