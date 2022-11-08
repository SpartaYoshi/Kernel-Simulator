#include <pthread.h>
#include <sys/types.h>


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



// Core
typedef struct {
		pthread_t *threads;
	} core_t;

// CPU
typedef struct _cpu {
		core_t *cores;
	} cpu_t;

// Machine
typedef struct {
	cpu_t *cpus;
} machine_t;