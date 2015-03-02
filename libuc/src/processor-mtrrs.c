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


#include <uc/processor.h>

#define PAGE_SHIFT   12
#define PAGE_SIZE    (1 << PAGE_SHIFT)
#define PAGE_MASK    (~(PAGE_SIZE - 1))
#define PAGE_UP(p)   (((unsigned long)(p) + PAGE_SIZE - 1) & PAGE_MASK)

// SINIT requires 36b mask
#define SINIT_MTRR_MASK 0xFFFFFF

static inline int fls(int mask) {
   return (mask == 0 ? mask : (int) bsrl((uintptr_t)mask) + 1);
}

bool UCCALL set_mem_type(uint64_t base, uint32_t size, uint32_t mem_type) {
   int               num_pages;
   int               ndx;
   mtrr_def_type_t   mtrr_def_type;
   mtrr_cap_t        mtrr_cap;
   mtrr_physmask_t   mtrr_physmask;
   mtrr_physbase_t   mtrr_physbase;

   // disable all fixed MTRRs
   // set default type to UC
   mtrr_def_type.raw = rdmsr(MSR_MTRRdefType);
   mtrr_def_type.fe = 0;
   mtrr_def_type.type = MTRR_TYPE_UNCACHABLE;
   wrmsr(MSR_MTRRdefType, mtrr_def_type.raw);

   // initially disable all variable MTRRs (we'll enable the ones we use)
   mtrr_cap.raw = rdmsr(MSR_MTRRcap);
   for(ndx = 0 ; ndx < mtrr_cap.vcnt ; ndx++) {
      mtrr_physmask.raw = rdmsr(MTRR_PHYS_MASK0_MSR + ndx * 2);
      mtrr_physmask.v = 0;
      wrmsr(MTRR_PHYS_MASK0_MSR + ndx * 2, mtrr_physmask.raw);
   }

   // map all AC module pages as mem_type
   num_pages = PAGE_UP(size) >> PAGE_SHIFT;
   ndx = 0;

   while(num_pages > 0) {
      uint32_t pages_in_range;

      // set the base of the current MTRR
      mtrr_physbase.raw = rdmsr(MTRR_PHYS_BASE0_MSR + ndx * 2);
      mtrr_physbase.base = ((unsigned long) base >> PAGE_SHIFT) & SINIT_MTRR_MASK;
      mtrr_physbase.type = mem_type;
      wrmsr(MTRR_PHYS_BASE0_MSR + ndx * 2, mtrr_physbase.raw);

      /*
      * calculate MTRR mask
      * MTRRs can map pages in power of 2
      * may need to use multiple MTRRS to map all of region
      */
      pages_in_range = 1 << (fls(num_pages) - 1);

      mtrr_physmask.raw = rdmsr(MTRR_PHYS_MASK0_MSR + ndx * 2);
      mtrr_physmask.mask = ~(pages_in_range - 1) & SINIT_MTRR_MASK;
      mtrr_physmask.v = 1;
      wrmsr(MTRR_PHYS_MASK0_MSR + ndx * 2, mtrr_physmask.raw);

      /* prepare for the next loop depending on number of pages
      * We figure out from the above how many pages could be used in this
      * mtrr. Then we decrement the count, increment the base,
      * increment the mtrr we are dealing with, and if num_pages is
      * still not zero, we do it again.
      */
      base += (pages_in_range * PAGE_SIZE);
      num_pages -= pages_in_range;
      ndx++;

      if(ndx == mtrr_cap.vcnt) {
         return false;
      }
   }

   return true;
}

// enable/disable all MTRRs
void UCCALL set_all_mtrrs(bool enable) {
   mtrr_def_type_t mtrr_def_type;

   mtrr_def_type.raw = rdmsr(MSR_MTRRdefType);
   mtrr_def_type.e = enable ? 1 : 0;
   wrmsr(MSR_MTRRdefType, mtrr_def_type.raw);
}

void UCCALL save_mtrrs(mtrr_state_t * saved_state) {
   mtrr_cap_t mtrr_cap;
   int        ndx;

   // IA32_MTRR_DEF_TYPE MSR
   saved_state->mtrr_def_type.raw = rdmsr(MSR_MTRRdefType);

   // number variable MTTRRs
   mtrr_cap.raw = rdmsr(MSR_MTRRcap);
   if(mtrr_cap.vcnt > MAX_VARIABLE_MTRRS) {
      saved_state->num_var_mtrrs = MAX_VARIABLE_MTRRS;
   } else {
      saved_state->num_var_mtrrs = (int) mtrr_cap.vcnt;
   }

   // physmask's and physbase's
   for(ndx = 0 ; ndx < saved_state->num_var_mtrrs ; ndx++) {
      saved_state->mtrr_physmasks[ndx].raw = rdmsr(MTRR_PHYS_MASK0_MSR + ndx * 2);
      saved_state->mtrr_physbases[ndx].raw = rdmsr(MTRR_PHYS_BASE0_MSR + ndx * 2);
   }
}

void UCCALL restore_mtrrs(const mtrr_state_t * saved_state) {
   int ndx;

   if(saved_state == NULL) {
      return;
   }

   // disable all MTRRs first
   set_all_mtrrs(false);

   // physmask's and physbase's
   for(ndx = 0 ; ndx < saved_state->num_var_mtrrs ; ndx++) {
      wrmsr(MTRR_PHYS_MASK0_MSR + ndx * 2, saved_state->mtrr_physmasks[ndx].raw);
      wrmsr(MTRR_PHYS_BASE0_MSR + ndx * 2, saved_state->mtrr_physbases[ndx].raw);
   }

   // IA32_MTRR_DEF_TYPE MSR
   wrmsr(MSR_MTRRdefType, saved_state->mtrr_def_type.raw);
}

