#ifndef _MEM_PAGE_H
#define _MEM_PAGE_H

#include <hal/halglobal.h>
#include <spinlock.h>
#include <struct/list.h>
#include <type.h>

// 4k
#define PAGE_SIZE (1UL << 12)
#define PAGE_SHR 12

#define MF_OLKTY_INIT (0)
#define MF_OLKTY_ODER (1)
#define MF_OLKTY_BAFH (2)
#define MF_OLKTY_TOBJ (3)

#define MF_LSTTY_LIST (0)
#define MF_MOCTY_FREE (0)
#define MF_MOCTY_KRNL (1)
#define MF_MOCTY_USER (2)
#define MF_MRV1_VAL (0)
#define MF_UINDX_INIT (0)
#define MF_UINDX_MAX (0xffffff)
#define MF_MARTY_INIT (0)
#define MF_MARTY_HWD (1)
#define MF_MARTY_KRL (2)
#define MF_MARTY_PRC (3)
#define MF_MARTY_SHD (4)
// 内存空间地址描述符标志
// memory page descriptor flags
typedef struct mpflgs {
  uint32_t mpf_olkty: 2;  // 挂入链表的类型
  uint32_t mpf_lstty: 1;  // 是否挂入链表
  uint32_t mpf_mocty: 2;  // 分配类型，被谁占用了，内核还是应用或者空闲
  uint32_t mpf_marty: 3;  // 属于哪个区
  uint32_t mpf_uindx: 24; // 分配计数
} __attribute__((packed)) mpflgs_t;

// 物理地址和标志
// physical address flags
#define PAF_NO_ALLOC (0)
#define PAF_ALLOC (1)
#define PAF_NO_SHARED (0)
#define PAF_NO_SWAP (0)
#define PAF_NO_CACHE (0)
#define PAF_NO_KMAP (0)
#define PAF_NO_LOCK (0)
#define PAF_NO_DIRTY (0)
#define PAF_NO_BUSY (0)
#define PAF_RSV_VAL (0)
#define PAF_INIT_PADRS (0)
#define PAF_ADDR_MASK (0xFFFFFFFFFFFFF000)

typedef struct phyadrflgs {
  uint64_t paf_alloc: 1;  // 分配位
  uint64_t paf_shared: 1; // 共享位
  uint64_t paf_swap: 1;   // 交换位
  uint64_t paf_cache: 1;  // 缓存位
  uint64_t paf_kmap: 1;   // 映射位
  uint64_t paf_lock: 1;   // 锁定位
  uint64_t paf_dirty: 1;  // 脏位
  uint64_t paf_busy: 1;   // 忙位
  uint64_t paf_rsv: 4;    // 保留位
  uint64_t paf_paddr: 52; // 页物理地址位
} __attribute__((packed)) phyadrflgs_t;

// 内存空间地址描述符
// memory page descriptor
typedef struct memory_page_descriptor {
  list_t mpd_list;            // 链表
  spinlock_t mpd_lock;        // 保护自身的自旋锁
  mpflgs_t mpd_indxflgs;      // 内存空间地址描述符标志
  union {
    uint64_t mpd_addr;
    phyadrflgs_t mpd_adrflgs; // 物理地址和标志
  } __attribute__((packed));
  void *mpd_odlink;           // 相邻且相同大小msadsc的指针
} mpdesc_t;

void init_memory_page();

void init_mempage_occupation();

addr_t mpdesc_ret_vaddr(mpdesc_t *retmsa);

#endif