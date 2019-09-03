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
#include <grub/i386/memory.h>

	.p2align	2

VARIABLE(grub_relocator_backward_start)
	/* mov imm32, %rax */
	.byte	0x48
	.byte	0xb8
VARIABLE(grub_relocator_backward_dest)
	.long	0, 0
	movq	%rax, %rdi

	/* mov imm64, %rax */
	.byte	0x48
	.byte	0xb8
VARIABLE(grub_relocator_backward_src)
	.long	0, 0
	movq	%rax, %rsi

	/* mov imm64, %rcx */
	.byte	0x48
	.byte	0xb9
VARIABLE(grub_relocator_backward_chunk_size)
	.long	0, 0

	add	%rcx, %rsi
	add	%rcx, %rdi

	/* Backward movsb is implicitly off-by-one.  compensate that.  */
	sub	$1,	%rsi
	sub	$1,	%rdi

	/* Backward copy.  */
	std

	rep
	movsb
VARIABLE(grub_relocator_backward_end)


VARIABLE(grub_relocator_forward_start)
	/* mov imm64, %rax */
	.byte	0x48
	.byte	0xb8
VARIABLE(grub_relocator_forward_dest)
	.long	0, 0
	movq	%rax, %rdi

	/* mov imm64, %rax */
	.byte	0x48
	.byte	0xb8
VARIABLE(grub_relocator_forward_src)
	.long	0, 0
	movq	%rax, %rsi

	/* mov imm64, %rcx */
	.byte	0x48
	.byte	0xb9
VARIABLE(grub_relocator_forward_chunk_size)
	.long	0, 0

	/* Forward copy.  */
	cld
	rep
	movsb
VARIABLE(grub_relocator_forward_end)
