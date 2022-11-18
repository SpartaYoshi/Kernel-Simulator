#include <pthread.h>

extern pthread_mutex_t sched_mtx;
extern pthread_cond_t sched_run_cnd;
extern pthread_cond_t sched_exit_cnd;

void timer_sched();
void ksched_disp();