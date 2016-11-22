/* i386 CPU-specific part of loadcore.c for 32-bit mode */
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

#include <grub/err.h>
#include <grub/mm.h>
#include <grub/misc.h>
#include <grub/efiemu/efiemu.h>
#include <grub/cpu/efiemu.h>
#include <grub/elf.h>
#include <grub/i18n.h>

/* Check if EHDR is a valid ELF header.  */
int
grub_arch_efiemu_check_header32 (void *ehdr)
{
  Elf32_Ehdr *e = ehdr;

  /* Check the magic numbers.  */
  return (e->e_ident[EI_CLASS] == ELFCLASS32
	  && e->e_ident[EI_DATA] == ELFDATA2LSB
	  && e->e_machine == EM_386);
}

/* Relocate symbols.  */
grub_err_t
grub_arch_efiemu_relocate_symbols32 (grub_efiemu_segment_t segs,
				     struct grub_efiemu_elf_sym *elfsyms,
				     void *ehdr)
{
  unsigned i;
  Elf32_Ehdr *e = ehdr;
  Elf32_Shdr *s;
  grub_err_t err;

  grub_dprintf ("efiemu", "relocating symbols %d %d\n",
		e->e_shoff, e->e_shnum);

  for (i = 0, s = (Elf32_Shdr *) ((char *) e + e->e_shoff);
       i < e->e_shnum;
       i++, s = (Elf32_Shdr *) ((char *) s + e->e_shentsize))
    if (s->sh_type == SHT_REL)
      {
	grub_efiemu_segment_t seg;
	grub_dprintf ("efiemu", "shtrel\n");

	/* Find the target segment.  */
	for (seg = segs; seg; seg = seg->next)
	  if (seg->section == s->sh_info)
	    break;

	if (seg)
	  {
	    Elf32_Rel *rel, *max;

	    for (rel = (Elf32_Rel *) ((char *) e + s->sh_offset),
		   max = rel + s->sh_size / s->sh_entsize;
		 rel < max;
		 rel++)
	      {
		Elf32_Word *addr;
		struct grub_efiemu_elf_sym sym;
		if (seg->size < rel->r_offset)
		  return grub_error (GRUB_ERR_BAD_MODULE,
				     "reloc offset is out of the segment");

		addr = (Elf32_Word *)
		  ((char *) grub_efiemu_mm_obtain_request (seg->handle)
		   + seg->off + rel->r_offset);
		sym = elfsyms[ELF32_R_SYM (rel->r_info)];

		switch (ELF32_R_TYPE (rel->r_info))
		  {
		  case R_386_32:
		    err = grub_efiemu_write_value (addr, sym.off + *addr,
						   sym.handle, 0,
						   seg->ptv_rel_needed,
						   sizeof (grub_uint32_t));
		    if (err)
		      return err;

		    break;

		  case R_386_PC32:
		    err = grub_efiemu_write_value (addr, sym.off + *addr
						   - rel->r_offset
						   - seg->off, sym.handle,
						   seg->handle,
						   seg->ptv_rel_needed,
						   sizeof (grub_uint32_t));
		    if (err)
		      return err;
		    break;
		  default:
		    return grub_error (GRUB_ERR_NOT_IMPLEMENTED_YET,
				       N_("relocation 0x%x is not implemented yet"),
				       ELF_R_TYPE (rel->r_info));
		  }
	      }
	  }
      }

  return GRUB_ERR_NONE;
}


