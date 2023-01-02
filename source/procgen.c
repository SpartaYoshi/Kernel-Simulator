#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "../include/global.h"
#include "../include/clock.h"
#include "../include/ansi.h"
#include "../include/queue.h"


pthread_mutex_t procgen_mtx;
pthread_cond_t procgen_run_cnd;
pthread_cond_t procgen_exit_cnd;

pcb_t* head = NULL;
int generated = 0;


// Timer for process generator
void timer_procgen() {
	printf("%sInitiated:%s Timer for process generator\n", C_BCYN, C_RESET);

	unsigned int current_tick = 0;

	while(!kernel_start);

	pthread_mutex_lock(&clock_mtx);
	pthread_mutex_lock(&procgen_mtx);
	while (1) {		
		timers_done++;

		while (current_tick < 10000*freq)	// Example: multiplication depending on frequency, it takes longer or shorter time
			current_tick++;
		current_tick = 0;

		// Here, the process generator takes action and the timer waits for it to finish
		pthread_cond_signal(&procgen_run_cnd);
		pthread_cond_wait(&procgen_exit_cnd, &procgen_mtx); 

		
		pthread_cond_signal(&tickwork_cnd);
		pthread_cond_wait(&pending_cnd, &clock_mtx);
	}
}

// Process generator
void kprocgen() {
	printf("%sInitiated:%s Process generator\n", C_BCYN, C_RESET);

	while(!kernel_start);
	init_queue(&idle_queue);

	while(1) {
		pthread_cond_wait(&procgen_run_cnd, &procgen_mtx);
		
		if (generated < MAX_THREADS) {
			// Create process block
			pcb_t* block = malloc(sizeof(pcb_t));
			block->pid = ++generated;
			block->state = PRSTAT_IDLE;
			block->priority = 20;
			block->context.PC = 0; // TODO: TBA
			block->quantum = 40;

			// Link to dynamic list of processes
			block->next = head;
			head = block;

			// Add to idle queues
			enqueue(&idle_queue, block);
		}
		
		pthread_cond_signal(&procgen_exit_cnd);
	}
}