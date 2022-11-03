#include <pthread.h>

#define NTIMERS 2
// Parameters
extern unsigned int nth;
extern unsigned int ncores;
extern unsigned int ncpu;

extern unsigned int freq;	// clock tick by hardware frequency

extern pthread_mutex_t clock_mtx;
extern pthread_mutex_t sched_mtx;
extern pthread_mutex_t init_mtx;
extern pthread_mutex_t procgen_mtx;

extern pthread_cond_t tickwork_cnd;
extern pthread_cond_t pending_cnd;
extern pthread_t tid;

extern int timers_done;
