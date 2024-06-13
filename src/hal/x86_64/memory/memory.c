#include <hal/memory/memobject.h>
#include <hal/memory/memarea.h>
#include <hal/memory/memgrob.h>
#include <hal/memory/memory.h>
#include <hal/memory/mempage.h>
#include <hal/memory/phymem.h>
#include <hal/memory/memdivmer.h>
#include "console.h"

// order of the fuction call matters
void init_memory() {
    init_physical_memory();
    init_memory_page();
    init_mempage_occupation();
    init_memory_area();
    load_mempage_memarea();
    init_memory_object_manager();
    init_memgrob();

    void * ret = mobj_alloc(32);
    if(ret == NULL)
        panic("Fuck you!");
}