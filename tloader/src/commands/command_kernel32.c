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
#include <uvideo/uvideo.h>
#include <libtpm/libtpm.h>
#include <txt.h>
#include <tloader.h>
#include <command.h>
#include <memory.h>
#include <utils.h>
#include <kernel32.h>
#include <log.h>
#include <sha1.h>

size_t                     linux_mem_size = 0;
uint8_t *                  real_mode_mem = 0;
uint8_t *                  prot_mode_mem = 0;
size_t                     prot_init_space = 0;
bool                       linux32_loaded = false;

static size_t              prot_mode_size = 0;
static size_t              maximal_cmdline_size = 0;

static unsigned int check_arg(char * c, int * has_space) {
   int space = 0;
   unsigned int size = 0;

   while (*c) {
      if(*c == '\\' || *c == '\'' || *c == '"') {
         size++;
      } else if (*c == ' ') {
         space = 1;
      }

      size++;
      c++;
   }

   if(space) {
      size += 2;
   }

   if(has_space) {
      *has_space = space;
   }

   return size;
}

static int create_loader_cmdline(int argc, char * argv[], char * buf, size_t size) {
   int            i;
   int            space;
   unsigned int   arg_size;
   char *         c;

   for(i = 0 ; i < argc ; i++) {
      c = argv[i];
      arg_size = check_arg(argv[i], &space);
      arg_size++; /* Separator space or NULL.  */

      if(size < arg_size) {
         break;
      }

      size -= arg_size;

      if(space) {
         *buf++ = '"';
      }

      while(*c) {
         if(*c == '\\' || *c == '\'' || *c == '"') {
            *buf++ = '\\';
         }

         *buf++ = *c;
         c++;
      }

      if(space) {
         *buf++ = '"';
      }

      *buf++ = ' ';
   }

   /* Replace last space with null.  */
   if(i) {
      buf--;
   }

   *buf = 0;

   return i;
}

extern void relocator32_start(void);
extern uint32_t relocator32_esp;
extern uint32_t relocator32_ebp;
extern uint32_t relocator32_esi;
extern uint32_t relocator32_edi;
extern uint32_t relocator32_eax;
extern uint32_t relocator32_ebx;
extern uint32_t relocator32_ecx;
extern uint32_t relocator32_edx;
extern uint32_t relocator32_eip;
extern const multiboot_info_t * g_mbi;

static void kernel2_prelaunch(void);
static void kernel2_launch(void);
static void kernel2_clear(void);

/**
 * Launch a Linux kernel
 */
