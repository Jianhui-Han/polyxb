#include <isl/union_map.h>
#include <isl/union_set.h>

#include "node.h"

/*
 */
int node_match_structure(__isl_keep isl_schedule_node *node, const char *desc)
{
    int child[MAX_NODES];
    int type[MAX_NODES];
    int m = 0, n = 0, s = 0, i;
    isl_schedule_node *stack[MAX_NODES];
    int visited[MAX_NODES];
    isl_schedule_node *list[MAX_NODES];
    isl_size c;

    /* Parse description string. */
    while (*desc) {
        if (*desc == 'b') {
            type[n] = isl_schedule_node_band;
            child[n++] = 1;
        }
        else if (*desc == 's') {
            type[n] = isl_schedule_node_sequence;
            child[n++] = *(++desc) - '0';
        }
        else if (*desc == 'l') {
            type[n] = isl_schedule_node_leaf;
            child[n++] = 0;
        }
        else if (*desc == 'f') {
            type[n] = isl_schedule_node_filter;
            child[n++] = 1;
        }
        desc++;
    }

    /* Traverse the substree and store to the list. */
    list[m++] = isl_schedule_node_copy(node);
    stack[0] = isl_schedule_node_copy(node);
    visited[0] = 0;
    while (s >= 0) {
        c = isl_schedule_node_n_children(stack[s]);
        if (c > visited[s]) {
            list[m] = isl_schedule_node_child(
                isl_schedule_node_copy(stack[s]), visited[s]);
            visited[s]++;
            stack[++s] = isl_schedule_node_copy(list[m++]);
            visited[s] = 0;
        }
        else
            isl_schedule_node_free(stack[s--]);
    }

    if (m != n)
        goto clean;

    /* Compare according to the list. */
    for (i=0; i<n; i++) {
        if ((isl_schedule_node_get_type(list[i]) != type[i]) ||
            (isl_schedule_node_n_children(list[i]) != child[i]))
            goto clean;
    }

    for (i=0; i<m; i++)
        isl_schedule_node_free(list[i]);
    return 1;
clean:
    for (i=0; i<m; i++)
        isl_schedule_node_free(list[i]);
    return 0;
}

/*
 */
__isl_give isl_schedule_node *node_get_subnode(
    __isl_keep isl_schedule_node *node, const char *desc, int pos)
{
    int child[MAX_NODES];
    int n = 0, s = 0, m = 0, i;
    isl_size c;
    isl_schedule_node *stack[MAX_NODES];
    int visited[MAX_NODES];
    isl_schedule_node *res;

    /* Parse description string. */
    while (*desc) {
        if (*desc == 'b')
            child[n++] = 1;
        else if (*desc == 's')
            child[n++] = *(++desc) - '0';
        else if (*desc == 'l')
            child[n++] = 0;
        else if (*desc == 'f')
            child[n++] = 1;
        desc++;
    }

    /* Traverse the substree and store to the list. */
    stack[0] = isl_schedule_node_copy(node);
    visited[0] = 0;
    while (s >= 0) {
        c = isl_schedule_node_n_children(stack[s]);
        if (c > visited[s]) {
            stack[s+1] = isl_schedule_node_child(
                isl_schedule_node_copy(stack[s]), visited[s]);
            visited[s]++;
            visited[++s] = 0;
            if (m == pos-1) {
                res = stack[s];
                break;
            }
            m++;
        }
        else
            isl_schedule_node_free(stack[s--]);
    }

    for (i=0; i<s; i++)
        isl_schedule_node_free(stack[i]);
    return res;
}

/*
 */
__isl_give isl_basic_set *node_band_get_domain(
    __isl_keep isl_schedule_node *node)
{
    isl_union_map *umap;
    isl_union_set *uset;
    isl_basic_set_list *list;
    isl_basic_set *bset;

    umap = isl_schedule_node_band_get_partial_schedule_union_map(node);
    uset = isl_union_set_intersect(isl_union_map_domain(umap),
        isl_schedule_node_get_domain(node));
    list = isl_union_set_get_basic_set_list(uset);
    bset = isl_basic_set_list_get_at(list, 0);

    isl_union_set_free(uset);
    isl_basic_set_list_free(list);
    return bset;
}

/*
 */
__isl_give isl_basic_set *node_filter_get_domain(
    __isl_keep isl_schedule_node *node)
{
    isl_union_set *uset;
    isl_basic_set_list *list;
    isl_basic_set *bset;

    uset = isl_union_set_intersect(isl_schedule_node_get_domain(node),
        isl_schedule_node_filter_get_filter(node));
    list = isl_union_set_get_basic_set_list(uset);
    bset = isl_basic_set_list_get_at(list, 0);

    isl_union_set_free(uset);
    isl_basic_set_list_free(list);
    return bset;
}

/*
 */
int node_match_structure_pipeline(__isl_keep isl_schedule_node *node,
    struct ast_info ***infos)
{
    isl_size n = isl_schedule_node_n_children(node);
    *infos = (struct ast_info**)malloc(sizeof(struct ast_info*)*n);
    isl_schedule_node *child, *grand, *ggrand;
    isl_id *id;
    int i;

    if (isl_schedule_node_get_type(node) != isl_schedule_node_band)
        return 0;
    
    child = isl_schedule_node_get_child(node, 0);
    n = isl_schedule_node_n_children(child);
    if (n == 0) {
        isl_schedule_node_free(child);
        return 0;
    }

    for (i=0; i<n; i++) {
        grand = isl_schedule_node_get_child(child, i);
        if (isl_schedule_node_get_type(grand) != isl_schedule_node_filter) {
            isl_schedule_node_free(grand);
            isl_schedule_node_free(child);
            return 0;
        }
        ggrand = isl_schedule_node_get_child(grand, 0);
        if (isl_schedule_node_get_type(ggrand) != isl_schedule_node_mark) {
            isl_schedule_node_free(ggrand);
            isl_schedule_node_free(grand);
            isl_schedule_node_free(child);
            return 0;
        }
        id = isl_schedule_node_mark_get_id(ggrand);
        (*infos)[i] = isl_id_get_user(id);
        isl_id_free(id);
        isl_schedule_node_free(ggrand);
        isl_schedule_node_free(grand);
    }

    isl_schedule_node_free(child);
    return n;
}