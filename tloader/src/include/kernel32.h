#pragma once

#include <e820.h>

//
#define ALIGN_UP(addr, align) \
	((addr + (typeof (addr)) align - 1) & ~((typeof (addr)) align - 1))
#define ALIGN_UP_OVERHEAD(addr, align) ((-(addr)) & ((typeof (addr)) (align) - 1))
//
#define DISK_SECTOR_BITS	9
#define DISK_SECTOR_SIZE	0x200
//
#define LINUX_IMAGE "BOOT_IMAGE="
//
#define LINUX_MAGIC_SIGNATURE	0x53726448      /* "HdrS" */
#define LINUX_DEFAULT_SETUP_SECTS	4
#define LINUX_INITRD_MAX_ADDRESS	0x37FFFFFF
#define LINUX_MAX_SETUP_SECTS	64
#define LINUX_BOOT_LOADER_TYPE	0x72
#define LINUX_HEAP_END_OFFSET	(0x9000 - 0x200)

#define LINUX_REALCODE_ADDR         0x80000
#define LINUX_BZIMAGE_ADDR		0x100000
#define LINUX_ZIMAGE_ADDR		0x10000
#define LINUX_OLD_REAL_MODE_ADDR	0x90000
#define LINUX_SETUP_STACK		0x9000

#define LINUX_FLAG_BIG_KERNEL	0x1
#define LINUX_FLAG_QUIET		0x20
#define LINUX_FLAG_CAN_USE_HEAP	0x80

/* Linux's video mode selection support. Actually I hate it!  */
#define LINUX_VID_MODE_NORMAL	0xFFFF
#define LINUX_VID_MODE_EXTENDED	0xFFFE
#define LINUX_VID_MODE_ASK		0xFFFD
#define LINUX_VID_MODE_VESA_START	0x0300

#define LINUX_CL_MAGIC		0xA33F

#ifdef __x86_64__

#define LINUX_EFI_SIGNATURE	\
  ('4' << 24 | '6' << 16 | 'L' << 8 | 'E')

#else

#define LINUX_EFI_SIGNATURE	\
  ('2' << 24 | '3' << 16 | 'L' << 8 | 'E')

#endif

#define LINUX_EFI_SIGNATURE_0204	\
  ('L' << 24 | 'I' << 16 | 'F' << 8 | 'E')

#define LINUX_OFW_SIGNATURE	\
  (' ' << 24 | 'W' << 16 | 'F' << 8 | 'O')

enum
{
   VIDEO_LINUX_TYPE_TEXT = 0x01,
   VIDEO_LINUX_TYPE_VESA = 0x23,    /* VESA VGA in graphic mode.  */
   VIDEO_LINUX_TYPE_EFIFB = 0x70,    /* EFI Framebuffer.  */
   VIDEO_LINUX_TYPE_SIMPLE = 0x70    /* Linear framebuffer without any additional functions.  */
};
 
/* For the Linux/i386 boot protocol version 2.10.  */
struct linux_kernel_header
{
  uint8_t code1[0x0020];
  uint16_t cl_magic;		/* Magic number 0xA33F */
  uint16_t cl_offset;		/* The offset of command line */
  uint8_t code2[0x01F1 - 0x0020 - 2 - 2];
  uint8_t setup_sects;		/* The size of the setup in sectors */
  uint16_t root_flags;		/* If the root is mounted readonly */
  uint16_t syssize;		/* obsolete */
  uint16_t swap_dev;		/* obsolete */
  uint16_t ram_size;		/* obsolete */
  uint16_t vid_mode;		/* Video mode control */
  uint16_t root_dev;		/* Default root device number */
  uint16_t boot_flag;		/* 0xAA55 magic number */
  uint16_t jump;			/* Jump instruction */
  uint32_t header;			/* Magic signature "HdrS" */
  uint16_t version;		/* Boot protocol version supported */
  uint32_t realmode_swtch;		/* Boot loader hook */
  uint16_t start_sys;		/* The load-low segment (obsolete) */
  uint16_t kernel_version;		/* Points to kernel version string */
  uint8_t type_of_loader;		/* Boot loader identifier */
#define LINUX_LOADER_ID_LILO		0x0
#define LINUX_LOADER_ID_LOADLIN		0x1
#define LINUX_LOADER_ID_BOOTSECT	0x2
#define LINUX_LOADER_ID_SYSLINUX	0x3
#define LINUX_LOADER_ID_ETHERBOOT	0x4
#define LINUX_LOADER_ID_ELILO		0x5
#define LINUX_LOADER_ID_GRUB		0x7
#define LINUX_LOADER_ID_UBOOT		0x8
#define LINUX_LOADER_ID_XEN		0x9
#define LINUX_LOADER_ID_GUJIN		0xa
#define LINUX_LOADER_ID_QEMU		0xb
  uint8_t loadflags;		/* Boot protocol option flags */
  uint16_t setup_move_size;	/* Move to high memory size */
  uint32_t code32_start;		/* Boot loader hook */
  uint32_t ramdisk_image;		/* initrd load address */
  uint32_t ramdisk_size;		/* initrd size */
  uint32_t bootsect_kludge;	/* obsolete */
  uint16_t heap_end_ptr;		/* Free memory after setup end */
  uint16_t pad1;			/* Unused */
  uint32_t cmd_line_ptr;		/* Points to the kernel command line */
  uint32_t initrd_addr_max;        /* Highest address for initrd */
  uint32_t kernel_alignment;
  uint8_t relocatable;
  uint8_t min_alignment;
  uint8_t pad[2];
  uint32_t cmdline_size;
  uint32_t hardware_subarch;
  uint64_t hardware_subarch_data;
  uint32_t payload_offset;
  uint32_t payload_length;
  uint64_t setup_data;
  uint64_t pref_address;
  uint32_t init_size;
} __attribute__ ((packed));

