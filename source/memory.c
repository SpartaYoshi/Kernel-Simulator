#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "../include/ansi.h"
#include "../include/commons.h"
#include "../include/memory.h"

uint8_t*  physical; 	 // Physical memory. Size = 2²⁴ words
nfsp_t*   kernel_space;	 // Head of list of pointers to empty space in kernel-owned memory
nfsp_t*   free_space;    // Head of list of pointers to empty space in memory

// Initialize memory environment
void init_memory() {
	// Allocate memory
	physical = (uint8_t *) calloc(0x1000000, sizeof(uint8_t));

	// Start list of free space for kernel-owned memory
	kernel_space = (nfsp_t *) malloc(sizeof(nfsp_t));
	kernel_space->prev	  = NULL;
	kernel_space->next    = NULL;
	kernel_space->address = 0;
	kernel_space->length  = KERNEL_LIMIT;
	
	// Start list of free space by putting all memory left as free space
	free_space = (nfsp_t *) malloc(sizeof(nfsp_t));
	free_space->prev	  = NULL;
	free_space->next      = NULL;
	free_space->address   = KERNEL_LIMIT;
	free_space->length    = 0x1000000 - KERNEL_LIMIT;
}

// Create page table
pte_t * create_page_table() {

	// Check for free space
	nfsp_t* nfsp = kernel_space;
	while(nfsp->length < (PAGE_TABLE_SIZE * sizeof(pte_t)) && nfsp->next != NULL)
		nfsp = nfsp->next;
	
	// Finished checking. If still nothing, exit
	if (nfsp == NULL)  					 				 return NULL;
	if (nfsp->length < PAGE_TABLE_SIZE * sizeof(pte_t))  return NULL; 
	
	// Get address of space = ptbr
	pte_t* ptbr = (pte_t *) &physical[nfsp->address];
	
	// Update slot
	nfsp->length  -= PAGE_TABLE_SIZE * sizeof(pte_t); // Reduce size
	if (nfsp->length <= 0){							  // Pop & delete if no more space left
		nfsp->prev->next = nfsp->next;
		nfsp->next->prev = nfsp->prev;
		free(nfsp);
	} else {
		nfsp->address += PAGE_TABLE_SIZE * sizeof(pte_t); 
	}

	return ptbr;
}

// Alloc free space
uint32_t alloc_page(){
	// Check for free space
	nfsp_t* nfsp = free_space;
	while(nfsp->length < PAGE_SIZE && nfsp->next != NULL)
		nfsp = nfsp->next;
	
	// Finished checking. If still nothing, exit
	if (nfsp == NULL)  			   return -1;
	if (nfsp->length < PAGE_SIZE)  return -1;
	
	// Get address of space = ptbr
	uint32_t addr = nfsp->address;
	
	// Update slot
	nfsp->length  -= PAGE_SIZE; // Reduce size
	if (nfsp->length <= 0){							  // Pop & delete if no more space left
		nfsp->prev->next = nfsp->next;
		nfsp->next->prev = nfsp->prev;
		free(nfsp);
	} else {
		nfsp->address += PAGE_SIZE; 
	}

	return addr;
}

uint32_t memread(uint32_t address){
	uint32_t word;
	word =         physical[address + 3];
	word = word | (physical[address + 2] << 8); 
	word = word | (physical[address + 1] << 16);
	word = word | (physical[address]     << 24);
    return word;
}


void memwrite(uint32_t address, uint32_t data){
	physical[address + 3] =   data & 0x000000FF;
	physical[address + 2] = ((data & 0x0000FF00) >>  8);
	physical[address + 1] = ((data & 0x00FF0000) >> 16);
	physical[address + 0] = ((data & 0xFF000000) >> 24);
}


void insert_frame(pcb_t * proc, uint32_t physadr){
	uint32_t nPag = proc->mm.pt_entries++; // - 1
	proc->mm.pgb[nPag].frame_address = physadr;
}

uint32_t get_frame(pcb_t * proc, uint32_t nPag){
	return proc->mm.pgb[nPag].frame_address;
}

uint32_t translate(thread_t * th, uint32_t logicadr){
	uint32_t offset = logicadr & OFFSET_MASK;
	uint32_t nPag   = logicadr >> OFFSET_LEN;

	// Check TLB cache
	int i;
	for (i = 0; i < TLB_CAPACITY; i++)
		if (th->mmu.tlb[i].nPag == nPag)
			return (th->mmu.tlb[i].nFrame << OFFSET_LEN) | offset; // hit = immediate return

	// Miss. Consult memory
	return (get_frame(th->proc, nPag) << OFFSET_LEN) | offset;
}