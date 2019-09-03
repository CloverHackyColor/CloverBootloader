/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2010  Free Software Foundation, Inc.
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

#include <grub/relocator.h>
#include <grub/relocator_private.h>
#include <grub/memory.h>
#include <grub/efi/efi.h>
#include <grub/efi/api.h>
#include <grub/term.h>

#define NEXT_MEMORY_DESCRIPTOR(desc, size)	\
  ((grub_efi_memory_descriptor_t *) ((char *) (desc) + (size)))

unsigned 
grub_relocator_firmware_get_max_events (void)
{
  grub_efi_uintn_t mmapsize = 0, descriptor_size = 0;
  grub_efi_uint32_t descriptor_version = 0;
  grub_efi_uintn_t key;
  grub_efi_get_memory_map (&mmapsize, NULL, &key, &descriptor_size,
			   &descriptor_version);
  /* Since grub_relocator_firmware_fill_events uses malloc
     we need some reserve. Hence +10.  */
  return 2 * (mmapsize / descriptor_size + 10);
}

unsigned 
grub_relocator_firmware_fill_events (struct grub_relocator_mmap_event *events)
{
  grub_efi_uintn_t mmapsize = 0, desc_size = 0;
  grub_efi_uint32_t descriptor_version = 0;
  grub_efi_memory_descriptor_t *descs = NULL;
  grub_efi_uintn_t key;
  int counter = 0;
  grub_efi_memory_descriptor_t *desc;

  grub_efi_get_memory_map (&mmapsize, NULL, &key, &desc_size,
			   &descriptor_version);
  descs = grub_malloc (mmapsize);
  if (!descs)
    return 0;

  grub_efi_get_memory_map (&mmapsize, descs, &key, &desc_size,
			   &descriptor_version);

  for (desc = descs;
       (char *) desc < ((char *) descs + mmapsize);
       desc = NEXT_MEMORY_DESCRIPTOR (desc, desc_size))
    {
      grub_uint64_t start = desc->physical_start;
      grub_uint64_t end = desc->physical_start + (desc->num_pages << 12);

      /* post-4G addresses are never supported on 32-bit EFI. 
	 Moreover it has been reported that some 64-bit EFI contrary to the
	 spec don't map post-4G pages. So if you enable post-4G allocations,
	 map pages manually or check that they are mapped.
       */
      if (end >= 0x100000000ULL)
	end = 0x100000000ULL;
      if (end <= start)
	continue;
      if (desc->type != GRUB_EFI_CONVENTIONAL_MEMORY)
	continue;
      events[counter].type = REG_FIRMWARE_START;
      events[counter].pos = start;
      counter++;
      events[counter].type = REG_FIRMWARE_END;
      events[counter].pos = end;
      counter++;      
    }

  return counter;
}

int
grub_relocator_firmware_alloc_region (grub_addr_t start, grub_size_t size)
{
  grub_efi_boot_services_t *b;
  grub_efi_physical_address_t address = start;
  grub_efi_status_t status;

  if (grub_efi_is_finished)
    return 1;
#ifdef DEBUG_RELOCATOR_NOMEM_DPRINTF
  grub_dprintf ("relocator", "EFI alloc: %llx, %llx\n",
		(unsigned long long) start, (unsigned long long) size);
#endif
  b = grub_efi_system_table->boot_services;
  status = efi_call_4 (b->allocate_pages, GRUB_EFI_ALLOCATE_ADDRESS,
		       GRUB_EFI_LOADER_DATA, size >> 12, &address);
  return (status == GRUB_EFI_SUCCESS);
}

void
grub_relocator_firmware_free_region (grub_addr_t start, grub_size_t size)
{
  grub_efi_boot_services_t *b;

  if (grub_efi_is_finished)
    return;

  b = grub_efi_system_table->boot_services;
  efi_call_2 (b->free_pages, start, size >> 12);
}
