#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "../include/structures.h"
#include "../include/global.h"
#include "../include/clock.h"

extern pthread_mutex_t sched_mtx;
unsigned int current_tick = 0;


void timer_sched() {
	printf("Initiated thread %lu, exec: Timer for scheduler \n", pthread_self());

	while (1) {		
		while (current_tick < clock_tick + 1);

		pthread_mutex_lock(&clock_mtx);
		pthread_mutex_unlock(&sched_mtx); // cosas del sched
		current_tick++;
		pthread_mutex_lock(&sched_mtx);
		pthread_mutex_unlock(&clock_mtx);
		
	}
}


void kscheduler() {
	printf("Initiated thread %lu, exec: Scheduler \n", pthread_self());

	while(1) {
		pthread_mutex_lock(&sched_mtx);
		printf(" >> Scheduler tickeado :), %d\n", current_tick);
		pthread_mutex_unlock(&sched_mtx);
	}
}