/* Code for managing symbols and pointers in efiemu */
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

#include <grub/err.h>
#include <grub/mm.h>
#include <grub/misc.h>
#include <grub/efiemu/efiemu.h>
#include <grub/efiemu/runtime.h>
#include <grub/i18n.h>

static int ptv_written = 0;
static int ptv_alloc = 0;
static int ptv_handle = 0;
static int relocated_handle = 0;
static int ptv_requested = 0;
static struct grub_efiemu_sym *efiemu_syms = 0;

struct grub_efiemu_sym
{
  struct grub_efiemu_sym *next;
  char *name;
  int handle;
  grub_off_t off;
};

void
grub_efiemu_free_syms (void)
{
  struct grub_efiemu_sym *cur, *d;
  for (cur = efiemu_syms; cur;)
    {
      d = cur->next;
      grub_free (cur->name);
      grub_free (cur);
      cur = d;
    }
  efiemu_syms = 0;
  ptv_written = 0;
  ptv_alloc = 0;
  ptv_requested = 0;
  grub_efiemu_mm_return_request (ptv_handle);
  ptv_handle = 0;
  grub_efiemu_mm_return_request (relocated_handle);
  relocated_handle = 0;
}

/* Announce that the module will need NUM allocators */
/* Because of deferred memory allocation all the relocators have to be
   announced during phase 1*/
grub_err_t
grub_efiemu_request_symbols (int num)
{
  if (ptv_alloc)
    return grub_error (GRUB_ERR_BAD_ARGUMENT,
		       "symbols have already been allocated");
  if (num < 0)
    return grub_error (GRUB_ERR_BUG,
		       "can't request negative symbols");
  ptv_requested += num;
  return GRUB_ERR_NONE;
}

/* Resolve the symbol name NAME and set HANDLE and OFF accordingly  */
grub_err_t
grub_efiemu_resolve_symbol (const char *name, int *handle, grub_off_t *off)
{
  struct grub_efiemu_sym *cur;
  for (cur = efiemu_syms; cur; cur = cur->next)
    if (!grub_strcmp (name, cur->name))
      {
	*handle = cur->handle;
	*off = cur->off;
	return GRUB_ERR_NONE;
      }
  grub_dprintf ("efiemu", "%s not found\n", name);
  return grub_error (GRUB_ERR_BAD_OS, N_("symbol `%s' not found"), name);
}

/* Register symbol named NAME in memory handle HANDLE at offset OFF */
grub_err_t
grub_efiemu_register_symbol (const char *name, int handle, grub_off_t off)
{
  struct grub_efiemu_sym *cur;
  cur = (struct grub_efiemu_sym *) grub_malloc (sizeof (*cur));
  grub_dprintf ("efiemu", "registering symbol '%s'\n", name);
  if (!cur)
    return grub_errno;
  cur->name = grub_strdup (name);
  cur->next = efiemu_syms;
  cur->handle = handle;
  cur->off = off;
  efiemu_syms = cur;

  return 0;
}

/* Go from phase 1 to phase 2. Must be called before similar function in mm.c */
grub_err_t
grub_efiemu_alloc_syms (void)
{
  ptv_alloc = ptv_requested;
  ptv_handle = grub_efiemu_request_memalign
    (1, (ptv_requested + 1) * sizeof (struct grub_efiemu_ptv_rel),
     GRUB_EFI_RUNTIME_SERVICES_DATA);
  relocated_handle = grub_efiemu_request_memalign
    (1, sizeof (grub_uint8_t), GRUB_EFI_RUNTIME_SERVICES_DATA);

  grub_efiemu_register_symbol ("efiemu_ptv_relocated", relocated_handle, 0);
  grub_efiemu_register_symbol ("efiemu_ptv_relloc", ptv_handle, 0);
  return grub_errno;
}

grub_err_t
grub_efiemu_write_sym_markers (void)
{
  struct grub_efiemu_ptv_rel *ptv_rels
    = grub_efiemu_mm_obtain_request (ptv_handle);
  grub_uint8_t *relocated = grub_efiemu_mm_obtain_request (relocated_handle);
  grub_memset (ptv_rels, 0, (ptv_requested + 1)
	       * sizeof (struct grub_efiemu_ptv_rel));
  *relocated = 0;
  return GRUB_ERR_NONE;
}

