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
#include <uc/types.h>
#include <command.h>
#include <tloader.h>
#include <kernel.h>
#include <kbd.h>
#include <log.h>

extern bool bDoSexit;

void prompt_password(const char * title, char * pwd, uint32_t len) {
   uint32_t i = 0;
   
   if(title == 0) {
      uvideo_printf("Enter password:");
   } else {
      uvideo_printf("%s", title);
   }
   
   for(i = 0 ; i < len ; i++) {
      pwd[i] = kbd_getc();
      //grub_putchar(pwd[i]);   // Display keystrokes
         
      if((pwd[i] == '\n') || (pwd[i] == '\r')) {
         pwd[i] = 0;
         uvideo_putc('\n');
         break;
      }
   }
   
   if(i == len) {
      LOGERROR("Error: buffer password too small, password is truncated ...\n");
      pwd[i] = 0;
   }
}

int32_t parse_cmdline(const char * cmdline, char ** argv, int32_t size_argv, char * buffer, uint32_t size_buffer) {
   size_t   len;
   char *   ptr;
   int32_t  cpt;
   
   if(cmdline == 0) {
      return 0;
   }
   
   len = strlen(cmdline);
   
   if(len >= size_buffer) {
      return -1;
   }
   
   strncpy(buffer, cmdline, size_buffer);
   
   // Replace all space characters by null character
   for(ptr = buffer ; ptr < buffer + len ; ptr++) {
      if(*ptr == ' ') {
         *ptr = 0;
      }
   }
   
   for(ptr = buffer, cpt = 0 ; (ptr < buffer + len) && (cpt < size_argv) ; ptr++) {
      if(*ptr) {
         // Get a word
         argv[cpt] = ptr;
         cpt++;
         
         // Go to end of word
         while(*ptr)
            ptr++;
      }
   }   
   
   return cpt;
}

typedef void (* pf_command)(uint8_t * module_addr, uint32_t module_size, char * module_name, int32_t argc, char ** argv);
typedef struct {
   const char *   command_name;
   pf_command     command_function;
} COMMAND_BLOB;

#define NB_CMD 6
static COMMAND_BLOB tab_commands[NB_CMD] = {
   { "loadkey",      command_loadkey },
   { "banner",       command_banner },
   { "sealedkernel32", command_sealedkernel32 },
   { "kernel",       command_kernel },
   { "kernel32",     command_kernel32 },
   { "initrd32",     command_initrd32 }
};

void cmd_process_module(const multiboot_module_t * module) {
   char buffer[1024];
   char * argv[256];
   int32_t argc;
   int32_t i;
   
   LOGDEBUG("Module processing\n");
   LOGDEBUG("  Address: %08p-%08p\n", module->mod_start, module->mod_end);
   LOGDEBUG("  Cmdline: %s\n", module->cmdline);
   
   argc = parse_cmdline((const char *) module->cmdline, argv, 256, buffer, sizeof(buffer));
   
   if(argc < 2) {
      LOGERROR("!! Bad module parameters (there must be at least a module name and a command name) !!\n");
      return;
   }

   for(i = 0 ; i < NB_CMD ; i++) {
      if(strcmp(tab_commands[i].command_name, argv[1]) == 0) {
         LOGDEBUG("Module command found '%s'\n", tab_commands[i].command_name);
         
         tab_commands[i].command_function((uint8_t *) module->mod_start,
                                          module->mod_end - module->mod_start,
                                          argv[0], argc - 2, &argv[2]);
         break;
      }
   }
   
   if(i == NB_CMD) {
       LOGERROR("!! %s command not found !!\n", argv[1]);
   }
}


void cmd_process_main(const multiboot_info_t * mbi) {
   char buffer[1024];
   char * argv[256];
   int32_t argc;
   int32_t i;
   
   argc = parse_cmdline((const char *) mbi->cmdline, argv, 256, buffer, sizeof(buffer));
   if(argc < 1) {
      LOGERROR("!! Invalid module parameters (there must be at least a module name !!)\n");
      return;
   }
   
   // If no command return
   if(argc == 1) {
      return;
   }

   // Actually nothing to do on the main command line
   for(i = 1 ; i < NB_CMD ; i++) {
      if(strncmp("nogetsecexit", argv[i], sizeof("nogetsecexit")) == 0) {
         bDoSexit = false;
      } else if(strncmp("nolog", argv[i], sizeof("nolog")) == 0) {
         SetLogVerbosity(TLOADER_NOLOG);
      } else if(strncmp("logerror", argv[i], sizeof("logerror")) == 0) {
         SetLogVerbosity(TLOADER_ERROR);
      } else if(strncmp("logdebug", argv[i], sizeof("logdebug")) == 0) {
         SetLogVerbosity(TLOADER_DEBUG);
      } else {
         LOGERROR("!! invalid option : %s !!\n", argv[i]);
      }
   }
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
