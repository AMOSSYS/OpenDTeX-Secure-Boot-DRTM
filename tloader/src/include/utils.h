#pragma once

#include <uc/types.h>
#include <multiboot.h>
#include <kernel32.h>

void print_mbi(const uint32_t magic, const multiboot_info_t * mbi);
void hexdump (unsigned char * buf, unsigned int len);
void delay(int millisecs);
void echo(void);
void print_e820_map(struct linux_kernel_params * params);

