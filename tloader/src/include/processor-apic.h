#pragma once 

#define LAPIC_REG_ID			      0x20
#define LAPIC_REG_VER            0x30
#define LAPIC_REG_MAXLVT		   0x32
#define LAPIC_REG_TPR			   0x80
#define LAPIC_REG_APR			   0x90
#define LAPIC_REG_PPR			   0xa0
#define LAPIC_REG_EOI			   0xb0
#define LAPIC_REG_LDR			   0xd0
#define LAPIC_REG_DFR			   0xe0
#define LAPIC_REG_SPURIOUS	      0xf0
#define LAPIC_REG_ISR            0x100
#define LAPIC_REG_TMR            0x180
#define LAPIC_REG_IRR            0x200
#define LAPIC_REG_ESR	         0x280
#define LAPIC_REG_LVTCMCI        0x2f0
#define LAPIC_REG_ICR0	         0x300
#define LAPIC_REG_ICR1	         0x310
#define LAPIC_REG_LVTTIMER	      0x320
#define LAPIC_REG_LVTTHERM	      0x330
#define LAPIC_REG_LVTPERF		   0x340
#define LAPIC_REG_LVTLINT0	      0x350
#define LAPIC_REG_LVTLINT1	      0x360
#define LAPIC_REG_LVTERROR	      0x370
#define LAPIC_REG_TMICR       	0x380
#define LAPIC_REG_TMCCR   	      0x390
#define LAPIC_REG_TMDCR   	      0x3e0

// ID
#define LAPIC_ID_OFF_ID          24

// VERSION
#define LAPIC_VER_OFF_MAXLVT     16

// LVT
#define LAPIC_LVT_OFF_TMMODE     17
#define LAPIC_LVT_OFF_MASK       16
#define LAPIC_LVT_OFF_TRIGGER    15
#define LAPIC_LVT_OFF_RIRR       14
#define LAPIC_LVT_OFF_IIPP       13
#define LAPIC_LVT_OFF_DLVSTS     12
#define LAPIC_LVT_OFF_DLVMODE    8
#define LAPIC_LVT_OFF_VECTOR     0

#define LAPIC_LVT_VAL_ONESHORT   0
#define LAPIC_LVT_VAL_TMPERIOD   (1 << LAPIC_LVT_OFF_TMMODE)
#define LAPIC_LVT_VAL_UNMASK     0
#define LAPIC_LVT_VAL_MASK       (1 << LAPIC_LVT_OFF_MASK)
#define LAPIC_LVT_VAL_TRG_EDGE   0
#define LAPIC_LVT_VAL_TRG_LEVEL  (1 << LAPIC_LVT_OFF_TRIGGER)

#define LAPIC_LVT_VAL_DLV_FIXED  0
#define LAPIC_LVT_VAL_DLV_SMI    (2 << LAPIC_LVT_OFF_DLVMODE)
#define LAPIC_LVT_VAL_DLV_NMI    (4 << LAPIC_LVT_OFF_DLVMODE)
#define LAPIC_LVT_VAL_DLV_INIT   (5 << LAPIC_LVT_OFF_DLVMODE)
#define LAPIC_LVT_VAL_DLV_EXTINT (7 << LAPIC_LVT_OFF_DLVMODE)

// ICR
#define LAPIC_ICR1_OFF_DEST 		24
#define LAPIC_ICR0_OFF_DESTSORT 	18
#define LAPIC_ICR0_OFF_TRIGGER 	15
#define LAPIC_ICR0_OFF_LEVEL 		14
#define LAPIC_ICR0_OFF_DLVSTS 	12
#define LAPIC_ICR0_OFF_DESTMODE 	11
#define LAPIC_ICR0_OFF_DLVMODE 	8
#define LAPIC_ICR0_OFF_VECTOR 	0

#define LAPIC_ICR0_VAL_DST_NOSH    0
#define LAPIC_ICR0_VAL_DST_SELF    (1 << LAPIC_ICR0_OFF_DESTSORT)
#define LAPIC_ICR0_VAL_DST_ALL     (2 << LAPIC_ICR0_OFF_DESTSORT)
#define LAPIC_ICR0_VAL_DST_OTHER   (3 << LAPIC_ICR0_OFF_DESTSORT)

#define LAPIC_ICR0_VAL_DLV_FIXED    0
#define LAPIC_ICR0_VAL_DLV_LOW      (1 << LAPIC_ICR0_OFF_DLVMODE)
#define LAPIC_ICR0_VAL_DLV_SMI      (2 << LAPIC_ICR0_OFF_DLVMODE)
#define LAPIC_ICR0_VAL_DLV_NMI      (4 << LAPIC_ICR0_OFF_DLVMODE)
#define LAPIC_ICR0_VAL_DLV_INIT     (5 << LAPIC_ICR0_OFF_DLVMODE)
#define LAPIC_ICR0_VAL_DLV_SIPI     (6 << LAPIC_ICR0_OFF_DLVMODE)

#define LAPIC_ICR0_VAL_TRG_EDGE     0
#define LAPIC_ICR0_VAL_TRG_LEVEL    (1 << LAPIC_ICR0_OFF_TRIGGER)

