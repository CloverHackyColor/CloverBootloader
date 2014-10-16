/* dl.c - arch-dependent part of loadable module support */
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

#include <grub/dl.h>
#include <grub/elf.h>
#include <grub/misc.h>
#include <grub/err.h>
#include <grub/mm.h>
#include <grub/i18n.h>
#include <grub/cpu/reloc.h>

struct trampoline
{
#define LDR 0x58000050
#define BR 0xd61f0200
  grub_uint32_t ldr; /* ldr	x16, 8 */
  grub_uint32_t br; /* br x16 */
  grub_uint64_t addr;
};

/*
 * Check if EHDR is a valid ELF header.
 */
grub_err_t
grub_arch_dl_check_header (void *ehdr)
{
  Elf_Ehdr *e = ehdr;

  /* Check the magic numbers.  */
  if (e->e_ident[EI_CLASS] != ELFCLASS64
      || e->e_ident[EI_DATA] != ELFDATA2LSB || e->e_machine != EM_AARCH64)
    return grub_error (GRUB_ERR_BAD_OS,
		       N_("invalid arch-dependent ELF magic"));

  return GRUB_ERR_NONE;
}

#pragma GCC diagnostic ignored "-Wcast-align"

grub_err_t
grub_arch_dl_get_tramp_got_size (const void *ehdr, grub_size_t *tramp,
				 grub_size_t *got)
{
  const Elf_Ehdr *e = ehdr;
  const Elf_Shdr *s;
  unsigned i;

  *tramp = 0;
  *got = 0;

  for (i = 0, s = (const Elf_Shdr *) ((grub_addr_t) e + e->e_shoff);
       i < e->e_shnum;
       i++, s = (const Elf_Shdr *) ((grub_addr_t) s + e->e_shentsize))
    if (s->sh_type == SHT_REL || s->sh_type == SHT_RELA)
      {
	const Elf_Rel *rel, *max;

	for (rel = (const Elf_Rel *) ((grub_addr_t) e + s->sh_offset),
	       max = rel + s->sh_size / s->sh_entsize;
	     rel < max;
	     rel = (const Elf_Rel *) ((grub_addr_t) rel + s->sh_entsize))
	  switch (ELF_R_TYPE (rel->r_info))
	    {
	    case R_AARCH64_CALL26:
	    case R_AARCH64_JUMP26:
	      {
		*tramp += sizeof (struct trampoline);
		break;
	      }
	    }
      }

  return GRUB_ERR_NONE;
}

/*
 * Unified function for both REL and RELA 
 */
grub_err_t
grub_arch_dl_relocate_symbols (grub_dl_t mod, void *ehdr,
			       Elf_Shdr *s, grub_dl_segment_t seg)
{
  Elf_Rel *rel, *max;

  for (rel = (Elf_Rel *) ((char *) ehdr + s->sh_offset),
	 max = (Elf_Rel *) ((char *) rel + s->sh_size);
       rel < max;
       rel = (Elf_Rel *) ((char *) rel + s->sh_entsize))
    {
      Elf_Sym *sym;
      void *place;
      grub_uint64_t sym_addr;

      if (rel->r_offset >= seg->size)
	return grub_error (GRUB_ERR_BAD_MODULE,
			   "reloc offset is out of the segment");

      sym = (Elf_Sym *) ((char *) mod->symtab
			 + mod->symsize * ELF_R_SYM (rel->r_info));

      sym_addr = sym->st_value;
      if (s->sh_type == SHT_RELA)
	sym_addr += ((Elf_Rela *) rel)->r_addend;

      place = (void *) ((grub_addr_t) seg->addr + rel->r_offset);

      switch (ELF_R_TYPE (rel->r_info))
	{
	case R_AARCH64_ABS64:
	  {
	    grub_uint64_t *abs_place = place;

	    grub_dprintf ("dl", "  reloc_abs64 %p => 0x%016llx\n",
			  place, (unsigned long long) sym_addr);

	    *abs_place = (grub_uint64_t) sym_addr;
	  }
	  break;
	case R_AARCH64_CALL26:
	case R_AARCH64_JUMP26:
	  {
	    grub_int64_t offset = sym_addr - (grub_uint64_t) place;

	    if (!grub_arm_64_check_xxxx26_offset (offset))
	      {
		struct trampoline *tp = mod->trampptr;
		mod->trampptr = tp + 1;
		tp->ldr = LDR;
		tp->br = BR;
		tp->addr = sym_addr;
		offset = (grub_uint8_t *) tp - (grub_uint8_t *) place;
	      }

	    if (!grub_arm_64_check_xxxx26_offset (offset))
		return grub_error (GRUB_ERR_BAD_MODULE,
				   "trampoline out of range");

	    grub_arm64_set_xxxx26_offset (place, offset);
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
