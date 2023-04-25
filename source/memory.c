#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "../include/ansi.h"
#include "../include/commons.h"
#include "../include/memory.h"

uint8_t* physical; 	// Physical memory. Size = 2²⁴ words
uint32_t  kernel_nfi;	// Index to next free space in kernel-owned memory
uint32_t  nfi;          // Index to next free space in memory


// Initialize memory environment
void init_memory() {
	physical = (uint8_t *) calloc(0x1000000, sizeof(uint8_t));
	kernel_nfi = 0;
	nfi = KERNEL_LIMIT;
}

// Create page table
uint8_t * create_page_table() {
	if (kernel_nfi >= KERNEL_LIMIT) // if pagetable > 0x3FFFFF
		return NULL;
	
	uint8_t* ptbr = &physical[kernel_nfi];
	kernel_nfi += PAGE_TABLE_SIZE;
	return ptbr;
}