#define LAPIC_ICR0_VAL_LVL_DEASSERT 0
#define LAPIC_ICR0_VAL_LVL_ASSERT   (1 << LAPIC_ICR0_OFF_LEVEL)

#define LAPIC_ICR0_VAL_DSTM_PHYS    0
#define LAPIC_ICR0_VAL_DSTM_LOG     (1 << LAPIC_ICR0_OFF_DESTMODE)

// LDR
#define LAPIC_LDR_OFF_LOCALAPICID   24
#define LAPIC_LDR_MASK        		(0xff << LAPIC_LDR_OFF_LOCALAPICID)

// ISR
#define LAPIC_ISR_NR                8

static inline bool apic_check_support(void) {
   return (cpuid_edx(1) & CPUID_1_EDX_APIC) != 0;
}

static inline bool apic_is_enable(void) {
   return (rdmsr(MSR_IA32_APIC_BASE) & MSR_IA32_APIC_BASE_ENABLE)!= 0;
}

static inline void apic_enable(void) {
   wrmsr(MSR_IA32_APIC_BASE, rdmsr(MSR_IA32_APIC_BASE) | MSR_IA32_APIC_BASE_ENABLE);
}

static inline void apic_disable(void) {
   wrmsr(MSR_IA32_APIC_BASE, rdmsr(MSR_IA32_APIC_BASE) & ~MSR_IA32_APIC_BASE_ENABLE);
}

static inline bool apic2_check_support(void) {
   return (cpuid_ecx(1) & CPUID_1_ECX_APIC2) != 0;
}

static inline bool apic2_is_enable(void) {
   uint64_t val = rdmsr(MSR_IA32_APIC_BASE);
   return (val & (MSR_IA32_APIC_BASE_APIC2_ENABLE | MSR_IA32_APIC_BASE_ENABLE)) == (MSR_IA32_APIC_BASE_APIC2_ENABLE | MSR_IA32_APIC_BASE_ENABLE);
}

static inline void apic2_enable(void) {
   wrmsr(MSR_IA32_APIC_BASE, rdmsr(MSR_IA32_APIC_BASE) | MSR_IA32_APIC_BASE_APIC2_ENABLE | MSR_IA32_APIC_BASE_ENABLE);
}

static inline void apic2_disable(void) {
   wrmsr(MSR_IA32_APIC_BASE, rdmsr(MSR_IA32_APIC_BASE) & ~(MSR_IA32_APIC_BASE_APIC2_ENABLE | MSR_IA32_APIC_BASE_ENABLE));
}

static inline uint64_t apic_get_base(void) {
   return MSR_IA32_APIC_BASE_BASE(rdmsr(MSR_IA32_APIC_BASE));
}

/*static inline void apic_enablemode(void) {
   // useless if we have a virtual wire mode
   outb(0x22, 0x70);
   outb(0x23, 0x01);
}

static inline void apic_disablemode(void) {
   // useless if we have a virtual wire mode
   outb(0x22, 0x70);
   outb(0x23, 0x00);
}*/

/////////////////////////////////////////////////////////////////////

static inline uint32_t apic_get_id(uint8_t * baseapic) {
   uint32_t v = *((uint32_t *) &baseapic[LAPIC_REG_ID]);
   return (v >> LAPIC_ID_OFF_ID) & 0xff;
}

static inline uint32_t apic_is_integrated(uint8_t * baseapic) {
   uint32_t v = *((uint32_t *) &baseapic[LAPIC_REG_VER]);
   
   if(v & 0xf0) {
      // Integrated apic
      return 1;
   } else {
      // 82489DX external APIC
      return 0;
   }
}

static inline uint32_t apic_get_maxlvt(uint8_t * baseapic) {
   uint32_t v = *((uint32_t *) &baseapic[LAPIC_REG_VER]);
   
   if(v & 0xf0) {
      // Integrated apic
      return (v >> LAPIC_VER_OFF_MAXLVT) & 0xff;
   } else {
      // 82489DX external APIC
      return 2;
   }
}

static inline uint32_t apic_get_version(uint8_t * baseapic) {
   return (*((uint32_t *) &baseapic[LAPIC_REG_VER])) & 0xff;
}

