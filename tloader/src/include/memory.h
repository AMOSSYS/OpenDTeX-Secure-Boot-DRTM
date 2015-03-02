#pragma once

#include <multiboot.h>

void memory_init(multiboot_info_t * mbi);
void memory_clear(void);
uint8_t * memory_getmaxmem(void);
void * malloc(size_t size);
void free(void * ptr);

