#pragma once

#include <uc/types.h>
#include <multiboot.h>


typedef void (* pf_boot)(void);

/**
 * Parse a module command line
 *
 * @param cmdline is the full command line
 * @param argv is the table of parsed arguments
 * @param size_argv is the max count of arguments
 * @param buffer is the modified cmdline for parsing
 * @param size_buffer is the size of the modified cmdline
 *
 * @return void
 */
int32_t parse_cmdline(const char * cmdline, char ** argv, int32_t size_argv, char * buffer, uint32_t size_buffer);

/**
 * Parse a module command line
 *
 * @param module is the Multiboot module structure
 *
 * @return void
 */
void cmd_process_module(const multiboot_module_t * module);

/**
 * Parse the main tloader command line
 *
 * @param mbi is the Multiboot structure
 *
 * @return void
 */
void cmd_process_main(const multiboot_info_t * mbi);

void prompt_password(const char * title, char * pwd, uint32_t len);

void command_loadkey(uint8_t * module_addr, uint32_t module_size, char * module_name, int32_t argc, char ** argv);
void command_banner(uint8_t * module_addr, uint32_t module_size, char * module_name, int32_t argc, char ** argv);
void command_sealedkernel32(uint8_t * module_addr, uint32_t module_size, char * module_name, int32_t argc, char ** argv);
void command_kernel(uint8_t * module_addr, uint32_t module_size, char * module_name, int32_t argc, char ** argv);
void command_kernel32(uint8_t * module_addr, uint32_t module_size, char * module_name, int32_t argc, char ** argv);
void command_initrd32(uint8_t * module_addr, uint32_t module_size, char * module_name, int32_t argc, char ** argv);

void boot_launch(void);
void boot_set(pf_boot prelaunch, pf_boot launch, pf_boot clear);

