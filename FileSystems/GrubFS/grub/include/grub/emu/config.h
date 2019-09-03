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

#ifndef GRUB_CONFIG_EMU_HEADER
#define GRUB_CONFIG_EMU_HEADER	1

#include <grub/disk.h>
#include <grub/partition.h>
#include <grub/emu/hostfile.h>
#include <stdio.h>

const char *
grub_util_get_config_filename (void);
const char *
grub_util_get_pkgdatadir (void);
const char *
grub_util_get_pkglibdir (void);
const char *
grub_util_get_localedir (void);

struct grub_util_config
{
  int is_cryptodisk_enabled;
  char *grub_distributor;
};

void
grub_util_load_config (struct grub_util_config *cfg);

void
grub_util_parse_config (FILE *f, struct grub_util_config *cfg, int simple);

#endif
