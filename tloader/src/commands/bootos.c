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
#include <uc/types.h>
#include <uvideo/uvideo.h>
#include <libtpm/libtpm.h>
#include <command.h>
#include <memory.h>
#include <log.h>
#include <txt.h>
#include <utils.h>

extern bool bDoSexit;

static pf_boot bootprelaunch = 0;
static pf_boot bootlaunch = 0;
static pf_boot bootclear = 0;

#define SIPIVECTOR 0x70
#define SIPIHANDLERADDR ((unsigned char *) (SIPIVECTOR << 12))

/* 
   [BITS 16]
   [ORG 0x70000]

   A32     lock inc dword [cpt]
           hlt
   align 4
   cpt:
           dd 0
   */
//uint8_t  trampocode[] = "\x67\x66\xF0\xFF\x05\x0C\x00\x07\x00\xF4\x89\xF6\x00\x00\x00\x00";
uint8_t  trampocode[] = "\xF4";

static void do_sexit(void) {
   uint32_t fake __attribute((unused));
   
   if(support_smx() == false) {
      return;
   }
   
   if(is_in_drtm() == false) {
      return;
   }

   if(bDoSexit == false) {
      return;
   }

   memcpy(SIPIHANDLERADDR, trampocode, sizeof(trampocode));
   
   LOGDEBUG("Send INIT\n");
   apic_send_ipi_other((uint8_t *) (uintptr_t) apic_get_base(), apic_prepare_ipi_init());
   delay(10);
   
   LOGDEBUG("Send SIPI\n");
   apic_send_ipi_other((uint8_t *) (uintptr_t) apic_get_base(), apic_prepare_ipi_sipi(SIPIVECTOR));
   delay(200);
   
   LOGDEBUG("Send SIPI\n");
   apic_send_ipi_other((uint8_t *) (uintptr_t) apic_get_base(), apic_prepare_ipi_sipi(SIPIVECTOR));
   delay(200);
   
   LOGDEBUG("Waiting\n"); 
   delay(200);
   
   LOGDEBUG("SEXIT ...\n");
   
   // Invalidate caches
   wbinvd();
   
   // Write NoSecrets
   txt_write_priv_config_reg((uint8_t *) TXT_PRIV_CONFIG_REGS_BASE, TXTCR_CMD_NO_SECRETS, 0x01);
   fake = txt_read_pub_config_reg((uint8_t *) TXT_PUB_CONFIG_REGS_BASE, TXTCR_E2STS);
   
   // Unlock the system memory configuration
   txt_write_priv_config_reg((uint8_t *) TXT_PRIV_CONFIG_REGS_BASE, TXTCR_CMD_UNLOCK_MEM_CONFIG, 0x01);
   fake = txt_read_pub_config_reg((uint8_t *) TXT_PUB_CONFIG_REGS_BASE, TXTCR_E2STS);
   
   // Close locality 1 and 2
   txt_write_priv_config_reg((uint8_t *) TXT_PRIV_CONFIG_REGS_BASE, TXTCR_CMD_CLOSE_LOCALITY1, 0x01);
   fake = txt_read_pub_config_reg((uint8_t *) TXT_PUB_CONFIG_REGS_BASE, TXTCR_E2STS);
   txt_write_priv_config_reg((uint8_t *) TXT_PRIV_CONFIG_REGS_BASE, TXTCR_CMD_CLOSE_PRIVATE, 0x01);
   fake = txt_read_pub_config_reg((uint8_t *) TXT_PUB_CONFIG_REGS_BASE, TXTCR_E2STS);

   __getsec_sexit();
   
   LOGDEBUG("SEXIT DONE\n");
}

void boot_launch(void) {
   if(bootprelaunch) {
      bootprelaunch();
   }

   TPM_ClearStoredKey();
   memory_clear();
   do_sexit();

   if(bootlaunch) {
      bootlaunch();
   } else {
      LOGERROR("!! Nothing to boot !!");
   }
}

void boot_set(pf_boot prelaunch, pf_boot launch, pf_boot clear) {
   if(bootclear) {
      bootclear();
   }
   
   bootprelaunch = prelaunch;
   bootlaunch = launch;
   bootclear = clear;
}

