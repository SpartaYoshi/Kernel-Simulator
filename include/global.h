#include <pthread.h>
#include <sys/types.h>


//////////////////
// Declarations //
//////////////////

// Constants
#define NTIMERS  2
#define MAX_THREADS    32
#
#define QUEUE_CAPACITY 16

#define MACH_OFF   0
#define MACH_ON    1

#define PRSTAT_IDLE 	 0
#define PRSTAT_RUNNING   1
#define PRSTAT_FINISHED  2

// Macros (for thread stack)
#define push(sp, n) (*((sp)++) = (n))
#define pop(sp)     (*--(sp))

// Parameters
extern unsigned int ncores;
extern unsigned int ncpu;
extern unsigned int freq;	// clock tick by hardware frequency

// Flag
extern int kernel_start;


///////////////
// Stuctures //
///////////////

// Translation Lookaside Buffer (TLB)
typedef struct {
	unsigned int virtual_adr;
	unsigned int physical_adr;
} tlb_t;

// Context for PCB
typedef struct {
	unsigned int PC; 	// Virtual address of next instruction
	//char* IR;		 	// Last instruction executed
	//tlb_t tlb;
} pcb_context_t;

// Process Control Block (PCB)
typedef struct _pcb { 
	struct _pcb * next;
	pid_t         pid;
	int           state;
	int           priority;
	pcb_context_t context;
	unsigned int  quantum;
	// [...]
} pcb_t;

// Process Queue
typedef struct {
	int    front, rear, size; 
	pcb_t* q[QUEUE_CAPACITY]; 
} process_queue_t;

// Thread
typedef struct {
	pthread_t    handle;
	pcb_t*       proc;
	char         proc_name[128];
	unsigned int PC;
} thread_t;

// Core
typedef struct {
	int      cid;
	thread_t thread[MAX_THREADS];
	int      thread_count;
} core_t;

// CPU
typedef struct {
	core_t* core;
} cpu_t;

// Machine
typedef struct {
	cpu_t* cpu;
	int    is_running;
} machine_t;

// Thread stack
typedef struct {
	thread_t*  stack[QUEUE_CAPACITY];
	thread_t** sp;
	int        size;
}thread_stack_t;


////////////////////////////
// Structure declarations // 
////////////////////////////

const pcb_t nullp = {NULL, -1, PRSTAT_IDLE, 140, 0, 0};

extern process_queue_t idle_queue; // Queue of waiting processes

