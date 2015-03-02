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

static bool verify_bios_spec_ver_elt(const heap_ext_data_element_t * elt) {
   const heap_bios_spec_ver_elt_t * bios_spec_ver_elt = (const heap_bios_spec_ver_elt_t *)elt->data;

   // wrong size ?
   if(elt->size != sizeof(*elt) + sizeof(*bios_spec_ver_elt)) {
      return false;
   }

   // any values are allowed
   return true;
}

static bool verify_acm_elt(const heap_ext_data_element_t * elt) {
   const heap_acm_elt_t * acm_elt = (const heap_acm_elt_t *) elt->data;

   // wrong size ?
   if(elt->size != sizeof(*elt) + sizeof(*acm_elt) + acm_elt->num_acms * sizeof(uint64_t)) {
      return false;
   }

   // no addrs is not error, but print warning
   if(acm_elt->num_acms == 0) {
      //printk("HEAP_ACM element has no ACM addrs\n");
      //WARN
   }

   for(uint32_t i = 0 ; i < acm_elt->num_acms ; i++ ) {
      if(acm_elt->acm_addrs[i] == 0) {
         return false;
      }

      if(acm_elt->acm_addrs[i] >= 0x100000000UL) {
         return false;
      }

      // not going to check if ACM addrs are valid ACMs
   }

   return true;
}

static bool verify_custom_elt(const heap_ext_data_element_t * elt) {
   const heap_custom_elt_t * custom_elt = (const heap_custom_elt_t *) elt->data;

   // wrong size ?
   if(elt->size < sizeof(*elt) + sizeof(*custom_elt)) {
      return false;
   }

   // any values are allowed
   return true;
}

static bool verify_evt_log_ptr_elt(const heap_ext_data_element_t * elt) {
   const heap_event_log_ptr_elt_t * elog_elt = (const heap_event_log_ptr_elt_t *) elt->data;

   // wrong size ?
   if(elt->size != sizeof(*elt) + sizeof(*elog_elt)) {
      return false;
   }

   // not going to check LOG
   return true;
}

static bool verify_ext_data_elts(const heap_ext_data_element_t elts[], size_t elts_size) {
   const heap_ext_data_element_t * elt = elts;

   while(true) {
      // elements too small ?
      if(elts_size < sizeof(*elt)) {
         return false;
      }
      
      switch (elt->type) {
      case HEAP_EXTDATA_TYPE_END:
         return true;
         
      case HEAP_EXTDATA_TYPE_BIOS_SPEC_VER:
         if(! verify_bios_spec_ver_elt(elt)) {
            return false;
         }
         break;
         
      case HEAP_EXTDATA_TYPE_ACM:
         if(! verify_acm_elt(elt)) {
            return false;
         }
         break;
         
      case HEAP_EXTDATA_TYPE_CUSTOM:
         if(! verify_custom_elt(elt)) {
            return false;
         }
         break;
         
      case HEAP_EXTDATA_TYPE_TPM_EVENT_LOG_PTR:
         if(! verify_evt_log_ptr_elt(elt)) {
            return false;
         }
         break;
         
      default:
         // Unknown elemet
         break;
      }
      
      elts_size -= elt->size;
      elt = (const heap_ext_data_element_t *) ((uint8_t *) elt + elt->size);
   }
   
   return true;
}

static bool verify_bios_data(const txt_heap_t * heap_base, uint64_t heap_size) {
   uint64_t size;
   bios_data_t * bios_data;
   
   // verify that heap base/size are valid
   if(heap_base == 0 || heap_size == 0) {
      return false;
   }

   // check size
   size = txt_get_bios_data_size(heap_base);
   if(size == 0) {
      return false;
   }
   
   if(size > heap_size) {
      return false;
   }

   bios_data = txt_get_bios_data_start(heap_base);

   // check version
   if(bios_data->version < 2) {
      return false;
   }
   
   // we assume backwards compatibility but print a warning
   if(bios_data->version > 4) {
      //printk("unsupported BIOS data version (%u)\n", bios_data->version);
      //WARN
   }

   // all TXT-capable CPUs support at least 2 cores
   if(bios_data->num_logical_procs < 2) {
      return false;
   }

   if(bios_data->version >= 4) {
      if (! verify_ext_data_elts(bios_data->ext_data_elts, size - sizeof(*bios_data))) {
         return false;
      }
   }

   return true;
}

