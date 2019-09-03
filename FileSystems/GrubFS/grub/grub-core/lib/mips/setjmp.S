/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2003,2007,2009  Free Software Foundation, Inc.
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
	sw $s0, 0($a0)
	sw $s1, 4($a0)
	sw $s2, 8($a0)
	sw $s3, 12($a0)
	sw $s4, 16($a0)
	sw $s5, 20($a0)
	sw $s6, 24($a0)
	sw $s7, 28($a0)
	sw $s8, 32($a0)
	sw $gp, 36($a0)
	sw $sp, 40($a0)
	sw $ra, 44($a0)
	move $v0, $zero
	move $v1, $zero
	jr $ra
/*
 * int grub_longjmp (grub_jmp_buf env, int val)
 */
FUNCTION(grub_longjmp)
	lw $s0, 0($a0)
	lw $s1, 4($a0)
	lw $s2, 8($a0)
	lw $s3, 12($a0)
	lw $s4, 16($a0)
	lw $s5, 20($a0)
	lw $s6, 24($a0)
	lw $s7, 28($a0)
	lw $s8, 32($a0)
	lw $gp, 36($a0)
	lw $sp, 40($a0)
	lw $ra, 44($a0)
	move $v0, $a1
	bne  $v0, $zero, 1f
	addiu $v0, $v0, 1
1:	
	move $v1, $zero
	jr $ra
