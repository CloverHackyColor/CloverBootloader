/* dl.c - arch-dependent part of loadable module support */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2002,2004,2005,2007,2009  Free Software Foundation, Inc.
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
#include <grub/mm.h>
#include <grub/i18n.h>
#include <grub/ia64/reloc.h>

#define MASK19 ((1 << 19) - 1)
#define MASK20 ((1 << 20) - 1)

/* Check if EHDR is a valid ELF header.  */
grub_err_t
grub_arch_dl_check_header (void *ehdr)
{
  Elf_Ehdr *e = ehdr;

  /* Check the magic numbers.  */
  if (e->e_ident[EI_CLASS] != ELFCLASS64
      || e->e_ident[EI_DATA] != ELFDATA2LSB
      || e->e_machine != EM_IA_64)
    return grub_error (GRUB_ERR_BAD_OS, N_("invalid arch-dependent ELF magic"));

  return GRUB_ERR_NONE;
}

#pragma GCC diagnostic ignored "-Wcast-align"

/* Relocate symbols.  */
grub_err_t
grub_arch_dl_relocate_symbols (grub_dl_t mod, void *ehdr,
			       Elf_Shdr *s, grub_dl_segment_t seg)
{
  Elf_Rela *rel, *max;

  for (rel = (Elf_Rela *) ((char *) ehdr + s->sh_offset),
	 max = (Elf_Rela *) ((char *) rel + s->sh_size);
       rel < max;
       rel = (Elf_Rela *) ((char *) rel + s->sh_entsize))
    {
      grub_addr_t addr;
      Elf_Sym *sym;
      grub_uint64_t value;

      if (seg->size < (rel->r_offset & ~3))
	return grub_error (GRUB_ERR_BAD_MODULE,
			   "reloc offset is out of the segment");

      addr = (grub_addr_t) seg->addr + rel->r_offset;
      sym = (Elf_Sym *) ((char *) mod->symtab
			 + mod->symsize * ELF_R_SYM (rel->r_info));

      /* On the PPC the value does not have an explicit
	 addend, add it.  */
      value = sym->st_value + rel->r_addend;

      switch (ELF_R_TYPE (rel->r_info))
	{
	case R_IA64_PCREL21B:
	  {
	    grub_int64_t noff;
	    if (ELF_ST_TYPE (sym->st_info) == STT_FUNC)
	      {
		struct grub_ia64_trampoline *tr = mod->trampptr;
		grub_ia64_make_trampoline (tr, value);
		noff = ((char *) tr - (char *) (addr & ~3)) >> 4;
		mod->trampptr = tr + 1;
	      }
	    else
	      noff = ((char *) value - (char *) (addr & ~3)) >> 4;

	    if ((noff & ~MASK19) && ((-noff) & ~MASK19))
	      return grub_error (GRUB_ERR_BAD_MODULE,
				 "jump offset too big (%lx)", noff);
	    grub_ia64_add_value_to_slot_20b (addr, noff);
	  }
	  break;
	case R_IA64_SEGREL64LSB:
	  *(grub_uint64_t *) addr += value - (grub_addr_t) seg->addr;
	  break;
	case R_IA64_FPTR64LSB:
	case R_IA64_DIR64LSB:
	  *(grub_uint64_t *) addr += value;
	  break;
	case R_IA64_PCREL64LSB:
	  *(grub_uint64_t *) addr += value - addr;
	  break;
	case R_IA64_GPREL22:
	  if ((value - (grub_addr_t) mod->base) & ~MASK20)
	    return grub_error (GRUB_ERR_BAD_MODULE,
			       "gprel offset too big (%lx)",
			       value - (grub_addr_t) mod->base);
	  grub_ia64_add_value_to_slot_21 (addr, value - (grub_addr_t) mod->base);
	  break;

	case R_IA64_LTOFF22X:
	case R_IA64_LTOFF22:
	  if (ELF_ST_TYPE (sym->st_info) == STT_FUNC)
	    value = *(grub_uint64_t *) sym->st_value + rel->r_addend;
	case R_IA64_LTOFF_FPTR22:
	  {
	    grub_uint64_t *gpptr = mod->gotptr;
	    *gpptr = value;
	    if (((grub_addr_t) gpptr - (grub_addr_t) mod->base) & ~MASK20)
	      return grub_error (GRUB_ERR_BAD_MODULE,
				 "gprel offset too big (%lx)",
				 (grub_addr_t) gpptr - (grub_addr_t) mod->base);
	    grub_ia64_add_value_to_slot_21 (addr, (grub_addr_t) gpptr - (grub_addr_t) mod->base);
	    mod->gotptr = gpptr + 1;
	    break;
	  }
	  /* We treat LTOFF22X as LTOFF22, so we can ignore LDXMOV.  */
	case R_IA64_LDXMOV:
	  break;
	default:
	  return grub_error (GRUB_ERR_NOT_IMPLEMENTED_YET,
			     N_("relocation 0x%x is not implemented yet"),
			     ELF_R_TYPE (rel->r_info));
	}
    }
  return GRUB_ERR_NONE;
}
