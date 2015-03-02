#pragma once

#include <stdint-gcc.h>
#include <stdbool.h>

static inline void outb(unsigned short port, unsigned char data)
{
	__asm __volatile("outb %0, %w1" : : "a" (data), "Nd" (port));
}

static inline unsigned char inb(unsigned short port)
{
	unsigned char	data;

	__asm volatile("inb %w1, %0" : "=a" (data) : "Nd" (port));
	return (data);
}

/*
 * EFLAGS bits
 */
#define X86_EFLAGS_CF	0x00000001 /* Carry Flag */
#define X86_EFLAGS_PF	0x00000004 /* Parity Flag */
#define X86_EFLAGS_AF	0x00000010 /* Auxillary carry Flag */
#define X86_EFLAGS_ZF	0x00000040 /* Zero Flag */
#define X86_EFLAGS_SF	0x00000080 /* Sign Flag */
#define X86_EFLAGS_TF	0x00000100 /* Trap Flag */
#define X86_EFLAGS_IF	0x00000200 /* Interrupt Flag */
#define X86_EFLAGS_DF	0x00000400 /* Direction Flag */
#define X86_EFLAGS_OF	0x00000800 /* Overflow Flag */
#define X86_EFLAGS_IOPL	0x00003000 /* IOPL mask */
#define X86_EFLAGS_NT	0x00004000 /* Nested Task */
#define X86_EFLAGS_RF	0x00010000 /* Resume Flag */
#define X86_EFLAGS_VM	0x00020000 /* Virtual Mode */
#define X86_EFLAGS_AC	0x00040000 /* Alignment Check */
#define X86_EFLAGS_VIF	0x00080000 /* Virtual Interrupt Flag */
#define X86_EFLAGS_VIP	0x00100000 /* Virtual Interrupt Pending */
#define X86_EFLAGS_ID	0x00200000 /* CPUID detection flag */

/*
 * Bits in 386 special registers:
 */
#define CR0_PE  0x00000001 /* Protected mode Enable */
#define CR0_MP  0x00000002 /* "Math" (fpu) Present */
#define CR0_EM  0x00000004 /* EMulate FPU instructions. (trap ESC only) */
#define CR0_TS  0x00000008 /* Task Switched (if MP, trap ESC and WAIT) */
#define CR0_PG  0x80000000 /* PaGing enable */

/*
 * Bits in 486 special registers:
 */
#define CR0_NE  0x00000020 /* Numeric Error enable (EX16 vs IRQ13) */
#define CR0_WP  0x00010000 /* Write Protect (honor page protect in all modes) */
#define CR0_AM  0x00040000 /* Alignment Mask (set to enable AC flag) */
#define CR0_NW  0x20000000 /* Not Write-through */
#define CR0_CD  0x40000000 /* Cache Disable */

/*
 * Bits in PPro special registers
 */
#define CR4_VME 0x00000001 /* Virtual 8086 mode extensions */
#define CR4_PVI 0x00000002 /* Protected-mode virtual interrupts */
#define CR4_TSD 0x00000004 /* Time stamp disable */
#define CR4_DE  0x00000008 /* Debugging extensions */
#define CR4_PSE 0x00000010 /* Page size extensions */
#define CR4_PAE 0x00000020 /* Physical address extension */
#define CR4_MCE 0x00000040 /* Machine check enable */
#define CR4_PGE 0x00000080 /* Page global enable */
#define CR4_PCE 0x00000100 /* Performance monitoring counter enable */
#define CR4_FXSR 0x00000200/* Fast FPU save/restore used by OS */
#define CR4_XMM 0x00000400 /* enable SIMD/MMX2 to use except 16 */
#define CR4_VMXE 0x00002000/* enable VMX */
#define CR4_SMXE 0x00004000/* enable SMX */

static inline uintptr_t read_cr0(void)
{
    uintptr_t data;
    __asm__ __volatile__ ("mov %%cr0,%0" : "=r" (data));
    return (data);
}
static inline void write_cr0(uintptr_t data)
{
    __asm__ __volatile__("mov %0,%%cr0" : : "r" (data));
}

static inline uintptr_t read_cr4(void)
{
    uintptr_t data;
    __asm__ __volatile__ ("mov %%cr4,%0" : "=r" (data));
    return (data);
}
static inline void write_cr4(uintptr_t data)
{
    __asm__ __volatile__ ("mov %0,%%cr4" : : "r" (data));
}

static inline uintptr_t read_cr3(void)
{
    uintptr_t data;
    __asm__ __volatile__ ("mov %%cr3,%0" : "=r" (data));
    return (data);
}
static inline void write_cr3(uintptr_t data)
{
    __asm__ __volatile__("mov %0,%%cr3" : : "r" (data) : "memory");
}


