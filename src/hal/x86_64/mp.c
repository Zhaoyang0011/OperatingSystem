#include <console.h>
#include <hal/mp.h>

#include "apci.h"
#include "interrupt/apic.h"

void init_mp()
{
    MADT_t *madt = search_madt();
    if (madt == NULL)
        panic("MADT address error.");

    init_apic(madt);
}