/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2007, 2008  Free Software Foundation, Inc.
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

#ifndef KERNEL_TIME_HEADER
#define KERNEL_TIME_HEADER	1

#include <grub/types.h>
#include <grub/symbol.h>
#ifndef GRUB_MACHINE_EMU
#include <grub/cpu/time.h>
#else
static inline void
grub_cpu_idle(void)
{
}
#endif

void EXPORT_FUNC(grub_millisleep) (grub_uint32_t ms);
grub_uint64_t EXPORT_FUNC(grub_get_time_ms) (void);

grub_uint64_t grub_rtc_get_time_ms (void);

static __inline void
grub_sleep (grub_uint32_t s)
{
  grub_millisleep (1000 * s);
}

void grub_install_get_time_ms (grub_uint64_t (*get_time_ms_func) (void));

#endif /* ! KERNEL_TIME_HEADER */
