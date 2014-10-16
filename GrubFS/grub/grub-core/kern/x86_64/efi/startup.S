/* startup.S - bootstrap GRUB itself */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2006,2007,2009  Free Software Foundation, Inc.
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

#include <config.h>
#include <grub/symbol.h>

        .file   "startup.S"
        .text
        .globl  start, _start
        .code64

start:
_start:
	movq	%rcx, EXT_C(grub_efi_image_handle)(%rip)
	movq	%rdx, EXT_C(grub_efi_system_table)(%rip)

	andq	$~0xf, %rsp
	call	EXT_C(grub_main)
	/* Doesn't return.  */
