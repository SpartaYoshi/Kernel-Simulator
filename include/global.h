#ifndef GLOBAL_H
#define GLOBAL_H

#include <pthread.h>
#include <sys/types.h>


//////////////////
// Declarations //
//////////////////

// Constants
#define NTIMERS  2

#define MAX_CPUS		3
#define MAX_CORES		8
#define MAX_THREADS    32

#define QUEUE_CAPACITY   100
#define QUANTUM_DEFAULT  40

#define MEMSIZE_BITS 24

// States
#define MACH_OFF   0
#define MACH_ON    1

#define PRSTAT_IDLE 	 0
#define PRSTAT_RUNNING   1
#define PRSTAT_FINISHED  2

#define POL_FCFS  0
#define POL_RR    1

// Macros (for thread stack)
#define push(sp, n) (*((sp)++) = (n))
#define pop(sp)     (*--(sp))

// Parameters
extern unsigned int ncores;
extern unsigned int ncpu;
extern unsigned int nth;
extern unsigned int freq;	// clock tick by hardware frequency
extern int policy;

// Flag
extern int kernel_start;


///////////////
// Stuctures //
///////////////

// Alias
typedef unsigned long dword;  // 4 byte word (32 bits) for memory

// Translation Lookaside Buffer (TLB)
typedef struct {
	dword virtual_adr;
	dword physical_adr;
} tlb_t;

// Memory Management Unit (MMU)
typedef struct {
	tlb_t tlb;
} mmu_t;

// Memory Management Type
typedef struct {
	dword* code;
	dword* data;
	dword* pgb;
} mm_t;

/*
// Context for PCB
typedef struct {
	unsigned int pc; 	// Virtual address of next instruction
	char* ir;		 	// Last instruction executed
	mmu_t mem_mgr;
} pcb_context_t;
*/

// Process Control Block (PCB)
typedef struct _pcb { 
	struct _pcb * next;
	pid_t         pid;
	int           state;
	int           priority;
	unsigned int  quantum;
	mm_t		  mm;
} pcb_t;

// Process Queue
typedef struct {
	int    front, rear, size; 
	pcb_t* q[QUEUE_CAPACITY]; 
} process_queue_t;

// Thread
typedef struct {
	int			global_tid;
	pthread_t   handle;
	pcb_t*      proc;
	char        proc_name[128];
	dword		pc;
	dword		ir;
	dword*		ptbr;		
	mmu_t		mmu;
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
	thread_t*  stack[MAX_THREADS * MAX_CORES * MAX_CPUS];
	thread_t** sp;
	int        size;
}thread_stack_t;

#endif
