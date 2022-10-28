#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "../include/structures.h"
#include "../include/global.h"

extern unsigned int clock_tick;
extern pthread_mutex_t clock_mtx;

void kclock() {
	clock_tick = 0;
	unsigned int freq_tick = 0;

	printf("Initiated thread %lu, exec: Clock \n", pthread_self());

	while(1) {
		freq_tick++;

		if (freq_tick == freq) {
			freq_tick = 0;
			pthread_mutex_lock(&clock_mtx);
			clock_tick++;
			pthread_mutex_unlock(&clock_mtx);
			
		}
	}
	
}

