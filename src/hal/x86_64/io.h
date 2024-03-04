#ifndef _IO_H
#define _IO_H

#include <type.h>

KLINE void outb(uint16_t port, uint8_t data)
{
    __asm__ __volatile__("out %0,%1" : : "a"(data), "dN"(port));
}

#endif