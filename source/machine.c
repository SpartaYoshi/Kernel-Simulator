#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "../include/commons.h"
#include "../include/machine.h"
#include "../include/clock.h"
#include "../include/sched.h"
#include "../include/procgen.h"
#include "../include/ansi.h"
#include "../include/memory.h"
#include "../include/queue.h"


machine_t      mach;
thread_stack_t thstack;

pthread_mutex_t machine_mtx;
pthread_cond_t machine_run_cnd;
pthread_cond_t machine_exit_cnd;


// Timer for machine
void timer_machine() {
	printf("%sInitiated:%s Timer for machine\n", C_BCYN, C_RESET);

	unsigned int current_tick = 0;

	while(!kernel_start);

	pthread_mutex_lock(&clock_mtx);
	pthread_mutex_lock(&machine_mtx);

	while (1) {		
		timers_done++;
		while (current_tick < 1000*freq)	// Example: multiplication depending on frequency,
											// it takes longer or shorter time
			current_tick++;
		current_tick = 0;

						printf("TIMER MACHINE\n");

		// Run loader...
		pthread_cond_signal(&machine_run_cnd);
			// Here, the loader takes action and the timer waits for it to finish
		pthread_cond_wait(&machine_exit_cnd, &machine_mtx); 
		
								printf("TIMER MACHINE2\n");

		// Signal to clock
		pthread_cond_signal(&tickwork_cnd);
		pthread_cond_wait(&pending_cnd, &clock_mtx);
	}
}

// Machine
void kmachine() {
	int i, j, k;
	printf("%sInitiated:%s Machine process\n", C_BCYN, C_RESET);

	init_machine();
	init_memory();
	while(!kernel_start);
	
	while(1) {
		pthread_cond_wait(&machine_run_cnd, &machine_mtx);
		
		printf("%smachine    %s>>   Execution tick.\n",\
		 C_BMAG, C_RESET); 

		for (i = 0; i < ncpu; i++) {
			for (j = 0; j < ncores; j++){
				core_t* jcore = &mach.cpu[i].core[j];
				for (k = 0; k < jcore->thread_count; k++) {
					pcb_t* process_block = jcore->thread[k].proc;
					if (!process_block)
						continue;

					if (process_block->state == PRSTAT_RUNNING)
						execute(&jcore->thread[k]); // execute instructions on all active threads
				}
			}
		}
		pthread_cond_signal(&machine_exit_cnd);
	}

}


void init_machine() {
	printf("%sInitiated:%s Machine (%d %d-core CPUs)\n",\
		 C_BCYN, C_RESET, ncpu, ncores);

	mach = (machine_t) { (cpu_t*) malloc(sizeof(cpu_t) * ncpu), MACH_OFF };
	int gtid = 0;

	for (int i = 0; i < ncpu; i++) {
		cpu_t icpu = { (core_t*) malloc(sizeof(core_t) * ncores) } ;

		for (int j = 0; j < ncores; j++) {
			printf("Initializing core %d of cpu %d... \n", j, i);
			core_t jcore;

			jcore.cid = j;
			jcore.thread_count = nth;

			// Initialize thread attributes
			for (int k = 0; k < nth; k++){
				jcore.thread[k].proc = NULL;
				jcore.thread[k].context = NULL;
				jcore.thread[k].ptbr = NULL;
				jcore.thread[k].global_tid = gtid++;
			}
			icpu.core[j] = jcore;
		}
		mach.cpu[i] = icpu;
	}
	mach.is_running = MACH_ON;
	
	// Init stack pointer
	thstack.sp = thstack.stack;
	thstack.size = 0;
}


void shutdown_machine() {
	printf("\nShutting down machine...\n");

	for (int i = 0; i < ncpu; i++) {
		for (int j = 0; j < ncores; j++){
			core_t* jcore = &mach.cpu[i].core[j];

			for (int k = 0; k < jcore->thread_count; k++) {
				pthread_join(jcore->thread[k].handle, NULL);
				free(jcore->thread[k].proc);
			}
		}
		free(mach.cpu[i].core);
	}
	free(mach.cpu);
	mach.is_running = MACH_OFF;
}


