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
#include <grub/dl.h>
#include <grub/misc.h>
#include <grub/arc/arc.h>

GRUB_MOD_LICENSE ("GPLv3+");

grub_err_t
grub_get_datetime (struct grub_datetime *datetime)
{
  struct grub_arc_timeinfo *dt;
  grub_memset (datetime, 0, sizeof (*datetime));

  dt = GRUB_ARC_FIRMWARE_VECTOR->gettime ();

  datetime->year = dt->y;
  datetime->month = dt->m;
  datetime->day = dt->d;
  datetime->hour = dt->h;
  datetime->minute = dt->min;
  datetime->second = dt->s;

  return 0;
}

grub_err_t
grub_set_datetime (struct grub_datetime *datetime __attribute__ ((unused)))
{
  return grub_error (GRUB_ERR_IO, "setting time isn't supported");
}
