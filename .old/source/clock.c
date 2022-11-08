#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "../include/global.h"
#include "../include/structures.h"

// Declaration
unsigned int clock_tick;
unsigned int runtime_tick; // info sobrante
pthread_mutex_t clock_mtx;
int timers_done = 0; // timer completion

// Clock
void kclock() {
	runtime_tick = 0;
	unsigned int freq_tick = 0;

	printf("Initiated thread %lu, exec: Clock \n", pthread_self());

	while(1) {
		freq_tick++;

		if (freq_tick == freq) {
			freq_tick = 0;
			runtime_tick++;

			pthread_mutex_lock(&clock_mtx);
			while(timers_done < NTIMERS)
				pthread_cond_wait(&tickwork_cnd, &clock_mtx);

			timers_done = 0; // reset timer completion counter
			pthread_cond_broadcast(&pending_cnd);

			pthread_mutex_unlock(&clock_mtx);
			
		}
	}
	
}