/* Boot parameters for Linux based on 2.6.12. This is used by the setup
   sectors of Linux, and must be simulated by GRUB on EFI, because
   the setup sectors depend on BIOS.  */
struct linux_kernel_params
{
  uint8_t video_cursor_x;		/* 0 */
  uint8_t video_cursor_y;

  uint16_t ext_mem;		/* 2 */

  uint16_t video_page;		/* 4 */
  uint8_t video_mode;		/* 6 */
  uint8_t video_width;		/* 7 */

  uint8_t padding1[0xa - 0x8];

  uint16_t video_ega_bx;		/* a */

  uint8_t padding2[0xe - 0xc];

  uint8_t video_height;		/* e */
  uint8_t have_vga;		/* f */
  uint16_t font_size;		/* 10 */

  uint16_t lfb_width;		/* 12 */
  uint16_t lfb_height;		/* 14 */
  uint16_t lfb_depth;		/* 16 */
  uint32_t lfb_base;		/* 18 */
  uint32_t lfb_size;		/* 1c */

  uint16_t cl_magic;		/* 20 */
  uint16_t cl_offset;

  uint16_t lfb_line_len;		/* 24 */
  uint8_t red_mask_size;		/* 26 */
  uint8_t red_field_pos;
  uint8_t green_mask_size;
  uint8_t green_field_pos;
  uint8_t blue_mask_size;
  uint8_t blue_field_pos;
  uint8_t reserved_mask_size;
  uint8_t reserved_field_pos;
  uint16_t vesapm_segment;		/* 2e */
  uint16_t vesapm_offset;		/* 30 */
  uint16_t lfb_pages;		/* 32 */
  uint16_t vesa_attrib;		/* 34 */
  uint32_t capabilities;		/* 36 */

  uint8_t padding3[0x40 - 0x3a];

  uint16_t apm_version;		/* 40 */
  uint16_t apm_code_segment;	/* 42 */
  uint32_t apm_entry;		/* 44 */
  uint16_t apm_16bit_code_segment;	/* 48 */
  uint16_t apm_data_segment;	/* 4a */
  uint16_t apm_flags;		/* 4c */
  uint32_t apm_code_len;		/* 4e */
  uint16_t apm_data_len;		/* 52 */

  uint8_t padding4[0x60 - 0x54];

  uint32_t ist_signature;		/* 60 */
  uint32_t ist_command;		/* 64 */
  uint32_t ist_event;		/* 68 */
  uint32_t ist_perf_level;		/* 6c */

  uint8_t padding5[0x80 - 0x70];

  uint8_t hd0_drive_info[0x10];	/* 80 */
  uint8_t hd1_drive_info[0x10];	/* 90 */
  uint16_t rom_config_len;		/* a0 */

  uint8_t padding6[0xb0 - 0xa2];

  uint32_t ofw_signature;		/* b0 */
  uint32_t ofw_num_items;		/* b4 */
  uint32_t ofw_cif_handler;	/* b8 */
  uint32_t ofw_idt;		/* bc */

