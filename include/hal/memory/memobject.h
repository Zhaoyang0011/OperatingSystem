#ifndef _MEMROY_OBJECT
#define _MEMROY_OBJECT

#include <spinlock.h>
#include <struct/list.h>
#include <type.h>

#define MSCLST_MAX (5)
#define KOBLST_MAX (64)
#define KUC_NEWFLG (1)
#define KUC_DELFLG (2)
#define KUC_DSYFLG (3)

typedef struct free_memory_object
{
    list_t oh_list;
    uint_t oh_stus;
    void *oh_stat;
} freememobj_t;

// 管理内存对象容器占用的内存页面所对应的mpgdesc_t结构
typedef struct memory_page_list
{
    uint_t mpl_msanr; // 多少个mpgdesc_t
    uint_t mpl_ompnr; // 一个mpgdesc_t对应的连续的物理内存页面数
    list_t mpl_list;  // 挂载mpgdesc_t的链表
} mpglist_t;

// 管理内存对象容器占用的内存
typedef struct s_MSOMDC
{
    // mpglist_t结构数组mc_lst[0]=1个连续页面的mpgdesc_t
    //                mc_lst[1]=2个连续页面的mpgdesc_t
    //                mc_lst[2]=4个连续页面的mpgdesc_t
    //                mc_lst[3]=8个连续页面的mpgdesc_t
    //                mc_lst[4]=16个连续页面的mpgdesc_t
    mpglist_t mc_lst[MSCLST_MAX];
    uint_t mc_mpgnr; // 总共多个mpgdesc_t结构
    list_t mc_list;
    // 内存对象容器第一个占用mpgdesc_t
    list_t mc_kmobinlst;
    // 内存对象容器第一个占用mpgdesc_t对应的连续的物理内存页面数
    uint_t mc_kmobinpnr;
} msomdc_t;

// 内存对象容器
typedef struct memory_object_manager
{
    list_t so_list;     // 链表
    spinlock_t so_lock; // 保护结构自身的自旋锁
    uint_t so_stus;     // 状态与标志
    uint_t so_flgs;
    addr_t so_vstat;    // 内存对象容器的开始地址
    addr_t so_vend;     // 内存对象容器的结束地址
    size_t so_objsz;    // 内存对象大小
    size_t so_objrelsz; // 内存对象实际大小
    uint_t so_mobjnr;   // 内存对象容器中总共的对象个数
    uint_t so_fobjnr;   // 内存对象容器中空闲的对象个数
    list_t so_frelst;   // 内存对象容器中空闲的对象链表头
    list_t so_alclst;   // 内存对象容器中分配的对象链表头
    list_t so_mextlst;  // 内存对象容器扩展kmbext_t结构链表头
    uint_t so_mextnr;   // 内存对象容器扩展kmbext_t结构个数
    msomdc_t so_mc;     // 内存对象容器占用内存页面管理结构
    void *so_privp;     // 本结构私有数据指针
    void *so_extdp;     // 本结构扩展数据指针
} momgr_t;

// 管理内存对象容器扩展容量
typedef struct memory_object_manager_extention
{
    list_t mt_list;   // 链表
    addr_t mt_vstat;  // 内存对象容器扩展容量开始地址
    addr_t mt_vend;   // 内存对象容器扩展容量结束地址
    momgr_t *mt_kmsb; // 指向内存对象容器结构
    uint_t mt_mobjnr; // 内存对象容器扩展容量的内存中有多少对象
} momgrext_t;

#define KOBLST_MAX (64)

// 挂载momgr_t结构
typedef struct memory_object_manager_list
{
    list_t ol_emplst;  // 挂载momgr_t结构的链表
    momgr_t *ol_cache; // 最近一次查找的momgr_t结构
    uint_t ol_emnr;    // 挂载momgr_t结构的数量
    size_t ol_sz;      // momgr_t结构中内存对象的大小
} momgrlist_t;

// 管理momgr_t结构的数据结构
typedef struct memory_object_manager_header
{
    spinlock_t ks_lock; // 保护自身的自旋锁
    list_t ks_tclst;    // 链表
    uint_t ks_tcnr;
    uint_t ks_msobnr;                   // 总共多少个momgr_t结构
    momgr_t *ks_msobche;                // 最近分配内存对象的momgr_t结构
    momgrlist_t ks_msoblst[KOBLST_MAX]; // mo_mgr_list_t结构数组
} momgrheader_t;

void init_memory_object_manager();

void momgrlist_init(momgrlist_t *initp, size_t mobsz);

#endif