#include <pthread.h>
#include "commons.h"

extern pthread_mutex_t loader_mtx;
extern pthread_cond_t loader_run_cnd;
extern pthread_cond_t loader_exit_cnd;

void timer_loader();
void kloader();
int load_elf(pcb_t * proc);
pcb_t * create_process();

