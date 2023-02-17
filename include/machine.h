#include "commons.h"

extern machine_t      mach;
extern thread_stack_t thstack;

void init_machine();
void shutdown_machine();
thread_t* find_thread(core_t* core, char* proc_name);
thread_t* get_thread(int tid);
void subtract_quantum();
void quantum_compiler();