/* misc.c - various system functions for an arm-based EFI system */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2013 Free Software Foundation, Inc.
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
#include <grub/mm.h>
#include <grub/cpu/linux.h>
#include <grub/cpu/system.h>
#include <grub/efi/efi.h>
#include <grub/machine/loader.h>

static inline grub_size_t
page_align (grub_size_t size)
{
  return (size + (1 << 12) - 1) & (~((1 << 12) - 1));
}

/* Find the optimal number of pages for the memory map. Is it better to
   move this code to efi/mm.c?  */
static grub_efi_uintn_t
find_mmap_size (void)
{
  static grub_efi_uintn_t mmap_size = 0;

  if (mmap_size != 0)
    return mmap_size;
  
  mmap_size = (1 << 12);
  while (1)
    {
      int ret;
      grub_efi_memory_descriptor_t *mmap;
      grub_efi_uintn_t desc_size;
      
      mmap = grub_malloc (mmap_size);
      if (! mmap)
	return 0;

      ret = grub_efi_get_memory_map (&mmap_size, mmap, 0, &desc_size, 0);
      grub_free (mmap);
      
      if (ret < 0)
	{
	  grub_error (GRUB_ERR_IO, "cannot get memory map");
	  return 0;
	}
      else if (ret > 0)
	break;

      mmap_size += (1 << 12);
    }

  /* Increase the size a bit for safety, because GRUB allocates more on
     later, and EFI itself may allocate more.  */
  mmap_size += (1 << 12);

  return page_align (mmap_size);
}

#define NEXT_MEMORY_DESCRIPTOR(desc, size)      \
  ((grub_efi_memory_descriptor_t *) ((char *) (desc) + (size)))
#define PAGE_SHIFT 12

void *
grub_efi_allocate_loader_memory (grub_uint32_t min_offset, grub_uint32_t size)
{
  grub_efi_uintn_t desc_size;
  grub_efi_memory_descriptor_t *mmap, *mmap_end;
  grub_efi_uintn_t mmap_size, tmp_mmap_size;
  grub_efi_memory_descriptor_t *desc;
  void *mem = NULL;
  grub_addr_t min_start = 0;

  mmap_size = find_mmap_size();
  if (!mmap_size)
    return NULL;

  mmap = grub_malloc(mmap_size);
  if (!mmap)
    return NULL;

  tmp_mmap_size = mmap_size;
  if (grub_efi_get_memory_map (&tmp_mmap_size, mmap, 0, &desc_size, 0) <= 0)
    {
      grub_error (GRUB_ERR_IO, "cannot get memory map");
      goto fail;
    }

  mmap_end = NEXT_MEMORY_DESCRIPTOR (mmap, tmp_mmap_size);
  /* Find lowest accessible RAM location */
  {
    int found = 0;
    for (desc = mmap ; !found && (desc < mmap_end) ;
	 desc = NEXT_MEMORY_DESCRIPTOR(desc, desc_size))
      {
	switch (desc->type)
	  {
	  case GRUB_EFI_CONVENTIONAL_MEMORY:
	  case GRUB_EFI_LOADER_CODE:
	  case GRUB_EFI_LOADER_DATA:
	    min_start = desc->physical_start + min_offset;
	    found = 1;
	    break;
	  default:
	    break;
	  }
      }
  }

  /* First, find free pages for the real mode code
     and the memory map buffer.  */
  for (desc = mmap ; desc < mmap_end ;
       desc = NEXT_MEMORY_DESCRIPTOR(desc, desc_size))
    {
      grub_uint64_t start, end;

      grub_dprintf("mm", "%s: 0x%08x bytes @ 0x%08x\n",
		   __FUNCTION__,
		   (grub_uint32_t) (desc->num_pages << PAGE_SHIFT),
		   (grub_uint32_t) (desc->physical_start));

      if (desc->type != GRUB_EFI_CONVENTIONAL_MEMORY)
	continue;

      start = desc->physical_start;
      end = start + (desc->num_pages << PAGE_SHIFT);
      grub_dprintf("mm", "%s: start=0x%016llx, end=0x%016llx\n",
		  __FUNCTION__, start, end);
      start = start < min_start ? min_start : start;
      if (start + size > end)
	continue;
      grub_dprintf("mm", "%s: let's allocate some (0x%x) pages @ 0x%08x...\n",
		  __FUNCTION__, (size >> PAGE_SHIFT), (grub_addr_t) start);
      mem = grub_efi_allocate_pages (start, (size >> PAGE_SHIFT) + 1);
      grub_dprintf("mm", "%s: retval=0x%08x\n",
		   __FUNCTION__, (grub_addr_t) mem);
      if (! mem)
	{
	  grub_error (GRUB_ERR_OUT_OF_MEMORY, "cannot allocate memory");
	  goto fail;
	}
      break;
    }

  if (! mem)
    {
      grub_error (GRUB_ERR_OUT_OF_MEMORY, "cannot allocate memory");
      goto fail;
    }

  grub_free (mmap);
  return mem;

 fail:
  grub_free (mmap);
  return NULL;
}

grub_err_t
grub_efi_prepare_platform (void)
{
  grub_efi_uintn_t mmap_size;
  grub_efi_uintn_t map_key;
  grub_efi_uintn_t desc_size;
  grub_efi_uint32_t desc_version;
  grub_efi_memory_descriptor_t *mmap_buf;
  grub_err_t err;

  /*
   * Cloned from IA64
   * Must be done after grub_machine_fini because map_key is used by
   *exit_boot_services.
   */
  mmap_size = find_mmap_size ();
  if (! mmap_size)
    return GRUB_ERR_OUT_OF_MEMORY;
  mmap_buf = grub_efi_allocate_pages (0, page_align (mmap_size) >> 12);
  if (! mmap_buf)
    return GRUB_ERR_OUT_OF_MEMORY;

  err = grub_efi_finish_boot_services (&mmap_size, mmap_buf, &map_key,
				       &desc_size, &desc_version);
  if (err != GRUB_ERR_NONE)
    return err;

  return GRUB_ERR_NONE;
}
