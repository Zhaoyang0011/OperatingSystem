#ifndef _LDR_TYPE_H
#define _LDR_TYPE_H

#include <type.h>

#define E820_DESC 0x5000
#define E820_ADR (E820_DESC + 4)

#define E820_MAX 200

#define RAM_USABLE 1
#define RAM_RESERV 2
#define RAM_ACPIREC 3
#define RAM_ACPINVS 4
#define RAM_AREACON 5
#define RAM_MEGER 6

typedef struct e820_map {
  uint64_t addr; /* start of memory segment8 */
  uint64_t size; /* size of memory segment8 */
  uint32_t type; /* type of memory segment 4*/
} __attribute__((packed)) e820_map_t;

typedef struct e820_desc {
  uint32_t e820_num;
  e820_map_t maps[E820_MAX];
} __attribute__((packed)) e820_desc_t;

#define KERNEL_START 0x2000000
#define KERNEL_DESC_OFF -0x1000
// stack macro
#define STACK_PHYADR (0x90000 - 0x10)
#define STACK_SIZE 0x1000
// kernel init page physical address
#define KINITPAGE_PHYADR 0x1000000
#define E820_DESC 0x5000
#define E820_MAX 200

// memory page macro
#define KRNL_VIRTUAL_ADDRESS_START 0xffff800000000000
#define KPML4_P (1 << 0)
#define KPML4_RW (1 << 1)
#define KPML4_US (1 << 2)
#define KPML4_PWT (1 << 3)
#define KPML4_PCD (1 << 4)
#define KPML4_A (1 << 5)

#define KPDPTE_P (1 << 0)
#define KPDPTE_RW (1 << 1)
#define KPDPTE_US (1 << 2)
#define KPDPTE_PWT (1 << 3)
#define KPDPTE_PCD (1 << 4)
#define KPDPTE_A (1 << 5)

#define KPDE_P (1 << 0)
#define KPDE_RW (1 << 1)
#define KPDE_US (1 << 2)
#define KPDE_PWT (1 << 3)
#define KPDE_PCD (1 << 4)
#define KPDE_A (1 << 5)
#define KPDE_D (1 << 6)
#define KPDE_PS (1 << 7)
#define KPDE_G (1 << 8)
#define KPDE_PAT (1 << 12)

#define KPML4_SHIFT 39
#define KPDPTTE_SHIFT 30
#define KPDP_SHIFT 21
#define PGENTY_SIZE 512

#define ZHOS_MAGIC                                               \
  (uint64_t)((((uint64_t)'Z') << 56) | (((uint64_t)'H') << 48) | \
             (((uint64_t)'O') << 40) | (((uint64_t)'S') << 32) | \
             (((uint64_t)'M') << 24) | (((uint64_t)'A') << 16) | \
             (((uint64_t)'C') << 8) | ((uint64_t)'H'))

typedef struct kernel_desc {
  // kernel info
  uint64_t kernel_magic;  // magic number of os
  uint64_t kernel_start;  // starting address of kernel
  uint64_t kernel_size;   // kernel size
  // stack info
  uint64_t stack_init_adr;  // kernel stack address
  uint64_t stack_size;      // kernel stack size
  // memory info
  uint64_t mach_memsize;  // memory size of machine
  uint64_t mmap_adr;      // address of e820 array
  uint64_t mmap_nr;       // number of e820 entry
  uint64_t mmap_sz;       // size of e820 array
  uint64_t mp_desc_arr;   // start address of memory page descriptor array
  uint64_t mp_desc_nr;    // number of memory page descriptors
  uint64_t mp_desc_sz;    // size of memory page descriptor array
  uint64_t ma_desc_arr;   // physical address of memory area array
  uint64_t ma_nr;         //
  uint64_t ma_sz;
  uint64_t next_pg;  // next paging address
} __attribute__((packed)) kernel_desc_t;

#endif