#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#include "../include/ansi.h"
#include "../include/clock.h"
#include "../include/commons.h"
#include "../include/loader.h"
#include "../include/memory.h"
#include "../include/queue.h"

pthread_mutex_t loader_mtx;
pthread_cond_t loader_run_cnd;
pthread_cond_t loader_exit_cnd;

pcb_t* head_l;
int pcbs_generated_l = 0;
int elfi = 0;


// Timer for loader
void timer_loader() {
	printf("%sInitiated:%s Timer for loader\n", C_BCYN, C_RESET);

	unsigned int current_tick = 0;

	while(!kernel_start);

	init_queue(&idle_queue);

	pthread_mutex_lock(&clock_mtx);
	pthread_mutex_lock(&loader_mtx);

	while (1) {		
		timers_done++;
		while (current_tick < 1000*freq)	// Example: multiplication depending on frequency, \
											   it takes longer or shorter time
			current_tick++;
		current_tick = 0;

		// Run loader...
		pthread_cond_signal(&loader_run_cnd);
			// Here, the loader takes action and the timer waits for it to finish
		pthread_cond_wait(&loader_exit_cnd, &loader_mtx); 
		
		// Signal to clock
		pthread_cond_signal(&tickwork_cnd);
		pthread_cond_wait(&pending_cnd, &clock_mtx);
	}
}


// Loader
void kloader() {
	int _;
	printf("%sInitiated:%s Loader\n", C_BCYN, C_RESET);

	while(!kernel_start);

	while(1) {
		pthread_cond_wait(&loader_run_cnd, &loader_mtx);
		
		if (pcbs_generated_l < (nth * ncores * ncpu) + QUEUE_CAPACITY && \
		    idle_queue.size < QUEUE_CAPACITY) {
			printf("%sloader    %s>>   Generating process %d...\n",\
				 C_BYEL, C_RESET, pcbs_generated_l + 1);

			// Create process block
			pcb_t* block = (pcb_t *) malloc(sizeof(pcb_t));
			block->pid = ++pcbs_generated_l;
			block->state = PRSTAT_IDLE;
			srand(time(NULL));
			block->priority = rand() % 139;
			block->quantum = rand() % 15 + QUANTUM_MIN;
			block->context = (pcb_context_t *) malloc(sizeof(pcb_context_t));

			// Link to dynamic list of processes
			block->next = head_l;
			head_l = block;

			// Reserve page table in memory
			block->mm.pgb = create_page_table();
			block->mm.pt_entries = 0;
			
			// Load program into memory
			sprintf(block->context->prog_name, "./programs/elfs/prog%03d.elf", elfi);
			_ = load_elf(block);
			elfi = (elfi + 1) % NPROGRAMS; 

			// Add to idle queues
			enqueue(&idle_queue, block);

			if (_ == 0)
				printf("%sloader    %s>>   Process %d successfully generated. Located at %p\n",
					C_BYEL, C_RESET, block->pid, block);
		}
		
		pthread_cond_signal(&loader_exit_cnd);
	}
}


int load_elf(pcb_t * proc){
	FILE *fp;
	char line[16];
	int _, bytes_loaded;
	uint32_t word;
	uint32_t address;

	#define HEADER 12*2+2
	#define CHAR_PER_LINE 8+1

	// Open file	
	fp = fopen(proc->context->prog_name, "r");
	if (fp == NULL) {
		printf("%sloader    %s>>   %sERROR: %sFile could not be opened. \n",
			C_BYEL, C_RESET, C_BRED, C_RESET);
		return 1;
	}

	// Get virtual addresses for code
	_ = fscanf(fp, "%s %X", line, &proc->mm.code);
	if (_ != 2) {
		printf("%sloader    %s>>   %sERROR: %sFile could not be read: Wrong format. \n",
			C_BYEL, C_RESET, C_BRED, C_RESET);
		fclose(fp);
		return 1;
	}

	// Get virtual addresses for data
	_ = fscanf(fp, "%s %X", line, &proc->mm.data);
	if (_ != 2) {
		printf("%sloader    %s>>   %sERROR: %sFile could not be read: Wrong format. \n",
			C_BYEL, C_RESET, C_BRED, C_RESET);
		fclose(fp);
		return 1;
	}	

	// Copy to memory
	bytes_loaded = PAGE_SIZE;  // to activate first alloc. logically it should be 0
	while (!feof(fp)){
		// Scan for word (4 bytes)
		_ = fscanf(fp, "%X", &word);
		if (_ != 1) {
			printf("%sloader    %s>>   %sERROR: %sFile could not be read: Wrong format. \n",
				C_BYEL, C_RESET, C_BRED, C_RESET);
			fclose(fp);
			return 1;
		}

		// Copy 4 bytes to physical[nfi]
		if (bytes_loaded == PAGE_SIZE) {
			address = alloc_page();	// alloc new page in memory
			bytes_loaded = 0;
			insert_frame(proc, address);

		}
    	memwrite(address + bytes_loaded, word);
		bytes_loaded += 4;
	}
	fclose(fp);

	
	// Begin context
	proc->context->pc = 0x0;
	proc->context->ri = 0x0;

	return 0;
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
			// raise flag for process unloading - free thread
			th->proc->state = PRSTAT_FINISHED;
		break;
		
	}
	

}