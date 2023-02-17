#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#include "../include/commons.h"
#include "../include/ansi.h"

// Declaration
unsigned int runtime_tick; 
int timers_done = 0; // timer completion

pthread_mutex_t clock_mtx;
pthread_cond_t tickwork_cnd;
pthread_cond_t pending_cnd;

// Clock
void kclock() {
	runtime_tick = 0;
	unsigned int freq_tick = 0;

	printf("%sInitiated:%s Clock\n", C_BCYN, C_RESET);
	while(!kernel_start);

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