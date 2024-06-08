#include <hal/memory/memarea.h>
#include <hal/memory/memgrob.h>
#include <hal/memory/memory.h>
#include <hal/memory/mempage.h>
#include <hal/memory/phymem.h>
#include <hal/memory/memdivmer.h>

// order of the fuction call matters
void init_memory()
{
    init_physical_memory();
    init_memory_page();
    init_mempage_occupation();
    init_memory_area();
    load_mempage_memarea();
    init_memgrob();

    uint_t ret_pgnr;
    mpdesc_t* mp = memory_divide_pages(&memgrob, 7, &ret_pgnr, MA_TYPE_KRNL, DMF_RELDIV);
    memory_merge_pages(&memgrob, mp,ret_pgnr);
    while (TRUE)
    {
        /* code */
    }
    
}