static bool verify_os_mle_data(const txt_heap_t * heap_base, uint64_t heap_size) {
   uint64_t size;

   // check size
   size = txt_get_os_mle_data_size(heap_base);
 
   if(size == 0) {
      return false;
   }
   
   if(size > heap_size) {
      return false;
   }
  
   // We don't know the size of os_mle_data
   
   return true;
}

static bool verify_os_sinit_data(const txt_heap_t * heap_base, uint64_t heap_size) {
   uint64_t size;
   os_sinit_data_t * os_sinit_data;

   // check size
   size = txt_get_os_sinit_data_size(heap_base);
   
   if(size == 0) {
      return false;
   }
   
   if(size > heap_size) {
      return false;
   }

   os_sinit_data = txt_get_os_sinit_data_start(heap_base);

   // check version (but since we create this, it should always be OK)
   if(os_sinit_data->version < MIN_OS_SINIT_DATA_VER || os_sinit_data->version > MAX_OS_SINIT_DATA_VER) {
      return false;
   }

   if(size != txt_calc_os_sinit_data_size(os_sinit_data->version)) {
      return false;
   }

   if(os_sinit_data->version >= 6) {
      if(! verify_ext_data_elts(os_sinit_data->ext_data_elts, size - sizeof(*os_sinit_data))) {
         return false;
      }
   }

   return true;
}

static bool verify_sinit_mle_data(const txt_heap_t * heap_base, uint64_t heap_size) {
   uint64_t size;
   sinit_mle_data_t * sinit_mle_data;

   // check size
   size = txt_get_sinit_mle_data_size(heap_base);
   
   if(size == 0) {
      return false;
   }
   
   if(size > heap_size) {
      return false;
   }

   // check version
   sinit_mle_data = txt_get_sinit_mle_data_start(heap_base);
   if(sinit_mle_data->version < 6) {
      return false;
   } else if(sinit_mle_data->version > 8) {
      //printk(TBOOT_WARN"unsupported SINIT to MLE data version (%u)\n", sinit_mle_data->version);
   }

   return true;
}

bool LIBTXTCALL txt_verify_heap(const txt_heap_t * heap_base, uint64_t heap_size, bool bios_data_only) {
   uint64_t size1;
   uint64_t size2;
   uint64_t size3;
   uint64_t size4;
   
   // verify BIOS to OS data
   if(! verify_bios_data(heap_base, heap_size)) {
      return false;
   }

   if(bios_data_only) {
      return true;
   }

   // check that total size is within the heap
   size1 = txt_get_bios_data_size(heap_base);
   size2 = txt_get_os_mle_data_size(heap_base);
   size3 = txt_get_os_sinit_data_size(heap_base);
   size4 = txt_get_sinit_mle_data_size(heap_base);

   // overflow?
   if(plus_overflow_u64(size1, size2)) {
      return false;
   }
   
   if(plus_overflow_u64(size3, size4)) {
      return false;
   }
   
   if(plus_overflow_u64(size1 + size2, size3 + size4)) {
      return false;
   }

   if((size1 + size2 + size3 + size4) > heap_size) {
      return false;
   }

   // verify OS to MLE data
   if(! verify_os_mle_data(heap_base, heap_size)) {
      return false;
   }

   // verify OS to SINIT data
   if(! verify_os_sinit_data(heap_base, heap_size)) {
      return false;
   }
   
   // verify SINIT to MLE data
   if(! verify_sinit_mle_data(heap_base, heap_size)) {
      return false;
   }

   return true;
}

