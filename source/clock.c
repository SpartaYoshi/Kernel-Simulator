#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#include "../include/global.h"
#include "../include/machine.h"

// Declaration
unsigned int runtime_tick; 

// Clock
void kclock(core_t *core) {
	runtime_tick = 0;
	unsigned int freq_tick = 0;

	while(mach.is_running) {
		freq_tick++;

		if (freq_tick == freq) {
			freq_tick = 0;
			runtime_tick++;

			pthread_mutex_lock(&core->clock_mtx);
			while(core->timers_done < NTIMERS)
				pthread_cond_wait(&core->tickwork_cnd, &core->clock_mtx);

			core->timers_done = 0; // reset timer completion counter
			pthread_cond_broadcast(&core->pending_cnd);

			pthread_mutex_unlock(&core->clock_mtx);
			
		}
	}
}

