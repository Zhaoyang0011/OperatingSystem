#ifndef _HAL_GLOBAL_H
#define _HAL_GLOBAL_H

#include <ldrtype.h>
#include <hal/cpu.h>

#ifdef HALGOBAL_HEAD
#undef EXTERN
#define EXTERN
#endif

#define HAL_DEFGLOB_VARIABLE(vartype, varname) \
    EXTERN __attribute__((section(".data"))) vartype varname

HAL_DEFGLOB_VARIABLE(kernel_desc_t, kernel_descriptor);

#endif