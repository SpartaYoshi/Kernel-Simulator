#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "../include/global.h"
#include "../include/clock.h"
#include "../include/ansi.h"

pthread_mutex_t sched_mtx;

// Timer for scheduler
void timer_sched() {
	printf("%sInitiated:%s Timer for scheduler\n", C_BCYN, C_RESET);

	unsigned int current_tick = 0;
	
	while(!kernel_start);

	pthread_mutex_lock(&clock_mtx);
	while (1) {		
		timers_done++;

		while (current_tick < 100*freq)	// Example: multiplication depending on frequency, it takes longer or shorter time
			current_tick++;
		current_tick = 0;

		pthread_mutex_unlock(&sched_mtx); 
		// scheduler iteration
		pthread_mutex_lock(&sched_mtx);

		pthread_cond_signal(&tickwork_cnd);
		pthread_cond_wait(&pending_cnd, &clock_mtx);	
	}
}

// Scheduler
void kscheduler() {
	printf("%sInitiated:%s Scheduler\n", C_BCYN, C_RESET);

	while(!kernel_start);

	while(1) {
		pthread_mutex_lock(&sched_mtx);
		printf(" >> Scheduler tickeado :), %d\n", runtime_tick);
		pthread_mutex_unlock(&sched_mtx);
	}
}