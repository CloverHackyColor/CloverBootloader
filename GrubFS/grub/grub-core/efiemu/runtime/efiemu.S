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

        .file   "efiemu.S"
	.text

FUNCTION (efiemu_get_time)
	push %rdi
	push %rsi
	mov %rcx, %rdi
	mov %rdx, %rsi
	call efiemu_get_time_real
	pop %rsi
	pop %rdi
	ret

FUNCTION (efiemu_set_time)
	push %rdi
	push %rsi
	mov %rcx, %rdi
	call efiemu_set_time_real
	pop %rsi
	pop %rdi
	ret


FUNCTION (efiemu_get_wakeup_time)
	push %rdi
	push %rsi
	mov %rcx, %rdi
	mov %rdx, %rsi
	mov %r8, %rdx
	call efiemu_get_wakeup_time_real
	pop %rsi
	pop %rdi
	ret

FUNCTION (efiemu_set_wakeup_time)
	push %rdi
	push %rsi
	mov %rcx, %rdi
	mov %rdx, %rsi
	call efiemu_set_wakeup_time_real
	pop %rsi
	pop %rdi
	ret

FUNCTION (efiemu_get_variable)
	push %rdi
	push %rsi
	mov %rcx, %rdi
	mov %rdx, %rsi
	mov %r8, %rdx
	mov %r9, %rcx
	mov 56(%rsp), %r8
	call efiemu_get_variable_real
	pop %rsi
	pop %rdi
	ret

FUNCTION (efiemu_get_next_variable_name)
	push %rdi
	push %rsi
	mov %rcx, %rdi
	mov %rdx, %rsi
	mov %r8, %rdx
	call efiemu_get_next_variable_name_real
	pop %rsi
	pop %rdi
	ret

FUNCTION (efiemu_set_variable)
	push %rdi
	push %rsi
	mov %rcx, %rdi
	mov %rdx, %rsi
	mov %r8, %rdx
	mov %r9, %rcx
	mov 56(%rsp), %r8
	call efiemu_set_variable_real
	pop %rsi
	pop %rdi
	ret

FUNCTION (efiemu_get_next_high_monotonic_count)
	push %rdi
	push %rsi
	mov %rcx, %rdi
	call efiemu_get_next_high_monotonic_count_real
	pop %rsi
	pop %rdi
	ret

FUNCTION (efiemu_reset_system)
	push %rdi
	push %rsi
	mov %rcx, %rdi
	mov %rdx, %rsi
	mov %r8, %rdx
	mov %r9, %rcx
	call efiemu_reset_system_real
	pop %rsi
	pop %rdi
	ret

	/* The following functions are always called in physical mode */
	.section ".text-physical", "ax"

FUNCTION (efiemu_set_virtual_address_map)
	push %rdi
	push %rsi
	mov %rcx, %rdi
	mov %rdx, %rsi
	mov %r8, %rdx
	mov %r9, %rcx
	call efiemu_set_virtual_address_map_real
	pop %rsi
	pop %rdi
	ret

FUNCTION (efiemu_convert_pointer)
	push %rdi
	push %rsi
	mov %rcx, %rdi
	mov %rdx, %rsi
	call efiemu_convert_pointer_real
	pop %rsi
	pop %rdi
	ret

