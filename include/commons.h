#ifndef COMMONS_H
#define COMMONS_H

#include <pthread.h>
#include <stdint.h>
#include <sys/types.h>


//////////////////
// Declarations //
//////////////////

// Constants
#define NTIMERS  3

#define MAX_CPUS		3
#define MAX_CORES		8
#define MAX_THREADS    32

#define QUEUE_CAPACITY   100
#define TLB_CAPACITY       6
//#define QUANTUM_DEFAULT   40
#define QUANTUM_MIN       35 // random quantum ranges from 35-50


#define NPROGRAMS 13


// States
#define MACH_OFF   0
#define MACH_ON    1

#define PRSTAT_IDLE 	 0
#define PRSTAT_RUNNING   1
#define PRSTAT_FINISHED  2
#define PRSTAT_NULL		 3

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

// Translation Lookaside Buffer Entry (TLB)
typedef struct {
	pid_t pid;
	uint32_t nPag; // page
	uint32_t nFrame; // frame
	uint8_t valid : 1;
	uint8_t frequency : 3; // lowest = 0, highest = 7
	uint8_t reserved : 4;  // unused
} tlb_t;

// Page Table Entry (PTE) (32 bytes - uint32_t)
typedef struct {
    uint32_t frame_address : 24; // frame number
    uint32_t reserved : 1; 		 // unused
	uint32_t perms : 3;          // protection bits: rwx
    uint32_t dirty : 1;  		 // 1 if page has been modified
	uint32_t accessed : 1;       // 1 if page has been accessed
	uint32_t present : 1;		 // 1 if it's present in physical memory
	uint32_t valid : 1;          // 1 if entry is valid

} pte_t;

// Memory Management Unit (MMU)
typedef struct {
	tlb_t tlb[TLB_CAPACITY];
} mmu_t;

// Memory Management
typedef struct {
	uint32_t  code;       // points to start of code segment
	uint32_t  data;       // points to start of data segment
	pte_t*    pgb;        // points to page table physical address
	uint32_t  pt_address; // address in memory of page table
	int       pt_entries; // page table entries
	uint32_t  mem_length; // how many positions do code and data occupy in memory
} mm_t;

// Context for PCB
typedef struct {
	char prog_name[128]; 	// Name of elf file
	uint32_t pc; 		 	// Virtual address of next instruction
	uint32_t ri;    		// Last instruction executed
	uint32_t rf[16];		// Register file
	uint16_t cc;            // Condition code (simplified version of FLAGS register)
} pcb_context_t;

// Process Control Block (PCB)
typedef struct _pcb { 
	struct _pcb *  next;
	pid_t          pid;
	int            state;
	int            priority;
	uint16_t	   quantum;
	mm_t		   mm;
	pcb_context_t* context;
} pcb_t;

// Process Queue
typedef struct {
	int    front, rear, size;
	pcb_t* q[QUEUE_CAPACITY];
} process_queue_t;

// Thread
typedef struct {
	int			   global_tid;
	pthread_t      handle;
	pcb_t*         proc;
	pte_t*	       ptbr;
	mmu_t	       mmu;
	pcb_context_t* context;
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
} thread_stack_t;

// Pointers to next free slot (List of free space in memory)
typedef struct _nfsp_t {
	struct _nfsp_t * prev;
	struct _nfsp_t * next;
	uint32_t  address;
	uint32_t  length;
} nfsp_t;

#endif
