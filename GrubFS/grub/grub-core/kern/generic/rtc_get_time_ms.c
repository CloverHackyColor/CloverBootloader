/* rtc_get_time_ms.c - get_time_ms implementation using platform RTC.
 * The generic implementation of these functions can be used for architectures
 * or platforms that do not have a more specialized implementation. */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2008  Free Software Foundation, Inc.
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

#include <grub/time.h>
#include <grub/misc.h>
#include <grub/machine/time.h>

/* Calculate the time in milliseconds since the epoch based on the RTC. */
grub_uint64_t
grub_rtc_get_time_ms (void)
{
  /* By dimensional analysis:

      1000 ms   N rtc ticks       1 s
      ------- * ----------- * ----------- = 1000*N/T ms
        1 s          1        T rtc ticks
   */
  grub_uint64_t ticks_ms_per_sec = ((grub_uint64_t) 1000) * grub_get_rtc ();
  return grub_divmod64 (ticks_ms_per_sec, GRUB_TICKS_PER_SECOND, 0);
}
