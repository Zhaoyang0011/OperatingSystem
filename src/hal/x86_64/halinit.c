#include <hal/halinit.h>
#include <hal/interrupt.h>
#include <hal/mp.h>
#include <hal/phymem.h>
#include <ldrtype.h>

void init_hal()
{
    init_physical_memory();
    init_mp();
    init_interrupt();
}