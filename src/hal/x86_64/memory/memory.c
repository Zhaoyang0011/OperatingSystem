#include <hal/memory/memory.h>
#include <hal/memory/mempage.h>
#include <hal/memory/phymem.h>

void init_memory()
{
    init_physical_memory();
    init_memory_page();
}