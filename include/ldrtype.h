#ifndef _LDR_TYPE_H
#define _LDR_TYPE_H

#include <type.h>

// kernel init page physical address
#define KINITPAGE_PHYADR 0x1000000

typedef struct kernel_desc{
    // kernel info
    uint64_t kernel_magic;  //magic number of os
    uint64_t kernel_start;  //starting address of kernel
    uint64_t kernel_size;   //kernel size
    // stack info
    uint64_t stack_init_adr;  //kernel stack address
    uint64_t stack_size;      //kernel stack size
    // memory info
    uint64_t mach_memsize;  //memory size of machine
    uint64_t mmap_adr;      //address of e820 array
    uint64_t mmap_nr;       //number of e820 entry
    uint64_t mmap_sz;       //size of e820 array
    uint64_t mp_desc_arr;   //start address of memory page descriptor array
    uint64_t mp_desc_nr;    //number of memory page descriptors
    uint64_t mp_desc_sz;    //size of memory page descriptor array
    uint64_t ma_desc_arr;   //physical address of memory area array
    uint64_t ma_nr;         //
    uint64_t ma_sz;
    uint64_t next_pg;       //next paging address
}__attribute__((packed)) kernel_desc_t;

#endif