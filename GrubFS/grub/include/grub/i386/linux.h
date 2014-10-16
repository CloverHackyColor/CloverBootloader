/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 1999,2000,2001,2002,2003,2004,2007,2008,2009  Free Software Foundation, Inc.
 *
 *  GRUB is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  GRUB is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GRUB.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GRUB_LINUX_MACHINE_HEADER
#define GRUB_LINUX_MACHINE_HEADER	1

#define GRUB_LINUX_MAGIC_SIGNATURE	0x53726448      /* "HdrS" */
#define GRUB_LINUX_DEFAULT_SETUP_SECTS	4
#define GRUB_LINUX_INITRD_MAX_ADDRESS	0x37FFFFFF
#define GRUB_LINUX_MAX_SETUP_SECTS	64
#define GRUB_LINUX_BOOT_LOADER_TYPE	0x72
#define GRUB_LINUX_HEAP_END_OFFSET	(0x9000 - 0x200)

#define GRUB_LINUX_BZIMAGE_ADDR		0x100000
#define GRUB_LINUX_ZIMAGE_ADDR		0x10000
#define GRUB_LINUX_OLD_REAL_MODE_ADDR	0x90000
#define GRUB_LINUX_SETUP_STACK		0x9000

#define GRUB_LINUX_FLAG_BIG_KERNEL	0x1
#define GRUB_LINUX_FLAG_QUIET		0x20
#define GRUB_LINUX_FLAG_CAN_USE_HEAP	0x80

/* Linux's video mode selection support. Actually I hate it!  */
#define GRUB_LINUX_VID_MODE_NORMAL	0xFFFF
#define GRUB_LINUX_VID_MODE_EXTENDED	0xFFFE
#define GRUB_LINUX_VID_MODE_ASK		0xFFFD
#define GRUB_LINUX_VID_MODE_VESA_START	0x0300

#define GRUB_LINUX_CL_MAGIC		0xA33F

#ifdef __x86_64__

#define GRUB_LINUX_EFI_SIGNATURE	\
  ('4' << 24 | '6' << 16 | 'L' << 8 | 'E')

#else

#define GRUB_LINUX_EFI_SIGNATURE	\
  ('2' << 24 | '3' << 16 | 'L' << 8 | 'E')

#endif

#define GRUB_LINUX_EFI_SIGNATURE_0204	\
  ('L' << 24 | 'I' << 16 | 'F' << 8 | 'E')

#define GRUB_LINUX_OFW_SIGNATURE	\
  (' ' << 24 | 'W' << 16 | 'F' << 8 | 'O')

#ifndef ASM_FILE

#define GRUB_E820_RAM        1
#define GRUB_E820_RESERVED   2
#define GRUB_E820_ACPI       3
#define GRUB_E820_NVS        4
#define GRUB_E820_BADRAM     5

struct grub_e820_mmap
{
  grub_uint64_t addr;
  grub_uint64_t size;
  grub_uint32_t type;
} GRUB_PACKED;

enum
  {
    GRUB_VIDEO_LINUX_TYPE_TEXT = 0x01,
    GRUB_VIDEO_LINUX_TYPE_VESA = 0x23,    /* VESA VGA in graphic mode.  */
    GRUB_VIDEO_LINUX_TYPE_EFIFB = 0x70,    /* EFI Framebuffer.  */
    GRUB_VIDEO_LINUX_TYPE_SIMPLE = 0x70    /* Linear framebuffer without any additional functions.  */
  };

/* For the Linux/i386 boot protocol version 2.10.  */
struct linux_kernel_header
{
  grub_uint8_t code1[0x0020];
  grub_uint16_t cl_magic;		/* Magic number 0xA33F */
  grub_uint16_t cl_offset;		/* The offset of command line */
  grub_uint8_t code2[0x01F1 - 0x0020 - 2 - 2];
  grub_uint8_t setup_sects;		/* The size of the setup in sectors */
  grub_uint16_t root_flags;		/* If the root is mounted readonly */
  grub_uint16_t syssize;		/* obsolete */
  grub_uint16_t swap_dev;		/* obsolete */
  grub_uint16_t ram_size;		/* obsolete */
  grub_uint16_t vid_mode;		/* Video mode control */
  grub_uint16_t root_dev;		/* Default root device number */
  grub_uint16_t boot_flag;		/* 0xAA55 magic number */
  grub_uint16_t jump;			/* Jump instruction */
  grub_uint32_t header;			/* Magic signature "HdrS" */
  grub_uint16_t version;		/* Boot protocol version supported */
  grub_uint32_t realmode_swtch;		/* Boot loader hook */
  grub_uint16_t start_sys;		/* The load-low segment (obsolete) */
  grub_uint16_t kernel_version;		/* Points to kernel version string */
  grub_uint8_t type_of_loader;		/* Boot loader identifier */
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
  grub_uint8_t loadflags;		/* Boot protocol option flags */
  grub_uint16_t setup_move_size;	/* Move to high memory size */
  grub_uint32_t code32_start;		/* Boot loader hook */
  grub_uint32_t ramdisk_image;		/* initrd load address */
  grub_uint32_t ramdisk_size;		/* initrd size */
  grub_uint32_t bootsect_kludge;	/* obsolete */
  grub_uint16_t heap_end_ptr;		/* Free memory after setup end */
  grub_uint16_t pad1;			/* Unused */
  grub_uint32_t cmd_line_ptr;		/* Points to the kernel command line */
  grub_uint32_t initrd_addr_max;        /* Highest address for initrd */
  grub_uint32_t kernel_alignment;
  grub_uint8_t relocatable;
  grub_uint8_t min_alignment;
  grub_uint8_t pad[2];
  grub_uint32_t cmdline_size;
  grub_uint32_t hardware_subarch;
  grub_uint64_t hardware_subarch_data;
  grub_uint32_t payload_offset;
  grub_uint32_t payload_length;
  grub_uint64_t setup_data;
  grub_uint64_t pref_address;
  grub_uint32_t init_size;
} GRUB_PACKED;

