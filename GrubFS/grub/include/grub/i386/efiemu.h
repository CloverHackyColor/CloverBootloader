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

#ifndef GRUB_ARCH_EFI_EMU_HEADER
#define GRUB_ARCH_EFI_EMU_HEADER	1

grub_err_t
grub_arch_efiemu_relocate_symbols32 (grub_efiemu_segment_t segs,
				     struct grub_efiemu_elf_sym *elfsyms,
				     void *ehdr);
grub_err_t
grub_arch_efiemu_relocate_symbols64 (grub_efiemu_segment_t segs,
				     struct grub_efiemu_elf_sym *elfsyms,
				     void *ehdr);

int grub_arch_efiemu_check_header32 (void *ehdr);
int grub_arch_efiemu_check_header64 (void *ehdr);
#endif
