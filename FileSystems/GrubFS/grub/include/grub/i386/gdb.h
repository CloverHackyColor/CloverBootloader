/* i386/gdb.h - i386 specific definitions for the remote GDB stub */
/*
 *  Copyright (C) 2006	Lubomir Kundrak
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef GRUB_GDB_CPU_HEADER
#define GRUB_GDB_CPU_HEADER	1

#define GRUB_GDB_LAST_TRAP	31
/* You may have to edit the bottom of machdep.S when adjusting
   GRUB_GDB_LAST_TRAP.	*/
#define GRUB_MACHINE_NR_REGS	16

#define	EAX	0
#define	ECX	1
#define	EDX	2
#define	EBX	3
#define	ESP	4
#define	EBP	5
#define	ESI	6
#define	EDI	7
#define	EIP	8
#define	EFLAGS	9
#define	CS	10
#define	SS	11
#define	DS	12
#define	ES	13
#define	FS	14
#define	GS	15

#define PC	EIP
#define FP	EBP
#define SP	ESP
#define PS	EFLAGS

#ifndef ASM_FILE

#include <grub/gdb.h>

#define GRUB_CPU_TRAP_GATE	15

struct grub_cpu_interrupt_gate
{
  grub_uint16_t offset_lo;
  grub_uint16_t selector;
  grub_uint8_t unused;
  grub_uint8_t gate;
  grub_uint16_t offset_hi;
} GRUB_PACKED;

struct grub_cpu_idt_descriptor
{
  grub_uint16_t limit;
  grub_uint32_t base;
} GRUB_PACKED;

extern void (*grub_gdb_trapvec[]) (void);
void grub_gdb_idtinit (void);
void grub_gdb_idtrestore (void);
void grub_gdb_trap (int trap_no) __attribute__ ((regparm(3)));

#endif /* ! ASM */
#endif /* ! GRUB_GDB_CPU_HEADER */