/* Boot parameters for Linux based on 2.6.12. This is used by the setup
   sectors of Linux, and must be simulated by GRUB on EFI, because
   the setup sectors depend on BIOS.  */
struct linux_kernel_params
{
  grub_uint8_t video_cursor_x;		/* 0 */
  grub_uint8_t video_cursor_y;

  grub_uint16_t ext_mem;		/* 2 */

  grub_uint16_t video_page;		/* 4 */
  grub_uint8_t video_mode;		/* 6 */
  grub_uint8_t video_width;		/* 7 */

  grub_uint8_t padding1[0xa - 0x8];

  grub_uint16_t video_ega_bx;		/* a */

  grub_uint8_t padding2[0xe - 0xc];

  grub_uint8_t video_height;		/* e */
  grub_uint8_t have_vga;		/* f */
  grub_uint16_t font_size;		/* 10 */

  grub_uint16_t lfb_width;		/* 12 */
  grub_uint16_t lfb_height;		/* 14 */
  grub_uint16_t lfb_depth;		/* 16 */
  grub_uint32_t lfb_base;		/* 18 */
  grub_uint32_t lfb_size;		/* 1c */

  grub_uint16_t cl_magic;		/* 20 */
  grub_uint16_t cl_offset;

  grub_uint16_t lfb_line_len;		/* 24 */
  grub_uint8_t red_mask_size;		/* 26 */
  grub_uint8_t red_field_pos;
  grub_uint8_t green_mask_size;
  grub_uint8_t green_field_pos;
  grub_uint8_t blue_mask_size;
  grub_uint8_t blue_field_pos;
  grub_uint8_t reserved_mask_size;
  grub_uint8_t reserved_field_pos;
  grub_uint16_t vesapm_segment;		/* 2e */
  grub_uint16_t vesapm_offset;		/* 30 */
  grub_uint16_t lfb_pages;		/* 32 */
  grub_uint16_t vesa_attrib;		/* 34 */
  grub_uint32_t capabilities;		/* 36 */

  grub_uint8_t padding3[0x40 - 0x3a];

  grub_uint16_t apm_version;		/* 40 */
  grub_uint16_t apm_code_segment;	/* 42 */
  grub_uint32_t apm_entry;		/* 44 */
  grub_uint16_t apm_16bit_code_segment;	/* 48 */
  grub_uint16_t apm_data_segment;	/* 4a */
  grub_uint16_t apm_flags;		/* 4c */
  grub_uint32_t apm_code_len;		/* 4e */
  grub_uint16_t apm_data_len;		/* 52 */

  grub_uint8_t padding4[0x60 - 0x54];

  grub_uint32_t ist_signature;		/* 60 */
  grub_uint32_t ist_command;		/* 64 */
  grub_uint32_t ist_event;		/* 68 */
  grub_uint32_t ist_perf_level;		/* 6c */

  grub_uint8_t padding5[0x80 - 0x70];

  grub_uint8_t hd0_drive_info[0x10];	/* 80 */
  grub_uint8_t hd1_drive_info[0x10];	/* 90 */
  grub_uint16_t rom_config_len;		/* a0 */

  grub_uint8_t padding6[0xb0 - 0xa2];

  grub_uint32_t ofw_signature;		/* b0 */
  grub_uint32_t ofw_num_items;		/* b4 */
  grub_uint32_t ofw_cif_handler;	/* b8 */
  grub_uint32_t ofw_idt;		/* bc */

