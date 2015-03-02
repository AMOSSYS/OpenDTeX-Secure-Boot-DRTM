// -*- coding: utf-8 -*-

/*
 * Copyright (c) 2012-2014, AMOSSYS, Bertin
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


#include <uc/uc.h>
#include <uc/processor.h>
#include <uvideo/uvideo.h>
#include <tloader.h>
#include <multiboot.h>
#include <utils.h>
#include <e820.h>

void print_mbi(const uint32_t magic, const multiboot_info_t * mbi) {
   uvideo_printf("Multiboot\n");
	uvideo_printf("  magic = %08x\n", magic);
	uvideo_printf("  mbi   = %08x\n", mbi);
	
	if(magic == MULTIBOOT_BOOTLOADER_MAGIC) {
		 uvideo_printf("    flags = 0x%x\n", mbi->flags);
		 
		if(mbi->flags & MULTIBOOT_INFO_MEMORY) {
			uvideo_printf("    mem_lower = %uKB, mem_upper = %uKB\n", mbi->mem_lower, mbi->mem_upper);
		}

		if(mbi->flags & MULTIBOOT_INFO_BOOTDEV ) {
			uvideo_printf("    boot_device = 0x%x\n", mbi->boot_device);
		}
		
		if(mbi->flags & MULTIBOOT_INFO_CMDLINE) {
			uvideo_printf("    cmdline = %s\n", (char *) mbi->cmdline);
		}
		
		if(mbi->flags & MULTIBOOT_INFO_MODS) {
			multiboot_module_t * mod;
			unsigned int i;
			
			uvideo_printf("    mods_count = %d, mods_addr = 0x%x\n", mbi->mods_count, mbi->mods_addr);
			
			for(i = 0, mod = (multiboot_module_t *) mbi->mods_addr ; i < mbi->mods_count ; i++, mod++) {
				uvideo_printf("      mod_start = 0x%x, mod_end = 0x%x, cmdline = %s\n", mod->mod_start, mod->mod_end, mod->cmdline);
			}
		}
		
		if((mbi->flags & MULTIBOOT_INFO_AOUT_SYMS) && (mbi->flags & MULTIBOOT_INFO_ELF_SHDR)) {
			uvideo_printf("    !! Both bits 4 and 5 are set (not good)!!\n");
		}
		
		if(mbi->flags & MULTIBOOT_INFO_AOUT_SYMS) {
			const multiboot_aout_symbol_table_t * multiboot_aout_sym = &(mbi->u.aout_sym);
     
			uvideo_printf("    multiboot_aout_symbol_table:\n");
			uvideo_printf("      tabsize = 0x%0x, strsize = 0x%x, addr = 0x%x\n", multiboot_aout_sym->tabsize);
			uvideo_printf("      strsize = 0x%x, addr = 0x%x\n", multiboot_aout_sym->strsize);
			uvideo_printf("      addr = 0x%x\n", multiboot_aout_sym->addr);
		}
		
		if(mbi->flags & MULTIBOOT_INFO_ELF_SHDR) {
			const multiboot_elf_section_header_table_t *multiboot_elf_sec = &(mbi->u.elf_sec);
     
			uvideo_printf("    multiboot_elf_sec:\n");
			uvideo_printf("      num = %u\n", multiboot_elf_sec->num);
			uvideo_printf("      size = 0x%x\n", multiboot_elf_sec->size);
			uvideo_printf("      addr = 0x%x\n", multiboot_elf_sec->addr);
			uvideo_printf("      shndx = 0x%x\n", multiboot_elf_sec->shndx);
		}
		
		if(mbi->flags & MULTIBOOT_INFO_MEM_MAP) {
			multiboot_memory_map_t * mmap;
      
			uvideo_printf("    mmap_addr = 0x%x, mmap_length = 0x%x\n", mbi->mmap_addr, mbi->mmap_length);
			for (mmap = (multiboot_memory_map_t *) mbi->mmap_addr ; (unsigned long) mmap < mbi->mmap_addr + mbi->mmap_length; mmap = (multiboot_memory_map_t *) ((unsigned long) mmap + mmap->size + sizeof (mmap->size))) {
				uvideo_printf("      size = 0x%x, base_addr = 0x%08llx, length = 0x%08llx, type = 0x%x\n", mmap->size, mmap->addr, mmap->len, mmap->type);
			}
		}
		
		if(mbi->flags & MULTIBOOT_INFO_DRIVE_INFO) {
		   uvideo_printf("    drives_addr = 0x%x, drives_length = 0x%x\n", mbi->drives_addr, mbi->drives_length);
		}
		
		if(mbi->flags & MULTIBOOT_INFO_CONFIG_TABLE) {
		   uvideo_printf("    config_table = 0x%x\n", mbi->config_table);
		}
		
		if(mbi->flags & MULTIBOOT_INFO_BOOT_LOADER_NAME) {
		   uvideo_printf("    boot_loader_name = %s\n", (char *) mbi->boot_loader_name);
		}
		
		if(mbi->flags & MULTIBOOT_INFO_APM_TABLE) {
         uvideo_printf("    apm_table: 0x%x\n", mbi->apm_table);
      }
      
      if(mbi->flags & MULTIBOOT_INFO_VIDEO_INFO) {
         uvideo_printf("    vbe_control_info: 0x%x\n", mbi->vbe_control_info);
         uvideo_printf("      vbe_mode_info: 0x%x\n", mbi->vbe_mode_info);
         uvideo_printf("      vbe_mode: 0x%x\n",  mbi->vbe_mode);
         uvideo_printf("      vbe_interface_seg: 0x%x\n", mbi->vbe_interface_seg);
         uvideo_printf("      vbe_interface_off: 0x%x\n", mbi->vbe_interface_off);
         uvideo_printf("      vbe_interface_len: 0x%x\n", mbi->vbe_interface_len);
      }
		
	} else {
		uvideo_printf("!! This is a not a multiboot boot process !!\n");
	}
}

void hexdump(unsigned char * buf, unsigned int len) {
   for(unsigned int i = 0; i < len ; i++) {
      if((i % 32 == 0) && i) {
         uvideo_printf("\n");
      }
      uvideo_printf("%02X", buf[i]);
   }
   uvideo_printf("\n");
   return;
}

void echo(void) {
  uvideo_printf("ICI ");
}


void print_e820_map(struct linux_kernel_params * params) {
  struct e820_mmap * entry;

  for(unsigned int i = 0 ; i < params->mmap_size ; i++) {
    entry = params->e820_map + i;
    uvideo_printf("E820[%2d] => [0x%08llx..0x%08llx] type %d\n", i, entry->addr, entry->addr + entry->size - 1, entry->type);
  }
}
