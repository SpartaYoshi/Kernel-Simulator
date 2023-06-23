#include "commons.h"

extern machine_t      mach;
extern thread_stack_t thstack;

extern pthread_mutex_t machine_mtx;
extern pthread_cond_t machine_run_cnd;
extern pthread_cond_t machine_exit_cnd;

void timer_machine();
void kmachine();

void init_machine();
void shutdown_machine();
thread_t* find_thread(core_t* core, char* proc_name);
thread_t* get_thread(int tid);
void subtract_quantum();
void quantum_compiler();
void execute(thread_t * th);