#include <pthread.h>

extern unsigned int clock_tick;
extern pthread_mutex_t clock_mtx;

void kclock();