/*
void init_core_thread(core_t* core, thread_t* thread, void* start_routine, char* proc_name) { // UNUSED YET
	strcpy(thread->proc_name,proc_name);

	pthread_create(&thread->handle, NULL, start_routine, (void *) &core);
	
	printf("Initiated thread %d: %s \n", core->thread_count, proc_name);
	core->thread_count++;

}
*/

thread_t* find_thread(core_t* core, char* proc_name) {
	thread_t* thread = NULL;
	for (int i = 0; i < MAX_THREADS; i++)
		if (strcmp(core->thread[i].proc->context->prog_name, proc_name) == 0){
			thread = &core->thread[i];
			break;
		}
	return thread;
}

thread_t* get_thread(int tid) {
	int i = (tid / nth) / ncores; // CPU nº
	int j = (tid / nth) % ncores; // Core nº
	int k = tid % nth;			  // Thread nº

	thread_t* out = &mach.cpu[i].core[j].thread[k];
	printf("%smachine    %s>>   Address found for thread %d, core %d, cpu %d; (tid = %d).\
		 Located at %p\n",\
		 C_BMAG, C_RESET, k, j, i, tid, out);

	return out;
}


// Quantum subtraction for all processes
void subtract_quantum() {
	printf("%smachine    %s>>   Subtracting quantum for all active processes...\n",\
		 C_BMAG, C_RESET);

	for (int i = 0; i < ncpu; i++) {
		for (int j = 0; j < ncores; j++){
			core_t* jcore = &mach.cpu[i].core[j];
			for (int k = 0; k < jcore->thread_count; k++) {
				pcb_t* process_block = jcore->thread[k].proc;
				if (!process_block)
					continue;

				if (process_block->state == PRSTAT_RUNNING && process_block->quantum > 0)
					process_block->quantum--;
			}
		}
	}
}

// Compile processed threads into stack
void quantum_compiler() {
	printf("%smachine    %s>>   Compiling timed out quantums...\n",\
		 C_BMAG, C_RESET);

	for (int i = 0; i < ncpu; i++) {
		for (int j = 0; j < ncores; j++){
			core_t* jcore = &mach.cpu[i].core[j];
			for (int k = 0; k < jcore->thread_count; k++) {
				pcb_t* process_block = jcore->thread[k].proc;
				if (!process_block)
					continue;

				if (process_block->state == PRSTAT_RUNNING && process_block->quantum == 0) {
					push(thstack.sp, &jcore->thread[k]);
					thstack.size++;
				}
			}
		}
	}
}


