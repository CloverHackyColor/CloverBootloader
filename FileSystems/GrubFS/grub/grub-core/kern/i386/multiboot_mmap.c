/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2002,2003,2004,2005,2006,2007,2008,2009  Free Software Foundation, Inc.
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
#include <grub/types.h>
#include <grub/multiboot.h>
#include <grub/err.h>
#include <grub/misc.h>

/* A pointer to the MBI in its initial location.  */
struct multiboot_info *startup_multiboot_info;

/* The MBI has to be copied to our BSS so that it won't be
   overwritten.  This is its final location.  */
static struct multiboot_info kern_multiboot_info;

/* Unfortunately we can't use heap at this point.  But 32 looks like a sane
   limit (used by memtest86).  */
static grub_uint8_t mmap_entries[sizeof (struct multiboot_mmap_entry) * 32];

void
grub_machine_mmap_init (void)
{
  if (! startup_multiboot_info)
    grub_fatal ("Unable to find Multiboot Information (is CONFIG_MULTIBOOT disabled in coreboot?)");

  /* Move MBI to a safe place.  */
  grub_memmove (&kern_multiboot_info, startup_multiboot_info, sizeof (struct multiboot_info));

  if ((kern_multiboot_info.flags & MULTIBOOT_INFO_MEM_MAP) == 0)
    grub_fatal ("Missing Multiboot memory information");

  /* Move the memory map to a safe place.  */
  if (kern_multiboot_info.mmap_length > sizeof (mmap_entries))
    {
      grub_printf ("WARNING: Memory map size exceeds limit (0x%x > 0x%x); it will be truncated\n",
		   kern_multiboot_info.mmap_length, sizeof (mmap_entries));
      kern_multiboot_info.mmap_length = sizeof (mmap_entries);
    }
  grub_memmove (mmap_entries, (void *) kern_multiboot_info.mmap_addr, kern_multiboot_info.mmap_length);
  kern_multiboot_info.mmap_addr = (grub_uint32_t) mmap_entries;
}

grub_err_t
grub_machine_mmap_iterate (grub_memory_hook_t hook, void *hook_data)
{
  struct multiboot_mmap_entry *entry = (void *) kern_multiboot_info.mmap_addr;

  while ((unsigned long) entry < kern_multiboot_info.mmap_addr + kern_multiboot_info.mmap_length)
    {
      if (hook (entry->addr, entry->len, entry->type, hook_data))
	break;

      entry = (void *) ((grub_addr_t) entry + entry->size + sizeof (entry->size));
    }

  return 0;
}
