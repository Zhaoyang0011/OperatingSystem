#ifndef _LDR_H
#define _LDR_H

/**
 * File name: ldr.h
 * Author: Zhaoyang Shi
 * Description: Header file of loader.
 */

#include <ldrtype.h>

/**
 * check if kernel is successfully loaded, and init kernel start address
 */
void chkini_kernel_info(kernel_desc_t *kernel_desc);

/**
 * init stack address and stack size
 */
void init_stack(kernel_desc_t *kernel_desc);

/**
 * init memory infomation from e820 structure
 */
void init_memory_info(kernel_desc_t *kernel_desc);

/**
 * This funcion prepares the memory page information that the long mode needed.
 * To simplify programming difficulty, we use 2M as the page size.
 * After entering long mode we will modify the paging settings and use 4K as the
 * page size.
 */
void init_pages(kernel_desc_t *kernel_desc);

#endif