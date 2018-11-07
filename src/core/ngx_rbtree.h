
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 *
 * @红黑树关联容器@
 * 用于 nginx 核心模块(定时器/文件缓存)的快速检索
 */


#ifndef _NGX_RBTREE_H_INCLUDED_
#define _NGX_RBTREE_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


typedef ngx_uint_t  ngx_rbtree_key_t;
typedef ngx_int_t   ngx_rbtree_key_int_t;


typedef struct ngx_rbtree_node_s  ngx_rbtree_node_t;

struct ngx_rbtree_node_s {
    ngx_rbtree_key_t       key;    /* 无符号整型关键字 */
    ngx_rbtree_node_t     *left;   /* 左子节点 */
    ngx_rbtree_node_t     *right;  /* 右子节点 */
    ngx_rbtree_node_t     *parent; /* 父节点 */
    u_char                 color;  /* 节点颜色, 0表示黑色,1表示红色 */
    u_char                 data;   /* 仅一个字节的节点数据,空间太小很少使用 */
};


typedef struct ngx_rbtree_s  ngx_rbtree_t;

/**
 * 为解决不同节点含有相同关键字的元素冲突问题,红黑树设置该回调函数可以灵活的添加冲突元素
 * nginx 提供了3种添加方法
 * 1)ngx_rbtree_insert_value   关键字唯一 
 * 2)ngx_rbtree_insert_time_value  关键字表示时间或者时间差
 * 1)ngx_str_rbtree_insert_value
 */
typedef void (*ngx_rbtree_insert_pt) (ngx_rbtree_node_t *root,
    ngx_rbtree_node_t *node, ngx_rbtree_node_t *sentinel);

struct ngx_rbtree_s {
    ngx_rbtree_node_t     *root;     /* 指向树的根节点,根节点也是数据元素 */
    ngx_rbtree_node_t     *sentinel; /* 指向NIL哨兵节点 */
    ngx_rbtree_insert_pt   insert;   /* 添加操作函数指针,决定了添加新节点的行为是替换还是新增 */
};


#define ngx_rbtree_init(tree, s, i)                                           \
    ngx_rbtree_sentinel_init(s);                                              \
    (tree)->root = s;                                                         \
    (tree)->sentinel = s;                                                     \
    (tree)->insert = i


void ngx_rbtree_insert(ngx_rbtree_t *tree, ngx_rbtree_node_t *node);
void ngx_rbtree_delete(ngx_rbtree_t *tree, ngx_rbtree_node_t *node);
void ngx_rbtree_insert_value(ngx_rbtree_node_t *root, ngx_rbtree_node_t *node,
    ngx_rbtree_node_t *sentinel);
void ngx_rbtree_insert_timer_value(ngx_rbtree_node_t *root,
    ngx_rbtree_node_t *node, ngx_rbtree_node_t *sentinel);
ngx_rbtree_node_t *ngx_rbtree_next(ngx_rbtree_t *tree,
    ngx_rbtree_node_t *node);


#define ngx_rbt_red(node)               ((node)->color = 1)
#define ngx_rbt_black(node)             ((node)->color = 0)
#define ngx_rbt_is_red(node)            ((node)->color)
#define ngx_rbt_is_black(node)          (!ngx_rbt_is_red(node))
#define ngx_rbt_copy_color(n1, n2)      (n1->color = n2->color)


/* a sentinel must be black */

#define ngx_rbtree_sentinel_init(node)  ngx_rbt_black(node)


static ngx_inline ngx_rbtree_node_t *
ngx_rbtree_min(ngx_rbtree_node_t *node, ngx_rbtree_node_t *sentinel)
{
    while (node->left != sentinel) {
        node = node->left;
    }

    return node;
}


#endif /* _NGX_RBTREE_H_INCLUDED_ */
