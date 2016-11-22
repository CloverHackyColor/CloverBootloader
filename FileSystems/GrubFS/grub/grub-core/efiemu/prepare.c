/* Prepare efiemu. E.g. allocate memory, load the runtime
   to appropriate place, etc */
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
#include <grub/types.h>
#include <grub/misc.h>
#include <grub/efiemu/efiemu.h>
#include <grub/crypto.h>

grub_err_t
SUFFIX (grub_efiemu_prepare) (struct grub_efiemu_prepare_hook *prepare_hooks,
			      struct grub_efiemu_configuration_table
			      *config_tables)
{
  grub_err_t err;
  int conftable_handle;
  struct grub_efiemu_configuration_table *cur;
  struct grub_efiemu_prepare_hook *curhook;

  int cntconftables = 0;
  struct SUFFIX (grub_efiemu_configuration_table) *conftables = 0;
  int i;
  int handle;
  grub_off_t off;

  grub_dprintf ("efiemu", "Preparing EfiEmu\n");

  /* Request space for the list of configuration tables */
  for (cur = config_tables; cur; cur = cur->next)
    cntconftables++;
  conftable_handle
    = grub_efiemu_request_memalign (GRUB_EFIEMU_PAGESIZE,
				    cntconftables * sizeof (*conftables),
				    GRUB_EFI_RUNTIME_SERVICES_DATA);

  /* Switch from phase 1 (counting) to phase 2 (real job) */
  grub_efiemu_alloc_syms ();
  grub_efiemu_mm_do_alloc ();
  grub_efiemu_write_sym_markers ();

  grub_efiemu_system_table32 = 0;
  grub_efiemu_system_table64 = 0;

  /* Execute hooks */
  for (curhook = prepare_hooks; curhook; curhook = curhook->next)
    curhook->hook (curhook->data);

  /* Move runtime to its due place */
  err = grub_efiemu_loadcore_load ();
  if (err)
    {
      grub_efiemu_unload ();
      return err;
    }

  err = grub_efiemu_resolve_symbol ("efiemu_system_table", &handle, &off);
  if (err)
    {
      grub_efiemu_unload ();
      return err;
    }

  SUFFIX (grub_efiemu_system_table)
    = (struct SUFFIX (grub_efi_system_table) *)
    ((grub_uint8_t *) grub_efiemu_mm_obtain_request (handle) + off);

  /* Put pointer to the list of configuration tables in system table */
  grub_efiemu_write_value
    (&(SUFFIX (grub_efiemu_system_table)->configuration_table), 0,
     conftable_handle, 0, 1,
     sizeof (SUFFIX (grub_efiemu_system_table)->configuration_table));
  SUFFIX(grub_efiemu_system_table)->num_table_entries = cntconftables;

  /* Fill the list of configuration tables */
  conftables = (struct SUFFIX (grub_efiemu_configuration_table) *)
    grub_efiemu_mm_obtain_request (conftable_handle);
  i = 0;
  for (cur = config_tables; cur; cur = cur->next, i++)
    {
      grub_memcpy (&(conftables[i].vendor_guid), &(cur->guid),
		       sizeof (cur->guid));
      if (cur->get_table)
	conftables[i].vendor_table = (grub_addr_t) cur->get_table (cur->data);
      else
	conftables[i].vendor_table = (grub_addr_t) cur->data;
    }

  err = SUFFIX (grub_efiemu_crc) ();
  if (err)
    {
      grub_efiemu_unload ();
      return err;
    }

  grub_dprintf ("efiemu","system_table = %p, conftables = %p (%d entries)\n",
		SUFFIX (grub_efiemu_system_table), conftables, cntconftables);

  return GRUB_ERR_NONE;
}

grub_err_t
SUFFIX (grub_efiemu_crc) (void)
{
  grub_err_t err;
  int handle;
  grub_off_t off;
  struct SUFFIX (grub_efiemu_runtime_services) *runtime_services;
  grub_uint32_t crc32_val;

  if (GRUB_MD_CRC32->mdlen != 4)
    return grub_error (GRUB_ERR_BUG, "incorrect mdlen");

  /* compute CRC32 of runtime_services */
  err = grub_efiemu_resolve_symbol ("efiemu_runtime_services",
				    &handle, &off);
  if (err)
    return err;

  runtime_services = (struct SUFFIX (grub_efiemu_runtime_services) *)
	((grub_uint8_t *) grub_efiemu_mm_obtain_request (handle) + off);

  runtime_services->hdr.crc32 = 0;

  grub_crypto_hash (GRUB_MD_CRC32, &crc32_val,
		    runtime_services, runtime_services->hdr.header_size);
  runtime_services->hdr.crc32 =
      grub_be_to_cpu32(crc32_val);

  err = grub_efiemu_resolve_symbol ("efiemu_system_table", &handle, &off);
  if (err)
    return err;

  /* compute CRC32 of system table */
  SUFFIX (grub_efiemu_system_table)->hdr.crc32 = 0;
  grub_crypto_hash (GRUB_MD_CRC32, &crc32_val,
		    SUFFIX (grub_efiemu_system_table),
		    SUFFIX (grub_efiemu_system_table)->hdr.header_size);
  SUFFIX (grub_efiemu_system_table)->hdr.crc32 =
      grub_be_to_cpu32(crc32_val);

  grub_dprintf ("efiemu","system_table = %p, runtime_services = %p\n",
		SUFFIX (grub_efiemu_system_table), runtime_services);

  return GRUB_ERR_NONE;
}
