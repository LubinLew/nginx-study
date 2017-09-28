
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_LIST_H_INCLUDED_
#define _NGX_LIST_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>

/* 名词定义:
 * 链表元素: 一个 ngx_list_part_t 即 一个链表元素
 * 内存元素: 一个 ngx_list_part_t 管理了 ngx_list_t.nalloc 个内存元素
 */
 
typedef struct ngx_list_part_s  ngx_list_part_t;

/* 链表(内存元素管理)结构
 * 该结构管理了ngx_list_t.nalloc个内存元素;所有的内存元素在内存中是连续的,与数组相同
 * 每个内存元素的长度不能超过 ngx_list_t.size;超过了则破环了链表结构(存下一个内存元素时就破环了上一个)
 * 当成员nelts等于ngx_list_t.nalloc时,表示当前ngx_list_part_t管理的内存元素耗尽,需要再申请一个 ngx_list_part_t)
*/
struct ngx_list_part_s {
    void             *elts; //该链表管理的内存的首地址,通过 elts + ((n-1) * ngx_list_t.size)即可计算第n个内存元素的首地址
    ngx_uint_t        nelts;//该链表已经使用了多少个内存元素, 最大值为 ngx_list_t.nalloc
    ngx_list_part_t  *next; //下一个链表元素 ngx_list_part_t 的地址
};

/* 链表(ngx_list_part_t)的管理结构
 *
 */
typedef struct {
    ngx_list_part_t  *last;   //最后一个链表元素(last->next = NULL)
    ngx_list_part_t   part;   //首个链表元素
    size_t            size;   //一个内存元素的最大长度
    ngx_uint_t        nalloc; //一个链表元素中的最大内存元素个数
    ngx_pool_t       *pool;   //内存池对象, ngx_list_t 中所有的内存都是从pool中申请的
} ngx_list_t;


ngx_list_t *ngx_list_create(ngx_pool_t *pool, ngx_uint_t n, size_t size);

static ngx_inline ngx_int_t
ngx_list_init(ngx_list_t *list, ngx_pool_t *pool, ngx_uint_t n, size_t size)
{
    list->part.elts = ngx_palloc(pool, n * size);
    if (list->part.elts == NULL) {
        return NGX_ERROR;
    }

    list->part.nelts = 0;
    list->part.next = NULL;
    list->last = &list->part;
    list->size = size;
    list->nalloc = n;
    list->pool = pool;

    return NGX_OK;
}


/*
 *
 *  the iteration through the list:
 *
 *  part = &list.part;
 *  data = part->elts;
 *
 *  for (i = 0 ;; i++) {
 *
 *      if (i >= part->nelts) {
 *          if (part->next == NULL) {
 *              break;
 *          }
 *
 *          part = part->next;
 *          data = part->elts;
 *          i = 0;
 *      }
 *
 *      ...  data[i] ...
 *
 *  }
 */


void *ngx_list_push(ngx_list_t *list);


#endif /* _NGX_LIST_H_INCLUDED_ */
