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
#include <uc/processor.h>
#include <stuff.h>
#include <txt/txt.h>

static acm_info_table_t * get_acmod_info_table(const acm_hdr_t * hdr) {
   uint32_t user_area_off;

   /* overflow? */
   if(plus_overflow_u32(hdr->header_len, hdr->scratch_size)) {
      return NULL;
   }

   if(multiply_overflow_u32((hdr->header_len + hdr->scratch_size), 4)) {
      return NULL;
   }

   /* this fn assumes that the ACM has already passed at least the initial */
   /* is_acmod() checks */

   user_area_off = (hdr->header_len + hdr->scratch_size) * 4;

   /* overflow? */
   if(plus_overflow_u32(user_area_off, sizeof(acm_info_table_t))) {
      return NULL;
   }

   /* check that table is within module */
   if(user_area_off + sizeof(acm_info_table_t) > hdr->size*4) {
      return NULL;
   }

   /* overflow? */
   if(plus_overflow_u32((uint32_t)(uintptr_t)hdr, user_area_off)) {
      return NULL;
   }

   return (acm_info_table_t *)((unsigned long)hdr + user_area_off);
}

static acm_chipset_id_list_t * get_acmod_chipset_list(const acm_hdr_t * hdr) {
   acm_info_table_t * info_table;
   uint32_t size, id_list_off;
   acm_chipset_id_list_t * chipset_id_list;

   /* this fn assumes that the ACM has already passed the is_acmod() checks */

   info_table = get_acmod_info_table(hdr);
   if(info_table == NULL) {
      return NULL;
   }
   id_list_off = info_table->chipset_id_list;

   size = hdr->size * 4;

   /* overflow? */
   if(plus_overflow_u32(id_list_off, sizeof(acm_chipset_id_t))) {
      return NULL;
   }

   /* check that chipset id table is w/in ACM */
   if(id_list_off + sizeof(acm_chipset_id_t) > size) {
      return NULL;
   }

   /* overflow? */
   if(plus_overflow_u32((uint32_t)(uintptr_t) hdr, id_list_off)) {
      return NULL;
   }

   chipset_id_list = (acm_chipset_id_list_t *) ((unsigned long)hdr + id_list_off);

   /* overflow? */
   if(multiply_overflow_u32(chipset_id_list->count, sizeof(acm_chipset_id_t))) {
      return NULL;
   }
   
   if(plus_overflow_u32(id_list_off + sizeof(acm_chipset_id_t), chipset_id_list->count * sizeof(acm_chipset_id_t))) {
      return NULL;
   }

   /* check that all entries are w/in ACM */
   if(id_list_off + sizeof(acm_chipset_id_t) + chipset_id_list->count * sizeof(acm_chipset_id_t) > size) {
      return NULL;
   }

   return chipset_id_list;
}

static acm_processor_id_list_t * get_acmod_processor_list(const acm_hdr_t * hdr) {
   acm_info_table_t * info_table;
   uint32_t size, id_list_off;
   acm_processor_id_list_t * proc_id_list;

   /* this fn assumes that the ACM has already passed the is_acmod() checks */

   info_table = get_acmod_info_table(hdr);
   if(info_table == NULL) {
      return NULL;
   }
   id_list_off = info_table->processor_id_list;

   size = hdr->size * 4;

   /* overflow? */
   if(plus_overflow_u32(id_list_off, sizeof(acm_processor_id_t))) {
      return NULL;
   }

   /* check that processor id table is w/in ACM */
   if(id_list_off + sizeof(acm_processor_id_t) > size) {
      return NULL;
   }

   /* overflow? */
   if(plus_overflow_u32((unsigned long)hdr, id_list_off)) {
      return NULL;
   }

   proc_id_list = (acm_processor_id_list_t *) ((unsigned long)hdr + id_list_off);

   /* overflow? */
   if(multiply_overflow_u32(proc_id_list->count,sizeof(acm_processor_id_t))) {
      return NULL;
   }
   
   if(plus_overflow_u32(id_list_off + sizeof(acm_processor_id_t), proc_id_list->count * sizeof(acm_processor_id_t))) {
      return NULL;
   }

   /* check that all entries are w/in ACM */
   if(id_list_off + sizeof(acm_processor_id_t) + proc_id_list->count * sizeof(acm_processor_id_t) > size) {
      return NULL;
   }

   return proc_id_list;
}

