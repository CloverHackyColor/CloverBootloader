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

#ifndef GRUB_SYSLINUX_PARSE_HEADER
#define GRUB_SYSLINUX_PARSE_HEADER 1

#include <grub/types.h>

typedef enum
  {
    GRUB_SYSLINUX_UNKNOWN,
    GRUB_SYSLINUX_ISOLINUX,
    GRUB_SYSLINUX_PXELINUX,
    GRUB_SYSLINUX_SYSLINUX,
  } grub_syslinux_flavour_t;

char *
grub_syslinux_config_file (const char *root, const char *target_root,
			   const char *cwd, const char *target_cwd,
			   const char *fname, grub_syslinux_flavour_t flav);

#endif