// All busy processes execute an instruction (per clock tick)
void execute(thread_t * th){
	// Alias
	uint32_t * pc = &th->context->pc;
	uint32_t * ri = &th->context->ri;
	uint32_t * rf = th->context->rf;
	uint16_t * cc = &th->context->cc;


	// Fetch instruction
	uint32_t iadr = translate(th, *pc); 
	*ri = memread(iadr);
	*pc += 4;

	printf("%smachine    %s>>   Executing process %d. Instruction: %08X\n",\
		 C_BMAG, C_RESET, th->proc->pid, *ri); 

	// Interpret
	uint32_t cmd, rd, rs, rf1, rf2, addr;
	cmd = (*ri & 0xF0000000) >> 28;  // C-------

	switch(cmd){
		// ld   rd,addr
		case 0x0:        // CRAAAAAA
			rd   = (*ri & 0x0F000000) >> 24;
			addr = (*ri & 0x00FFFFFF);

			rf[rd] = memread(translate(th, addr));
		break;


		// st   rs,addr
		case 0x1:        // CRAAAAAA
			rs   = (*ri & 0x0F000000) >> 24;
			addr = (*ri & 0x00FFFFFF);

			memwrite(translate(th, addr), rf[rs]);
		break;

		
		// add  rd,rf1,rf2
		case 0x2:       // CRRR----
			rd  = (*ri & 0x0F000000) >> 24;
			rf1 = (*ri & 0x00F00000) >> 20;
			rf2 = (*ri & 0x000F0000) >> 16;

			rf[rd] = rf[rf1] + rf[rf2];
		break;


		// sub  rd,rf1,rf2
		case 0x3:       // CRRR----
			rd  = (*ri & 0x0F000000) >> 24;
			rf1 = (*ri & 0x00F00000) >> 20;
			rf2 = (*ri & 0x000F0000) >> 16;

			rf[rd] = rf[rf1] - rf[rf2];
		break;


		// mul  rd,rf1,rf2
		case 0x4:       // CRRR----
			rd  = (*ri & 0x0F000000) >> 24;
			rf1 = (*ri & 0x00F00000) >> 20;
			rf2 = (*ri & 0x000F0000) >> 16;

			rf[rd] = rf[rf1] * rf[rf2];
		break;


		// div  rd,rf1,rf2
		case 0x5:       // CRRR----
			rd  = (*ri & 0x0F000000) >> 24;
			rf1 = (*ri & 0x00F00000) >> 20;
			rf2 = (*ri & 0x000F0000) >> 16;

			rf[rd] = rf[rf1] / rf[rf2];
		break;

		
		// and  rd,rf1,rf2
		case 0x6:       // CRRR----
			rd  = (*ri & 0x0F000000) >> 24;
			rf1 = (*ri & 0x00F00000) >> 20;
			rf2 = (*ri & 0x000F0000) >> 16;

			rf[rd] = rf[rf1] & rf[rf2];
		break;


		// or  rd,rf1,rf2
		case 0x7:       // CRRR----
			rd  = (*ri & 0x0F000000) >> 24;
			rf1 = (*ri & 0x00F00000) >> 20;
			rf2 = (*ri & 0x000F0000) >> 16;

			rf[rd] = rf[rf1] | rf[rf2];
		break;

		
		// xor  rd,rf1,rf2
		case 0x8:       // CRRR----
			rd  = (*ri & 0x0F000000) >> 24;
			rf1 = (*ri & 0x00F00000) >> 20;
			rf2 = (*ri & 0x000F0000) >> 16;

			rf[rd] = rf[rf1] ^ rf[rf2];
		break;


		// mov  rd,rs
		case 0x9:      // CRR-----
			rd = (*ri & 0x0F000000) >> 24;
			rs = (*ri & 0x00F00000) >> 20;
			
			rf[rd] = rf[rs];
		break;


		// cmp  rf1,rf2
		case 0xA:      // CRR-----
			rf1 = (*ri & 0x0F000000) >> 24;
			rf2 = (*ri & 0x00F00000) >> 20;
			
			*cc = rf[rf1] - rf[rf2];
		break;


		// b    addr
		case 0xB:      // C-AAAAAA
			addr = (*ri & 0x00FFFFFF);
			
			*pc = addr;
		break;


		// beq  addr
		case 0xC:      // C-AAAAAA
			addr = (*ri & 0x00FFFFFF);
			
			if(cc == 0)  *pc = addr;
		break;


		// bgt  addr
		case 0xD:      // C-AAAAAA
			addr = (*ri & 0x00FFFFFF);
			
			if (cc > 0)  *pc = addr;
		break;


		// blt  addr
		case 0xE:      // C-AAAAAA
			addr = (*ri & 0x00FFFFFF);
			
			if (cc < 0)  *pc = addr;
		break;


		// exit
		case 0xF:      // C-------
			th->proc->state = PRSTAT_FINISHED;
			enqueue(&finished_queue, th->proc);

			// Free own TLB cache entries if any (needs thread for this)
			for (int i = 0; i < TLB_CAPACITY; i++)
				if (th->proc->pid == th->mmu.tlb[i].pid)
					th->mmu.tlb[i].valid = 0;
		break;
		
	}
	

}