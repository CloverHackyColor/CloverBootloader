/* startup.S - Startup code for the MIPS.  */
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
#include <grub/offsets.h>
#include <grub/machine/memory.h>
#include <grub/machine/kernel.h>
#include <grub/offsets.h>

#define BASE_ADDR 8	

	.globl __start, _start, start
	.set noreorder
	.set nomacro
__start:
_start:
start:		
.extern __bss_start
.extern _end
	bal cont
	 nop

	. = _start + GRUB_KERNEL_MACHINE_TOTAL_MODULE_SIZE
VARIABLE(grub_total_modules_size)
	.long 0

VARIABLE (grub_arch_busclock)
	.long 0
VARIABLE (grub_arch_cpuclock)
	.long 0
VARIABLE (grub_arch_memsize)
	.long 0
VARIABLE (grub_arch_highmemsize)
	.long 0
#ifdef GRUB_MACHINE_MIPS_LOONGSON
VARIABLE (grub_arch_machine)
	.long GRUB_ARCH_MACHINE_FULOONG2F
#endif
cont:
	/* Save our base.  */
	move $s0, $ra

#ifdef GRUB_MACHINE_MIPS_QEMU_MIPS
	lui $t1, %hi(grub_arch_busclock)
	addiu $t1, %lo(grub_arch_busclock)
	sw $s4, 8($t1)
#endif

#ifdef GRUB_MACHINE_MIPS_LOONGSON
	lui $t1, %hi(grub_arch_busclock)
	addiu $t1, %lo(grub_arch_busclock)
	sw $s2, 0($t1)
	sw $s3, 4($t1)
	sw $s4, 8($t1)
	sw $s5, 12($t1)
	sw $s7, 16($t1)
#endif

	/* Move the modules out of BSS.  */
	lui $t2, %hi(__bss_start)
	addiu $t2, %lo(__bss_start)
	
	lui $t1, %hi(_end)
	addiu $t1, %lo(_end)
	addiu $t1, (GRUB_KERNEL_MACHINE_MOD_ALIGN - 1)
	li $t3, (GRUB_KERNEL_MACHINE_MOD_ALIGN - 1)
	nor $t3, $t3, $0
	and $t1, $t1, $t3
	
	lw $t3, (GRUB_KERNEL_MACHINE_TOTAL_MODULE_SIZE - BASE_ADDR)($s0)

	/* Backward copy.  */
	add $t1, $t1, $t3
	add $t2, $t2, $t3
	addiu $t1, $t1, -1
	addiu $t2, $t2, -1

	/* $t2 is source. $t1 is destination. $t3 is size.  */
modulesmovcont:
	beq $t3, $0, modulesmovdone
         nop
	lb $t4, 0($t2)
	sb $t4, 0($t1)
	addiu $t2, $t2, -1
	addiu $t1, $t1, -1
	b modulesmovcont
	 addiu $t3, $t3, -1
modulesmovdone:

	/* Clean BSS.  */
	
	lui $t1, %hi(__bss_start)
	addiu $t1, $t1, %lo(__bss_start)
	lui $t2, %hi(_end)
	addiu $t2, $t2, %lo(_end)
bsscont:
	sb $0,0($t1)
	addiu $t1, $t1, 1
	sltu $t3, $t1, $t2
	bne $t3, $0, bsscont
         nop

	lui $t9, %hi(grub_main)
	addiu $t9, %lo(grub_main)

	lui $sp, %hi(GRUB_MACHINE_MEMORY_STACK_HIGH)
	jr $t9
	 addiu $sp, $sp, %lo(GRUB_MACHINE_MEMORY_STACK_HIGH)

