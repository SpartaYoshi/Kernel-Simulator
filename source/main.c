#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "../include/clock.h"
#include "../include/global.h"
#include "../include/structures.h"

void *thread_exec();
void onexit();


int main(int argc, char *argv[]) {
	
	machine_t mach = { NULL };

	// Parameter assigning
	printf("Indicate number of CPUs: ");
	scanf("%d", &ncpu);
	printf("Indicate number of cores (per CPU): ");
	scanf("%d", &ncores);
	printf("Indicate number of threads (per core): ");
	scanf("%d", &nth);


	// Mutex initialization
	pthread_mutex_init(&clock_mtx, NULL);
	pthread_mutex_init(&sched_mtx, NULL);


	// Thread creation and role assignation
	for (int i = 0; i < ncpu; i++) {
		cpu_t icpu = { NULL };
		for (int j = 0; j < ncores; j++) {
			core_t jcore = { NULL };
			for (int k = 0; k < nth; k++) {
				if (pthread_create(&jcore.threads[k], NULL, thread_exec, NULL) != 0){
					perror("ERROR: Failed to create thread.\n");
					exit(1);
				}
			}
			icpu.cores[j] = jcore;
		}
		mach.cpus[i] = icpu;
		
	}

	while(1); //temp
	exit(0);

}


void *thread_exec() {
	int tid = pthread_self();
	switch (tid) {
		case 0:
			kclock();
			break;
		case 1:
			timer_sched();
			break;
		case 2:
			//timer_procgen();
			break;
		case 3:
			kscheduler();
			break;
		default:
			break;

	}

	pthread_exit(0);
}

void onexit() {
	pthread_mutex_destroy(&clock_mtx);

}