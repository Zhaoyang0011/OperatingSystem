#ifndef _LDR_H
#define _LDR_H

/**
 * File name: ldr.h
 * Author: Zhaoyang Shi
 * Description: Header file of loader.
 */

#include <ldrtype.h>

/**
 * This is the main function of the loader. 
 * It will call other functions that prepares our machine info.
 */
void init_machine_param();

#endif