bool UCCALL set_mtrrs_for_acmod(uint64_t sinit, uint64_t size) {
   uintptr_t eflags;
   uintptr_t cr0, cr4;
   bool bRes = true;

   // disable interruptssinit
   eflags = read_eflags();
   cli();

   // save CR0 then disable cache (CRO.CD=1, CR0.NW=0)
   cr0 = read_cr0();
   write_cr0((cr0 & ~CR0_NW) | CR0_CD);

   // flush caches
   wbinvd();

   // save CR4 and disable global pages (CR4.PGE=0)
   cr4 = read_cr4();
   write_cr4(cr4 & ~CR4_PGE);

   // disable MTRRs
   set_all_mtrrs(false);

   // now set MTRRs for AC mod and rest of memory
   if(! set_mem_type(sinit, size, MTRR_TYPE_WRBACK)) {
      bRes = false;
   }

   // flush caches
   wbinvd();

   // enable MTRRs
   set_all_mtrrs(true);

   // restore CR0 (cacheing)
   write_cr0(cr0);

   // restore CR4 (global pages)
   write_cr4(cr4);

   // restore eflags
   write_eflags(eflags);

   return bRes;
}

bool UCCALL validate_mtrrs(const mtrr_state_t * saved_state) {
   mtrr_cap_t mtrr_cap;
   int ndx;

   // check is meaningless if MTRRs were disabled
   if(saved_state->mtrr_def_type.e == 0) {
      return true;
   }

   // number variable MTRRs
   mtrr_cap.raw = rdmsr(MSR_MTRRcap);
   if(mtrr_cap.vcnt < saved_state->num_var_mtrrs) {
      return false;
   }

   // variable MTRRs describing non-contiguous memory regions
   // TBD: assert(MAXPHYADDR == 36);
   for(ndx = 0 ; ndx < saved_state->num_var_mtrrs ; ndx++) {
      uint64_t tb;

      if(saved_state->mtrr_physmasks[ndx].v == 0) {
         continue;
      }

      for(tb = 0x1 ; tb != 0x1000000 ; tb = tb << 1) {
         if((tb & saved_state->mtrr_physmasks[ndx].mask) != 0) {
            break;
         }
      }

      for( ; tb != 0x1000000 ; tb = tb << 1) {
         if((tb & saved_state->mtrr_physmasks[ndx].mask) == 0) {
            break;
         }
      }

      if(tb != 0x1000000) {
         return false;
      }
   }

   // overlaping regions with invalid memory type combinations
   for(ndx = 0 ; ndx < saved_state->num_var_mtrrs ; ndx++) {
      int i;
      const mtrr_physbase_t * base_ndx = &saved_state->mtrr_physbases[ndx];
      const mtrr_physmask_t * mask_ndx = &saved_state->mtrr_physmasks[ndx];

      if(mask_ndx->v == 0) {
         continue;
      }

      for(i = ndx + 1 ; i < saved_state->num_var_mtrrs ; i++) {
         int j;
         const mtrr_physbase_t * base_i = &saved_state->mtrr_physbases[i];
         const mtrr_physmask_t * mask_i = &saved_state->mtrr_physmasks[i];

         if(mask_i->v == 0) {
            continue;
         }

         if((base_ndx->base & mask_ndx->mask & mask_i->mask)
         != (base_i->base & mask_i->mask)
         && (base_i->base & mask_i->mask & mask_ndx->mask)
         != (base_ndx->base & mask_ndx->mask)) {
            continue;
         }

         if(base_ndx->type == base_i->type) {
            continue;
         }

         if(base_ndx->type == MTRR_TYPE_UNCACHABLE
         || base_i->type == MTRR_TYPE_UNCACHABLE) {
            continue;
         }

         if(base_ndx->type == MTRR_TYPE_WRTHROUGH
         && base_i->type == MTRR_TYPE_WRBACK) {
            continue;
         }

         if(base_ndx->type == MTRR_TYPE_WRBACK
         && base_i->type == MTRR_TYPE_WRTHROUGH) {
            continue;
         }

         // 2 overlapped regions have invalid mem type combination,
         // need to check whether there is a third region which has type
         // of UNCACHABLE and contains at least one of these two regions.
         // If there is, then the combination of these 3 region is valid
         for(j = 0 ; j < saved_state->num_var_mtrrs ; j++ ) {
            const mtrr_physbase_t * base_j = &saved_state->mtrr_physbases[j];
            const mtrr_physmask_t * mask_j = &saved_state->mtrr_physmasks[j];

            if(mask_j->v == 0) {
               continue;
            }

            if(base_j->type != MTRR_TYPE_UNCACHABLE) {
               continue;
            }

            if((base_ndx->base & mask_ndx->mask & mask_j->mask)
            == (base_j->base & mask_j->mask)
            && (mask_j->mask & ~mask_ndx->mask) == 0) {
               break;
            }

            if((base_i->base & mask_i->mask & mask_j->mask)
            == (base_j->base & mask_j->mask)
            && (mask_j->mask & ~mask_i->mask) == 0) {
               break;
            }
         }

         if(j < saved_state->num_var_mtrrs) {
            continue;
         }

         return false;
      }
   }

   return true;
}

