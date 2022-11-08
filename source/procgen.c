#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "../include/global.h"
#include "../include/machine.h"
#include "../include/clock.h"

pthread_mutex_t procgen_mtx;

void timer_procgen(core_t *core) {
	unsigned int current_tick = 0;

	pthread_mutex_lock(&core->clock_mtx);
	while (mach.is_running) {		
		core->timers_done++;

		while (current_tick < 100*freq)	// Example: multiplication depending on frequency, it takes longer or shorter time
			current_tick++;
		current_tick = 0;

		pthread_mutex_unlock(&core->procgen_mtx); 
		// scheduler iteration
		pthread_mutex_lock(&core->procgen_mtx);

		pthread_cond_signal(&core->tickwork_cnd);
		pthread_cond_wait(&core->pending_cnd, &core->clock_mtx);	
	}
}


void kprocgen(core_t *core) {

	while(mach.is_running) {
		pthread_mutex_lock(&procgen_mtx);
		printf(" >> Generador de procesos del core %d tickeado :), %u\n", core->cid, runtime_tick);
		pthread_mutex_unlock(&procgen_mtx);
	}
}