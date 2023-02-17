#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "../include/ansi.h"
#include "../include/clock.h"
#include "../include/commons.h"
#include "../include/memory.h"
#include "../include/queue.h"


pthread_mutex_t loader_mtx;
pthread_cond_t loader_run_cnd;
pthread_cond_t loader_exit_cnd;


pcb_t* head_l;
int pcbs_generated_l = 0;


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

			printf("%sloader    %s>>   Process %d successfully generated. Located at %p\n",\
				 C_BYEL, C_RESET, block->pid, block);
		}
		
		pthread_cond_signal(&loader_exit_cnd);
	}
}
