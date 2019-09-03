/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 1999,2000,2001,2002,2003,2005,2006,2007,2008,2009,2011 Free Software Foundation, Inc.
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


/*
 * Note: These functions defined in this file may be called from C.
 *       Be careful of that you must not modify some registers. Quote
 *       from gcc-2.95.2/gcc/config/i386/i386.h:

   1 for registers not available across function calls.
   These must include the FIXED_REGISTERS and also any
   registers that can be used without being saved.
   The latter must include the registers where values are returned
   and the register where structure-value addresses are passed.
   Aside from that, you can include as many other registers as you like.

  ax,dx,cx,bx,si,di,bp,sp,st,st1,st2,st3,st4,st5,st6,st7,arg
{  1, 1, 1, 0, 0, 0, 0, 1, 1,  1,  1,  1,  1,  1,  1,  1,  1 }
 */

/*
 * Note: GRUB is compiled with the options -mrtd and -mregparm=3.
 *       So the first three arguments are passed in %eax, %edx, and %ecx,
 *       respectively, and if a function has a fixed number of arguments
 *       and the number is greater than three, the function must return
 *       with "ret $N" where N is ((the number of arguments) - 3) * 4.
 */

#include <config.h>
#include <grub/symbol.h>
#include <multiboot.h>
#ifdef __APPLE__
#include <grub/i386/pc/memory.h>
#endif

	.file	"startup.S"

	.text

	.globl	start, _start, __start
start:
_start:
__start:
#ifdef __APPLE__
LOCAL(start):
#endif
	.code32

	movl	%ecx, (LOCAL(real_to_prot_addr) - _start) (%esi)
	movl	%edi, (LOCAL(prot_to_real_addr) - _start) (%esi)
	movl	%eax, (EXT_C(grub_realidt) - _start) (%esi)

	/* copy back the decompressed part (except the modules) */
#ifdef __APPLE__
	movl	$EXT_C(_edata), %ecx
	subl    $LOCAL(start), %ecx
#else
	movl	$(_edata - _start), %ecx
#endif
	movl	$(_start), %edi
	rep
	movsb

	movl	$LOCAL (cont), %esi
	jmp	*%esi
LOCAL(cont):

#if 0
	/* copy modules before cleaning out the bss */
	movl	EXT_C(grub_total_module_size), %ecx
	movl	EXT_C(grub_kernel_image_size), %esi
	addl	%ecx, %esi
	addl	$_start, %esi
	decl	%esi
	movl	$END_SYMBOL, %edi
	addl	%ecx, %edi
	decl	%edi
	std
	rep
	movsb
#endif

#ifdef __APPLE__
	/* clean out the bss */
	movl	$EXT_C(_edata), %edi

	/* compute the bss length */
	movl	$GRUB_MEMORY_MACHINE_SCRATCH_ADDR, %ecx
#else
	/* clean out the bss */
	movl	$BSS_START_SYMBOL, %edi

	/* compute the bss length */
	movl	$END_SYMBOL, %ecx
#endif
	subl	%edi, %ecx

	/* clean out */
	xorl	%eax, %eax
	cld
	rep
	stosb

	movl	%edx, EXT_C(grub_boot_device)

	/*
	 *  Call the start of main body of C code.
	 */
	call EXT_C(grub_main)

LOCAL(real_to_prot_addr):
	.long 0
LOCAL(prot_to_real_addr):
	.long 0

	.macro PROT_TO_REAL
	movl	LOCAL(prot_to_real_addr), %eax
	call	*%eax
	.endm

	.macro REAL_TO_PROT
	movl	LOCAL(real_to_prot_addr), %eax
	DATA32	call	*%ax
	.endm

/*
 * grub_exit()
 *
 * Exit the system.
 */
FUNCTION(grub_exit)
	PROT_TO_REAL
	.code16
	/* Tell the BIOS a boot failure. If this does not work, reboot.  */
	int	$0x18
	/* set 0x472 to 0x0000 for cold boot (0x1234 for warm boot) */
	xorw	%ax, %ax
	movw	$0x0472, %di
	movw	%ax, (%di)
	ljmp	$0xf000, $0xfff0
	.code32

/*
 * int grub_pxe_call (int func, void* data, grub_uint32_t pxe_rm_entry);
 */
FUNCTION(grub_pxe_call)
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%edi
	pushl	%ebx

	movl	%ecx, %ebx
	movl	%eax, %ecx
	movl	%edx, %eax
	andl	$0xF, %eax
	shrl	$4, %edx
	shll	$16, %edx
	addl	%eax, %edx

	PROT_TO_REAL
	.code16

	pushl	%ebx
	pushl	%edx
	pushw	%cx
	movw	%sp, %bx
	lcall	*%ss:6(%bx)
	cld
	addw	$10, %sp
	movw	%ax, %cx

	REAL_TO_PROT
	.code32

	movzwl	%cx, %eax

	popl	%ebx
	popl	%edi
	popl	%esi
	popl	%ebp
	ret

#include "../int.S"

VARIABLE(grub_realidt)
	.long 0

#ifdef __APPLE__
	/* Older versions of objconv assume that there is the same number
	   of text and data sections. Hence this dummy.  */
	.section __TEXT, __zz_dummy
	.byte 0
	.globl EXT_C(_edata)
	.globl EXT_C(grub_boot_device)
	.zerofill __DATA, __aa_before_bss, EXT_C(_edata), 1, 0
	.zerofill __DATA, __bss, EXT_C(grub_boot_device), 4, 2
#else
	.bss
VARIABLE(grub_boot_device)
	.long	0
#endif
