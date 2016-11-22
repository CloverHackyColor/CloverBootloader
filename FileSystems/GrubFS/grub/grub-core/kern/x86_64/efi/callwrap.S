/* callwrap.S - wrapper for x86_64 efi calls */
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

/*
 * x86_64 uses registry to pass parameters. Unfortunately, gcc and efi use
 * different call conversion, so we need to do some conversion.
 *
 * gcc:
 *   %rdi,  %rsi,  %rdx,  %rcx, %r8, %r9, 8(%rsp), 16(%rsp), ...
 *
 * efi:
 *   %rcx,  %rdx,  %r8,  %r9,  32(%rsp), 40(%rsp), 48(%rsp), ...
 *
 */

        .file   "callwrap.S"
        .text

FUNCTION(efi_wrap_0)
	subq $40, %rsp
	call *%rdi
	addq $40, %rsp
	ret

FUNCTION(efi_wrap_1)
	subq $40, %rsp
	mov  %rsi, %rcx
	call *%rdi
	addq $40, %rsp
	ret

FUNCTION(efi_wrap_2)
	subq $40, %rsp
	mov  %rsi, %rcx
	call *%rdi
	addq $40, %rsp
	ret

FUNCTION(efi_wrap_3)
	subq $40, %rsp
	mov  %rcx, %r8
	mov  %rsi, %rcx
	call *%rdi
	addq $40, %rsp
	ret

FUNCTION(efi_wrap_4)
	subq $40, %rsp
	mov %r8, %r9
	mov %rcx, %r8
	mov %rsi, %rcx
	call *%rdi
	addq $40, %rsp
	ret

FUNCTION(efi_wrap_5)
	subq $40, %rsp
	mov %r9, 32(%rsp)
	mov %r8, %r9
	mov %rcx, %r8
	mov %rsi, %rcx
	call *%rdi
	addq $40, %rsp
	ret

FUNCTION(efi_wrap_6)
	subq $56, %rsp
	mov 56+8(%rsp), %rax
	mov %rax, 40(%rsp)
	mov %r9, 32(%rsp)
	mov %r8, %r9
	mov %rcx, %r8
	mov %rsi, %rcx
	call *%rdi
	addq $56, %rsp
	ret

FUNCTION(efi_wrap_7)
	subq $88, %rsp
	mov 88+16(%rsp), %rax
	mov %rax, 48(%rsp)
	mov 88+8(%rsp), %rax
	mov %rax, 40(%rsp)
	mov %r9, 32(%rsp)
	mov %r8, %r9
	mov %rcx, %r8
	mov %rsi, %rcx
	call *%rdi
	addq $88, %rsp
	ret

FUNCTION(efi_wrap_10)
	subq $88, %rsp
	mov 88+40(%rsp), %rax
	mov %rax, 72(%rsp)
	mov 88+32(%rsp), %rax
	mov %rax, 64(%rsp)
	mov 88+24(%rsp), %rax
	mov %rax, 56(%rsp)
	mov 88+16(%rsp), %rax
	mov %rax, 48(%rsp)
	mov 88+8(%rsp), %rax
	mov %rax, 40(%rsp)
	mov %r9, 32(%rsp)
	mov %r8, %r9
	mov %rcx, %r8
	mov %rsi, %rcx
	call *%rdi
	addq $88, %rsp
	ret
