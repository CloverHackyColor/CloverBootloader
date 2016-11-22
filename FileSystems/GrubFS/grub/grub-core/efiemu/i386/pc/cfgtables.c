/* Register SMBIOS and ACPI tables. */
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
#include <grub/efiemu/efiemu.h>
#include <grub/misc.h>
#include <grub/mm.h>
#include <grub/acpi.h>

grub_err_t
grub_machine_efiemu_init_tables (void)
{
  grub_uint8_t *ptr;
  void *table;
  grub_err_t err;
  grub_efi_guid_t smbios = GRUB_EFI_SMBIOS_TABLE_GUID;
  grub_efi_guid_t acpi20 = GRUB_EFI_ACPI_20_TABLE_GUID;
  grub_efi_guid_t acpi = GRUB_EFI_ACPI_TABLE_GUID;

  err = grub_efiemu_unregister_configuration_table (smbios);
  if (err)
    return err;
  err = grub_efiemu_unregister_configuration_table (acpi);
  if (err)
    return err;
  err = grub_efiemu_unregister_configuration_table (acpi20);
  if (err)
    return err;

  table = grub_acpi_get_rsdpv1 ();
  if (table)
    {
      err = grub_efiemu_register_configuration_table (acpi, 0, 0, table);
      if (err)
	return err;
    }
  table = grub_acpi_get_rsdpv2 ();
  if (table)
    {
      err = grub_efiemu_register_configuration_table (acpi20, 0, 0, table);
      if (err)
	return err;
    }

  for (ptr = (grub_uint8_t *) 0xf0000; ptr < (grub_uint8_t *) 0x100000;
       ptr += 16)
    if (grub_memcmp (ptr, "_SM_", 4) == 0
	&& grub_byte_checksum (ptr, *(ptr + 5)) == 0)
      break;

  if (ptr < (grub_uint8_t *) 0x100000)
    {
      grub_dprintf ("efiemu", "Registering SMBIOS\n");
      err = grub_efiemu_register_configuration_table (smbios, 0, 0, ptr);
      if (err)
	return err;
    }

  return GRUB_ERR_NONE;
}