  grub_uint8_t padding7[0x1b8 - 0xc0];

  union
    {
      struct
        {
          grub_uint32_t efi_system_table;	/* 1b8 */
          grub_uint32_t padding7_1;		/* 1bc */
          grub_uint32_t efi_signature;		/* 1c0 */
          grub_uint32_t efi_mem_desc_size;	/* 1c4 */
          grub_uint32_t efi_mem_desc_version;	/* 1c8 */
          grub_uint32_t efi_mmap_size;		/* 1cc */
          grub_uint32_t efi_mmap;		/* 1d0 */
        } v0204;
      struct
        {
          grub_uint32_t padding7_1;		/* 1b8 */
          grub_uint32_t padding7_2;		/* 1bc */
          grub_uint32_t efi_signature;		/* 1c0 */
          grub_uint32_t efi_system_table;	/* 1c4 */
          grub_uint32_t efi_mem_desc_size;	/* 1c8 */
          grub_uint32_t efi_mem_desc_version;	/* 1cc */
          grub_uint32_t efi_mmap;		/* 1d0 */
          grub_uint32_t efi_mmap_size;		/* 1d4 */
	} v0206;
      struct
        {
          grub_uint32_t padding7_1;		/* 1b8 */
          grub_uint32_t padding7_2;		/* 1bc */
          grub_uint32_t efi_signature;		/* 1c0 */
          grub_uint32_t efi_system_table;	/* 1c4 */
          grub_uint32_t efi_mem_desc_size;	/* 1c8 */
          grub_uint32_t efi_mem_desc_version;	/* 1cc */
          grub_uint32_t efi_mmap;		/* 1d0 */
          grub_uint32_t efi_mmap_size;		/* 1d4 */
          grub_uint32_t efi_system_table_hi;	/* 1d8 */
          grub_uint32_t efi_mmap_hi;		/* 1dc */
        } v0208;
    };

  grub_uint32_t alt_mem;		/* 1e0 */

  grub_uint8_t padding8[0x1e8 - 0x1e4];

  grub_uint8_t mmap_size;		/* 1e8 */

  grub_uint8_t padding9[0x1f1 - 0x1e9];

  grub_uint8_t setup_sects;		/* The size of the setup in sectors */
  grub_uint16_t root_flags;		/* If the root is mounted readonly */
  grub_uint16_t syssize;		/* obsolete */
  grub_uint16_t swap_dev;		/* obsolete */
  grub_uint16_t ram_size;		/* obsolete */
  grub_uint16_t vid_mode;		/* Video mode control */
  grub_uint16_t root_dev;		/* Default root device number */

  grub_uint8_t padding10;		/* 1fe */
  grub_uint8_t ps_mouse;		/* 1ff */

  grub_uint16_t jump;			/* Jump instruction */
  grub_uint32_t header;			/* Magic signature "HdrS" */
  grub_uint16_t version;		/* Boot protocol version supported */
  grub_uint32_t realmode_swtch;		/* Boot loader hook */
  grub_uint16_t start_sys;		/* The load-low segment (obsolete) */
  grub_uint16_t kernel_version;		/* Points to kernel version string */
  grub_uint8_t type_of_loader;		/* Boot loader identifier */
  grub_uint8_t loadflags;		/* Boot protocol option flags */
  grub_uint16_t setup_move_size;	/* Move to high memory size */
  grub_uint32_t code32_start;		/* Boot loader hook */
  grub_uint32_t ramdisk_image;		/* initrd load address */
  grub_uint32_t ramdisk_size;		/* initrd size */
  grub_uint32_t bootsect_kludge;	/* obsolete */
  grub_uint16_t heap_end_ptr;		/* Free memory after setup end */
  grub_uint8_t ext_loader_ver;		/* Extended loader version */
  grub_uint8_t ext_loader_type;		/* Extended loader type */  
  grub_uint32_t cmd_line_ptr;		/* Points to the kernel command line */
  grub_uint32_t initrd_addr_max;	/* Maximum initrd address */
  grub_uint32_t kernel_alignment;	/* Alignment of the kernel */
  grub_uint8_t relocatable_kernel;	/* Is the kernel relocatable */
  grub_uint8_t pad1[3];
  grub_uint32_t cmdline_size;		/* Size of the kernel command line */
  grub_uint32_t hardware_subarch;
  grub_uint64_t hardware_subarch_data;
  grub_uint32_t payload_offset;
  grub_uint32_t payload_length;
  grub_uint64_t setup_data;
  grub_uint8_t pad2[120];		/* 258 */
  struct grub_e820_mmap e820_map[(0x400 - 0x2d0) / 20];	/* 2d0 */

} GRUB_PACKED;
#endif /* ! ASM_FILE */

#endif /* ! GRUB_LINUX_MACHINE_HEADER */
