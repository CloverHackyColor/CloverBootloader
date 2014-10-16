/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2009 Free Software Foundation, Inc.
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

#ifndef GRUB_MEMORY_MACHINE_HEADER
#define GRUB_MEMORY_MACHINE_HEADER	1

#ifndef ASM_FILE
#include <grub/symbol.h>
#include <grub/err.h>
#include <grub/types.h>
#endif

#define GRUB_MACHINE_MEMORY_STACK_HIGH       0x80f00000
#define GRUB_MACHINE_MEMORY_USABLE       0x81000000

#ifndef ASM_FILE

extern grub_uint32_t grub_arch_memsize __attribute__ ((section(".text")));

static inline grub_err_t
grub_machine_mmap_register (grub_uint64_t start __attribute__ ((unused)),
			    grub_uint64_t size __attribute__ ((unused)),
			    int type __attribute__ ((unused)),
			    int handle __attribute__ ((unused)))
{
  return GRUB_ERR_NONE;
}
static inline grub_err_t
grub_machine_mmap_unregister (int handle  __attribute__ ((unused)))
{
  return GRUB_ERR_NONE;
}
#endif

#endif
