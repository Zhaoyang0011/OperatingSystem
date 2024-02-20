#ifndef _MULTI_PROCESS_H
#define _MULTI_PROCESS_H

#include <type.h>

typedef struct multi_proc
{                         // floating pointer
    uchar_t signature[4]; // "_MP_"
    void *physaddr;       // phys addr of MP config table
    uchar_t length;       // 1
    uchar_t specrev;      // [14]
    uchar_t checksum;     // all bytes must add up to 0
    uchar_t type;         // MP system config type
    uchar_t imcrp;
    uchar_t reserved[3];
} __attribute__((packed)) multi_proc_t;

#endif