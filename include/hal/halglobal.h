#ifndef _HAL_GLOBAL_H
#define _HAL_GLOBAL_H

#include <ldrtype.h>

#define HAL_DEFGLOB_VARIABLE(vartype,varname) \
__attribute__((section(".data"))) vartype varname

#ifdef X86_64
HAL_DEFGLOB_VARIABLE(kernel_desc_t, kernel_info);
#endif

#endif