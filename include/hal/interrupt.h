#ifndef _INTERRUPT_H
#define _INTERRUPT_H

#include <spinlock.h>
#include <type.h>
#include <struct/list.h>

// interrupt fault descriptor 中断异常描述符
typedef struct int_fault_desc {
    spinlock_t i_lock;
    uint32_t i_flag;
    uint32_t i_status;
    uint_t i_priority; // 中断优先级
    uint_t i_number;   // 中断号
    uint_t i_depth;    // 中断嵌套深度
    uint64_t i_index;  // 中断计数
    list_t i_serlist;
    uint_t i_sernr;
    list_t i_threadlst;  // 中断线程链表头
    uint_t i_threadnr;   // 中断线程个数
    void *i_onethread;   // 只有一个中断线程时直接用指针
    void *i_rbtreeroot;  // 如果中断线程太多则按优先级组成红黑树
    list_t i_serfisrlst; // 也可以使用中断回调函数的方式
    uint_t i_serfisrnr;  // 中断回调函数个数
    void *i_msgmpool;    // 可能的中断消息池
    void *i_privp;
    void *i_extp;
} int_flt_desc_t;

void init_interrupt();

#endif