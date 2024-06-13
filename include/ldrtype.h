#ifndef _LDR_TYPE_H
#define _LDR_TYPE_H

#include <type.h>

/**
 * Physical memory macro and strucure
 */
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
    uint64_t addr; // start of memory segment8
    uint64_t size; // size of memory segment8
    uint32_t type; // type of memory segment 4
} __attribute__((packed)) e820_map_t;

typedef struct e820_desc {
    uint32_t e820_num;
    e820_map_t maps[E820_MAX];
} __attribute__((packed)) e820_desc_t;

#define KERNEL_START 0x2000000
#define KERNEL_DESC_OFF -0x1000
// stack macro
#define STACK_PHYADR (0x200000 - 0x1000)
#define STACK_SIZE 0x1000
// kernel init page physical address
#define KINITPAGE_PHYADR 0x1000000
#define E820_DESC 0x5000
#define E820_MAX 200

/**
 * cpu information macro and structure
 */
typedef struct cpuid_desc {
    uint32_t function_id;
    uint32_t info[4];
} __attribute__((packed)) cpuid_desc_t;

typedef struct cpu_desc {

} __attribute__((packed)) cpu_info_t;

#define ZHOS_MAGIC                                                                                                     \
    (uint64_t)((((uint64_t)'Z') << 56) | (((uint64_t)'H') << 48) | (((uint64_t)'O') << 40) | (((uint64_t)'S') << 32) | \
               (((uint64_t)'M') << 24) | (((uint64_t)'A') << 16) | (((uint64_t)'C') << 8) | ((uint64_t)'H'))

typedef struct kernel_desc {
    // kernel info
    uint64_t kernel_magic; // magic number of os
    uint64_t kernel_start; // starting address of kernel
    uint64_t kernel_size;  // kernel size
    // stack info
    uint64_t stack_init_adr; // kernel stack address
    uint64_t stack_size;     // kernel stack size
    // memory info
    uint64_t mach_memsize; // memory size of machine
    uint64_t mmap_adr;     // address of e820 array
    uint64_t mmap_nr;      // number of e820 entry
    uint64_t mmap_sz;      // size of e820 array
    uint64_t mp_desc_arr;  // start address of memory page descriptor array
    uint64_t mp_desc_nr;   // number of memory page descriptors
    uint64_t mp_desc_sz;   // size of memory page descriptor array
    uint64_t ma_desc_arr;  // physical address of memory area array
    uint64_t ma_nr;        //
    uint64_t ma_sz;
    uint64_t next_pg; // next paging address
} __attribute__((packed)) kernel_desc_t;

#endif