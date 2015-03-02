#pragma once

static inline void do_cpuid(uint32_t ax, uint32_t p[4])
{
    __asm__ __volatile__ ("cpuid"
                          : "=a" (p[0]), "=b" (p[1]), "=c" (p[2]), "=d" (p[3])
                          :  "0" (ax));
}

static inline uint32_t cpuid_eax(uint32_t op)
{
     /* eax: regs[0], ebx: regs[1], ecx: regs[2], edx: regs[3] */
    uint32_t regs[4];

    do_cpuid(op, regs);

    return regs[0];
}

static inline uint32_t cpuid_ebx(uint32_t op)
{
     /* eax: regs[0], ebx: regs[1], ecx: regs[2], edx: regs[3] */
    uint32_t regs[4];

    do_cpuid(op, regs);

    return regs[1];
}

static inline uint32_t cpuid_ecx(uint32_t op)
{
     /* eax: regs[0], ebx: regs[1], ecx: regs[2], edx: regs[3] */
    uint32_t regs[4];

    do_cpuid(op, regs);

    return regs[2];
}

static inline uint32_t cpuid_edx(uint32_t op)
{
     /* eax: regs[0], ebx: regs[1], ecx: regs[2], edx: regs[3] */
    uint32_t regs[4];

    do_cpuid(op, regs);

    return regs[3];
}

#define CPUID_1_ECX_XMM3         (1<<0)
#define CPUID_1_ECX_VMX          (1<<5)
#define CPUID_1_ECX_SMX          (1<<6)
#define CPUID_1_ECX_CMPXCHG16B   (1<<13)
#define CPUID_1_ECX_APIC2        (1<<21)

#define CPUID_1_EDX_PSE          (1<<3)
#define CPUID_1_EDX_MSR          (1<<5)
#define CPUID_1_EDX_PAE          (1<<6)
#define CPUID_1_EDX_CMPXCHG8B    (1<<8)
#define CPUID_1_EDX_APIC         (1<<9)
#define CPUID_1_EDX_SEP          (1<<11)
#define CPUID_1_EDX_PGE          (1<<13)
#define CPUID_1_EDX_PSE36        (1<<17)
#define CPUID_1_EDX_HTT          (1<<28)

