#include "gdtidt.h"
#include <hal/halglobal.h>

HAL_DEFGLOB_VARIABLE(gate_t, x64_idt)[IDT_MAX];

typedef void (*int_handler_t)();

void set_int_desc(uint8_t vector, uint8_t desc_type, int_handler_t handler, uint8_t privilege)
{
    gate_t *p_gate = &x64_idt[vector];
    uint64_t base = (uint64_t)handler;
    p_gate->offset1 = (uint16_t)base;
    p_gate->offset2 = (uint16_t)((base >> 16) & 0xffff);
    p_gate->offset3 = (uint16_t)(base >> 32 & 0xffffffff);

    p_gate->selector = SELECTOR_KERNEL_CS;
    p_gate->dcount = 0;

    p_gate->attr = (uint8_t)(desc_type | (privilege << 5));
}

void init_idt()
{
    for (uint16_t i = 0; i <= 255; i++)
    {
        set_int_desc((uint32_t)i, DA_386IGate, hxi_exc_general_intpfault, PRIVILEGE_KRNL);
    }
}