/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2013  Free Software Foundation, Inc.
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

#include <grub/symbol.h>

	.file 	"startup.S"
	.text
FUNCTION(_start)
	/*
	 *  EFI_SYSTEM_TABLE and EFI_HANDLE are passed in x1/x0.
	 */
	ldr	x2, efi_image_handle_val
	str	x0, [x2]
	ldr	x2, efi_system_table_val
	str	x1, [x2]
	ldr	x2, grub_main_val
	br	x2
grub_main_val:
	.quad	EXT_C(grub_main)
efi_system_table_val:
	.quad	EXT_C(grub_efi_system_table)
efi_image_handle_val:
	.quad	EXT_C(grub_efi_image_handle)

