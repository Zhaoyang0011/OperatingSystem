#define HALGOBAL_HEAD

#include "interrupt/gdtidt.h"
#include "interrupt/apic.h"
#include <hal/cpu.h>
#include <hal/halglobal.h>
#include <hal/interrupt.h>
#include <hal/memory/memarea.h>
#include <hal/memory/memgrob.h>

// gdtidt.c
HAL_DEFGLOB_VARIABLE(gate_t, x64_idt)[IDT_MAX];
HAL_DEFGLOB_VARIABLE(idtr_t, idtr);
HAL_DEFGLOB_VARIABLE(int_flt_desc_t, int_fault)[IDT_MAX];
HAL_DEFGLOB_VARIABLE(gdtr_t, x64_gdtr)[CPU_CORE_MAX];
HAL_DEFGLOB_VARIABLE(descriptor_t, x64_gdt)[CPU_CORE_MAX][GDT_MAX];
HAL_DEFGLOB_VARIABLE(x64tss_t, x64_tss)[CPU_CORE_MAX];

// lapic.c
HAL_DEFGLOB_VARIABLE(volatile uint32_t *, lapic);
HAL_DEFGLOB_VARIABLE(volatile uint32_t, lapic_cpu)[LAPIC_MAX];

// memarea.c
HAL_DEFGLOB_VARIABLE(memarea_t, memarea_arr)[MEMAREA_MAX];

// memgrob.c
HAL_DEFGLOB_VARIABLE(memgrob_t, memgrob);
