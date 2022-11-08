#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#include "../include/structures.h" //includes global
#include "../include/clock.h"
#include "../include/sched.h"
#include "../include/procgen.h"

// Declaration
machine_t mach;

void init_thread(core_t *core, thread_t *thread, void *start_routine, char *proc_name);


/////////////////////////

void init_machine() {
	mach = (machine_t) { (cpu_t*) malloc(sizeof(cpu_t) * ncpu), MACH_ON };

	for (int i = 0; i < ncpu; i++) {
		cpu_t icpu = { (core_t*) malloc(sizeof(core_t) * ncores) } ;


		// Init cores
		for (int j = 0; j < ncores; j++) {
			printf("Initializing core %d... \n", j);
			core_t jcore;

			jcore.cid = j;
			jcore.timers_done = 0;
			jcore.thread_count = 0;

			// Mutex initialization
			pthread_mutex_init(&jcore.clock_mtx, NULL);
			pthread_mutex_init(&jcore.sched_mtx, NULL);
			pthread_mutex_init(&jcore.procgen_mtx, NULL);
			pthread_cond_init(&jcore.tickwork_cnd, NULL);
			pthread_cond_init(&jcore.pending_cnd, NULL);

			// Thread 0 = clock
			init_thread(&jcore, &jcore.thread[0], (void *) kclock, "CLOCK");

			// Thread 1 = timer for scheduler
			init_thread(&jcore, &jcore.thread[1], (void *) timer_sched, "TIMER_SCHEDULER");

			// Thread 2 = timer for process generator
			init_thread(&jcore, &jcore.thread[2], (void *) timer_procgen, "TIMER_PROCESS_GENERATOR");

			// Thread 3 = scheduler
			init_thread(&jcore, &jcore.thread[3], (void *) kscheduler, "SCHEDULER");

			// Thread 4 = process generator
			init_thread(&jcore, &jcore.thread[3], (void *) kprocgen, "PROCESS_GENERATOR");

			icpu.core[j] = jcore;
		}
		mach.cpu[i] = icpu;
		
	}

}

void shutdown_machine() {
	for (int i = 0; i < ncpu; i++) {
		for (int j = 0; j < ncores; j++){
			core_t *jcore = &mach.cpu[i].core[j];

			pthread_mutex_destroy(&jcore->clock_mtx);
			pthread_mutex_destroy(&jcore->sched_mtx);
			pthread_mutex_destroy(&jcore->procgen_mtx);
			pthread_cond_destroy(&jcore->tickwork_cnd);
			pthread_cond_destroy(&jcore->pending_cnd);

			for (int k = 0; k < jcore->thread_count; k++) {
				pthread_join(jcore->thread[k].handle, NULL);
				free(jcore->thread[k].proc);
			}
		}
		free(mach.cpu[i].core);
	}
	free(mach.cpu);

	exit(0);
}

void init_thread(core_t *core, thread_t *thread, void *start_routine, char *proc_name) {
	thread->proc = malloc(sizeof(pcb_t));
	strcpy(thread->proc_name,proc_name);

	sleep(1); // Enough time to view init
	pthread_create(&thread->handle, NULL, (void *) start_routine, (void *) &core);
	printf("Initiated thread %d: %s \n", core->thread_count, proc_name);
	core->thread_count++;
}

thread_t *find_thread(core_t *core, char *proc_name) {
	thread_t *thread = NULL;
	for (int i = 0; i < MAX_THREADS; i++)
		if (strcmp(core->thread[i].proc_name, proc_name) == 0){
			thread = &core->thread[i];
			break;
		}
	return thread;
}