#ifndef _CONSOLE_H
#define _CONSOLE_H

#include <kprint.h>
#include <type.h>

KLINE void panic(char *info) {
  kprint(info);
  while (TRUE);
}

#endif