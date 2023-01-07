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
int tid = 0;

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
		if (tid >= (nth * ncores * ncpu)) {
			if (!thstack.size)
				quantum_compiler(); // compile stack of all running processes with 0 quantum
			
			// If after compiling there are threads to check...
			if (thstack.size > 0) {
				// Pop from stack
				thread_selected = pop(thstack.sp);
				thstack.size--;
				stacked_th = true;
			}

			// If not, clear thread selection to cancel scheduling
			else thread_selected = NULL;
		}

		// Run scheduler/dispatcher...
		if (stacked_th || idle_queue.size) {
			pthread_cond_signal(&sched_run_cnd);
				// Here, the scheduler takes action and the timer waits for it to finish
			pthread_cond_wait(&sched_exit_cnd, &sched_mtx);
		}

		// Subtract quantum to running processes
		/* if (pcbs_generated > 0)   // avoid first unneccessary iteration */
		subtract_quantum();

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
		if (tid < (nth * ncores * ncpu)) {
			printf("%sscheduler  %s>>   Preparing thread %d...\n", C_BBLU, C_RESET, tid);
			thread_selected = get_thread(tid);
			// NEW_PROCESS = schedule()
			tid++;
		}

		// If all threads are busy and the stack has found a thread to switch context
		else if (thread_selected) {
			current_pcb = thread_selected->proc;
			// REPLACEMENT = schedule()
			printf("%sscheduler  %s>>   Process %d has timed out in thread %d. Located at %p\n", C_BBLU, C_RESET, current_pcb->pid, thread_selected->global_tid, current_pcb);
		}

		// Else then compilation found no threads available. Cancel scheduling.
		else {
			printf("%sscheduler  %s>>   No threads are available for scheduling at this moment.\n", C_BBLU, C_RESET);
			pthread_cond_signal(&sched_exit_cnd);
			continue;
		}

		printf("%sscheduler  %s>>   Scheduling...\n", C_BBLU, C_RESET);
		pcb_t* replacement_pcb = schedule();
		printf("%sscheduler  %s>>   Scheduled for process %d. Located at %p\n", C_BBLU, C_RESET, replacement_pcb->pid, replacement_pcb);
		

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
		printf("%sdispatcher %s>>   Switching context for thread %d...\n", C_BGRN, C_RESET, thread->global_tid);
		printf("%sdispatcher %s>>   Idling process %d. Located at %p\n", C_BGRN, C_RESET, current_pcb->pid, current_pcb);
		current_pcb->state = PRSTAT_IDLE;
		enqueue(&idle_queue, current_pcb);
	}

	printf("%sdispatcher %s>>   Running process %d. Located at %p\n", C_BGRN, C_RESET, replacement_pcb->pid, replacement_pcb);

	// Replace pointer to process
	thread->proc = replacement_pcb;

	// Load context of replacement process and run it
		/*
		thread->PC = replacement_pcb->context.PC;
		*/

	// Update properties
	replacement_pcb->state = PRSTAT_RUNNING;
	replacement_pcb->quantum = QUANTUM_DEFAULT;
	if (replacement_pcb->priority < 140)
		replacement_pcb->priority++;
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
		break;

		case POL_RR:
			// Sort queue by priority
			qsort(&idle_queue, QUEUE_CAPACITY, sizeof(pcb_t), *priocmp);
		break;

		default:
		break;
	}

	return out;
}

