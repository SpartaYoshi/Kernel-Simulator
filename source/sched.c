#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "../include/machine.h"  // includes "global.h"
#include "../include/clock.h"
#include "../include/ansi.h"

pthread_mutex_t sched_mtx;
pthread_cond_t sched_run_cnd;
pthread_cond_t sched_exit_cnd;
thread_t* thread_selected;

process_queue_t idle_queue = {-1, -1, 0};

pcb_t* schedule();
void dispatch(thread_t* thread, pcb_t* current_pcb, pcb_t* replacement_pcb);


// Timer for scheduler
void timer_sched() {
	printf("%sInitiated:%s Timer for scheduler\n", C_BCYN, C_RESET);

	unsigned int current_tick = 0;
	
	while(!kernel_start);

	pthread_mutex_lock(&clock_mtx);
	pthread_mutex_lock(&sched_mtx);
	while (1) {		
		timers_done++;

		// Wait (force scheduler to take longer time)
		/*   Example: multiplication depending on frequency, it takes longer or shorter time */
		while (current_tick < 100*freq)	
			current_tick++;
		current_tick = 0;

		quantum_compiler(); // quantum--; for all processes + compile stack of all running processes with 0 quantum

		while(thstack.size) {
			// Pop from stack
			thread_selected = pop(thstack.sp);
			thstack.size--;

			// Run scheduler/dispatcher
			pthread_cond_signal(&sched_run_cnd);
				// Here, the scheduler takes action and the timer waits for it to finish
			pthread_cond_wait(&sched_exit_cnd, &sched_mtx); 

			pthread_cond_signal(&tickwork_cnd);
			pthread_cond_wait(&pending_cnd, &clock_mtx);	
		}
	}
}


// Scheduler/Dispatcher (main executor)
void ksched_disp() {
	printf("%sInitiated:%s Scheduler/Dispatcher\n", C_BCYN, C_RESET);

	while(!kernel_start);
	
	// Initialize idle process queue
	for(int i = 0; i < QUEUE_CAPACITY; i++)
		idle_queue.q[i] = (pcb_t*) &nullp;

	// Start scheduler-dispatcher
	while(1) {
		pthread_cond_wait(&sched_run_cnd, &sched_mtx);

		pcb_t* current_pcb = thread_selected->proc;
		pcb_t* replacement_pcb = schedule();
		if (replacement_pcb->pid != current_pcb->pid)
			dispatch(thread_selected, current_pcb, replacement_pcb);

		pthread_cond_signal(&sched_exit_cnd);
	}
}


// Dispatching function
void dispatch(thread_t* thread, pcb_t* current_pcb, pcb_t* replacement_pcb) {
	// Save context of current process and stop it
		/*
		current_pcb->context.PC = thread->PC;
		current_pcb->context.IR = "???";
		*/
	current_pcb->state = PRSTAT_IDLE;

	// Replace process
	thread->proc = replacement_pcb;

	// Load context of replacement process and run it
		/*
		thread->PC = replacement_pcb->context.PC;
		*/
	replacement_pcb->state = PRSTAT_RUNNING;
}


// Compare priorities to sort from biggest to smallest
int priocmp(const void* a, const void* b) {
	pcb_t* ap = (pcb_t*) a;
	pcb_t* bp = (pcb_t*) b;
	return -(ap->priority - bp->priority);
}


// Scheduling function
pcb_t* schedule() {
	// Sort queue by priority
	qsort(&idle_queue, QUEUE_CAPACITY, sizeof(pcb_t), *priocmp);
	return NULL;
}

