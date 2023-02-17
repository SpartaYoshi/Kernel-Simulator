#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

#define KERNEL_LIMIT 0x3FFFFF
#define PAGE_TABLE_SIZE 16

void init_memory();
uint8_t * create_page_table();

#endif