/* Load runtime image of EFIemu. Functions specific to 32/64-bit mode */
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

/* ELF symbols and their values */
static struct grub_efiemu_elf_sym *grub_efiemu_elfsyms = 0;
static int grub_efiemu_nelfsyms = 0;

/* Return the address of a section whose index is N.  */
static grub_err_t
grub_efiemu_get_section_addr (grub_efiemu_segment_t segs, unsigned n,
			      int *handle, grub_off_t *off)
{
  grub_efiemu_segment_t seg;

  for (seg = segs; seg; seg = seg->next)
    if (seg->section == n)
      {
	*handle = seg->handle;
	*off = seg->off;
	return GRUB_ERR_NONE;
      }

  return grub_error (GRUB_ERR_BAD_OS, "section %d not found", n);
}

grub_err_t
SUFFIX (grub_efiemu_loadcore_unload) (void)
{
  grub_free (grub_efiemu_elfsyms);
  grub_efiemu_elfsyms = 0;
  return GRUB_ERR_NONE;
}

/* Check if EHDR is a valid ELF header.  */
int
SUFFIX (grub_efiemu_check_header) (void *ehdr, grub_size_t size)
{
  Elf_Ehdr *e = ehdr;

  /* Check the header size.  */
  if (size < sizeof (Elf_Ehdr))
    return 0;

  /* Check the magic numbers.  */
  if (!SUFFIX (grub_arch_efiemu_check_header) (ehdr)
      || e->e_ident[EI_MAG0] != ELFMAG0
      || e->e_ident[EI_MAG1] != ELFMAG1
      || e->e_ident[EI_MAG2] != ELFMAG2
      || e->e_ident[EI_MAG3] != ELFMAG3
      || e->e_ident[EI_VERSION] != EV_CURRENT
      || e->e_version != EV_CURRENT)
    return 0;

  return 1;
}

/* Load all segments from memory specified by E.  */
static grub_err_t
grub_efiemu_load_segments (grub_efiemu_segment_t segs, const Elf_Ehdr *e)
{
  Elf_Shdr *s;
  grub_efiemu_segment_t cur;

  grub_dprintf ("efiemu", "loading segments\n");

  for (cur=segs; cur; cur = cur->next)
    {
      s = (Elf_Shdr *)cur->srcptr;

      if ((s->sh_flags & SHF_ALLOC) && s->sh_size)
	{
	  void *addr;

	  addr = (grub_uint8_t *) grub_efiemu_mm_obtain_request (cur->handle)
	    + cur->off;

	  switch (s->sh_type)
	    {
	    case SHT_PROGBITS:
	      grub_memcpy (addr, (char *) e + s->sh_offset, s->sh_size);
	      break;
	    case SHT_NOBITS:
	      grub_memset (addr, 0, s->sh_size);
	      break;
	    }
	}
    }

  return GRUB_ERR_NONE;
}

/* Get a string at offset OFFSET from strtab */
static char *
grub_efiemu_get_string (unsigned offset, const Elf_Ehdr *e)
{
  unsigned i;
  Elf_Shdr *s;

  for (i = 0, s = (Elf_Shdr *) ((char *) e + e->e_shoff);
       i < e->e_shnum;
       i++, s = (Elf_Shdr *) ((char *) s + e->e_shentsize))
    if (s->sh_type == SHT_STRTAB && offset < s->sh_size)
      return (char *) e + s->sh_offset + offset;
  return 0;
}

