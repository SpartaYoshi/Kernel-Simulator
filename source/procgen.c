#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "../include/commons.h"
#include "../include/clock.h"
#include "../include/ansi.h"
#include "../include/queue.h"


pthread_mutex_t procgen_mtx;
pthread_cond_t procgen_run_cnd;
pthread_cond_t procgen_exit_cnd;

pcb_t* head;
int pcbs_generated = 0;


// Timer for process generator
void timer_procgen() {
	printf("%sInitiated:%s Timer for process generator\n", C_BCYN, C_RESET);

	unsigned int current_tick = 0;

	while(!kernel_start);

	init_queue(&idle_queue);

	pthread_mutex_lock(&clock_mtx);
	pthread_mutex_lock(&procgen_mtx);

	while (1) {		
		timers_done++;
		while (current_tick < 1000*freq)	// Example: multiplication depending on frequency,\
											   it takes longer or shorter time
			current_tick++;
		current_tick = 0;

		// Run process generator...
		pthread_cond_signal(&procgen_run_cnd);
			// Here, the process generator takes action and the timer waits for it to finish
		pthread_cond_wait(&procgen_exit_cnd, &procgen_mtx); 
		
		// Signal to clock
		pthread_cond_signal(&tickwork_cnd);
		pthread_cond_wait(&pending_cnd, &clock_mtx);
	}
}

// Process generator
void kprocgen() {
	printf("%sInitiated:%s Process generator\n", C_BCYN, C_RESET);

	while(!kernel_start);

	while(1) {
		pthread_cond_wait(&procgen_run_cnd, &procgen_mtx);
		
		if (pcbs_generated < (nth * ncores * ncpu) + QUEUE_CAPACITY && \
		    idle_queue.size < QUEUE_CAPACITY) {
			printf("%sprocgen    %s>>   Generating process %d...\n",\
				 C_BYEL, C_RESET, pcbs_generated + 1);

			// Create process block
			pcb_t* block = malloc(sizeof(pcb_t));
			block->pid = ++pcbs_generated;
			block->state = PRSTAT_IDLE;
			srand(time(NULL));
			block->priority = rand() % 139;
			block->quantum = rand() % 15 + QUANTUM_MIN;
			block->context = (pcb_context_t *) malloc(sizeof(pcb_context_t));


			// Link to dynamic list of processes
			block->next = head;
			head = block;

			// Add to idle queues
			enqueue(&idle_queue, block);

			printf("%sprocgen    %s>>   Process %d successfully generated.\
				 Located at %p\n",\
				 C_BYEL, C_RESET, block->pid, block);
		}
		
		pthread_cond_signal(&procgen_exit_cnd);
	}
}