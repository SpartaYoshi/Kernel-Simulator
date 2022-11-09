#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "../include/global.h"
#include "../include/clock.h"
#include "../include/ansi.h"

pthread_mutex_t procgen_mtx;

// Timer for process generator
void timer_procgen() {
	printf("%sInitiated:%s Timer for process generator\n", C_BCYN, C_RESET);

	unsigned int current_tick = 0;

	while(!kernel_start);

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

// Process generator
void kprocgen() {
	printf("%sInitiated:%s Process generator\n", C_BCYN, C_RESET);

	while(!kernel_start);

	while(1) {
		pthread_mutex_lock(&procgen_mtx);
		printf(" >> Generador de procesos tickeado :), %d\n", runtime_tick);
		pthread_mutex_unlock(&procgen_mtx);
	}
}