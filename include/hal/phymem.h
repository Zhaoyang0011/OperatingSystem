/**********************************************************
        physical memory header file
***********************************************************
            Zhaoyang Shi
**********************************************************/
#ifndef _HAL_MEM_H
#define _HAL_MEM_H

#include <spinlock.h>

#define RAM_USABLE 1
#define RAM_RESERV 2
#define RAM_ACPIREC 3
#define RAM_ACPINVS 4
#define RAM_AREACON 5

#define PM_T_OSAPUSERRAM 1
#define PM_T_RESERVRAM 2
#define PM_T_HWUSERRAM 8
#define PM_T_ARACONRAM 0xf
#define PM_T_BUGRAM 0xff
#define PM_F_X86_32 (1<<0)
#define PM_F_X86_64 (1<<1)
#define PM_F_ARM_32 (1<<2)
#define PM_F_ARM_64 (1<<3)
#define PM_F_HAL_MASK 0xff

// structure for physical memory, we will use it to replace original memory structure
typedef struct physical_memory
{
    spinlock_t lock;                // The spinclock that protects this structure
    uint32_t pm_type;               // 
    uint32_t pm_stype;              //
    uint32_t pm_dtype;              //
    uint32_t pm_flags;              //
    uint32_t pm_status;             //
    uint64_t pm_saddr;              //
    uint64_t pm_size;               //
    uint64_t pm_end;                //
    uint64_t pm_rrvmsaddr;          //
    uint64_t pm_rrvmend;            //
} physical_memory_t;

void init_halphymem();

#endif