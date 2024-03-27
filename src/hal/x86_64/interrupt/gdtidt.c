#include "gdtidt.h"

extern void hxi_exc_general_intpfault();
extern void exc_divide_error();

void set_int_desc(uint8_t vector, uint8_t desc_type, int_handler_t handler, uint8_t privilege)
{
    gate_t *p_gate = &x64_idt[vector];
    uint64_t base = (uint64_t)handler;
    p_gate->offset1 = (uint16_t)(base & 0xffff);
    p_gate->offset2 = (uint16_t)((base >> 16) & 0xffff);
    p_gate->offset3 = (uint16_t)(base >> 32 & 0xffffffff);

    p_gate->selector = SELECTOR_KERNEL_CS;
    p_gate->dcount = 0;

    p_gate->attr = (uint8_t)(desc_type | (privilege << 5));
}

KLINE void load_x64_idt(idtr_t *idtr)
{
    __asm__ __volatile__("lidt (%0) \n\t"

                         :
                         : "r"(idtr)
                         : "memory");
    return;
}

void set_idtr(gate_t *idtptr)
{
    idtr.idtbass = (uint64_t)idtptr;
    idtr.idtLen = sizeof(x64_idt) - 1;
}

void init_idt()
{
    for (uint16_t i = 0; i < 256; i++)
    {
        set_int_desc((uint32_t)i, DA_386IGate, hxi_exc_general_intpfault, PRIVILEGE_KRNL);
    }
    
    set_int_desc(INT_VECTOR_DIVIDE, DA_386IGate, exc_divide_error, PRIVILEGE_KRNL);

    set_idtr(x64_idt);
    load_x64_idt(&idtr);
    return;
}