#define HALGOBAL_HEAD

#include "interrupt/gdtidt.h"
#include <hal/halglobal.h>
#include <hal/interrupt.h>
#include <hal/memory/memarea.h>
#include <hal/memory/memgrob.h>

// gdtidt.h
HAL_DEFGLOB_VARIABLE(gate_t, x64_idt)[IDT_MAX];
HAL_DEFGLOB_VARIABLE(idtr_t, idtr);
HAL_DEFGLOB_VARIABLE(int_flt_desc_t, int_fault)[IDT_MAX];

// lapic.h
HAL_DEFGLOB_VARIABLE(volatile uint32_t *, lapic);

// mempage.y
HAL_DEFGLOB_VARIABLE(uint64_t, occunr);

// memarea.h
HAL_DEFGLOB_VARIABLE(memarea_t, memarea_arr)[MEMAREA_MAX];
HAL_DEFGLOB_VARIABLE(uint64_t, frenr);

// memgrob.h
HAL_DEFGLOB_VARIABLE(memgrob_t, memgrob);