/* Request memory for segments and fill segments info */
static grub_err_t
grub_efiemu_init_segments (grub_efiemu_segment_t *segs, const Elf_Ehdr *e)
{
  unsigned i;
  Elf_Shdr *s;

  for (i = 0, s = (Elf_Shdr *) ((char *) e + e->e_shoff);
       i < e->e_shnum;
       i++, s = (Elf_Shdr *) ((char *) s + e->e_shentsize))
    {
      if (s->sh_flags & SHF_ALLOC)
	{
	  grub_efiemu_segment_t seg;
	  seg = (grub_efiemu_segment_t) grub_malloc (sizeof (*seg));
	  if (! seg)
	    return grub_errno;

	  if (s->sh_size)
	    {
	      seg->handle
		= grub_efiemu_request_memalign
		(s->sh_addralign, s->sh_size,
		 s->sh_flags & SHF_EXECINSTR ? GRUB_EFI_RUNTIME_SERVICES_CODE
		 : GRUB_EFI_RUNTIME_SERVICES_DATA);
	      if (seg->handle < 0)
		return grub_errno;
	      seg->off = 0;
	    }

	  /*
	     .text-physical doesn't need to be relocated when switching to
	     virtual mode
	   */
	  if (!grub_strcmp (grub_efiemu_get_string (s->sh_name, e),
			    ".text-physical"))
	    seg->ptv_rel_needed = 0;
	  else
	    seg->ptv_rel_needed = 1;
	  seg->size = s->sh_size;
	  seg->section = i;
	  seg->next = *segs;
	  seg->srcptr = s;
	  *segs = seg;
	}
    }

  return GRUB_ERR_NONE;
}

/* Count symbols and relocators and allocate/request memory for them */
static grub_err_t
grub_efiemu_count_symbols (const Elf_Ehdr *e)
{
  unsigned i;
  Elf_Shdr *s;
  int num = 0;

  /* Symbols */
  for (i = 0, s = (Elf_Shdr *) ((char *) e + e->e_shoff);
       i < e->e_shnum;
       i++, s = (Elf_Shdr *) ((char *) s + e->e_shentsize))
    if (s->sh_type == SHT_SYMTAB)
      break;

  if (i == e->e_shnum)
    return grub_error (GRUB_ERR_BAD_OS, N_("no symbol table"));

  grub_efiemu_nelfsyms = (unsigned) s->sh_size / (unsigned) s->sh_entsize;
  grub_efiemu_elfsyms = (struct grub_efiemu_elf_sym *)
    grub_malloc (sizeof (struct grub_efiemu_elf_sym) * grub_efiemu_nelfsyms);

  /* Relocators */
  for (i = 0, s = (Elf_Shdr *) ((char *) e + e->e_shoff);
       i < e->e_shnum;
       i++, s = (Elf_Shdr *) ((char *) s + e->e_shentsize))
    if (s->sh_type == SHT_REL || s->sh_type == SHT_RELA)
      num += ((unsigned) s->sh_size) / ((unsigned) s->sh_entsize);

  grub_efiemu_request_symbols (num);

  return GRUB_ERR_NONE;
}

