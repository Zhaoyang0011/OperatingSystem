#define HALGOBAL_HEAD
#include "interrupt/gdtidt.h"
#include <hal/halglobal.h>

HAL_DEFGLOB_VARIABLE(volatile uint32_t *, lapic);

// gdtidt.h
HAL_DEFGLOB_VARIABLE(gate_t, x64_idt)[IDT_MAX];
HAL_DEFGLOB_VARIABLE(idtr_t, idtr);