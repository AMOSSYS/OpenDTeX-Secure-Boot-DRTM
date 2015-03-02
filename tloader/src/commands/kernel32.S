/* The code segment of the protected mode.  */
#define CODE_SEGMENT	0x10

/* The data segment of the protected mode.  */
#define DATA_SEGMENT	0x18

#include <tloader.h>

   .globl relocator32_esp
   .globl relocator32_ebp
   .globl relocator32_esi
   .globl relocator32_edi
   .globl relocator32_eax
   .globl relocator32_ebx
   .globl relocator32_ecx
   .globl relocator32_edx
   .globl relocator32_eip

	.p2align	4	/* force 16-byte alignment */
   .code32
   
ENTRY(relocator32_start)

   cli
   
   // Reload GDT	
	leal  gdt, %eax
	mov   %eax, gdt_addr
	lgdt  gdtdesc
	ljmp	$CODE_SEGMENT, $(1f)
1:	

	/* Update other registers. */
	movl	$DATA_SEGMENT, %eax
	movl	%eax, %ds
	movl	%eax, %es
	movl	%eax, %fs
	movl	%eax, %gs
	movl	%eax, %ss

   // Disable paging
	movl	%cr0, %eax
	andl	$0x7fffffff, %eax
	movl	%eax, %cr0

	/* Turn off PAE. */
	movl	%cr4, %eax
	andl	$0xffffffdf, %eax
	movl	%eax, %cr4

	jmp	cont2
cont2:

	.code32

	/* mov imm32, %eax */
	.byte	0xb8
relocator32_esp:
	.long	0

	movl	%eax, %esp

	/* mov imm32, %eax */
	.byte	0xb8
relocator32_ebp:
	.long	0

	movl	%eax, %ebp

	/* mov imm32, %eax */
	.byte	0xb8
relocator32_esi:
	.long	0

	movl	%eax, %esi

	/* mov imm32, %eax */
	.byte	0xb8
relocator32_edi:
	.long	0

	movl	%eax, %edi

	/* mov imm32, %eax */
	.byte	0xb8
relocator32_eax:
	.long	0

	/* mov imm32, %ebx */
	.byte	0xbb
relocator32_ebx:
	.long	0

	/* mov imm32, %ecx */
	.byte	0xb9
relocator32_ecx:
	.long	0

	/* mov imm32, %edx */
	.byte	0xba
relocator32_edx:
	.long	0

	/* Cleared direction flag is of no problem with any current
	   payload and makes this implementation easier.  */
	cld

	.byte	0xea
relocator32_eip:
	.long	0
	.word	CODE_SEGMENT

	/* GDT. Copied from loader/i386/linux.c. */
	.p2align	4
gdt:
	/* NULL.  */
	.byte 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

	/* Reserved.  */
	.byte 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

	/* Code segment.  */
	.byte 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x9A, 0xCF, 0x00

	/* Data segment.  */
	.byte 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x92, 0xCF, 0x00
gdt_end:

   	.p2align	4
gdtdesc:
	.word	gdt_end - gdt
gdt_addr:
	.long	0

