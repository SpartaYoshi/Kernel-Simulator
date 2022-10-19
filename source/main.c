#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "definitions.h"


int main(int argc, char *argv[]) {
	extern int nth;

	printf("Indicate number of threads: ");
	scanf("%d", &nth);

	//for (int i = 0; i < nth; i++)
		//pthread_create(newthread, const pthread_attr_t *restrict attr, void *(*start_routine)(void *), void *restrict arg)

}