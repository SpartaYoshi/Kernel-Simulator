#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#include "../include/ansi.h"
#include "../include/clock.h"
#include "../include/commons.h"
#include "../include/loader.h"
#include "../include/memory.h"
#include "../include/queue.h"

pthread_mutex_t loader_mtx;
pthread_cond_t loader_run_cnd;
pthread_cond_t loader_exit_cnd;

pcb_t* head_l;
int pcbs_generated_l = 0;
int elfi = 0;


// Timer for loader
void timer_loader() {
	printf("%sInitiated:%s Timer for loader\n", C_BCYN, C_RESET);

	unsigned int current_tick = 0;

	while(!kernel_start);

	init_queue(&idle_queue);

	pthread_mutex_lock(&clock_mtx);
	pthread_mutex_lock(&loader_mtx);

	while (1) {		
		timers_done++;
		while (current_tick < 1000*freq)	// Example: multiplication depending on frequency, \
											   it takes longer or shorter time
			current_tick++;
		current_tick = 0;

		// Run loader...
		pthread_cond_signal(&loader_run_cnd);
			// Here, the loader takes action and the timer waits for it to finish
		pthread_cond_wait(&loader_exit_cnd, &loader_mtx); 
		
		// Signal to clock
		pthread_cond_signal(&tickwork_cnd);
		pthread_cond_wait(&pending_cnd, &clock_mtx);
	}
}


// Loader
void kloader() {
	int _;
	printf("%sInitiated:%s Loader\n", C_BCYN, C_RESET);

	while(!kernel_start);

	while(1) {
		pthread_cond_wait(&loader_run_cnd, &loader_mtx);
		
		if (pcbs_generated_l < (nth * ncores * ncpu) + QUEUE_CAPACITY && \
		    idle_queue.size < QUEUE_CAPACITY) {
			printf("%sloader    %s>>   Generating process %d...\n",\
				 C_BYEL, C_RESET, pcbs_generated_l + 1);

			// Create process block
			pcb_t* block = (pcb_t *) malloc(sizeof(pcb_t));
			block->pid = ++pcbs_generated_l;
			block->state = PRSTAT_IDLE;
			block->priority = 20;
			block->quantum = QUANTUM_DEFAULT;
			block->context = (pcb_context_t *) malloc(sizeof(pcb_context_t));

			// Link to dynamic list of processes
			block->next = head_l;
			head_l = block;

			// Add to idle queues
			enqueue(&idle_queue, block);

			// Reserve page table in memory
			block->mm.pgb = create_page_table();
			
			// Load program into memory
			sprintf(block->context->prog_name, "./programs/elfs/prog%03d.elf", elfi);
			_ = boot_elf(block);
			elfi = (elfi + 1) % NPROGRAMS; 

			if (_ == 0)
				printf("%sloader    %s>>   Process %d successfully generated. Located at %p\n",
					C_BYEL, C_RESET, block->pid, block);
		}
		
		pthread_cond_signal(&loader_exit_cnd);
	}
}


int boot_elf(pcb_t *proc){
	FILE *fp;
	char line[16];
	int _;
	uint32_t dword;

	#define HEADER 12*2+2
	#define CHAR_PER_LINE 8+1

	// Open file	
	fp = fopen(proc->context->prog_name, "r");
	if (fp == NULL) {
		printf("%sloader    %s>>   %sERROR: %sFile could not be opened. \n",
			C_BYEL, C_RESET, C_BRED, C_RESET);
		return 1;
	}
	// TODO RELLENAR PAGE TABLE


	// Get virtual addresses for code
	_ = fscanf(fp, "%s %X", line, &proc->mm.code);
	if (_ != 2) {
		printf("%sloader    %s>>   %sERROR: %sFile could not be read: Wrong format. \n",
			C_BYEL, C_RESET, C_BRED, C_RESET);
		fclose(fp);
		return 1;
	}
	// TODO RELLENAR PAGE TABLE

	// Get virtual addresses for data
	_ = fscanf(fp, "%s %X", line, &proc->mm.data);
	if (_ != 2) {
		printf("%sloader    %s>>   %sERROR: %sFile could not be read: Wrong format. \n",
			C_BYEL, C_RESET, C_BRED, C_RESET);
		fclose(fp);
		return 1;
	}	

	// Copy to memory
	while (!feof(fp)){
		_ = fscanf(fp, "%X", &physical[nfi++]);
		if (_ != 2) {
			printf("%sloader    %s>>   %sERROR: %sFile could not be read: Wrong format. \n",
				C_BYEL, C_RESET, C_BRED, C_RESET);
			fclose(fp);
			return 1;
		}

		proc->mm.mem_length++;
	}
	fclose(fp);

	return 0;
}