void command_kernel32(uint8_t * module_addr, uint32_t module_size, char * module_name, int32_t argc, char ** argv) {
   struct linux_kernel_header    lh;
   struct linux_kernel_params *  params;
   uint8_t                       setup_sects;
   size_t                        real_size;
   size_t                        prot_size;
   size_t                        prot_file_size;
   size_t                        len;
   int32_t                       i;
   size_t                        align;
   size_t                        min_align;
   size_t                        relocatable;
   uint64_t                      preffered_address = LINUX_BZIMAGE_ADDR;
   char                          temp[512];
   size_t                        cl_offset;
   
   if(module_size < sizeof(lh)) {
      LOGERROR("!! module too small !!\n");
      return;
   }
   
   memcpy(&lh, module_addr, sizeof(lh));

   if(lh.boot_flag != 0xaa55) {
      LOGERROR("!! invalid magic number !!\n");
      return;
   }

   if(lh.setup_sects > LINUX_MAX_SETUP_SECTS) {
      LOGERROR("!! too many setup sectors !!\n");
      return;
   }
   
   /* FIXME: Grub says 2.03 is not always good enough (Linux 2.4 can be 2.03 and
    still not support 32-bit boot.  */
   if((lh.header != LINUX_MAGIC_SIGNATURE) || (lh.version < 0x0203)) {
      LOGERROR("!! version too old for 32-bit boot !!\n");
      return;
   }

   if(! (lh.loadflags & LINUX_FLAG_BIG_KERNEL)) {
      LOGERROR("!! zImage doesn't support 32-bit boot !!\n");
      return;
   }

   if(lh.version >= 0x0206) {
      maximal_cmdline_size = lh.cmdline_size + 1;
   } else {
      maximal_cmdline_size = 256;
   }

   if(maximal_cmdline_size < 128) {
      maximal_cmdline_size = 128;
   }

   setup_sects = lh.setup_sects;

   /* If SETUP_SECTS is not set, set it to the default (4).  */
   if(! setup_sects) {
      setup_sects = LINUX_DEFAULT_SETUP_SECTS;
   }

   real_size = setup_sects << DISK_SECTOR_BITS;
   prot_file_size = module_size - real_size - DISK_SECTOR_SIZE;

   if((lh.version >= 0x205) && (lh.kernel_alignment != 0) && (((lh.kernel_alignment - 1) & lh.kernel_alignment) == 0)) {
      for(align = 0 ; align < 32 ; align++) {
	      if(lh.kernel_alignment & (1 << align)) {
	         break;
	      }
	   }
      relocatable = lh.relocatable;
   } else {
      align = 0;
      relocatable = 0;
   }
    
   if(lh.version >= 0x020a) {
      min_align = lh.min_alignment;
      prot_size = lh.init_size;
      prot_init_space = page_align(prot_size);
      if(relocatable) {
         preffered_address = lh.pref_address;
      } else {
         preffered_address = LINUX_BZIMAGE_ADDR;
      }
   } else {
      min_align = align;
      prot_size = prot_file_size;
      preffered_address = LINUX_BZIMAGE_ADDR;
      /* Usually, the compression ratio is about 50%.  */
      prot_init_space = page_align(prot_size) * 3;
   }

   real_mode_mem = (uint8_t *) LINUX_REALCODE_ADDR;
   prot_mode_mem = (uint8_t *) (uintptr_t) preffered_address;

   params = (struct linux_kernel_params *) real_mode_mem;
   memset(params, 0, sizeof(*params));
   memcpy(&params->setup_sects, &lh.setup_sects, sizeof(lh) - 0x1F1);

   params->code32_start = (uint32_t) prot_mode_mem + lh.code32_start - LINUX_BZIMAGE_ADDR;
   params->kernel_alignment = (1 << align);
   params->ps_mouse = params->padding10 =  0;

   len = sizeof (*params) - sizeof (lh);
   if(module_size < sizeof(lh) + len) {
      LOGERROR("!! module too small !!\n");
      return;
   }

   memcpy((char *) params + sizeof (lh), &module_addr[sizeof(lh)], len);

   params->type_of_loader = LINUX_BOOT_LOADER_TYPE;

   /* These two are used (instead of cmd_line_ptr) by older versions of Linux,
   and otherwise ignored.  */
   params->cl_magic = LINUX_CL_MAGIC;
   params->cl_offset = 0x1000;

   params->ramdisk_image = 0;
   params->ramdisk_size = 0;

   params->heap_end_ptr = LINUX_HEAP_END_OFFSET;
   params->loadflags |= LINUX_FLAG_CAN_USE_HEAP;

   /* These are not needed to be precise, because Linux uses these values
   only to raise an error when the decompression code cannot find good
   space.  */
   params->ext_mem = ((32 * 0x100000) >> 10);
   params->alt_mem = ((32 * 0x100000) >> 10);

   /* Ignored by Linux.  */
   params->video_page = 0;

   /* Only used when `video_mode == 0x7', otherwise ignored.  */
   params->video_ega_bx = 0;

   params->font_size = 16; /* XXX */

   /* The other parameters are filled when booting.  */
   LOGDEBUG("linux %s, min_align=0x%x, setup=0x%x, size=0x%x\n", module_name, min_align, (uint32_t) real_size, (uint32_t) prot_size);

   /* Look for memory size and video mode specified on the command line.  */
   linux_mem_size = 0;
   for(i = 1; i < argc; i++) {
      //#ifdef MACHINE_PCBIOS
      #if 0
      if(grub_memcmp (argv[i], "vga=", 4) == 0) {
         /* Video mode selection support.  */
         char *val = argv[i] + 4;
         unsigned vid_mode = LINUX_VID_MODE_NORMAL;
         struct grub_vesa_mode_table_entry *linux_mode;
         grub_err_t err;
         char *buf;

         grub_dl_load ("vbe");

         if (grub_strcmp (val, "normal") == 0) {
            vid_mode = LINUX_VID_MODE_NORMAL;
         } else if(grub_strcmp (val, "ext") == 0) {
            vid_mode = LINUX_VID_MODE_EXTENDED;
         } else if (grub_strcmp (val, "ask") == 0) {
            grub_puts_ (N_("Legacy `ask' parameter no longer supported."));
            /* We usually would never do this in a loader, but "vga=ask" means user
            requested interaction, so it can't hurt to request keyboard input.  */
            grub_wait_after_message ();
            goto fail;
         } else {
            vid_mode = (grub_uint16_t) grub_strtoul (val, 0, 0);
         }

         switch (vid_mode) {
         case 0:
         case LINUX_VID_MODE_NORMAL:
            grub_env_set ("gfxpayload", "text");
            grub_printf_ (N_("%s is deprecated. "
            "Use set gfxpayload=%s before "
            "linux command instead.\n"), "text",
            argv[i]);
            break;

         case 1:
         case LINUX_VID_MODE_EXTENDED:
            /* FIXME: support 80x50 text. */
            grub_env_set ("gfxpayload", "text");
            grub_printf_ (N_("%s is deprecated. "
               "Use set gfxpayload=%s before "
               "linux command instead.\n"), "text",
               argv[i]);
            break;
         default:
            /* Ignore invalid values.  */
            if (vid_mode < VESA_MODE_TABLE_START || vid_mode > VESA_MODE_TABLE_END) {
               grub_env_set ("gfxpayload", "text");
               /* TRANSLATORS: "x" has to be entered in, like an identifier,
               so please don't use better Unicode codepoints.  */
               grub_printf_ (N_("%s is deprecated. VGA mode %d isn't recognized. "
                  "Use set gfxpayload=WIDTHxHEIGHT[xDEPTH] "
                  "before linux command instead.\n"),
                  argv[i], vid_mode);
               break;
            }

            linux_mode = &grub_vesa_mode_table[vid_mode - VESA_MODE_TABLE_START];

            buf = grub_xasprintf ("%ux%ux%u,%ux%u",
            linux_mode->width, linux_mode->height,
            linux_mode->depth,
            linux_mode->width, linux_mode->height);
            if(! buf) {
               goto fail;
            }

            grub_printf_ (N_("%s is deprecated. "
               "Use set gfxpayload=%s before "
               "linux command instead.\n"),
               argv[i], buf);
            err = grub_env_set ("gfxpayload", buf);
            grub_free (buf);
            if (err)
               goto fail;
            }
         }
      } else
#endif /* MACHINE_PCBIOS */

      if(memcmp(argv[i], "mem=", 4) == 0) {
         char * val = argv[i] + 4;
         int shift = 0;

         linux_mem_size = strtoul(val, &val, 0);

         switch(grub_tolower (val[0])) {
         case 'g':
            shift += 10;
         case 'm':
            shift += 10;
         case 'k':
            shift += 10;
         default:
            break;
         }

         /* Check an overflow.  */
         if(linux_mem_size > (~0UL >> shift)) {
            linux_mem_size = 0;
         } else {
            linux_mem_size <<= shift;
         }
      } else if(memcmp(argv[i], "quiet", sizeof ("quiet") - 1) == 0) {
         params->loadflags |= LINUX_FLAG_QUIET;
      }
   }
      
   /* Create kernel command line.  */  
   cl_offset = ALIGN_UP(4096 + sizeof(*params), 4096);
   if(cl_offset < ((size_t) params->setup_sects << DISK_SECTOR_BITS)) {
      cl_offset = ALIGN_UP ((size_t) (params->setup_sects << DISK_SECTOR_BITS), 4096);
   }
   
   params->cmd_line_ptr = (uint32_t) real_mode_mem + cl_offset;
   
   create_loader_cmdline(argc, argv, temp, sizeof(temp));
   snprintf((char *) params->cmd_line_ptr, maximal_cmdline_size, "%s%s %s", LINUX_IMAGE, module_name, temp);

   len = prot_file_size;
   if(module_size < real_size + DISK_SECTOR_SIZE + len) {
      LOGERROR("!! module too small !!\n");
      return;
   }
   
   prot_mode_size = len;
   memcpy(prot_mode_mem, &module_addr[real_size + DISK_SECTOR_SIZE], len);
   
   // from grub_linux_boot 
   if(uvideo_istext()) {
      params->have_vga = VIDEO_LINUX_TYPE_TEXT;
      params->video_mode = 0x3;
      params->video_cursor_x = 0;
      params->video_cursor_y = 0;
      params->video_width = uvideo_getwidth();
	   params->video_height = uvideo_getheight();
   } else {
      params->lfb_width = uvideo_getwidth();
      params->lfb_height = uvideo_getheight();
      params->lfb_depth = uvideo_getbpp();
      params->lfb_line_len = uvideo_getlinelen();

      params->lfb_base = (uint32_t) uvideo_getbuffer();
      params->lfb_size = ALIGN_UP(params->lfb_line_len * params->lfb_height, 65536);

      params->red_mask_size = uvideo_getredsize();
      params->red_field_pos = uvideo_getredpos();
      params->green_mask_size = uvideo_getgreensize();
      params->green_field_pos = uvideo_getgreenpos();
      params->blue_mask_size = uvideo_getbluesize();
      params->blue_field_pos = uvideo_getbluepos();
      params->reserved_mask_size = uvideo_getrsvdsize();
      params->reserved_field_pos = uvideo_getrsvdpos();
      
      params->lfb_size >>= 16;
      params->have_vga = VIDEO_LINUX_TYPE_VESA;
   }
   
   params->ramdisk_image = 0;
   params->ramdisk_size = 0;
   params->root_dev = 0x0100; /* XXX */
   
   if(g_mbi->flags & MULTIBOOT_INFO_MEM_MAP) {
     uint32_t k = 0;
     multiboot_memory_map_t * mmap;
#if 0  
     for(mmap = (multiboot_memory_map_t *) g_mbi->mmap_addr, k = 0 ;
         (unsigned long) mmap < g_mbi->mmap_addr + g_mbi->mmap_length ;
         mmap = (multiboot_memory_map_t *) ((unsigned long) mmap + mmap->size + sizeof (mmap->size)), k++) {
       if((k > 0) &&
          (params->e820_map[k - 1].addr + params->e820_map[k - 1].size == mmap->addr &&
           params->e820_map[k - 1].type == mmap->type)) {

         params->e820_map[k - 1].size += mmap->len;

       } else {
         params->e820_map[k].addr = mmap->addr;
         params->e820_map[k].size = mmap->len;
         params->e820_map[k].type = mmap->type;
       }
     }
#else
     for (mmap = (multiboot_memory_map_t *) g_mbi->mmap_addr ;
          (unsigned long) mmap < g_mbi->mmap_addr + g_mbi->mmap_length;
          mmap = (multiboot_memory_map_t *) ((unsigned long) mmap + mmap->size + sizeof (mmap->size))) {
       params->e820_map[k].addr = mmap->addr;
       params->e820_map[k].size = mmap->len;
       params->e820_map[k].type = mmap->type;
       k++;
     }
#endif
     params->mmap_size = k;
   } else {
     params->mmap_size = 0;
   }

   //print_e820_map(params);
  
   relocator32_esp = (uint32_t) real_mode_mem;
   relocator32_ebp = 0;
   relocator32_esi = (uint32_t) real_mode_mem;
   relocator32_edi = 0;
   relocator32_eax = 0;
   relocator32_ebx = 0;
   relocator32_ecx = 0;
   relocator32_edx = 0;
   relocator32_eip = (uint32_t) params->code32_start;
   
   boot_set(kernel2_prelaunch, kernel2_launch, kernel2_clear);

   {
      TCPA_PCRVALUE hash;
      TCPA_PCRVALUE tmp;
      
      if(support_smx() == false) {
         return;
      }
      
      if(is_in_drtm() == false) {
         return;
      }
      
      sha1_buffer(prot_mode_mem, prot_mode_size, hash.digest);
      TCS_Extend(TLOADER_LOCALITY, PCR_TLOADER_CODE, &hash, &tmp);

      sha1_buffer((uint8_t *) ((struct linux_kernel_params *) real_mode_mem)->cmd_line_ptr, maximal_cmdline_size, hash.digest); 
      TCS_Extend(TLOADER_LOCALITY, PCR_TLOADER_CONFSEC, &hash, &tmp);
   }
   
   linux32_loaded = true;
}

static void kernel2_prelaunch(void) {
}

static void kernel2_launch(void) {
   relocator32_start();
}

static void kernel2_clear(void) {
   memset((uint8_t *) ((struct linux_kernel_params *) real_mode_mem)->cmd_line_ptr, 0, maximal_cmdline_size);
   memset(real_mode_mem, 0, sizeof(struct linux_kernel_params));
   memset(prot_mode_mem, 0, prot_mode_size);
   
   relocator32_esp = 0;
   relocator32_ebp = 0;
   relocator32_esi = 0;
   relocator32_edi = 0;
   relocator32_eax = 0;
   relocator32_ebx = 0;
   relocator32_ecx = 0;
   relocator32_edx = 0;
   relocator32_eip = 0;
   
   linux_mem_size = 0;
   real_mode_mem = 0;
   prot_mode_mem = 0;
   prot_init_space = 0;
   prot_mode_size = 0;
   maximal_cmdline_size = 0;
   
   linux32_loaded = false;  
}