static bool is_acmod(const void * acmod_base, uint32_t acmod_size, uint8_t * type) {
   acm_hdr_t * acm_hdr = (acm_hdr_t *)acmod_base;

   /* first check size */
   if(acmod_size < sizeof(acm_hdr_t)) {
      return false;
   }

   /* then check overflow */
   if(multiply_overflow_u32(acm_hdr->size, 4)) {
      return false;
   }

   /* then check size equivalency */
   if(acmod_size != acm_hdr->size * 4) {
      return false;
   }

   /* then check type and vendor */
   if((acm_hdr->module_type != ACM_TYPE_CHIPSET) || (acm_hdr->module_vendor != ACM_VENDOR_INTEL)) {
      return false;
   }

   acm_info_table_t * info_table = get_acmod_info_table(acm_hdr);
   if(info_table == NULL) {
      return false;
   }

   /* check if ACM UUID is present */
   if(! txt_are_uuids_equal(&(info_table->uuid), &((uuid_t)ACM_UUID_V3))) {
      return false;
   }

   if(type != NULL) {
      *type = info_table->chipset_acm_type;
   }

   if(info_table->version < 3) {
      return false;
   }
   /* there is forward compatibility, so this is just a warning */
   else if(info_table->version > 4) {
      // Warn ...
   }

   return true;
}

bool LIBTXTCALL txt_is_racm_acmod(const void * acmod_base, uint32_t acmod_size) {
   uint8_t type;

   if(! is_acmod(acmod_base, acmod_size, &type)) {
      return false;
   }

   if(type != ACM_CHIPSET_TYPE_BIOS) {
      return false;
   }

   if(acmod_size != 0x8000 && acmod_size != 0x10000) {
      return false;
   }

   return true;
}

bool LIBTXTCALL txt_is_sinit_acmod(const void * acmod_base, uint32_t acmod_size) {
   uint8_t type;

   if(! is_acmod(acmod_base, acmod_size, &type)) {
      return false;
   }

   if(type != ACM_CHIPSET_TYPE_SINIT) {
      return false;
   }

   return true;
}

bool LIBTXTCALL txt_verify_racm(const acm_hdr_t * acm_hdr) {
   getsec_parameters_t params;
   uint32_t size;

   /* assumes this already passed is_acmod() test */

   size = acm_hdr->size * 4;        /* hdr size is in dwords, we want bytes */

   /*
   * AC mod must start on 4k page boundary
   */

   if((unsigned long)acm_hdr & 0xfff) {
      return false;
   }

   /* AC mod size must:
   * - be multiple of 64
   * - greater than ???
   * - less than max supported size for this processor
   */

   if((size == 0) || ((size % 64) != 0)) {
      return false;
   }

   if(! txt_get_parameters(&params)) {
      return false;
   }

   if(size > params.acm_max_size) {
      return false;
   }

   /*
   * perform checks on AC mod structure
   */

   /* entry point is offset from base addr so make sure it is within module */
   if(acm_hdr->entry_point >= size) {
      return false;
   }

   /* overflow? */
   if(plus_overflow_u32(acm_hdr->seg_sel, 8)) {
      return false;
   }

   if(! acm_hdr->seg_sel       ||       /* invalid selector */
   (acm_hdr->seg_sel & 0x07)   ||       /* LDT, PL!=0 */
   (acm_hdr->seg_sel + 8 > acm_hdr->gdt_limit)) {
      return false;
   }

   return true;
}

/*
 * Do some AC module sanity checks because any violations will cause
 * an TXT.RESET.  Instead detect these, print a desriptive message,
 * and skip SENTER/ENTERACCS
 */
bool LIBTXTCALL txt_verify_acmod(const acm_hdr_t * acm_hdr) {
   getsec_parameters_t params;
   uint32_t size;

   /* assumes this already passed is_acmod() test */

   size = acm_hdr->size * 4;        /* hdr size is in dwords, we want bytes */

   /*
   * AC mod must start on 4k page boundary
   */

   if((unsigned long)acm_hdr & 0xfff) {
      return false;
   }

   /* AC mod size must:
   * - be multiple of 64
   * - greater than ???
   * - less than max supported size for this processor
   */

   if((size == 0) || ((size % 64) != 0)) {
      return false;
   }

   if(! txt_get_parameters(&params)) {
      return false;
   }

   if(size > params.acm_max_size) {
      return false;
   }

   /*
   * perform checks on AC mod structure
   */

   /* entry point is offset from base addr so make sure it is within module */
   if(acm_hdr->entry_point >= size) {
      return false;
   }

   /* overflow? */
   if(plus_overflow_u32(acm_hdr->seg_sel, 8)) {
      return false;
   }

   if(! acm_hdr->seg_sel       ||       /* invalid selector */
   (acm_hdr->seg_sel & 0x07)   ||       /* LDT, PL!=0 */
   (acm_hdr->seg_sel + 8 > acm_hdr->gdt_limit) ) {
      return false;
   }

   /*
   * check for compatibility with this MLE
   */

   acm_info_table_t * info_table = get_acmod_info_table(acm_hdr);
   if(info_table == NULL) {
      return false;
   }

   /* check MLE header versions support */
   if(info_table->min_mle_hdr_ver < MLE_HDR_VER20) {
      return false;
   }

   /* check capabilities */
   /* we need to match one of rlp_wake_{getsec, monitor} */
   if((info_table->capabilities.rlp_wake_getsec || info_table->capabilities.rlp_wake_monitor) == 0) {
      return false;
   }

   /* check for version of OS to SINIT data */
   /* we don't support old versions */
   if(info_table->os_sinit_data_ver < MIN_OS_SINIT_DATA_VER ) {
      return false;
   }
   /* only warn if SINIT supports more recent version than us */
   else if(info_table->os_sinit_data_ver > MAX_OS_SINIT_DATA_VER) {
      // Warn ...
   }

   return true;
}

