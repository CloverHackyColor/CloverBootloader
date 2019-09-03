/* hw.c - U-Boot hardware discovery */
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

#include <grub/kernel.h>
#include <grub/memory.h>
#include <grub/misc.h>
#include <grub/mm.h>
#include <grub/offsets.h>
#include <grub/machine/kernel.h>
#include <grub/uboot/disk.h>
#include <grub/uboot/uboot.h>
#include <grub/uboot/api_public.h>

grub_addr_t start_of_ram;

/*
 * grub_uboot_probe_memory():
 *   Queries U-Boot for available memory regions.
 *
 *   Sets up heap near the image in memory and sets up "start_of_ram".
 */
void
grub_uboot_mm_init (void)
{
  struct sys_info *si = grub_uboot_get_sys_info ();

  grub_mm_init_region ((void *) grub_modules_get_end (),
		       GRUB_KERNEL_MACHINE_HEAP_SIZE);

  if (si && (si->mr_no != 0))
    {
      int i;
      start_of_ram = GRUB_UINT_MAX;

      for (i = 0; i < si->mr_no; i++)
	if ((si->mr[i].flags & MR_ATTR_MASK) == MR_ATTR_DRAM)
	  if (si->mr[i].start < start_of_ram)
	    start_of_ram = si->mr[i].start;
    }
}

/*
 * grub_uboot_probe_hardware():
 */
grub_err_t
grub_uboot_probe_hardware (void)
{
  int devcount, i;

  devcount = grub_uboot_dev_enum ();
  grub_dprintf ("init", "%d devices found\n", devcount);

  for (i = 0; i < devcount; i++)
    {
      struct device_info *devinfo = grub_uboot_dev_get (i);

      grub_dprintf ("init", "device handle: %d\n", i);
      grub_dprintf ("init", "  cookie\t= 0x%08x\n",
		    (grub_uint32_t) devinfo->cookie);

      if (devinfo->type & DEV_TYP_STOR)
	{
	  grub_dprintf ("init", "  type\t\t= DISK\n");
	  grub_ubootdisk_register (devinfo);
	}
      else if (devinfo->type & DEV_TYP_NET)
	{
	  /* Dealt with in ubootnet module. */
	  grub_dprintf ("init", "  type\t\t= NET (not supported yet)\n");
	}
      else
	{
	  grub_dprintf ("init", "%s: unknown device type", __FUNCTION__);
	}
    }

  return GRUB_ERR_NONE;
}

grub_err_t
grub_machine_mmap_iterate (grub_memory_hook_t hook, void *hook_data)
{
  int i;
  struct sys_info *si = grub_uboot_get_sys_info ();

  if (!si || (si->mr_no < 1))
    return GRUB_ERR_BUG;

  /* Iterate and call `hook'.  */
  for (i = 0; i < si->mr_no; i++)
    if ((si->mr[i].flags & MR_ATTR_MASK) == MR_ATTR_DRAM)
      hook (si->mr[i].start, si->mr[i].size, GRUB_MEMORY_AVAILABLE,
	    hook_data);

  return GRUB_ERR_NONE;
}
