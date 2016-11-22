/* millisleep.c - generic millisleep function.
 * The generic implementation of these functions can be used for architectures
 * or platforms that do not have a more specialized implementation. */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 1999,2000,2001,2002,2003,2004,2005,2006,2007,2008  Free Software Foundation, Inc.
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

#include <grub/misc.h>
#include <grub/time.h>

void
grub_millisleep (grub_uint32_t ms)
{
  grub_uint64_t start;

  start = grub_get_time_ms ();

  /* Instead of setting an end time and looping while the current time is
     less than that, comparing the elapsed sleep time with the desired sleep
     time handles the (unlikely!) case that the timer would wrap around
     during the sleep. */

  while (grub_get_time_ms () - start < ms)
    grub_cpu_idle ();
}
