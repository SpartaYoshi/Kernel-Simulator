#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "../include/global.h"
#include "../include/structures.h"
#include "../include/clock.h"

pthread_mutex_t sched_mtx;


void timer_sched() {
	printf("Initiated thread %lu, exec: Timer for scheduler \n", pthread_self());

	unsigned int current_tick = 0;

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


void kscheduler() {
	printf("Initiated thread %lu, exec: Scheduler \n", pthread_self());

	while(1) {
		pthread_mutex_lock(&sched_mtx);
		printf(" >> Scheduler tickeado :), %d\n", runtime_tick);
		sleep(1); //temp

		pthread_mutex_unlock(&sched_mtx);
	}
}