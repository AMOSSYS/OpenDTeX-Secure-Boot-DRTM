#include <tloader.h>
#undef ENTRY

OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(start)
PHDRS
{
  text PT_LOAD ;
}
SECTIONS
{
  . = TLOADER_BASE_ADDR;
  
  .text : {
		*(.tloader_multiboot_header)
		. = ALIGN(4096);
	
  		_stext = .;	                /* text */
		*(.text)
		*(.fixup)
		*(.gnu.warning)
	} :text = 0x9090

	_etext = .;			/* end of text section */
	
	. = ALIGN(4096);
	__to_relocate = .;
	.bios_sector : {
		*(.bios_sector)
	}
	__to_relocate_end = .;
	. = ALIGN(4096);

	.rodata : { *(.rodata) *(.rodata.*) }
	. = ALIGN(4096);

	.data : {			/* Data */
		*(.data)
		*(.tboot_shared)
		CONSTRUCTORS
	}
	
	. = ALIGN(4096);
	.bss : {
		*(.bss.stack_aligned)
	}

	_end = . ;
}

