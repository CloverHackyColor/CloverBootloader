/* dl-x86_64.c - arch-dependent part of loadable module support */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2002,2005,2007,2009  Free Software Foundation, Inc.
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

#include <grub/dl.h>
#include <grub/elf.h>
#include <grub/misc.h>
#include <grub/err.h>
#include <grub/i18n.h>

/* Check if EHDR is a valid ELF header.  */
grub_err_t
grub_arch_dl_check_header (void *ehdr)
{
  Elf64_Ehdr *e = ehdr;

  /* Check the magic numbers.  */
  if (e->e_ident[EI_CLASS] != ELFCLASS64
      || e->e_ident[EI_DATA] != ELFDATA2LSB
      || e->e_machine != EM_X86_64)
    return grub_error (GRUB_ERR_BAD_OS, N_("invalid arch-dependent ELF magic"));

  return GRUB_ERR_NONE;
}

/* Relocate symbols.  */
grub_err_t
grub_arch_dl_relocate_symbols (grub_dl_t mod, void *ehdr,
			       Elf_Shdr *s, grub_dl_segment_t seg)
{
  Elf64_Rela *rel, *max;

  for (rel = (Elf64_Rela *) ((char *) ehdr + s->sh_offset),
	 max = (Elf64_Rela *) ((char *) rel + s->sh_size);
       rel < max;
       rel = (Elf64_Rela *) ((char *) rel + s->sh_entsize))
    {
      Elf64_Word *addr32;
      Elf64_Xword *addr64;
      Elf64_Sym *sym;

      if (seg->size < rel->r_offset)
	return grub_error (GRUB_ERR_BAD_MODULE,
			   "reloc offset is out of the segment");

      addr32 = (Elf64_Word *) ((char *) seg->addr + rel->r_offset);
      addr64 = (Elf64_Xword *) addr32;
      sym = (Elf64_Sym *) ((char *) mod->symtab
			   + mod->symsize * ELF_R_SYM (rel->r_info));

      switch (ELF_R_TYPE (rel->r_info))
	{
	case R_X86_64_64:
	  *addr64 += rel->r_addend + sym->st_value;
	  break;

	case R_X86_64_PC32:
	  {
	    grub_int64_t value;
	    value = ((grub_int32_t) *addr32) + rel->r_addend + sym->st_value -
	      (Elf64_Xword) seg->addr - rel->r_offset;
	    if (value != (grub_int32_t) value)
	      return grub_error (GRUB_ERR_BAD_MODULE, "relocation out of range");
	    *addr32 = value;
	  }
	  break;

	case R_X86_64_PC64:
	  {
	    *addr64 += rel->r_addend + sym->st_value -
	      (Elf64_Xword) seg->addr - rel->r_offset;
	  }
	  break;

	case R_X86_64_32:
	  {
	    grub_uint64_t value = *addr32 + rel->r_addend + sym->st_value;
	    if (value != (grub_uint32_t) value)
	      return grub_error (GRUB_ERR_BAD_MODULE, "relocation out of range");
	    *addr32 = value;
	  }
	  break;
	case R_X86_64_32S:
	  {
	    grub_int64_t value = ((grub_int32_t) *addr32) + rel->r_addend + sym->st_value;
	    if (value != (grub_int32_t) value)
	      return grub_error (GRUB_ERR_BAD_MODULE, "relocation out of range");
	    *addr32 = value;
	  }
	  break;

	default:
	  return grub_error (GRUB_ERR_NOT_IMPLEMENTED_YET,
			     N_("relocation 0x%x is not implemented yet"),
			     ELF_R_TYPE (rel->r_info));
	}
    }

  return GRUB_ERR_NONE;
}
