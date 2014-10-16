/* dl-386.c - arch-dependent part of loadable module support */
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
#include <grub/cpu/types.h>
#include <grub/mm.h>
#include <grub/i18n.h>

/* Dummy __gnu_local_gp. Resolved by linker.  */
static char __gnu_local_gp_dummy;
static char _gp_disp_dummy;

/* Check if EHDR is a valid ELF header.  */
grub_err_t
grub_arch_dl_check_header (void *ehdr)
{
  Elf_Ehdr *e = ehdr;

  /* Check the magic numbers.  */
#ifdef GRUB_CPU_WORDS_BIGENDIAN
  if (e->e_ident[EI_CLASS] != ELFCLASS32
      || e->e_ident[EI_DATA] != ELFDATA2MSB
      || e->e_machine != EM_MIPS)
#else
  if (e->e_ident[EI_CLASS] != ELFCLASS32
      || e->e_ident[EI_DATA] != ELFDATA2LSB
      || e->e_machine != EM_MIPS)
#endif
    return grub_error (GRUB_ERR_BAD_OS, N_("invalid arch-dependent ELF magic"));

  return GRUB_ERR_NONE;
}

#pragma GCC diagnostic ignored "-Wcast-align"

grub_err_t
grub_arch_dl_get_tramp_got_size (const void *ehdr, grub_size_t *tramp,
				 grub_size_t *got)
{
  const Elf_Ehdr *e = ehdr;
  const Elf_Shdr *s;
  /* FIXME: suboptimal.  */
  grub_size_t gp_size = 0;
  unsigned i;

  *tramp = 0;
  *got = 0;

  for (i = 0, s = (const Elf_Shdr *) ((const char *) e + e->e_shoff);
       i < e->e_shnum;
       i++, s = (const Elf_Shdr *) ((const char *) s + e->e_shentsize))
    if (s->sh_type == SHT_REL)
      {
	const Elf_Rel *rel, *max;

	for (rel = (const Elf_Rel *) ((const char *) e + s->sh_offset),
	       max = rel + s->sh_size / s->sh_entsize;
	     rel < max;
	     rel++)
	  switch (ELF_R_TYPE (rel->r_info))
	    {
	    case R_MIPS_GOT16:
	    case R_MIPS_CALL16:
	    case R_MIPS_GPREL32:
	      gp_size += 4;
	      break;
	    }
      }

  if (gp_size > 0x08000)
    return grub_error (GRUB_ERR_OUT_OF_RANGE, "__gnu_local_gp is too big\n");

  *got = gp_size;

  return GRUB_ERR_NONE;
}

