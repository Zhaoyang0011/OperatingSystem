#ifndef _VIR_MEM_H
#define _VIR_MEM_H

#include <spinlock.h>
#include <struct/list.h>

// typedef struct vm_area
// {
//     spinlock_t vma_lock;  // 保护自身自旋锁
//     uint32_t vma_maptype; // 映射类型
//     list_t vma_list;      // 链表
//     uint64_t vma_flgs;    // 相关标志
//     uint64_t vma_limits;
//     void *vma_mcstruct;      // 指向它的上层结构
//     addr_t vma_start;        // 虚拟地址的开始
//     addr_t vma_end;          // 虚拟地址的结束
//     kvmemcbox_t *vma_kvmbox; // 管理这个结构映射的物理页面
//     void *vma_kvmcobj;

// } vm_area_t;

// typedef struct vm_space
// {
//     spinlock_t vms_lock; // 保护自身的自旋锁
//     uint32_t vms_resalin;
//     list_t vms_list;            // 链表，链接虚拟地址区间
//     uint_t vms_flgs;              // 标志
//     uint_t vms_kmvdscnr;          // 多少个虚拟地址区间
//     mmadrsdsc_t *vms_mm;          // 指向它的上层的数据结构
//     vm_area_t *vms_startkmvdsc; // 开始的虚拟地址区间
//     vm_area_t *vms_endkmvdsc;   // 结束的虚拟地址区间
//     vm_area_t *vms_currkmvdsc;  // 当前的虚拟地址区间
//     addr_t vms_isalcstart;         // 能分配的开始虚拟地址
//     addr_t vms_isalcend;           // 能分配的结束虚拟地址
//     void *vms_privte;             // 私有数据指针
//     void *vms_ext;                // 扩展数据指针
// } vm_space_t;

#endif