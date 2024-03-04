#define HALGOBAL_HEAD
#include "interrupt/gdtidt.h"
#include <hal/halglobal.h>

HAL_DEFGLOB_VARIABLE(uint32_t *, lapic);
HAL_DEFGLOB_VARIABLE(gate_t, x64_idt)[IDT_MAX];