/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2002,2003,2004,2005,2006,2007,2008,2009,2010  Free Software Foundation, Inc.
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
#include <grub/mm.h>
#include <grub/machine/boot.h>
#include <grub/i386/floppy.h>
#include <grub/machine/memory.h>
#include <grub/machine/console.h>
#include <grub/machine/kernel.h>
#include <grub/machine/int.h>
#include <grub/types.h>
#include <grub/err.h>
#include <grub/dl.h>
#include <grub/misc.h>
#include <grub/loader.h>
#include <grub/env.h>
#include <grub/cache.h>
#include <grub/time.h>
#include <grub/cpu/tsc.h>
#include <grub/machine/time.h>

struct mem_region
{
  grub_addr_t addr;
  grub_size_t size;
};

#define MAX_REGIONS	32

static struct mem_region mem_regions[MAX_REGIONS];
static int num_regions;

void (*grub_pc_net_config) (char **device, char **path);

/*
 *	return the real time in ticks, of which there are about
 *	18-20 per second
 */
grub_uint64_t
grub_rtc_get_time_ms (void)
{
  struct grub_bios_int_registers regs;

  regs.eax = 0;
  regs.flags = GRUB_CPU_INT_FLAGS_DEFAULT;
  grub_bios_interrupt (0x1a, &regs);

  return ((regs.ecx << 16) | (regs.edx & 0xffff)) * 55ULL;
}

void
grub_machine_get_bootlocation (char **device, char **path)
{
  char *ptr;
  grub_uint8_t boot_drive, dos_part, bsd_part;

  boot_drive = (grub_boot_device >> 24);
  dos_part = (grub_boot_device >> 16);
  bsd_part = (grub_boot_device >> 8);

  /* No hardcoded root partition - make it from the boot drive and the
     partition number encoded at the install time.  */
  if (boot_drive == GRUB_BOOT_MACHINE_PXE_DL)
    {
      if (grub_pc_net_config)
	grub_pc_net_config (device, path);
      return;
    }

  /* XXX: This should be enough.  */
#define DEV_SIZE 100
  *device = grub_malloc (DEV_SIZE);
  ptr = *device;
  grub_snprintf (*device, DEV_SIZE,
		 "%cd%u", (boot_drive & 0x80) ? 'h' : 'f',
		 boot_drive & 0x7f);
  ptr += grub_strlen (ptr);

  if (dos_part != 0xff)
    grub_snprintf (ptr, DEV_SIZE - (ptr - *device),
		   ",%u", dos_part + 1);
  ptr += grub_strlen (ptr);

  if (bsd_part != 0xff)
    grub_snprintf (ptr, DEV_SIZE - (ptr - *device), ",%u",
		   bsd_part + 1);
  ptr += grub_strlen (ptr);
  *ptr = 0;
}

/* Add a memory region.  */
static void
add_mem_region (grub_addr_t addr, grub_size_t size)
{
  if (num_regions == MAX_REGIONS)
    /* Ignore.  */
    return;

  mem_regions[num_regions].addr = addr;
  mem_regions[num_regions].size = size;
  num_regions++;
}

/* Compact memory regions.  */
static void
compact_mem_regions (void)
{
  int i, j;

  /* Sort them.  */
  for (i = 0; i < num_regions - 1; i++)
    for (j = i + 1; j < num_regions; j++)
      if (mem_regions[i].addr > mem_regions[j].addr)
	{
	  struct mem_region tmp = mem_regions[i];
	  mem_regions[i] = mem_regions[j];
	  mem_regions[j] = tmp;
	}

  /* Merge overlaps.  */
  for (i = 0; i < num_regions - 1; i++)
    if (mem_regions[i].addr + mem_regions[i].size >= mem_regions[i + 1].addr)
      {
	j = i + 1;

	if (mem_regions[i].addr + mem_regions[i].size
	    < mem_regions[j].addr + mem_regions[j].size)
	  mem_regions[i].size = (mem_regions[j].addr + mem_regions[j].size
				 - mem_regions[i].addr);

	grub_memmove (mem_regions + j, mem_regions + j + 1,
		      (num_regions - j - 1) * sizeof (struct mem_region));
	i--;
        num_regions--;
      }
}

grub_addr_t grub_modbase;
extern grub_uint8_t _start[], _edata[];

/* Helper for grub_machine_init.  */
static int
mmap_iterate_hook (grub_uint64_t addr, grub_uint64_t size,
		   grub_memory_type_t type,
		   void *data __attribute__ ((unused)))
{
  /* Avoid the lower memory.  */
  if (addr < GRUB_MEMORY_MACHINE_UPPER_START)
    {
      if (size <= GRUB_MEMORY_MACHINE_UPPER_START - addr)
	return 0;

      size -= GRUB_MEMORY_MACHINE_UPPER_START - addr;
      addr = GRUB_MEMORY_MACHINE_UPPER_START;
    }

  /* Ignore >4GB.  */
  if (addr <= 0xFFFFFFFF && type == GRUB_MEMORY_AVAILABLE)
    {
      grub_size_t len;

      len = (grub_size_t) ((addr + size > 0xFFFFFFFF)
	     ? 0xFFFFFFFF - addr
	     : size);
      add_mem_region (addr, len);
    }

  return 0;
}

void
grub_machine_init (void)
{
  int i;
#if 0
  int grub_lower_mem;
#endif
  grub_addr_t modend;

  grub_modbase = GRUB_MEMORY_MACHINE_DECOMPRESSION_ADDR + (_edata - _start);

  /* Initialize the console as early as possible.  */
  grub_console_init ();

  /* This sanity check is useless since top of GRUB_MEMORY_MACHINE_RESERVED_END
     is used for stack and if it's unavailable we wouldn't have gotten so far.
   */
#if 0
  grub_lower_mem = grub_get_conv_memsize () << 10;

  /* Sanity check.  */
  if (grub_lower_mem < GRUB_MEMORY_MACHINE_RESERVED_END)
    grub_fatal ("too small memory");
#endif

/* FIXME: This prevents loader/i386/linux.c from using low memory.  When our
   heap implements support for requesting a chunk in low memory, this should
   no longer be a problem.  */
#if 0
  /* Add the lower memory into free memory.  */
  if (grub_lower_mem >= GRUB_MEMORY_MACHINE_RESERVED_END)
    add_mem_region (GRUB_MEMORY_MACHINE_RESERVED_END,
		    grub_lower_mem - GRUB_MEMORY_MACHINE_RESERVED_END);
#endif

  grub_machine_mmap_iterate (mmap_iterate_hook, NULL);

  compact_mem_regions ();

  modend = grub_modules_get_end ();
  for (i = 0; i < num_regions; i++)
    {
      grub_addr_t beg = mem_regions[i].addr;
      grub_addr_t fin = mem_regions[i].addr + mem_regions[i].size;
      if (modend && beg < modend)
	beg = modend;
      if (beg >= fin)
	continue;
      grub_mm_init_region ((void *) beg, fin - beg);
    }

  grub_tsc_init ();
}

void
grub_machine_fini (int flags)
{
  if (flags & GRUB_LOADER_FLAG_NORETURN)
    grub_console_fini ();
  grub_stop_floppy ();
}
