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
grub_arch_efiemu_check_header64 (void *ehdr)
{
  Elf64_Ehdr *e = ehdr;

  return (e->e_ident[EI_CLASS] == ELFCLASS64
	  && e->e_ident[EI_DATA] == ELFDATA2LSB
	  && e->e_machine == EM_X86_64);
}

/* Relocate symbols.  */
grub_err_t
grub_arch_efiemu_relocate_symbols64 (grub_efiemu_segment_t segs,
				     struct grub_efiemu_elf_sym *elfsyms,
				     void *ehdr)
{
  unsigned i;
  Elf64_Ehdr *e = ehdr;
  Elf64_Shdr *s;
  grub_err_t err;

  for (i = 0, s = (Elf64_Shdr *) ((char *) e + e->e_shoff);
       i < e->e_shnum;
       i++, s = (Elf64_Shdr *) ((char *) s + e->e_shentsize))
    if (s->sh_type == SHT_RELA)
      {
	grub_efiemu_segment_t seg;
	grub_dprintf ("efiemu", "shtrel\n");

	/* Find the target segment.  */
	for (seg = segs; seg; seg = seg->next)
	  if (seg->section == s->sh_info)
	    break;

	if (seg)
	  {
	    Elf64_Rela *rel, *max;

	    for (rel = (Elf64_Rela *) ((char *) e + s->sh_offset),
		   max = rel + (unsigned long) s->sh_size
		   / (unsigned long)s->sh_entsize;
		 rel < max;
		 rel++)
	      {
		void *addr;
		grub_uint32_t *addr32;
		grub_uint64_t *addr64;
		struct grub_efiemu_elf_sym sym;
		if (seg->size < rel->r_offset)
		  return grub_error (GRUB_ERR_BAD_MODULE,
				     "reloc offset is out of the segment");

		addr =
		  ((char *) grub_efiemu_mm_obtain_request (seg->handle)
		   + seg->off + rel->r_offset);
		addr32 = (grub_uint32_t *) addr;
		addr64 = (grub_uint64_t *) addr;
		sym = elfsyms[ELF64_R_SYM (rel->r_info)];

		switch (ELF64_R_TYPE (rel->r_info))
		  {
		  case R_X86_64_64:
		    err = grub_efiemu_write_value (addr,
						   *addr64 + rel->r_addend
						   + sym.off, sym.handle,
						   0, seg->ptv_rel_needed,
						   sizeof (grub_uint64_t));
		    if (err)
		      return err;
		    break;

		  case R_X86_64_PC32:
		    err = grub_efiemu_write_value (addr,
						   *addr32 + rel->r_addend
						   + sym.off
						   - rel->r_offset - seg->off,
						   sym.handle, seg->handle,
						   seg->ptv_rel_needed,
						   sizeof (grub_uint32_t));
		    if (err)
		      return err;
		    break;

                  case R_X86_64_32:
                  case R_X86_64_32S:
		    err = grub_efiemu_write_value (addr,
						   *addr32 + rel->r_addend
						   + sym.off, sym.handle,
						   0, seg->ptv_rel_needed,
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
