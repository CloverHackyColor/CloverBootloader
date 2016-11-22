/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2004,2007  Free Software Foundation, Inc.
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

	.file	"setjmp.S"

GRUB_MOD_LICENSE "GPLv3+"

	.text

/*
 * int grub_setjmp (grub_jmp_buf env)
 */
FUNCTION(grub_setjmp)
	stw	1, 0(3)
	stw	14, 4(3)
	stw	15, 8(3)
	stw	16, 12(3)
	stw	17, 16(3)
	stw	18, 20(3)
	stw	19, 24(3)
	stw	20, 28(3)
	stw	21, 32(3)
	stw	22, 36(3)
	stw	23, 40(3)
	stw	24, 44(3)
	stw	25, 48(3)
	stw	26, 52(3)
	stw	27, 56(3)
	stw	28, 60(3)
	stw	29, 64(3)
	stw	30, 68(3)
	stw	31, 72(3)
	mflr	4
	stw	4, 76(3)
	mfcr	4
	stw	4, 80(3)
	li	3, 0
	blr

/*
 * int grub_longjmp (grub_jmp_buf env, int val)
 */
FUNCTION(grub_longjmp)
	lwz	1, 0(3)
	lwz	14, 4(3)
	lwz	15, 8(3)
	lwz	16, 12(3)
	lwz	17, 16(3)
	lwz	18, 20(3)
	lwz	19, 24(3)
	lwz	20, 28(3)
	lwz	21, 32(3)
	lwz	22, 36(3)
	lwz	23, 40(3)
	lwz	24, 44(3)
	lwz	25, 48(3)
	lwz	26, 52(3)
	lwz	27, 56(3)
	lwz	28, 60(3)
	lwz	29, 64(3)
	lwz	30, 68(3)
	lwz	31, 72(3)
	lwz	5, 76(3)
	mtlr	5
	lwz	5, 80(3)
	mtcr	5
	mr.	3, 4
	bne	1f
	li	3, 1
1:	blr

