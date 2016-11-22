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

#include <grub/symbol.h>
	
	.p2align	4	/* force 16-byte alignment */

	.set noreorder
	.set nomacro

VARIABLE (grub_relocator_forward_start)
	move $a0, $9
	move $a1, $10

copycont1:	
	lb $11,0($8)
	sb $11,0($9)
	addiu $8, $8, 1
	addiu $10, $10, -1
	bne $10, $0, copycont1
	 addiu $9, $9, 1

#include "../../kern/mips/cache_flush.S"

VARIABLE (grub_relocator_forward_end)

VARIABLE (grub_relocator_backward_start)
	move $a0, $9
	move $a1, $10

	addu $9, $9, $10
	addu $8, $8, $10
	/* Backward movsl is implicitly off-by-one.  compensate that.  */
	addiu $9, $9, -1
	addiu $8, $8, -1
copycont2:	
	lb $11,0($8)
	sb $11,0($9)
	addiu $8, $8, -1
	addiu $10, $10, -1
	bne $10, $0, copycont2
	 addiu $9, $9, -1

#include "../../kern/mips/cache_flush.S"

VARIABLE (grub_relocator_backward_end)
