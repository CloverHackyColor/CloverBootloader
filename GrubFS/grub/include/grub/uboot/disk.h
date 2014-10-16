/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2013  Free Software Foundation, Inc.
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

#ifndef GRUB_UBOOT_DISK_HEADER
#define GRUB_UBOOT_DISK_HEADER	1

#include <grub/symbol.h>
#include <grub/disk.h>
#include <grub/uboot/uboot.h>

void grub_ubootdisk_init (void);
void grub_ubootdisk_fini (void);

enum disktype
{ cd, fd, hd };

struct ubootdisk_data
{
  void *cookie;
  struct device_info *dev;
  int opencount;
  enum disktype type;
  grub_uint32_t block_size;
};

grub_err_t grub_ubootdisk_register (struct device_info *newdev);

#endif /* ! GRUB_UBOOT_DISK_HEADER */
