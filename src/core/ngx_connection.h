
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_CONNECTION_H_INCLUDED_
#define _NGX_CONNECTION_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


typedef struct ngx_listening_s  ngx_listening_t;

struct ngx_listening_s {
    ngx_socket_t        fd; //socket套接字句柄

    struct sockaddr    *sockaddr; //监听地址
    socklen_t           socklen;    /* size of sockaddr */
    size_t              addr_text_max_len; //?
    ngx_str_t           addr_text; //IP地址的字符串形式

    int                 type; //套接字类型 eg SOCK_STREAM

    int                 backlog; //TCP监听时的backlog队列, 表示允许正在通过过上次握手建立TCP连接但是还没有任何进程处理的连接的最大个数
    int                 rcvbuf;  //内核中对这个套接字的接收缓冲区大小
    int                 sndbuf;  //内核中对这个套接字的发送缓冲区大小
#if (NGX_HAVE_KEEPALIVE_TUNABLE)
    int                 keepidle;
    int                 keepintvl;
    int                 keepcnt;
#endif

    /* handler of accepted connection */
    ngx_connection_handler_pt   handler; //当新的TCP连接建立成功后的处理方法. eg ngx_http_init_connection
	/* 框架本身并不使用,主要是http或者mail模块 用来保存当前监听端口对应着的所有主机名   */
    void               *servers;  /* array of ngx_http_in_addr_t, for example */

    ngx_log_t           log;
    ngx_log_t          *logp;

    size_t              pool_size; //新的TCP连接创建的内存池初始化大小
    /* should be here because of the AcceptEx() preread */
    size_t              post_accept_buffer_size;
    /* should be here because of the deferred accept */
    ngx_msec_t          post_accept_timeout;

    ngx_listening_t    *previous;
    ngx_connection_t   *connection;

    ngx_rbtree_t        rbtree;
    ngx_rbtree_node_t   sentinel;

    ngx_uint_t          worker;

    unsigned            open:1;
    unsigned            remain:1;
    unsigned            ignore:1;

    unsigned            bound:1;       /* already bound */
    unsigned            inherited:1;   /* inherited from previous process */
    unsigned            nonblocking_accept:1;
    unsigned            listen:1;
    unsigned            nonblocking:1;
    unsigned            shared:1;    /* shared between threads or processes */
    unsigned            addr_ntop:1;
    unsigned            wildcard:1;

#if (NGX_HAVE_INET6)
    unsigned            ipv6only:1;
#endif
    unsigned            reuseport:1;
    unsigned            add_reuseport:1;
    unsigned            keepalive:2;

    unsigned            deferred_accept:1;
    unsigned            delete_deferred:1;
    unsigned            add_deferred:1;
#if (NGX_HAVE_DEFERRED_ACCEPT && defined SO_ACCEPTFILTER)
    char               *accept_filter;
#endif
#if (NGX_HAVE_SETFIB)
    int                 setfib;
#endif

#if (NGX_HAVE_TCP_FASTOPEN)
    int                 fastopen;
#endif

};


typedef enum {
    NGX_ERROR_ALERT = 0,
    NGX_ERROR_ERR,
    NGX_ERROR_INFO,
    NGX_ERROR_IGNORE_ECONNRESET,
    NGX_ERROR_IGNORE_EINVAL
} ngx_connection_log_error_e;


typedef enum {
    NGX_TCP_NODELAY_UNSET = 0,
    NGX_TCP_NODELAY_SET,
    NGX_TCP_NODELAY_DISABLED
} ngx_connection_tcp_nodelay_e;


typedef enum {
    NGX_TCP_NOPUSH_UNSET = 0,
    NGX_TCP_NOPUSH_SET,
    NGX_TCP_NOPUSH_DISABLED
} ngx_connection_tcp_nopush_e;


#define NGX_LOWLEVEL_BUFFERED  0x0f
#define NGX_SSL_BUFFERED       0x01
#define NGX_HTTP_V2_BUFFERED   0x02

/* 服务器的被动连接(TCP连接) */
struct ngx_connection_s {
    void               *data;  /* 自定义数据,例如针对http该指针指向 ngx_http_connection_t   或者 ngx_http_request_t */
    ngx_event_t        *read;  /* 该连接对应的读事件 */
    ngx_event_t        *write; /* 该连接对应的写事件 */

    ngx_socket_t        fd;    /* 套接字句柄 */

    ngx_recv_pt         recv;  /* 接收TCP数据的方法 */
    ngx_send_pt         send;  /* 发送TCP数据的方法    */
    ngx_recv_chain_pt   recv_chain; /* 以ngx_chain_t链表为参数,接收TCP数据的方法 */
    ngx_send_chain_pt   send_chain; /* 以ngx_chain_t链表为参数,发送TCP数据的方法 */

