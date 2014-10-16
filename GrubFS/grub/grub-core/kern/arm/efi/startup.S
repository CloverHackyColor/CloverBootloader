/*
 * (C) Copyright 2013 Free Software Foundation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */

#include <grub/symbol.h>

	.file 	"startup.S"
	.text
	.arm
FUNCTION(_start)
	/*
	 *  EFI_SYSTEM_TABLE and EFI_HANDLE are passed in r1/r0.
	 */
	ldr	ip, =EXT_C(grub_efi_image_handle)
	str	r0, [ip]
	ldr	ip, =EXT_C(grub_efi_system_table)
	str	r1, [ip]
	ldr	ip, =EXT_C(grub_main)
	bx	ip
	END