/* Write value (pointer to memory PLUS_HANDLE)
   - (pointer to memory MINUS_HANDLE) + VALUE to ADDR assuming that the
   size SIZE bytes. If PTV_NEEDED is 1 then announce it to runtime that this
   value needs to be recomputed before going to virtual mode
*/
grub_err_t
grub_efiemu_write_value (void *addr, grub_uint32_t value, int plus_handle,
			 int minus_handle, int ptv_needed, int size)
{
  /* Announce relocator to runtime */
  if (ptv_needed)
    {
      struct grub_efiemu_ptv_rel *ptv_rels
	= grub_efiemu_mm_obtain_request (ptv_handle);

      if (ptv_needed && ptv_written >= ptv_alloc)
	return grub_error (GRUB_ERR_BUG,
			   "your module didn't declare efiemu "
			   " relocators correctly");

      if (minus_handle)
	ptv_rels[ptv_written].minustype
	  = grub_efiemu_mm_get_type (minus_handle);
      else
	ptv_rels[ptv_written].minustype = 0;

      if (plus_handle)
	ptv_rels[ptv_written].plustype
	  = grub_efiemu_mm_get_type (plus_handle);
      else
	ptv_rels[ptv_written].plustype = 0;

      ptv_rels[ptv_written].addr = (grub_addr_t) addr;
      ptv_rels[ptv_written].size = size;
      ptv_written++;

      /* memset next value to zero to mark the end */
      grub_memset (&ptv_rels[ptv_written], 0, sizeof (ptv_rels[ptv_written]));
    }

  /* Compute the value */
  if (minus_handle)
    value -= (grub_addr_t) grub_efiemu_mm_obtain_request (minus_handle);

  if (plus_handle)
    value += (grub_addr_t) grub_efiemu_mm_obtain_request (plus_handle);

  /* Write the value */
  switch (size)
    {
    case 8:
      *((grub_uint64_t *) addr) = value;
      break;
    case 4:
      *((grub_uint32_t *) addr) = value;
      break;
    case 2:
      *((grub_uint16_t *) addr) = value;
      break;
    case 1:
      *((grub_uint8_t *) addr) = value;
      break;
    default:
      return grub_error (GRUB_ERR_BUG, "wrong symbol size");
    }

  return GRUB_ERR_NONE;
}

grub_err_t
grub_efiemu_set_virtual_address_map (grub_efi_uintn_t memory_map_size,
				     grub_efi_uintn_t descriptor_size,
				     grub_efi_uint32_t descriptor_version
				     __attribute__ ((unused)),
				     grub_efi_memory_descriptor_t *virtual_map)
{
  grub_uint8_t *ptv_relocated;
  struct grub_efiemu_ptv_rel *cur_relloc;
  struct grub_efiemu_ptv_rel *ptv_rels;

  ptv_relocated = grub_efiemu_mm_obtain_request (relocated_handle);
  ptv_rels = grub_efiemu_mm_obtain_request (ptv_handle);

  /* Ensure that we are called only once */
  if (*ptv_relocated)
    return grub_error (GRUB_ERR_BUG, "EfiEmu is already relocated");
  *ptv_relocated = 1;

  /* Correct addresses using information supplied by grub */
  for (cur_relloc = ptv_rels; cur_relloc->size; cur_relloc++)
    {
      grub_int64_t corr = 0;
      grub_efi_memory_descriptor_t *descptr;

      /* Compute correction */
      for (descptr = virtual_map;
	   (grub_size_t) ((grub_uint8_t *) descptr
			  - (grub_uint8_t *) virtual_map) < memory_map_size;
	   descptr = (grub_efi_memory_descriptor_t *)
	     ((grub_uint8_t *) descptr + descriptor_size))
	{
	  if (descptr->type == cur_relloc->plustype)
	    corr += descptr->virtual_start - descptr->physical_start;
	  if (descptr->type == cur_relloc->minustype)
	    corr -= descptr->virtual_start - descptr->physical_start;
	}

      /* Apply correction */
      switch (cur_relloc->size)
	{
	case 8:
	  *((grub_uint64_t *) (grub_addr_t) cur_relloc->addr) += corr;
	  break;
	case 4:
	  *((grub_uint32_t *) (grub_addr_t) cur_relloc->addr) += corr;
	  break;
	case 2:
	  *((grub_uint16_t *) (grub_addr_t) cur_relloc->addr) += corr;
	  break;
	case 1:
	  *((grub_uint8_t *) (grub_addr_t) cur_relloc->addr) += corr;
	  break;
	}
    }

  /* Recompute crc32 of system table and runtime services */

  if (grub_efiemu_sizeof_uintn_t () == 4)
    return grub_efiemu_crc32 ();
  else
    return grub_efiemu_crc64 ();
}
