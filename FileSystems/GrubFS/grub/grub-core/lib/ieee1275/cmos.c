/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2011  Free Software Foundation, Inc.
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

#include <grub/datetime.h>
#include <grub/cmos.h>
#include <grub/dl.h>
#include <grub/ieee1275/ieee1275.h>
#include <grub/misc.h>

volatile grub_uint8_t *grub_cmos_port = 0;

/* Helper for grub_cmos_find_port.  */
static int
grub_cmos_find_port_iter (struct grub_ieee1275_devalias *alias)
{
  grub_ieee1275_phandle_t dev;
  grub_uint32_t addr[2];
  grub_ssize_t actual;
  /* Enough to check if it's "m5819" */
  char compat[100];
  if (grub_ieee1275_finddevice (alias->path, &dev))
    return 0;
  if (grub_ieee1275_get_property (dev, "compatible", compat, sizeof (compat),
				  0))
    return 0;
  if (grub_strcmp (compat, "m5819") != 0)
    return 0;
  if (grub_ieee1275_get_integer_property (dev, "address",
					  addr, sizeof (addr), &actual))
    return 0;
  if (actual == 4)
    {
      grub_cmos_port = (volatile grub_uint8_t *) (grub_addr_t) addr[0];
      return 1;
    }

#if GRUB_CPU_SIZEOF_VOID_P == 8
  if (actual == 8)
    {
      grub_cmos_port = (volatile grub_uint8_t *) 
	((((grub_addr_t) addr[0]) << 32) | addr[1]);
      return 1;
    }
#else
  if (actual == 8 && addr[0] == 0)
    {
      grub_cmos_port = (volatile grub_uint8_t *) addr[1];
      return 1;
    }
#endif
  return 0;
}

grub_err_t
grub_cmos_find_port (void)
{
  grub_ieee1275_devices_iterate (grub_cmos_find_port_iter);
  if (!grub_cmos_port)
    return grub_error (GRUB_ERR_IO, "no cmos found");
  
  return GRUB_ERR_NONE;
}
