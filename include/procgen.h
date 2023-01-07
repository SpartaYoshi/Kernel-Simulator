#include <pthread.h>
#include "../include/global.h"

extern pthread_mutex_t procgen_mtx;
extern pthread_cond_t procgen_run_cnd;
extern pthread_cond_t procgen_exit_cnd;
extern int pcbs_generated;

void timer_procgen();
void kprocgen();