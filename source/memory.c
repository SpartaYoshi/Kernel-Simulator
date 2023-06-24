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
	printf("%sInitiated:%s Memory (16 MB)\n",\
		 C_BCYN, C_RESET);

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


// Create free slot node on list of empty spaces
void create_nfs_node(nfsp_t * prev, nfsp_t * next, uint32_t address, uint32_t length){
	nfsp_t * new_fs = (nfsp_t *) malloc(sizeof(nfsp_t));

	// Link to chain
	new_fs->next = next;
	new_fs->prev = prev;

	if (prev != NULL) prev->next = new_fs;
	if (next != NULL) next->prev = new_fs;

	// Copy info
	new_fs->address = address;
	new_fs->length = length;
}


// Delete free slot node from list of empty spaces
void delete_nfs_node(nfsp_t * nfsp){
	nfsp->prev->next = nfsp->next;
	nfsp->next->prev = nfsp->prev;
	free(nfsp);
}


// Create page table
void create_page_table(pcb_t * proc) {

	printf("%smemory     %s>>   Creating page table for process %d...\n",
		C_BWHT, C_RESET, proc->pid);

	// Check for free space
	nfsp_t* nfsp = kernel_space;
	while(nfsp->length < (PAGE_TABLE_SIZE * sizeof(pte_t)) && nfsp->next != NULL)
		nfsp = nfsp->next;
	
	// Finished checking. If still nothing, exit
	if (nfsp == NULL)  					 				 return;
	if (nfsp->length < PAGE_TABLE_SIZE * sizeof(pte_t))  return; 
	
	// Get address of space = ptbr
	proc->mm.pgb = (pte_t *) &physical[nfsp->address];
	proc->mm.pt_address = nfsp->address;
	
	// Update slot
	nfsp->length -= PAGE_TABLE_SIZE * sizeof(pte_t); // Reduce size
	if (nfsp->length <= 0){							 // Pop & delete if no more space left
		delete_nfs_node(nfsp);
	} else {
		nfsp->address += PAGE_TABLE_SIZE * sizeof(pte_t); 
	}	
}


// Delete page table
void delete_page_table(pcb_t * proc) {

	printf("%smemory     %s>>   Deleting page table for process %d...\n",
		C_BWHT, C_RESET, proc->pid);

	nfsp_t * nfsp = kernel_space;
	int extended_prev, extended_next;
	uint32_t pt_adr = proc->mm.pt_address; // alias

	// Go to location in kernel memory
	while (nfsp->address < pt_adr) nfsp = nfsp->next;
	
	nfsp_t * prev = nfsp->prev; // alias
	extended_prev = extended_next = 0;

	// Check if it can be an extension of an existing nfs
	// If merges with previous slot
	if (prev != NULL && (prev->address + prev->length) == pt_adr){
		prev->length += PAGE_TABLE_SIZE * sizeof(pte_t); // Increase size
		extended_prev = 1;
	}

	// If merges with next slot
	if (pt_adr + (PAGE_TABLE_SIZE * sizeof(pte_t)) == nfsp->address){
		// If also merged with previous, merge both slots and delete duplicate
		if (extended_prev){
			prev->length += nfsp->length;
			delete_nfs_node(nfsp);
		} else {
			nfsp->address = pt_adr;
			nfsp->length += PAGE_TABLE_SIZE * sizeof(pte_t);
		}
	}

	// If not an extension, create new node
	if (extended_prev || extended_next) return;
	create_nfs_node(prev, nfsp, pt_adr, PAGE_TABLE_SIZE * sizeof(pte_t));
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
	nfsp->length -= PAGE_SIZE; 						// Reduce size
	if (nfsp->length <= 0){							// Pop & delete if no more space left
		delete_nfs_node(nfsp);
	} else {
		nfsp->address += PAGE_SIZE; 
	}

	printf("%smemory     %s>>   Reserved space in memory at mem address 0x%08x.\n",
		C_BWHT, C_RESET, addr);
	return addr;
}


// Free page from memory
void free_page(uint32_t pg_adr) {
	printf("%smemory     %s>>   Freeing space in memory at mem address 0x%08x...\n",
		C_BWHT, C_RESET, pg_adr);
		
	nfsp_t * nfsp = free_space;
	int extended_prev, extended_next;

	// Go to location in kernel memory
	while (nfsp->address < pg_adr) nfsp = nfsp->next;
	
	nfsp_t * prev = nfsp->prev; // alias
	extended_prev = extended_next = 0;

	// Check if it can be an extension of an existing nfs
	// If merges with previous slot
	if (prev != NULL && (prev->address + prev->length) == pg_adr){
		prev->length += PAGE_SIZE; // Increase size
		extended_prev = 1;
	}

	// If merges with next slot
	if (pg_adr + PAGE_SIZE == nfsp->address){
		// If also merged with previous, merge both slots and delete duplicate
		if (extended_prev){
			prev->length += nfsp->length;
			delete_nfs_node(nfsp);
		} else {
			nfsp->address = pg_adr;
			nfsp->length += PAGE_SIZE;
		}
	}

	// If not an extension, create new node
	if (extended_prev || extended_next) return;
	create_nfs_node(prev, nfsp, pg_adr, PAGE_SIZE);
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
	uint32_t nPag = proc->mm.pt_entries++;
	proc->mm.pgb[nPag].frame_address = physadr;
}


uint32_t get_frame(pcb_t * proc, uint32_t nPag){
	return proc->mm.pgb[nPag].frame_address;
}


uint32_t translate(thread_t * th, uint32_t logicadr){
	uint32_t offset = logicadr & OFFSET_MASK;
	uint32_t nPag   = logicadr & FRAME_MASK;
	uint32_t nFrame;

	// Check TLB cache
	int i;
	for (i = 0; i < TLB_CAPACITY; i++)
		if (th->proc->pid == th->mmu.tlb[i].pid
		 && th->mmu.tlb[i].nPag == nPag
		 && th->mmu.tlb[i].valid){ 
			nFrame = th->mmu.tlb[i].nFrame; // hit = immediate return
			th->mmu.tlb[i].frequency = 7;   // reset to max

			return nFrame | offset;
		}

	// Miss. Consult memory
	nFrame = get_frame(th->proc, nPag);
	
	// Update TLB
	int idx = find_tlb_slot(th);
	update_tlb(th, idx, nPag, nFrame);

	return nFrame | offset;
}


int find_tlb_slot(thread_t * th){
	int i;
	int lowest_freq_idx = 0; // start from 0
	int lowest_freq = th->mmu.tlb[0].frequency;

	// 1. Check for any invalid entries
	for (i = 0; i < TLB_CAPACITY; i++){
		if (!th->mmu.tlb[i].valid)  return i;
	}

	// 2. Check for least frequently used
	for (i = 0; i < TLB_CAPACITY; i++){
		if (lowest_freq > th->mmu.tlb[i].frequency){ // find less frequently used in cache
			lowest_freq = i;
			lowest_freq = th->mmu.tlb[i].frequency;
		}
		if (th->mmu.tlb[i].frequency > 0)  th->mmu.tlb[i].frequency--; // lower frequencies for all
	}

	return lowest_freq_idx;
}


void update_tlb(thread_t * th, int idx, uint32_t nPag, uint32_t nFrame){
	th->mmu.tlb[idx].pid = th->proc->pid;
	th->mmu.tlb[idx].nPag = nPag;
	th->mmu.tlb[idx].nFrame = nFrame;
	th->mmu.tlb[idx].valid = 1;
	th->mmu.tlb[idx].frequency = 7; // most recently used
}