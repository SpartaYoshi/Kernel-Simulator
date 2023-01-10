#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "../include/global.h"
#include "../include/ansi.h"

dword* pmem;
size_t pmem_size;


void init_memory() {
	pmem_size = pow(2, MEMSIZE_BITS);
	pmem = malloc(sizeof(dword) * pmem_size); // long = 4 bytes = word size
}