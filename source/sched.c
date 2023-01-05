#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

#include "../include/global.h"
#include "../include/machine.h"
#include "../include/clock.h"
#include "../include/ansi.h"
#include "../include/queue.h"
#include "../include/procgen.h"

pthread_mutex_t sched_mtx;
pthread_cond_t sched_run_cnd;
pthread_cond_t sched_exit_cnd;
thread_t* thread_selected;



pcb_t* schedule();
void dispatch(thread_t* thread, pcb_t* current_pcb, pcb_t* replacement_pcb);


// Timer for scheduler
void timer_sched() {
	printf("%sInitiated:%s Timer for scheduler\n", C_BCYN, C_RESET);

	unsigned int current_tick = 0;
	bool stacked_th = false;
	
	while(!kernel_start);
	sleep(1); // Force clock to go first

	pthread_mutex_lock(&clock_mtx);
	pthread_mutex_lock(&sched_mtx);

	while (1) {		
		timers_done++;

		// Wait (force scheduler to take longer time)
		/*   Example: multiplication depending on frequency, it takes longer or shorter time */
		while (current_tick < 100*freq)	
			current_tick++;
		current_tick = 0;

		stacked_th = false;

		// If all threads are busy, tick quantums and compile timed out processes
		if (pcbs_generated >= MAX_THREADS) {
			if (thstack.size) {
				// Pop from stack
				thread_selected = pop(thstack.sp);
				thstack.size--;
				stacked_th = true;
			}
			else quantum_compiler(); // quantum--; for all processes + compile stack of all running processes with 0 quantum
		}

		// Run scheduler/dispatcher...
		if (stacked_th || idle_queue.size) {
			pthread_cond_signal(&sched_run_cnd);
				// Here, the scheduler takes action and the timer waits for it to finish
			pthread_cond_wait(&sched_exit_cnd, &sched_mtx);
		}

		// Signal to clock
		pthread_cond_signal(&tickwork_cnd);
		pthread_cond_wait(&pending_cnd, &clock_mtx);
	}
}


// Scheduler/Dispatcher (main executor)
void ksched_disp() {
	printf("%sInitiated:%s Scheduler/Dispatcher\n", C_BCYN, C_RESET);

	while(!kernel_start);
	// Start scheduler-dispatcher
	while(1) {
		pthread_cond_wait(&sched_run_cnd, &sched_mtx);

		pcb_t* current_pcb = NULL;

		// If there are free threads, select the next free one
		if (pcbs_generated < MAX_THREADS) {
			printf("%sscheduler  %s>>   Preparing thread %d...\n", C_BBLU, C_RESET, pcbs_generated - 1);
			thread_selected = get_thread(pcbs_generated - 1);
		}

		// If all threads are busy, get the process from the thread stack previously compiled
		else {
			current_pcb = thread_selected->proc;
			printf("%sscheduler  %s>>   Found process %d timed out. Scheduling...\n", C_BBLU, C_RESET, current_pcb->pid);
		}

		pcb_t* replacement_pcb = schedule();

		switch(policy) {
			case POL_RR:
				if (replacement_pcb->pid != current_pcb->pid)
					dispatch(thread_selected, current_pcb, replacement_pcb);
			break;

			default:
				dispatch(thread_selected, current_pcb, replacement_pcb);
			break;
		}
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

	if (current_pcb != NULL) { // Only if all threads are busy
		printf("%sdispatcher %s>>   Idling process %d\n", C_BGRN, C_RESET, current_pcb->pid);
		current_pcb->state = PRSTAT_IDLE;
		enqueue(&idle_queue, current_pcb);
	}

	printf("%sdispatcher %s>>   Running process %d\n", C_BGRN, C_RESET, replacement_pcb->pid);

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

	pcb_t* out;
	switch(policy) {
		case POL_FCFS:
			out = dequeue(&idle_queue);
		case POL_RR:
			// Sort queue by priority
			qsort(&idle_queue, QUEUE_CAPACITY, sizeof(pcb_t), *priocmp);
		break;

		default:
		break;
	}
	return out;
}

