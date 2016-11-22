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

#include <grub/env.h>
#include <grub/misc.h>
#include <grub/disk.h>
#include <grub/machine/biosnum.h>

static int
grub_get_root_biosnumber_default (void)
{
  const char *biosnum;
  int ret = -1;
  grub_device_t dev;

  biosnum = grub_env_get ("biosnum");

  if (biosnum)
    return grub_strtoul (biosnum, 0, 0);

  dev = grub_device_open (0);
  if (dev && dev->disk && dev->disk->dev
      && dev->disk->dev->id == GRUB_DISK_DEVICE_BIOSDISK_ID)
    ret = (int) dev->disk->id;

  if (dev)
    grub_device_close (dev);

  return ret;
}

int (*grub_get_root_biosnumber) (void) = grub_get_root_biosnumber_default;
