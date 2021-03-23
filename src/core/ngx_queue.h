
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 * 
 * @双向循环链表容器@
 * 拥有排序功能
 * 轻量级,不负责内存的维护
 * 支持两个容器的合并
 *
 * 使用双向循环链表,必须先定义一个ngx_queue_t变量,我们可以将这个变量代表的节点称为管理节点
 * 下面的一些宏定义中, h表示的是管理节点, q表示的任意一个节点,这个要分清楚
 *
 * 这个双向循环链表初看比较奇怪, 因为ngx_queue_t结构中只有前后的指针,
 * 没有任何保存该节点对应数据的成员, 完全想不到应用的场合.
 * 这个双向循环链表的使用精髓就在于 ngx_queue_data 这个宏.
 * 使用的方法是 在某个结构体中定义一个ngx_queue_t成员, 之后使用ngx_queue_data找到这个结构体,
 * 这个结构体就是一个链表节点的数据存储位置,详细参考 ngx_queue_data 的定义处的说明.
 */


#include <ngx_config.h>
#include <ngx_core.h>


#ifndef _NGX_QUEUE_H_INCLUDED_
#define _NGX_QUEUE_H_INCLUDED_


typedef struct ngx_queue_s  ngx_queue_t;

struct ngx_queue_s {
    ngx_queue_t  *prev;
    ngx_queue_t  *next;
};


/* 双向循环链表的初始化
 * 从初始化的方式可以看出这是个循环的双向链表
 * 管理节点q的初始化
 */
#define ngx_queue_init(q)                                                     \
    (q)->prev = q;                                                            \
    (q)->next = q


/* 双向循环链表是否为空
 * 也可以判断 (h == (h)->next)
 */
#define ngx_queue_empty(h)                                                    \
    (h == (h)->prev)


/* 将元素x插入到容器h的头部
 * 将元素x变成容器h的第一个元素
 */
#define ngx_queue_insert_head(h, x)                                           \
    (x)->next = (h)->next;                                                    \
    (x)->next->prev = x;                                                      \
    (x)->prev = h;                                                            \
    (h)->next = x


#define ngx_queue_insert_after   ngx_queue_insert_head


/* 将元素x插入到容器h的尾部 
 * 将元素x变成容器h的最后一个元素
 */
#define ngx_queue_insert_tail(h, x)                                           \
    (x)->prev = (h)->prev;                                                    \
    (x)->prev->next = x;                                                      \
    (x)->next = h;                                                            \
    (h)->prev = x


/* 获取双向链表容器的第一个元素 */
#define ngx_queue_head(h)                                                     \
    (h)->next


/* 获取双向链表容器的最后一个元素 */
#define ngx_queue_last(h)                                                     \
    (h)->prev


/* 获取双向链表容器的结构体指针 */
#define ngx_queue_sentinel(h)                                                 \
    (h)


/* 获取节点q的下一个节点 */
#define ngx_queue_next(q)                                                     \
    (q)->next


/* 获取节点q的上一个节点 */
#define ngx_queue_prev(q)                                                     \
    (q)->prev

/* 删除节点x */
#if (NGX_DEBUG)

#define ngx_queue_remove(x)                                                   \
    (x)->next->prev = (x)->prev;                                              \
    (x)->prev->next = (x)->next;                                              \
    (x)->prev = NULL;                                                         \
    (x)->next = NULL

#else

#define ngx_queue_remove(x)                                                   \
    (x)->next->prev = (x)->prev;                                              \
    (x)->prev->next = (x)->next

#endif

/* 链表的拆分
 * 将链表h以q为分界拆分为 h 和 n 两个链表
 * h由原来h链表的第一个元素到q(不包括q)组成
 * n由q到原h的最后一个元素组成, q是n的第一个元素
 */
#define ngx_queue_split(h, q, n)                                              \
    (n)->prev = (h)->prev;                                                    \
    (n)->prev->next = n;                                                      \
    (n)->next = q;                                                            \
    (h)->prev = (q)->prev;                                                    \
    (h)->prev->next = h;                                                      \
    (q)->prev = n;

/* 合并双向链表h 和 n (n追加到h的末尾)
 * 将 链表n的第一个元素 链到 链表h的最后一个元素 
 * 将 链表h的第一个元素 链到 链表n的最后一个元素
 * 链表h的最后一个元素变成 链表n的最后一个元素
 */
#define ngx_queue_add(h, n)                                                   \
    (h)->prev->next = (n)->next;                                              \
    (n)->next->prev = (h)->prev;                                              \
    (h)->prev = (n)->prev;                                                    \
    (h)->prev->next = h;

/* 获取链表节点q所在的数据结构(结构体)的地址
 * q 为链表的一个元素,type为一个结构体类型, 该结构中必须有ngx_queue_t 类型成员,
 * link为type类型中 ngx_queue_t成员的名字.
 * offsetof(type, link) 就是 ngx_queue_t成员相对于这个结构的偏移量,
 * q为链表元素的指针,即内存位置,该内存位置减去偏移量即为该结构体的最开始位置,即该结构体的指针.
 */
#define ngx_queue_data(q, type, link)                                         \
    (type *) ((u_char *) q - offsetof(type, link))

/* 返回链表中心元素, 返回第N/2+1个元素 */
ngx_queue_t *ngx_queue_middle(ngx_queue_t *queue);

/* 使用插入排序法对链表进行排序,排序方法由用户定义的回调函数决定 */
void ngx_queue_sort(ngx_queue_t *queue,
    ngx_int_t (*cmp)(const ngx_queue_t *, const ngx_queue_t *));


#endif /* _NGX_QUEUE_H_INCLUDED_ */
