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

#ifndef GRUB_ARM_RELOC_H
#define GRUB_ARM_RELOC_H 1

grub_err_t grub_arm_reloc_abs32 (grub_uint32_t *addr, Elf32_Addr sym_addr);

int
grub_arm_thm_call_check_offset (grub_int32_t offset, int is_thumb);
grub_int32_t
grub_arm_thm_call_get_offset (grub_uint16_t *target);
grub_err_t
grub_arm_thm_call_set_offset (grub_uint16_t *target, grub_int32_t offset);

grub_int32_t
grub_arm_thm_jump19_get_offset (grub_uint16_t *target);
void
grub_arm_thm_jump19_set_offset (grub_uint16_t *target, grub_int32_t offset);
int
grub_arm_thm_jump19_check_offset (grub_int32_t offset);

grub_int32_t
grub_arm_jump24_get_offset (grub_uint32_t *target);
int
grub_arm_jump24_check_offset (grub_int32_t offset);
void
grub_arm_jump24_set_offset (grub_uint32_t *target,
			    grub_int32_t offset);

#endif
