#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

#define KERNEL_LIMIT 0x400000
#define PAGE_TABLE_SIZE 16

void init_memory();
uint8_t * create_page_table();

#endif