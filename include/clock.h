#include <pthread.h>

extern int NTIMERS;
extern int timers_done;

extern unsigned int runtime_tick;

extern pthread_mutex_t clock_mtx;
extern pthread_cond_t tickwork_cnd;
extern pthread_cond_t pending_cnd;

void kclock();