static inline uintptr_t read_eflags(void)
{
    uintptr_t ef;
    __asm__ __volatile__ ("pushfl; popl %0" : "=r" (ef));
    return (ef);
}
static inline void write_eflags(uintptr_t ef)
{
    __asm__ __volatile__ ("pushl %0; popfl" : : "r" (ef));
}


static inline void cli(void)
{
    __asm__ __volatile__ ("cli" : : : "memory");
}
static inline void sti(void)
{
    __asm__ __volatile__ ("sti");
}

static inline void pause(void)
{
    __asm__ __volatile__ ("pause");
}


static inline void halt(void)
{
    __asm__ __volatile__ ("hlt");
}

static inline uint64_t rdtsc(void)
{
	uint64_t rv;

	__asm__ __volatile__ ("rdtsc" : "=A" (rv));
	return (rv);
}

static inline void wbinvd(void)
{
    __asm__ __volatile__ ("wbinvd");
}

static inline uintptr_t bsrl(uintptr_t mask)
{
    uintptr_t   result;

    __asm__ __volatile__ ("bsr %1,%0" : "=r" (result) : "rm" (mask) : "cc");
    return (result);
}

static inline void monitor(const void *addr, int extensions, int hints)
{
    __asm __volatile__ ("monitor;" : :"a" (addr), "c" (extensions), "d"(hints));
}

static inline void mwait(int extensions, int hints)
{
    __asm __volatile__ ("mwait;" : :"a" (hints), "c" (extensions));
}

static inline void cpu_irq_save(uintptr_t * flags)
{
   #if ARCHI == 32
   __asm__ __volatile__ ("pushfl ; popl %0" : "=g" (*flags));
   #elif ARCHI == 64
   __asm__ __volatile__ ("pushfq ; popq %0" : "=g" (*flags));
   #endif
   cli();
}

static inline void cpu_irq_restore(uintptr_t flags)
{
   #if ARCHI == 32
   __asm__ __volatile__ ("pushl %0 ; popfl" : : "g" (flags) : "memory", "cc" );
   #elif ARCHI == 64
   __asm__ __volatile__ ("pushq %0 ; popfq" : : "g" (flags) : "memory", "cc" );
   #endif
}

#include "processor-cpuid.h"
#include "processor-msr.h"
#include "processor-apic.h"

static inline bool cpu_isintel(void)
{
   uint32_t regs[4];
	
	do_cpuid(0, regs);
	if((regs[1] == 0x756e6547) && (regs[2] == 0x6c65746e) && (regs[3] == 0x49656e69))
	   return true;
	return false;
}

static inline bool cpu_isamd(void)
{
   uint32_t regs[4];
	
	do_cpuid(0, regs);
	if((regs[1] == 0x68747541) && (regs[2] == 0x444d4163) && (regs[3] == 0x69746e65))
	   return true;
	return false;
}

static inline bool cpu_isbsp(void) 
{
   return (rdmsr(MSR_IA32_APIC_BASE) & MSR_IA32_APIC_BASE_BSP) != 0;
}

static inline uint32_t cpu_getid(void) 
{
   return (cpuid_ebx(1) >> 24) & 0xff;
}

static inline uint32_t cpu_getnb(void)
{
	uint32_t regs[4];
	
	do_cpuid(0, regs);
	if((regs[1] == 0x68747541) && (regs[2] == 0x444d4163) && (regs[3] == 0x69746e65)) {          // 'Auth' 'cAMD' 'enti'
	   //uint32_t features = cpuid_edx(1);
	   uint32_t logical = (cpuid_ebx(1) >> 16) & 0xff;
	   //uint32_t cores = logical;
	   
	   //cores = (cpuid_ecx(0x80000008) & 0xff) + 1;
	   
	   return logical;   
	
	} else if((regs[1] == 0x756e6547) && (regs[2] == 0x6c65746e) && (regs[3] == 0x49656e69)) {   // 'Genu' 'ntel' 'ineI'
	   //uint32_t features = cpuid_edx(1);
	   uint32_t logical = (cpuid_ebx(1) >> 16) & 0xff;
	   //uint32_t cores = logical;
	   
	   //__asm__ __volatile__ ("xor %ecx, %ecx");
	   //cores = ((cpuid_eax(4) >> 26) & 0x3f) + 1;
	   __asm__ __volatile__ ("xor %ecx, %ecx");
	   logical = cpuid_eax(4);
	   if(logical & 0x1f) {
		   logical = (logical >> 26) + 1;
	   } else {
		   logical = 1;
		}

	   return logical;
	}
	
	return 0;
}
