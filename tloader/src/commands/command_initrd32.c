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


#include <uc/uc.h>
#include <uvideo/uvideo.h>
#include <libtpm/libtpm.h>
#include <txt.h>
#include <command.h>
#include <memory.h>
#include <utils.h>
#include <kernel32.h>
#include <tloader.h>
#include <log.h>
#include <sha1.h>

extern uint8_t *                    real_mode_mem;
extern uint8_t *                    prot_mode_mem;
extern size_t                       prot_init_space;
extern size_t                       linux_mem_size;
extern bool                         linux32_loaded;

void command_initrd32(uint8_t * module_addr, uint32_t module_size, char * module_name __UNUSED__, int32_t argc __UNUSED__, char ** argv __UNUSED__) {
   uint8_t *   mem_max;
   uint8_t *   addr;
   uint8_t *   addr_max;
   uint8_t *   addr_min;
   size_t      size = 0;
   uint8_t *   initrd_mem;
   struct linux_kernel_params * linux_params = (struct linux_kernel_params *) real_mode_mem;

   if(! linux32_loaded) {
      LOGERROR("!! no kernel32 loaded !!\n");
      return;
   }

   size = ALIGN_UP(module_size, 4);
   
   /* Get the highest address available for the initrd.  */
   if(linux_params->version >= 0x0203) {
      addr_max = (uint8_t *) linux_params->initrd_addr_max;

      /* XXX in reality, Linux specifies a bogus value, so
      it is necessary to make sure that ADDR_MAX does not exceed
      0x3fffffff.  */
      if(addr_max > (uint8_t *) LINUX_INITRD_MAX_ADDRESS) {
         addr_max = (uint8_t *) LINUX_INITRD_MAX_ADDRESS;
      }
   } else {
      addr_max = (uint8_t *) LINUX_INITRD_MAX_ADDRESS;
   }

   if(linux_mem_size != 0 && (uint8_t *) linux_mem_size < addr_max) {
      addr_max = (uint8_t *) linux_mem_size;
   }
   
   mem_max = memory_getmaxmem();
   if(addr_max > mem_max) {
      addr_max = mem_max;
   }

   /* Linux 2.3.xx has a bug in the memory range check, so avoid
   the last page.
   Linux 2.2.xx has a bug in the memory range check, which is
   worse than that of Linux 2.3.xx, so avoid the last 64kb.  */
   addr_max -= 0x10000;

   addr_min = (uint8_t *) prot_mode_mem + prot_init_space + page_align(size);

   /* Put the initrd as high as possible, 4KiB aligned.  */
   addr = (uint8_t *) ((uintptr_t) (addr_max - size) & ~0xFFF);

   if(addr < addr_min) {
      LOGERROR("!! initrd32 is too big !!");
      return;
   }

   initrd_mem = addr;
   
   memcpy(initrd_mem, module_addr, module_size);
   memset(initrd_mem + module_size, 0, ALIGN_UP_OVERHEAD((uint32_t) initrd_mem + module_size, 4));
   
   
   linux_params->ramdisk_image = (uint32_t) initrd_mem;
   linux_params->ramdisk_size = size;

   if(((struct linux_kernel_params *) real_mode_mem)->ramdisk_image && ((struct linux_kernel_params *) real_mode_mem)->ramdisk_size) {
      TCPA_PCRVALUE hash;
      TCPA_PCRVALUE tmp;
      
      if(support_smx() == false) {
         return;
      }
      
      if(is_in_drtm() == false) {
         return;
      }

      sha1_buffer((uint8_t *) ((struct linux_kernel_params *) real_mode_mem)->ramdisk_image, ((struct linux_kernel_params *) real_mode_mem)->ramdisk_size, hash.digest);
      TCS_Extend(TLOADER_LOCALITY, PCR_TLOADER_CODE, &hash, &tmp);
   }
}

