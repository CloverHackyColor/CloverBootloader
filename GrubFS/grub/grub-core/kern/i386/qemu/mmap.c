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

#include <grub/i386/memory.h>
#include <grub/machine/memory.h>
#include <grub/machine/boot.h>
#include <grub/types.h>
#include <grub/err.h>
#include <grub/misc.h>
#include <grub/cmos.h>
#include <grub/offsets.h>

#define QEMU_CMOS_MEMSIZE_HIGH		0x35
#define QEMU_CMOS_MEMSIZE_LOW		0x34

#define QEMU_CMOS_MEMSIZE2_HIGH		0x31
#define QEMU_CMOS_MEMSIZE2_LOW		0x30

#define min(a,b)	((a) > (b) ? (b) : (a))

extern char _start[];
extern char _end[];

static grub_uint64_t mem_size, above_4g;

void
grub_machine_mmap_init (void)
{
  grub_uint8_t high, low, b, c, d;
  grub_cmos_read (QEMU_CMOS_MEMSIZE_HIGH, &high);
  grub_cmos_read (QEMU_CMOS_MEMSIZE_LOW, &low);
  mem_size = ((grub_uint64_t) high) << 24
    | ((grub_uint64_t) low) << 16;
  if (mem_size > 0)
    {
      /* Don't ask... */
      mem_size += (16 * 1024 * 1024);
    }
  else
    {
      grub_cmos_read (QEMU_CMOS_MEMSIZE2_HIGH, &high);
      grub_cmos_read (QEMU_CMOS_MEMSIZE2_LOW, &low);
      mem_size
	= ((((grub_uint64_t) high) << 18) | (((grub_uint64_t) low) << 10))
	+ 1024 * 1024;
    }

  grub_cmos_read (0x5b, &b);
  grub_cmos_read (0x5c, &c);
  grub_cmos_read (0x5d, &d);
  above_4g = (((grub_uint64_t) b) << 16)
    | (((grub_uint64_t) c) << 24)
    | (((grub_uint64_t) d) << 32);
}

grub_err_t
grub_machine_mmap_iterate (grub_memory_hook_t hook, void *hook_data)
{
  if (hook (0x0,
	    (grub_addr_t) _start,
	    GRUB_MEMORY_AVAILABLE, hook_data))
    return 1;

  if (hook ((grub_addr_t) _end,
           0xa0000 - (grub_addr_t) _end,
           GRUB_MEMORY_AVAILABLE, hook_data))
    return 1;

  if (hook (GRUB_MEMORY_MACHINE_UPPER,
	    0x100000 - GRUB_MEMORY_MACHINE_UPPER,
	    GRUB_MEMORY_RESERVED, hook_data))
    return 1;

  /* Everything else is free.  */
  if (hook (0x100000,
	    min (mem_size, (grub_uint32_t) -GRUB_BOOT_MACHINE_SIZE) - 0x100000,
	    GRUB_MEMORY_AVAILABLE, hook_data))
    return 1;

  /* Protect boot.img, which contains the gdt.  It is mapped at the top of memory
     (it is also mapped below 0x100000, but we already reserved that area).  */
  if (hook ((grub_uint32_t) -GRUB_BOOT_MACHINE_SIZE,
	    GRUB_BOOT_MACHINE_SIZE,
	    GRUB_MEMORY_RESERVED, hook_data))
    return 1;

  if (above_4g != 0 && hook (0x100000000ULL, above_4g,
			     GRUB_MEMORY_AVAILABLE, hook_data))
    return 1;

  return 0;
}
