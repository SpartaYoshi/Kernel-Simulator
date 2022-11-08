#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "../include/global.h"
#include "../include/clock.h"
#include "../include/sched.h"
#include "../include/procgen.h"
#include "../include/machine.h" // includes "structures.h"


// Declaration
unsigned int ncores;
unsigned int ncpu;
unsigned int freq;	// clock tick by hardware frequency

// MAIN
int main(int argc, char *argv[]) {

	// Parameter assigning
	printf("Indicate number of CPUs: ");
	scanf("%d", &ncpu);
	printf("Indicate number of cores (per CPU): ");
	scanf("%d", &ncores);
	printf("Indicate frequency (cycles per tick): ");
	scanf("%d", &freq);



	init_machine();
	while (mach.is_running != 0);
	shutdown_machine();

	exit(0);

}
