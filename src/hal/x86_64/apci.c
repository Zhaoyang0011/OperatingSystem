#include "apci.h"

#include <kprint.h>
#include <string.h>

uchar_t chk_sum(uchar_t *addr, uint32_t len) {
    sint32_t i;

    uchar_t sum = 0;
    for (i = 0; i < len; i++)
        sum += addr[i];
    return sum;
}

bool_t chk_rsdp_sum(uchar_t *addr) {
    if (addr == NULL)
        return FALSE;

    sint_t sum = 1;
    RSDP_t *rsdp = (RSDP_t *) addr;
    XSDP_t *xsdp = (XSDP_t *) addr;

    if (rsdp->Revision == 0)
        sum = chk_sum(addr, sizeof(RSDP_t));
    else if (rsdp->Revision == 1)
        sum = chk_sum(addr, xsdp->Length);

    return sum == 0;
}

void *find_rsdp_range(uchar_t *start, uint32_t length) {
    uchar_t *end, *p;
    end = start + length;

    for (p = start; p < end; p += 16)
        if (strncmp(p, "RSD PTR ", 8) == 0 && chk_rsdp_sum(p))
            return (RSDP_t *) p;

    return NULL;
}

void *find_sdt(uchar_t *signature, uchar_t *start, uint32_t length) {
    uchar_t *end, *p;
    end = start + length;

    for (p = start; p < end; p += 1)
        if (strncmp(p, signature, 4) == 0 && chk_rsdp_sum(p))
            return (RSDP_t *) p;

    return NULL;
}

void *find_rsdp() {
    uchar_t *bda = (uchar_t *) (0x400);
    uchar_t *p;
    void *rsdp = NULL;

    p = (uchar_t *) (((bda[0x0F] << 8) | bda[0x0E]) << 4);
    rsdp = find_rsdp_range(p, 1024);
    if (rsdp != NULL)
        return rsdp;

    return find_rsdp_range((uchar_t *) 0xE0000, 0x20000);
}

void *find_madt_rsdt(RSDT_t *redt) {
    if (redt == NULL)
        return NULL;

    if (chk_sum(&redt->header, redt->header.Length) != 0)
        return NULL;

    int length = (redt->header.Length - sizeof(ACPISDTHeader_t)) / 4;
    uint32_t *otherSDT = redt->PointerToOtherSDT;
    for (int i = 0; i < length; i++) {
        char *sdt = (char *) otherSDT[i];
        ACPISDTHeader_t *header = sdt;
        if (strncmp(sdt, "APIC", 4) == 0 && chk_sum(sdt, header->Length) == 0)
            return (char *) otherSDT[i];
    }

    return NULL;
}

void *find_madt_xsdt(XSDT_t *xedt) {
    if (xedt == NULL)
        return NULL;

    if (chk_sum(&xedt->header, xedt->header.Length) != 0)
        return NULL;

    int length = (xedt->header.Length - sizeof(ACPISDTHeader_t)) / 4;
    uint64_t *otherSDT = xedt->PointerToOtherSDT;
    for (int i = 0; i < length; i++) {
        char *sdt = (char *) otherSDT[i];
        ACPISDTHeader_t *header = sdt;
        if (strncmp(sdt, "APIC", 4) == 0 && chk_sum(sdt, header->Length) == 0)
            return (char *) otherSDT[i];
    }

    return NULL;
}

MADT_t *find_madt() {
    void *rsdp_adr = find_rsdp();

    if (rsdp_adr == NULL) {
        kprint("Unable to find RSDP!");
        while (TRUE);
    }

    XSDP_t *xsdp = rsdp_adr;

    if (xsdp->Revision == 0)
        return find_madt_rsdt((void *) xsdp->RsdtAddress);

    return find_madt_xsdt((void *) xsdp->XsdtAddress);
}