bool LIBTXTCALL txt_does_acmod_match_platform(uint8_t * txtbase, const acm_hdr_t * hdr) {
   /* this fn assumes that the ACM has already passed the is_acmod() checks */

   /* get chipset fusing, device, and vendor id info */
   txt_didvid_t didvid;
   didvid._raw = txt_read_pub_config_reg(txtbase, TXTCR_DIDVID);
   txt_ver_fsbif_qpiif_t ver;
   ver._raw = txt_read_pub_config_reg(txtbase, TXTCR_VER_FSBIF);
   if((ver._raw & 0xffffffff) == 0xffffffff ||
   (ver._raw & 0xffffffff) == 0x00) {         /* need to use VER.QPIIF */
      ver._raw = txt_read_pub_config_reg(txtbase, TXTCR_VER_QPIIF);
   }
   
   /* get processor family/model/stepping and platform ID */
   uint64_t platform_id;
   uint32_t fms = cpuid_eax(1);
   platform_id = rdmsr(MSR_IA32_PLATFORM_ID);

   /*
   * check if chipset fusing is same
   */
   if(ver.prod_fused != !hdr->flags.debug_signed) {
      return false;
   }

   /*
   * check if chipset vendor/device/revision IDs match
   */
   acm_chipset_id_list_t * chipset_id_list = get_acmod_chipset_list(hdr);
   if(chipset_id_list == NULL) {
      return false;
   }

   unsigned int i;
   for(i = 0 ; i < chipset_id_list->count ; i++) {
      acm_chipset_id_t * chipset_id = &(chipset_id_list->chipset_ids[i]);
      
      if((didvid.vendor_id == chipset_id->vendor_id) &&
      (didvid.device_id == chipset_id->device_id) &&
      ((((chipset_id->flags & 0x1) == 0) &&
      (didvid.revision_id == chipset_id->revision_id)) ||
      (((chipset_id->flags & 0x1) == 1) &&
      ((didvid.revision_id & chipset_id->revision_id) != 0)))) {
         break;
      }
   }
   
   if(i >= chipset_id_list->count) {
      return false;
   }

   /*
   * check if processor family/model/stepping and platform IDs match
   */
   acm_info_table_t * info_table = get_acmod_info_table(hdr);
   if(info_table->version >= 4) {
      acm_processor_id_list_t * proc_id_list = get_acmod_processor_list(hdr);
      if(proc_id_list == NULL) {
         return false;
      }

      for ( i = 0; i < proc_id_list->count; i++ ) {
         acm_processor_id_t * proc_id = &(proc_id_list->processor_ids[i]);

         if((proc_id->fms == (fms & proc_id->fms_mask)) && (proc_id->platform_id == (platform_id & proc_id->platform_mask))) {
            break;
         }
      }

      if(i >= proc_id_list->count) {
         return false;
      }
   }

   return true;
}

uint32_t LIBTXTCALL txt_get_supported_os_sinit_data_ver(const acm_hdr_t * hdr) {
   /* assumes that it passed is_sinit_acmod() */

   acm_info_table_t * info_table = get_acmod_info_table(hdr);
   if(info_table == NULL) {
      return 0;
   }

   return info_table->os_sinit_data_ver;
}

uint32_t LIBTXTCALL txt_get_sinit_capabilities(const acm_hdr_t * hdr) {
   /* assumes that it passed is_sinit_acmod() */

   acm_info_table_t * info_table = get_acmod_info_table(hdr);
   if(info_table == NULL || info_table->version < 3 ) {
      return 0;
   }

   return info_table->capabilities._raw;
}

