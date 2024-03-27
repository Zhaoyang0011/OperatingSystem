#ifndef _MEM_PAGE_H
#define _MEM_PAGE_H

#include <spinlock.h>
#include <struct/list.h>
#include <type.h>

// 内存空间地址描述符标志
// memory page descriptor flags
typedef struct mpflgs
{
    uint32_t mpf_olkty : 2;  // 挂入链表的类型
    uint32_t mpf_lstty : 1;  // 是否挂入链表
    uint32_t mpf_mocty : 2;  // 分配类型，被谁占用了，内核还是应用或者空闲
    uint32_t mpf_marty : 3;  // 属于哪个区
    uint32_t mpf_uindx : 24; // 分配计数
} __attribute__((packed)) mpflgs_t;

// 物理地址和标志
// physical address flags
typedef struct phyadrflgs
{
    uint64_t paf_alloc : 1;  // 分配位
    uint64_t paf_shared : 1; // 共享位
    uint64_t paf_swap : 1;   // 交换位
    uint64_t paf_cache : 1;  // 缓存位
    uint64_t paf_kmap : 1;   // 映射位
    uint64_t paf_lock : 1;   // 锁定位
    uint64_t paf_dirty : 1;  // 脏位
    uint64_t paf_busy : 1;   // 忙位
    uint64_t paf_rv2 : 4;    // 保留位
    uint64_t paf_padrs : 52; // 页物理地址位
} __attribute__((packed)) phyadrflgs_t;

// 内存空间地址描述符
// memory page descriptor
typedef struct mpdesc
{
    list_t mpd_list;          // 链表
    spinlock_t mpd_lock;      // 保护自身的自旋锁
    mpflgs_t mpd_indxflgs;    // 内存空间地址描述符标志
    phyadrflgs_t mpd_phyadrs; // 物理地址和标志
    void *mpd_odlink;         // 相邻且相同大小msadsc的指针
} __attribute__((packed)) mpdesc_t;

void init_memory_page();

#endif