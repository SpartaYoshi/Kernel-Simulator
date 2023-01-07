#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>

#include "../include/global.h"
#include "../include/clock.h"
#include "../include/sched.h"
#include "../include/procgen.h"
#include "../include/machine.h"
#include "../include/ansi.h"

#define KEY_ESC 0x001b
#define KEY_SPC 0x0020

// Declaration
unsigned int ncores;
unsigned int ncpu;
unsigned int nth;
unsigned int freq;	// clock tick by hardware frequency
int kernel_start = 0;
int policy;

// Non-canonical terminal mode
static struct termios term, oterm;

static int getch(void) {
    int c = 0;

    tcgetattr(0, &oterm);
    memcpy(&term, &oterm, sizeof(term));
    term.c_lflag &= ~(ICANON | ECHO);
    term.c_cc[VMIN] = 1;
    term.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &term);
    c = getchar();
    tcsetattr(0, TCSANOW, &oterm);
    return c;
}


/////////////////////////////////

// MAIN
int main(int argc, char *argv[]) {

	// Parameter assigning
	printf("%sIndicate number of CPUs. %s(MAX = %d): %s", C_BYEL, C_YEL, MAX_CPUS, C_RESET);
	scanf("%d", &ncpu);
	while (ncpu < 1 || ncpu > MAX_CPUS) {
		printf("%sERROR: %sInvalid configuration. %sPlease try another value: %s", C_BRED, C_RESET, C_BWHT, C_RESET);
		scanf("%d", &ncpu);
	}

	printf("%sIndicate number of cores (per CPU). %s(MAX = %d): %s", C_BYEL, C_YEL, MAX_CORES, C_RESET);
	scanf("%d", &ncores);
	while (ncores < 1 || ncores > MAX_CORES) {
		printf("%sERROR: %sInvalid configuration. %sPlease try another value: %s", C_BRED, C_RESET, C_BWHT, C_RESET);
		scanf("%d", &ncores);
	}

	printf("%sIndicate number of active threads (per core) %s(MAX = %d): %s", C_BYEL, C_YEL, MAX_THREADS, C_RESET);
	scanf("%d", &nth);
	while (nth < 1 || nth > MAX_THREADS) {
		printf("%sERROR: %sInvalid configuration. %sPlease try another value: %s", C_BRED, C_RESET, C_BWHT, C_RESET);
		scanf("%d", &nth);
	}

	printf("%sIndicate frequency (cycles per tick): %s", C_BYEL, C_RESET);
	scanf("%d", &freq);
	while (freq < 1) {
		printf("%sERROR: %sInvalid configuration. %sPlease try another value: %s", C_BRED, C_RESET, C_BWHT, C_RESET);
		scanf("%d", &freq);
	}

	printf("%sIndicate scheduling policy. %s(0 = FCFS, 1 = Round Robin): %s", C_BYEL, C_YEL, C_RESET);
	scanf("%d", &policy);
	while (policy < POL_FCFS || policy > POL_RR) {
		printf("%sERROR: %sInvalid configuration. %sPlease try another value: %s", C_BRED, C_RESET, C_BWHT, C_RESET);
		scanf("%d", &policy);
	}


	// Mutex and conditionals initialization
	pthread_mutex_init(&clock_mtx, NULL);
	pthread_mutex_init(&sched_mtx, NULL);
	pthread_mutex_init(&procgen_mtx, NULL);
	pthread_cond_init(&tickwork_cnd, NULL);
	pthread_cond_init(&pending_cnd, NULL);
	pthread_cond_init(&sched_run_cnd, NULL);
	pthread_cond_init(&sched_exit_cnd, NULL);
	pthread_cond_init(&procgen_run_cnd, NULL);
	pthread_cond_init(&procgen_exit_cnd, NULL);


	// Hardware initialization
	pthread_t clock_o;
	pthread_create(&clock_o, NULL, (void *) kclock, NULL);
	pthread_t timer_sched_o;
	pthread_create(&timer_sched_o, NULL, (void *) timer_sched, NULL);
	pthread_t timer_procgen_o;
	pthread_create(&timer_procgen_o, NULL, (void *) timer_procgen, NULL);
	pthread_t sched_o;
	pthread_create(&sched_o, NULL, (void *) ksched_disp, NULL);
	pthread_t procgen_o;
	pthread_create(&procgen_o, NULL, (void *) kprocgen, NULL);

	init_machine();

	// Kernel simulation.
	sleep(2); // To force print after thread init
	printf("\n%sThe kernel has been successfully initialized. Press %sSPACE%s to start!%s\n", C_BBLU, C_BYEL, C_BBLU, C_RESET);
	while (getch() != KEY_SPC);

	kernel_start = 1;
	printf("\n%sStarting...%s\n", C_BHRED, C_RESET);

	while (1) {
		fflush(stdout);
	};
	
	
	/**
	while (1) {
		
		char c = getch();
		if (c == 'q' || c == KEY_ESC) 
			break;
	}
	

	// Hardware shutdown
	shutdown_machine();
	pthread_join(clock_o, NULL);
	pthread_join(timer_sched_o, NULL);
	pthread_join(timer_procgen_o, NULL);
	pthread_join(sched_o, NULL);
	pthread_join(procgen_o, NULL);


	// Mutex and conditionals cleanup
	pthread_mutex_destroy(&clock_mtx);
	pthread_mutex_destroy(&sched_mtx);
	pthread_mutex_destroy(&procgen_mtx);
	pthread_cond_destroy(&tickwork_cnd);
	pthread_cond_destroy(&pending_cnd);
	pthread_cond_destroy(&sched_run_cnd);
	pthread_cond_destroy(&sched_exit_cnd);
	pthread_cond_destroy(&procgen_run_cnd);
	pthread_cond_destroy(&procgen_exit_cnd);

	**/
	exit(0);
}
