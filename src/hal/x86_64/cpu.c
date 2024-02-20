#include <hal/cpu.h>
#include <kprint.h>
#include <memory.h>
#include <type.h>

#include "sdt.h"

static uchar_t sum(uchar_t *addr, int len)
{
    int i, sum;

    sum = 0;
    for (i = 0; i < len; i++)
        sum += addr[i];
    return sum;
}

uchar_t chk_sum(uchar_t *addr, uint32_t len)
{
    sint32_t i;

    uchar_t sum = 0;
    for (i = 0; i < len; i++)
        sum += addr[i];
    return sum;
}

bool_t chk_rsdp_sum(uchar_t *addr)
{
    if (addr == NULL)
        return FALSE;

    sint_t sum = 1;
    RSDP_t *rsdp = (RSDP_t *)addr;
    XSDP_t *xsdp = (XSDP_t *)addr;

    if (rsdp->Revision == 0)
        sum = chk_sum(addr, sizeof(RSDP_t));
    else if (rsdp->Revision == 1)
        sum = chk_sum(addr, xsdp->Length);

    return sum == 0;
}

void *search_multiproc_range(uchar_t *start, uint32_t length)
{
    uchar_t *end, *p;
    end = start + length;

    for (p = start; p < end; p += 16)
        if (strncmp(p, "RSD PTR ", 8) == 0 && chk_rsdp_sum(p))
            return (RSDP_t *)p;

    return NULL;
}

void *search_multiproc()
{
    uchar_t *bda = (uchar_t *)(0x400);
    uchar_t *p;
    void *mp = NULL;

    p = (uchar_t *)(((bda[0x0F] << 8) | bda[0x0E]) << 4);
    mp = search_multiproc_range(p, 1024);
    if (mp != NULL)
        return mp;

    return search_multiproc_range((uchar_t *)0xE0000, 0x20000);
}

void find_mp(XSDP_t *xsdp)
{
    void *addr = NULL;
    if (xsdp->Revision == 0)
        addr = (void *)xsdp->RsdtAddress;
    else if (xsdp->Revision == 2)
        addr = (void *)xsdp->XsdtAddress;
}

void init_mp()
{
    uchar_t *rsdp_adr = search_multiproc();

    if (rsdp_adr == NULL)
    {
        kprint("Incorrect kernel head!");
        while (TRUE)
            ;
    }

    XSDP_t *xsdp = (XSDP_t *)rsdp_adr;
    find_mp(xsdp);
}

void init_cpu()
{
    init_mp();
}