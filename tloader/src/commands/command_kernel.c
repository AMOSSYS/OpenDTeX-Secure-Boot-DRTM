// -*- coding: utf-8 -*-

/*
 * Copyright (c) 2012-2014, AMOSSYS
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the organizations nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include <uc/types.h>
#include <uc/uc.h>
#include <uvideo/uvideo.h>
#include <command.h>
#include <kernel.h>

// From load.S
extern char* linux_bzimage_addr;
extern char* linux_zimage_addr;
extern char* linux_text_len;
extern char* linux_data_tmp_addr;
extern char* linux_data_real_addr;


// From main.c
extern const multiboot_info_t * g_mbi;

// Internal
static int cur_addr;

/**
 * Launch a Linux kernel
 */
void command_kernel(uint8_t * module_addr, uint32_t module_size,
                 char * module_name,
                 int32_t argc, char ** argv) {
   struct linux_kernel_header *lh;
   unsigned long protected_mode_size = 0, real_mode_size = 0;
   int big_linux = 0;
   unsigned int idx = 0;
   
   uvideo_printf("launchkernel : '%s' mod_start: 0x%x, mod_end: 0x%x\r\n",
                module_name, module_addr, module_addr + module_size);

   uvideo_printf("launchkernel : cmd line '");
   for(idx=0; idx<(unsigned int)argc; idx++) uvideo_printf("%s ", argv[idx]);
   uvideo_printf("'\r\n");

   // First we read header to make some checks
   uvideo_printf ("launchkernel: read kernel header\n");

   // Check kernel validity, cast as a linux header
   lh = (struct linux_kernel_header *) module_addr;
   if ( lh->boot_flag != BOOTSEC_SIGNATURE ) {
      uvideo_printf("launchkernel : boot_flag != %x\r\n", BOOTSEC_SIGNATURE);
      return;
   }

   // setup_sects initialized from header, should be not defined so set a
   // default value
   if ( lh->setup_sects == 0 ) lh->setup_sects = LINUX_DEFAULT_SETUP_SECTS;
   else if ( lh->setup_sects > LINUX_MAX_SETUP_SECTS ) {
      uvideo_printf("launchkernel : linux max sectors > %x\r\n",
                   LINUX_MAX_SETUP_SECTS);
      return;
   }
   if ( lh->header != LINUX_MAGIC_SIGNATURE ) {
      uvideo_printf("launchkernel : linux magic signature != %x\r\n",
                   LINUX_MAGIC_SIGNATURE);
      return;
   }
   if ( lh->version < 0x0200 ) {
      uvideo_printf("launchkernel : linux version < 0x0200\r\n");
      return;
   }

   // Kernel format and loader type
   big_linux = (lh->loadflags & LINUX_FLAG_BIG_KERNEL);
   lh->type_of_loader = LINUX_BOOT_LOADER_TYPE;

   // Put the real data part at high level but under traditional area
   linux_data_real_addr = (char *) ((g_mbi->mem_lower << 10) -
                                    LINUX_SETUP_MOVE_SIZE);
   if (linux_data_real_addr > (char *) LINUX_OLD_REAL_MODE_ADDR)
      linux_data_real_addr = (char *) LINUX_OLD_REAL_MODE_ADDR;

   if ( lh->version >= 0x0201 ) {
      lh->heap_end_ptr = LINUX_HEAP_END_OFFSET;
      lh->loadflags |= LINUX_FLAG_CAN_USE_HEAP;
   }

   if ( lh->version >= 0x0202 ) {
      lh->cmd_line_ptr = linux_data_real_addr + LINUX_CL_OFFSET;
   }
   else {
      lh->cl_magic = LINUX_CL_MAGIC;
      lh->cl_offset = LINUX_CL_OFFSET;
      lh->setup_move_size = LINUX_SETUP_MOVE_SIZE;
   }

   real_mode_size = ( lh->setup_sects ) << 9;
   protected_mode_size = module_size - real_mode_size - SECTOR_SIZE;

   // Kernel is not relocatable ... protected base is 0x100000
   linux_data_tmp_addr = (char *) LINUX_BZIMAGE_ADDR + protected_mode_size;

   // Some checks on the kernel size
   if (! big_linux  && protected_mode_size > 
       (unsigned long)(linux_data_real_addr - (char *) LINUX_ZIMAGE_ADDR)) {
      uvideo_printf("launchkernel : kernel too big, try a bzImage\r\n");
      return;
   } else if (linux_data_real_addr + LINUX_SETUP_MOVE_SIZE
              > (char *) (g_mbi->mem_lower << 10)) {
      uvideo_printf("launchkernel : kernel too big !\r\n");
      return;
   }

   // Everything seems good !
   uvideo_printf ("launchkernel: %s, setup=0x%x, size=0x%x]\n",
                 (big_linux ? "bzImage" : "zImage"),
                 real_mode_size, protected_mode_size);
   
   // Ignore the vga=() setup ... arrrgh very bad, really should move to multiboot
   // Ignore the mem=() setup

   // Load real-mode part
   uvideo_printf ("launchkernel: load real-mode part of kernel ... ");
   memcpy(linux_data_tmp_addr, module_addr, real_mode_size + SECTOR_SIZE);
   uvideo_printf ("DONE\n");
   uvideo_printf ("launchkernel: linux_data_tmp_addr=%p, real_mode_size + SECTOR_SIZE=%d"
                 " module_size=%d\n",
                 linux_data_tmp_addr, real_mode_size + SECTOR_SIZE, module_size);
   
   if (lh->header != LINUX_MAGIC_SIGNATURE ||
       lh->version < 0x0200) {
      uvideo_printf ("launchkernel: set heap space\n");
       // Clear the heap space.
      memset (linux_data_tmp_addr + ((lh->setup_sects + 1) << 9),
              0, (64 - lh->setup_sects - 1) << 9);
   }

   // Ignore command line for the moment ...

   uvideo_printf ("launchkernel: prepare launch, seek=%d\n",
                 real_mode_size + SECTOR_SIZE);

   // offset into file    
   cur_addr = (int) linux_data_tmp_addr + LINUX_SETUP_MOVE_SIZE;

   uvideo_printf("launchkernel: KERNEL_TYPE = %d\n", big_linux);

   uvideo_printf ("launchkernel: load protected-mode part of kernel ... ");
   uvideo_printf ("launchkernel: LINUX_BZIMAGE_ADDR=%p, from=%d / %d\n",
                 LINUX_BZIMAGE_ADDR, protected_mode_size, module_size);

   /*
   for(idx=0; idx<protected_mode_size; idx++) {
      uvideo_printf("<%x/%x", LINUX_BZIMAGE_ADDR + idx, &module_addr[real_mode_size + SECTOR_SIZE + idx]);
      memcpy((char *) LINUX_BZIMAGE_ADDR + idx,
              &module_addr[real_mode_size + SECTOR_SIZE + idx],
              1);
      uvideo_printf(">");
   }
   */
   memcpy((char *) LINUX_BZIMAGE_ADDR, 
           &module_addr[real_mode_size + SECTOR_SIZE], protected_mode_size);
   uvideo_printf ("DONE\n");

   uvideo_printf("launchkernel : will boot kernel...\n");
   
   // Hack
   linux_text_len = (char*) protected_mode_size;
         
   uvideo_printf ("launchkernel: linux_data_tmp_addr=%p\n", linux_data_tmp_addr);
   uvideo_printf ("launchkernel: linux_data_real_addr=%p\n", linux_data_real_addr);

   big_linux_boot();

   return;
}

