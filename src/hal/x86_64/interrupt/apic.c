#include "apic.h"
#include "../apci.h"
#include "../x86.h"
#include "i8259.h"
#include <console.h>

bool_t cpu_has_msr()
{
    cpuid_desc_t cpuid_desc;
    cpuid_desc.function_id = 1;
    cpuid(&cpuid_desc);
    return cpuid_desc.info[3] & CPUID_FLAG_MSR;
}

void read_msr(uint32_t msr, uint32_t *lo, uint32_t *hi)
{
    __asm__ __volatile__("rdmsr" : "=a"(*lo), "=d"(*hi) : "c"(msr));
}

void write_msr(uint32_t msr, uint32_t lo, uint32_t hi)
{
    __asm__ __volatile__("wrmsr" : : "a"(lo), "d"(hi), "c"(msr));
}

void local_apic_found(localapic *local_apic)
{
}

void io_apic_found(ioapic *local_apic)
{
}

void find_one_apic(APICHeader_t *apic_header)
{
    switch (apic_header->type)
    {
    case APIC_LOCAL: {
        local_apic_found((localapic *)apic_header);
        break;
    }
    case APIC_IO: {
        io_apic_found((ioapic *)apic_header);
        break;
    }
    case APIC_IO_ISO: {
        // TODO: deal with ioapic_iso
        ioapic_iso *apic2 = (ioapic_iso *)apic_header;
        break;
    }
    case APIC_IO_NMIS: {
        // TODO: deal with ioapic_nmis
        ioapic_nmis *apic3 = (ioapic_nmis *)apic_header;
        break;
    }
    case APIC_LOCAL_NMI: {
        // TODO: deal with lapic_nmi
        lapic_nmi *apic4 = (lapic_nmi *)apic_header;
        break;
    }
    case APIC_LOCAL_ADR_OVR: {
        // TODO: deal with lapic_adr_ovr
        lapic_adr_ovr *apic5 = (lapic_adr_ovr *)apic_header;
        break;
    }
    case APIC_LOCAL_X2: {
        // TODO: deal with lapic_x2
        lapic_x2 *apic9 = (lapic_x2 *)apic_header;
        break;
    }
    default: {
        panic("APIC type error!");
        break;
    }
    }
}

void find_apic(MADT_t *madt)
{
    if (madt == NULL)
        panic("MADT address error!");

    // set local apic address
    uint8_t *end = ((uint8_t *)madt) + madt->header.Length;
    lapic = (uint32_t *)madt->lapicaddr;

    for (uint8_t *apic_desc = madt->entry; apic_desc < end;)
    {
        APICHeader_t *header = (APICHeader_t *)apic_desc;
        find_one_apic(header);
        apic_desc += header->length;
    }
}

void init_apic()
{
    if (!lapic)
        panic("Local apic not initiated!");

    disable_i8259();

    // Enable local APIC; set spurious interrupt vector.
    lapicw(LAPIC_SVR, LAPIC_SVR_ENABLE | (T_IRQ0 + IRQ_SPURIOUS));

    // The timer repeatedly counts down at bus frequency
    // from lapic[TICR] and then issues an interrupt.
    // If xv6 cared more about precise timekeeping,
    // TICR would be calibrated using an external time source.
    lapicw(LAPIC_TIMER_DCR, LAPIC_TIMER_X1);
    lapicw(LAPIC_TIMER, LAPIC_TIMER_PERIODIC | (T_IRQ0 + IRQ_TIMER));
    lapicw(LAPIC_TIMER_ICR, 10000000);

    // Disable logical interrupt lines.
    lapicw(LAPIC_LINT0, LAPIC_MASKED);
    lapicw(LAPIC_LINT1, LAPIC_MASKED);

    // Disable performance counter overflow interrupts
    // on machines that provide that interrupt entry.
    if (((lapic[LAPIC_VER] >> 16) & 0xFF) >= 4)
        lapicw(LAPIC_PCINT, LAPIC_MASKED);

    // Map error interrupt to IRQ_ERROR.
    lapicw(LAPIC_ERROR, T_IRQ0 + IRQ_ERROR);

    // Clear error status register (requires back-to-back writes).
    lapicw(LAPIC_ESR, 0);
    lapicw(LAPIC_ESR, 0);

    // Ack any outstanding interrupts.
    lapicw(LAPIC_EOI, 0);

    // Send an Init Level De-Assert to synchronise arbitration ID's.
    lapicw(LAPIC_ICRHI, 0);
    lapicw(LAPIC_ICRLO, LAPIC_ICRLO_BCAST | LAPIC_ICRLO_INIT | LAPIC_ICRLO_LEVEL);
    while (lapic[LAPIC_ICRLO] & LAPIC_ICRLO_DELIVS)
        ;

    // Enable interrupts on the APIC (but not on the processor).
    lapicw(LAPIC_TPR, 0);
}