static inline void apic_clear(uint8_t * baseapic) {
   uint32_t maxlvt = apic_get_maxlvt(baseapic);
   
   // (Cf. Xen source code) Work around AMD Erratum 411
   *((uint32_t *) &baseapic[LAPIC_REG_TMICR]) = 0;
   
   // (Cf. Xen source code) Masking an LVT entry on a P6 can trigger a local APIC error
   // if the vector is zero. Mask LVTERR first to prevent this.
   if(maxlvt >= 3) {      
      *((uint32_t *) &baseapic[LAPIC_REG_LVTERROR]) |= LAPIC_LVT_VAL_MASK;
   }
   
   // (Cf. Xen source code) Careful: we have to set masks only first to deassert
   //
   *((uint32_t *) &baseapic[LAPIC_REG_LVTTIMER]) |= LAPIC_LVT_VAL_MASK;
   *((uint32_t *) &baseapic[LAPIC_REG_LVTLINT0]) |= LAPIC_LVT_VAL_MASK;
   *((uint32_t *) &baseapic[LAPIC_REG_LVTLINT1]) |= LAPIC_LVT_VAL_MASK;
   
   if(maxlvt >= 4) {
      *((uint32_t *) &baseapic[LAPIC_REG_LVTPERF]) |= LAPIC_LVT_VAL_MASK;
   }
   
   /*if(maxlvt >= 5) {
      *((uint32_t *) &baseapic[LAPIC_REG_LVTTHERM]) |= LAPIC_LVT_VAL_MASK;
   }*/
   
   if(maxlvt >= 6) {
      *((uint32_t *) &baseapic[LAPIC_REG_LVTCMCI]) |= LAPIC_LVT_VAL_MASK;
   }

   // APIC state
   *((uint32_t *) &baseapic[LAPIC_REG_LVTTIMER]) = LAPIC_LVT_VAL_MASK;
   *((uint32_t *) &baseapic[LAPIC_REG_LVTLINT0]) = LAPIC_LVT_VAL_MASK;
   *((uint32_t *) &baseapic[LAPIC_REG_LVTLINT1]) = LAPIC_LVT_VAL_MASK;
   
   if(maxlvt >= 3) {      
      *((uint32_t *) &baseapic[LAPIC_REG_LVTERROR]) = LAPIC_LVT_VAL_MASK;
   }

   if(maxlvt >= 4) {
      *((uint32_t *) &baseapic[LAPIC_REG_LVTPERF]) = LAPIC_LVT_VAL_MASK;
   }
   
   /*if(maxlvt >= 5) {
      *((uint32_t *) &baseapic[LAPIC_REG_LVTTHERM]) = LAPIC_LVT_VAL_MASK;
   }*/
   
   if(maxlvt >= 6) {
      *((uint32_t *) &baseapic[LAPIC_REG_LVTCMCI]) = LAPIC_LVT_VAL_MASK;
   }
   
   
   if(apic_is_integrated(baseapic)) {
      uint32_t fake __attribute__((unused));
      
      if(maxlvt > 3) {
         // Due to Pentium errata 3AP and 11AP.
         *((uint32_t *) &baseapic[LAPIC_REG_ESR]) = 0;
      }
      
      fake = *((uint32_t *) &baseapic[LAPIC_REG_ESR]);
   }
}

/////////////////////////////////////////////////////////////////////

static inline void apic_eoi(uint8_t * baseapic) {
   *((uint32_t *) &baseapic[LAPIC_REG_EOI]) = 0;
}

static inline void send_ipi(uint8_t * baseapic, uint32_t high, uint32_t low) {
   uintptr_t flags;
   
   cpu_irq_save(&flags);
   
   while(*((uint32_t *) &baseapic[LAPIC_REG_ICR0]) & (1 << LAPIC_ICR0_OFF_DLVSTS)) {
      pause();
   }
   
   *((uint32_t *) &baseapic[LAPIC_REG_ICR1]) = high;
   *((uint32_t *) &baseapic[LAPIC_REG_ICR0]) = low;
   
   cpu_irq_restore(flags);
}

static inline void apic_send_ipi_tophys(uint8_t * baseapic, uint8_t dest, uint32_t type) {
   uint32_t high = dest << LAPIC_ICR1_OFF_DEST;
   uint32_t low = LAPIC_ICR0_VAL_DST_NOSH | LAPIC_ICR0_VAL_DSTM_PHYS | type;
   send_ipi(baseapic, high, low);
}

static inline void apic_send_ipi_self(uint8_t * baseapic, uint16_t type) {
   uint32_t high = 0x0f000000;
   uint32_t low = LAPIC_ICR0_VAL_DST_SELF | type;
   send_ipi(baseapic, high, low);
}

static inline void apic_send_ipi_other(uint8_t * baseapic, uint16_t type) {
   uint32_t high = 0x00000000;
   uint32_t low = LAPIC_ICR0_VAL_DST_OTHER | type;
   send_ipi(baseapic, high, low);
}

static inline void apic_send_ipi_all(uint8_t * baseapic, uint16_t type) {
   uint32_t high = 0x0f000000;
   uint32_t low = LAPIC_ICR0_VAL_DST_ALL | type;
   send_ipi(baseapic, high, low);
}

static inline uint16_t apic_prepare_ipi_init(void) {
   return LAPIC_ICR0_VAL_TRG_EDGE | LAPIC_ICR0_VAL_LVL_ASSERT | LAPIC_ICR0_VAL_DLV_INIT;
}

static inline uint16_t apic_prepare_ipi_sipi(uint8_t vector) {
   return LAPIC_ICR0_VAL_TRG_EDGE | LAPIC_ICR0_VAL_LVL_ASSERT | LAPIC_ICR0_VAL_DLV_SIPI | vector;
}

static inline uint16_t apic_prepare_ipi_fixed(uint8_t vector) {
   return LAPIC_ICR0_VAL_TRG_EDGE | LAPIC_ICR0_VAL_LVL_ASSERT | LAPIC_ICR0_VAL_DLV_FIXED | vector;
}

