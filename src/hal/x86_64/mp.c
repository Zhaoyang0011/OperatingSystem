#include <console.h>
#include <hal/mp.h>

#include "apci.h"
#include "interrupt/apic.h"

void init_mp() {
  MADT_t *madt = find_madt();
  if (madt == NULL)
	panic("MADT address error.");

  find_apic(madt);
}