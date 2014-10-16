/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2012  Free Software Foundation, Inc.
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

#ifndef GRUB_EFI_EDID_HEADER
#define GRUB_EFI_EDID_HEADER	1

/* Based on UEFI specification.  */

#define GRUB_EFI_EDID_ACTIVE_GUID \
  { 0xbd8c1056, 0x9f36, 0x44ec, { 0x92, 0xa8, 0xa6, 0x33, 0x7f, 0x81, 0x79, 0x86 }}

#define GRUB_EFI_EDID_DISCOVERED_GUID \
  {0x1c0c34f6,0xd380,0x41fa, {0xa0,0x49,0x8a,0xd0,0x6c,0x1a,0x66,0xaa}}

#define GRUB_EFI_EDID_OVERRIDE_GUID \
  {0x48ecb431,0xfb72,0x45c0, {0xa9,0x22,0xf4,0x58,0xfe,0x4,0xb,0xd5}}

struct grub_efi_edid_override;

typedef grub_efi_status_t
(*grub_efi_edid_override_get_edid) (struct grub_efi_edid_override *this,
				    grub_efi_handle_t *childhandle,
				    grub_efi_uint32_t *attributes,
				    grub_efi_uintn_t *edidsize,
				    grub_efi_uint8_t *edid);
struct grub_efi_edid_override {
  grub_efi_edid_override_get_edid get_edid;
};

typedef struct grub_efi_edid_override grub_efi_edid_override_t;
  

struct grub_efi_active_edid
{
  grub_uint32_t size_of_edid;
  grub_uint8_t *edid;
};

#endif
