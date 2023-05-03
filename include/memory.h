#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include "commons.h"

#define KERNEL_LIMIT 0x400000
#define PAGE_TABLE_SIZE 16

void init_memory();
pte_t * create_page_table();

extern uint8_t * physical; 	    // Physical memory. Size = 2²⁴ bytes
extern uint32_t  kernel_nfi;	// Index to next free space in kernel-owned memory
extern uint32_t  nfi;           // Index to next free space in memory

#endif