  uint8_t padding7[0x1b8 - 0xc0];

  union
    {
      struct
        {
          uint32_t efi_system_table;	/* 1b8 */
          uint32_t padding7_1;		/* 1bc */
          uint32_t efi_signature;		/* 1c0 */
          uint32_t efi_mem_desc_size;	/* 1c4 */
          uint32_t efi_mem_desc_version;	/* 1c8 */
          uint32_t efi_mmap_size;		/* 1cc */
          uint32_t efi_mmap;		/* 1d0 */
        } v0204;
      struct
        {
          uint32_t padding7_1;		/* 1b8 */
          uint32_t padding7_2;		/* 1bc */
          uint32_t efi_signature;		/* 1c0 */
          uint32_t efi_system_table;	/* 1c4 */
          uint32_t efi_mem_desc_size;	/* 1c8 */
          uint32_t efi_mem_desc_version;	/* 1cc */
          uint32_t efi_mmap;		/* 1d0 */
          uint32_t efi_mmap_size;		/* 1d4 */
	} v0206;
      struct
        {
          uint32_t padding7_1;		/* 1b8 */
          uint32_t padding7_2;		/* 1bc */
          uint32_t efi_signature;		/* 1c0 */
          uint32_t efi_system_table;	/* 1c4 */
          uint32_t efi_mem_desc_size;	/* 1c8 */
          uint32_t efi_mem_desc_version;	/* 1cc */
          uint32_t efi_mmap;		/* 1d0 */
          uint32_t efi_mmap_size;		/* 1d4 */
          uint32_t efi_system_table_hi;	/* 1d8 */
          uint32_t efi_mmap_hi;		/* 1dc */
        } v0208;
    };

  uint32_t alt_mem;		/* 1e0 */

  uint8_t padding8[0x1e8 - 0x1e4];

  uint8_t mmap_size;		/* 1e8 */

  uint8_t padding9[0x1f1 - 0x1e9];

  uint8_t setup_sects;		/* The size of the setup in sectors */
  uint16_t root_flags;		/* If the root is mounted readonly */
  uint16_t syssize;		/* obsolete */
  uint16_t swap_dev;		/* obsolete */
  uint16_t ram_size;		/* obsolete */
  uint16_t vid_mode;		/* Video mode control */
  uint16_t root_dev;		/* Default root device number */

  uint8_t padding10;		/* 1fe */
  uint8_t ps_mouse;		/* 1ff */

  uint16_t jump;			/* Jump instruction */
  uint32_t header;			/* Magic signature "HdrS" */
  uint16_t version;		/* Boot protocol version supported */
  uint32_t realmode_swtch;		/* Boot loader hook */
  uint16_t start_sys;		/* The load-low segment (obsolete) */
  uint16_t kernel_version;		/* Points to kernel version string */
  uint8_t type_of_loader;		/* Boot loader identifier */
  uint8_t loadflags;		/* Boot protocol option flags */
  uint16_t setup_move_size;	/* Move to high memory size */
  uint32_t code32_start;		/* Boot loader hook */
  uint32_t ramdisk_image;		/* initrd load address */
  uint32_t ramdisk_size;		/* initrd size */
  uint32_t bootsect_kludge;	/* obsolete */
  uint16_t heap_end_ptr;		/* Free memory after setup end */
  uint8_t ext_loader_ver;		/* Extended loader version */
  uint8_t ext_loader_type;		/* Extended loader type */  
  uint32_t cmd_line_ptr;		/* Points to the kernel command line */
  uint32_t initrd_addr_max;	/* Maximum initrd address */
  uint32_t kernel_alignment;	/* Alignment of the kernel */
  uint8_t relocatable_kernel;	/* Is the kernel relocatable */
  uint8_t pad1[3];
  uint32_t cmdline_size;		/* Size of the kernel command line */
  uint32_t hardware_subarch;
  uint64_t hardware_subarch_data;
  uint32_t payload_offset;
  uint32_t payload_length;
  uint64_t setup_data;
  uint8_t pad2[120];		/* 258 */
  struct e820_mmap e820_map[(0x400 - 0x2d0) / 20];	/* 2d0 */

} __attribute__ ((packed));

static inline size_t page_align(size_t size) {
   return (size + (1 << 12) - 1) & (~((1 << 12) - 1));
}

static inline int grub_tolower(int c) {
   if(c >= 'A' && c <= 'Z') {
      return c - 'A' + 'a';
   }
   return c;
}
