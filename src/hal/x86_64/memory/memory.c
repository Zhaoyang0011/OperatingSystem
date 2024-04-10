#include "memarea.h"
#include <hal/memory/memory.h>
#include <hal/memory/mempage.h>
#include <hal/memory/phymem.h>

// order of the fuction call matters
void init_memory()
{
    init_physical_memory();
    init_memory_page();
    init_mempage_occupation();
    init_memory_area();
    load_mempage_memarea();
}