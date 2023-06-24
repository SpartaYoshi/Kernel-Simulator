#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include "commons.h"

#define KERNEL_LIMIT 0x400000
#define PAGE_TABLE_SIZE 16
#define PAGE_SIZE 4096 // 4KB pages - 4096 pages
#define FRAME_MASK  0xFFFFF000
#define OFFSET_MASK 0x00000FFF

void init_memory();
void create_page_table(pcb_t * proc);
void delete_page_table(pcb_t * proc);

extern uint8_t * physical; 	    // Physical memory. Size = 2²⁴ bytes

uint32_t memread(uint32_t address);
void memwrite(uint32_t address, uint32_t data);
uint32_t alloc_page();
void free_page(uint32_t pg_adr);

uint32_t translate(thread_t * th, uint32_t logicadr);
void insert_frame(pcb_t * proc, uint32_t physadr);

int find_tlb_slot(thread_t * th);
void update_tlb(thread_t * th, int idx, uint32_t nPag, uint32_t nFrame);

#endif