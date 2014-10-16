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

#ifndef GRUB_INTERRUPT_MACHINE_HEADER
#define GRUB_INTERRUPT_MACHINE_HEADER	1

#include <grub/symbol.h>
#include <grub/types.h>

struct grub_bios_int_registers
{
  grub_uint32_t eax;
  grub_uint16_t es;
  grub_uint16_t ds;
  grub_uint16_t flags;
  grub_uint16_t dummy;
  grub_uint32_t ebx;
  grub_uint32_t ecx;
  grub_uint32_t edi;
  grub_uint32_t esi;
  grub_uint32_t edx;
};

#define  GRUB_CPU_INT_FLAGS_CARRY     0x1
#define  GRUB_CPU_INT_FLAGS_PARITY    0x4
#define  GRUB_CPU_INT_FLAGS_ADJUST    0x10
#define  GRUB_CPU_INT_FLAGS_ZERO      0x40
#define  GRUB_CPU_INT_FLAGS_SIGN      0x80
#define  GRUB_CPU_INT_FLAGS_TRAP      0x100
#define  GRUB_CPU_INT_FLAGS_INTERRUPT 0x200
#define  GRUB_CPU_INT_FLAGS_DIRECTION 0x400
#define  GRUB_CPU_INT_FLAGS_OVERFLOW  0x800
#ifdef GRUB_MACHINE_PCBIOS
#define  GRUB_CPU_INT_FLAGS_DEFAULT   GRUB_CPU_INT_FLAGS_INTERRUPT
#else
#define  GRUB_CPU_INT_FLAGS_DEFAULT   0
#endif

void EXPORT_FUNC (grub_bios_interrupt) (grub_uint8_t intno,
					struct grub_bios_int_registers *regs)
     __attribute__ ((regparm(3)));
struct grub_i386_idt
{
  grub_uint16_t limit;
  grub_uint32_t base;
} GRUB_PACKED;

#ifdef GRUB_MACHINE_PCBIOS
extern struct grub_i386_idt *EXPORT_VAR(grub_realidt);
#endif

#endif
