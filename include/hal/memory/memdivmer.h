#ifndef _MEMDIVMER_H
#define _MEMDIVMER_H

#include <hal/memory/memgrob.h>
#include <hal/memory/mempage.h>

#define DMF_RELDIV 0
#define DMF_MAXDIV 1

mpdesc_t *memory_divide_pages(memgrob_t *mmobjp, uint_t pages, uint_t *retrealpnr, uint_t mrtype, uint_t flgs);
bool_t memory_merge_pages(memgrob_t *mmobjp, mpdesc_t *freemsa, uint_t freepgs);

#endif