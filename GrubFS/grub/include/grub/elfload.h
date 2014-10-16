/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2003, 2004, 2005, 2006, 2007  Free Software Foundation, Inc.
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

#ifndef GRUB_ELFLOAD_HEADER
#define GRUB_ELFLOAD_HEADER	1

#include <grub/err.h>
#include <grub/elf.h>
#include <grub/file.h>
#include <grub/symbol.h>
#include <grub/types.h>

struct grub_elf_file
{
  grub_file_t file;
  union {
    Elf64_Ehdr ehdr64;
    Elf32_Ehdr ehdr32;
  } ehdr;
  void *phdrs;
  char *filename;
};
typedef struct grub_elf_file *grub_elf_t;

typedef int (*grub_elf32_phdr_iterate_hook_t)
  (grub_elf_t elf, Elf32_Phdr *phdr, void *arg);
typedef int (*grub_elf64_phdr_iterate_hook_t)
  (grub_elf_t elf, Elf64_Phdr *phdr, void *arg);

grub_elf_t grub_elf_open (const char *);
grub_elf_t grub_elf_file (grub_file_t file, const char *filename);
grub_err_t grub_elf_close (grub_elf_t);

int grub_elf_is_elf32 (grub_elf_t);
grub_size_t grub_elf32_size (grub_elf_t,
			     Elf32_Addr *, grub_uint32_t *);
enum grub_elf_load_flags
  {
    GRUB_ELF_LOAD_FLAGS_NONE = 0,
    GRUB_ELF_LOAD_FLAGS_LOAD_PT_DYNAMIC = 1,
    GRUB_ELF_LOAD_FLAGS_BITS = 6,
    GRUB_ELF_LOAD_FLAGS_ALL_BITS = 0,
    GRUB_ELF_LOAD_FLAGS_28BITS = 2,
    GRUB_ELF_LOAD_FLAGS_30BITS = 4,
    GRUB_ELF_LOAD_FLAGS_62BITS = 6,
  };
grub_err_t grub_elf32_load (grub_elf_t, const char *filename,
			    void *load_offset, enum grub_elf_load_flags flags, grub_addr_t *,
			    grub_size_t *);

int grub_elf_is_elf64 (grub_elf_t);
grub_size_t grub_elf64_size (grub_elf_t,
			     Elf64_Addr *, grub_uint64_t *);
grub_err_t grub_elf64_load (grub_elf_t, const char *filename,
			    void *load_offset, enum grub_elf_load_flags flags, grub_addr_t *,
			    grub_size_t *);
grub_err_t grub_elf32_load_phdrs (grub_elf_t elf);
grub_err_t grub_elf64_load_phdrs (grub_elf_t elf);

#define FOR_ELF32_PHDRS(elf, phdr) \
  for (grub_elf32_load_phdrs (elf), phdr = elf->phdrs; \
       phdr && phdr < (Elf32_Phdr *) elf->phdrs + elf->ehdr.ehdr32.e_phnum; phdr++)
#define FOR_ELF64_PHDRS(elf, phdr) \
  for (grub_elf64_load_phdrs (elf), phdr = elf->phdrs;		\
       phdr && phdr < (Elf64_Phdr *) elf->phdrs + elf->ehdr.ehdr64.e_phnum; phdr++)

#endif /* ! GRUB_ELFLOAD_HEADER */
