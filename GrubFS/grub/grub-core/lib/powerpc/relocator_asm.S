/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2009,2010  Free Software Foundation, Inc.
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
	
	.p2align	4	/* force 16-byte alignment */

VARIABLE (grub_relocator_forward_start)
	mr 3, 9
	mr 4, 10

copycont1:	
	lbz 11,0(8)
	stb 11,0(9)
	addi 8, 8, 0x1
	addi 9, 9, 0x1
	addi 10, 10, -1
	cmpwi 10, 0
	bne copycont1

#include "../../kern/powerpc/cache_flush.S"

VARIABLE (grub_relocator_forward_end)

VARIABLE (grub_relocator_backward_start)
	mr 3, 9
	mr 4, 10

	add 9, 9, 10
	add 8, 8, 10
	/* Backward movsl is implicitly off-by-one.  compensate that.  */
	addi 9, 9, -1
	addi 8, 8, -1
copycont2:	
	lbz 11,0(8)
	stb 11,0(9)
	addi 8, 8, -1
	addi 9, 9, -1
	addi 10, 10, -1
	cmpwi 10, 0
	bne copycont2

#include "../../kern/powerpc/cache_flush.S"

VARIABLE (grub_relocator_backward_end)