/* Fill grub_efiemu_elfsyms with symbol values */
static grub_err_t
grub_efiemu_resolve_symbols (grub_efiemu_segment_t segs, Elf_Ehdr *e)
{
  unsigned i;
  Elf_Shdr *s;
  Elf_Sym *sym;
  const char *str;
  Elf_Word size, entsize;

  grub_dprintf ("efiemu", "resolving symbols\n");

  for (i = 0, s = (Elf_Shdr *) ((char *) e + e->e_shoff);
       i < e->e_shnum;
       i++, s = (Elf_Shdr *) ((char *) s + e->e_shentsize))
    if (s->sh_type == SHT_SYMTAB)
      break;

  if (i == e->e_shnum)
    return grub_error (GRUB_ERR_BAD_OS, N_("no symbol table"));

  sym = (Elf_Sym *) ((char *) e + s->sh_offset);
  size = s->sh_size;
  entsize = s->sh_entsize;

  s = (Elf_Shdr *) ((char *) e + e->e_shoff + e->e_shentsize * s->sh_link);
  str = (char *) e + s->sh_offset;

  for (i = 0;
       i < size / entsize;
       i++, sym = (Elf_Sym *) ((char *) sym + entsize))
    {
      unsigned char type = ELF_ST_TYPE (sym->st_info);
      unsigned char bind = ELF_ST_BIND (sym->st_info);
      int handle;
      grub_off_t off;
      grub_err_t err;
      const char *name = str + sym->st_name;
      grub_efiemu_elfsyms[i].section = sym->st_shndx;
      switch (type)
	{
	case STT_NOTYPE:
	  /* Resolve a global symbol.  */
	  if (sym->st_name != 0 && sym->st_shndx == 0)
	    {
	      err = grub_efiemu_resolve_symbol (name, &handle, &off);
	      if (err)
		return err;
	      grub_efiemu_elfsyms[i].handle = handle;
	      grub_efiemu_elfsyms[i].off = off;
	    }
	  else
	    sym->st_value = 0;
	  break;

	case STT_OBJECT:
	  err = grub_efiemu_get_section_addr (segs, sym->st_shndx,
					      &handle, &off);
	  if (err)
	    return err;

	  off += sym->st_value;
	  if (bind != STB_LOCAL)
	    {
	      err = grub_efiemu_register_symbol (name, handle, off);
	      if (err)
		return err;
	    }
	  grub_efiemu_elfsyms[i].handle = handle;
	  grub_efiemu_elfsyms[i].off = off;
	  break;

	case STT_FUNC:
	  err = grub_efiemu_get_section_addr (segs, sym->st_shndx,
					      &handle, &off);
	  if (err)
	    return err;

	  off += sym->st_value;
	  if (bind != STB_LOCAL)
	    {
	      err = grub_efiemu_register_symbol (name, handle, off);
	      if (err)
		return err;
	    }
	  grub_efiemu_elfsyms[i].handle = handle;
	  grub_efiemu_elfsyms[i].off = off;
	  break;

	case STT_SECTION:
	  err = grub_efiemu_get_section_addr (segs, sym->st_shndx,
					      &handle, &off);
	  if (err)
	    {
	      grub_efiemu_elfsyms[i].handle = 0;
	      grub_efiemu_elfsyms[i].off = 0;
	      grub_errno = GRUB_ERR_NONE;
	      break;
	    }

	  grub_efiemu_elfsyms[i].handle = handle;
	  grub_efiemu_elfsyms[i].off = off;
	  break;

	case STT_FILE:
	  grub_efiemu_elfsyms[i].handle = 0;
	  grub_efiemu_elfsyms[i].off = 0;
	  break;

	default:
	  return grub_error (GRUB_ERR_BAD_MODULE,
			     "unknown symbol type `%d'", (int) type);
	}
    }

  return GRUB_ERR_NONE;
}

/* Load runtime to the memory and request memory for definitive location*/
grub_err_t
SUFFIX (grub_efiemu_loadcore_init) (void *core, const char *filename,
				    grub_size_t core_size,
				    grub_efiemu_segment_t *segments)
{
  Elf_Ehdr *e = (Elf_Ehdr *) core;
  grub_err_t err;

  if (e->e_type != ET_REL)
    return grub_error (GRUB_ERR_BAD_MODULE, N_("this ELF file is not of the right type"));

  /* Make sure that every section is within the core.  */
  if ((grub_size_t) core_size < e->e_shoff + e->e_shentsize * e->e_shnum)
    return grub_error (GRUB_ERR_BAD_OS, N_("premature end of file %s"),
		       filename);

  err = grub_efiemu_init_segments (segments, core);
  if (err)
    return err;
  err = grub_efiemu_count_symbols (core);
  if (err)
    return err;

  grub_efiemu_request_symbols (1);
  return GRUB_ERR_NONE;
}

/* Load runtime definitively */
grub_err_t
SUFFIX (grub_efiemu_loadcore_load) (void *core,
				    grub_size_t core_size
				    __attribute__ ((unused)),
				    grub_efiemu_segment_t segments)
{
  grub_err_t err;
  err = grub_efiemu_load_segments (segments, core);
  if (err)
    return err;

  err = grub_efiemu_resolve_symbols (segments, core);
  if (err)
    return err;

  err = SUFFIX (grub_arch_efiemu_relocate_symbols) (segments,
						    grub_efiemu_elfsyms,
						    core);
  if (err)
    return err;

  return GRUB_ERR_NONE;
}