/* Relocate symbols.  */
grub_err_t
grub_arch_dl_relocate_symbols (grub_dl_t mod, void *ehdr,
			       Elf_Shdr *s, grub_dl_segment_t seg)
{
  grub_uint32_t gp0;
  Elf_Ehdr *e = ehdr;

  if (!mod->reginfo)
    {
      unsigned i;
      Elf_Shdr *ri;

      /* Find reginfo. */
      for (i = 0, ri = (Elf_Shdr *) ((char *) ehdr + e->e_shoff);
	   i < e->e_shnum;
	   i++, ri = (Elf_Shdr *) ((char *) ri + e->e_shentsize))
	if (ri->sh_type == SHT_MIPS_REGINFO)
	  break;
      if (i == e->e_shnum)
	return grub_error (GRUB_ERR_BAD_MODULE, "no reginfo found");
      mod->reginfo = (grub_uint32_t *)((char *) ehdr + ri->sh_offset);
    }

  gp0 = mod->reginfo[5];
  Elf_Rel *rel, *max;

  for (rel = (Elf_Rel *) ((char *) e + s->sh_offset),
	 max = (Elf_Rel *) ((char *) rel + s->sh_size);
       rel < max;
       rel = (Elf_Rel *) ((char *) rel + s->sh_entsize))
    {
      grub_uint8_t *addr;
      Elf_Sym *sym;
      grub_uint32_t sym_value;

      if (seg->size < rel->r_offset)
	return grub_error (GRUB_ERR_BAD_MODULE,
			   "reloc offset is out of the segment");

      addr = (grub_uint8_t *) ((char *) seg->addr + rel->r_offset);
      sym = (Elf_Sym *) ((char *) mod->symtab
			 + mod->symsize * ELF_R_SYM (rel->r_info));
      sym_value = sym->st_value;
      if (sym_value == (grub_addr_t) &__gnu_local_gp_dummy)
	sym_value = (grub_addr_t) mod->got;
      else if (sym_value == (grub_addr_t) &_gp_disp_dummy)
	{
	  sym_value = (grub_addr_t) mod->got - (grub_addr_t) addr;
	  if (ELF_R_TYPE (rel->r_info) == R_MIPS_LO16)
	    /* ABI mandates +4 even if partner lui doesn't
	       immediately precede addiu.  */
	    sym_value += 4;
	}
      switch (ELF_R_TYPE (rel->r_info))
	{
	case R_MIPS_HI16:
	  {
	    grub_uint32_t value;
	    Elf_Rel *rel2;

#ifdef GRUB_CPU_WORDS_BIGENDIAN
	    addr += 2;
#endif

	    /* Handle partner lo16 relocation. Lower part is
	       treated as signed. Hence add 0x8000 to compensate. 
	    */
	    value = (*(grub_uint16_t *) addr << 16)
	      + sym_value + 0x8000;
	    for (rel2 = rel + 1; rel2 < max; rel2++)
	      if (ELF_R_SYM (rel2->r_info)
		  == ELF_R_SYM (rel->r_info)
		  && ELF_R_TYPE (rel2->r_info) == R_MIPS_LO16)
		{
		  value += *(grub_int16_t *)
		    ((char *) seg->addr + rel2->r_offset
#ifdef GRUB_CPU_WORDS_BIGENDIAN
		     + 2
#endif
		     );
		  break;
		}
	    *(grub_uint16_t *) addr = (value >> 16) & 0xffff;
	  }
	  break;
	case R_MIPS_LO16:
#ifdef GRUB_CPU_WORDS_BIGENDIAN
	  addr += 2;
#endif
	  *(grub_uint16_t *) addr += sym_value & 0xffff;
	  break;
	case R_MIPS_32:
	  *(grub_uint32_t *) addr += sym_value;
	  break;
	case R_MIPS_GPREL32:
	  *(grub_uint32_t *) addr = sym_value
	    + *(grub_uint32_t *) addr + gp0 - (grub_uint32_t)mod->got;
	  break;

	case R_MIPS_26:
	  {
	    grub_uint32_t value;
	    grub_uint32_t raw;
	    raw = (*(grub_uint32_t *) addr) & 0x3ffffff;
	    value = raw << 2;
	    value += sym_value;
	    raw = (value >> 2) & 0x3ffffff;
			
	    *(grub_uint32_t *) addr = 
	      raw | ((*(grub_uint32_t *) addr) & 0xfc000000);
	  }
	  break;
	case R_MIPS_GOT16:
	  if (ELF_ST_BIND (sym->st_info) == STB_LOCAL)
	    {
	      Elf_Rel *rel2;
	      /* Handle partner lo16 relocation. Lower part is
		 treated as signed. Hence add 0x8000 to compensate.
	      */
	      sym_value += (*(grub_uint16_t *) addr << 16)
		+ 0x8000;
	      for (rel2 = rel + 1; rel2 < max; rel2++)
		if (ELF_R_SYM (rel2->r_info)
		    == ELF_R_SYM (rel->r_info)
		    && ELF_R_TYPE (rel2->r_info) == R_MIPS_LO16)
		  {
		    sym_value += *(grub_int16_t *)
		      ((char *) seg->addr + rel2->r_offset
#ifdef GRUB_CPU_WORDS_BIGENDIAN
		       + 2
#endif
		       );
		    break;
		  }
	      sym_value &= 0xffff0000;
	      *(grub_uint16_t *) addr = 0;
	    }
	case R_MIPS_CALL16:
	  {
	    grub_uint32_t *gpptr = mod->gotptr;
	  /* FIXME: reuse*/
#ifdef GRUB_CPU_WORDS_BIGENDIAN
	    addr += 2;
#endif
	    *gpptr = sym_value + *(grub_uint16_t *) addr;
	    *(grub_uint16_t *) addr
	      = sizeof (grub_uint32_t) * (gpptr - (grub_uint32_t *) mod->got);
	    mod->gotptr = gpptr + 1;
	    break;
	  }
	case R_MIPS_JALR:
	  break;
	default:
	  {
	    return grub_error (GRUB_ERR_NOT_IMPLEMENTED_YET,
			       N_("relocation 0x%x is not implemented yet"),
			       ELF_R_TYPE (rel->r_info));
	  }
	  break;
	}
    }

  return GRUB_ERR_NONE;
}

void 
grub_arch_dl_init_linker (void)
{
  grub_dl_register_symbol ("__gnu_local_gp", &__gnu_local_gp_dummy, 0, 0);
  grub_dl_register_symbol ("_gp_disp", &_gp_disp_dummy, 0, 0);
}

