
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_PALLOC_H_INCLUDED_
#define _NGX_PALLOC_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


/*
 * NGX_MAX_ALLOC_FROM_POOL should be (ngx_pagesize - 1), i.e. 4095 on x86.
 * On Windows NT it decreases a number of locked pages in a kernel.
 */
#define NGX_MAX_ALLOC_FROM_POOL  (ngx_pagesize - 1)

#define NGX_DEFAULT_POOL_SIZE    (16 * 1024)

#define NGX_POOL_ALIGNMENT       16
#define NGX_MIN_POOL_SIZE                                                     \
    ngx_align((sizeof(ngx_pool_t) + 2 * sizeof(ngx_pool_large_t)),            \
              NGX_POOL_ALIGNMENT)

/* 名词定义:
 * 内存块: 一个 ngx_pool_data_t 即 一个内存块
 * 大块内存: 链表 ngx_pool_large_t 的一个节点 就是 一个大块内存
 * 内存池: 即ngx_pool_t, 他管理内存块 和 大块内存
 */

typedef void (*ngx_pool_cleanup_pt)(void *data);

typedef struct ngx_pool_cleanup_s  ngx_pool_cleanup_t;

struct ngx_pool_cleanup_s {
    ngx_pool_cleanup_pt   handler;
    void                 *data;
    ngx_pool_cleanup_t   *next;
};


typedef struct ngx_pool_large_s  ngx_pool_large_t;

struct ngx_pool_large_s {
    ngx_pool_large_t     *next;
    void                 *alloc;
};

/* 内存块结构
 *
 */
typedef struct {
    u_char               *last;  //当前内存块分配到此处，即下一次分配从此处开始  
    u_char               *end;   //内存块结束位置
    ngx_pool_t           *next;  //指向下一个内存池
    ngx_uint_t            failed;//内存块分配失败次数
} ngx_pool_data_t;

/* 内存池结构(内存块管理结构)
 * 优点: 一个过程申请的内存都统一维护,减少了malloc的调用;过程结束后统一释放,规避内存泄露,减少开发者工作量
 */
struct ngx_pool_s {
    ngx_pool_data_t       d;      //内存块的数据块
    size_t                max;    //内存块最大长度;申请大于这个长度的内存时,就malloc一块内存放到large链表中 
    ngx_pool_t           *current;//指向当前正在使用内存快
    ngx_chain_t          *chain;  //该指针挂接一个ngx_chain_t结构
    ngx_pool_large_t     *large;  //大块内存链表，即分配空间超过max的内存
    ngx_pool_cleanup_t   *cleanup;//释放内存池的callback
    ngx_log_t            *log;    //日志信息
};


typedef struct {
    ngx_fd_t              fd;
    u_char               *name;
    ngx_log_t            *log;
} ngx_pool_cleanup_file_t;



ngx_pool_t *ngx_create_pool(size_t size, ngx_log_t *log);
void ngx_destroy_pool(ngx_pool_t *pool);
void ngx_reset_pool(ngx_pool_t *pool);

void *ngx_palloc(ngx_pool_t *pool, size_t size);
void *ngx_pnalloc(ngx_pool_t *pool, size_t size);
void *ngx_pcalloc(ngx_pool_t *pool, size_t size);
void *ngx_pmemalign(ngx_pool_t *pool, size_t size, size_t alignment);
ngx_int_t ngx_pfree(ngx_pool_t *pool, void *p);


ngx_pool_cleanup_t *ngx_pool_cleanup_add(ngx_pool_t *p, size_t size);
void ngx_pool_run_cleanup_file(ngx_pool_t *p, ngx_fd_t fd);
void ngx_pool_cleanup_file(void *data);
void ngx_pool_delete_file(void *data);


#endif /* _NGX_PALLOC_H_INCLUDED_ */
