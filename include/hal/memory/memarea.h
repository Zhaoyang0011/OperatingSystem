#ifndef _MEMAREA_H
#define _MEMAREA_H

#include <hal/halglobal.h>
#include <hal/memory/mempage.h>
#include <spinlock.h>
#include <struct/list.h>
#include <type.h>

#define MPAF_STUS_INIT 0
#define MPAF_STUS_ONEM 1
#define MPAF_STUS_DIVP 2
#define MPAF_STUS_DIVM 3

// memory page alloc free list
typedef struct mpaflist
{
    spinlock_t af_lock; // 保护自身结构的自旋锁
    uint32_t af_stus;   // 状态
    uint_t af_oder;     // 页面数的位移量
    uint_t af_oderpnr;  // oder对应的页面数比如 oder为2那就是1<<2=4
    uint_t af_fobjnr;   // 多少个空闲mpdesc_t结构，即空闲页面
    uint_t af_mobjnr;   // 此结构的mpdesc_t结构总数，即此结构总页面
    uint_t af_alcindx;  // 此结构的分配计数
    uint_t af_freindx;  // 此结构的释放计数
    list_t af_frelist;  // 挂载此结构的空闲mpdesc_t结构
    list_t af_alclist;  // 挂载此结构已经分配的mpdesc_t结构
} mpaflist_t;

void mpaflist_t_init(mpaflist_t *mapflist, uint32_t stus, uint_t oder, uint_t oderpnr);

// memory divide merge
#define MDIVMER_ARR_LMAX 52
#define MDIVMER_ARR_BMAX 11
#define MDIVMER_ARR_OMAX 9
typedef struct memdivmer
{
    spinlock_t dm_lock;
    uint32_t dm_stus;
    uint_t dm_dmmaxindx;
    uint_t dm_phydmindx;
    uint_t dm_predmindx;
    uint_t dm_divnr;
    uint_t dm_mernr;
    mpaflist_t dm_mdmlst[MDIVMER_ARR_LMAX];
    mpaflist_t dm_onelst;
} memdivmer_t;

void memdivmer_t_init(memdivmer_t *memdivmer);

#define MEMAREA_MAX 4
#define MA_TYPE_INIT 0
#define MA_TYPE_HWAD 1
#define MA_TYPE_KRNL 2
#define MA_TYPE_PROC 3
#define MA_TYPE_SHAR 4
#define MA_HWAD_LSTART 0
#define MA_HWAD_LSZ 0x2000000
#define MA_HWAD_LEND (MA_HWAD_LSTART + MA_HWAD_LSZ - 1)
#define MA_KRNL_LSTART 0x2000000
#define MA_KRNL_LSZ (0x400000000 - 0x2000000)
#define MA_KRNL_LEND (MA_KRNL_LSTART + MA_KRNL_LSZ - 1)
#define MA_PROC_LSTART 0x400000000
#define MA_PROC_LSZ (0xffffffffffffffff - 0x400000000)
#define MA_PROC_LEND (MA_PROC_LSTART + MA_PROC_LSZ)
// 0x400000000  0x40000000
typedef struct s_MEMAREA
{
    list_t ma_list;     // 内存区自身的链表
    spinlock_t ma_lock; // 保护内存区的自旋锁
    uint_t ma_stus;     // 内存区的状态
    uint_t ma_flgs;     // 内存区的标志
    uint_t ma_type;     // 内存区的类型
    // sem_t ma_sem;//内存区的信号量
    // wait_l_head_t ma_waitlst;//内存区的等待队列
    uint_t ma_maxpages;   // 内存区总的页面数
    uint_t ma_allocpages; // 内存区分配的页面数
    uint_t ma_freepages;  // 内存区空闲的页面数
    uint_t ma_resvpages;  // 内存区保留的页面数
    uint_t ma_horizline;  // 内存区分配时的水位线
    addr_t ma_logicstart; // 内存区开始地址
    addr_t ma_logicend;   // 内存区结束地址
    uint_t ma_logicsz;    // 内存区大小
    addr_t ma_effectstart;
    addr_t ma_effectend;
    uint_t ma_effectsz;
    list_t ma_allmsadsclst;
    uint_t ma_allmsadscnr;
    // arclst_t ma_arcpglst;
    // mafuncobjs_t ma_funcobj;
    memdivmer_t ma_mdmdata;
    void *ma_privp;
    /*
     *这个结构至少占用一个页面，当然
     *也可以是多个连续的的页面，但是
     *该结构从第一个页面的首地址开始
     *存放，后面的空间用于存放实现分
     *配算法的数据结构，这样每个区可
     *方便的实现不同的分配策略，或者
     *有天你觉得我的分配算法是渣渣，
     *完全可以替换mafuncobjs_t结构
     *中的指针，指向你的函数。
     */
} memarea_t;

void init_memory_area();
void memarea_t_init(memarea_t *memarea);
void load_mempage_memarea();

HAL_DEFGLOB_VARIABLE(memarea_t, memarea_arr)[MEMAREA_MAX];

#endif