/* Load runtime image of EFIemu. Functions common to 32/64-bit mode */
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

#include <grub/file.h>
#include <grub/err.h>
#include <grub/mm.h>
#include <grub/misc.h>
#include <grub/efiemu/efiemu.h>
#include <grub/cpu/efiemu.h>

/* Are we in 32 or 64-bit mode?*/
static grub_efiemu_mode_t grub_efiemu_mode = GRUB_EFIEMU_NOTLOADED;
/* Runtime ELF file */
static grub_ssize_t efiemu_core_size;
static void *efiemu_core = 0;
/* Linked list of segments */
static grub_efiemu_segment_t efiemu_segments = 0;

/* equivalent to sizeof (grub_efi_uintn_t) but taking the mode into account*/
int
grub_efiemu_sizeof_uintn_t (void)
{
  if (grub_efiemu_mode == GRUB_EFIEMU32)
    return 4;
  if (grub_efiemu_mode == GRUB_EFIEMU64)
    return 8;
  return 0;
}

/* Check the header and set mode */
static grub_err_t
grub_efiemu_check_header (void *ehdr, grub_size_t size,
			  grub_efiemu_mode_t *mode)
{
  /* Check the magic numbers.  */
  if ((*mode == GRUB_EFIEMU_NOTLOADED || *mode == GRUB_EFIEMU32)
      && grub_efiemu_check_header32 (ehdr,size))
    {
      *mode = GRUB_EFIEMU32;
      return GRUB_ERR_NONE;
    }
  if ((*mode == GRUB_EFIEMU_NOTLOADED || *mode == GRUB_EFIEMU64)
      && grub_efiemu_check_header64 (ehdr,size))
    {
      *mode = GRUB_EFIEMU64;
      return GRUB_ERR_NONE;
    }
  return grub_error (GRUB_ERR_BAD_OS, "invalid ELF magic");
}

/* Unload segments */
static int
grub_efiemu_unload_segs (grub_efiemu_segment_t seg)
{
  grub_efiemu_segment_t segn;
  for (; seg; seg = segn)
    {
      segn = seg->next;
      grub_efiemu_mm_return_request (seg->handle);
      grub_free (seg);
    }
  return 1;
}


grub_err_t
grub_efiemu_loadcore_unload(void)
{
  switch (grub_efiemu_mode)
    {
    case GRUB_EFIEMU32:
      grub_efiemu_loadcore_unload32 ();
      break;

    case GRUB_EFIEMU64:
      grub_efiemu_loadcore_unload64 ();
      break;

    default:
      break;
    }

  grub_efiemu_mode = GRUB_EFIEMU_NOTLOADED;

  grub_free (efiemu_core);
  efiemu_core = 0;

  grub_efiemu_unload_segs (efiemu_segments);
  efiemu_segments = 0;

  grub_efiemu_free_syms ();

  return GRUB_ERR_NONE;
}

/* Load runtime file and do some initial preparations */
grub_err_t
grub_efiemu_loadcore_init (grub_file_t file,
			   const char *filename)
{
  grub_err_t err;

  efiemu_core_size = grub_file_size (file);
  efiemu_core = 0;
  efiemu_core = grub_malloc (efiemu_core_size);
  if (! efiemu_core)
    return grub_errno;

  if (grub_file_read (file, efiemu_core, efiemu_core_size)
      != (int) efiemu_core_size)
    {
      grub_free (efiemu_core);
      efiemu_core = 0;
      return grub_errno;
    }

  if (grub_efiemu_check_header (efiemu_core, efiemu_core_size,
				&grub_efiemu_mode))
    {
      grub_free (efiemu_core);
      efiemu_core = 0;
      return GRUB_ERR_BAD_MODULE;
    }

  switch (grub_efiemu_mode)
    {
    case GRUB_EFIEMU32:
      err = grub_efiemu_loadcore_init32 (efiemu_core, filename,
					 efiemu_core_size,
					 &efiemu_segments);
      if (err)
	{
	  grub_free (efiemu_core);
	  efiemu_core = 0;
	  grub_efiemu_mode = GRUB_EFIEMU_NOTLOADED;
	  return err;
	}
      break;

    case GRUB_EFIEMU64:
      err = grub_efiemu_loadcore_init64 (efiemu_core, filename,
					 efiemu_core_size,
					 &efiemu_segments);
      if (err)
	{
	  grub_free (efiemu_core);
	  efiemu_core = 0;
	  grub_efiemu_mode = GRUB_EFIEMU_NOTLOADED;
	  return err;
	}
      break;

    default:
      return grub_error (GRUB_ERR_BUG, "unknown EFI runtime");
    }
  return GRUB_ERR_NONE;
}

grub_err_t
grub_efiemu_loadcore_load (void)
{
  grub_err_t err;
  switch (grub_efiemu_mode)
    {
    case GRUB_EFIEMU32:
      err = grub_efiemu_loadcore_load32 (efiemu_core, efiemu_core_size,
					 efiemu_segments);
      if (err)
	grub_efiemu_loadcore_unload ();
      return err;
    case GRUB_EFIEMU64:
      err = grub_efiemu_loadcore_load64 (efiemu_core, efiemu_core_size,
					 efiemu_segments);
      if (err)
	grub_efiemu_loadcore_unload ();
      return err;
    default:
      return grub_error (GRUB_ERR_BUG, "unknown EFI runtime");
    }
}
