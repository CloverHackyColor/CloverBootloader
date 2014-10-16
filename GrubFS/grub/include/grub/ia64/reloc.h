/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2013  Free Software Foundation, Inc.
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

#ifndef GRUB_IA64_RELOC_H
#define GRUB_IA64_RELOC_H 1

struct grub_ia64_trampoline;

void
grub_ia64_add_value_to_slot_20b (grub_addr_t addr, grub_uint32_t value);
void
grub_ia64_add_value_to_slot_21 (grub_addr_t addr, grub_uint32_t value);
void
grub_ia64_make_trampoline (struct grub_ia64_trampoline *tr, grub_uint64_t addr);

struct grub_ia64_trampoline
{
  /* nop.m */
  grub_uint8_t nop[5];
  /* movl r15 = addr*/
  grub_uint8_t addr_hi[6];
  grub_uint8_t e0;
  grub_uint8_t addr_lo[4];
  grub_uint8_t jump[0x20];
};

#endif
