#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include "commons.h"

#define KERNEL_LIMIT 0x400000
#define PAGE_TABLE_SIZE 16
#define PAGE_SIZE 4096 // 4KB pages - 4096 pages
#define OFFSET_LEN 12 

void init_memory();
pte_t * create_page_table();

extern uint8_t * physical; 	    // Physical memory. Size = 2²⁴ bytes

uint32_t memread(uint32_t address);
void memwrite(uint32_t address, uint32_t data);
uint32_t alloc_page();

uint32_t translate(thread_t * th, uint32_t logicadr);
void insert_frame(thread_t * th, uint32_t physadr);
#endif