#include "gdtidt.h"

extern void hxi_exc_general_intpfault();
extern void exc_divide_error();
extern void exc_nmi();
extern void exc_breakpoint_exception();
extern void exc_overflow();
extern void exc_bounds_check();
extern void exc_inval_opcode();
extern void exc_copr_not_available();
extern void exc_double_fault();
extern void exc_copr_seg_overrun();
extern void exc_inval_tss();
extern void exc_segment_not_present();
extern void exc_stack_exception();
extern void exc_general_protection();
extern void exc_page_fault();
extern void exc_copr_error();
extern void exc_alignment_check();
extern void exc_machine_check();
extern void exc_simd_fault();
extern void hxi_exc_general_intpfault();
extern void hxi_hwint00();
extern void hxi_hwint01();
extern void hxi_hwint02();
extern void hxi_hwint03();
extern void hxi_hwint04();
extern void hxi_hwint05();
extern void hxi_hwint06();
extern void hxi_hwint07();
extern void hxi_hwint08();
extern void hxi_hwint09();
extern void hxi_hwint10();
extern void hxi_hwint11();
extern void hxi_hwint12();
extern void hxi_hwint13();
extern void hxi_hwint14();
extern void hxi_hwint15();
extern void hxi_hwint16();
extern void hxi_hwint17();
extern void hxi_hwint18();
extern void hxi_hwint19();
extern void hxi_hwint20();
extern void hxi_hwint21();
extern void hxi_hwint22();
extern void hxi_hwint23();
extern void hxi_apic_svr();
extern void hxi_apic_ipi_schedul();
extern void hxi_apic_timer();
extern void hxi_apic_thermal();
extern void hxi_apic_performonitor();
extern void hxi_apic_lint0();
extern void hxi_apic_lint1();
extern void hxi_apic_error();
extern void exi_sys_call();
extern void _ret_from_user_mode();

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

    set_int_desc(INT_VECTOR_PAGE_FAULT, DA_386IGate, exc_page_fault, PRIVILEGE_KRNL);

    set_idtr(x64_idt);
    load_x64_idt(&idtr);
    return;
}