#include "global.h"

extern machine_t mach;

void init_machine();
void shutdown_machine();
thread_t* find_thread(core_t* core, char* proc_name);