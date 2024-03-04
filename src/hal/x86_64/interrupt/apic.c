#include "apic.h"
#include "../apci.h"
#include "../x86.h"
#include <console.h>
#include <hal/halglobal.h>

HAL_DEFGLOB_VARIABLE(uint32_t *, lapic);

bool_t cpuHasMSR()
{
    static uint32_t a, d; // eax, edx
    cpuid_desc_t cpuid_desc;
    cpuid(&cpuid_desc);
    return d & CPUID_FLAG_MSR;
}

void cpuGetMSR(uint32_t msr, uint32_t *lo, uint32_t *hi)
{
    asm volatile("rdmsr" : "=a"(*lo), "=d"(*hi) : "c"(msr));
}

void cpuSetMSR(uint32_t msr, uint32_t lo, uint32_t hi)
{
    asm volatile("wrmsr" : : "a"(lo), "d"(hi), "c"(msr));
}

void init_local_apic(localapic *local_apic)
{
}

void init_io_apic(ioapic *io_apic)
{
}

void init_one_apic(APICHeader_t *apic_header)
{
    switch (apic_header->type)
    {
    case APIC_LOCAL: {
        init_local_apic((localapic *)apic_header);
        break;
    }
    case APIC_IO: {
        init_io_apic((ioapic *)apic_header);
        break;
    }
    case APIC_IO_ISO: {
        ioapic_iso *apic2 = (ioapic_iso *)apic_header;
        break;
    }
    case APIC_IO_NMIS: {
        ioapic_nmis *apic3 = (ioapic_nmis *)apic_header;
        break;
    }
    case APIC_LOCAL_NMI: {
        lapic_nmi *apic4 = (lapic_nmi *)apic_header;
        break;
    }
    case APIC_LOCAL_ADR_OVR: {
        lapic_adr_ovr *apic5 = (lapic_adr_ovr *)apic_header;
        break;
    }
    case APIC_LOCAL_X2: {
        lapic_x2 *apic9 = (lapic_x2 *)apic_header;
        break;
    }
    default: {
        panic("APIC type error!");
        break;
    }
    }
}

void init_apic(MADT_t *madt)
{
    if (madt == NULL)
        panic("MADT address error.");

    // set local apic address
    uint8_t *end = ((uint8_t *)madt) + madt->header.Length;
    lapic = (uint32_t *)madt->lapicaddr;

    for (uint8_t *apic_desc = madt->entry; apic_desc < end;)
    {
        APICHeader_t *header = (APICHeader_t *)apic_desc;
        init_one_apic(header);
        apic_desc += header->length;
    }
}