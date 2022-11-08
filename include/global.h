#include <pthread.h>

#define NTIMERS 2
#define MAX_THREADS 32

#define MACH_OFF 0
#define MACH_ON 1


// Parameters
extern unsigned int ncores;
extern unsigned int ncpu;

extern unsigned int freq;	// clock tick by hardware frequency

extern int thread_count;