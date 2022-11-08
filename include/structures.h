#include <pthread.h>
#include <sys/types.h>
#include "global.h"


// Process Control Block
typedef struct _pcb { 
	struct _pcb *next;
	pid_t pid;
	// [...]
} pcb_t;

// Process Queue
typedef struct {
	pcb_t head;
} process_queue_t;


// Thread
typedef struct {
	pthread_t handle;
	pcb_t *proc;
	char proc_name[32];
} thread_t;


// Core
typedef struct {
	int cid;

	thread_t thread[MAX_THREADS];
	pthread_mutex_t clock_mtx;
	pthread_mutex_t sched_mtx;
	pthread_mutex_t procgen_mtx;

	pthread_cond_t tickwork_cnd;
	pthread_cond_t pending_cnd;
	int timers_done;
	int thread_count;
} core_t;

// CPU
typedef struct _cpu {
	core_t *core;
} cpu_t;

// Machine
typedef struct {
	cpu_t *cpu;
	int is_running;
} machine_t;