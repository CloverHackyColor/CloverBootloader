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

#ifndef GRUB_EFI_EMU_RUNTIME_HEADER
#define GRUB_EFI_EMU_RUNTIME_HEADER	1

struct grub_efiemu_ptv_rel
{
  grub_uint64_t addr;
  grub_efi_memory_type_t plustype;
  grub_efi_memory_type_t minustype;
  grub_uint32_t size;
} GRUB_PACKED;

struct efi_variable
{
  grub_efi_guid_t guid;
  grub_uint32_t namelen;
  grub_uint32_t size;
  grub_efi_uint32_t attributes;
} GRUB_PACKED;
#endif /* ! GRUB_EFI_EMU_RUNTIME_HEADER */
