/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2008,2009  Free Software Foundation, Inc.
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

#ifndef GRUB_BSD_CPU_HEADER
#define GRUB_BSD_CPU_HEADER	1

#include <grub/types.h>
#include <grub/relocator.h>

#include <grub/i386/freebsd_reboot.h>
#include <grub/i386/netbsd_reboot.h>
#include <grub/i386/openbsd_reboot.h>
#include <grub/i386/freebsd_linker.h>
#include <grub/i386/netbsd_bootinfo.h>
#include <grub/i386/openbsd_bootarg.h>

enum bsd_kernel_types
  {
    KERNEL_TYPE_NONE,
    KERNEL_TYPE_FREEBSD,
    KERNEL_TYPE_OPENBSD,
    KERNEL_TYPE_NETBSD,
  };

#define GRUB_BSD_TEMP_BUFFER   0x80000

#define FREEBSD_B_DEVMAGIC	OPENBSD_B_DEVMAGIC
#define FREEBSD_B_SLICESHIFT	OPENBSD_B_CTRLSHIFT
#define FREEBSD_B_UNITSHIFT	OPENBSD_B_UNITSHIFT
#define FREEBSD_B_PARTSHIFT	OPENBSD_B_PARTSHIFT
#define FREEBSD_B_TYPESHIFT	OPENBSD_B_TYPESHIFT

#define FREEBSD_MODTYPE_KERNEL		"elf kernel"
#define FREEBSD_MODTYPE_KERNEL64	"elf64 kernel"
#define FREEBSD_MODTYPE_ELF_MODULE	"elf module"
#define FREEBSD_MODTYPE_ELF_MODULE_OBJ	"elf obj module"
#define FREEBSD_MODTYPE_RAW		"raw"

#define FREEBSD_BOOTINFO_VERSION 1

struct grub_freebsd_bootinfo
{
  grub_uint32_t version;
  grub_uint8_t unused1[44];
  grub_uint32_t length;
  grub_uint8_t unused2;
  grub_uint8_t boot_device;
  grub_uint8_t unused3[18];
  grub_uint32_t kern_end;
  grub_uint32_t environment;
  grub_uint32_t tags;
} GRUB_PACKED;

struct freebsd_tag_header
{
  grub_uint32_t type;
  grub_uint32_t len;
};

grub_err_t grub_freebsd_load_elfmodule32 (struct grub_relocator *relocator,
					  grub_file_t file, int argc,
					  char *argv[], grub_addr_t *kern_end);
grub_err_t grub_freebsd_load_elfmodule_obj64 (struct grub_relocator *relocator,
					      grub_file_t file, int argc,
					      char *argv[],
					      grub_addr_t *kern_end);
grub_err_t grub_freebsd_load_elf_meta32 (struct grub_relocator *relocator,
					 grub_file_t file,
					 const char *filename,
					 grub_addr_t *kern_end);
grub_err_t grub_freebsd_load_elf_meta64 (struct grub_relocator *relocator,
					 grub_file_t file,
					 const char *filename,
					 grub_addr_t *kern_end);

grub_err_t grub_netbsd_load_elf_meta32 (struct grub_relocator *relocator,
					grub_file_t file,
					const char *filename,
					grub_addr_t *kern_end);
grub_err_t grub_netbsd_load_elf_meta64 (struct grub_relocator *relocator,
					grub_file_t file,
					const char *filename,
					grub_addr_t *kern_end);

grub_err_t grub_bsd_add_meta (grub_uint32_t type, 
			      const void *data, grub_uint32_t len);
grub_err_t grub_freebsd_add_meta_module (const char *filename, const char *type,
					 int argc, char **argv,
					 grub_addr_t addr, grub_uint32_t size);

struct grub_openbsd_ramdisk_descriptor
{
  grub_size_t max_size;
  grub_uint8_t *target;
  grub_uint32_t *size;
};

grub_err_t grub_openbsd_find_ramdisk32 (grub_file_t file,
					const char *filename,
					grub_addr_t kern_start,
					void *kern_chunk_src,
					struct grub_openbsd_ramdisk_descriptor *desc);
grub_err_t grub_openbsd_find_ramdisk64 (grub_file_t file,
					const char *filename,
					grub_addr_t kern_start,
					void *kern_chunk_src,
					struct grub_openbsd_ramdisk_descriptor *desc);

extern grub_uint8_t grub_bsd64_trampoline_start, grub_bsd64_trampoline_end;
extern grub_uint32_t grub_bsd64_trampoline_selfjump;
extern grub_uint32_t grub_bsd64_trampoline_gdt;

#endif /* ! GRUB_BSD_CPU_HEADER */
