#include <hal/halinit.h>
#include <hal/phymem.h>
#include <hal/interrupt.h>
#include <ldrtype.h>

void init_hal()
{
    init_physical_memory();
    //init_interrupt();
}