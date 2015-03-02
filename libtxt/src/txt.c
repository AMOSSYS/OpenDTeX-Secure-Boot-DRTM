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
#include <uc/processor.h>
#include <stuff.h>
#include <txt/txt.h>

#define ACM_MEM_TYPE_UC                 0x0100
#define ACM_MEM_TYPE_WC                 0x0200
#define ACM_MEM_TYPE_WT                 0x1000
#define ACM_MEM_TYPE_WP                 0x2000
#define ACM_MEM_TYPE_WB                 0x4000

#define DEF_ACM_MAX_SIZE                0x8000
#define DEF_ACM_VER_MASK                0xffffffff
#define DEF_ACM_VER_SUPPORTED           0x00
#define DEF_ACM_MEM_TYPES               ACM_MEM_TYPE_UC
#define DEF_SENTER_CTRLS                0x00

bool LIBTXTCALL txt_get_parameters(getsec_parameters_t * params) {
   unsigned long cr4;
   uint32_t index, eax, ebx, ecx;
   int param_type;

   /* sanity check because GETSEC[PARAMETERS] will fail if not set */
   cr4 = read_cr4();
   if(! (cr4 & CR4_SMXE)) {
      return false;
   }

   memset(params, 0, sizeof(*params));
   params->acm_max_size = DEF_ACM_MAX_SIZE;
   params->acm_mem_types = DEF_ACM_MEM_TYPES;
   params->senter_controls = DEF_SENTER_CTRLS;
   params->proc_based_scrtm = false;
   params->preserve_mce = false;

   index = 0;
   do {
      __getsec_parameters(index++, &param_type, &eax, &ebx, &ecx);
      /* the code generated for a 'switch' statement doesn't work in this */
      /* environment, so use if/else blocks instead */

      /* NULL - all reserved */
      if(param_type == 0) {
         ;
      }
      /* supported ACM versions */
      else if( param_type == 1) {
         if(params->n_versions == MAX_SUPPORTED_ACM_VERSIONS) {
            // WARN ...
         } else {
            params->acm_versions[params->n_versions].mask = ebx;
            params->acm_versions[params->n_versions].version = ecx;
            params->n_versions++;
         }
      }
      /* max size AC execution area */
      else if(param_type == 2) {
         params->acm_max_size = eax & 0xffffffe0;
      }
      /* supported non-AC mem types */
      else if(param_type == 3) {
         params->acm_mem_types = eax & 0xffffffe0;
      }
      /* SENTER controls */
      else if(param_type == 4) {
         params->senter_controls = (eax & 0x00007fff) >> 8;
      }
      /* TXT extensions support */
      else if(param_type == 5) {
         params->proc_based_scrtm = (eax & 0x00000020) ? true : false;
         params->preserve_mce = (eax & 0x00000040) ? true : false;
      } else {
         param_type = 0;    /* set so that we break out of the loop */
      }
   } while(param_type != 0);

   if(params->n_versions == 0) {
      params->acm_versions[0].mask = DEF_ACM_VER_MASK;
      params->acm_versions[0].version = DEF_ACM_VER_SUPPORTED;
      params->n_versions = 1;
   }

   return true;
}

bool LIBTXTCALL txt_smx_is_supported(void) {
   uint32_t val = cpuid_ecx(1);
   return (val & CPUID_1_ECX_SMX ? true : false);
}


