#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "../include/ansi.h"
#include "../include/commons.h"
#include "../include/memory.h"

uint8_t * physical; 	// Physical memory. Size = 2²⁴ bytes
uint32_t  kernel_nfi;	// Pointer to next free index in kernel-owned memory


// Initialize memory environment
void init_memory() {
	physical = (uint8_t *) calloc(0x1000000, sizeof(uint8_t));
	kernel_nfi = 0;
}

// Create page table
uint8_t * create_page_table() {
	if (kernel_nfi > KERNEL_LIMIT)
		return NULL;
	
	uint8_t* page_table = &physical[kernel_nfi];
	kernel_nfi += PAGE_TABLE_SIZE;
	return page_table;
}