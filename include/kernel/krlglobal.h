#ifndef _KRLGLOBAL_H
#define _KRLGLOBAL_H

#define KRL_DEFGLOB_VARIABLE(vartype, varname) __attribute__((section(".data"))) vartype varname

#endif
