#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "../include/global.h"
#include "../include/structures.h"
#include "../include/clock.h"

pthread_mutex_t procgen_mtx;

void timer_procgen() {
	printf("Initiated thread %lu, exec: Timer for process generator \n", pthread_self());

	unsigned int current_tick = 0;

	pthread_mutex_lock(&clock_mtx);
	while (1) {		
		timers_done++;

		while (current_tick < 100*freq)	// Example: multiplication depending on frequency, it takes longer or shorter time
			current_tick++;
		current_tick = 0;

		pthread_mutex_unlock(&procgen_mtx); 
		// process generator iteration
		pthread_mutex_lock(&procgen_mtx);
		
		
		pthread_cond_signal(&tickwork_cnd);
		pthread_cond_wait(&pending_cnd, &clock_mtx);	
	}
}


void kprocgen() {
	printf("Initiated thread %lu, exec: Process generator \n", pthread_self());

	while(1) {
		pthread_mutex_lock(&procgen_mtx);
		printf(" >> Generador de procesos tickeado :), %d\n", runtime_tick);
		sleep(1); //temp
		pthread_mutex_unlock(&procgen_mtx);
	}
}