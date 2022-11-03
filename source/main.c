#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "../include/global.h"
#include "../include/structures.h"
#include "../include/clock.h"
#include "../include/sched.h"
#include "../include/procgen.h"


// Declaration
void *thread_exec();
void onexit();

unsigned int nth;
unsigned int ncores;
unsigned int ncpu;
unsigned int freq;	// clock tick by hardware frequency

pthread_t tid;
pthread_mutex_t init_mtx;
pthread_cond_t tickwork_cnd;
pthread_cond_t pending_cnd;


// MAIN
int main(int argc, char *argv[]) {

	// Parameter assigning
	printf("Indicate number of CPUs: ");
	scanf("%d", &ncpu);
	printf("Indicate number of cores (per CPU): ");
	scanf("%d", &ncores);
	printf("Indicate number of threads (per core): ");
	scanf("%d", &nth);
	printf("Indicate frequency (cycles per tick): ");
	scanf("%d", &freq);


	// Mutex initialization
	pthread_mutex_init(&init_mtx, NULL);
	pthread_mutex_init(&clock_mtx, NULL);
	pthread_mutex_init(&sched_mtx, NULL);
	pthread_mutex_init(&procgen_mtx, NULL);
	pthread_cond_init(&tickwork_cnd, NULL);
	pthread_cond_init(&pending_cnd, NULL);
	


	// Thread creation and role assignation + memory allocation
	machine_t mach = { (cpu_t*) malloc(sizeof(cpu_t) * ncpu) };
	tid = 0;

	for (int i = 0; i < ncpu; i++) {
		cpu_t icpu = { (core_t*) malloc(sizeof(core_t) * ncores) } ;

		for (int j = 0; j < ncores; j++) {
			core_t jcore = { (pthread_t*) malloc(sizeof(pthread_t) * nth) } ;
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

	onexit();
	exit(0);

}

void inc_tid() {
	pthread_mutex_lock(&init_mtx);
	tid++;
	pthread_mutex_unlock(&init_mtx);
}


void *thread_exec() {
	printf("Started thread %lu, pid: %lu \n", tid, pthread_self());
	inc_tid();
	switch (tid-1) {
		case 0:
			kclock();
			break;
		case 1:
			timer_sched();
			break;
		case 2:
			timer_procgen();
			break;
		case 3:
			kscheduler();
			break;
		case 4:
			kprocgen();
			break;
		default:
			break;
	}
	return 0;
}

void onexit() {
	// hacer free
	pthread_mutex_destroy(&clock_mtx);
	pthread_mutex_destroy(&sched_mtx);
	pthread_mutex_destroy(&init_mtx);
	pthread_exit(0);
}