#pragma once

#include <uc/types.h>
#include <uc/processor.h>
#include <txt/txt.h>

static inline bool support_smx(void) {
   uint32_t features = cpuid_ecx(1);
   
   if(! (features & CPUID_1_ECX_SMX)) {
      return false;
   }
   
   return true;
}

static inline bool is_in_drtm(void) {
   txt_sts_t sts;
   
   if(! (read_cr4() & CR4_SMXE)) {
      return false;
   }
   
   sts._raw = txt_read_config_reg((uint8_t *) TXT_PUB_CONFIG_REGS_BASE, TXTCR_STS);
   if(sts.senter_done_sts == 0) {
      return false;
   }
   
   
   return true;
}

