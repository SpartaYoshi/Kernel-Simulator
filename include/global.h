#include <pthread.h>
#include <sys/types.h>

// Constants
#define NTIMERS 2
#define MAX_THREADS 32
#define QUEUE_CAPACITY 16

#define MACH_OFF 0
#define MACH_ON 1

// Parameters
extern unsigned int ncores;
extern unsigned int ncpu;
extern unsigned int freq;	// clock tick by hardware frequency

// Flag
extern int kernel_start;

///////////////////////////////////
// Stuctures //

// Process Control Block
typedef struct _pcb { 
	struct _pcb * next;
	pid_t pid;
	// [...]
} pcb_t;

// Process Queue
typedef struct {
	pcb_t* process[QUEUE_CAPACITY];
} process_queue_t;

// Thread
typedef struct {
	pthread_t handle;
	pcb_t* proc;
	char proc_name[128];
} thread_t;

// Core
typedef struct {
	int cid;
	thread_t thread[MAX_THREADS];
	int thread_count;
} core_t;

// CPU
typedef struct {
	core_t* core;
} cpu_t;

// Machine
typedef struct {
	cpu_t* cpu;
	int is_running;
} machine_t;