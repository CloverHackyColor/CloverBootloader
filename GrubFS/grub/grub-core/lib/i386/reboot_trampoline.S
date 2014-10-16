/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2011  Free Software Foundation, Inc.
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
#include <grub/i386/reboot.h>

	.p2align 4

VARIABLE(grub_reboot_start)
	.code16
	
	/* set 0x472 to 0x0000 for cold boot (0x1234 for warm boot) */
	movw	$0x0472, %di
	xorw	%ax, %ax
	movw	%ax, (%di)
	ljmp	$0xf000, $0xfff0

	.code32
VARIABLE(grub_reboot_end)
