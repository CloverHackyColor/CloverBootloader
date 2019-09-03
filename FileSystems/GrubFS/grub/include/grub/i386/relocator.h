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

#ifndef GRUB_RELOCATOR_CPU_HEADER
#define GRUB_RELOCATOR_CPU_HEADER	1

#include <grub/types.h>
#include <grub/err.h>
#include <grub/relocator.h>

struct grub_relocator32_state
{
  grub_uint32_t esp;
  grub_uint32_t ebp;
  grub_uint32_t eax;
  grub_uint32_t ebx;
  grub_uint32_t ecx;
  grub_uint32_t edx;
  grub_uint32_t eip;
  grub_uint32_t esi;
  grub_uint32_t edi;
};

struct grub_relocator16_state
{
  grub_uint16_t cs;
  grub_uint16_t ds;
  grub_uint16_t es;
  grub_uint16_t fs;
  grub_uint16_t gs;
  grub_uint16_t ss;
  grub_uint16_t sp;
  grub_uint16_t ip;
  grub_uint32_t ebx;
  grub_uint32_t edx;
  grub_uint32_t esi;
  grub_uint32_t ebp;
  int a20;
};

struct grub_relocator64_state
{
  grub_uint64_t rsp;
  grub_uint64_t rax;
  grub_uint64_t rbx;
  grub_uint64_t rcx;
  grub_uint64_t rdx;
  grub_uint64_t rip;
  grub_uint64_t rsi;
  grub_addr_t cr3;
};

grub_err_t grub_relocator16_boot (struct grub_relocator *rel,
				  struct grub_relocator16_state state);

grub_err_t grub_relocator32_boot (struct grub_relocator *rel,
				  struct grub_relocator32_state state,
				  int avoid_efi_bootservices);

grub_err_t grub_relocator64_boot (struct grub_relocator *rel,
				  struct grub_relocator64_state state,
				  grub_addr_t min_addr, grub_addr_t max_addr);

#endif /* ! GRUB_RELOCATOR_CPU_HEADER */
