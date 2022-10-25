#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

/**
#include "../include/definitions.h"
**/


int main(int argc, char *argv[]) {
	int nth = 0;

	printf("Indicate number of threads: ");
	if(scanf("%d", &nth) == 1){};

	//for (int i = 0; i < nth; i++)
		//pthread_create(newthread, const pthread_attr_t *restrict attr, void *(*start_routine)(void *), void *restrict arg)

	exit(0);

}