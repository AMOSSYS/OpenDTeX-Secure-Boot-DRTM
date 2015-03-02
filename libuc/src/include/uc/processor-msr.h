#pragma once

static inline uint64_t rdmsr(uint32_t msr)
{
    uint64_t rv;

    __asm__ __volatile__ ("rdmsr" : "=A" (rv) : "c" (msr));
    return (rv);
}

static inline void wrmsr(uint32_t msr, uint64_t newval)
{
    __asm__ __volatile__ ("wrmsr" : : "A" (newval), "c" (msr));
}

// MSRs bits for local APIC
#define MSR_IA32_APIC_BASE                     0x1b
#define MSR_IA32_APIC_BASE_BSP                 (1ULL<<8)
#define MSR_IA32_APIC_BASE_APIC2_ENABLE        (1ULL<<10)
#define MSR_IA32_APIC_BASE_ENABLE              (1ULL<<11)
#define MSR_IA32_APIC_BASE_BASE(x)             (x & 0xffffff000ULL)

#define MSR_IA32_PLATFORM_ID                   0x017
#define MSR_IA32_FEATURE_CONTROL               0x03a
#define MSR_IA32_SMM_MONITOR_CTL               0x09b
#define MSR_MTRRcap                            0x0fe
#define MSR_IA32_SYSENTER_CS                   0x174
#define MSR_IA32_SYSENTER_ESP                  0x175
#define MSR_IA32_SYSENTER_EIP                  0x176

#define MSR_MCG_CAP                            0x179
#define MSR_MCG_STATUS                         0x17a
#define MSR_IA32_MISC_ENABLE                   0x1a0
#define MSR_IA32_MISC_ENABLE_MONITOR_FSM       (1<<18)
#define MSR_MTRRdefType                        0x2ff
#define MSR_MC0_STATUS                         0x401
#define MSR_IA32_VMX_BASIC_MSR                 0x480
#define MSR_IA32_VMX_PINBASED_CTLS_MSR         0x481
#define MSR_IA32_VMX_PROCBASED_CTLS_MSR        0x482
#define MSR_IA32_VMX_EXIT_CTLS_MSR             0x483
#define MSR_IA32_VMX_ENTRY_CTLS_MSR            0x484
#define MSR_IA32_VMX_MISC                      0x485
#define MSR_IA32_VMX_CR0_FIXED0                0x486
#define MSR_IA32_VMX_CR0_FIXED1                0x487
#define MSR_IA32_VMX_CR4_FIXED0                0x488
#define MSR_IA32_VMX_CR4_FIXED1                0x489
#define MSR_IA32_VMX_VMCS_ENUM                 0x48a
#define MSR_IA32_VMX_PROCBASED_CTLS2           0x48b


/*
 * Constants related to MSR's.
 */
#define APICBASE_BSP                                  0x00000100

#define MSR_IA32_SMM_MONITOR_CTL_VALID                1
#define MSR_IA32_SMM_MONITOR_CTL_MSEG_BASE(x)         (x>>12)

/* MSRs & bits used for VMX enabling */
#define IA32_FEATURE_CONTROL_MSR_LOCK                 0x1
#define IA32_FEATURE_CONTROL_MSR_ENABLE_VMX_IN_SMX    0x2
#define IA32_FEATURE_CONTROL_MSR_SENTER_PARAM_CTL     0x7f00
#define IA32_FEATURE_CONTROL_MSR_ENABLE_SENTER        0x8000

/* AMD64 MSR's */
#define MSR_EFER        0xc0000080      /* extended features */

/* EFER bits */
#define _EFER_LME     8               /* Long mode enable */

#define MTRR_TYPE_UNCACHABLE     0
#define MTRR_TYPE_WRTHROUGH      4
#define MTRR_TYPE_WRBACK         6

