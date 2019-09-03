/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2005,2007,2009  Free Software Foundation, Inc.
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
#include <grub/dl.h>

        .file   "setjmp.S"

GRUB_MOD_LICENSE "GPLv3+"

        .text

/*
 * int grub_setjmp (grub_jmp_buf env)
 */
FUNCTION(grub_setjmp)
	stx	%o7, [%o0 + 0x00]
	stx	%sp, [%o0 + 0x08]
	stx	%fp, [%o0 + 0x10]
	retl
	 clr	%o0

/*
 * int grub_longjmp (grub_jmp_buf env, int val)
 */
FUNCTION(grub_longjmp)
	ldx	[%o0 + 0x10], %g1
	movrz	%o1, 1, %o1

	save %sp, -64, %sp
	flushw
	restore

	ldx	[%o0 + 0x00], %o7
	ldx	[%o0 + 0x08], %fp
	sub	%fp, 192, %sp
	stx	%g1, [%sp + 2047 + (14 * 8)]
	retl
	 restore %o1, 0, %o0
