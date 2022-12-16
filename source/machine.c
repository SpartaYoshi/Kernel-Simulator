#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#include "../include/global.h"
#include "../include/clock.h"
#include "../include/sched.h"
#include "../include/procgen.h"
#include "../include/ansi.h"

// Declaration
machine_t      mach;
thread_stack_t thstack;

void init_core_thread(core_t* core, thread_t* thread, void* start_routine, char* proc_name);

/////////////////////////

void init_machine() {
	printf("%sInitiated:%s Machine (%d %d-core CPUs)\n", C_BCYN, C_RESET, ncpu, ncores);

	mach = (machine_t) { (cpu_t*) malloc(sizeof(cpu_t) * ncpu), MACH_OFF };

	for (int i = 0; i < ncpu; i++) {
		cpu_t icpu = { (core_t*) malloc(sizeof(core_t) * ncores) } ;

		for (int j = 0; j < ncores; j++) {
			printf("Initializing core %d of cpu %d... \n", j, i);
			core_t jcore;

			jcore.cid = j;
			jcore.thread_count = 0;

			icpu.core[j] = jcore;
		}
		mach.cpu[i] = icpu;
	}
	mach.is_running = MACH_ON;
	
	// Init stack pointer
	thstack.sp = thstack.stack;
	thstack.size = 0;
}

void shutdown_machine() {
	printf("\nShutting down machine...\n");

	for (int i = 0; i < ncpu; i++) {
		for (int j = 0; j < ncores; j++){
			core_t* jcore = &mach.cpu[i].core[j];

			for (int k = 0; k < jcore->thread_count; k++) {
				pthread_join(jcore->thread[k].handle, NULL);
				free(jcore->thread[k].proc);
			}
		}
		free(mach.cpu[i].core);
	}
	free(mach.cpu);
	mach.is_running = MACH_OFF;
}



void init_core_thread(core_t* core, thread_t* thread, void* start_routine, char* proc_name) {
	thread->proc = malloc(sizeof(pcb_t));
	strcpy(thread->proc_name,proc_name);

	pthread_create(&thread->handle, NULL, start_routine, (void *) &core);
	
	printf("Initiated thread %d: %s \n", core->thread_count, proc_name);
	core->thread_count++;

}


thread_t* find_thread(core_t* core, char* proc_name) {
	thread_t* thread = NULL;
	for (int i = 0; i < MAX_THREADS; i++)
		if (strcmp(core->thread[i].proc_name, proc_name) == 0){
			thread = &core->thread[i];
			break;
		}
	return thread;
}



// Quantum subtraction for all processes + compile processed threads into stack
void quantum_compiler() {
	for (int i = 0; i < ncpu; i++) {
		for (int j = 0; j < ncores; j++){
			core_t* jcore = &mach.cpu[i].core[j];
			for (int k = 0; k < jcore->thread_count; k++) {
				pcb_t* process_block = jcore->thread[k].proc;
				if (process_block->state == PRSTAT_RUNNING && process_block->quantum > 0)
					process_block->quantum--;
				if(!process_block->quantum) {
					push(*thstack.sp, jcore->thread[k]);
					thstack.size++;
				}
			}
		}
	}
}
