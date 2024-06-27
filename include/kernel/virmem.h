#ifndef _VIRMEM_H
#define _VIRMEM_H

#include <spinlock.h>
#include <struct/list.h>
#include <semaphore.h>
#include <atomic.h>

#define KRNL_MAP_VIRTADDRESS_SIZE 0x400000000
#define KRNL_VIRTUAL_ADDRESS_START 0xffff800000000000
#define KRNL_VIRTUAL_ADDRESS_END 0xffffffffffffffff

#define USER_VIRTUAL_ADDRESS_START 0
#define USER_VIRTUAL_ADDRESS_END 0x00007fffffffffff

#define KRNL_MAP_PHYADDRESS_START 0
#define KRNL_MAP_PHYADDRESS_END 0x400000000
#define KRNL_MAP_PHYADDRESS_SIZE 0x400000000
#define KRNL_MAP_VIRTADDRESS_START KRNL_VIRTUAL_ADDRESS_START
#define KRNL_MAP_VIRTADDRESS_END (KRNL_MAP_VIRTADDRESS_START+KRNL_MAP_VIRTADDRESS_SIZE)
#define KRNL_ADDR_ERROR 0xf800000000000

#define VADSZ_ALIGN(x) ALIGN(x,0x1000)

typedef struct kernel_virtual_memory_container_box_manager {
    list_t kbm_list;          //链表
    spinlock_t kbm_lock;      //保护自身的自旋锁
    uint64_t kbm_flgs;        //标志与状态
    uint64_t kbm_stus;
    uint_t kbm_kmbnr;         //kvmemcbox_t结构个数
    list_t kbm_kmbhead;       //挂载kvmemcbox_t结构的链表
    uint_t kbm_cachenr;       //缓存空闲kvmemcbox_t结构的个数
    uint_t kbm_cachemax;      //最大缓存个数，超过了就要释放
    uint_t kbm_cachemin;      //最小缓存个数
    list_t kbm_cachehead;     //缓存kvmemcbox_t结构的链表
    void *kbm_ext;            //扩展数据指针
} kvmemcboxmgr_t;

typedef struct kernel_virtual_memory_container_box {
    list_t kmb_list;          //链表
    spinlock_t kmb_lock;      //保护自身的自旋锁
    refcount_t kmb_cont;      //共享的计数器
    uint64_t kmb_flgs;        //状态和标志
    uint64_t kmb_stus;
    uint64_t kmb_type;        //类型
    uint_t kmb_msanr;         //多少个msadsc_t
    list_t kmb_msalist;       //挂载msadsc_t结构的链表
    kvmemcboxmgr_t *kmb_mgr;  //指向上层结构
    void *kmb_filenode;       //指向文件节点描述符
    void *kmb_pager;          //指向分页器 暂时不使用
    void *kmb_ext;            //自身扩展数据指针
} kvmemcbox_t;

typedef struct kernel_virtual_memory_area {
    spinlock_t kva_lock;           //保护自身自旋锁
    uint32_t kva_maptype;          //映射类型
    list_t kva_list;               //链表
    uint64_t kva_flgs;             //相关标志
    uint64_t kva_limits;
    void *kva_mcstruct;            //指向它的上层结构
    addr_t kva_start;              //虚拟地址的开始
    addr_t kva_end;                //虚拟地址的结束
    kvmemcbox_t *kva_kvmbox;       //管理这个结构映射的物理页面
    void *kva_kvmcobj;
} kvmareadesc_t;

typedef struct process_virtual_memory_space_descriptor pvmspacdesc_t;

typedef struct virtual_memory_space {
    spinlock_t vs_lock;              //保护自身的自旋锁
    uint32_t vs_resalin;
    list_t vs_list;                  //链表，链接虚拟地址区间
    uint_t vs_flgs;                  //标志
    uint_t vs_kmvdscnr;              //多少个虚拟地址区间
    pvmspacdesc_t *vs_mm;            //指向它的上层的数据结构
    kvmareadesc_t *vs_startkmvdsc;   //开始的虚拟地址区间
    kvmareadesc_t *vs_endkmvdsc;     //结束的虚拟地址区间
    kvmareadesc_t *vs_currkmvdsc;    //当前的虚拟地址区间
    addr_t vs_isalcstart;            //能分配的开始虚拟地址
    addr_t vs_isalcend;              //能分配的结束虚拟地址
    void *vs_privte;                 //私有数据指针
    void *vs_ext;                    //扩展数据指针
} virmemspace_t;

typedef struct process_virtual_memory_space_descriptor {
    spinlock_t msd_lock;                //保护自身的自旋锁
    list_t msd_list;                    //链表
    uint_t msd_flag;                    //状态和标志
    uint_t msd_stus;
    uint_t msd_scount;                  //计数，该结构可能被共享
    sem_t msd_sem;                     //信号量
//    mmudsc_t msd_mmu;                 //MMU相关的信息
    virmemspace_t msd_virmemadrs;       //虚拟地址空间
    addr_t msd_stext;                   //应用的指令区的开始、结束地址
    addr_t msd_etext;
    addr_t msd_sdata;                   //应用的数据区的开始、结束地址
    addr_t msd_edata;
    addr_t msd_sbss;
    addr_t msd_ebss;
    addr_t msd_sbrk;                    //应用的堆区的开始、结束地址
    addr_t msd_ebrk;
} pvmspacdesc_t;

kvmareadesc_t *new_kvmareadesc();
bool_t del_kvmareadesc(kvmareadesc_t *desc);

bool_t virmemspace_init(virmemspace_t *vma);

#endif