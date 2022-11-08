#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#include "../include/global.h"
#include "../include/clock.h"
#include "../include/machine.h"


void timer_sched(core_t *core) {
	unsigned int current_tick = 0;

	pthread_mutex_lock(&core->clock_mtx);
	while (mach.is_running) {		
		core->timers_done++;

		while (current_tick < 100*freq)	// Example: multiplication depending on frequency, it takes longer or shorter time
			current_tick++;
		current_tick = 0;

		pthread_mutex_unlock(&core->sched_mtx); 
		// scheduler iteration
		pthread_mutex_lock(&core->sched_mtx);

		pthread_cond_signal(&core->tickwork_cnd);
		pthread_cond_wait(&core->pending_cnd, &core->clock_mtx);	
	}
}


void kscheduler(core_t *core) {

	while(mach.is_running) {
		pthread_mutex_lock(&core->sched_mtx);
		printf(" >> Scheduler del core %d tickeado :), %u\n", core->cid, runtime_tick);
		pthread_mutex_unlock(&core->sched_mtx);
	}
}