/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2009  Free Software Foundation, Inc.
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

#ifndef GRUB_CPU_XNU_H
#define GRUB_CPU_XNU_H 1

#include <grub/err.h>
#include <grub/efi/api.h>
#include <grub/cpu/relocator.h>

#define XNU_RELOCATOR(x) (grub_relocator32_ ## x)

#define GRUB_XNU_PAGESIZE 4096
typedef grub_uint32_t grub_xnu_ptr_t;

struct grub_xnu_boot_params_common
{
  /* Command line passed to xnu. */
  grub_uint8_t cmdline[1024];

  /* Later are the same as EFI's get_memory_map (). */
  grub_xnu_ptr_t efi_mmap;
  grub_uint32_t efi_mmap_size;
  grub_uint32_t efi_mem_desc_size;
  grub_uint32_t efi_mem_desc_version;

  /* Later are video parameters. */
  grub_xnu_ptr_t lfb_base;
#define GRUB_XNU_VIDEO_SPLASH 1
#define GRUB_XNU_VIDEO_TEXT_IN_VIDEO 2
  grub_uint32_t lfb_mode;
  grub_uint32_t lfb_line_len;
  grub_uint32_t lfb_width;
  grub_uint32_t lfb_height;
  grub_uint32_t lfb_depth;

  /* Pointer to device tree and its len. */
  grub_xnu_ptr_t devtree;
  grub_uint32_t devtreelen;

  /* First used address by kernel or boot structures. */
  grub_xnu_ptr_t heap_start;
  /* Last used address by kernel or boot structures minus previous value. */
  grub_uint32_t heap_size;
  /* First memory page containing runtime code or data. */
  grub_uint32_t efi_runtime_first_page;
  /* First memory page containing runtime code or data minus previous value. */
  grub_uint32_t efi_runtime_npages;
} GRUB_PACKED;

struct grub_xnu_boot_params_v1
{
  grub_uint16_t verminor;
  grub_uint16_t vermajor;
  struct grub_xnu_boot_params_common common;

  grub_uint32_t efi_system_table;
  /* Size of grub_efi_uintn_t in bits. */
  grub_uint8_t efi_uintnbits;
} GRUB_PACKED;
#define GRUB_XNU_BOOTARGSV1_VERMINOR 5
#define GRUB_XNU_BOOTARGSV1_VERMAJOR 1

struct grub_xnu_boot_params_v2
{
  grub_uint16_t verminor;
  grub_uint16_t vermajor;

  /* Size of grub_efi_uintn_t in bits. */
  grub_uint8_t efi_uintnbits;
  grub_uint8_t unused[3];

  struct grub_xnu_boot_params_common common;

  grub_uint64_t efi_runtime_first_page_virtual;
  grub_uint32_t efi_system_table;
  grub_uint32_t unused2[11];
  grub_uint64_t fsbfreq;
  grub_uint32_t unused3[734];
} GRUB_PACKED;
#define GRUB_XNU_BOOTARGSV2_VERMINOR 0
#define GRUB_XNU_BOOTARGSV2_VERMAJOR 2

union grub_xnu_boot_params_any
{
  struct grub_xnu_boot_params_v1 v1;
  struct grub_xnu_boot_params_v2 v2;
};

struct grub_xnu_devprop_header
{
  grub_uint32_t length;
  /* Always set to 1. Version?  */
  grub_uint32_t alwaysone;
  grub_uint32_t num_devices;
};

struct grub_xnu_devprop_device_header
{
  grub_uint32_t length;
  grub_uint32_t num_values;
};

void grub_cpu_xnu_unload (void);

struct grub_xnu_devprop_device_descriptor;

struct grub_xnu_devprop_device_descriptor *
grub_xnu_devprop_add_device (struct grub_efi_device_path *path, int length);
grub_err_t
grub_xnu_devprop_remove_device (struct grub_xnu_devprop_device_descriptor *dev);
grub_err_t
grub_xnu_devprop_remove_property (struct grub_xnu_devprop_device_descriptor *dev,
				  char *name);
grub_err_t
grub_xnu_devprop_add_property_utf8 (struct grub_xnu_devprop_device_descriptor *dev,
				    char *name, void *data, int datalen);
grub_err_t
grub_xnu_devprop_add_property_utf16 (struct grub_xnu_devprop_device_descriptor *dev,
				     grub_uint16_t *name, int namelen,
				     void *data, int datalen);
grub_err_t
grub_xnu_devprop_remove_property_utf8 (struct grub_xnu_devprop_device_descriptor *dev,
				       char *name);
void grub_cpu_xnu_init (void);
void grub_cpu_xnu_fini (void);

extern grub_uint32_t grub_xnu_entry_point;
extern grub_uint32_t grub_xnu_stack;
extern grub_uint32_t grub_xnu_arg1;
extern char grub_xnu_cmdline[1024];
grub_err_t grub_xnu_boot (void);
#endif
