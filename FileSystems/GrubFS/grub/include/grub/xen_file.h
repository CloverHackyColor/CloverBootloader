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

#ifndef GRUB_XEN_FILE_HEADER
#define GRUB_XEN_FILE_HEADER 1

#include <grub/types.h>
#include <grub/elf.h>
#include <grub/elfload.h>

grub_elf_t grub_xen_file (grub_file_t file);

struct grub_xen_file_info
{
  grub_uint64_t kern_start, kern_end;
  grub_uint64_t virt_base;
  grub_uint64_t entry_point;
  grub_uint64_t hypercall_page;
  grub_uint64_t paddr_offset;
  int has_hypercall_page;
  int has_note;
  int has_xen_guest;
  int extended_cr3;
  enum
  {
    GRUB_XEN_FILE_I386 = 1,
    GRUB_XEN_FILE_I386_PAE = 2,
    GRUB_XEN_FILE_I386_PAE_BIMODE = 3,
    GRUB_XEN_FILE_X86_64 = 4
  } arch;
};

grub_err_t
grub_xen_get_info32 (grub_elf_t elf, struct grub_xen_file_info *xi);
grub_err_t
grub_xen_get_info64 (grub_elf_t elf, struct grub_xen_file_info *xi);
grub_err_t grub_xen_get_info (grub_elf_t elf, struct grub_xen_file_info *xi);

#endif
