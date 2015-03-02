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


#include <uc/types.h>
#include <uc/uc.h>
#include <uc/processor.h>
#include <uvideo/uvideo.h>
#include <uvideo/vbe.h>
#include <uvideo/vga.h>
#include <stub.h>
#include <multiboot.h>
#include <utils.h>
#include <command.h>
#include <memory.h>
#include <log.h>

// Do SEXIT
bool bDoSexit = true;

// Global Multiboot variable
const multiboot_info_t * g_mbi;

//main video console object
video_console mainconsole;

static void configure_video(const multiboot_info_t * mbi) {
   VbeInfoBlock *       vbe_ci;
   VbeModeInfoBlock *   vbe_mi;
   char                 vbeError[256];
	
	if(mbi == 0) {
	   snprintf(vbeError, sizeof(vbeError), "!! No MBI !!");
	   goto vgatext;
	}
	
	if(! (mbi->flags & MULTIBOOT_INFO_VIDEO_INFO)) {
	   snprintf(vbeError, sizeof(vbeError), "!! No video information in MBI !!");
	   goto vgatext;
	}
	
   vbe_ci = (VbeInfoBlock *) mbi->vbe_control_info;
   vbe_mi = (VbeModeInfoBlock *) mbi->vbe_mode_info;
      
   if(vbe_ci->VbeSignature != VBESIGNATURE) {
      snprintf(vbeError, sizeof(vbeError), "!! Bad VBE signature: %08x !!", vbe_ci->VbeSignature);
      goto vgatext;
   }
   
   if(vbe_ci->VbeVersion < VBE2_0) {
      snprintf(vbeError, sizeof(vbeError), "!! Bad VBE version (not supported): %08x !!", vbe_ci->VbeVersion);
      goto vgatext;
   }
   
   if(vbe_mi->MemoryModel != VBE_DIRECT) {
      snprintf(vbeError, sizeof(vbeError), "!! Bad VBE memory model (not supported): %u !!", vbe_mi->MemoryModel);
      goto vgatext;
   }
   
   if((vbe_mi->ModeAttributes & 0x8) == 0) {
      snprintf(vbeError, sizeof(vbeError), "!! Bad VBE mode attributes (monochrome not supported): %u !!", vbe_mi->ModeAttributes);
      goto vgatext;
   }
   
   if((vbe_mi->ModeAttributes & 0x10) == 0) {
      snprintf(vbeError, sizeof(vbeError), "!! Bad VBE mode attributes (text not supported): %u !!", vbe_mi->ModeAttributes);
      goto vgatext;
   }
   
   uvideo_init(false, vbe_mi->XResolution, vbe_mi->YResolution, (uint8_t *) vbe_mi->PhysBaseAddr, (vbe_mi->BitsPerPixel + 1) / 8,  
      RGBRPACK(vbe_mi->RedMaskSize, vbe_mi->GreenMaskSize, vbe_mi->BlueMaskSize, vbe_mi->RsvdMaskSize), 
      RGBRPACK(vbe_mi->RedFieldPosition, vbe_mi->GreenFieldPosition, vbe_mi->BlueFieldPosition, vbe_mi->RsvdFieldPosition));

   uvideo_reset();
   uvideo_createcons(&mainconsole, 0, 0, vbe_mi->XResolution, vbe_mi->YResolution, 0xffff, 0x0);   
   uvideo_setfocus(&mainconsole);
   uvideo_cls();

	return;
	
vgatext:
   // We assume that we are in VGA text mode 80x25 with buffer at B8000h
   uvideo_init(true, VGA_TEXT_WIDTH, VGA_TEXT_HEIGHT, (unsigned char *) VGA_TEXT_BUFFER, 0, 0, 0);
   uvideo_reset();
   uvideo_createcons(&mainconsole, 0, 0, VGA_TEXT_WIDTH, VGA_TEXT_HEIGHT, VGA_COLOR_WHITE, VGA_COLOR_BLACK);
   uvideo_setfocus(&mainconsole);
   uvideo_cls();
   LOGERROR("%s\n", vbeError);
}

/**
 * Tloader main entry point
 */
void tloader_main(const uint32_t magic, const multiboot_info_t * mbi);
void tloader_main(const uint32_t magic, const multiboot_info_t * mbi) {

   // This is not a multiboot boot process goto into the loop
   if((magic != MULTIBOOT_BOOTLOADER_MAGIC) || (mbi == NULL)) {
      goto end;
   }

   // Set log verbosity
   SetLogVerbosity(TLOADER_ERROR);

   // Configure video
   configure_video(mbi);

   uvideo_printf("Welcome in Tloader\n");

   // Check protected mode
   if(! (CR0_PE & read_cr0())) {
      LOGERROR("!! Protected mode disabled !!\n");
      goto end;
   }

   // Check multiboot command line
   if(!(mbi->flags & MULTIBOOT_INFO_CMDLINE)) {
      uvideo_printf("!! No Tloader command line found !!\n");
      goto end;
   }
   
   g_mbi = mbi;
   
   //Initialize memory
   memory_init((multiboot_info_t *) mbi);
   
   // Print mbi
   print_mbi(magic, mbi);
   
   // Init LIBTPM
   LIBTPM_SetLogVerbosity(LIBTPM_ERROR);
   LIBTPM_Init(stub_tpm_io, stub_malloc, stub_free, stub_printf);
   
   // Init TPM Key storage
   TPM_InitStoredKey();
   
   // Parse main command line
   cmd_process_main(mbi);
   
   // Parse all modules command line
   if((mbi->flags & MULTIBOOT_INFO_MODS) && mbi->mods_count) {
      uint32_t i;
      for(i = 0 ; i < mbi->mods_count ; i++) {
         multiboot_module_t * module = (multiboot_module_t *)(mbi->mods_addr + i * sizeof(multiboot_module_t));
         cmd_process_module(module);
      }
    }
   
end:
   uvideo_printf("End Tloader\n");
   boot_launch();
}

/*
 * Local variables:
 * mode: C
 * c-set-style: "BSD"
 * c-basic-offset: 3
 * tab-width: 3
 * indent-tabs-mode: nil
 * End:
 */