    ngx_listening_t    *listening;

    off_t               sent;  /* 这个连接已经发送出去的字节数 */

    ngx_log_t          *log;

     /* 在accept一个新连接的时候,会创建一个内存池,而这个连接结束时候,会销毁这个内存池.  
      * 这里所说的连接是成功建立的tcp连接.内存池的大小由pool_size决定  
      * 所有的ngx_connect_t结构体都是预分配的  
      */
     ngx_pool_t         *pool;

    int                 type;

    struct sockaddr    *sockaddr; /* 客户端的socket地址 */
    socklen_t           socklen;  /* 客户端的socket地址长度 */
    ngx_str_t           addr_text;/* 客户端的ip(字符串形式) */

    ngx_proxy_protocol_t  *proxy_protocol;

#if (NGX_SSL || NGX_COMPAT)
    ngx_ssl_connection_t  *ssl;
#endif
    /* 本机中监听端口对应的socketaddr结构体, 也就是listen监听对象中的socketaddr成员 */
    ngx_udp_connection_t  *udp;
    struct sockaddr    *local_sockaddr;
    socklen_t           local_socklen;

    ngx_buf_t          *buffer; /* 用于接收和缓存客户端发来的数据流 */

    /* 该字段表示将该连接以双向链表形式添加到cycle结构体中
     * 的reusable_connections_queen双向链表中,表示可以重用的连接.
     */
    ngx_queue_t         queue;
	/* 连接使用次数,每次建立一条来自客户端的连接, 或者建立一条与后端服务器的连接,number+1 */
    ngx_atomic_uint_t   number;

    ngx_uint_t          requests; /* 处理请求的次数 */

    unsigned            buffered:8;
    /* 日志级别   */
    unsigned            log_error:3;     /* ngx_connection_log_error_e */

    unsigned            timedout:1;  /* 连接是否超时的标志位 */
    unsigned            error:1;     /* 连接处理过程中出现错误 */
    unsigned            destroyed:1; /* 连接是否已销毁的标志位 */

    unsigned            idle:1;     /* 连接是否处于空闲状态的标志位 */
    unsigned            reusable:1; /* 连接是否可以重用的标志位 */
    unsigned            close:1;    /* 连接是否关闭的标志位 */
    unsigned            shared:1;

    unsigned            sendfile:1; /* 是否正在发送文件的标志位 */
    unsigned            sndlowat:1;
	/* 是否使用tcp的nodely特性 */
    unsigned            tcp_nodelay:2;   /* ngx_connection_tcp_nodelay_e */
	/* 是否使用tcp的nopush特性 */
    unsigned            tcp_nopush:2;    /* ngx_connection_tcp_nopush_e */

    unsigned            need_last_buf:1;

#if (NGX_HAVE_AIO_SENDFILE || NGX_COMPAT)
    unsigned            busy_count:2;
#endif

#if (NGX_THREADS || NGX_COMPAT)
    ngx_thread_task_t  *sendfile_task;
#endif
};


#define ngx_set_connection_log(c, l)                                         \
                                                                             \
    c->log->file = l->file;                                                  \
    c->log->next = l->next;                                                  \
    c->log->writer = l->writer;                                              \
    c->log->wdata = l->wdata;                                                \
    if (!(c->log->log_level & NGX_LOG_DEBUG_CONNECTION)) {                   \
        c->log->log_level = l->log_level;                                    \
    }


ngx_listening_t *ngx_create_listening(ngx_conf_t *cf, struct sockaddr *sockaddr,
    socklen_t socklen);
ngx_int_t ngx_clone_listening(ngx_cycle_t *cycle, ngx_listening_t *ls);
ngx_int_t ngx_set_inherited_sockets(ngx_cycle_t *cycle);
ngx_int_t ngx_open_listening_sockets(ngx_cycle_t *cycle);
void ngx_configure_listening_sockets(ngx_cycle_t *cycle);
void ngx_close_listening_sockets(ngx_cycle_t *cycle);
void ngx_close_connection(ngx_connection_t *c);
void ngx_close_idle_connections(ngx_cycle_t *cycle);
ngx_int_t ngx_connection_local_sockaddr(ngx_connection_t *c, ngx_str_t *s,
    ngx_uint_t port);
ngx_int_t ngx_tcp_nodelay(ngx_connection_t *c);
ngx_int_t ngx_connection_error(ngx_connection_t *c, ngx_err_t err, char *text);

ngx_connection_t *ngx_get_connection(ngx_socket_t s, ngx_log_t *log);
void ngx_free_connection(ngx_connection_t *c);

void ngx_reusable_connection(ngx_connection_t *c, ngx_uint_t reusable);

#endif /* _NGX_CONNECTION_H_INCLUDED_ */
