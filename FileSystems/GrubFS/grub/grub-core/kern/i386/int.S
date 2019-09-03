/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2010,2011 Free Software Foundation, Inc.
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

FUNCTION(grub_bios_interrupt)
	pushf
	cli
	popf
	pushl    %ebp
	pushl    %ecx
	pushl    %eax
	pushl    %ebx
	pushl    %esi
	pushl    %edi	
	pushl    %edx
	
	movb     %al, intno
	movl	 (%edx), %eax
	movl	 %eax, LOCAL(bios_register_eax)
	movw	 4(%edx), %ax
	movw	 %ax, LOCAL(bios_register_es)
	movw	 6(%edx), %ax
	movw	 %ax, LOCAL(bios_register_ds)
	movw	 8(%edx), %ax
	movw	 %ax, LOCAL(bios_register_flags)

	movl 	12(%edx), %ebx
	movl 	16(%edx), %ecx
	movl 	20(%edx), %edi
	movl 	24(%edx), %esi
	movl 	28(%edx), %edx

	PROT_TO_REAL
	.code16
	pushf
	cli
	
	mov	%ds, %ax
	push	%ax

	/* movw imm16, %ax*/
	.byte	0xb8
LOCAL(bios_register_es):
	.short 	0
	movw	%ax, %es
	/* movw imm16, %ax*/
	.byte	0xb8
LOCAL(bios_register_ds):
	.short 	0
	movw	%ax, %ds

	/* movw imm16, %ax*/
	.byte	0xb8
LOCAL(bios_register_flags):
	.short 	0
	push	%ax
	popf

	/* movl imm32, %eax*/
	.byte	0x66, 0xb8
LOCAL(bios_register_eax):
	.long 	0
	
	/* int imm8.  */
	.byte   0xcd
intno:	
	.byte   0

	movl 	%eax, %cs:LOCAL(bios_register_eax)
	movw	%ds, %ax
	movw 	%ax, %cs:LOCAL(bios_register_ds)
	pop 	%ax
	mov	%ax, %ds
	pushf
	pop	%ax
	movw	%ax, LOCAL(bios_register_flags)
	mov 	%es, %ax
	movw	%ax, LOCAL(bios_register_es)

	popf
	REAL_TO_PROT
	.code32

	popl    %eax

	movl 	%ebx, 12(%eax)
	movl 	%ecx, 16(%eax)
	movl 	%edi, 20(%eax)
	movl 	%esi, 24(%eax)
	movl 	%edx, 28(%eax)

	movl     %eax, %edx

	movl	 LOCAL(bios_register_eax), %eax
	movl	 %eax, (%edx)
	movw	 LOCAL(bios_register_es), %ax
	movw	 %ax, 4(%edx)
	movw	 LOCAL(bios_register_ds), %ax
	movw	 %ax, 6(%edx)
	movw	 LOCAL(bios_register_flags), %ax
	movw	 %ax, 8(%edx)

	popl 	%edi
	popl 	%esi
	popl    %ebx
	popl    %eax
	popl    %ecx
	popl    %ebp
	ret
