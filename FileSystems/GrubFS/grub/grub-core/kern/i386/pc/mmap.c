/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2002,2003,2004,2005,2006,2007,2008  Free Software Foundation, Inc.
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

#include <grub/machine/memory.h>
#include <grub/machine/int.h>
#include <grub/err.h>
#include <grub/types.h>
#include <grub/misc.h>

struct grub_machine_mmap_entry
{
  grub_uint32_t size;
  grub_uint64_t addr;
  grub_uint64_t len;
#define GRUB_MACHINE_MEMORY_AVAILABLE	1
#define GRUB_MACHINE_MEMORY_RESERVED	2
#define GRUB_MACHINE_MEMORY_ACPI	3
#define GRUB_MACHINE_MEMORY_NVS 	4
#define GRUB_MACHINE_MEMORY_BADRAM 	5
  grub_uint32_t type;
} GRUB_PACKED;


/*
 *
 * grub_get_conv_memsize(i) :  return the conventional memory size in KB.
 *	BIOS call "INT 12H" to get conventional memory size
 *      The return value in AX.
 */
static inline grub_uint16_t
grub_get_conv_memsize (void)
{
  struct grub_bios_int_registers regs;

  regs.flags = GRUB_CPU_INT_FLAGS_DEFAULT;
  grub_bios_interrupt (0x12, &regs);
  return regs.eax & 0xffff;
}

/*
 * grub_get_ext_memsize() :  return the extended memory size in KB.
 *	BIOS call "INT 15H, AH=88H" to get extended memory size
 *	The return value in AX.
 *
 */
static inline grub_uint16_t
grub_get_ext_memsize (void)
{
  struct grub_bios_int_registers regs;

  regs.eax = 0x8800;
  regs.flags = GRUB_CPU_INT_FLAGS_DEFAULT;
  grub_bios_interrupt (0x15, &regs);
  return regs.eax & 0xffff;
}

/* Get a packed EISA memory map. Lower 16 bits are between 1MB and 16MB
   in 1KB parts, and upper 16 bits are above 16MB in 64KB parts. If error, return zero.
   BIOS call "INT 15H, AH=E801H" to get EISA memory map,
     AX = memory between 1M and 16M in 1K parts.
     BX = memory above 16M in 64K parts. 
*/
 
static inline grub_uint32_t
grub_get_eisa_mmap (void)
{
  struct grub_bios_int_registers regs;

  regs.flags = GRUB_CPU_INT_FLAGS_DEFAULT;
  regs.eax = 0xe801;
  grub_bios_interrupt (0x15, &regs);

  if ((regs.eax & 0xff00) == 0x8600)
    return 0;

  return (regs.eax & 0xffff) | (regs.ebx << 16);
}

/*
 *
 * grub_get_mmap_entry(addr, cont) : address and old continuation value (zero to
 *		start), for the Query System Address Map BIOS call.
 *
 *  Sets the first 4-byte int value of "addr" to the size returned by
 *  the call.  If the call fails, sets it to zero.
 *
 *	Returns:  new (non-zero) continuation value, 0 if done.
 */
/* Get a memory map entry. Return next continuation value. Zero means
   the end.  */
static grub_uint32_t
grub_get_mmap_entry (struct grub_machine_mmap_entry *entry,
		     grub_uint32_t cont)
{
  struct grub_bios_int_registers regs;

  regs.flags = GRUB_CPU_INT_FLAGS_DEFAULT;

  /* place address (+4) in ES:DI */
  regs.es = ((grub_addr_t) &entry->addr) >> 4;
  regs.edi = ((grub_addr_t) &entry->addr) & 0xf;
	
  /* set continuation value */
  regs.ebx = cont;

  /* set default maximum buffer size */
  regs.ecx = sizeof (*entry) - sizeof (entry->size);

  /* set EDX to 'SMAP' */
  regs.edx = 0x534d4150;

  regs.eax = 0xe820;
  grub_bios_interrupt (0x15, &regs);

  /* write length of buffer (zero if error) into ADDR */	
  if ((regs.flags & GRUB_CPU_INT_FLAGS_CARRY) || regs.eax != 0x534d4150
      || regs.ecx < 0x14 || regs.ecx > 0x400)
    entry->size = 0;
  else
    entry->size = regs.ecx;

  /* return the continuation value */
  return regs.ebx;
}

grub_err_t
grub_machine_mmap_iterate (grub_memory_hook_t hook, void *hook_data)
{
  grub_uint32_t cont = 0;
  struct grub_machine_mmap_entry *entry
    = (struct grub_machine_mmap_entry *) GRUB_MEMORY_MACHINE_SCRATCH_ADDR;
  int e820_works = 0;

  while (1)
    {
      grub_memset (entry, 0, sizeof (entry));

      cont = grub_get_mmap_entry (entry, cont);

      if (!entry->size)
	break;

      if (entry->len)
	e820_works = 1;
      if (entry->len
	  && hook (entry->addr, entry->len,
		   /* GRUB mmaps have been defined to match with
		      the E820 definition.
		      Therefore, we can just pass type through.  */
		   entry->type, hook_data))
	break;

      if (! cont)
	break;
    }

  if (!e820_works)
    {
      grub_uint32_t eisa_mmap = grub_get_eisa_mmap ();

      if (hook (0x0, ((grub_uint32_t) grub_get_conv_memsize ()) << 10,
		GRUB_MEMORY_AVAILABLE, hook_data))
	return 0;

      if (eisa_mmap)
	{
	  if (hook (0x100000, (eisa_mmap & 0xFFFF) << 10,
		    GRUB_MEMORY_AVAILABLE, hook_data) == 0)
	    hook (0x1000000, eisa_mmap & ~0xFFFF, GRUB_MEMORY_AVAILABLE,
		  hook_data);
	}
      else
	hook (0x100000, ((grub_uint32_t) grub_get_ext_memsize ()) << 10,
	      GRUB_MEMORY_AVAILABLE, hook_data);
    }

  return 0;
}
