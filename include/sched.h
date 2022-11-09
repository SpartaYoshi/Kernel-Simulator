#include <pthread.h>

extern pthread_mutex_t sched_mtx;

void timer_sched();
void kscheduler();