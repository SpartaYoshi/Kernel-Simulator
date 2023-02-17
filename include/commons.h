#ifndef COMMONS_H
#define COMMONS_H

#include <pthread.h>
#include <stdint.h>
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
#define TLB_CAPACITY       6
#define QUANTUM_DEFAULT   40


// States
#define MACH_OFF   0
#define MACH_ON    1

#define PRSTAT_IDLE 	 0
#define PRSTAT_RUNNING   1
#define PRSTAT_FINISHED  2

#define POL_FCFS  0
#define POL_RR    1

#define MOD_LOADER  0
#define MOD_PROCGEN 1

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

// Translation Lookaside Buffer (TLB)
typedef struct {
	uint32_t virt_adr; // page
	uint32_t phys_adr; // frame
} tlb_entry_t;

// Translation Lookaside Buffer (TLB)
typedef struct {
	tlb_entry_t tlb_table[TLB_CAPACITY];

} tlb_t;

// Memory Management Unit (MMU)
typedef struct {
	tlb_t tlb;
} mmu_t;

// Memory Management
typedef struct {
	uint32_t code; // points to start of code segment
	uint32_t data; // points to start of data segment
	uint32_t pgb;  // points to page table physical address
} mm_t;

// Context for PCB
typedef struct {
	uint32_t pc; 	// Virtual address of next instruction
	uint32_t ri;    // Last instruction executed
} pcb_context_t;

// Process Control Block (PCB)
typedef struct _pcb { 
	struct _pcb * next;
	pid_t         pid;
	int           state;
	int           priority;
	uint16_t	  quantum;
	mm_t		  mm;
	pcb_context_t context;
} pcb_t;

// Process Queue
typedef struct {
	int    front, rear, size; 
	pcb_t* q[QUEUE_CAPACITY]; 
} process_queue_t;

// Thread
typedef struct {
	int			 global_tid;
	pthread_t    handle;
	pcb_t*       proc;
	char         proc_name[128];
	uint8_t*	 ptbr;		
	mmu_t	     mmu;
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
