/* readelf.c -- display contents of an ELF format file
   Copyright 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007,
   2008, 2009, 2010, 2011, 2012
   Free Software Foundation, Inc.

   Originally developed by Eric Youngdale <eric@andante.jic.com>
   Modifications by Nick Clifton <nickc@redhat.com>

   This file is part of GNU Binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

/* The difference between readelf and objdump:

  Both programs are capable of displaying the contents of ELF format files,
  so why does the binutils project have two file dumpers ?

  The reason is that objdump sees an ELF file through a BFD filter of the
  world; if BFD has a bug where, say, it disagrees about a machine constant
  in e_flags, then the odds are good that it will remain internally
  consistent.  The linker sees it the BFD way, objdump sees it the BFD way,
  GAS sees it the BFD way.  There was need for a tool to go find out what
  the file actually says.

  This is why the readelf program does not link against the BFD library - it
  exists as an independent program to help verify the correct working of BFD.

  There is also the case that readelf can provide more information about an
  ELF file than is provided by objdump.  In particular it can display DWARF
  debugging information which (at the moment) objdump cannot.  */

#include "sysdep.h"
#include <assert.h>
#include <time.h>
#ifdef HAVE_ZLIB_H
#include <zlib.h>
#endif
#ifdef HAVE_WCHAR_H
#include <wchar.h>
#endif

#if __GNUC__ >= 2
/* Define BFD64 here, even if our default architecture is 32 bit ELF
   as this will allow us to read in and parse 64bit and 32bit ELF files.
   Only do this if we believe that the compiler can support a 64 bit
   data type.  For now we only rely on GCC being able to do this.  */
#define BFD64
#endif

#include "bfd.h"
#include "bucomm.h"
#include "elfcomm.h"
#include "dwarf.h"

#include "elf/common.h"
#include "elf/external.h"
#include "elf/internal.h"


/* Included here, before RELOC_MACROS_GEN_FUNC is defined, so that
   we can obtain the H8 reloc numbers.  We need these for the
   get_reloc_size() function.  We include h8.h again after defining
   RELOC_MACROS_GEN_FUNC so that we get the naming function as well.  */

#include "elf/h8.h"
#undef _ELF_H8_H

/* Undo the effects of #including reloc-macros.h.  */

#undef START_RELOC_NUMBERS
#undef RELOC_NUMBER
#undef FAKE_RELOC
#undef EMPTY_RELOC
#undef END_RELOC_NUMBERS
#undef _RELOC_MACROS_H

/* The following headers use the elf/reloc-macros.h file to
   automatically generate relocation recognition functions
   such as elf_mips_reloc_type()  */

#define RELOC_MACROS_GEN_FUNC

#include "elf/aarch64.h"
#include "elf/alpha.h"
#include "elf/arc.h"
#include "elf/arm.h"
#include "elf/avr.h"
#include "elf/bfin.h"
#include "elf/cr16.h"
#include "elf/cris.h"
#include "elf/crx.h"
#include "elf/d10v.h"
#include "elf/d30v.h"
#include "elf/dlx.h"
#include "elf/epiphany.h"
#include "elf/fr30.h"
#include "elf/frv.h"
#include "elf/h8.h"
#include "elf/hppa.h"
#include "elf/i386.h"
#include "elf/i370.h"
#include "elf/i860.h"
#include "elf/i960.h"
#include "elf/ia64.h"
#include "elf/ip2k.h"
#include "elf/lm32.h"
#include "elf/iq2000.h"
#include "elf/m32c.h"
#include "elf/m32r.h"
#include "elf/m68k.h"
#include "elf/m68hc11.h"
#include "elf/mcore.h"
#include "elf/mep.h"
#include "elf/microblaze.h"
#include "elf/mips.h"
#include "elf/mmix.h"
#include "elf/mn10200.h"
#include "elf/mn10300.h"
#include "elf/moxie.h"
#include "elf/mt.h"
#include "elf/msp430.h"
#include "elf/or32.h"
#include "elf/pj.h"
#include "elf/ppc.h"
#include "elf/ppc64.h"
#include "elf/rl78.h"
#include "elf/rx.h"
#include "elf/s390.h"
#include "elf/score.h"
#include "elf/sh.h"
#include "elf/sparc.h"
#include "elf/spu.h"
#include "elf/tic6x.h"
#include "elf/tilegx.h"
#include "elf/tilepro.h"
#include "elf/v850.h"
#include "elf/vax.h"
#include "elf/x86-64.h"
#include "elf/xc16x.h"
#include "elf/xgate.h"
#include "elf/xstormy16.h"
#include "elf/xtensa.h"

#include "getopt.h"
#include "libiberty.h"
#include "safe-ctype.h"
#include "filenames.h"

char * program_name = "readelf";
static long archive_file_offset;
static unsigned long archive_file_size;
static unsigned long dynamic_addr;
static bfd_size_type dynamic_size;
static unsigned int dynamic_nent;
static char * dynamic_strings;
static unsigned long dynamic_strings_length;
static char * string_table;
static unsigned long string_table_length;
static unsigned long num_dynamic_syms;
static Elf_Internal_Sym * dynamic_symbols;
static Elf_Internal_Syminfo * dynamic_syminfo;
static unsigned long dynamic_syminfo_offset;
static unsigned int dynamic_syminfo_nent;
static char program_interpreter[PATH_MAX];
static bfd_vma dynamic_info[DT_ENCODING];
static bfd_vma dynamic_info_DT_GNU_HASH;
static bfd_vma version_info[16];
static Elf_Internal_Ehdr elf_header;
static Elf_Internal_Shdr * section_headers;
static Elf_Internal_Phdr * program_headers;
static Elf_Internal_Dyn *  dynamic_section;
static Elf_Internal_Shdr * symtab_shndx_hdr;
static int show_name;
static int do_dynamic;
static int do_syms;
static int do_dyn_syms;
static int do_reloc;
static int do_sections;
static int do_section_groups;
static int do_section_details;
static int do_segments;
static int do_unwind;
static int do_using_dynamic;
static int do_header;
static int do_dump;
static int do_version;
static int do_histogram;
static int do_debugging;
static int do_arch;
static int do_notes;
static int do_archive_index;
static int is_32bit_elf;

struct group_list
{
  struct group_list * next;
  unsigned int section_index;
};

struct group
{
  struct group_list * root;
  unsigned int group_index;
};

static size_t group_count;
static struct group * section_groups;
static struct group ** section_headers_groups;


/* Flag bits indicating particular types of dump.  */
#define HEX_DUMP	(1 << 0)	/* The -x command line switch.  */
#define DISASS_DUMP	(1 << 1)	/* The -i command line switch.  */
#define DEBUG_DUMP	(1 << 2)	/* The -w command line switch.  */
#define STRING_DUMP     (1 << 3)	/* The -p command line switch.  */
#define RELOC_DUMP      (1 << 4)	/* The -R command line switch.  */

typedef unsigned char dump_type;

/* A linked list of the section names for which dumps were requested.  */
struct dump_list_entry
{
  char * name;
  dump_type type;
  struct dump_list_entry * next;
};
static struct dump_list_entry * dump_sects_byname;

/* A dynamic array of flags indicating for which sections a dump
   has been requested via command line switches.  */
static dump_type *   cmdline_dump_sects = NULL;
static unsigned int  num_cmdline_dump_sects = 0;

/* A dynamic array of flags indicating for which sections a dump of
   some kind has been requested.  It is reset on a per-object file
   basis and then initialised from the cmdline_dump_sects array,
   the results of interpreting the -w switch, and the
   dump_sects_byname list.  */
static dump_type *   dump_sects = NULL;
static unsigned int  num_dump_sects = 0;


/* How to print a vma value.  */
typedef enum print_mode
{
  HEX,
  DEC,
  DEC_5,
  UNSIGNED,
  PREFIX_HEX,
  FULL_HEX,
  LONG_HEX
}
print_mode;

#define UNKNOWN -1

#define SECTION_NAME(X)						\
  ((X) == NULL ? _("<none>")					\
   : string_table == NULL ? _("<no-name>")			\
   : ((X)->sh_name >= string_table_length ? _("<corrupt>")	\
  : string_table + (X)->sh_name))

#define DT_VERSIONTAGIDX(tag)	(DT_VERNEEDNUM - (tag))	/* Reverse order!  */

#define GET_ELF_SYMBOLS(file, section, sym_count)			\
  (is_32bit_elf ? get_32bit_elf_symbols (file, section, sym_count)	\
   : get_64bit_elf_symbols (file, section, sym_count))

#define VALID_DYNAMIC_NAME(offset)	((dynamic_strings != NULL) && (offset < dynamic_strings_length))
/* GET_DYNAMIC_NAME asssumes that VALID_DYNAMIC_NAME has
   already been called and verified that the string exists.  */
#define GET_DYNAMIC_NAME(offset)	(dynamic_strings + offset)

#define REMOVE_ARCH_BITS(ADDR)			\
  do						\
    {						\
      if (elf_header.e_machine == EM_ARM)	\
	(ADDR) &= ~1;				\
    }						\
  while (0)

/* Retrieve NMEMB structures, each SIZE bytes long from FILE starting at OFFSET.
   Put the retrieved data into VAR, if it is not NULL.  Otherwise allocate a buffer
   using malloc and fill that.  In either case return the pointer to the start of
   the retrieved data or NULL if something went wrong.  If something does go wrong
   emit an error message using REASON as part of the context.  */

static void *
get_data (void * var, FILE * file, long offset, size_t size, size_t nmemb,
	  const char * reason)
{
  void * mvar;

  if (size == 0 || nmemb == 0)
    return NULL;

  if (fseek (file, archive_file_offset + offset, SEEK_SET))
    {
      error (_("Unable to seek to 0x%lx for %s\n"),
	     (unsigned long) archive_file_offset + offset, reason);
      return NULL;
    }

  mvar = var;
  if (mvar == NULL)
    {
      /* Check for overflow.  */
      if (nmemb < (~(size_t) 0 - 1) / size)
	/* + 1 so that we can '\0' terminate invalid string table sections.  */
	mvar = malloc (size * nmemb + 1);

      if (mvar == NULL)
	{
	  error (_("Out of memory allocating 0x%lx bytes for %s\n"),
		 (unsigned long)(size * nmemb), reason);
	  return NULL;
	}

      ((char *) mvar)[size * nmemb] = '\0';
    }

  if (fread (mvar, size, nmemb, file) != nmemb)
    {
      error (_("Unable to read in 0x%lx bytes of %s\n"),
	     (unsigned long)(size * nmemb), reason);
      if (mvar != var)
	free (mvar);
      return NULL;
    }

  return mvar;
}

/* Print a VMA value.  */

static int
print_vma (bfd_vma vma, print_mode mode)
{
  int nc = 0;

  switch (mode)
    {
    case FULL_HEX:
      nc = printf ("0x");
      /* Drop through.  */

    case LONG_HEX:
#ifdef BFD64
      if (is_32bit_elf)
	return nc + printf ("%8.8" BFD_VMA_FMT "x", vma);
#endif
      printf_vma (vma);
      return nc + 16;

    case DEC_5:
      if (vma <= 99999)
	return printf ("%5" BFD_VMA_FMT "d", vma);
      /* Drop through.  */

    case PREFIX_HEX:
      nc = printf ("0x");
      /* Drop through.  */

    case HEX:
      return nc + printf ("%" BFD_VMA_FMT "x", vma);

    case DEC:
      return printf ("%" BFD_VMA_FMT "d", vma);

    case UNSIGNED:
      return printf ("%" BFD_VMA_FMT "u", vma);
    }
  return 0;
}

/* Display a symbol on stdout.  Handles the display of control characters and
   multibye characters (assuming the host environment supports them).

   Display at most abs(WIDTH) characters, truncating as necessary, unless do_wide is true.

   If WIDTH is negative then ensure that the output is at least (- WIDTH) characters,
   padding as necessary.

   Returns the number of emitted characters.  */

static unsigned int
print_symbol (int width, const char *symbol)
{
  bfd_boolean extra_padding = FALSE;
  int num_printed = 0;
#ifdef HAVE_MBSTATE_T
  mbstate_t state;
#endif
  int width_remaining;

  if (width < 0)
    {
      /* Keep the width positive.  This also helps.  */
      width = - width;
      extra_padding = TRUE;
    }  

  if (do_wide)
    /* Set the remaining width to a very large value.
       This simplifies the code below.  */
    width_remaining = INT_MAX;
  else
    width_remaining = width;

#ifdef HAVE_MBSTATE_T
  /* Initialise the multibyte conversion state.  */
  memset (& state, 0, sizeof (state));
#endif

  while (width_remaining)
    {
      size_t  n;
      const char c = *symbol++;

      if (c == 0)
	break;

      /* Do not print control characters directly as they can affect terminal
	 settings.  Such characters usually appear in the names generated
	 by the assembler for local labels.  */
      if (ISCNTRL (c))
	{
	  if (width_remaining < 2)
	    break;

	  printf ("^%c", c + 0x40);
	  width_remaining -= 2;
	  num_printed += 2;
	}
      else if (ISPRINT (c))
	{
	  putchar (c);
	  width_remaining --;
	  num_printed ++;
	}
      else
	{
#ifdef HAVE_MBSTATE_T
	  wchar_t w;
#endif
	  /* Let printf do the hard work of displaying multibyte characters.  */
	  printf ("%.1s", symbol - 1);
	  width_remaining --;
	  num_printed ++;

#ifdef HAVE_MBSTATE_T
	  /* Try to find out how many bytes made up the character that was
	     just printed.  Advance the symbol pointer past the bytes that
	     were displayed.  */
	  n = mbrtowc (& w, symbol - 1, MB_CUR_MAX, & state);
#else
	  n = 1;
#endif
	  if (n != (size_t) -1 && n != (size_t) -2 && n > 0)
	    symbol += (n - 1);
	}
    }

  if (extra_padding && num_printed < width)
    {
      /* Fill in the remaining spaces.  */
      printf ("%-*s", width - num_printed, " ");
      num_printed = width;
    }

  return num_printed;
}

/* Return a pointer to section NAME, or NULL if no such section exists.  */

static Elf_Internal_Shdr *
find_section (const char * name)
{
  unsigned int i;

  for (i = 0; i < elf_header.e_shnum; i++)
    if (streq (SECTION_NAME (section_headers + i), name))
      return section_headers + i;

  return NULL;
}

/* Return a pointer to a section containing ADDR, or NULL if no such
   section exists.  */

static Elf_Internal_Shdr *
find_section_by_address (bfd_vma addr)
{
  unsigned int i;

  for (i = 0; i < elf_header.e_shnum; i++)
    {
      Elf_Internal_Shdr *sec = section_headers + i;
      if (addr >= sec->sh_addr && addr < sec->sh_addr + sec->sh_size)
	return sec;
    }

  return NULL;
}

/* Read an unsigned LEB128 encoded value from p.  Set *PLEN to the number of
   bytes read.  */

static unsigned long
read_uleb128 (unsigned char *data, unsigned int *length_return)
{
  return read_leb128 (data, length_return, 0);
}

/* Return true if the current file is for IA-64 machine and OpenVMS ABI.
   This OS has so many departures from the ELF standard that we test it at
   many places.  */

static inline int
is_ia64_vms (void)
{
  return elf_header.e_machine == EM_IA_64
    && elf_header.e_ident[EI_OSABI] == ELFOSABI_OPENVMS;
}

/* Guess the relocation size commonly used by the specific machines.  */

static int
guess_is_rela (unsigned int e_machine)
{
  switch (e_machine)
    {
      /* Targets that use REL relocations.  */
    case EM_386:
    case EM_486:
    case EM_960:
    case EM_ARM:
    case EM_D10V:
    case EM_CYGNUS_D10V:
    case EM_DLX:
    case EM_MIPS:
    case EM_MIPS_RS3_LE:
    case EM_CYGNUS_M32R:
    case EM_OPENRISC:
    case EM_OR32:
    case EM_SCORE:
    case EM_XGATE:
      return FALSE;

      /* Targets that use RELA relocations.  */
    case EM_68K:
    case EM_860:
    case EM_AARCH64:
    case EM_ADAPTEVA_EPIPHANY:
    case EM_ALPHA:
    case EM_ALTERA_NIOS2:
    case EM_AVR:
    case EM_AVR_OLD:
    case EM_BLACKFIN:
    case EM_CR16:
    case EM_CRIS:
    case EM_CRX:
    case EM_D30V:
    case EM_CYGNUS_D30V:
    case EM_FR30:
    case EM_CYGNUS_FR30:
    case EM_CYGNUS_FRV:
    case EM_H8S:
    case EM_H8_300:
    case EM_H8_300H:
    case EM_IA_64:
    case EM_IP2K:
    case EM_IP2K_OLD:
    case EM_IQ2000:
    case EM_LATTICEMICO32:
    case EM_M32C_OLD:
    case EM_M32C:
    case EM_M32R:
    case EM_MCORE:
    case EM_CYGNUS_MEP:
    case EM_MMIX:
    case EM_MN10200:
    case EM_CYGNUS_MN10200:
    case EM_MN10300:
    case EM_CYGNUS_MN10300:
    case EM_MOXIE:
    case EM_MSP430:
    case EM_MSP430_OLD:
    case EM_MT:
    case EM_NIOS32:
    case EM_PPC64:
    case EM_PPC:
    case EM_RL78:
    case EM_RX:
    case EM_S390:
    case EM_S390_OLD:
    case EM_SH:
    case EM_SPARC:
    case EM_SPARC32PLUS:
    case EM_SPARCV9:
    case EM_SPU:
    case EM_TI_C6000:
    case EM_TILEGX:
    case EM_TILEPRO:
    case EM_V850:
    case EM_CYGNUS_V850:
    case EM_VAX:
    case EM_X86_64:
    case EM_L1OM:
    case EM_K1OM:
    case EM_XSTORMY16:
    case EM_XTENSA:
    case EM_XTENSA_OLD:
    case EM_MICROBLAZE:
    case EM_MICROBLAZE_OLD:
      return TRUE;

    case EM_68HC05:
    case EM_68HC08:
    case EM_68HC11:
    case EM_68HC16:
    case EM_FX66:
    case EM_ME16:
    case EM_MMA:
    case EM_NCPU:
    case EM_NDR1:
    case EM_PCP:
    case EM_ST100:
    case EM_ST19:
    case EM_ST7:
    case EM_ST9PLUS:
    case EM_STARCORE:
    case EM_SVX:
    case EM_TINYJ:
    default:
      warn (_("Don't know about relocations on this machine architecture\n"));
      return FALSE;
    }
}

static int
slurp_rela_relocs (FILE * file,
		   unsigned long rel_offset,
		   unsigned long rel_size,
		   Elf_Internal_Rela ** relasp,
		   unsigned long * nrelasp)
{
  Elf_Internal_Rela * relas;
  unsigned long nrelas;
  unsigned int i;

  if (is_32bit_elf)
    {
      Elf32_External_Rela * erelas;

      erelas = (Elf32_External_Rela *) get_data (NULL, file, rel_offset, 1,
                                                 rel_size, _("32-bit relocation data"));
      if (!erelas)
	return 0;

      nrelas = rel_size / sizeof (Elf32_External_Rela);

      relas = (Elf_Internal_Rela *) cmalloc (nrelas,
                                             sizeof (Elf_Internal_Rela));

      if (relas == NULL)
	{
	  free (erelas);
	  error (_("out of memory parsing relocs\n"));
	  return 0;
	}

      for (i = 0; i < nrelas; i++)
	{
	  relas[i].r_offset = BYTE_GET (erelas[i].r_offset);
	  relas[i].r_info   = BYTE_GET (erelas[i].r_info);
	  relas[i].r_addend = BYTE_GET_SIGNED (erelas[i].r_addend);
	}

      free (erelas);
    }
  else
    {
      Elf64_External_Rela * erelas;

      erelas = (Elf64_External_Rela *) get_data (NULL, file, rel_offset, 1,
                                                 rel_size, _("64-bit relocation data"));
      if (!erelas)
	return 0;

      nrelas = rel_size / sizeof (Elf64_External_Rela);

      relas = (Elf_Internal_Rela *) cmalloc (nrelas,
                                             sizeof (Elf_Internal_Rela));

      if (relas == NULL)
	{
	  free (erelas);
	  error (_("out of memory parsing relocs\n"));
	  return 0;
	}

      for (i = 0; i < nrelas; i++)
	{
	  relas[i].r_offset = BYTE_GET (erelas[i].r_offset);
	  relas[i].r_info   = BYTE_GET (erelas[i].r_info);
	  relas[i].r_addend = BYTE_GET_SIGNED (erelas[i].r_addend);

	  /* The #ifdef BFD64 below is to prevent a compile time
	     warning.  We know that if we do not have a 64 bit data
	     type that we will never execute this code anyway.  */
#ifdef BFD64
	  if (elf_header.e_machine == EM_MIPS
	      && elf_header.e_ident[EI_DATA] != ELFDATA2MSB)
	    {
	      /* In little-endian objects, r_info isn't really a
		 64-bit little-endian value: it has a 32-bit
		 little-endian symbol index followed by four
		 individual byte fields.  Reorder INFO
		 accordingly.  */
	      bfd_vma inf = relas[i].r_info;
	      inf = (((inf & 0xffffffff) << 32)
		      | ((inf >> 56) & 0xff)
		      | ((inf >> 40) & 0xff00)
		      | ((inf >> 24) & 0xff0000)
		      | ((inf >> 8) & 0xff000000));
	      relas[i].r_info = inf;
	    }
#endif /* BFD64 */
	}

      free (erelas);
    }
  *relasp = relas;
  *nrelasp = nrelas;
  return 1;
}

static int
slurp_rel_relocs (FILE * file,
		  unsigned long rel_offset,
		  unsigned long rel_size,
		  Elf_Internal_Rela ** relsp,
		  unsigned long * nrelsp)
{
  Elf_Internal_Rela * rels;
  unsigned long nrels;
  unsigned int i;

  if (is_32bit_elf)
    {
      Elf32_External_Rel * erels;

      erels = (Elf32_External_Rel *) get_data (NULL, file, rel_offset, 1,
                                               rel_size, _("32-bit relocation data"));
      if (!erels)
	return 0;

      nrels = rel_size / sizeof (Elf32_External_Rel);

      rels = (Elf_Internal_Rela *) cmalloc (nrels, sizeof (Elf_Internal_Rela));

      if (rels == NULL)
	{
	  free (erels);
	  error (_("out of memory parsing relocs\n"));
	  return 0;
	}

      for (i = 0; i < nrels; i++)
	{
	  rels[i].r_offset = BYTE_GET (erels[i].r_offset);
	  rels[i].r_info   = BYTE_GET (erels[i].r_info);
	  rels[i].r_addend = 0;
	}

      free (erels);
    }
  else
    {
      Elf64_External_Rel * erels;

      erels = (Elf64_External_Rel *) get_data (NULL, file, rel_offset, 1,
                                               rel_size, _("64-bit relocation data"));
      if (!erels)
	return 0;

      nrels = rel_size / sizeof (Elf64_External_Rel);

      rels = (Elf_Internal_Rela *) cmalloc (nrels, sizeof (Elf_Internal_Rela));

      if (rels == NULL)
	{
	  free (erels);
	  error (_("out of memory parsing relocs\n"));
	  return 0;
	}

      for (i = 0; i < nrels; i++)
	{
	  rels[i].r_offset = BYTE_GET (erels[i].r_offset);
	  rels[i].r_info   = BYTE_GET (erels[i].r_info);
	  rels[i].r_addend = 0;

	  /* The #ifdef BFD64 below is to prevent a compile time
	     warning.  We know that if we do not have a 64 bit data
	     type that we will never execute this code anyway.  */
#ifdef BFD64
	  if (elf_header.e_machine == EM_MIPS
	      && elf_header.e_ident[EI_DATA] != ELFDATA2MSB)
	    {
	      /* In little-endian objects, r_info isn't really a
		 64-bit little-endian value: it has a 32-bit
		 little-endian symbol index followed by four
		 individual byte fields.  Reorder INFO
		 accordingly.  */
	      bfd_vma inf = rels[i].r_info;
	      inf = (((inf & 0xffffffff) << 32)
		     | ((inf >> 56) & 0xff)
		     | ((inf >> 40) & 0xff00)
		     | ((inf >> 24) & 0xff0000)
		     | ((inf >> 8) & 0xff000000));
	      rels[i].r_info = inf;
	    }
#endif /* BFD64 */
	}

      free (erels);
    }
  *relsp = rels;
  *nrelsp = nrels;
  return 1;
}

/* Returns the reloc type extracted from the reloc info field.  */

static unsigned int
get_reloc_type (bfd_vma reloc_info)
{
  if (is_32bit_elf)
    return ELF32_R_TYPE (reloc_info);

  switch (elf_header.e_machine)
    {
    case EM_MIPS:
      /* Note: We assume that reloc_info has already been adjusted for us.  */
      return ELF64_MIPS_R_TYPE (reloc_info);

    case EM_SPARCV9:
      return ELF64_R_TYPE_ID (reloc_info);

    default:
      return ELF64_R_TYPE (reloc_info);
    }
}

/* Return the symbol index extracted from the reloc info field.  */

static bfd_vma
get_reloc_symindex (bfd_vma reloc_info)
{
  return is_32bit_elf ? ELF32_R_SYM (reloc_info) : ELF64_R_SYM (reloc_info);
}

/* Display the contents of the relocation data found at the specified
   offset.  */

static void
dump_relocations (FILE * file,
		  unsigned long rel_offset,
		  unsigned long rel_size,
		  Elf_Internal_Sym * symtab,
		  unsigned long nsyms,
		  char * strtab,
		  unsigned long strtablen,
		  int is_rela)
{
  unsigned int i;
  Elf_Internal_Rela * rels;

  if (is_rela == UNKNOWN)
    is_rela = guess_is_rela (elf_header.e_machine);

  if (is_rela)
    {
      if (!slurp_rela_relocs (file, rel_offset, rel_size, &rels, &rel_size))
	return;
    }
  else
    {
      if (!slurp_rel_relocs (file, rel_offset, rel_size, &rels, &rel_size))
	return;
    }

  if (is_32bit_elf)
    {
      if (is_rela)
	{
	  if (do_wide)
	    printf (_(" Offset     Info    Type                Sym. Value  Symbol's Name + Addend\n"));
	  else
	    printf (_(" Offset     Info    Type            Sym.Value  Sym. Name + Addend\n"));
	}
      else
	{
	  if (do_wide)
	    printf (_(" Offset     Info    Type                Sym. Value  Symbol's Name\n"));
	  else
	    printf (_(" Offset     Info    Type            Sym.Value  Sym. Name\n"));
	}
    }
  else
    {
      if (is_rela)
	{
	  if (do_wide)
	    printf (_("    Offset             Info             Type               Symbol's Value  Symbol's Name + Addend\n"));
	  else
	    printf (_("  Offset          Info           Type           Sym. Value    Sym. Name + Addend\n"));
	}
      else
	{
	  if (do_wide)
	    printf (_("    Offset             Info             Type               Symbol's Value  Symbol's Name\n"));
	  else
	    printf (_("  Offset          Info           Type           Sym. Value    Sym. Name\n"));
	}
    }

  for (i = 0; i < rel_size; i++)
    {
      const char * rtype;
      bfd_vma offset;
      bfd_vma inf;
      bfd_vma symtab_index;
      bfd_vma type;

      offset = rels[i].r_offset;
      inf    = rels[i].r_info;

      type = get_reloc_type (inf);
      symtab_index = get_reloc_symindex  (inf);

      if (is_32bit_elf)
	{
	  printf ("%8.8lx  %8.8lx ",
		  (unsigned long) offset & 0xffffffff,
		  (unsigned long) inf & 0xffffffff);
	}
      else
	{
#if BFD_HOST_64BIT_LONG
	  printf (do_wide
		  ? "%16.16lx  %16.16lx "
		  : "%12.12lx  %12.12lx ",
		  offset, inf);
#elif BFD_HOST_64BIT_LONG_LONG
#ifndef __MSVCRT__
	  printf (do_wide
		  ? "%16.16llx  %16.16llx "
		  : "%12.12llx  %12.12llx ",
		  offset, inf);
#else
	  printf (do_wide
		  ? "%16.16I64x  %16.16I64x "
		  : "%12.12I64x  %12.12I64x ",
		  offset, inf);
#endif
#else
	  printf (do_wide
		  ? "%8.8lx%8.8lx  %8.8lx%8.8lx "
		  : "%4.4lx%8.8lx  %4.4lx%8.8lx ",
		  _bfd_int64_high (offset),
		  _bfd_int64_low (offset),
		  _bfd_int64_high (inf),
		  _bfd_int64_low (inf));
#endif
	}

      switch (elf_header.e_machine)
	{
	default:
	  rtype = NULL;
	  break;

	case EM_AARCH64:
	  rtype = elf_aarch64_reloc_type (type);
	  break;

	case EM_M32R:
	case EM_CYGNUS_M32R:
	  rtype = elf_m32r_reloc_type (type);
	  break;

	case EM_386:
	case EM_486:
	  rtype = elf_i386_reloc_type (type);
	  break;

	case EM_68HC11:
	case EM_68HC12:
	  rtype = elf_m68hc11_reloc_type (type);
	  break;

	case EM_68K:
	  rtype = elf_m68k_reloc_type (type);
	  break;

	case EM_960:
	  rtype = elf_i960_reloc_type (type);
	  break;

	case EM_AVR:
	case EM_AVR_OLD:
	  rtype = elf_avr_reloc_type (type);
	  break;

	case EM_OLD_SPARCV9:
	case EM_SPARC32PLUS:
	case EM_SPARCV9:
	case EM_SPARC:
	  rtype = elf_sparc_reloc_type (type);
	  break;

	case EM_SPU:
	  rtype = elf_spu_reloc_type (type);
	  break;

	case EM_V850:
	case EM_CYGNUS_V850:
	  rtype = v850_reloc_type (type);
	  break;

	case EM_D10V:
	case EM_CYGNUS_D10V:
	  rtype = elf_d10v_reloc_type (type);
	  break;

	case EM_D30V:
	case EM_CYGNUS_D30V:
	  rtype = elf_d30v_reloc_type (type);
	  break;

	case EM_DLX:
	  rtype = elf_dlx_reloc_type (type);
	  break;

	case EM_SH:
	  rtype = elf_sh_reloc_type (type);
	  break;

	case EM_MN10300:
	case EM_CYGNUS_MN10300:
	  rtype = elf_mn10300_reloc_type (type);
	  break;

	case EM_MN10200:
	case EM_CYGNUS_MN10200:
	  rtype = elf_mn10200_reloc_type (type);
	  break;

	case EM_FR30:
	case EM_CYGNUS_FR30:
	  rtype = elf_fr30_reloc_type (type);
	  break;

	case EM_CYGNUS_FRV:
	  rtype = elf_frv_reloc_type (type);
	  break;

	case EM_MCORE:
	  rtype = elf_mcore_reloc_type (type);
	  break;

	case EM_MMIX:
	  rtype = elf_mmix_reloc_type (type);
	  break;

	case EM_MOXIE:
	  rtype = elf_moxie_reloc_type (type);
	  break;

	case EM_MSP430:
	case EM_MSP430_OLD:
	  rtype = elf_msp430_reloc_type (type);
	  break;

	case EM_PPC:
	  rtype = elf_ppc_reloc_type (type);
	  break;

	case EM_PPC64:
	  rtype = elf_ppc64_reloc_type (type);
	  break;

	case EM_MIPS:
	case EM_MIPS_RS3_LE:
	  rtype = elf_mips_reloc_type (type);
	  break;

	case EM_ALPHA:
	  rtype = elf_alpha_reloc_type (type);
	  break;

	case EM_ARM:
	  rtype = elf_arm_reloc_type (type);
	  break;

	case EM_ARC:
	  rtype = elf_arc_reloc_type (type);
	  break;

	case EM_PARISC:
	  rtype = elf_hppa_reloc_type (type);
	  break;

	case EM_H8_300:
	case EM_H8_300H:
	case EM_H8S:
	  rtype = elf_h8_reloc_type (type);
	  break;

	case EM_OPENRISC:
	case EM_OR32:
	  rtype = elf_or32_reloc_type (type);
	  break;

	case EM_PJ:
	case EM_PJ_OLD:
	  rtype = elf_pj_reloc_type (type);
	  break;
	case EM_IA_64:
	  rtype = elf_ia64_reloc_type (type);
	  break;

	case EM_CRIS:
	  rtype = elf_cris_reloc_type (type);
	  break;

	case EM_860:
	  rtype = elf_i860_reloc_type (type);
	  break;

	case EM_X86_64:
	case EM_L1OM:
	case EM_K1OM:
	  rtype = elf_x86_64_reloc_type (type);
	  break;

	case EM_S370:
	  rtype = i370_reloc_type (type);
	  break;

	case EM_S390_OLD:
	case EM_S390:
	  rtype = elf_s390_reloc_type (type);
	  break;

	case EM_SCORE:
	  rtype = elf_score_reloc_type (type);
	  break;

	case EM_XSTORMY16:
	  rtype = elf_xstormy16_reloc_type (type);
	  break;

	case EM_CRX:
	  rtype = elf_crx_reloc_type (type);
	  break;

	case EM_VAX:
	  rtype = elf_vax_reloc_type (type);
	  break;

	case EM_ADAPTEVA_EPIPHANY:
	  rtype = elf_epiphany_reloc_type (type);
	  break;

	case EM_IP2K:
	case EM_IP2K_OLD:
	  rtype = elf_ip2k_reloc_type (type);
	  break;

	case EM_IQ2000:
	  rtype = elf_iq2000_reloc_type (type);
	  break;

	case EM_XTENSA_OLD:
	case EM_XTENSA:
	  rtype = elf_xtensa_reloc_type (type);
	  break;

	case EM_LATTICEMICO32:
	  rtype = elf_lm32_reloc_type (type);
	  break;

	case EM_M32C_OLD:
	case EM_M32C:
	  rtype = elf_m32c_reloc_type (type);
	  break;

	case EM_MT:
	  rtype = elf_mt_reloc_type (type);
	  break;

	case EM_BLACKFIN:
	  rtype = elf_bfin_reloc_type (type);
	  break;

	case EM_CYGNUS_MEP:
	  rtype = elf_mep_reloc_type (type);
	  break;

	case EM_CR16:
	  rtype = elf_cr16_reloc_type (type);
	  break;

	case EM_MICROBLAZE:
	case EM_MICROBLAZE_OLD:
	  rtype = elf_microblaze_reloc_type (type);
	  break;

	case EM_RL78:
	  rtype = elf_rl78_reloc_type (type);
	  break;

	case EM_RX:
	  rtype = elf_rx_reloc_type (type);
	  break;

	case EM_XC16X:
	case EM_C166:
	  rtype = elf_xc16x_reloc_type (type);
	  break;

	case EM_TI_C6000:
	  rtype = elf_tic6x_reloc_type (type);
	  break;

	case EM_TILEGX:
	  rtype = elf_tilegx_reloc_type (type);
	  break;

	case EM_TILEPRO:
	  rtype = elf_tilepro_reloc_type (type);
	  break;

	case EM_XGATE:
	  rtype = elf_xgate_reloc_type (type);
	  break;
	}

      if (rtype == NULL)
	printf (_("unrecognized: %-7lx"), (unsigned long) type & 0xffffffff);
      else
	printf (do_wide ? "%-22.22s" : "%-17.17s", rtype);

      if (elf_header.e_machine == EM_ALPHA
	  && rtype != NULL
	  && streq (rtype, "R_ALPHA_LITUSE")
	  && is_rela)
	{
	  switch (rels[i].r_addend)
	    {
	    case LITUSE_ALPHA_ADDR:   rtype = "ADDR";   break;
	    case LITUSE_ALPHA_BASE:   rtype = "BASE";   break;
	    case LITUSE_ALPHA_BYTOFF: rtype = "BYTOFF"; break;
	    case LITUSE_ALPHA_JSR:    rtype = "JSR";    break;
	    case LITUSE_ALPHA_TLSGD:  rtype = "TLSGD";  break;
	    case LITUSE_ALPHA_TLSLDM: rtype = "TLSLDM"; break;
	    case LITUSE_ALPHA_JSRDIRECT: rtype = "JSRDIRECT"; break;
	    default: rtype = NULL;
	    }
	  if (rtype)
	    printf (" (%s)", rtype);
	  else
	    {
	      putchar (' ');
	      printf (_("<unknown addend: %lx>"),
		      (unsigned long) rels[i].r_addend);
	    }
	}
      else if (symtab_index)
	{
	  if (symtab == NULL || symtab_index >= nsyms)
	    printf (_(" bad symbol index: %08lx"), (unsigned long) symtab_index);
	  else
	    {
	      Elf_Internal_Sym * psym;

	      psym = symtab + symtab_index;

	      printf (" ");

	      if (ELF_ST_TYPE (psym->st_info) == STT_GNU_IFUNC)
		{
		  const char * name;
		  unsigned int len;
		  unsigned int width = is_32bit_elf ? 8 : 14;

		  /* Relocations against GNU_IFUNC symbols do not use the value
		     of the symbol as the address to relocate against.  Instead
		     they invoke the function named by the symbol and use its
		     result as the address for relocation.

		     To indicate this to the user, do not display the value of
		     the symbol in the "Symbols's Value" field.  Instead show
		     its name followed by () as a hint that the symbol is
		     invoked.  */

		  if (strtab == NULL
		      || psym->st_name == 0
		      || psym->st_name >= strtablen)
		    name = "??";
		  else
		    name = strtab + psym->st_name;

		  len = print_symbol (width, name);
		  printf ("()%-*s", len <= width ? (width + 1) - len : 1, " ");
		}
	      else
		{
		  print_vma (psym->st_value, LONG_HEX);

		  printf (is_32bit_elf ? "   " : " ");
		}

	      if (psym->st_name == 0)
		{
		  const char * sec_name = "<null>";
		  char name_buf[40];

		  if (ELF_ST_TYPE (psym->st_info) == STT_SECTION)
		    {
		      if (psym->st_shndx < elf_header.e_shnum)
			sec_name
			  = SECTION_NAME (section_headers + psym->st_shndx);
		      else if (psym->st_shndx == SHN_ABS)
			sec_name = "ABS";
		      else if (psym->st_shndx == SHN_COMMON)
			sec_name = "COMMON";
		      else if ((elf_header.e_machine == EM_MIPS
				&& psym->st_shndx == SHN_MIPS_SCOMMON)
			       || (elf_header.e_machine == EM_TI_C6000
				   && psym->st_shndx == SHN_TIC6X_SCOMMON))
			sec_name = "SCOMMON";
		      else if (elf_header.e_machine == EM_MIPS
			       && psym->st_shndx == SHN_MIPS_SUNDEFINED)
			sec_name = "SUNDEF";
		      else if ((elf_header.e_machine == EM_X86_64
				|| elf_header.e_machine == EM_L1OM
				|| elf_header.e_machine == EM_K1OM)
			       && psym->st_shndx == SHN_X86_64_LCOMMON)
			sec_name = "LARGE_COMMON";
		      else if (elf_header.e_machine == EM_IA_64
			       && elf_header.e_ident[EI_OSABI] == ELFOSABI_HPUX
			       && psym->st_shndx == SHN_IA_64_ANSI_COMMON)
			sec_name = "ANSI_COM";
		      else if (is_ia64_vms ()
			       && psym->st_shndx == SHN_IA_64_VMS_SYMVEC)
			sec_name = "VMS_SYMVEC";
		      else
			{
			  sprintf (name_buf, "<section 0x%x>",
				   (unsigned int) psym->st_shndx);
			  sec_name = name_buf;
			}
		    }
		  print_symbol (22, sec_name);
		}
	      else if (strtab == NULL)
		printf (_("<string table index: %3ld>"), psym->st_name);
	      else if (psym->st_name >= strtablen)
		printf (_("<corrupt string table index: %3ld>"), psym->st_name);
	      else
		print_symbol (22, strtab + psym->st_name);

	      if (is_rela)
		{
		  bfd_signed_vma off = rels[i].r_addend;

		  if (off < 0)
		    printf (" - %" BFD_VMA_FMT "x", - off);
		  else
		    printf (" + %" BFD_VMA_FMT "x", off);
		}
	    }
	}
      else if (is_rela)
	{
	  bfd_signed_vma off = rels[i].r_addend;

	  printf ("%*c", is_32bit_elf ? 12 : 20, ' ');
	  if (off < 0)
	    printf ("-%" BFD_VMA_FMT "x", - off);
	  else
	    printf ("%" BFD_VMA_FMT "x", off);
	}

      if (elf_header.e_machine == EM_SPARCV9
	  && rtype != NULL
	  && streq (rtype, "R_SPARC_OLO10"))
	printf (" + %lx", (unsigned long) ELF64_R_TYPE_DATA (inf));

      putchar ('\n');

#ifdef BFD64
      if (! is_32bit_elf && elf_header.e_machine == EM_MIPS)
	{
	  bfd_vma type2 = ELF64_MIPS_R_TYPE2 (inf);
	  bfd_vma type3 = ELF64_MIPS_R_TYPE3 (inf);
	  const char * rtype2 = elf_mips_reloc_type (type2);
	  const char * rtype3 = elf_mips_reloc_type (type3);

	  printf ("                    Type2: ");

	  if (rtype2 == NULL)
	    printf (_("unrecognized: %-7lx"),
		    (unsigned long) type2 & 0xffffffff);
	  else
	    printf ("%-17.17s", rtype2);

	  printf ("\n                    Type3: ");

	  if (rtype3 == NULL)
	    printf (_("unrecognized: %-7lx"),
		    (unsigned long) type3 & 0xffffffff);
	  else
	    printf ("%-17.17s", rtype3);

	  putchar ('\n');
	}
#endif /* BFD64 */
    }

  free (rels);
}

static const char *
get_mips_dynamic_type (unsigned long type)
{
  switch (type)
    {
    case DT_MIPS_RLD_VERSION: return "MIPS_RLD_VERSION";
    case DT_MIPS_TIME_STAMP: return "MIPS_TIME_STAMP";
    case DT_MIPS_ICHECKSUM: return "MIPS_ICHECKSUM";
    case DT_MIPS_IVERSION: return "MIPS_IVERSION";
    case DT_MIPS_FLAGS: return "MIPS_FLAGS";
    case DT_MIPS_BASE_ADDRESS: return "MIPS_BASE_ADDRESS";
    case DT_MIPS_MSYM: return "MIPS_MSYM";
    case DT_MIPS_CONFLICT: return "MIPS_CONFLICT";
    case DT_MIPS_LIBLIST: return "MIPS_LIBLIST";
    case DT_MIPS_LOCAL_GOTNO: return "MIPS_LOCAL_GOTNO";
    case DT_MIPS_CONFLICTNO: return "MIPS_CONFLICTNO";
    case DT_MIPS_LIBLISTNO: return "MIPS_LIBLISTNO";
    case DT_MIPS_SYMTABNO: return "MIPS_SYMTABNO";
    case DT_MIPS_UNREFEXTNO: return "MIPS_UNREFEXTNO";
    case DT_MIPS_GOTSYM: return "MIPS_GOTSYM";
    case DT_MIPS_HIPAGENO: return "MIPS_HIPAGENO";
    case DT_MIPS_RLD_MAP: return "MIPS_RLD_MAP";
    case DT_MIPS_DELTA_CLASS: return "MIPS_DELTA_CLASS";
    case DT_MIPS_DELTA_CLASS_NO: return "MIPS_DELTA_CLASS_NO";
    case DT_MIPS_DELTA_INSTANCE: return "MIPS_DELTA_INSTANCE";
    case DT_MIPS_DELTA_INSTANCE_NO: return "MIPS_DELTA_INSTANCE_NO";
    case DT_MIPS_DELTA_RELOC: return "MIPS_DELTA_RELOC";
    case DT_MIPS_DELTA_RELOC_NO: return "MIPS_DELTA_RELOC_NO";
    case DT_MIPS_DELTA_SYM: return "MIPS_DELTA_SYM";
    case DT_MIPS_DELTA_SYM_NO: return "MIPS_DELTA_SYM_NO";
    case DT_MIPS_DELTA_CLASSSYM: return "MIPS_DELTA_CLASSSYM";
    case DT_MIPS_DELTA_CLASSSYM_NO: return "MIPS_DELTA_CLASSSYM_NO";
    case DT_MIPS_CXX_FLAGS: return "MIPS_CXX_FLAGS";
    case DT_MIPS_PIXIE_INIT: return "MIPS_PIXIE_INIT";
    case DT_MIPS_SYMBOL_LIB: return "MIPS_SYMBOL_LIB";
    case DT_MIPS_LOCALPAGE_GOTIDX: return "MIPS_LOCALPAGE_GOTIDX";
    case DT_MIPS_LOCAL_GOTIDX: return "MIPS_LOCAL_GOTIDX";
    case DT_MIPS_HIDDEN_GOTIDX: return "MIPS_HIDDEN_GOTIDX";
    case DT_MIPS_PROTECTED_GOTIDX: return "MIPS_PROTECTED_GOTIDX";
    case DT_MIPS_OPTIONS: return "MIPS_OPTIONS";
    case DT_MIPS_INTERFACE: return "MIPS_INTERFACE";
    case DT_MIPS_DYNSTR_ALIGN: return "MIPS_DYNSTR_ALIGN";
    case DT_MIPS_INTERFACE_SIZE: return "MIPS_INTERFACE_SIZE";
    case DT_MIPS_RLD_TEXT_RESOLVE_ADDR: return "MIPS_RLD_TEXT_RESOLVE_ADDR";
    case DT_MIPS_PERF_SUFFIX: return "MIPS_PERF_SUFFIX";
    case DT_MIPS_COMPACT_SIZE: return "MIPS_COMPACT_SIZE";
    case DT_MIPS_GP_VALUE: return "MIPS_GP_VALUE";
    case DT_MIPS_AUX_DYNAMIC: return "MIPS_AUX_DYNAMIC";
    case DT_MIPS_PLTGOT: return "MIPS_PLTGOT";
    case DT_MIPS_RWPLT: return "MIPS_RWPLT";
    default:
      return NULL;
    }
}

static const char *
get_sparc64_dynamic_type (unsigned long type)
{
  switch (type)
    {
    case DT_SPARC_REGISTER: return "SPARC_REGISTER";
    default:
      return NULL;
    }
}

static const char *
get_ppc_dynamic_type (unsigned long type)
{
  switch (type)
    {
    case DT_PPC_GOT:    return "PPC_GOT";
    case DT_PPC_TLSOPT: return "PPC_TLSOPT";
    default:
      return NULL;
    }
}

static const char *
get_ppc64_dynamic_type (unsigned long type)
{
  switch (type)
    {
    case DT_PPC64_GLINK:  return "PPC64_GLINK";
    case DT_PPC64_OPD:    return "PPC64_OPD";
    case DT_PPC64_OPDSZ:  return "PPC64_OPDSZ";
    case DT_PPC64_TLSOPT: return "PPC64_TLSOPT";
    default:
      return NULL;
    }
}

static const char *
get_parisc_dynamic_type (unsigned long type)
{
  switch (type)
    {
    case DT_HP_LOAD_MAP:	return "HP_LOAD_MAP";
    case DT_HP_DLD_FLAGS:	return "HP_DLD_FLAGS";
    case DT_HP_DLD_HOOK:	return "HP_DLD_HOOK";
    case DT_HP_UX10_INIT:	return "HP_UX10_INIT";
    case DT_HP_UX10_INITSZ:	return "HP_UX10_INITSZ";
    case DT_HP_PREINIT:		return "HP_PREINIT";
    case DT_HP_PREINITSZ:	return "HP_PREINITSZ";
    case DT_HP_NEEDED:		return "HP_NEEDED";
    case DT_HP_TIME_STAMP:	return "HP_TIME_STAMP";
    case DT_HP_CHECKSUM:	return "HP_CHECKSUM";
    case DT_HP_GST_SIZE:	return "HP_GST_SIZE";
    case DT_HP_GST_VERSION:	return "HP_GST_VERSION";
    case DT_HP_GST_HASHVAL:	return "HP_GST_HASHVAL";
    case DT_HP_EPLTREL:		return "HP_GST_EPLTREL";
    case DT_HP_EPLTRELSZ:	return "HP_GST_EPLTRELSZ";
    case DT_HP_FILTERED:	return "HP_FILTERED";
    case DT_HP_FILTER_TLS:	return "HP_FILTER_TLS";
    case DT_HP_COMPAT_FILTERED:	return "HP_COMPAT_FILTERED";
    case DT_HP_LAZYLOAD:	return "HP_LAZYLOAD";
    case DT_HP_BIND_NOW_COUNT:	return "HP_BIND_NOW_COUNT";
    case DT_PLT:		return "PLT";
    case DT_PLT_SIZE:		return "PLT_SIZE";
    case DT_DLT:		return "DLT";
    case DT_DLT_SIZE:		return "DLT_SIZE";
    default:
      return NULL;
    }
}

static const char *
get_ia64_dynamic_type (unsigned long type)
{
  switch (type)
    {
    case DT_IA_64_PLT_RESERVE:         return "IA_64_PLT_RESERVE";
    case DT_IA_64_VMS_SUBTYPE:         return "VMS_SUBTYPE";
    case DT_IA_64_VMS_IMGIOCNT:        return "VMS_IMGIOCNT";
    case DT_IA_64_VMS_LNKFLAGS:        return "VMS_LNKFLAGS";
    case DT_IA_64_VMS_VIR_MEM_BLK_SIZ: return "VMS_VIR_MEM_BLK_SIZ";
    case DT_IA_64_VMS_IDENT:           return "VMS_IDENT";
    case DT_IA_64_VMS_NEEDED_IDENT:    return "VMS_NEEDED_IDENT";
    case DT_IA_64_VMS_IMG_RELA_CNT:    return "VMS_IMG_RELA_CNT";
    case DT_IA_64_VMS_SEG_RELA_CNT:    return "VMS_SEG_RELA_CNT";
    case DT_IA_64_VMS_FIXUP_RELA_CNT:  return "VMS_FIXUP_RELA_CNT";
    case DT_IA_64_VMS_FIXUP_NEEDED:    return "VMS_FIXUP_NEEDED";
    case DT_IA_64_VMS_SYMVEC_CNT:      return "VMS_SYMVEC_CNT";
    case DT_IA_64_VMS_XLATED:          return "VMS_XLATED";
    case DT_IA_64_VMS_STACKSIZE:       return "VMS_STACKSIZE";
    case DT_IA_64_VMS_UNWINDSZ:        return "VMS_UNWINDSZ";
    case DT_IA_64_VMS_UNWIND_CODSEG:   return "VMS_UNWIND_CODSEG";
    case DT_IA_64_VMS_UNWIND_INFOSEG:  return "VMS_UNWIND_INFOSEG";
    case DT_IA_64_VMS_LINKTIME:        return "VMS_LINKTIME";
    case DT_IA_64_VMS_SEG_NO:          return "VMS_SEG_NO";
    case DT_IA_64_VMS_SYMVEC_OFFSET:   return "VMS_SYMVEC_OFFSET";
    case DT_IA_64_VMS_SYMVEC_SEG:      return "VMS_SYMVEC_SEG";
    case DT_IA_64_VMS_UNWIND_OFFSET:   return "VMS_UNWIND_OFFSET";
    case DT_IA_64_VMS_UNWIND_SEG:      return "VMS_UNWIND_SEG";
    case DT_IA_64_VMS_STRTAB_OFFSET:   return "VMS_STRTAB_OFFSET";
    case DT_IA_64_VMS_SYSVER_OFFSET:   return "VMS_SYSVER_OFFSET";
    case DT_IA_64_VMS_IMG_RELA_OFF:    return "VMS_IMG_RELA_OFF";
    case DT_IA_64_VMS_SEG_RELA_OFF:    return "VMS_SEG_RELA_OFF";
    case DT_IA_64_VMS_FIXUP_RELA_OFF:  return "VMS_FIXUP_RELA_OFF";
    case DT_IA_64_VMS_PLTGOT_OFFSET:   return "VMS_PLTGOT_OFFSET";
    case DT_IA_64_VMS_PLTGOT_SEG:      return "VMS_PLTGOT_SEG";
    case DT_IA_64_VMS_FPMODE:          return "VMS_FPMODE";
    default:
      return NULL;
    }
}

static const char *
get_alpha_dynamic_type (unsigned long type)
{
  switch (type)
    {
    case DT_ALPHA_PLTRO: return "ALPHA_PLTRO";
    default:
      return NULL;
    }
}

static const char *
get_score_dynamic_type (unsigned long type)
{
  switch (type)
    {
    case DT_SCORE_BASE_ADDRESS: return "SCORE_BASE_ADDRESS";
    case DT_SCORE_LOCAL_GOTNO:  return "SCORE_LOCAL_GOTNO";
    case DT_SCORE_SYMTABNO:     return "SCORE_SYMTABNO";
    case DT_SCORE_GOTSYM:       return "SCORE_GOTSYM";
    case DT_SCORE_UNREFEXTNO:   return "SCORE_UNREFEXTNO";
    case DT_SCORE_HIPAGENO:     return "SCORE_HIPAGENO";
    default:
      return NULL;
    }
}

static const char *
get_tic6x_dynamic_type (unsigned long type)
{
  switch (type)
    {
    case DT_C6000_GSYM_OFFSET: return "C6000_GSYM_OFFSET";
    case DT_C6000_GSTR_OFFSET: return "C6000_GSTR_OFFSET";
    case DT_C6000_DSBT_BASE:   return "C6000_DSBT_BASE";
    case DT_C6000_DSBT_SIZE:   return "C6000_DSBT_SIZE";
    case DT_C6000_PREEMPTMAP:  return "C6000_PREEMPTMAP";
    case DT_C6000_DSBT_INDEX:  return "C6000_DSBT_INDEX";
    default:
      return NULL;
    }
}

static const char *
get_dynamic_type (unsigned long type)
{
  static char buff[64];

  switch (type)
    {
    case DT_NULL:	return "NULL";
    case DT_NEEDED:	return "NEEDED";
    case DT_PLTRELSZ:	return "PLTRELSZ";
    case DT_PLTGOT:	return "PLTGOT";
    case DT_HASH:	return "HASH";
    case DT_STRTAB:	return "STRTAB";
    case DT_SYMTAB:	return "SYMTAB";
    case DT_RELA:	return "RELA";
    case DT_RELASZ:	return "RELASZ";
    case DT_RELAENT:	return "RELAENT";
    case DT_STRSZ:	return "STRSZ";
    case DT_SYMENT:	return "SYMENT";
    case DT_INIT:	return "INIT";
    case DT_FINI:	return "FINI";
    case DT_SONAME:	return "SONAME";
    case DT_RPATH:	return "RPATH";
    case DT_SYMBOLIC:	return "SYMBOLIC";
    case DT_REL:	return "REL";
    case DT_RELSZ:	return "RELSZ";
    case DT_RELENT:	return "RELENT";
    case DT_PLTREL:	return "PLTREL";
    case DT_DEBUG:	return "DEBUG";
    case DT_TEXTREL:	return "TEXTREL";
    case DT_JMPREL:	return "JMPREL";
    case DT_BIND_NOW:   return "BIND_NOW";
    case DT_INIT_ARRAY: return "INIT_ARRAY";
    case DT_FINI_ARRAY: return "FINI_ARRAY";
    case DT_INIT_ARRAYSZ: return "INIT_ARRAYSZ";
    case DT_FINI_ARRAYSZ: return "FINI_ARRAYSZ";
    case DT_RUNPATH:    return "RUNPATH";
    case DT_FLAGS:      return "FLAGS";

    case DT_PREINIT_ARRAY: return "PREINIT_ARRAY";
    case DT_PREINIT_ARRAYSZ: return "PREINIT_ARRAYSZ";

    case DT_CHECKSUM:	return "CHECKSUM";
    case DT_PLTPADSZ:	return "PLTPADSZ";
    case DT_MOVEENT:	return "MOVEENT";
    case DT_MOVESZ:	return "MOVESZ";
    case DT_FEATURE:	return "FEATURE";
    case DT_POSFLAG_1:	return "POSFLAG_1";
    case DT_SYMINSZ:	return "SYMINSZ";
    case DT_SYMINENT:	return "SYMINENT"; /* aka VALRNGHI */

    case DT_ADDRRNGLO:  return "ADDRRNGLO";
    case DT_CONFIG:	return "CONFIG";
    case DT_DEPAUDIT:	return "DEPAUDIT";
    case DT_AUDIT:	return "AUDIT";
    case DT_PLTPAD:	return "PLTPAD";
    case DT_MOVETAB:	return "MOVETAB";
    case DT_SYMINFO:	return "SYMINFO"; /* aka ADDRRNGHI */

    case DT_VERSYM:	return "VERSYM";

    case DT_TLSDESC_GOT: return "TLSDESC_GOT";
    case DT_TLSDESC_PLT: return "TLSDESC_PLT";
    case DT_RELACOUNT:	return "RELACOUNT";
    case DT_RELCOUNT:	return "RELCOUNT";
    case DT_FLAGS_1:	return "FLAGS_1";
    case DT_VERDEF:	return "VERDEF";
    case DT_VERDEFNUM:	return "VERDEFNUM";
    case DT_VERNEED:	return "VERNEED";
    case DT_VERNEEDNUM:	return "VERNEEDNUM";

    case DT_AUXILIARY:	return "AUXILIARY";
    case DT_USED:	return "USED";
    case DT_FILTER:	return "FILTER";

    case DT_GNU_PRELINKED: return "GNU_PRELINKED";
    case DT_GNU_CONFLICT: return "GNU_CONFLICT";
    case DT_GNU_CONFLICTSZ: return "GNU_CONFLICTSZ";
    case DT_GNU_LIBLIST: return "GNU_LIBLIST";
    case DT_GNU_LIBLISTSZ: return "GNU_LIBLISTSZ";
    case DT_GNU_HASH:	return "GNU_HASH";

    default:
      if ((type >= DT_LOPROC) && (type <= DT_HIPROC))
	{
	  const char * result;

	  switch (elf_header.e_machine)
	    {
	    case EM_MIPS:
	    case EM_MIPS_RS3_LE:
	      result = get_mips_dynamic_type (type);
	      break;
	    case EM_SPARCV9:
	      result = get_sparc64_dynamic_type (type);
	      break;
	    case EM_PPC:
	      result = get_ppc_dynamic_type (type);
	      break;
	    case EM_PPC64:
	      result = get_ppc64_dynamic_type (type);
	      break;
	    case EM_IA_64:
	      result = get_ia64_dynamic_type (type);
	      break;
	    case EM_ALPHA:
	      result = get_alpha_dynamic_type (type);
	      break;
	    case EM_SCORE:
	      result = get_score_dynamic_type (type);
	      break;
	    case EM_TI_C6000:
	      result = get_tic6x_dynamic_type (type);
	      break;
	    default:
	      result = NULL;
	      break;
	    }

	  if (result != NULL)
	    return result;

	  snprintf (buff, sizeof (buff), _("Processor Specific: %lx"), type);
	}
      else if (((type >= DT_LOOS) && (type <= DT_HIOS))
	       || (elf_header.e_machine == EM_PARISC
		   && (type >= OLD_DT_LOOS) && (type <= OLD_DT_HIOS)))
	{
	  const char * result;

	  switch (elf_header.e_machine)
	    {
	    case EM_PARISC:
	      result = get_parisc_dynamic_type (type);
	      break;
	    case EM_IA_64:
	      result = get_ia64_dynamic_type (type);
	      break;
	    default:
	      result = NULL;
	      break;
	    }

	  if (result != NULL)
	    return result;

	  snprintf (buff, sizeof (buff), _("Operating System specific: %lx"),
		    type);
	}
      else
	snprintf (buff, sizeof (buff), _("<unknown>: %lx"), type);

      return buff;
    }
}

static char *
get_file_type (unsigned e_type)
{
  static char buff[32];

  switch (e_type)
    {
    case ET_NONE:	return _("NONE (None)");
    case ET_REL:	return _("REL (Relocatable file)");
    case ET_EXEC:	return _("EXEC (Executable file)");
    case ET_DYN:	return _("DYN (Shared object file)");
    case ET_CORE:	return _("CORE (Core file)");

    default:
      if ((e_type >= ET_LOPROC) && (e_type <= ET_HIPROC))
	snprintf (buff, sizeof (buff), _("Processor Specific: (%x)"), e_type);
      else if ((e_type >= ET_LOOS) && (e_type <= ET_HIOS))
	snprintf (buff, sizeof (buff), _("OS Specific: (%x)"), e_type);
      else
	snprintf (buff, sizeof (buff), _("<unknown>: %x"), e_type);
      return buff;
    }
}

static char *
get_machine_name (unsigned e_machine)
{
  static char buff[64]; /* XXX */

  switch (e_machine)
    {
    case EM_NONE:		return _("None");
    case EM_AARCH64:		return "AArch64";
    case EM_M32:		return "WE32100";
    case EM_SPARC:		return "Sparc";
    case EM_SPU:		return "SPU";
    case EM_386:		return "Intel 80386";
    case EM_68K:		return "MC68000";
    case EM_88K:		return "MC88000";
    case EM_486:		return "Intel 80486";
    case EM_860:		return "Intel 80860";
    case EM_MIPS:		return "MIPS R3000";
    case EM_S370:		return "IBM System/370";
    case EM_MIPS_RS3_LE:	return "MIPS R4000 big-endian";
    case EM_OLD_SPARCV9:	return "Sparc v9 (old)";
    case EM_PARISC:		return "HPPA";
    case EM_PPC_OLD:		return "Power PC (old)";
    case EM_SPARC32PLUS:	return "Sparc v8+" ;
    case EM_960:		return "Intel 90860";
    case EM_PPC:		return "PowerPC";
    case EM_PPC64:		return "PowerPC64";
    case EM_V800:		return "NEC V800";
    case EM_FR20:		return "Fujitsu FR20";
    case EM_RH32:		return "TRW RH32";
    case EM_MCORE:		return "MCORE";
    case EM_ARM:		return "ARM";
    case EM_OLD_ALPHA:		return "Digital Alpha (old)";
    case EM_SH:			return "Renesas / SuperH SH";
    case EM_SPARCV9:		return "Sparc v9";
    case EM_TRICORE:		return "Siemens Tricore";
    case EM_ARC:		return "ARC";
    case EM_H8_300:		return "Renesas H8/300";
    case EM_H8_300H:		return "Renesas H8/300H";
    case EM_H8S:		return "Renesas H8S";
    case EM_H8_500:		return "Renesas H8/500";
    case EM_IA_64:		return "Intel IA-64";
    case EM_MIPS_X:		return "Stanford MIPS-X";
    case EM_COLDFIRE:		return "Motorola Coldfire";
    case EM_ALPHA:		return "Alpha";
    case EM_CYGNUS_D10V:
    case EM_D10V:		return "d10v";
    case EM_CYGNUS_D30V:
    case EM_D30V:		return "d30v";
    case EM_CYGNUS_M32R:
    case EM_M32R:		return "Renesas M32R (formerly Mitsubishi M32r)";
    case EM_CYGNUS_V850:
    case EM_V850:		return "Renesas V850";
    case EM_CYGNUS_MN10300:
    case EM_MN10300:		return "mn10300";
    case EM_CYGNUS_MN10200:
    case EM_MN10200:		return "mn10200";
    case EM_MOXIE:		return "Moxie";
    case EM_CYGNUS_FR30:
    case EM_FR30:		return "Fujitsu FR30";
    case EM_CYGNUS_FRV:		return "Fujitsu FR-V";
    case EM_PJ_OLD:
    case EM_PJ:			return "picoJava";
    case EM_MMA:		return "Fujitsu Multimedia Accelerator";
    case EM_PCP:		return "Siemens PCP";
    case EM_NCPU:		return "Sony nCPU embedded RISC processor";
    case EM_NDR1:		return "Denso NDR1 microprocesspr";
    case EM_STARCORE:		return "Motorola Star*Core processor";
    case EM_ME16:		return "Toyota ME16 processor";
    case EM_ST100:		return "STMicroelectronics ST100 processor";
    case EM_TINYJ:		return "Advanced Logic Corp. TinyJ embedded processor";
    case EM_PDSP:		return "Sony DSP processor";
    case EM_PDP10:		return "Digital Equipment Corp. PDP-10";
    case EM_PDP11:		return "Digital Equipment Corp. PDP-11";
    case EM_FX66:		return "Siemens FX66 microcontroller";
    case EM_ST9PLUS:		return "STMicroelectronics ST9+ 8/16 bit microcontroller";
    case EM_ST7:		return "STMicroelectronics ST7 8-bit microcontroller";
    case EM_68HC16:		return "Motorola MC68HC16 Microcontroller";
    case EM_68HC12:		return "Motorola MC68HC12 Microcontroller";
    case EM_68HC11:		return "Motorola MC68HC11 Microcontroller";
    case EM_68HC08:		return "Motorola MC68HC08 Microcontroller";
    case EM_68HC05:		return "Motorola MC68HC05 Microcontroller";
    case EM_SVX:		return "Silicon Graphics SVx";
    case EM_ST19:		return "STMicroelectronics ST19 8-bit microcontroller";
    case EM_VAX:		return "Digital VAX";
    case EM_AVR_OLD:
    case EM_AVR:		return "Atmel AVR 8-bit microcontroller";
    case EM_CRIS:		return "Axis Communications 32-bit embedded processor";
    case EM_JAVELIN:		return "Infineon Technologies 32-bit embedded cpu";
    case EM_FIREPATH:		return "Element 14 64-bit DSP processor";
    case EM_ZSP:		return "LSI Logic's 16-bit DSP processor";
    case EM_MMIX:		return "Donald Knuth's educational 64-bit processor";
    case EM_HUANY:		return "Harvard Universitys's machine-independent object format";
    case EM_PRISM:		return "Vitesse Prism";
    case EM_X86_64:		return "Advanced Micro Devices X86-64";
    case EM_L1OM:		return "Intel L1OM";
    case EM_K1OM:		return "Intel K1OM";
    case EM_S390_OLD:
    case EM_S390:		return "IBM S/390";
    case EM_SCORE:		return "SUNPLUS S+Core";
    case EM_XSTORMY16:		return "Sanyo XStormy16 CPU core";
    case EM_OPENRISC:
    case EM_OR32:		return "OpenRISC";
    case EM_ARC_A5:		return "ARC International ARCompact processor";
    case EM_CRX:		return "National Semiconductor CRX microprocessor";
    case EM_ADAPTEVA_EPIPHANY:	return "Adapteva EPIPHANY";
    case EM_DLX:		return "OpenDLX";
    case EM_IP2K_OLD:
    case EM_IP2K:		return "Ubicom IP2xxx 8-bit microcontrollers";
    case EM_IQ2000:       	return "Vitesse IQ2000";
    case EM_XTENSA_OLD:
    case EM_XTENSA:		return "Tensilica Xtensa Processor";
    case EM_VIDEOCORE:		return "Alphamosaic VideoCore processor";
    case EM_TMM_GPP:		return "Thompson Multimedia General Purpose Processor";
    case EM_NS32K:		return "National Semiconductor 32000 series";
    case EM_TPC:		return "Tenor Network TPC processor";
    case EM_ST200:		return "STMicroelectronics ST200 microcontroller";
    case EM_MAX:		return "MAX Processor";
    case EM_CR:			return "National Semiconductor CompactRISC";
    case EM_F2MC16:		return "Fujitsu F2MC16";
    case EM_MSP430:		return "Texas Instruments msp430 microcontroller";
    case EM_LATTICEMICO32:	return "Lattice Mico32";
    case EM_M32C_OLD:
    case EM_M32C:	        return "Renesas M32c";
    case EM_MT:                 return "Morpho Techologies MT processor";
    case EM_BLACKFIN:		return "Analog Devices Blackfin";
    case EM_SE_C33:		return "S1C33 Family of Seiko Epson processors";
    case EM_SEP:		return "Sharp embedded microprocessor";
    case EM_ARCA:		return "Arca RISC microprocessor";
    case EM_UNICORE:		return "Unicore";
    case EM_EXCESS:		return "eXcess 16/32/64-bit configurable embedded CPU";
    case EM_DXP:		return "Icera Semiconductor Inc. Deep Execution Processor";
    case EM_NIOS32:		return "Altera Nios";
    case EM_ALTERA_NIOS2:	return "Altera Nios II";
    case EM_C166:
    case EM_XC16X:		return "Infineon Technologies xc16x";
    case EM_M16C:		return "Renesas M16C series microprocessors";
    case EM_DSPIC30F:		return "Microchip Technology dsPIC30F Digital Signal Controller";
    case EM_CE:			return "Freescale Communication Engine RISC core";
    case EM_TSK3000:		return "Altium TSK3000 core";
    case EM_RS08:		return "Freescale RS08 embedded processor";
    case EM_ECOG2:		return "Cyan Technology eCOG2 microprocessor";
    case EM_DSP24:		return "New Japan Radio (NJR) 24-bit DSP Processor";
    case EM_VIDEOCORE3:		return "Broadcom VideoCore III processor";
    case EM_SE_C17:		return "Seiko Epson C17 family";
    case EM_TI_C6000:		return "Texas Instruments TMS320C6000 DSP family";
    case EM_TI_C2000:		return "Texas Instruments TMS320C2000 DSP family";
    case EM_TI_C5500:		return "Texas Instruments TMS320C55x DSP family";
    case EM_MMDSP_PLUS:		return "STMicroelectronics 64bit VLIW Data Signal Processor";
    case EM_CYPRESS_M8C:	return "Cypress M8C microprocessor";
    case EM_R32C:		return "Renesas R32C series microprocessors";
    case EM_TRIMEDIA:		return "NXP Semiconductors TriMedia architecture family";
    case EM_QDSP6:		return "QUALCOMM DSP6 Processor";
    case EM_8051:		return "Intel 8051 and variants";
    case EM_STXP7X:		return "STMicroelectronics STxP7x family";
    case EM_NDS32:		return "Andes Technology compact code size embedded RISC processor family";
    case EM_ECOG1X:		return "Cyan Technology eCOG1X family";
    case EM_MAXQ30:		return "Dallas Semiconductor MAXQ30 Core microcontrollers";
    case EM_XIMO16:		return "New Japan Radio (NJR) 16-bit DSP Processor";
    case EM_MANIK:		return "M2000 Reconfigurable RISC Microprocessor";
    case EM_CRAYNV2:		return "Cray Inc. NV2 vector architecture";
    case EM_CYGNUS_MEP:         return "Toshiba MeP Media Engine";
    case EM_CR16:
    case EM_MICROBLAZE:
    case EM_MICROBLAZE_OLD:	return "Xilinx MicroBlaze";
    case EM_RL78:		return "Renesas RL78";
    case EM_RX:			return "Renesas RX";
    case EM_METAG:		return "Imagination Technologies META processor architecture";
    case EM_MCST_ELBRUS:	return "MCST Elbrus general purpose hardware architecture";
    case EM_ECOG16:		return "Cyan Technology eCOG16 family";
    case EM_ETPU:		return "Freescale Extended Time Processing Unit";
    case EM_SLE9X:		return "Infineon Technologies SLE9X core";
    case EM_AVR32:		return "Atmel Corporation 32-bit microprocessor family";
    case EM_STM8:		return "STMicroeletronics STM8 8-bit microcontroller";
    case EM_TILE64:		return "Tilera TILE64 multicore architecture family";
    case EM_TILEPRO:		return "Tilera TILEPro multicore architecture family";
    case EM_TILEGX:		return "Tilera TILE-Gx multicore architecture family";
    case EM_CUDA:		return "NVIDIA CUDA architecture";
    case EM_XGATE:		return "Motorola XGATE embedded processor";
    default:
      snprintf (buff, sizeof (buff), _("<unknown>: 0x%x"), e_machine);
      return buff;
    }
}

static void
decode_ARM_machine_flags (unsigned e_flags, char buf[])
{
  unsigned eabi;
  int unknown = 0;

  eabi = EF_ARM_EABI_VERSION (e_flags);
  e_flags &= ~ EF_ARM_EABIMASK;

  /* Handle "generic" ARM flags.  */
  if (e_flags & EF_ARM_RELEXEC)
    {
      strcat (buf, ", relocatable executable");
      e_flags &= ~ EF_ARM_RELEXEC;
    }

  if (e_flags & EF_ARM_HASENTRY)
    {
      strcat (buf, ", has entry point");
      e_flags &= ~ EF_ARM_HASENTRY;
    }

  /* Now handle EABI specific flags.  */
  switch (eabi)
    {
    default:
      strcat (buf, ", <unrecognized EABI>");
      if (e_flags)
	unknown = 1;
      break;

    case EF_ARM_EABI_VER1:
      strcat (buf, ", Version1 EABI");
      while (e_flags)
	{
	  unsigned flag;

	  /* Process flags one bit at a time.  */
	  flag = e_flags & - e_flags;
	  e_flags &= ~ flag;

	  switch (flag)
	    {
	    case EF_ARM_SYMSARESORTED: /* Conflicts with EF_ARM_INTERWORK.  */
	      strcat (buf, ", sorted symbol tables");
	      break;

	    default:
	      unknown = 1;
	      break;
	    }
	}
      break;

    case EF_ARM_EABI_VER2:
      strcat (buf, ", Version2 EABI");
      while (e_flags)
	{
	  unsigned flag;

	  /* Process flags one bit at a time.  */
	  flag = e_flags & - e_flags;
	  e_flags &= ~ flag;

	  switch (flag)
	    {
	    case EF_ARM_SYMSARESORTED: /* Conflicts with EF_ARM_INTERWORK.  */
	      strcat (buf, ", sorted symbol tables");
	      break;

	    case EF_ARM_DYNSYMSUSESEGIDX:
	      strcat (buf, ", dynamic symbols use segment index");
	      break;

	    case EF_ARM_MAPSYMSFIRST:
	      strcat (buf, ", mapping symbols precede others");
	      break;

	    default:
	      unknown = 1;
	      break;
	    }
	}
      break;

    case EF_ARM_EABI_VER3:
      strcat (buf, ", Version3 EABI");
      break;

    case EF_ARM_EABI_VER4:
      strcat (buf, ", Version4 EABI");
      goto eabi;

    case EF_ARM_EABI_VER5:
      strcat (buf, ", Version5 EABI");
    eabi:
      while (e_flags)
	{
	  unsigned flag;

	  /* Process flags one bit at a time.  */
	  flag = e_flags & - e_flags;
	  e_flags &= ~ flag;

	  switch (flag)
	    {
	    case EF_ARM_BE8:
	      strcat (buf, ", BE8");
	      break;

	    case EF_ARM_LE8:
	      strcat (buf, ", LE8");
	      break;

	    default:
	      unknown = 1;
	      break;
	    }
	}
      break;

    case EF_ARM_EABI_UNKNOWN:
      strcat (buf, ", GNU EABI");
      while (e_flags)
	{
	  unsigned flag;

	  /* Process flags one bit at a time.  */
	  flag = e_flags & - e_flags;
	  e_flags &= ~ flag;

	  switch (flag)
	    {
	    case EF_ARM_INTERWORK:
	      strcat (buf, ", interworking enabled");
	      break;

	    case EF_ARM_APCS_26:
	      strcat (buf, ", uses APCS/26");
	      break;

	    case EF_ARM_APCS_FLOAT:
	      strcat (buf, ", uses APCS/float");
	      break;

	    case EF_ARM_PIC:
	      strcat (buf, ", position independent");
	      break;

	    case EF_ARM_ALIGN8:
	      strcat (buf, ", 8 bit structure alignment");
	      break;

	    case EF_ARM_NEW_ABI:
	      strcat (buf, ", uses new ABI");
	      break;

	    case EF_ARM_OLD_ABI:
	      strcat (buf, ", uses old ABI");
	      break;

	    case EF_ARM_SOFT_FLOAT:
	      strcat (buf, ", software FP");
	      break;

	    case EF_ARM_VFP_FLOAT:
	      strcat (buf, ", VFP");
	      break;

	    case EF_ARM_MAVERICK_FLOAT:
	      strcat (buf, ", Maverick FP");
	      break;

	    default:
	      unknown = 1;
	      break;
	    }
	}
    }

  if (unknown)
    strcat (buf,_(", <unknown>"));
}

static char *
get_machine_flags (unsigned e_flags, unsigned e_machine)
{
  static char buf[1024];

  buf[0] = '\0';

  if (e_flags)
    {
      switch (e_machine)
	{
	default:
	  break;

	case EM_ARM:
	  decode_ARM_machine_flags (e_flags, buf);
	  break;

	case EM_BLACKFIN:
	  if (e_flags & EF_BFIN_PIC)
	    strcat (buf, ", PIC");

	  if (e_flags & EF_BFIN_FDPIC)
	    strcat (buf, ", FDPIC");

	  if (e_flags & EF_BFIN_CODE_IN_L1)
	    strcat (buf, ", code in L1");

	  if (e_flags & EF_BFIN_DATA_IN_L1)
	    strcat (buf, ", data in L1");

	  break;

	case EM_CYGNUS_FRV:
	  switch (e_flags & EF_FRV_CPU_MASK)
	    {
	    case EF_FRV_CPU_GENERIC:
	      break;

	    default:
	      strcat (buf, ", fr???");
	      break;

	    case EF_FRV_CPU_FR300:
	      strcat (buf, ", fr300");
	      break;

	    case EF_FRV_CPU_FR400:
	      strcat (buf, ", fr400");
	      break;
	    case EF_FRV_CPU_FR405:
	      strcat (buf, ", fr405");
	      break;

	    case EF_FRV_CPU_FR450:
	      strcat (buf, ", fr450");
	      break;

	    case EF_FRV_CPU_FR500:
	      strcat (buf, ", fr500");
	      break;
	    case EF_FRV_CPU_FR550:
	      strcat (buf, ", fr550");
	      break;

	    case EF_FRV_CPU_SIMPLE:
	      strcat (buf, ", simple");
	      break;
	    case EF_FRV_CPU_TOMCAT:
	      strcat (buf, ", tomcat");
	      break;
	    }
	  break;

	case EM_68K:
	  if ((e_flags & EF_M68K_ARCH_MASK) == EF_M68K_M68000)
	    strcat (buf, ", m68000");
	  else if ((e_flags & EF_M68K_ARCH_MASK) == EF_M68K_CPU32)
	    strcat (buf, ", cpu32");
	  else if ((e_flags & EF_M68K_ARCH_MASK) == EF_M68K_FIDO)
	    strcat (buf, ", fido_a");
	  else
	    {
	      char const * isa = _("unknown");
	      char const * mac = _("unknown mac");
	      char const * additional = NULL;

	      switch (e_flags & EF_M68K_CF_ISA_MASK)
		{
		case EF_M68K_CF_ISA_A_NODIV:
		  isa = "A";
		  additional = ", nodiv";
		  break;
		case EF_M68K_CF_ISA_A:
		  isa = "A";
		  break;
		case EF_M68K_CF_ISA_A_PLUS:
		  isa = "A+";
		  break;
		case EF_M68K_CF_ISA_B_NOUSP:
		  isa = "B";
		  additional = ", nousp";
		  break;
		case EF_M68K_CF_ISA_B:
		  isa = "B";
		  break;
		case EF_M68K_CF_ISA_C:
		  isa = "C";
		  break;
		case EF_M68K_CF_ISA_C_NODIV:
		  isa = "C";
		  additional = ", nodiv";
		  break;
		}
	      strcat (buf, ", cf, isa ");
	      strcat (buf, isa);
	      if (additional)
		strcat (buf, additional);
	      if (e_flags & EF_M68K_CF_FLOAT)
		strcat (buf, ", float");
	      switch (e_flags & EF_M68K_CF_MAC_MASK)
		{
		case 0:
		  mac = NULL;
		  break;
		case EF_M68K_CF_MAC:
		  mac = "mac";
		  break;
		case EF_M68K_CF_EMAC:
		  mac = "emac";
		  break;
		case EF_M68K_CF_EMAC_B:
		  mac = "emac_b";
		  break;
		}
	      if (mac)
		{
		  strcat (buf, ", ");
		  strcat (buf, mac);
		}
	    }
	  break;

	case EM_PPC:
	  if (e_flags & EF_PPC_EMB)
	    strcat (buf, ", emb");

	  if (e_flags & EF_PPC_RELOCATABLE)
	    strcat (buf, _(", relocatable"));

	  if (e_flags & EF_PPC_RELOCATABLE_LIB)
	    strcat (buf, _(", relocatable-lib"));
	  break;

	case EM_V850:
	case EM_CYGNUS_V850:
	  switch (e_flags & EF_V850_ARCH)
	    {
	    case E_V850E2V3_ARCH:
	      strcat (buf, ", v850e2v3");
	      break;
	    case E_V850E2_ARCH:
	      strcat (buf, ", v850e2");
	      break;
            case E_V850E1_ARCH:
              strcat (buf, ", v850e1");
	      break;
	    case E_V850E_ARCH:
	      strcat (buf, ", v850e");
	      break;
	    case E_V850_ARCH:
	      strcat (buf, ", v850");
	      break;
	    default:
	      strcat (buf, _(", unknown v850 architecture variant"));
	      break;
	    }
	  break;

	case EM_M32R:
	case EM_CYGNUS_M32R:
	  if ((e_flags & EF_M32R_ARCH) == E_M32R_ARCH)
	    strcat (buf, ", m32r");
	  break;

	case EM_MIPS:
	case EM_MIPS_RS3_LE:
	  if (e_flags & EF_MIPS_NOREORDER)
	    strcat (buf, ", noreorder");

	  if (e_flags & EF_MIPS_PIC)
	    strcat (buf, ", pic");

	  if (e_flags & EF_MIPS_CPIC)
	    strcat (buf, ", cpic");

	  if (e_flags & EF_MIPS_UCODE)
	    strcat (buf, ", ugen_reserved");

	  if (e_flags & EF_MIPS_ABI2)
	    strcat (buf, ", abi2");

	  if (e_flags & EF_MIPS_OPTIONS_FIRST)
	    strcat (buf, ", odk first");

	  if (e_flags & EF_MIPS_32BITMODE)
	    strcat (buf, ", 32bitmode");

	  switch ((e_flags & EF_MIPS_MACH))
	    {
	    case E_MIPS_MACH_3900: strcat (buf, ", 3900"); break;
	    case E_MIPS_MACH_4010: strcat (buf, ", 4010"); break;
	    case E_MIPS_MACH_4100: strcat (buf, ", 4100"); break;
	    case E_MIPS_MACH_4111: strcat (buf, ", 4111"); break;
	    case E_MIPS_MACH_4120: strcat (buf, ", 4120"); break;
	    case E_MIPS_MACH_4650: strcat (buf, ", 4650"); break;
	    case E_MIPS_MACH_5400: strcat (buf, ", 5400"); break;
	    case E_MIPS_MACH_5500: strcat (buf, ", 5500"); break;
	    case E_MIPS_MACH_SB1:  strcat (buf, ", sb1");  break;
	    case E_MIPS_MACH_9000: strcat (buf, ", 9000"); break;
  	    case E_MIPS_MACH_LS2E: strcat (buf, ", loongson-2e"); break;
  	    case E_MIPS_MACH_LS2F: strcat (buf, ", loongson-2f"); break;
  	    case E_MIPS_MACH_LS3A: strcat (buf, ", loongson-3a"); break;
	    case E_MIPS_MACH_OCTEON: strcat (buf, ", octeon"); break;
	    case E_MIPS_MACH_OCTEON2: strcat (buf, ", octeon2"); break;
	    case E_MIPS_MACH_XLR:  strcat (buf, ", xlr"); break;
	    case 0:
	    /* We simply ignore the field in this case to avoid confusion:
	       MIPS ELF does not specify EF_MIPS_MACH, it is a GNU
	       extension.  */
	      break;
	    default: strcat (buf, _(", unknown CPU")); break;
	    }

	  switch ((e_flags & EF_MIPS_ABI))
	    {
	    case E_MIPS_ABI_O32: strcat (buf, ", o32"); break;
	    case E_MIPS_ABI_O64: strcat (buf, ", o64"); break;
	    case E_MIPS_ABI_EABI32: strcat (buf, ", eabi32"); break;
	    case E_MIPS_ABI_EABI64: strcat (buf, ", eabi64"); break;
	    case 0:
	    /* We simply ignore the field in this case to avoid confusion:
	       MIPS ELF does not specify EF_MIPS_ABI, it is a GNU extension.
	       This means it is likely to be an o32 file, but not for
	       sure.  */
	      break;
	    default: strcat (buf, _(", unknown ABI")); break;
	    }

	  if (e_flags & EF_MIPS_ARCH_ASE_MDMX)
	    strcat (buf, ", mdmx");

	  if (e_flags & EF_MIPS_ARCH_ASE_M16)
	    strcat (buf, ", mips16");

	  if (e_flags & EF_MIPS_ARCH_ASE_MICROMIPS)
	    strcat (buf, ", micromips");

	  switch ((e_flags & EF_MIPS_ARCH))
	    {
	    case E_MIPS_ARCH_1: strcat (buf, ", mips1"); break;
	    case E_MIPS_ARCH_2: strcat (buf, ", mips2"); break;
	    case E_MIPS_ARCH_3: strcat (buf, ", mips3"); break;
	    case E_MIPS_ARCH_4: strcat (buf, ", mips4"); break;
	    case E_MIPS_ARCH_5: strcat (buf, ", mips5"); break;
	    case E_MIPS_ARCH_32: strcat (buf, ", mips32"); break;
	    case E_MIPS_ARCH_32R2: strcat (buf, ", mips32r2"); break;
	    case E_MIPS_ARCH_64: strcat (buf, ", mips64"); break;
	    case E_MIPS_ARCH_64R2: strcat (buf, ", mips64r2"); break;
	    default: strcat (buf, _(", unknown ISA")); break;
	    }

	  if (e_flags & EF_SH_PIC)
	    strcat (buf, ", pic");

	  if (e_flags & EF_SH_FDPIC)
	    strcat (buf, ", fdpic");
	  break;

	case EM_SH:
	  switch ((e_flags & EF_SH_MACH_MASK))
	    {
	    case EF_SH1: strcat (buf, ", sh1"); break;
	    case EF_SH2: strcat (buf, ", sh2"); break;
	    case EF_SH3: strcat (buf, ", sh3"); break;
	    case EF_SH_DSP: strcat (buf, ", sh-dsp"); break;
	    case EF_SH3_DSP: strcat (buf, ", sh3-dsp"); break;
	    case EF_SH4AL_DSP: strcat (buf, ", sh4al-dsp"); break;
	    case EF_SH3E: strcat (buf, ", sh3e"); break;
	    case EF_SH4: strcat (buf, ", sh4"); break;
	    case EF_SH5: strcat (buf, ", sh5"); break;
	    case EF_SH2E: strcat (buf, ", sh2e"); break;
	    case EF_SH4A: strcat (buf, ", sh4a"); break;
	    case EF_SH2A: strcat (buf, ", sh2a"); break;
	    case EF_SH4_NOFPU: strcat (buf, ", sh4-nofpu"); break;
	    case EF_SH4A_NOFPU: strcat (buf, ", sh4a-nofpu"); break;
	    case EF_SH2A_NOFPU: strcat (buf, ", sh2a-nofpu"); break;
	    case EF_SH3_NOMMU: strcat (buf, ", sh3-nommu"); break;
	    case EF_SH4_NOMMU_NOFPU: strcat (buf, ", sh4-nommu-nofpu"); break;
	    case EF_SH2A_SH4_NOFPU: strcat (buf, ", sh2a-nofpu-or-sh4-nommu-nofpu"); break;
	    case EF_SH2A_SH3_NOFPU: strcat (buf, ", sh2a-nofpu-or-sh3-nommu"); break;
	    case EF_SH2A_SH4: strcat (buf, ", sh2a-or-sh4"); break;
	    case EF_SH2A_SH3E: strcat (buf, ", sh2a-or-sh3e"); break;
	    default: strcat (buf, _(", unknown ISA")); break;
	    }

	  break;

	case EM_SPARCV9:
	  if (e_flags & EF_SPARC_32PLUS)
	    strcat (buf, ", v8+");

	  if (e_flags & EF_SPARC_SUN_US1)
	    strcat (buf, ", ultrasparcI");

	  if (e_flags & EF_SPARC_SUN_US3)
	    strcat (buf, ", ultrasparcIII");

	  if (e_flags & EF_SPARC_HAL_R1)
	    strcat (buf, ", halr1");

	  if (e_flags & EF_SPARC_LEDATA)
	    strcat (buf, ", ledata");

	  if ((e_flags & EF_SPARCV9_MM) == EF_SPARCV9_TSO)
	    strcat (buf, ", tso");

	  if ((e_flags & EF_SPARCV9_MM) == EF_SPARCV9_PSO)
	    strcat (buf, ", pso");

	  if ((e_flags & EF_SPARCV9_MM) == EF_SPARCV9_RMO)
	    strcat (buf, ", rmo");
	  break;

	case EM_PARISC:
	  switch (e_flags & EF_PARISC_ARCH)
	    {
	    case EFA_PARISC_1_0:
	      strcpy (buf, ", PA-RISC 1.0");
	      break;
	    case EFA_PARISC_1_1:
	      strcpy (buf, ", PA-RISC 1.1");
	      break;
	    case EFA_PARISC_2_0:
	      strcpy (buf, ", PA-RISC 2.0");
	      break;
	    default:
	      break;
	    }
	  if (e_flags & EF_PARISC_TRAPNIL)
	    strcat (buf, ", trapnil");
	  if (e_flags & EF_PARISC_EXT)
	    strcat (buf, ", ext");
	  if (e_flags & EF_PARISC_LSB)
	    strcat (buf, ", lsb");
	  if (e_flags & EF_PARISC_WIDE)
	    strcat (buf, ", wide");
	  if (e_flags & EF_PARISC_NO_KABP)
	    strcat (buf, ", no kabp");
	  if (e_flags & EF_PARISC_LAZYSWAP)
	    strcat (buf, ", lazyswap");
	  break;

	case EM_PJ:
	case EM_PJ_OLD:
	  if ((e_flags & EF_PICOJAVA_NEWCALLS) == EF_PICOJAVA_NEWCALLS)
	    strcat (buf, ", new calling convention");

	  if ((e_flags & EF_PICOJAVA_GNUCALLS) == EF_PICOJAVA_GNUCALLS)
	    strcat (buf, ", gnu calling convention");
	  break;

	case EM_IA_64:
	  if ((e_flags & EF_IA_64_ABI64))
	    strcat (buf, ", 64-bit");
	  else
	    strcat (buf, ", 32-bit");
	  if ((e_flags & EF_IA_64_REDUCEDFP))
	    strcat (buf, ", reduced fp model");
	  if ((e_flags & EF_IA_64_NOFUNCDESC_CONS_GP))
	    strcat (buf, ", no function descriptors, constant gp");
	  else if ((e_flags & EF_IA_64_CONS_GP))
	    strcat (buf, ", constant gp");
	  if ((e_flags & EF_IA_64_ABSOLUTE))
	    strcat (buf, ", absolute");
          if (elf_header.e_ident[EI_OSABI] == ELFOSABI_OPENVMS)
            {
              if ((e_flags & EF_IA_64_VMS_LINKAGES))
                strcat (buf, ", vms_linkages");
              switch ((e_flags & EF_IA_64_VMS_COMCOD))
                {
                case EF_IA_64_VMS_COMCOD_SUCCESS:
                  break;
                case EF_IA_64_VMS_COMCOD_WARNING:
                  strcat (buf, ", warning");
                  break;
                case EF_IA_64_VMS_COMCOD_ERROR:
                  strcat (buf, ", error");
                  break;
                case EF_IA_64_VMS_COMCOD_ABORT:
                  strcat (buf, ", abort");
                  break;
                default:
                  abort ();
                }
            }
	  break;

	case EM_VAX:
	  if ((e_flags & EF_VAX_NONPIC))
	    strcat (buf, ", non-PIC");
	  if ((e_flags & EF_VAX_DFLOAT))
	    strcat (buf, ", D-Float");
	  if ((e_flags & EF_VAX_GFLOAT))
	    strcat (buf, ", G-Float");
	  break;

	case EM_RX:
	  if (e_flags & E_FLAG_RX_64BIT_DOUBLES)
	    strcat (buf, ", 64-bit doubles");
	  if (e_flags & E_FLAG_RX_DSP)
	    strcat (buf, ", dsp");
	  if (e_flags & E_FLAG_RX_PID)
	    strcat (buf, ", pid");	  
	  break;

	case EM_S390:
	  if (e_flags & EF_S390_HIGH_GPRS)
	    strcat (buf, ", highgprs");
	  break;

	case EM_TI_C6000:
	  if ((e_flags & EF_C6000_REL))
	    strcat (buf, ", relocatable module");
	  break;
	}
    }

  return buf;
}

static const char *
get_osabi_name (unsigned int osabi)
{
  static char buff[32];

  switch (osabi)
    {
    case ELFOSABI_NONE:		return "UNIX - System V";
    case ELFOSABI_HPUX:		return "UNIX - HP-UX";
    case ELFOSABI_NETBSD:	return "UNIX - NetBSD";
    case ELFOSABI_GNU:		return "UNIX - GNU";
    case ELFOSABI_SOLARIS:	return "UNIX - Solaris";
    case ELFOSABI_AIX:		return "UNIX - AIX";
    case ELFOSABI_IRIX:		return "UNIX - IRIX";
    case ELFOSABI_FREEBSD:	return "UNIX - FreeBSD";
    case ELFOSABI_TRU64:	return "UNIX - TRU64";
    case ELFOSABI_MODESTO:	return "Novell - Modesto";
    case ELFOSABI_OPENBSD:	return "UNIX - OpenBSD";
    case ELFOSABI_OPENVMS:	return "VMS - OpenVMS";
    case ELFOSABI_NSK:		return "HP - Non-Stop Kernel";
    case ELFOSABI_AROS:		return "AROS";
    case ELFOSABI_FENIXOS:	return "FenixOS";
    default:
      if (osabi >= 64)
	switch (elf_header.e_machine)
	  {
	  case EM_ARM:
	    switch (osabi)
	      {
	      case ELFOSABI_ARM:	return "ARM";
	      default:
		break;
	      }
	    break;

	  case EM_MSP430:
	  case EM_MSP430_OLD:
	    switch (osabi)
	      {
	      case ELFOSABI_STANDALONE:	return _("Standalone App");
	      default:
		break;
	      }
	    break;

	  case EM_TI_C6000:
	    switch (osabi)
	      {
	      case ELFOSABI_C6000_ELFABI:	return _("Bare-metal C6000");
	      case ELFOSABI_C6000_LINUX:	return "Linux C6000";
	      default:
		break;
	      }
	    break;

	  default:
	    break;
	  }
      snprintf (buff, sizeof (buff), _("<unknown: %x>"), osabi);
      return buff;
    }
}

static const char *
get_aarch64_segment_type (unsigned long type)
{
  switch (type)
    {
    case PT_AARCH64_ARCHEXT:
      return "AARCH64_ARCHEXT";
    default:
      break;
    }

  return NULL;
}

static const char *
get_arm_segment_type (unsigned long type)
{
  switch (type)
    {
    case PT_ARM_EXIDX:
      return "EXIDX";
    default:
      break;
    }

  return NULL;
}

static const char *
get_mips_segment_type (unsigned long type)
{
  switch (type)
    {
    case PT_MIPS_REGINFO:
      return "REGINFO";
    case PT_MIPS_RTPROC:
      return "RTPROC";
    case PT_MIPS_OPTIONS:
      return "OPTIONS";
    default:
      break;
    }

  return NULL;
}

static const char *
get_parisc_segment_type (unsigned long type)
{
  switch (type)
    {
    case PT_HP_TLS:		return "HP_TLS";
    case PT_HP_CORE_NONE:	return "HP_CORE_NONE";
    case PT_HP_CORE_VERSION:	return "HP_CORE_VERSION";
    case PT_HP_CORE_KERNEL:	return "HP_CORE_KERNEL";
    case PT_HP_CORE_COMM:	return "HP_CORE_COMM";
    case PT_HP_CORE_PROC:	return "HP_CORE_PROC";
    case PT_HP_CORE_LOADABLE:	return "HP_CORE_LOADABLE";
    case PT_HP_CORE_STACK:	return "HP_CORE_STACK";
    case PT_HP_CORE_SHM:	return "HP_CORE_SHM";
    case PT_HP_CORE_MMF:	return "HP_CORE_MMF";
    case PT_HP_PARALLEL:	return "HP_PARALLEL";
    case PT_HP_FASTBIND:	return "HP_FASTBIND";
    case PT_HP_OPT_ANNOT:	return "HP_OPT_ANNOT";
    case PT_HP_HSL_ANNOT:	return "HP_HSL_ANNOT";
    case PT_HP_STACK:		return "HP_STACK";
    case PT_HP_CORE_UTSNAME:	return "HP_CORE_UTSNAME";
    case PT_PARISC_ARCHEXT:	return "PARISC_ARCHEXT";
    case PT_PARISC_UNWIND:	return "PARISC_UNWIND";
    case PT_PARISC_WEAKORDER:	return "PARISC_WEAKORDER";
    default:
      break;
    }

  return NULL;
}

static const char *
get_ia64_segment_type (unsigned long type)
{
  switch (type)
    {
    case PT_IA_64_ARCHEXT:	return "IA_64_ARCHEXT";
    case PT_IA_64_UNWIND:	return "IA_64_UNWIND";
    case PT_HP_TLS:		return "HP_TLS";
    case PT_IA_64_HP_OPT_ANOT:	return "HP_OPT_ANNOT";
    case PT_IA_64_HP_HSL_ANOT:	return "HP_HSL_ANNOT";
    case PT_IA_64_HP_STACK:	return "HP_STACK";
    default:
      break;
    }

  return NULL;
}

static const char *
get_tic6x_segment_type (unsigned long type)
{
  switch (type)
    {
    case PT_C6000_PHATTR:	return "C6000_PHATTR";
    default:
      break;
    }

  return NULL;
}

static const char *
get_segment_type (unsigned long p_type)
{
  static char buff[32];

  switch (p_type)
    {
    case PT_NULL:	return "NULL";
    case PT_LOAD:	return "LOAD";
    case PT_DYNAMIC:	return "DYNAMIC";
    case PT_INTERP:	return "INTERP";
    case PT_NOTE:	return "NOTE";
    case PT_SHLIB:	return "SHLIB";
    case PT_PHDR:	return "PHDR";
    case PT_TLS:	return "TLS";

    case PT_GNU_EH_FRAME:
			return "GNU_EH_FRAME";
    case PT_GNU_STACK:	return "GNU_STACK";
    case PT_GNU_RELRO:  return "GNU_RELRO";

    default:
      if ((p_type >= PT_LOPROC) && (p_type <= PT_HIPROC))
	{
	  const char * result;

	  switch (elf_header.e_machine)
	    {
	    case EM_AARCH64:
	      result = get_aarch64_segment_type (p_type);
	      break;
	    case EM_ARM:
	      result = get_arm_segment_type (p_type);
	      break;
	    case EM_MIPS:
	    case EM_MIPS_RS3_LE:
	      result = get_mips_segment_type (p_type);
	      break;
	    case EM_PARISC:
	      result = get_parisc_segment_type (p_type);
	      break;
	    case EM_IA_64:
	      result = get_ia64_segment_type (p_type);
	      break;
	    case EM_TI_C6000:
	      result = get_tic6x_segment_type (p_type);
	      break;
	    default:
	      result = NULL;
	      break;
	    }

	  if (result != NULL)
	    return result;

	  sprintf (buff, "LOPROC+%lx", p_type - PT_LOPROC);
	}
      else if ((p_type >= PT_LOOS) && (p_type <= PT_HIOS))
	{
	  const char * result;

	  switch (elf_header.e_machine)
	    {
	    case EM_PARISC:
	      result = get_parisc_segment_type (p_type);
	      break;
	    case EM_IA_64:
	      result = get_ia64_segment_type (p_type);
	      break;
	    default:
	      result = NULL;
	      break;
	    }

	  if (result != NULL)
	    return result;

	  sprintf (buff, "LOOS+%lx", p_type - PT_LOOS);
	}
      else
	snprintf (buff, sizeof (buff), _("<unknown>: %lx"), p_type);

      return buff;
    }
}

static const char *
get_mips_section_type_name (unsigned int sh_type)
{
  switch (sh_type)
    {
    case SHT_MIPS_LIBLIST:	 return "MIPS_LIBLIST";
    case SHT_MIPS_MSYM:		 return "MIPS_MSYM";
    case SHT_MIPS_CONFLICT:	 return "MIPS_CONFLICT";
    case SHT_MIPS_GPTAB:	 return "MIPS_GPTAB";
    case SHT_MIPS_UCODE:	 return "MIPS_UCODE";
    case SHT_MIPS_DEBUG:	 return "MIPS_DEBUG";
    case SHT_MIPS_REGINFO:	 return "MIPS_REGINFO";
    case SHT_MIPS_PACKAGE:	 return "MIPS_PACKAGE";
    case SHT_MIPS_PACKSYM:	 return "MIPS_PACKSYM";
    case SHT_MIPS_RELD:		 return "MIPS_RELD";
    case SHT_MIPS_IFACE:	 return "MIPS_IFACE";
    case SHT_MIPS_CONTENT:	 return "MIPS_CONTENT";
    case SHT_MIPS_OPTIONS:	 return "MIPS_OPTIONS";
    case SHT_MIPS_SHDR:		 return "MIPS_SHDR";
    case SHT_MIPS_FDESC:	 return "MIPS_FDESC";
    case SHT_MIPS_EXTSYM:	 return "MIPS_EXTSYM";
    case SHT_MIPS_DENSE:	 return "MIPS_DENSE";
    case SHT_MIPS_PDESC:	 return "MIPS_PDESC";
    case SHT_MIPS_LOCSYM:	 return "MIPS_LOCSYM";
    case SHT_MIPS_AUXSYM:	 return "MIPS_AUXSYM";
    case SHT_MIPS_OPTSYM:	 return "MIPS_OPTSYM";
    case SHT_MIPS_LOCSTR:	 return "MIPS_LOCSTR";
    case SHT_MIPS_LINE:		 return "MIPS_LINE";
    case SHT_MIPS_RFDESC:	 return "MIPS_RFDESC";
    case SHT_MIPS_DELTASYM:	 return "MIPS_DELTASYM";
    case SHT_MIPS_DELTAINST:	 return "MIPS_DELTAINST";
    case SHT_MIPS_DELTACLASS:	 return "MIPS_DELTACLASS";
    case SHT_MIPS_DWARF:	 return "MIPS_DWARF";
    case SHT_MIPS_DELTADECL:	 return "MIPS_DELTADECL";
    case SHT_MIPS_SYMBOL_LIB:	 return "MIPS_SYMBOL_LIB";
    case SHT_MIPS_EVENTS:	 return "MIPS_EVENTS";
    case SHT_MIPS_TRANSLATE:	 return "MIPS_TRANSLATE";
    case SHT_MIPS_PIXIE:	 return "MIPS_PIXIE";
    case SHT_MIPS_XLATE:	 return "MIPS_XLATE";
    case SHT_MIPS_XLATE_DEBUG:	 return "MIPS_XLATE_DEBUG";
    case SHT_MIPS_WHIRL:	 return "MIPS_WHIRL";
    case SHT_MIPS_EH_REGION:	 return "MIPS_EH_REGION";
    case SHT_MIPS_XLATE_OLD:	 return "MIPS_XLATE_OLD";
    case SHT_MIPS_PDR_EXCEPTION: return "MIPS_PDR_EXCEPTION";
    default:
      break;
    }
  return NULL;
}

static const char *
get_parisc_section_type_name (unsigned int sh_type)
{
  switch (sh_type)
    {
    case SHT_PARISC_EXT:	return "PARISC_EXT";
    case SHT_PARISC_UNWIND:	return "PARISC_UNWIND";
    case SHT_PARISC_DOC:	return "PARISC_DOC";
    case SHT_PARISC_ANNOT:	return "PARISC_ANNOT";
    case SHT_PARISC_SYMEXTN:	return "PARISC_SYMEXTN";
    case SHT_PARISC_STUBS:	return "PARISC_STUBS";
    case SHT_PARISC_DLKM:	return "PARISC_DLKM";
    default:
      break;
    }
  return NULL;
}

static const char *
get_ia64_section_type_name (unsigned int sh_type)
{
  /* If the top 8 bits are 0x78 the next 8 are the os/abi ID.  */
  if ((sh_type & 0xFF000000) == SHT_IA_64_LOPSREG)
    return get_osabi_name ((sh_type & 0x00FF0000) >> 16);

  switch (sh_type)
    {
    case SHT_IA_64_EXT:		       return "IA_64_EXT";
    case SHT_IA_64_UNWIND:	       return "IA_64_UNWIND";
    case SHT_IA_64_PRIORITY_INIT:      return "IA_64_PRIORITY_INIT";
    case SHT_IA_64_VMS_TRACE:          return "VMS_TRACE";
    case SHT_IA_64_VMS_TIE_SIGNATURES: return "VMS_TIE_SIGNATURES";
    case SHT_IA_64_VMS_DEBUG:          return "VMS_DEBUG";
    case SHT_IA_64_VMS_DEBUG_STR:      return "VMS_DEBUG_STR";
    case SHT_IA_64_VMS_LINKAGES:       return "VMS_LINKAGES";
    case SHT_IA_64_VMS_SYMBOL_VECTOR:  return "VMS_SYMBOL_VECTOR";
    case SHT_IA_64_VMS_FIXUP:          return "VMS_FIXUP";
    default:
      break;
    }
  return NULL;
}

static const char *
get_x86_64_section_type_name (unsigned int sh_type)
{
  switch (sh_type)
    {
    case SHT_X86_64_UNWIND:	return "X86_64_UNWIND";
    default:
      break;
    }
  return NULL;
}

static const char *
get_aarch64_section_type_name (unsigned int sh_type)
{
  switch (sh_type)
    {
    case SHT_AARCH64_ATTRIBUTES:
      return "AARCH64_ATTRIBUTES";
    default:
      break;
    }
  return NULL;
}

static const char *
get_arm_section_type_name (unsigned int sh_type)
{
  switch (sh_type)
    {
    case SHT_ARM_EXIDX:           return "ARM_EXIDX";
    case SHT_ARM_PREEMPTMAP:      return "ARM_PREEMPTMAP";
    case SHT_ARM_ATTRIBUTES:      return "ARM_ATTRIBUTES";
    case SHT_ARM_DEBUGOVERLAY:    return "ARM_DEBUGOVERLAY";
    case SHT_ARM_OVERLAYSECTION:  return "ARM_OVERLAYSECTION";
    default:
      break;
    }
  return NULL;
}

static const char *
get_tic6x_section_type_name (unsigned int sh_type)
{
  switch (sh_type)
    {
    case SHT_C6000_UNWIND:
      return "C6000_UNWIND";
    case SHT_C6000_PREEMPTMAP:
      return "C6000_PREEMPTMAP";
    case SHT_C6000_ATTRIBUTES:
      return "C6000_ATTRIBUTES";
    case SHT_TI_ICODE:
      return "TI_ICODE";
    case SHT_TI_XREF:
      return "TI_XREF";
    case SHT_TI_HANDLER:
      return "TI_HANDLER";
    case SHT_TI_INITINFO:
      return "TI_INITINFO";
    case SHT_TI_PHATTRS:
      return "TI_PHATTRS";
    default:
      break;
    }
  return NULL;
}

static const char *
get_section_type_name (unsigned int sh_type)
{
  static char buff[32];

  switch (sh_type)
    {
    case SHT_NULL:		return "NULL";
    case SHT_PROGBITS:		return "PROGBITS";
    case SHT_SYMTAB:		return "SYMTAB";
    case SHT_STRTAB:		return "STRTAB";
    case SHT_RELA:		return "RELA";
    case SHT_HASH:		return "HASH";
    case SHT_DYNAMIC:		return "DYNAMIC";
    case SHT_NOTE:		return "NOTE";
    case SHT_NOBITS:		return "NOBITS";
    case SHT_REL:		return "REL";
    case SHT_SHLIB:		return "SHLIB";
    case SHT_DYNSYM:		return "DYNSYM";
    case SHT_INIT_ARRAY:	return "INIT_ARRAY";
    case SHT_FINI_ARRAY:	return "FINI_ARRAY";
    case SHT_PREINIT_ARRAY:	return "PREINIT_ARRAY";
    case SHT_GNU_HASH:		return "GNU_HASH";
    case SHT_GROUP:		return "GROUP";
    case SHT_SYMTAB_SHNDX:	return "SYMTAB SECTION INDICIES";
    case SHT_GNU_verdef:	return "VERDEF";
    case SHT_GNU_verneed:	return "VERNEED";
    case SHT_GNU_versym:	return "VERSYM";
    case 0x6ffffff0:		return "VERSYM";
    case 0x6ffffffc:		return "VERDEF";
    case 0x7ffffffd:		return "AUXILIARY";
    case 0x7fffffff:		return "FILTER";
    case SHT_GNU_LIBLIST:	return "GNU_LIBLIST";

    default:
      if ((sh_type >= SHT_LOPROC) && (sh_type <= SHT_HIPROC))
	{
	  const char * result;

	  switch (elf_header.e_machine)
	    {
	    case EM_MIPS:
	    case EM_MIPS_RS3_LE:
	      result = get_mips_section_type_name (sh_type);
	      break;
	    case EM_PARISC:
	      result = get_parisc_section_type_name (sh_type);
	      break;
	    case EM_IA_64:
	      result = get_ia64_section_type_name (sh_type);
	      break;
	    case EM_X86_64:
	    case EM_L1OM:
	    case EM_K1OM:
	      result = get_x86_64_section_type_name (sh_type);
	      break;
	    case EM_AARCH64:
	      result = get_aarch64_section_type_name (sh_type);
	      break;
	    case EM_ARM:
	      result = get_arm_section_type_name (sh_type);
	      break;
	    case EM_TI_C6000:
	      result = get_tic6x_section_type_name (sh_type);
	      break;
	    default:
	      result = NULL;
	      break;
	    }

	  if (result != NULL)
	    return result;

	  sprintf (buff, "LOPROC+%x", sh_type - SHT_LOPROC);
	}
      else if ((sh_type >= SHT_LOOS) && (sh_type <= SHT_HIOS))
	{
	  const char * result;

	  switch (elf_header.e_machine)
	    {
	    case EM_IA_64:
	      result = get_ia64_section_type_name (sh_type);
	      break;
	    default:
	      result = NULL;
	      break;
	    }

	  if (result != NULL)
	    return result;

	  sprintf (buff, "LOOS+%x", sh_type - SHT_LOOS);
	}
      else if ((sh_type >= SHT_LOUSER) && (sh_type <= SHT_HIUSER))
	sprintf (buff, "LOUSER+%x", sh_type - SHT_LOUSER);
      else
	/* This message is probably going to be displayed in a 15
	   character wide field, so put the hex value first.  */
	snprintf (buff, sizeof (buff), _("%08x: <unknown>"), sh_type);

      return buff;
    }
}

#define OPTION_DEBUG_DUMP	512
#define OPTION_DYN_SYMS		513
#define OPTION_DWARF_DEPTH	514
#define OPTION_DWARF_START	515
#define OPTION_DWARF_CHECK	516

static struct option options[] =
{
  {"all",	       no_argument, 0, 'a'},
  {"file-header",      no_argument, 0, 'h'},
  {"program-headers",  no_argument, 0, 'l'},
  {"headers",	       no_argument, 0, 'e'},
  {"histogram",	       no_argument, 0, 'I'},
  {"segments",	       no_argument, 0, 'l'},
  {"sections",	       no_argument, 0, 'S'},
  {"section-headers",  no_argument, 0, 'S'},
  {"section-groups",   no_argument, 0, 'g'},
  {"section-details",  no_argument, 0, 't'},
  {"full-section-name",no_argument, 0, 'N'},
  {"symbols",	       no_argument, 0, 's'},
  {"syms",	       no_argument, 0, 's'},
  {"dyn-syms",	       no_argument, 0, OPTION_DYN_SYMS},
  {"relocs",	       no_argument, 0, 'r'},
  {"notes",	       no_argument, 0, 'n'},
  {"dynamic",	       no_argument, 0, 'd'},
  {"arch-specific",    no_argument, 0, 'A'},
  {"version-info",     no_argument, 0, 'V'},
  {"use-dynamic",      no_argument, 0, 'D'},
  {"unwind",	       no_argument, 0, 'u'},
  {"archive-index",    no_argument, 0, 'c'},
  {"hex-dump",	       required_argument, 0, 'x'},
  {"relocated-dump",   required_argument, 0, 'R'},
  {"string-dump",      required_argument, 0, 'p'},
#ifdef SUPPORT_DISASSEMBLY
  {"instruction-dump", required_argument, 0, 'i'},
#endif
  {"debug-dump",       optional_argument, 0, OPTION_DEBUG_DUMP},

  {"dwarf-depth",      required_argument, 0, OPTION_DWARF_DEPTH},
  {"dwarf-start",      required_argument, 0, OPTION_DWARF_START},
  {"dwarf-check",      no_argument, 0, OPTION_DWARF_CHECK},

  {"version",	       no_argument, 0, 'v'},
  {"wide",	       no_argument, 0, 'W'},
  {"help",	       no_argument, 0, 'H'},
  {0,		       no_argument, 0, 0}
};

static void
usage (FILE * stream)
{
  fprintf (stream, _("Usage: readelf <option(s)> elf-file(s)\n"));
  fprintf (stream, _(" Display information about the contents of ELF format files\n"));
  fprintf (stream, _(" Options are:\n\
  -a --all               Equivalent to: -h -l -S -s -r -d -V -A -I\n\
  -h --file-header       Display the ELF file header\n\
  -l --program-headers   Display the program headers\n\
     --segments          An alias for --program-headers\n\
  -S --section-headers   Display the sections' header\n\
     --sections          An alias for --section-headers\n\
  -g --section-groups    Display the section groups\n\
  -t --section-details   Display the section details\n\
  -e --headers           Equivalent to: -h -l -S\n\
  -s --syms              Display the symbol table\n\
     --symbols           An alias for --syms\n\
  --dyn-syms             Display the dynamic symbol table\n\
  -n --notes             Display the core notes (if present)\n\
  -r --relocs            Display the relocations (if present)\n\
  -u --unwind            Display the unwind info (if present)\n\
  -d --dynamic           Display the dynamic section (if present)\n\
  -V --version-info      Display the version sections (if present)\n\
  -A --arch-specific     Display architecture specific information (if any)\n\
  -c --archive-index     Display the symbol/file index in an archive\n\
  -D --use-dynamic       Use the dynamic section info when displaying symbols\n\
  -x --hex-dump=<number|name>\n\
                         Dump the contents of section <number|name> as bytes\n\
  -p --string-dump=<number|name>\n\
                         Dump the contents of section <number|name> as strings\n\
  -R --relocated-dump=<number|name>\n\
                         Dump the contents of section <number|name> as relocated bytes\n\
  -w[lLiaprmfFsoRt] or\n\
  --debug-dump[=rawline,=decodedline,=info,=abbrev,=pubnames,=aranges,=macro,=frames,\n\
               =frames-interp,=str,=loc,=Ranges,=pubtypes,\n\
               =gdb_index,=trace_info,=trace_abbrev,=trace_aranges]\n\
                         Display the contents of DWARF2 debug sections\n"));
  fprintf (stream, _("\
  --dwarf-depth=N        Do not display DIEs at depth N or greater\n\
  --dwarf-start=N        Display DIEs starting with N, at the same depth\n\
                         or deeper\n"));
#ifdef SUPPORT_DISASSEMBLY
  fprintf (stream, _("\
  -i --instruction-dump=<number|name>\n\
                         Disassemble the contents of section <number|name>\n"));
#endif
  fprintf (stream, _("\
  -I --histogram         Display histogram of bucket list lengths\n\
  -W --wide              Allow output width to exceed 80 characters\n\
  @<file>                Read options from <file>\n\
  -H --help              Display this information\n\
  -v --version           Display the version number of readelf\n"));

  if (REPORT_BUGS_TO[0] && stream == stdout)
    fprintf (stdout, _("Report bugs to %s\n"), REPORT_BUGS_TO);

  exit (stream == stdout ? 0 : 1);
}

/* Record the fact that the user wants the contents of section number
   SECTION to be displayed using the method(s) encoded as flags bits
   in TYPE.  Note, TYPE can be zero if we are creating the array for
   the first time.  */

static void
request_dump_bynumber (unsigned int section, dump_type type)
{
  if (section >= num_dump_sects)
    {
      dump_type * new_dump_sects;

      new_dump_sects = (dump_type *) calloc (section + 1,
                                             sizeof (* dump_sects));

      if (new_dump_sects == NULL)
	error (_("Out of memory allocating dump request table.\n"));
      else
	{
	  /* Copy current flag settings.  */
	  memcpy (new_dump_sects, dump_sects, num_dump_sects * sizeof (* dump_sects));

	  free (dump_sects);

	  dump_sects = new_dump_sects;
	  num_dump_sects = section + 1;
	}
    }

  if (dump_sects)
    dump_sects[section] |= type;

  return;
}

/* Request a dump by section name.  */

static void
request_dump_byname (const char * section, dump_type type)
{
  struct dump_list_entry * new_request;

  new_request = (struct dump_list_entry *)
      malloc (sizeof (struct dump_list_entry));
  if (!new_request)
    error (_("Out of memory allocating dump request table.\n"));

  new_request->name = strdup (section);
  if (!new_request->name)
    error (_("Out of memory allocating dump request table.\n"));

  new_request->type = type;

  new_request->next = dump_sects_byname;
  dump_sects_byname = new_request;
}

static inline void
request_dump (dump_type type)
{
  int section;
  char * cp;

  do_dump++;
  section = strtoul (optarg, & cp, 0);

  if (! *cp && section >= 0)
    request_dump_bynumber (section, type);
  else
    request_dump_byname (optarg, type);
}


static void
parse_args (int argc, char ** argv)
{
  int c;

  if (argc < 2)
    usage (stderr);

  while ((c = getopt_long
	  (argc, argv, "ADHINR:SVWacdeghi:lnp:rstuvw::x:", options, NULL)) != EOF)
    {
      switch (c)
	{
	case 0:
	  /* Long options.  */
	  break;
	case 'H':
	  usage (stdout);
	  break;

	case 'a':
	  do_syms++;
	  do_reloc++;
	  do_unwind++;
	  do_dynamic++;
	  do_header++;
	  do_sections++;
	  do_section_groups++;
	  do_segments++;
	  do_version++;
	  do_histogram++;
	  do_arch++;
	  do_notes++;
	  break;
	case 'g':
	  do_section_groups++;
	  break;
	case 't':
	case 'N':
	  do_sections++;
	  do_section_details++;
	  break;
	case 'e':
	  do_header++;
	  do_sections++;
	  do_segments++;
	  break;
	case 'A':
	  do_arch++;
	  break;
	case 'D':
	  do_using_dynamic++;
	  break;
	case 'r':
	  do_reloc++;
	  break;
	case 'u':
	  do_unwind++;
	  break;
	case 'h':
	  do_header++;
	  break;
	case 'l':
	  do_segments++;
	  break;
	case 's':
	  do_syms++;
	  break;
	case 'S':
	  do_sections++;
	  break;
	case 'd':
	  do_dynamic++;
	  break;
	case 'I':
	  do_histogram++;
	  break;
	case 'n':
	  do_notes++;
	  break;
	case 'c':
	  do_archive_index++;
	  break;
	case 'x':
	  request_dump (HEX_DUMP);
	  break;
	case 'p':
	  request_dump (STRING_DUMP);
	  break;
	case 'R':
	  request_dump (RELOC_DUMP);
	  break;
	case 'w':
	  do_dump++;
	  if (optarg == 0)
	    {
	      do_debugging = 1;
	      dwarf_select_sections_all ();
	    }
	  else
	    {
	      do_debugging = 0;
	      dwarf_select_sections_by_letters (optarg);
	    }
	  break;
	case OPTION_DEBUG_DUMP:
	  do_dump++;
	  if (optarg == 0)
	    do_debugging = 1;
	  else
	    {
	      do_debugging = 0;
	      dwarf_select_sections_by_names (optarg);
	    }
	  break;
	case OPTION_DWARF_DEPTH:
	  {
	    char *cp;

	    dwarf_cutoff_level = strtoul (optarg, & cp, 0);
	  }
	  break;
	case OPTION_DWARF_START:
	  {
	    char *cp;

	    dwarf_start_die = strtoul (optarg, & cp, 0);
	  }
	  break;
	case OPTION_DWARF_CHECK:
	  dwarf_check = 1;
	  break;
	case OPTION_DYN_SYMS:
	  do_dyn_syms++;
	  break;
#ifdef SUPPORT_DISASSEMBLY
	case 'i':
	  request_dump (DISASS_DUMP);
	  break;
#endif
	case 'v':
	  print_version (program_name);
	  break;
	case 'V':
	  do_version++;
	  break;
	case 'W':
	  do_wide++;
	  break;
	default:
	  /* xgettext:c-format */
	  error (_("Invalid option '-%c'\n"), c);
	  /* Drop through.  */
	case '?':
	  usage (stderr);
	}
    }

  if (!do_dynamic && !do_syms && !do_reloc && !do_unwind && !do_sections
      && !do_segments && !do_header && !do_dump && !do_version
      && !do_histogram && !do_debugging && !do_arch && !do_notes
      && !do_section_groups && !do_archive_index
      && !do_dyn_syms)
    usage (stderr);
  else if (argc < 3)
    {
      warn (_("Nothing to do.\n"));
      usage (stderr);
    }
}

static const char *
get_elf_class (unsigned int elf_class)
{
  static char buff[32];

  switch (elf_class)
    {
    case ELFCLASSNONE: return _("none");
    case ELFCLASS32:   return "ELF32";
    case ELFCLASS64:   return "ELF64";
    default:
      snprintf (buff, sizeof (buff), _("<unknown: %x>"), elf_class);
      return buff;
    }
}

static const char *
get_data_encoding (unsigned int encoding)
{
  static char buff[32];

  switch (encoding)
    {
    case ELFDATANONE: return _("none");
    case ELFDATA2LSB: return _("2's complement, little endian");
    case ELFDATA2MSB: return _("2's complement, big endian");
    default:
      snprintf (buff, sizeof (buff), _("<unknown: %x>"), encoding);
      return buff;
    }
}

/* Decode the data held in 'elf_header'.  */

static int
process_file_header (void)
{
  if (   elf_header.e_ident[EI_MAG0] != ELFMAG0
      || elf_header.e_ident[EI_MAG1] != ELFMAG1
      || elf_header.e_ident[EI_MAG2] != ELFMAG2
      || elf_header.e_ident[EI_MAG3] != ELFMAG3)
    {
      error
	(_("Not an ELF file - it has the wrong magic bytes at the start\n"));
      return 0;
    }

  init_dwarf_regnames (elf_header.e_machine);

  if (do_header)
    {
      int i;

      printf (_("ELF Header:\n"));
      printf (_("  Magic:   "));
      for (i = 0; i < EI_NIDENT; i++)
	printf ("%2.2x ", elf_header.e_ident[i]);
      printf ("\n");
      printf (_("  Class:                             %s\n"),
	      get_elf_class (elf_header.e_ident[EI_CLASS]));
      printf (_("  Data:                              %s\n"),
	      get_data_encoding (elf_header.e_ident[EI_DATA]));
      printf (_("  Version:                           %d %s\n"),
	      elf_header.e_ident[EI_VERSION],
	      (elf_header.e_ident[EI_VERSION] == EV_CURRENT
	       ? "(current)"
	       : (elf_header.e_ident[EI_VERSION] != EV_NONE
		  ? _("<unknown: %lx>")
		  : "")));
      printf (_("  OS/ABI:                            %s\n"),
	      get_osabi_name (elf_header.e_ident[EI_OSABI]));
      printf (_("  ABI Version:                       %d\n"),
	      elf_header.e_ident[EI_ABIVERSION]);
      printf (_("  Type:                              %s\n"),
	      get_file_type (elf_header.e_type));
      printf (_("  Machine:                           %s\n"),
	      get_machine_name (elf_header.e_machine));
      printf (_("  Version:                           0x%lx\n"),
	      (unsigned long) elf_header.e_version);

      printf (_("  Entry point address:               "));
      print_vma ((bfd_vma) elf_header.e_entry, PREFIX_HEX);
      printf (_("\n  Start of program headers:          "));
      print_vma ((bfd_vma) elf_header.e_phoff, DEC);
      printf (_(" (bytes into file)\n  Start of section headers:          "));
      print_vma ((bfd_vma) elf_header.e_shoff, DEC);
      printf (_(" (bytes into file)\n"));

      printf (_("  Flags:                             0x%lx%s\n"),
	      (unsigned long) elf_header.e_flags,
	      get_machine_flags (elf_header.e_flags, elf_header.e_machine));
      printf (_("  Size of this header:               %ld (bytes)\n"),
	      (long) elf_header.e_ehsize);
      printf (_("  Size of program headers:           %ld (bytes)\n"),
	      (long) elf_header.e_phentsize);
      printf (_("  Number of program headers:         %ld"),
	      (long) elf_header.e_phnum);
      if (section_headers != NULL
	  && elf_header.e_phnum == PN_XNUM
	  && section_headers[0].sh_info != 0)
	printf (" (%ld)", (long) section_headers[0].sh_info);
      putc ('\n', stdout);
      printf (_("  Size of section headers:           %ld (bytes)\n"),
	      (long) elf_header.e_shentsize);
      printf (_("  Number of section headers:         %ld"),
	      (long) elf_header.e_shnum);
      if (section_headers != NULL && elf_header.e_shnum == SHN_UNDEF)
	printf (" (%ld)", (long) section_headers[0].sh_size);
      putc ('\n', stdout);
      printf (_("  Section header string table index: %ld"),
	      (long) elf_header.e_shstrndx);
      if (section_headers != NULL
	  && elf_header.e_shstrndx == (SHN_XINDEX & 0xffff))
	printf (" (%u)", section_headers[0].sh_link);
      else if (elf_header.e_shstrndx != SHN_UNDEF
	       && elf_header.e_shstrndx >= elf_header.e_shnum)
	printf (_(" <corrupt: out of range>"));
      putc ('\n', stdout);
    }

  if (section_headers != NULL)
    {
      if (elf_header.e_phnum == PN_XNUM
	  && section_headers[0].sh_info != 0)
	elf_header.e_phnum = section_headers[0].sh_info;
      if (elf_header.e_shnum == SHN_UNDEF)
	elf_header.e_shnum = section_headers[0].sh_size;
      if (elf_header.e_shstrndx == (SHN_XINDEX & 0xffff))
	elf_header.e_shstrndx = section_headers[0].sh_link;
      else if (elf_header.e_shstrndx >= elf_header.e_shnum)
	elf_header.e_shstrndx = SHN_UNDEF;
      free (section_headers);
      section_headers = NULL;
    }

  return 1;
}


static int
get_32bit_program_headers (FILE * file, Elf_Internal_Phdr * pheaders)
{
  Elf32_External_Phdr * phdrs;
  Elf32_External_Phdr * external;
  Elf_Internal_Phdr *   internal;
  unsigned int i;

  phdrs = (Elf32_External_Phdr *) get_data (NULL, file, elf_header.e_phoff,
                                            elf_header.e_phentsize,
                                            elf_header.e_phnum,
                                            _("program headers"));
  if (!phdrs)
    return 0;

  for (i = 0, internal = pheaders, external = phdrs;
       i < elf_header.e_phnum;
       i++, internal++, external++)
    {
      internal->p_type   = BYTE_GET (external->p_type);
      internal->p_offset = BYTE_GET (external->p_offset);
      internal->p_vaddr  = BYTE_GET (external->p_vaddr);
      internal->p_paddr  = BYTE_GET (external->p_paddr);
      internal->p_filesz = BYTE_GET (external->p_filesz);
      internal->p_memsz  = BYTE_GET (external->p_memsz);
      internal->p_flags  = BYTE_GET (external->p_flags);
      internal->p_align  = BYTE_GET (external->p_align);
    }

  free (phdrs);

  return 1;
}

static int
get_64bit_program_headers (FILE * file, Elf_Internal_Phdr * pheaders)
{
  Elf64_External_Phdr * phdrs;
  Elf64_External_Phdr * external;
  Elf_Internal_Phdr *   internal;
  unsigned int i;

  phdrs = (Elf64_External_Phdr *) get_data (NULL, file, elf_header.e_phoff,
                                            elf_header.e_phentsize,
                                            elf_header.e_phnum,
                                            _("program headers"));
  if (!phdrs)
    return 0;

  for (i = 0, internal = pheaders, external = phdrs;
       i < elf_header.e_phnum;
       i++, internal++, external++)
    {
      internal->p_type   = BYTE_GET (external->p_type);
      internal->p_flags  = BYTE_GET (external->p_flags);
      internal->p_offset = BYTE_GET (external->p_offset);
      internal->p_vaddr  = BYTE_GET (external->p_vaddr);
      internal->p_paddr  = BYTE_GET (external->p_paddr);
      internal->p_filesz = BYTE_GET (external->p_filesz);
      internal->p_memsz  = BYTE_GET (external->p_memsz);
      internal->p_align  = BYTE_GET (external->p_align);
    }

  free (phdrs);

  return 1;
}

/* Returns 1 if the program headers were read into `program_headers'.  */

static int
get_program_headers (FILE * file)
{
  Elf_Internal_Phdr * phdrs;

  /* Check cache of prior read.  */
  if (program_headers != NULL)
    return 1;

  phdrs = (Elf_Internal_Phdr *) cmalloc (elf_header.e_phnum,
                                         sizeof (Elf_Internal_Phdr));

  if (phdrs == NULL)
    {
      error (_("Out of memory\n"));
      return 0;
    }

  if (is_32bit_elf
      ? get_32bit_program_headers (file, phdrs)
      : get_64bit_program_headers (file, phdrs))
    {
      program_headers = phdrs;
      return 1;
    }

  free (phdrs);
  return 0;
}

/* Returns 1 if the program headers were loaded.  */

static int
process_program_headers (FILE * file)
{
  Elf_Internal_Phdr * segment;
  unsigned int i;

  if (elf_header.e_phnum == 0)
    {
      /* PR binutils/12467.  */
      if (elf_header.e_phoff != 0)
	warn (_("possibly corrupt ELF header - it has a non-zero program"
		" header offset, but no program headers"));
      else if (do_segments)
	printf (_("\nThere are no program headers in this file.\n"));
      return 0;
    }

  if (do_segments && !do_header)
    {
      printf (_("\nElf file type is %s\n"), get_file_type (elf_header.e_type));
      printf (_("Entry point "));
      print_vma ((bfd_vma) elf_header.e_entry, PREFIX_HEX);
      printf (_("\nThere are %d program headers, starting at offset "),
	      elf_header.e_phnum);
      print_vma ((bfd_vma) elf_header.e_phoff, DEC);
      printf ("\n");
    }

  if (! get_program_headers (file))
      return 0;

  if (do_segments)
    {
      if (elf_header.e_phnum > 1)
	printf (_("\nProgram Headers:\n"));
      else
	printf (_("\nProgram Headers:\n"));

      if (is_32bit_elf)
	printf
	  (_("  Type           Offset   VirtAddr   PhysAddr   FileSiz MemSiz  Flg Align\n"));
      else if (do_wide)
	printf
	  (_("  Type           Offset   VirtAddr           PhysAddr           FileSiz  MemSiz   Flg Align\n"));
      else
	{
	  printf
	    (_("  Type           Offset             VirtAddr           PhysAddr\n"));
	  printf
	    (_("                 FileSiz            MemSiz              Flags  Align\n"));
	}
    }

  dynamic_addr = 0;
  dynamic_size = 0;

  for (i = 0, segment = program_headers;
       i < elf_header.e_phnum;
       i++, segment++)
    {
      if (do_segments)
	{
	  printf ("  %-14.14s ", get_segment_type (segment->p_type));

	  if (is_32bit_elf)
	    {
	      printf ("0x%6.6lx ", (unsigned long) segment->p_offset);
	      printf ("0x%8.8lx ", (unsigned long) segment->p_vaddr);
	      printf ("0x%8.8lx ", (unsigned long) segment->p_paddr);
	      printf ("0x%5.5lx ", (unsigned long) segment->p_filesz);
	      printf ("0x%5.5lx ", (unsigned long) segment->p_memsz);
	      printf ("%c%c%c ",
		      (segment->p_flags & PF_R ? 'R' : ' '),
		      (segment->p_flags & PF_W ? 'W' : ' '),
		      (segment->p_flags & PF_X ? 'E' : ' '));
	      printf ("%#lx", (unsigned long) segment->p_align);
	    }
	  else if (do_wide)
	    {
	      if ((unsigned long) segment->p_offset == segment->p_offset)
		printf ("0x%6.6lx ", (unsigned long) segment->p_offset);
	      else
		{
		  print_vma (segment->p_offset, FULL_HEX);
		  putchar (' ');
		}

	      print_vma (segment->p_vaddr, FULL_HEX);
	      putchar (' ');
	      print_vma (segment->p_paddr, FULL_HEX);
	      putchar (' ');

	      if ((unsigned long) segment->p_filesz == segment->p_filesz)
		printf ("0x%6.6lx ", (unsigned long) segment->p_filesz);
	      else
		{
		  print_vma (segment->p_filesz, FULL_HEX);
		  putchar (' ');
		}

	      if ((unsigned long) segment->p_memsz == segment->p_memsz)
		printf ("0x%6.6lx", (unsigned long) segment->p_memsz);
	      else
		{
		  print_vma (segment->p_memsz, FULL_HEX);
		}

	      printf (" %c%c%c ",
		      (segment->p_flags & PF_R ? 'R' : ' '),
		      (segment->p_flags & PF_W ? 'W' : ' '),
		      (segment->p_flags & PF_X ? 'E' : ' '));

	      if ((unsigned long) segment->p_align == segment->p_align)
		printf ("%#lx", (unsigned long) segment->p_align);
	      else
		{
		  print_vma (segment->p_align, PREFIX_HEX);
		}
	    }
	  else
	    {
	      print_vma (segment->p_offset, FULL_HEX);
	      putchar (' ');
	      print_vma (segment->p_vaddr, FULL_HEX);
	      putchar (' ');
	      print_vma (segment->p_paddr, FULL_HEX);
	      printf ("\n                 ");
	      print_vma (segment->p_filesz, FULL_HEX);
	      putchar (' ');
	      print_vma (segment->p_memsz, FULL_HEX);
	      printf ("  %c%c%c    ",
		      (segment->p_flags & PF_R ? 'R' : ' '),
		      (segment->p_flags & PF_W ? 'W' : ' '),
		      (segment->p_flags & PF_X ? 'E' : ' '));
	      print_vma (segment->p_align, HEX);
	    }
	}

      switch (segment->p_type)
	{
	case PT_DYNAMIC:
	  if (dynamic_addr)
	    error (_("more than one dynamic segment\n"));

	  /* By default, assume that the .dynamic section is the first
	     section in the DYNAMIC segment.  */
	  dynamic_addr = segment->p_offset;
	  dynamic_size = segment->p_filesz;

	  /* Try to locate the .dynamic section. If there is
	     a section header table, we can easily locate it.  */
	  if (section_headers != NULL)
	    {
	      Elf_Internal_Shdr * sec;

	      sec = find_section (".dynamic");
	      if (sec == NULL || sec->sh_size == 0)
		{
                  /* A corresponding .dynamic section is expected, but on
                     IA-64/OpenVMS it is OK for it to be missing.  */
                  if (!is_ia64_vms ())
                    error (_("no .dynamic section in the dynamic segment\n"));
		  break;
		}

	      if (sec->sh_type == SHT_NOBITS)
		{
		  dynamic_size = 0;
		  break;
		}

	      dynamic_addr = sec->sh_offset;
	      dynamic_size = sec->sh_size;

	      if (dynamic_addr < segment->p_offset
		  || dynamic_addr > segment->p_offset + segment->p_filesz)
		warn (_("the .dynamic section is not contained"
			" within the dynamic segment\n"));
	      else if (dynamic_addr > segment->p_offset)
		warn (_("the .dynamic section is not the first section"
			" in the dynamic segment.\n"));
	    }
	  break;

	case PT_INTERP:
	  if (fseek (file, archive_file_offset + (long) segment->p_offset,
		     SEEK_SET))
	    error (_("Unable to find program interpreter name\n"));
	  else
	    {
	      char fmt [32];
	      int ret = snprintf (fmt, sizeof (fmt), "%%%ds", PATH_MAX);

	      if (ret >= (int) sizeof (fmt) || ret < 0)
		error (_("Internal error: failed to create format string to display program interpreter\n"));

	      program_interpreter[0] = 0;
	      if (fscanf (file, fmt, program_interpreter) <= 0)
		error (_("Unable to read program interpreter name\n"));

	      if (do_segments)
		printf (_("\n      [Requesting program interpreter: %s]"),
		    program_interpreter);
	    }
	  break;
	}

      if (do_segments)
	putc ('\n', stdout);
    }

  if (do_segments && section_headers != NULL && string_table != NULL)
    {
      printf (_("\n Section to Segment mapping:\n"));
      printf (_("  Segment Sections...\n"));

      for (i = 0; i < elf_header.e_phnum; i++)
	{
	  unsigned int j;
	  Elf_Internal_Shdr * section;

	  segment = program_headers + i;
	  section = section_headers + 1;

	  printf ("   %2.2d     ", i);

	  for (j = 1; j < elf_header.e_shnum; j++, section++)
	    {
	      if (!ELF_TBSS_SPECIAL (section, segment)
		  && ELF_SECTION_IN_SEGMENT_STRICT (section, segment))
		printf ("%s ", SECTION_NAME (section));
	    }

	  putc ('\n',stdout);
	}
    }

  return 1;
}


/* Find the file offset corresponding to VMA by using the program headers.  */

static long
offset_from_vma (FILE * file, bfd_vma vma, bfd_size_type size)
{
  Elf_Internal_Phdr * seg;

  if (! get_program_headers (file))
    {
      warn (_("Cannot interpret virtual addresses without program headers.\n"));
      return (long) vma;
    }

  for (seg = program_headers;
       seg < program_headers + elf_header.e_phnum;
       ++seg)
    {
      if (seg->p_type != PT_LOAD)
	continue;

      if (vma >= (seg->p_vaddr & -seg->p_align)
	  && vma + size <= seg->p_vaddr + seg->p_filesz)
	return vma - seg->p_vaddr + seg->p_offset;
    }

  warn (_("Virtual address 0x%lx not located in any PT_LOAD segment.\n"),
	(unsigned long) vma);
  return (long) vma;
}


static int
get_32bit_section_headers (FILE * file, unsigned int num)
{
  Elf32_External_Shdr * shdrs;
  Elf_Internal_Shdr *   internal;
  unsigned int i;

  shdrs = (Elf32_External_Shdr *) get_data (NULL, file, elf_header.e_shoff,
                                            elf_header.e_shentsize, num,
                                            _("section headers"));
  if (!shdrs)
    return 0;

  section_headers = (Elf_Internal_Shdr *) cmalloc (num,
                                                   sizeof (Elf_Internal_Shdr));

  if (section_headers == NULL)
    {
      error (_("Out of memory\n"));
      return 0;
    }

  for (i = 0, internal = section_headers;
       i < num;
       i++, internal++)
    {
      internal->sh_name      = BYTE_GET (shdrs[i].sh_name);
      internal->sh_type      = BYTE_GET (shdrs[i].sh_type);
      internal->sh_flags     = BYTE_GET (shdrs[i].sh_flags);
      internal->sh_addr      = BYTE_GET (shdrs[i].sh_addr);
      internal->sh_offset    = BYTE_GET (shdrs[i].sh_offset);
      internal->sh_size      = BYTE_GET (shdrs[i].sh_size);
      internal->sh_link      = BYTE_GET (shdrs[i].sh_link);
      internal->sh_info      = BYTE_GET (shdrs[i].sh_info);
      internal->sh_addralign = BYTE_GET (shdrs[i].sh_addralign);
      internal->sh_entsize   = BYTE_GET (shdrs[i].sh_entsize);
    }

  free (shdrs);

  return 1;
}

static int
get_64bit_section_headers (FILE * file, unsigned int num)
{
  Elf64_External_Shdr * shdrs;
  Elf_Internal_Shdr *   internal;
  unsigned int i;

  shdrs = (Elf64_External_Shdr *) get_data (NULL, file, elf_header.e_shoff,
                                            elf_header.e_shentsize, num,
                                            _("section headers"));
  if (!shdrs)
    return 0;

  section_headers = (Elf_Internal_Shdr *) cmalloc (num,
                                                   sizeof (Elf_Internal_Shdr));

  if (section_headers == NULL)
    {
      error (_("Out of memory\n"));
      return 0;
    }

  for (i = 0, internal = section_headers;
       i < num;
       i++, internal++)
    {
      internal->sh_name      = BYTE_GET (shdrs[i].sh_name);
      internal->sh_type      = BYTE_GET (shdrs[i].sh_type);
      internal->sh_flags     = BYTE_GET (shdrs[i].sh_flags);
      internal->sh_addr      = BYTE_GET (shdrs[i].sh_addr);
      internal->sh_size      = BYTE_GET (shdrs[i].sh_size);
      internal->sh_entsize   = BYTE_GET (shdrs[i].sh_entsize);
      internal->sh_link      = BYTE_GET (shdrs[i].sh_link);
      internal->sh_info      = BYTE_GET (shdrs[i].sh_info);
      internal->sh_offset    = BYTE_GET (shdrs[i].sh_offset);
      internal->sh_addralign = BYTE_GET (shdrs[i].sh_addralign);
    }

  free (shdrs);

  return 1;
}

static Elf_Internal_Sym *
get_32bit_elf_symbols (FILE * file,
		       Elf_Internal_Shdr * section,
		       unsigned long * num_syms_return)
{
  unsigned long number = 0;
  Elf32_External_Sym * esyms = NULL;
  Elf_External_Sym_Shndx * shndx = NULL;
  Elf_Internal_Sym * isyms = NULL;
  Elf_Internal_Sym * psym;
  unsigned int j;

  /* Run some sanity checks first.  */
  if (section->sh_entsize == 0)
    {
      error (_("sh_entsize is zero\n"));
      goto exit_point;
    }

  number = section->sh_size / section->sh_entsize;

  if (number * sizeof (Elf32_External_Sym) > section->sh_size + 1)
    {
      error (_("Invalid sh_entsize\n"));
      goto exit_point;
    }

  esyms = (Elf32_External_Sym *) get_data (NULL, file, section->sh_offset, 1,
                                           section->sh_size, _("symbols"));
  if (esyms == NULL)
    goto exit_point;

  shndx = NULL;
  if (symtab_shndx_hdr != NULL
      && (symtab_shndx_hdr->sh_link
	  == (unsigned long) (section - section_headers)))
    {
      shndx = (Elf_External_Sym_Shndx *) get_data (NULL, file,
                                                   symtab_shndx_hdr->sh_offset,
                                                   1, symtab_shndx_hdr->sh_size,
                                                   _("symbol table section indicies"));
      if (shndx == NULL)
	goto exit_point;
    }

  isyms = (Elf_Internal_Sym *) cmalloc (number, sizeof (Elf_Internal_Sym));

  if (isyms == NULL)
    {
      error (_("Out of memory\n"));
      goto exit_point;
    }

  for (j = 0, psym = isyms; j < number; j++, psym++)
    {
      psym->st_name  = BYTE_GET (esyms[j].st_name);
      psym->st_value = BYTE_GET (esyms[j].st_value);
      psym->st_size  = BYTE_GET (esyms[j].st_size);
      psym->st_shndx = BYTE_GET (esyms[j].st_shndx);
      if (psym->st_shndx == (SHN_XINDEX & 0xffff) && shndx != NULL)
	psym->st_shndx
	  = byte_get ((unsigned char *) &shndx[j], sizeof (shndx[j]));
      else if (psym->st_shndx >= (SHN_LORESERVE & 0xffff))
	psym->st_shndx += SHN_LORESERVE - (SHN_LORESERVE & 0xffff);
      psym->st_info  = BYTE_GET (esyms[j].st_info);
      psym->st_other = BYTE_GET (esyms[j].st_other);
    }

 exit_point:
  if (shndx != NULL)
    free (shndx);
  if (esyms != NULL)
    free (esyms);

  if (num_syms_return != NULL)
    * num_syms_return = isyms == NULL ? 0 : number;

  return isyms;
}

static Elf_Internal_Sym *
get_64bit_elf_symbols (FILE * file,
		       Elf_Internal_Shdr * section,
		       unsigned long * num_syms_return)
{
  unsigned long number = 0;
  Elf64_External_Sym * esyms = NULL;
  Elf_External_Sym_Shndx * shndx = NULL;
  Elf_Internal_Sym * isyms = NULL;
  Elf_Internal_Sym * psym;
  unsigned int j;

  /* Run some sanity checks first.  */
  if (section->sh_entsize == 0)
    {
      error (_("sh_entsize is zero\n"));
      goto exit_point;
    }

  number = section->sh_size / section->sh_entsize;

  if (number * sizeof (Elf64_External_Sym) > section->sh_size + 1)
    {
      error (_("Invalid sh_entsize\n"));
      goto exit_point;
    }

  esyms = (Elf64_External_Sym *) get_data (NULL, file, section->sh_offset, 1,
                                           section->sh_size, _("symbols"));
  if (!esyms)
    goto exit_point;

  if (symtab_shndx_hdr != NULL
      && (symtab_shndx_hdr->sh_link
	  == (unsigned long) (section - section_headers)))
    {
      shndx = (Elf_External_Sym_Shndx *) get_data (NULL, file,
                                                   symtab_shndx_hdr->sh_offset,
                                                   1, symtab_shndx_hdr->sh_size,
                                                   _("symbol table section indicies"));
      if (shndx == NULL)
	goto exit_point;
    }

  isyms = (Elf_Internal_Sym *) cmalloc (number, sizeof (Elf_Internal_Sym));

  if (isyms == NULL)
    {
      error (_("Out of memory\n"));
      goto exit_point;
    }

  for (j = 0, psym = isyms; j < number; j++, psym++)
    {
      psym->st_name  = BYTE_GET (esyms[j].st_name);
      psym->st_info  = BYTE_GET (esyms[j].st_info);
      psym->st_other = BYTE_GET (esyms[j].st_other);
      psym->st_shndx = BYTE_GET (esyms[j].st_shndx);

      if (psym->st_shndx == (SHN_XINDEX & 0xffff) && shndx != NULL)
	psym->st_shndx
	  = byte_get ((unsigned char *) &shndx[j], sizeof (shndx[j]));
      else if (psym->st_shndx >= (SHN_LORESERVE & 0xffff))
	psym->st_shndx += SHN_LORESERVE - (SHN_LORESERVE & 0xffff);

      psym->st_value = BYTE_GET (esyms[j].st_value);
      psym->st_size  = BYTE_GET (esyms[j].st_size);
    }

 exit_point:
  if (shndx != NULL)
    free (shndx);
  if (esyms != NULL)
    free (esyms);

  if (num_syms_return != NULL)
    * num_syms_return = isyms == NULL ? 0 : number;

  return isyms;
}

static const char *
get_elf_section_flags (bfd_vma sh_flags)
{
  static char buff[1024];
  char * p = buff;
  int field_size = is_32bit_elf ? 8 : 16;
  int sindex;
  int size = sizeof (buff) - (field_size + 4 + 1);
  bfd_vma os_flags = 0;
  bfd_vma proc_flags = 0;
  bfd_vma unknown_flags = 0;
  static const struct
    {
      const char * str;
      int len;
    }
  flags [] =
    {
      /*  0 */ { STRING_COMMA_LEN ("WRITE") },
      /*  1 */ { STRING_COMMA_LEN ("ALLOC") },
      /*  2 */ { STRING_COMMA_LEN ("EXEC") },
      /*  3 */ { STRING_COMMA_LEN ("MERGE") },
      /*  4 */ { STRING_COMMA_LEN ("STRINGS") },
      /*  5 */ { STRING_COMMA_LEN ("INFO LINK") },
      /*  6 */ { STRING_COMMA_LEN ("LINK ORDER") },
      /*  7 */ { STRING_COMMA_LEN ("OS NONCONF") },
      /*  8 */ { STRING_COMMA_LEN ("GROUP") },
      /*  9 */ { STRING_COMMA_LEN ("TLS") },
      /* IA-64 specific.  */
      /* 10 */ { STRING_COMMA_LEN ("SHORT") },
      /* 11 */ { STRING_COMMA_LEN ("NORECOV") },
      /* IA-64 OpenVMS specific.  */
      /* 12 */ { STRING_COMMA_LEN ("VMS_GLOBAL") },
      /* 13 */ { STRING_COMMA_LEN ("VMS_OVERLAID") },
      /* 14 */ { STRING_COMMA_LEN ("VMS_SHARED") },
      /* 15 */ { STRING_COMMA_LEN ("VMS_VECTOR") },
      /* 16 */ { STRING_COMMA_LEN ("VMS_ALLOC_64BIT") },
      /* 17 */ { STRING_COMMA_LEN ("VMS_PROTECTED") },
      /* Generic.  */
      /* 18 */ { STRING_COMMA_LEN ("EXCLUDE") },
      /* SPARC specific.  */
      /* 19 */ { STRING_COMMA_LEN ("ORDERED") }
    };

  if (do_section_details)
    {
      sprintf (buff, "[%*.*lx]: ",
	       field_size, field_size, (unsigned long) sh_flags);
      p += field_size + 4;
    }

  while (sh_flags)
    {
      bfd_vma flag;

      flag = sh_flags & - sh_flags;
      sh_flags &= ~ flag;

      if (do_section_details)
	{
	  switch (flag)
	    {
	    case SHF_WRITE:		sindex = 0; break;
	    case SHF_ALLOC:		sindex = 1; break;
	    case SHF_EXECINSTR:		sindex = 2; break;
	    case SHF_MERGE:		sindex = 3; break;
	    case SHF_STRINGS:		sindex = 4; break;
	    case SHF_INFO_LINK:		sindex = 5; break;
	    case SHF_LINK_ORDER:	sindex = 6; break;
	    case SHF_OS_NONCONFORMING:	sindex = 7; break;
	    case SHF_GROUP:		sindex = 8; break;
	    case SHF_TLS:		sindex = 9; break;
	    case SHF_EXCLUDE:		sindex = 18; break;

	    default:
	      sindex = -1;
	      switch (elf_header.e_machine)
		{
		case EM_IA_64:
		  if (flag == SHF_IA_64_SHORT)
		    sindex = 10;
		  else if (flag == SHF_IA_64_NORECOV)
		    sindex = 11;
#ifdef BFD64
		  else if (elf_header.e_ident[EI_OSABI] == ELFOSABI_OPENVMS)
		    switch (flag)
		      {
		      case SHF_IA_64_VMS_GLOBAL:      sindex = 12; break;
		      case SHF_IA_64_VMS_OVERLAID:    sindex = 13; break;
		      case SHF_IA_64_VMS_SHARED:      sindex = 14; break;
		      case SHF_IA_64_VMS_VECTOR:      sindex = 15; break;
		      case SHF_IA_64_VMS_ALLOC_64BIT: sindex = 16; break;
		      case SHF_IA_64_VMS_PROTECTED:   sindex = 17; break;
		      default:                        break;
		      }
#endif
		  break;

		case EM_386:
		case EM_486:
		case EM_X86_64:
		case EM_L1OM:
		case EM_K1OM:
		case EM_OLD_SPARCV9:
		case EM_SPARC32PLUS:
		case EM_SPARCV9:
		case EM_SPARC:
		  if (flag == SHF_ORDERED)
		    sindex = 19;
		  break;
		default:
		  break;
		}
	    }

	  if (sindex != -1)
	    {
	      if (p != buff + field_size + 4)
		{
		  if (size < (10 + 2))
		    abort ();
		  size -= 2;
		  *p++ = ',';
		  *p++ = ' ';
		}

	      size -= flags [sindex].len;
	      p = stpcpy (p, flags [sindex].str);
	    }
	  else if (flag & SHF_MASKOS)
	    os_flags |= flag;
	  else if (flag & SHF_MASKPROC)
	    proc_flags |= flag;
	  else
	    unknown_flags |= flag;
	}
      else
	{
	  switch (flag)
	    {
	    case SHF_WRITE:		*p = 'W'; break;
	    case SHF_ALLOC:		*p = 'A'; break;
	    case SHF_EXECINSTR:		*p = 'X'; break;
	    case SHF_MERGE:		*p = 'M'; break;
	    case SHF_STRINGS:		*p = 'S'; break;
	    case SHF_INFO_LINK:		*p = 'I'; break;
	    case SHF_LINK_ORDER:	*p = 'L'; break;
	    case SHF_OS_NONCONFORMING:	*p = 'O'; break;
	    case SHF_GROUP:		*p = 'G'; break;
	    case SHF_TLS:		*p = 'T'; break;
	    case SHF_EXCLUDE:		*p = 'E'; break;

	    default:
	      if ((elf_header.e_machine == EM_X86_64
		   || elf_header.e_machine == EM_L1OM
		   || elf_header.e_machine == EM_K1OM)
		  && flag == SHF_X86_64_LARGE)
		*p = 'l';
	      else if (flag & SHF_MASKOS)
		{
		  *p = 'o';
		  sh_flags &= ~ SHF_MASKOS;
		}
	      else if (flag & SHF_MASKPROC)
		{
		  *p = 'p';
		  sh_flags &= ~ SHF_MASKPROC;
		}
	      else
		*p = 'x';
	      break;
	    }
	  p++;
	}
    }

  if (do_section_details)
    {
      if (os_flags)
	{
	  size -= 5 + field_size;
	  if (p != buff + field_size + 4)
	    {
	      if (size < (2 + 1))
		abort ();
	      size -= 2;
	      *p++ = ',';
	      *p++ = ' ';
	    }
	  sprintf (p, "OS (%*.*lx)", field_size, field_size,
		   (unsigned long) os_flags);
	  p += 5 + field_size;
	}
      if (proc_flags)
	{
	  size -= 7 + field_size;
	  if (p != buff + field_size + 4)
	    {
	      if (size < (2 + 1))
		abort ();
	      size -= 2;
	      *p++ = ',';
	      *p++ = ' ';
	    }
	  sprintf (p, "PROC (%*.*lx)", field_size, field_size,
		   (unsigned long) proc_flags);
	  p += 7 + field_size;
	}
      if (unknown_flags)
	{
	  size -= 10 + field_size;
	  if (p != buff + field_size + 4)
	    {
	      if (size < (2 + 1))
		abort ();
	      size -= 2;
	      *p++ = ',';
	      *p++ = ' ';
	    }
	  sprintf (p, _("UNKNOWN (%*.*lx)"), field_size, field_size,
		   (unsigned long) unknown_flags);
	  p += 10 + field_size;
	}
    }

  *p = '\0';
  return buff;
}

static int
process_section_headers (FILE * file)
{
  Elf_Internal_Shdr * section;
  unsigned int i;

  section_headers = NULL;

  if (elf_header.e_shnum == 0)
    {
      /* PR binutils/12467.  */
      if (elf_header.e_shoff != 0)
	warn (_("possibly corrupt ELF file header - it has a non-zero"
		" section header offset, but no section headers\n"));
      else if (do_sections)
	printf (_("\nThere are no sections in this file.\n"));

      return 1;
    }

  if (do_sections && !do_header)
    printf (_("There are %d section headers, starting at offset 0x%lx:\n"),
	    elf_header.e_shnum, (unsigned long) elf_header.e_shoff);

  if (is_32bit_elf)
    {
      if (! get_32bit_section_headers (file, elf_header.e_shnum))
	return 0;
    }
  else if (! get_64bit_section_headers (file, elf_header.e_shnum))
    return 0;

  /* Read in the string table, so that we have names to display.  */
  if (elf_header.e_shstrndx != SHN_UNDEF
       && elf_header.e_shstrndx < elf_header.e_shnum)
    {
      section = section_headers + elf_header.e_shstrndx;

      if (section->sh_size != 0)
	{
	  string_table = (char *) get_data (NULL, file, section->sh_offset,
                                            1, section->sh_size,
                                            _("string table"));

	  string_table_length = string_table != NULL ? section->sh_size : 0;
	}
    }

  /* Scan the sections for the dynamic symbol table
     and dynamic string table and debug sections.  */
  dynamic_symbols = NULL;
  dynamic_strings = NULL;
  dynamic_syminfo = NULL;
  symtab_shndx_hdr = NULL;

  eh_addr_size = is_32bit_elf ? 4 : 8;
  switch (elf_header.e_machine)
    {
    case EM_MIPS:
    case EM_MIPS_RS3_LE:
      /* The 64-bit MIPS EABI uses a combination of 32-bit ELF and 64-bit
	 FDE addresses.  However, the ABI also has a semi-official ILP32
	 variant for which the normal FDE address size rules apply.

	 GCC 4.0 marks EABI64 objects with a dummy .gcc_compiled_longXX
	 section, where XX is the size of longs in bits.  Unfortunately,
	 earlier compilers provided no way of distinguishing ILP32 objects
	 from LP64 objects, so if there's any doubt, we should assume that
	 the official LP64 form is being used.  */
      if ((elf_header.e_flags & EF_MIPS_ABI) == E_MIPS_ABI_EABI64
	  && find_section (".gcc_compiled_long32") == NULL)
	eh_addr_size = 8;
      break;

    case EM_H8_300:
    case EM_H8_300H:
      switch (elf_header.e_flags & EF_H8_MACH)
	{
	case E_H8_MACH_H8300:
	case E_H8_MACH_H8300HN:
	case E_H8_MACH_H8300SN:
	case E_H8_MACH_H8300SXN:
	  eh_addr_size = 2;
	  break;
	case E_H8_MACH_H8300H:
	case E_H8_MACH_H8300S:
	case E_H8_MACH_H8300SX:
	  eh_addr_size = 4;
	  break;
	}
      break;

    case EM_M32C_OLD:
    case EM_M32C:
      switch (elf_header.e_flags & EF_M32C_CPU_MASK)
	{
	case EF_M32C_CPU_M16C:
	  eh_addr_size = 2;
	  break;
	}
      break;
    }

#define CHECK_ENTSIZE_VALUES(section, i, size32, size64) \
  do									    \
    {									    \
      size_t expected_entsize						    \
	= is_32bit_elf ? size32 : size64;				    \
      if (section->sh_entsize != expected_entsize)			    \
	error (_("Section %d has invalid sh_entsize %lx (expected %lx)\n"), \
	       i, (unsigned long int) section->sh_entsize,		    \
	       (unsigned long int) expected_entsize);			    \
      section->sh_entsize = expected_entsize;				    \
    }									    \
  while (0)
#define CHECK_ENTSIZE(section, i, type) \
  CHECK_ENTSIZE_VALUES (section, i, sizeof (Elf32_External_##type),	    \
			sizeof (Elf64_External_##type))

  for (i = 0, section = section_headers;
       i < elf_header.e_shnum;
       i++, section++)
    {
      char * name = SECTION_NAME (section);

      if (section->sh_type == SHT_DYNSYM)
	{
	  if (dynamic_symbols != NULL)
	    {
	      error (_("File contains multiple dynamic symbol tables\n"));
	      continue;
	    }

	  CHECK_ENTSIZE (section, i, Sym);
	  dynamic_symbols = GET_ELF_SYMBOLS (file, section, & num_dynamic_syms);
	}
      else if (section->sh_type == SHT_STRTAB
	       && streq (name, ".dynstr"))
	{
	  if (dynamic_strings != NULL)
	    {
	      error (_("File contains multiple dynamic string tables\n"));
	      continue;
	    }

	  dynamic_strings = (char *) get_data (NULL, file, section->sh_offset,
                                               1, section->sh_size,
                                               _("dynamic strings"));
	  dynamic_strings_length = dynamic_strings == NULL ? 0 : section->sh_size;
	}
      else if (section->sh_type == SHT_SYMTAB_SHNDX)
	{
	  if (symtab_shndx_hdr != NULL)
	    {
	      error (_("File contains multiple symtab shndx tables\n"));
	      continue;
	    }
	  symtab_shndx_hdr = section;
	}
      else if (section->sh_type == SHT_SYMTAB)
	CHECK_ENTSIZE (section, i, Sym);
      else if (section->sh_type == SHT_GROUP)
	CHECK_ENTSIZE_VALUES (section, i, GRP_ENTRY_SIZE, GRP_ENTRY_SIZE);
      else if (section->sh_type == SHT_REL)
	CHECK_ENTSIZE (section, i, Rel);
      else if (section->sh_type == SHT_RELA)
	CHECK_ENTSIZE (section, i, Rela);
      else if ((do_debugging || do_debug_info || do_debug_abbrevs
		|| do_debug_lines || do_debug_pubnames || do_debug_pubtypes
		|| do_debug_aranges || do_debug_frames || do_debug_macinfo
		|| do_debug_str || do_debug_loc || do_debug_ranges)
	       && (const_strneq (name, ".debug_")
                   || const_strneq (name, ".zdebug_")))
	{
          if (name[1] == 'z')
            name += sizeof (".zdebug_") - 1;
          else
            name += sizeof (".debug_") - 1;

	  if (do_debugging
	      || (do_debug_info     && const_strneq (name, "info"))
	      || (do_debug_info     && const_strneq (name, "types"))
	      || (do_debug_abbrevs  && const_strneq (name, "abbrev"))
	      || (do_debug_lines    && const_strneq (name, "line"))
	      || (do_debug_pubnames && const_strneq (name, "pubnames"))
	      || (do_debug_pubtypes && const_strneq (name, "pubtypes"))
	      || (do_debug_aranges  && const_strneq (name, "aranges"))
	      || (do_debug_ranges   && const_strneq (name, "ranges"))
	      || (do_debug_frames   && const_strneq (name, "frame"))
	      || (do_debug_macinfo  && const_strneq (name, "macinfo"))
	      || (do_debug_macinfo  && const_strneq (name, "macro"))
	      || (do_debug_str      && const_strneq (name, "str"))
	      || (do_debug_loc      && const_strneq (name, "loc"))
	      )
	    request_dump_bynumber (i, DEBUG_DUMP);
	}
      /* Linkonce section to be combined with .debug_info at link time.  */
      else if ((do_debugging || do_debug_info)
	       && const_strneq (name, ".gnu.linkonce.wi."))
	request_dump_bynumber (i, DEBUG_DUMP);
      else if (do_debug_frames && streq (name, ".eh_frame"))
	request_dump_bynumber (i, DEBUG_DUMP);
      else if (do_gdb_index && streq (name, ".gdb_index"))
	request_dump_bynumber (i, DEBUG_DUMP);
      /* Trace sections for Itanium VMS.  */
      else if ((do_debugging || do_trace_info || do_trace_abbrevs
                || do_trace_aranges)
	       && const_strneq (name, ".trace_"))
	{
          name += sizeof (".trace_") - 1;

	  if (do_debugging
	      || (do_trace_info     && streq (name, "info"))
	      || (do_trace_abbrevs  && streq (name, "abbrev"))
	      || (do_trace_aranges  && streq (name, "aranges"))
	      )
	    request_dump_bynumber (i, DEBUG_DUMP);
	}

    }

  if (! do_sections)
    return 1;

  if (elf_header.e_shnum > 1)
    printf (_("\nSection Headers:\n"));
  else
    printf (_("\nSection Header:\n"));

  if (is_32bit_elf)
    {
      if (do_section_details)
	{
	  printf (_("  [Nr] Name\n"));
	  printf (_("       Type            Addr     Off    Size   ES   Lk Inf Al\n"));
	}
      else
	printf
	  (_("  [Nr] Name              Type            Addr     Off    Size   ES Flg Lk Inf Al\n"));
    }
  else if (do_wide)
    {
      if (do_section_details)
	{
	  printf (_("  [Nr] Name\n"));
	  printf (_("       Type            Address          Off    Size   ES   Lk Inf Al\n"));
	}
      else
	printf
	  (_("  [Nr] Name              Type            Address          Off    Size   ES Flg Lk Inf Al\n"));
    }
  else
    {
      if (do_section_details)
	{
	  printf (_("  [Nr] Name\n"));
	  printf (_("       Type              Address          Offset            Link\n"));
	  printf (_("       Size              EntSize          Info              Align\n"));
	}
      else
	{
	  printf (_("  [Nr] Name              Type             Address           Offset\n"));
	  printf (_("       Size              EntSize          Flags  Link  Info  Align\n"));
	}
    }

  if (do_section_details)
    printf (_("       Flags\n"));

  for (i = 0, section = section_headers;
       i < elf_header.e_shnum;
       i++, section++)
    {
      printf ("  [%2u] ", i);
      if (do_section_details)
	{
	  print_symbol (INT_MAX, SECTION_NAME (section));
	  printf ("\n      ");
	}
      else
	{
	  print_symbol (-17, SECTION_NAME (section));
	}
      
      printf (do_wide ? " %-15s " : " %-15.15s ",
	      get_section_type_name (section->sh_type));
      
      if (is_32bit_elf)
	{
	  const char * link_too_big = NULL;

	  print_vma (section->sh_addr, LONG_HEX);

	  printf ( " %6.6lx %6.6lx %2.2lx",
		   (unsigned long) section->sh_offset,
		   (unsigned long) section->sh_size,
		   (unsigned long) section->sh_entsize);

	  if (do_section_details)
	    fputs ("  ", stdout);
	  else
	    printf (" %3s ", get_elf_section_flags (section->sh_flags));

	  if (section->sh_link >= elf_header.e_shnum)
	    {
	      link_too_big = "";
	      /* The sh_link value is out of range.  Normally this indicates
		 an error but it can have special values in Solaris binaries.  */
	      switch (elf_header.e_machine)
		{
		case EM_386:
		case EM_486:
		case EM_X86_64:
		case EM_L1OM:
		case EM_K1OM:
		case EM_OLD_SPARCV9:
		case EM_SPARC32PLUS:
		case EM_SPARCV9:
		case EM_SPARC:
		  if (section->sh_link == (SHN_BEFORE & 0xffff))
		    link_too_big = "BEFORE";
		  else if (section->sh_link == (SHN_AFTER & 0xffff))
		    link_too_big = "AFTER";
		  break;
		default:
		  break;
		}
	    }

	  if (do_section_details)
	    {
	      if (link_too_big != NULL && * link_too_big)
		printf ("<%s> ", link_too_big);
	      else
		printf ("%2u ", section->sh_link);
	      printf ("%3u %2lu\n", section->sh_info,
		      (unsigned long) section->sh_addralign);
	    }
	  else
	    printf ("%2u %3u %2lu\n",
		    section->sh_link,
		    section->sh_info,
		    (unsigned long) section->sh_addralign);

	  if (link_too_big && ! * link_too_big)
	    warn (_("section %u: sh_link value of %u is larger than the number of sections\n"),
		  i, section->sh_link);
	}
      else if (do_wide)
	{
	  print_vma (section->sh_addr, LONG_HEX);

	  if ((long) section->sh_offset == section->sh_offset)
	    printf (" %6.6lx", (unsigned long) section->sh_offset);
	  else
	    {
	      putchar (' ');
	      print_vma (section->sh_offset, LONG_HEX);
	    }

	  if ((unsigned long) section->sh_size == section->sh_size)
	    printf (" %6.6lx", (unsigned long) section->sh_size);
	  else
	    {
	      putchar (' ');
	      print_vma (section->sh_size, LONG_HEX);
	    }

	  if ((unsigned long) section->sh_entsize == section->sh_entsize)
	    printf (" %2.2lx", (unsigned long) section->sh_entsize);
	  else
	    {
	      putchar (' ');
	      print_vma (section->sh_entsize, LONG_HEX);
	    }

	  if (do_section_details)
	    fputs ("  ", stdout);
	  else
	    printf (" %3s ", get_elf_section_flags (section->sh_flags));

	  printf ("%2u %3u ", section->sh_link, section->sh_info);

	  if ((unsigned long) section->sh_addralign == section->sh_addralign)
	    printf ("%2lu\n", (unsigned long) section->sh_addralign);
	  else
	    {
	      print_vma (section->sh_addralign, DEC);
	      putchar ('\n');
	    }
	}
      else if (do_section_details)
	{
	  printf ("       %-15.15s  ",
		  get_section_type_name (section->sh_type));
	  print_vma (section->sh_addr, LONG_HEX);
	  if ((long) section->sh_offset == section->sh_offset)
	    printf ("  %16.16lx", (unsigned long) section->sh_offset);
	  else
	    {
	      printf ("  ");
	      print_vma (section->sh_offset, LONG_HEX);
	    }
	  printf ("  %u\n       ", section->sh_link);
	  print_vma (section->sh_size, LONG_HEX);
	  putchar (' ');
	  print_vma (section->sh_entsize, LONG_HEX);

	  printf ("  %-16u  %lu\n",
		  section->sh_info,
		  (unsigned long) section->sh_addralign);
	}
      else
	{
	  putchar (' ');
	  print_vma (section->sh_addr, LONG_HEX);
	  if ((long) section->sh_offset == section->sh_offset)
	    printf ("  %8.8lx", (unsigned long) section->sh_offset);
	  else
	    {
	      printf ("  ");
	      print_vma (section->sh_offset, LONG_HEX);
	    }
	  printf ("\n       ");
	  print_vma (section->sh_size, LONG_HEX);
	  printf ("  ");
	  print_vma (section->sh_entsize, LONG_HEX);

	  printf (" %3s ", get_elf_section_flags (section->sh_flags));

	  printf ("     %2u   %3u     %lu\n",
		  section->sh_link,
		  section->sh_info,
		  (unsigned long) section->sh_addralign);
	}

      if (do_section_details)
	printf ("       %s\n", get_elf_section_flags (section->sh_flags));
    }

  if (!do_section_details)
    {
      if (elf_header.e_machine == EM_X86_64
	  || elf_header.e_machine == EM_L1OM
	  || elf_header.e_machine == EM_K1OM)
	printf (_("Key to Flags:\n\
  W (write), A (alloc), X (execute), M (merge), S (strings), l (large)\n\
  I (info), L (link order), G (group), T (TLS), E (exclude), x (unknown)\n\
  O (extra OS processing required) o (OS specific), p (processor specific)\n"));
      else
	printf (_("Key to Flags:\n\
  W (write), A (alloc), X (execute), M (merge), S (strings)\n\
  I (info), L (link order), G (group), T (TLS), E (exclude), x (unknown)\n\
  O (extra OS processing required) o (OS specific), p (processor specific)\n"));
    }	

  return 1;
}

static const char *
get_group_flags (unsigned int flags)
{
  static char buff[32];
  switch (flags)
    {
    case 0:
      return "";

    case GRP_COMDAT:
      return "COMDAT ";

   default:
      snprintf (buff, sizeof (buff), _("[<unknown>: 0x%x] "), flags);
      break;
    }
  return buff;
}

static int
process_section_groups (FILE * file)
{
  Elf_Internal_Shdr * section;
  unsigned int i;
  struct group * group;
  Elf_Internal_Shdr * symtab_sec;
  Elf_Internal_Shdr * strtab_sec;
  Elf_Internal_Sym * symtab;
  unsigned long num_syms;
  char * strtab;
  size_t strtab_size;

  /* Don't process section groups unless needed.  */
  if (!do_unwind && !do_section_groups)
    return 1;

  if (elf_header.e_shnum == 0)
    {
      if (do_section_groups)
	printf (_("\nThere are no sections to group in this file.\n"));

      return 1;
    }

  if (section_headers == NULL)
    {
      error (_("Section headers are not available!\n"));
      /* PR 13622: This can happen with a corrupt ELF header.  */
      return 0;
    }

  section_headers_groups = (struct group **) calloc (elf_header.e_shnum,
                                                     sizeof (struct group *));

  if (section_headers_groups == NULL)
    {
      error (_("Out of memory\n"));
      return 0;
    }

  /* Scan the sections for the group section.  */
  group_count = 0;
  for (i = 0, section = section_headers;
       i < elf_header.e_shnum;
       i++, section++)
    if (section->sh_type == SHT_GROUP)
      group_count++;

  if (group_count == 0)
    {
      if (do_section_groups)
	printf (_("\nThere are no section groups in this file.\n"));

      return 1;
    }

  section_groups = (struct group *) calloc (group_count, sizeof (struct group));

  if (section_groups == NULL)
    {
      error (_("Out of memory\n"));
      return 0;
    }

  symtab_sec = NULL;
  strtab_sec = NULL;
  symtab = NULL;
  num_syms = 0;
  strtab = NULL;
  strtab_size = 0;
  for (i = 0, section = section_headers, group = section_groups;
       i < elf_header.e_shnum;
       i++, section++)
    {
      if (section->sh_type == SHT_GROUP)
	{
	  char * name = SECTION_NAME (section);
	  char * group_name;
	  unsigned char * start;
	  unsigned char * indices;
	  unsigned int entry, j, size;
	  Elf_Internal_Shdr * sec;
	  Elf_Internal_Sym * sym;

	  /* Get the symbol table.  */
	  if (section->sh_link >= elf_header.e_shnum
	      || ((sec = section_headers + section->sh_link)->sh_type
		  != SHT_SYMTAB))
	    {
	      error (_("Bad sh_link in group section `%s'\n"), name);
	      continue;
	    }

	  if (symtab_sec != sec)
	    {
	      symtab_sec = sec;
	      if (symtab)
		free (symtab);
	      symtab = GET_ELF_SYMBOLS (file, symtab_sec, & num_syms);
	    }

	  if (symtab == NULL)
	    {
	      error (_("Corrupt header in group section `%s'\n"), name);
	      continue;
	    }

	  if (section->sh_info >= num_syms)
	    {
	      error (_("Bad sh_info in group section `%s'\n"), name);
	      continue;
	    }

	  sym = symtab + section->sh_info;

	  if (ELF_ST_TYPE (sym->st_info) == STT_SECTION)
	    {
	      if (sym->st_shndx == 0
		  || sym->st_shndx >= elf_header.e_shnum)
		{
		  error (_("Bad sh_info in group section `%s'\n"), name);
		  continue;
		}

	      group_name = SECTION_NAME (section_headers + sym->st_shndx);
	      strtab_sec = NULL;
	      if (strtab)
		free (strtab);
	      strtab = NULL;
	      strtab_size = 0;
	    }
	  else
	    {
	      /* Get the string table.  */
	      if (symtab_sec->sh_link >= elf_header.e_shnum)
		{
		  strtab_sec = NULL;
		  if (strtab)
		    free (strtab);
		  strtab = NULL;
		  strtab_size = 0;
		}
	      else if (strtab_sec
		       != (sec = section_headers + symtab_sec->sh_link))
		{
		  strtab_sec = sec;
		  if (strtab)
		    free (strtab);
		  strtab = (char *) get_data (NULL, file, strtab_sec->sh_offset,
                                              1, strtab_sec->sh_size,
                                              _("string table"));
		  strtab_size = strtab != NULL ? strtab_sec->sh_size : 0;
		}
	      group_name = sym->st_name < strtab_size
		? strtab + sym->st_name : _("<corrupt>");
	    }

	  start = (unsigned char *) get_data (NULL, file, section->sh_offset,
                                              1, section->sh_size,
                                              _("section data"));
	  if (start == NULL)
	    continue;

	  indices = start;
	  size = (section->sh_size / section->sh_entsize) - 1;
	  entry = byte_get (indices, 4);
	  indices += 4;

	  if (do_section_groups)
	    {
	      printf (_("\n%sgroup section [%5u] `%s' [%s] contains %u sections:\n"),
		      get_group_flags (entry), i, name, group_name, size);

	      printf (_("   [Index]    Name\n"));
	    }

	  group->group_index = i;

	  for (j = 0; j < size; j++)
	    {
	      struct group_list * g;

	      entry = byte_get (indices, 4);
	      indices += 4;

	      if (entry >= elf_header.e_shnum)
		{
		  error (_("section [%5u] in group section [%5u] > maximum section [%5u]\n"),
			 entry, i, elf_header.e_shnum - 1);
		  continue;
		}

	      if (section_headers_groups [entry] != NULL)
		{
		  if (entry)
		    {
		      error (_("section [%5u] in group section [%5u] already in group section [%5u]\n"),
			     entry, i,
			     section_headers_groups [entry]->group_index);
		      continue;
		    }
		  else
		    {
		      /* Intel C/C++ compiler may put section 0 in a
			 section group. We just warn it the first time
			 and ignore it afterwards.  */
		      static int warned = 0;
		      if (!warned)
			{
			  error (_("section 0 in group section [%5u]\n"),
				 section_headers_groups [entry]->group_index);
			  warned++;
			}
		    }
		}

	      section_headers_groups [entry] = group;

	      if (do_section_groups)
		{
		  sec = section_headers + entry;
		  printf ("   [%5u]   %s\n", entry, SECTION_NAME (sec));
		}

	      g = (struct group_list *) xmalloc (sizeof (struct group_list));
	      g->section_index = entry;
	      g->next = group->root;
	      group->root = g;
	    }

	  if (start)
	    free (start);

	  group++;
	}
    }

  if (symtab)
    free (symtab);
  if (strtab)
    free (strtab);
  return 1;
}

/* Data used to display dynamic fixups.  */

struct ia64_vms_dynfixup
{
  bfd_vma needed_ident;		/* Library ident number.  */
  bfd_vma needed;		/* Index in the dstrtab of the library name.  */
  bfd_vma fixup_needed;		/* Index of the library.  */
  bfd_vma fixup_rela_cnt;	/* Number of fixups.  */
  bfd_vma fixup_rela_off;	/* Fixups offset in the dynamic segment.  */
};

/* Data used to display dynamic relocations.  */

struct ia64_vms_dynimgrela
{
  bfd_vma img_rela_cnt;		/* Number of relocations.  */
  bfd_vma img_rela_off;		/* Reloc offset in the dynamic segment.  */
};

/* Display IA-64 OpenVMS dynamic fixups (used to dynamically link a shared
   library).  */

static void
dump_ia64_vms_dynamic_fixups (FILE *file, struct ia64_vms_dynfixup *fixup,
                              const char *strtab, unsigned int strtab_sz)
{
  Elf64_External_VMS_IMAGE_FIXUP *imfs;
  long i;
  const char *lib_name;

  imfs = get_data (NULL, file, dynamic_addr + fixup->fixup_rela_off,
		   1, fixup->fixup_rela_cnt * sizeof (*imfs),
		   _("dynamic section image fixups"));
  if (!imfs)
    return;

  if (fixup->needed < strtab_sz)
    lib_name = strtab + fixup->needed;
  else
    {
      warn ("corrupt library name index of 0x%lx found in dynamic entry",
            (unsigned long) fixup->needed);
      lib_name = "???";
    }
  printf (_("\nImage fixups for needed library #%d: %s - ident: %lx\n"),
	  (int) fixup->fixup_needed, lib_name, (long) fixup->needed_ident);
  printf
    (_("Seg Offset           Type                             SymVec DataType\n"));

  for (i = 0; i < (long) fixup->fixup_rela_cnt; i++)
    {
      unsigned int type;
      const char *rtype;

      printf ("%3u ", (unsigned) BYTE_GET (imfs [i].fixup_seg));
      printf_vma ((bfd_vma) BYTE_GET (imfs [i].fixup_offset));
      type = BYTE_GET (imfs [i].type);
      rtype = elf_ia64_reloc_type (type);
      if (rtype == NULL)
        printf (" 0x%08x                       ", type);
      else
        printf (" %-32s ", rtype);
      printf ("%6u ", (unsigned) BYTE_GET (imfs [i].symvec_index));
      printf ("0x%08x\n", (unsigned) BYTE_GET (imfs [i].data_type));
    }

  free (imfs);
}

/* Display IA-64 OpenVMS dynamic relocations (used to relocate an image).  */

static void
dump_ia64_vms_dynamic_relocs (FILE *file, struct ia64_vms_dynimgrela *imgrela)
{
  Elf64_External_VMS_IMAGE_RELA *imrs;
  long i;

  imrs = get_data (NULL, file, dynamic_addr + imgrela->img_rela_off,
		   1, imgrela->img_rela_cnt * sizeof (*imrs),
		   _("dynamic section image relocations"));
  if (!imrs)
    return;

  printf (_("\nImage relocs\n"));
  printf
    (_("Seg Offset   Type                            Addend            Seg Sym Off\n"));

  for (i = 0; i < (long) imgrela->img_rela_cnt; i++)
    {
      unsigned int type;
      const char *rtype;

      printf ("%3u ", (unsigned) BYTE_GET (imrs [i].rela_seg));
      printf ("%08" BFD_VMA_FMT "x ",
              (bfd_vma) BYTE_GET (imrs [i].rela_offset));
      type = BYTE_GET (imrs [i].type);
      rtype = elf_ia64_reloc_type (type);
      if (rtype == NULL)
        printf ("0x%08x                      ", type);
      else
        printf ("%-31s ", rtype);
      print_vma (BYTE_GET (imrs [i].addend), FULL_HEX);
      printf ("%3u ", (unsigned) BYTE_GET (imrs [i].sym_seg));
      printf ("%08" BFD_VMA_FMT "x\n",
              (bfd_vma) BYTE_GET (imrs [i].sym_offset));
    }

  free (imrs);
}

/* Display IA-64 OpenVMS dynamic relocations and fixups.  */

static int
process_ia64_vms_dynamic_relocs (FILE *file)
{
  struct ia64_vms_dynfixup fixup;
  struct ia64_vms_dynimgrela imgrela;
  Elf_Internal_Dyn *entry;
  int res = 0;
  bfd_vma strtab_off = 0;
  bfd_vma strtab_sz = 0;
  char *strtab = NULL;

  memset (&fixup, 0, sizeof (fixup));
  memset (&imgrela, 0, sizeof (imgrela));

  /* Note: the order of the entries is specified by the OpenVMS specs.  */
  for (entry = dynamic_section;
       entry < dynamic_section + dynamic_nent;
       entry++)
    {
      switch (entry->d_tag)
        {
        case DT_IA_64_VMS_STRTAB_OFFSET:
          strtab_off = entry->d_un.d_val;
          break;
        case DT_STRSZ:
          strtab_sz = entry->d_un.d_val;
          if (strtab == NULL)
            strtab = get_data (NULL, file, dynamic_addr + strtab_off,
                               1, strtab_sz, _("dynamic string section"));
          break;

        case DT_IA_64_VMS_NEEDED_IDENT:
          fixup.needed_ident = entry->d_un.d_val;
          break;
        case DT_NEEDED:
          fixup.needed = entry->d_un.d_val;
          break;
        case DT_IA_64_VMS_FIXUP_NEEDED:
          fixup.fixup_needed = entry->d_un.d_val;
          break;
        case DT_IA_64_VMS_FIXUP_RELA_CNT:
          fixup.fixup_rela_cnt = entry->d_un.d_val;
          break;
        case DT_IA_64_VMS_FIXUP_RELA_OFF:
          fixup.fixup_rela_off = entry->d_un.d_val;
          res++;
          dump_ia64_vms_dynamic_fixups (file, &fixup, strtab, strtab_sz);
          break;

        case DT_IA_64_VMS_IMG_RELA_CNT:
	  imgrela.img_rela_cnt = entry->d_un.d_val;
          break;
        case DT_IA_64_VMS_IMG_RELA_OFF:
	  imgrela.img_rela_off = entry->d_un.d_val;
          res++;
          dump_ia64_vms_dynamic_relocs (file, &imgrela);
          break;

        default:
          break;
	}
    }

  if (strtab != NULL)
    free (strtab);

  return res;
}

static struct
{
  const char * name;
  int reloc;
  int size;
  int rela;
} dynamic_relocations [] =
{
    { "REL", DT_REL, DT_RELSZ, FALSE },
    { "RELA", DT_RELA, DT_RELASZ, TRUE },
    { "PLT", DT_JMPREL, DT_PLTRELSZ, UNKNOWN }
};

/* Process the reloc section.  */

static int
process_relocs (FILE * file)
{
  unsigned long rel_size;
  unsigned long rel_offset;


  if (!do_reloc)
    return 1;

  if (do_using_dynamic)
    {
      int is_rela;
      const char * name;
      int has_dynamic_reloc;
      unsigned int i;

      has_dynamic_reloc = 0;

      for (i = 0; i < ARRAY_SIZE (dynamic_relocations); i++)
	{
	  is_rela = dynamic_relocations [i].rela;
	  name = dynamic_relocations [i].name;
	  rel_size = dynamic_info [dynamic_relocations [i].size];
	  rel_offset = dynamic_info [dynamic_relocations [i].reloc];

	  has_dynamic_reloc |= rel_size;

	  if (is_rela == UNKNOWN)
	    {
	      if (dynamic_relocations [i].reloc == DT_JMPREL)
		switch (dynamic_info[DT_PLTREL])
		  {
		  case DT_REL:
		    is_rela = FALSE;
		    break;
		  case DT_RELA:
		    is_rela = TRUE;
		    break;
		  }
	    }

	  if (rel_size)
	    {
	      printf
		(_("\n'%s' relocation section at offset 0x%lx contains %ld bytes:\n"),
		 name, rel_offset, rel_size);

	      dump_relocations (file,
				offset_from_vma (file, rel_offset, rel_size),
				rel_size,
				dynamic_symbols, num_dynamic_syms,
				dynamic_strings, dynamic_strings_length, is_rela);
	    }
	}

      if (is_ia64_vms ())
        has_dynamic_reloc |= process_ia64_vms_dynamic_relocs (file);

      if (! has_dynamic_reloc)
	printf (_("\nThere are no dynamic relocations in this file.\n"));
    }
  else
    {
      Elf_Internal_Shdr * section;
      unsigned long i;
      int found = 0;

      for (i = 0, section = section_headers;
	   i < elf_header.e_shnum;
	   i++, section++)
	{
	  if (   section->sh_type != SHT_RELA
	      && section->sh_type != SHT_REL)
	    continue;

	  rel_offset = section->sh_offset;
	  rel_size   = section->sh_size;

	  if (rel_size)
	    {
	      Elf_Internal_Shdr * strsec;
	      int is_rela;

	      printf (_("\nRelocation section "));

	      if (string_table == NULL)
		printf ("%d", section->sh_name);
	      else
		printf ("'%s'", SECTION_NAME (section));

	      printf (_(" at offset 0x%lx contains %lu entries:\n"),
		 rel_offset, (unsigned long) (rel_size / section->sh_entsize));

	      is_rela = section->sh_type == SHT_RELA;

	      if (section->sh_link != 0
		  && section->sh_link < elf_header.e_shnum)
		{
		  Elf_Internal_Shdr * symsec;
		  Elf_Internal_Sym *  symtab;
		  unsigned long nsyms;
		  unsigned long strtablen = 0;
		  char * strtab = NULL;

		  symsec = section_headers + section->sh_link;
		  if (symsec->sh_type != SHT_SYMTAB
		      && symsec->sh_type != SHT_DYNSYM)
                    continue;

		  symtab = GET_ELF_SYMBOLS (file, symsec, & nsyms);

		  if (symtab == NULL)
		    continue;

		  if (symsec->sh_link != 0
		      && symsec->sh_link < elf_header.e_shnum)
		    {
		      strsec = section_headers + symsec->sh_link;

		      strtab = (char *) get_data (NULL, file, strsec->sh_offset,
                                                  1, strsec->sh_size,
                                                  _("string table"));
		      strtablen = strtab == NULL ? 0 : strsec->sh_size;
		    }

		  dump_relocations (file, rel_offset, rel_size,
				    symtab, nsyms, strtab, strtablen, is_rela);
		  if (strtab)
		    free (strtab);
		  free (symtab);
		}
	      else
		dump_relocations (file, rel_offset, rel_size,
				  NULL, 0, NULL, 0, is_rela);

	      found = 1;
	    }
	}

      if (! found)
	printf (_("\nThere are no relocations in this file.\n"));
    }

  return 1;
}

/* Process the unwind section.  */

#include "unwind-ia64.h"

/* An absolute address consists of a section and an offset.  If the
   section is NULL, the offset itself is the address, otherwise, the
   address equals to LOAD_ADDRESS(section) + offset.  */

struct absaddr
  {
    unsigned short section;
    bfd_vma offset;
  };

#define ABSADDR(a) \
  ((a).section \
   ? section_headers [(a).section].sh_addr + (a).offset \
   : (a).offset)

struct ia64_unw_table_entry
  {
    struct absaddr start;
    struct absaddr end;
    struct absaddr info;
  };

struct ia64_unw_aux_info
  {

    struct ia64_unw_table_entry *table;	/* Unwind table.  */
    unsigned long table_len;	/* Length of unwind table.  */
    unsigned char * info;	/* Unwind info.  */
    unsigned long info_size;	/* Size of unwind info.  */
    bfd_vma info_addr;		/* starting address of unwind info.  */
    bfd_vma seg_base;		/* Starting address of segment.  */
    Elf_Internal_Sym * symtab;	/* The symbol table.  */
    unsigned long nsyms;	/* Number of symbols.  */
    char * strtab;		/* The string table.  */
    unsigned long strtab_size;	/* Size of string table.  */
  };

static void
find_symbol_for_address (Elf_Internal_Sym * symtab,
			 unsigned long nsyms,
			 const char * strtab,
			 unsigned long strtab_size,
			 struct absaddr addr,
			 const char ** symname,
			 bfd_vma * offset)
{
  bfd_vma dist = 0x100000;
  Elf_Internal_Sym * sym;
  Elf_Internal_Sym * best = NULL;
  unsigned long i;

  REMOVE_ARCH_BITS (addr.offset);

  for (i = 0, sym = symtab; i < nsyms; ++i, ++sym)
    {
      bfd_vma value = sym->st_value;

      REMOVE_ARCH_BITS (value);

      if (ELF_ST_TYPE (sym->st_info) == STT_FUNC
	  && sym->st_name != 0
	  && (addr.section == SHN_UNDEF || addr.section == sym->st_shndx)
	  && addr.offset >= value
	  && addr.offset - value < dist)
	{
	  best = sym;
	  dist = addr.offset - value;
	  if (!dist)
	    break;
	}
    }

  if (best)
    {
      *symname = (best->st_name >= strtab_size
		  ? _("<corrupt>") : strtab + best->st_name);
      *offset = dist;
      return;
    }

  *symname = NULL;
  *offset = addr.offset;
}

static void
dump_ia64_unwind (struct ia64_unw_aux_info * aux)
{
  struct ia64_unw_table_entry * tp;
  int in_body;

  for (tp = aux->table; tp < aux->table + aux->table_len; ++tp)
    {
      bfd_vma stamp;
      bfd_vma offset;
      const unsigned char * dp;
      const unsigned char * head;
      const char * procname;

      find_symbol_for_address (aux->symtab, aux->nsyms, aux->strtab,
			       aux->strtab_size, tp->start, &procname, &offset);

      fputs ("\n<", stdout);

      if (procname)
	{
	  fputs (procname, stdout);

	  if (offset)
	    printf ("+%lx", (unsigned long) offset);
	}

      fputs (">: [", stdout);
      print_vma (tp->start.offset, PREFIX_HEX);
      fputc ('-', stdout);
      print_vma (tp->end.offset, PREFIX_HEX);
      printf ("], info at +0x%lx\n",
	      (unsigned long) (tp->info.offset - aux->seg_base));

      head = aux->info + (ABSADDR (tp->info) - aux->info_addr);
      stamp = byte_get ((unsigned char *) head, sizeof (stamp));

      printf ("  v%u, flags=0x%lx (%s%s), len=%lu bytes\n",
	      (unsigned) UNW_VER (stamp),
	      (unsigned long) ((stamp & UNW_FLAG_MASK) >> 32),
	      UNW_FLAG_EHANDLER (stamp) ? " ehandler" : "",
	      UNW_FLAG_UHANDLER (stamp) ? " uhandler" : "",
	      (unsigned long) (eh_addr_size * UNW_LENGTH (stamp)));

      if (UNW_VER (stamp) != 1)
	{
	  printf (_("\tUnknown version.\n"));
	  continue;
	}

      in_body = 0;
      for (dp = head + 8; dp < head + 8 + eh_addr_size * UNW_LENGTH (stamp);)
	dp = unw_decode (dp, in_body, & in_body);
    }
}

static int
slurp_ia64_unwind_table (FILE * file,
			 struct ia64_unw_aux_info * aux,
			 Elf_Internal_Shdr * sec)
{
  unsigned long size, nrelas, i;
  Elf_Internal_Phdr * seg;
  struct ia64_unw_table_entry * tep;
  Elf_Internal_Shdr * relsec;
  Elf_Internal_Rela * rela;
  Elf_Internal_Rela * rp;
  unsigned char * table;
  unsigned char * tp;
  Elf_Internal_Sym * sym;
  const char * relname;

  /* First, find the starting address of the segment that includes
     this section: */

  if (elf_header.e_phnum)
    {
      if (! get_program_headers (file))
	  return 0;

      for (seg = program_headers;
	   seg < program_headers + elf_header.e_phnum;
	   ++seg)
	{
	  if (seg->p_type != PT_LOAD)
	    continue;

	  if (sec->sh_addr >= seg->p_vaddr
	      && (sec->sh_addr + sec->sh_size <= seg->p_vaddr + seg->p_memsz))
	    {
	      aux->seg_base = seg->p_vaddr;
	      break;
	    }
	}
    }

  /* Second, build the unwind table from the contents of the unwind section:  */
  size = sec->sh_size;
  table = (unsigned char *) get_data (NULL, file, sec->sh_offset, 1, size,
                                      _("unwind table"));
  if (!table)
    return 0;

  aux->table = (struct ia64_unw_table_entry *)
      xcmalloc (size / (3 * eh_addr_size), sizeof (aux->table[0]));
  tep = aux->table;
  for (tp = table; tp < table + size; ++tep)
    {
      tep->start.section = SHN_UNDEF;
      tep->end.section   = SHN_UNDEF;
      tep->info.section  = SHN_UNDEF;
      tep->start.offset = byte_get (tp, eh_addr_size); tp += eh_addr_size;
      tep->end.offset   = byte_get (tp, eh_addr_size); tp += eh_addr_size;
      tep->info.offset  = byte_get (tp, eh_addr_size); tp += eh_addr_size;
      tep->start.offset += aux->seg_base;
      tep->end.offset   += aux->seg_base;
      tep->info.offset  += aux->seg_base;
    }
  free (table);

  /* Third, apply any relocations to the unwind table:  */
  for (relsec = section_headers;
       relsec < section_headers + elf_header.e_shnum;
       ++relsec)
    {
      if (relsec->sh_type != SHT_RELA
	  || relsec->sh_info >= elf_header.e_shnum
	  || section_headers + relsec->sh_info != sec)
	continue;

      if (!slurp_rela_relocs (file, relsec->sh_offset, relsec->sh_size,
			      & rela, & nrelas))
	return 0;

      for (rp = rela; rp < rela + nrelas; ++rp)
	{
	  relname = elf_ia64_reloc_type (get_reloc_type (rp->r_info));
	  sym = aux->symtab + get_reloc_symindex (rp->r_info);

	  if (! const_strneq (relname, "R_IA64_SEGREL"))
	    {
	      warn (_("Skipping unexpected relocation type %s\n"), relname);
	      continue;
	    }

	  i = rp->r_offset / (3 * eh_addr_size);

	  switch (rp->r_offset/eh_addr_size % 3)
	    {
	    case 0:
	      aux->table[i].start.section = sym->st_shndx;
	      aux->table[i].start.offset  = rp->r_addend + sym->st_value;
	      break;
	    case 1:
	      aux->table[i].end.section   = sym->st_shndx;
	      aux->table[i].end.offset    = rp->r_addend + sym->st_value;
	      break;
	    case 2:
	      aux->table[i].info.section  = sym->st_shndx;
	      aux->table[i].info.offset   = rp->r_addend + sym->st_value;
	      break;
	    default:
	      break;
	    }
	}

      free (rela);
    }

  aux->table_len = size / (3 * eh_addr_size);
  return 1;
}

static void
ia64_process_unwind (FILE * file)
{
  Elf_Internal_Shdr * sec;
  Elf_Internal_Shdr * unwsec = NULL;
  Elf_Internal_Shdr * strsec;
  unsigned long i, unwcount = 0, unwstart = 0;
  struct ia64_unw_aux_info aux;

  memset (& aux, 0, sizeof (aux));

  for (i = 0, sec = section_headers; i < elf_header.e_shnum; ++i, ++sec)
    {
      if (sec->sh_type == SHT_SYMTAB
	  && sec->sh_link < elf_header.e_shnum)
	{
	  aux.symtab = GET_ELF_SYMBOLS (file, sec, & aux.nsyms);

	  strsec = section_headers + sec->sh_link;
	  assert (aux.strtab == NULL);
	  aux.strtab = (char *) get_data (NULL, file, strsec->sh_offset,
                                          1, strsec->sh_size,
                                          _("string table"));
	  aux.strtab_size = aux.strtab != NULL ? strsec->sh_size : 0;
	}
      else if (sec->sh_type == SHT_IA_64_UNWIND)
	unwcount++;
    }

  if (!unwcount)
    printf (_("\nThere are no unwind sections in this file.\n"));

  while (unwcount-- > 0)
    {
      char * suffix;
      size_t len, len2;

      for (i = unwstart, sec = section_headers + unwstart;
	   i < elf_header.e_shnum; ++i, ++sec)
	if (sec->sh_type == SHT_IA_64_UNWIND)
	  {
	    unwsec = sec;
	    break;
	  }

      unwstart = i + 1;
      len = sizeof (ELF_STRING_ia64_unwind_once) - 1;

      if ((unwsec->sh_flags & SHF_GROUP) != 0)
	{
	  /* We need to find which section group it is in.  */
	  struct group_list * g = section_headers_groups [i]->root;

	  for (; g != NULL; g = g->next)
	    {
	      sec = section_headers + g->section_index;

	      if (streq (SECTION_NAME (sec), ELF_STRING_ia64_unwind_info))
		break;
	    }

	  if (g == NULL)
	    i = elf_header.e_shnum;
	}
      else if (strneq (SECTION_NAME (unwsec), ELF_STRING_ia64_unwind_once, len))
	{
	  /* .gnu.linkonce.ia64unw.FOO -> .gnu.linkonce.ia64unwi.FOO.  */
	  len2 = sizeof (ELF_STRING_ia64_unwind_info_once) - 1;
	  suffix = SECTION_NAME (unwsec) + len;
	  for (i = 0, sec = section_headers; i < elf_header.e_shnum;
	       ++i, ++sec)
	    if (strneq (SECTION_NAME (sec), ELF_STRING_ia64_unwind_info_once, len2)
		&& streq (SECTION_NAME (sec) + len2, suffix))
	      break;
	}
      else
	{
	  /* .IA_64.unwindFOO -> .IA_64.unwind_infoFOO
	     .IA_64.unwind or BAR -> .IA_64.unwind_info.  */
	  len = sizeof (ELF_STRING_ia64_unwind) - 1;
	  len2 = sizeof (ELF_STRING_ia64_unwind_info) - 1;
	  suffix = "";
	  if (strneq (SECTION_NAME (unwsec), ELF_STRING_ia64_unwind, len))
	    suffix = SECTION_NAME (unwsec) + len;
	  for (i = 0, sec = section_headers; i < elf_header.e_shnum;
	       ++i, ++sec)
	    if (strneq (SECTION_NAME (sec), ELF_STRING_ia64_unwind_info, len2)
		&& streq (SECTION_NAME (sec) + len2, suffix))
	      break;
	}

      if (i == elf_header.e_shnum)
	{
	  printf (_("\nCould not find unwind info section for "));

	  if (string_table == NULL)
	    printf ("%d", unwsec->sh_name);
	  else
	    printf (_("'%s'"), SECTION_NAME (unwsec));
	}
      else
	{
	  aux.info_addr = sec->sh_addr;
	  aux.info = (unsigned char *) get_data (NULL, file, sec->sh_offset, 1,
                                                 sec->sh_size,
                                                 _("unwind info"));
	  aux.info_size = aux.info == NULL ? 0 : sec->sh_size;

	  printf (_("\nUnwind section "));

	  if (string_table == NULL)
	    printf ("%d", unwsec->sh_name);
	  else
	    printf (_("'%s'"), SECTION_NAME (unwsec));

	  printf (_(" at offset 0x%lx contains %lu entries:\n"),
		  (unsigned long) unwsec->sh_offset,
		  (unsigned long) (unwsec->sh_size / (3 * eh_addr_size)));

	  (void) slurp_ia64_unwind_table (file, & aux, unwsec);

	  if (aux.table_len > 0)
	    dump_ia64_unwind (& aux);

	  if (aux.table)
	    free ((char *) aux.table);
	  if (aux.info)
	    free ((char *) aux.info);
	  aux.table = NULL;
	  aux.info = NULL;
	}
    }

  if (aux.symtab)
    free (aux.symtab);
  if (aux.strtab)
    free ((char *) aux.strtab);
}

struct hppa_unw_table_entry
  {
    struct absaddr start;
    struct absaddr end;
    unsigned int Cannot_unwind:1;			/* 0 */
    unsigned int Millicode:1;			/* 1 */
    unsigned int Millicode_save_sr0:1;		/* 2 */
    unsigned int Region_description:2;		/* 3..4 */
    unsigned int reserved1:1;			/* 5 */
    unsigned int Entry_SR:1;			/* 6 */
    unsigned int Entry_FR:4;     /* number saved */	/* 7..10 */
    unsigned int Entry_GR:5;     /* number saved */	/* 11..15 */
    unsigned int Args_stored:1;			/* 16 */
    unsigned int Variable_Frame:1;			/* 17 */
    unsigned int Separate_Package_Body:1;		/* 18 */
    unsigned int Frame_Extension_Millicode:1;	/* 19 */
    unsigned int Stack_Overflow_Check:1;		/* 20 */
    unsigned int Two_Instruction_SP_Increment:1;	/* 21 */
    unsigned int Ada_Region:1;			/* 22 */
    unsigned int cxx_info:1;			/* 23 */
    unsigned int cxx_try_catch:1;			/* 24 */
    unsigned int sched_entry_seq:1;			/* 25 */
    unsigned int reserved2:1;			/* 26 */
    unsigned int Save_SP:1;				/* 27 */
    unsigned int Save_RP:1;				/* 28 */
    unsigned int Save_MRP_in_frame:1;		/* 29 */
    unsigned int extn_ptr_defined:1;		/* 30 */
    unsigned int Cleanup_defined:1;			/* 31 */

    unsigned int MPE_XL_interrupt_marker:1;		/* 0 */
    unsigned int HP_UX_interrupt_marker:1;		/* 1 */
    unsigned int Large_frame:1;			/* 2 */
    unsigned int Pseudo_SP_Set:1;			/* 3 */
    unsigned int reserved4:1;			/* 4 */
    unsigned int Total_frame_size:27;		/* 5..31 */
  };

struct hppa_unw_aux_info
  {
    struct hppa_unw_table_entry *table;	/* Unwind table.  */
    unsigned long table_len;	/* Length of unwind table.  */
    bfd_vma seg_base;		/* Starting address of segment.  */
    Elf_Internal_Sym * symtab;	/* The symbol table.  */
    unsigned long nsyms;	/* Number of symbols.  */
    char * strtab;		/* The string table.  */
    unsigned long strtab_size;	/* Size of string table.  */
  };

static void
dump_hppa_unwind (struct hppa_unw_aux_info * aux)
{
  struct hppa_unw_table_entry * tp;

  for (tp = aux->table; tp < aux->table + aux->table_len; ++tp)
    {
      bfd_vma offset;
      const char * procname;

      find_symbol_for_address (aux->symtab, aux->nsyms, aux->strtab,
			       aux->strtab_size, tp->start, &procname,
			       &offset);

      fputs ("\n<", stdout);

      if (procname)
	{
	  fputs (procname, stdout);

	  if (offset)
	    printf ("+%lx", (unsigned long) offset);
	}

      fputs (">: [", stdout);
      print_vma (tp->start.offset, PREFIX_HEX);
      fputc ('-', stdout);
      print_vma (tp->end.offset, PREFIX_HEX);
      printf ("]\n\t");

#define PF(_m) if (tp->_m) printf (#_m " ");
#define PV(_m) if (tp->_m) printf (#_m "=%d ", tp->_m);
      PF(Cannot_unwind);
      PF(Millicode);
      PF(Millicode_save_sr0);
      /* PV(Region_description);  */
      PF(Entry_SR);
      PV(Entry_FR);
      PV(Entry_GR);
      PF(Args_stored);
      PF(Variable_Frame);
      PF(Separate_Package_Body);
      PF(Frame_Extension_Millicode);
      PF(Stack_Overflow_Check);
      PF(Two_Instruction_SP_Increment);
      PF(Ada_Region);
      PF(cxx_info);
      PF(cxx_try_catch);
      PF(sched_entry_seq);
      PF(Save_SP);
      PF(Save_RP);
      PF(Save_MRP_in_frame);
      PF(extn_ptr_defined);
      PF(Cleanup_defined);
      PF(MPE_XL_interrupt_marker);
      PF(HP_UX_interrupt_marker);
      PF(Large_frame);
      PF(Pseudo_SP_Set);
      PV(Total_frame_size);
#undef PF
#undef PV
    }

  printf ("\n");
}

static int
slurp_hppa_unwind_table (FILE * file,
			 struct hppa_unw_aux_info * aux,
			 Elf_Internal_Shdr * sec)
{
  unsigned long size, unw_ent_size, nentries, nrelas, i;
  Elf_Internal_Phdr * seg;
  struct hppa_unw_table_entry * tep;
  Elf_Internal_Shdr * relsec;
  Elf_Internal_Rela * rela;
  Elf_Internal_Rela * rp;
  unsigned char * table;
  unsigned char * tp;
  Elf_Internal_Sym * sym;
  const char * relname;

  /* First, find the starting address of the segment that includes
     this section.  */

  if (elf_header.e_phnum)
    {
      if (! get_program_headers (file))
	return 0;

      for (seg = program_headers;
	   seg < program_headers + elf_header.e_phnum;
	   ++seg)
	{
	  if (seg->p_type != PT_LOAD)
	    continue;

	  if (sec->sh_addr >= seg->p_vaddr
	      && (sec->sh_addr + sec->sh_size <= seg->p_vaddr + seg->p_memsz))
	    {
	      aux->seg_base = seg->p_vaddr;
	      break;
	    }
	}
    }

  /* Second, build the unwind table from the contents of the unwind
     section.  */
  size = sec->sh_size;
  table = (unsigned char *) get_data (NULL, file, sec->sh_offset, 1, size,
                                      _("unwind table"));
  if (!table)
    return 0;

  unw_ent_size = 16;
  nentries = size / unw_ent_size;
  size = unw_ent_size * nentries;

  tep = aux->table = (struct hppa_unw_table_entry *)
      xcmalloc (nentries, sizeof (aux->table[0]));

  for (tp = table; tp < table + size; tp += unw_ent_size, ++tep)
    {
      unsigned int tmp1, tmp2;

      tep->start.section = SHN_UNDEF;
      tep->end.section   = SHN_UNDEF;

      tep->start.offset = byte_get ((unsigned char *) tp + 0, 4);
      tep->end.offset = byte_get ((unsigned char *) tp + 4, 4);
      tmp1 = byte_get ((unsigned char *) tp + 8, 4);
      tmp2 = byte_get ((unsigned char *) tp + 12, 4);

      tep->start.offset += aux->seg_base;
      tep->end.offset   += aux->seg_base;

      tep->Cannot_unwind = (tmp1 >> 31) & 0x1;
      tep->Millicode = (tmp1 >> 30) & 0x1;
      tep->Millicode_save_sr0 = (tmp1 >> 29) & 0x1;
      tep->Region_description = (tmp1 >> 27) & 0x3;
      tep->reserved1 = (tmp1 >> 26) & 0x1;
      tep->Entry_SR = (tmp1 >> 25) & 0x1;
      tep->Entry_FR = (tmp1 >> 21) & 0xf;
      tep->Entry_GR = (tmp1 >> 16) & 0x1f;
      tep->Args_stored = (tmp1 >> 15) & 0x1;
      tep->Variable_Frame = (tmp1 >> 14) & 0x1;
      tep->Separate_Package_Body = (tmp1 >> 13) & 0x1;
      tep->Frame_Extension_Millicode = (tmp1 >> 12) & 0x1;
      tep->Stack_Overflow_Check = (tmp1 >> 11) & 0x1;
      tep->Two_Instruction_SP_Increment = (tmp1 >> 10) & 0x1;
      tep->Ada_Region = (tmp1 >> 9) & 0x1;
      tep->cxx_info = (tmp1 >> 8) & 0x1;
      tep->cxx_try_catch = (tmp1 >> 7) & 0x1;
      tep->sched_entry_seq = (tmp1 >> 6) & 0x1;
      tep->reserved2 = (tmp1 >> 5) & 0x1;
      tep->Save_SP = (tmp1 >> 4) & 0x1;
      tep->Save_RP = (tmp1 >> 3) & 0x1;
      tep->Save_MRP_in_frame = (tmp1 >> 2) & 0x1;
      tep->extn_ptr_defined = (tmp1 >> 1) & 0x1;
      tep->Cleanup_defined = tmp1 & 0x1;

      tep->MPE_XL_interrupt_marker = (tmp2 >> 31) & 0x1;
      tep->HP_UX_interrupt_marker = (tmp2 >> 30) & 0x1;
      tep->Large_frame = (tmp2 >> 29) & 0x1;
      tep->Pseudo_SP_Set = (tmp2 >> 28) & 0x1;
      tep->reserved4 = (tmp2 >> 27) & 0x1;
      tep->Total_frame_size = tmp2 & 0x7ffffff;
    }
  free (table);

  /* Third, apply any relocations to the unwind table.  */
  for (relsec = section_headers;
       relsec < section_headers + elf_header.e_shnum;
       ++relsec)
    {
      if (relsec->sh_type != SHT_RELA
	  || relsec->sh_info >= elf_header.e_shnum
	  || section_headers + relsec->sh_info != sec)
	continue;

      if (!slurp_rela_relocs (file, relsec->sh_offset, relsec->sh_size,
			      & rela, & nrelas))
	return 0;

      for (rp = rela; rp < rela + nrelas; ++rp)
	{
	  relname = elf_hppa_reloc_type (get_reloc_type (rp->r_info));
	  sym = aux->symtab + get_reloc_symindex (rp->r_info);

	  /* R_PARISC_SEGREL32 or R_PARISC_SEGREL64.  */
	  if (! const_strneq (relname, "R_PARISC_SEGREL"))
	    {
	      warn (_("Skipping unexpected relocation type %s\n"), relname);
	      continue;
	    }

	  i = rp->r_offset / unw_ent_size;

	  switch ((rp->r_offset % unw_ent_size) / eh_addr_size)
	    {
	    case 0:
	      aux->table[i].start.section = sym->st_shndx;
	      aux->table[i].start.offset  = sym->st_value + rp->r_addend;
	      break;
	    case 1:
	      aux->table[i].end.section   = sym->st_shndx;
	      aux->table[i].end.offset    = sym->st_value + rp->r_addend;
	      break;
	    default:
	      break;
	    }
	}

      free (rela);
    }

  aux->table_len = nentries;

  return 1;
}

static void
hppa_process_unwind (FILE * file)
{
  struct hppa_unw_aux_info aux;
  Elf_Internal_Shdr * unwsec = NULL;
  Elf_Internal_Shdr * strsec;
  Elf_Internal_Shdr * sec;
  unsigned long i;

  if (string_table == NULL)
    return;

  memset (& aux, 0, sizeof (aux));

  for (i = 0, sec = section_headers; i < elf_header.e_shnum; ++i, ++sec)
    {
      if (sec->sh_type == SHT_SYMTAB
	  && sec->sh_link < elf_header.e_shnum)
	{
	  aux.symtab = GET_ELF_SYMBOLS (file, sec, & aux.nsyms);

	  strsec = section_headers + sec->sh_link;
	  assert (aux.strtab == NULL);
	  aux.strtab = (char *) get_data (NULL, file, strsec->sh_offset,
                                          1, strsec->sh_size,
                                          _("string table"));
	  aux.strtab_size = aux.strtab != NULL ? strsec->sh_size : 0;
	}
      else if (streq (SECTION_NAME (sec), ".PARISC.unwind"))
	unwsec = sec;
    }

  if (!unwsec)
    printf (_("\nThere are no unwind sections in this file.\n"));

  for (i = 0, sec = section_headers; i < elf_header.e_shnum; ++i, ++sec)
    {
      if (streq (SECTION_NAME (sec), ".PARISC.unwind"))
	{
	  printf (_("\nUnwind section "));
	  printf (_("'%s'"), SECTION_NAME (sec));

	  printf (_(" at offset 0x%lx contains %lu entries:\n"),
		  (unsigned long) sec->sh_offset,
		  (unsigned long) (sec->sh_size / (2 * eh_addr_size + 8)));

          slurp_hppa_unwind_table (file, &aux, sec);
	  if (aux.table_len > 0)
	    dump_hppa_unwind (&aux);

	  if (aux.table)
	    free ((char *) aux.table);
	  aux.table = NULL;
	}
    }

  if (aux.symtab)
    free (aux.symtab);
  if (aux.strtab)
    free ((char *) aux.strtab);
}

struct arm_section
{
  unsigned char *      data;		/* The unwind data.  */
  Elf_Internal_Shdr *  sec;		/* The cached unwind section header.  */
  Elf_Internal_Rela *  rela;		/* The cached relocations for this section.  */
  unsigned long        nrelas;		/* The number of relocations.  */
  unsigned int         rel_type;	/* REL or RELA ?  */
  Elf_Internal_Rela *  next_rela;	/* Cyclic pointer to the next reloc to process.  */
};

struct arm_unw_aux_info
{
  FILE *              file;		/* The file containing the unwind sections.  */
  Elf_Internal_Sym *  symtab;		/* The file's symbol table.  */
  unsigned long       nsyms;		/* Number of symbols.  */
  char *              strtab;		/* The file's string table.  */
  unsigned long       strtab_size;	/* Size of string table.  */
};

static const char *
arm_print_vma_and_name (struct arm_unw_aux_info *aux,
			bfd_vma fn, struct absaddr addr)
{
  const char *procname;
  bfd_vma sym_offset;

  if (addr.section == SHN_UNDEF)
    addr.offset = fn;

  find_symbol_for_address (aux->symtab, aux->nsyms, aux->strtab,
			   aux->strtab_size, addr, &procname,
			   &sym_offset);

  print_vma (fn, PREFIX_HEX);

  if (procname)
    {
      fputs (" <", stdout);
      fputs (procname, stdout);

      if (sym_offset)
	printf ("+0x%lx", (unsigned long) sym_offset);
      fputc ('>', stdout);
    }

  return procname;
}

static void
arm_free_section (struct arm_section *arm_sec)
{
  if (arm_sec->data != NULL)
    free (arm_sec->data);

  if (arm_sec->rela != NULL)
    free (arm_sec->rela);
}

/* 1) If SEC does not match the one cached in ARM_SEC, then free the current
      cached section and install SEC instead.
   2) Locate the 32-bit word at WORD_OFFSET in unwind section SEC
      and return its valued in * WORDP, relocating if necessary.
   3) Update the NEXT_RELA field in ARM_SEC and store the section index and
      relocation's offset in ADDR.
   4) If SYM_NAME is non-NULL and a relocation was applied, record the offset
      into the string table of the symbol associated with the reloc.  If no
      reloc was applied store -1 there.
   5) Return TRUE upon success, FALSE otherwise.  */

static bfd_boolean
get_unwind_section_word (struct arm_unw_aux_info *  aux,
			 struct arm_section *       arm_sec,
			 Elf_Internal_Shdr *        sec,
			 bfd_vma 		    word_offset,
			 unsigned int *             wordp,
			 struct absaddr *           addr,
			 bfd_vma *		    sym_name)
{
  Elf_Internal_Rela *rp;
  Elf_Internal_Sym *sym;
  const char * relname;
  unsigned int word;
  bfd_boolean wrapped;

  addr->section = SHN_UNDEF;
  addr->offset = 0;

  if (sym_name != NULL)
    *sym_name = (bfd_vma) -1;

  /* If necessary, update the section cache.  */
  if (sec != arm_sec->sec)
    {
      Elf_Internal_Shdr *relsec;

      arm_free_section (arm_sec);

      arm_sec->sec = sec;
      arm_sec->data = get_data (NULL, aux->file, sec->sh_offset, 1,
				sec->sh_size, _("unwind data"));
      arm_sec->rela = NULL;
      arm_sec->nrelas = 0;

      for (relsec = section_headers;
	   relsec < section_headers + elf_header.e_shnum;
	   ++relsec)
	{
	  if (relsec->sh_info >= elf_header.e_shnum
	      || section_headers + relsec->sh_info != sec)
	    continue;

	  arm_sec->rel_type = relsec->sh_type;
	  if (relsec->sh_type == SHT_REL)
	    {
	      if (!slurp_rel_relocs (aux->file, relsec->sh_offset,
				     relsec->sh_size,
				     & arm_sec->rela, & arm_sec->nrelas))
		return FALSE;
	      break;
	    }
	  else if (relsec->sh_type == SHT_RELA)
	    {
	      if (!slurp_rela_relocs (aux->file, relsec->sh_offset,
				      relsec->sh_size,
				      & arm_sec->rela, & arm_sec->nrelas))
		return FALSE;
	      break;
	    }
	  else
	    warn (_("unexpected relocation type (%d) for section %d"),
		  relsec->sh_type, relsec->sh_info);
	}

      arm_sec->next_rela = arm_sec->rela;
    }

  /* If there is no unwind data we can do nothing.  */
  if (arm_sec->data == NULL)
    return FALSE;

  /* Get the word at the required offset.  */
  word = byte_get (arm_sec->data + word_offset, 4);

  /* Look through the relocs to find the one that applies to the provided offset.  */
  wrapped = FALSE;
  for (rp = arm_sec->next_rela; rp != arm_sec->rela + arm_sec->nrelas; rp++)
    {
      bfd_vma prelval, offset;

      if (rp->r_offset > word_offset && !wrapped)
	{
	  rp = arm_sec->rela;
	  wrapped = TRUE;
	}
      if (rp->r_offset > word_offset)
	break;

      if (rp->r_offset & 3)
	{
	  warn (_("Skipping unexpected relocation at offset 0x%lx\n"),
		(unsigned long) rp->r_offset);
	  continue;
	}

      if (rp->r_offset < word_offset)
	continue;

      sym = aux->symtab + ELF32_R_SYM (rp->r_info);

      if (arm_sec->rel_type == SHT_REL)
	{
	  offset = word & 0x7fffffff;
	  if (offset & 0x40000000)
	    offset |= ~ (bfd_vma) 0x7fffffff;
	}
      else if (arm_sec->rel_type == SHT_RELA)
	offset = rp->r_addend;
      else
	abort ();

      offset += sym->st_value;
      prelval = offset - (arm_sec->sec->sh_addr + rp->r_offset);

      /* Check that we are processing the expected reloc type.  */
      if (elf_header.e_machine == EM_ARM)
	{
	  relname = elf_arm_reloc_type (ELF32_R_TYPE (rp->r_info));

	  if (streq (relname, "R_ARM_NONE"))
	      continue;
	  
	  if (! streq (relname, "R_ARM_PREL31"))
	    {
	      warn (_("Skipping unexpected relocation type %s\n"), relname);
	      continue;
	    }
	}
      else if (elf_header.e_machine == EM_TI_C6000)
	{
	  relname = elf_tic6x_reloc_type (ELF32_R_TYPE (rp->r_info));
	  
	  if (streq (relname, "R_C6000_NONE"))
	    continue;

	  if (! streq (relname, "R_C6000_PREL31"))
	    {
	      warn (_("Skipping unexpected relocation type %s\n"), relname);
	      continue;
	    }

	  prelval >>= 1;
	}
      else
	/* This function currently only supports ARM and TI unwinders.  */
	abort ();

      word = (word & ~ (bfd_vma) 0x7fffffff) | (prelval & 0x7fffffff);
      addr->section = sym->st_shndx;
      addr->offset = offset;
      if (sym_name)
	* sym_name = sym->st_name;
      break;
    }

  *wordp = word;
  arm_sec->next_rela = rp;

  return TRUE;
}

static const char *tic6x_unwind_regnames[16] =
{
  "A15", "B15", "B14", "B13", "B12", "B11", "B10", "B3", 
  "A14", "A13", "A12", "A11", "A10", 
  "[invalid reg 13]", "[invalid reg 14]", "[invalid reg 15]"
};

static void
decode_tic6x_unwind_regmask (unsigned int mask)
{
  int i;

  for (i = 12; mask; mask >>= 1, i--)
    {
      if (mask & 1)
	{
	  fputs (tic6x_unwind_regnames[i], stdout);
	  if (mask > 1)
	    fputs (", ", stdout);
	}
    }
}

#define ADVANCE							\
  if (remaining == 0 && more_words)				\
    {								\
      data_offset += 4;						\
      if (! get_unwind_section_word (aux, data_arm_sec, data_sec,	\
				     data_offset, & word, & addr, NULL))	\
	return;							\
      remaining = 4;						\
      more_words--;						\
    }								\

#define GET_OP(OP)			\
  ADVANCE;				\
  if (remaining)			\
    {					\
      remaining--;			\
      (OP) = word >> 24;		\
      word <<= 8;			\
    }					\
  else					\
    {					\
      printf (_("[Truncated opcode]\n"));	\
      return;				\
    }					\
  printf ("0x%02x ", OP)

static void
decode_arm_unwind_bytecode (struct arm_unw_aux_info *aux,
			    unsigned int word, unsigned int remaining,
			    unsigned int more_words,
			    bfd_vma data_offset, Elf_Internal_Shdr *data_sec,
			    struct arm_section *data_arm_sec)
{
  struct absaddr addr;

  /* Decode the unwinding instructions.  */
  while (1)
    {
      unsigned int op, op2;

      ADVANCE;
      if (remaining == 0)
	break;
      remaining--;
      op = word >> 24;
      word <<= 8;

      printf ("  0x%02x ", op);

      if ((op & 0xc0) == 0x00)
	{
	  int offset = ((op & 0x3f) << 2) + 4;

	  printf ("     vsp = vsp + %d", offset);
	}
      else if ((op & 0xc0) == 0x40)
	{
	  int offset = ((op & 0x3f) << 2) + 4;

	  printf ("     vsp = vsp - %d", offset);
	}
      else if ((op & 0xf0) == 0x80)
	{
	  GET_OP (op2);
	  if (op == 0x80 && op2 == 0)
	    printf (_("Refuse to unwind"));
	  else
	    {
	      unsigned int mask = ((op & 0x0f) << 8) | op2;
	      int first = 1;
	      int i;

	      printf ("pop {");
	      for (i = 0; i < 12; i++)
		if (mask & (1 << i))
		  {
		    if (first)
		      first = 0;
		    else
		      printf (", ");
		    printf ("r%d", 4 + i);
		  }
	      printf ("}");
	    }
	}
      else if ((op & 0xf0) == 0x90)
	{
	  if (op == 0x9d || op == 0x9f)
	    printf (_("     [Reserved]"));
	  else
	    printf ("     vsp = r%d", op & 0x0f);
	}
      else if ((op & 0xf0) == 0xa0)
	{
	  int end = 4 + (op & 0x07);
	  int first = 1;
	  int i;

	  printf ("     pop {");
	  for (i = 4; i <= end; i++)
	    {
	      if (first)
		first = 0;
	      else
		printf (", ");
	      printf ("r%d", i);
	    }
	  if (op & 0x08)
	    {
	      if (!first)
		printf (", ");
	      printf ("r14");
	    }
	  printf ("}");
	}
      else if (op == 0xb0)
	printf (_("     finish"));
      else if (op == 0xb1)
	{
	  GET_OP (op2);
	  if (op2 == 0 || (op2 & 0xf0) != 0)
	    printf (_("[Spare]"));
	  else
	    {
	      unsigned int mask = op2 & 0x0f;
	      int first = 1;
	      int i;

	      printf ("pop {");
	      for (i = 0; i < 12; i++)
		if (mask & (1 << i))
		  {
		    if (first)
		      first = 0;
		    else
		      printf (", ");
		    printf ("r%d", i);
		  }
	      printf ("}");
	    }
	}
      else if (op == 0xb2)
	{
	  unsigned char buf[9];
	  unsigned int i, len;
	  unsigned long offset;

	  for (i = 0; i < sizeof (buf); i++)
	    {
	      GET_OP (buf[i]);
	      if ((buf[i] & 0x80) == 0)
		break;
	    }
	  assert (i < sizeof (buf));
	  offset = read_uleb128 (buf, &len);
	  assert (len == i + 1);
	  offset = offset * 4 + 0x204;
	  printf ("vsp = vsp + %ld", offset);
	}
      else if (op == 0xb3 || op == 0xc8 || op == 0xc9)
	{
	  unsigned int first, last;

	  GET_OP (op2);
	  first = op2 >> 4;
	  last = op2 & 0x0f;
	  if (op == 0xc8)
	    first = first + 16;
	  printf ("pop {D%d", first);
	  if (last)
	    printf ("-D%d", first + last);
	  printf ("}");
	}
      else if ((op & 0xf8) == 0xb8 || (op & 0xf8) == 0xd0)
	{
	  unsigned int count = op & 0x07;

	  printf ("pop {D8");
	  if (count)
	    printf ("-D%d", 8 + count);
	  printf ("}");
	}
      else if (op >= 0xc0 && op <= 0xc5)
	{
	  unsigned int count = op & 0x07;

	  printf ("     pop {wR10");
	  if (count)
	    printf ("-wR%d", 10 + count);
	  printf ("}");
	}
      else if (op == 0xc6)
	{
	  unsigned int first, last;

	  GET_OP (op2);
	  first = op2 >> 4;
	  last = op2 & 0x0f;
	  printf ("pop {wR%d", first);
	  if (last)
	    printf ("-wR%d", first + last);
	  printf ("}");
	}
      else if (op == 0xc7)
	{
	  GET_OP (op2);
	  if (op2 == 0 || (op2 & 0xf0) != 0)
	    printf (_("[Spare]"));
	  else
	    {
	      unsigned int mask = op2 & 0x0f;
	      int first = 1;
	      int i;

	      printf ("pop {");
	      for (i = 0; i < 4; i++)
		if (mask & (1 << i))
		  {
		    if (first)
		      first = 0;
		    else
		      printf (", ");
		    printf ("wCGR%d", i);
		  }
	      printf ("}");
	    }
	}
      else
	printf (_("     [unsupported opcode]"));
      printf ("\n");
    }
}

static void
decode_tic6x_unwind_bytecode (struct arm_unw_aux_info *aux,
			    unsigned int word, unsigned int remaining,
			    unsigned int more_words,
			    bfd_vma data_offset, Elf_Internal_Shdr *data_sec,
			    struct arm_section *data_arm_sec)
{
  struct absaddr addr;

  /* Decode the unwinding instructions.  */
  while (1)
    {
      unsigned int op, op2;

      ADVANCE;
      if (remaining == 0)
	break;
      remaining--;
      op = word >> 24;
      word <<= 8;

      printf ("  0x%02x ", op);

      if ((op & 0xc0) == 0x00)
	{
	  int offset = ((op & 0x3f) << 3) + 8;
	  printf ("     sp = sp + %d", offset);
	}
      else if ((op & 0xc0) == 0x80)
	{
	  GET_OP (op2);
	  if (op == 0x80 && op2 == 0)
	    printf (_("Refuse to unwind"));
	  else
	    {
	      unsigned int mask = ((op & 0x1f) << 8) | op2;
	      if (op & 0x20)
		printf ("pop compact {");
	      else
		printf ("pop {");

	      decode_tic6x_unwind_regmask (mask);
	      printf("}");
	    }
	}
      else if ((op & 0xf0) == 0xc0)
	{
	  unsigned int reg;
	  unsigned int nregs;
	  unsigned int i;
	  const char *name;
	  struct
	  {
	      unsigned int offset;
	      unsigned int reg;
	  } regpos[16];

	  /* Scan entire instruction first so that GET_OP output is not
	     interleaved with disassembly.  */
	  nregs = 0;
	  for (i = 0; nregs < (op & 0xf); i++)
	    {
	      GET_OP (op2);
	      reg = op2 >> 4;
	      if (reg != 0xf)
		{
		  regpos[nregs].offset = i * 2;
		  regpos[nregs].reg = reg;
		  nregs++;
		}

	      reg = op2 & 0xf;
	      if (reg != 0xf)
		{
		  regpos[nregs].offset = i * 2 + 1;
		  regpos[nregs].reg = reg;
		  nregs++;
		}
	    }

	  printf (_("pop frame {"));
	  reg = nregs - 1;
	  for (i = i * 2; i > 0; i--)
	    {
	      if (regpos[reg].offset == i - 1)
		{
		  name = tic6x_unwind_regnames[regpos[reg].reg];
		  if (reg > 0)
		    reg--;
		}
	      else
		name = _("[pad]");

	      fputs (name, stdout);
	      if (i > 1)
		printf (", ");
	    }

	  printf ("}");
	}
      else if (op == 0xd0)
	printf ("     MOV FP, SP");
      else if (op == 0xd1)
	printf ("     __c6xabi_pop_rts");
      else if (op == 0xd2)
	{
	  unsigned char buf[9];
	  unsigned int i, len;
	  unsigned long offset;

	  for (i = 0; i < sizeof (buf); i++)
	    {
	      GET_OP (buf[i]);
	      if ((buf[i] & 0x80) == 0)
		break;
	    }
	  assert (i < sizeof (buf));
	  offset = read_uleb128 (buf, &len);
	  assert (len == i + 1);
	  offset = offset * 8 + 0x408;
	  printf (_("sp = sp + %ld"), offset);
	}
      else if ((op & 0xf0) == 0xe0)
	{
	  if ((op & 0x0f) == 7)
	    printf ("     RETURN");
	  else
	    printf ("     MV %s, B3", tic6x_unwind_regnames[op & 0x0f]);
	}
      else
	{
	  printf (_("     [unsupported opcode]"));
	}
      putchar ('\n');
    }
}

static bfd_vma
arm_expand_prel31 (bfd_vma word, bfd_vma where)
{
  bfd_vma offset;

  offset = word & 0x7fffffff;
  if (offset & 0x40000000)
    offset |= ~ (bfd_vma) 0x7fffffff;

  if (elf_header.e_machine == EM_TI_C6000)
    offset <<= 1;

  return offset + where;
}

static void
decode_arm_unwind (struct arm_unw_aux_info *  aux,
		   unsigned int               word,
		   unsigned int               remaining,
		   bfd_vma                    data_offset,
		   Elf_Internal_Shdr *        data_sec,
		   struct arm_section *       data_arm_sec)
{
  int per_index;
  unsigned int more_words = 0;
  struct absaddr addr;
  bfd_vma sym_name = (bfd_vma) -1;

  if (remaining == 0)
    {
      /* Fetch the first word.
	 Note - when decoding an object file the address extracted
	 here will always be 0.  So we also pass in the sym_name
	 parameter so that we can find the symbol associated with
	 the personality routine.  */
      if (! get_unwind_section_word (aux, data_arm_sec, data_sec, data_offset,
				     & word, & addr, & sym_name))
	return;

      remaining = 4;
    }

  if ((word & 0x80000000) == 0)
    {
      /* Expand prel31 for personality routine.  */
      bfd_vma fn;
      const char *procname;

      fn = arm_expand_prel31 (word, data_sec->sh_addr + data_offset);
      printf (_("  Personality routine: "));
      if (fn == 0
	  && addr.section == SHN_UNDEF && addr.offset == 0
	  && sym_name != (bfd_vma) -1 && sym_name < aux->strtab_size)
	{
	  procname = aux->strtab + sym_name;
	  print_vma (fn, PREFIX_HEX);
	  if (procname)
	    {
	      fputs (" <", stdout);
	      fputs (procname, stdout);
	      fputc ('>', stdout);
	    }
	}
      else
	procname = arm_print_vma_and_name (aux, fn, addr);
      fputc ('\n', stdout);

      /* The GCC personality routines use the standard compact
	 encoding, starting with one byte giving the number of
	 words.  */
      if (procname != NULL
	  && (const_strneq (procname, "__gcc_personality_v0")
	      || const_strneq (procname, "__gxx_personality_v0")
	      || const_strneq (procname, "__gcj_personality_v0")
	      || const_strneq (procname, "__gnu_objc_personality_v0")))
	{
	  remaining = 0;
	  more_words = 1;
	  ADVANCE;
	  if (!remaining)
	    {
	      printf (_("  [Truncated data]\n"));
	      return;
	    }
	  more_words = word >> 24;
	  word <<= 8;
	  remaining--;
	  per_index = -1;
	}
      else
	return;
    }
  else
    {
      /* ARM EHABI Section 6.3:
	 
	 An exception-handling table entry for the compact model looks like:
	 
           31 30-28 27-24 23-0
	   -- ----- ----- ----
            1   0   index Data for personalityRoutine[index]    */

      if (elf_header.e_machine == EM_ARM
	  && (word & 0x70000000))
	warn (_("Corrupt ARM compact model table entry: %x \n"), word);

      per_index = (word >> 24) & 0x7f;
      printf (_("  Compact model index: %d\n"), per_index);
      if (per_index == 0)
	{
	  more_words = 0;
	  word <<= 8;
	  remaining--;
	}
      else if (per_index < 3)
	{
	  more_words = (word >> 16) & 0xff;
	  word <<= 16;
	  remaining -= 2;
	}
    }

  switch (elf_header.e_machine)
    {
    case EM_ARM:
      if (per_index < 3)
	{
	  decode_arm_unwind_bytecode (aux, word, remaining, more_words,
				      data_offset, data_sec, data_arm_sec);
	}
      else
	{
	  warn (_("Unknown ARM compact model index encountered\n"));
	  printf (_("  [reserved]\n"));
	}
      break;

    case EM_TI_C6000:
      if (per_index < 3)
	{
	  decode_tic6x_unwind_bytecode (aux, word, remaining, more_words,
					data_offset, data_sec, data_arm_sec);
	}
      else if (per_index < 5)
	{
	  if (((word >> 17) & 0x7f) == 0x7f)
	    printf (_("  Restore stack from frame pointer\n"));
	  else
	    printf (_("  Stack increment %d\n"), (word >> 14) & 0x1fc);
	  printf (_("  Registers restored: "));
	  if (per_index == 4)
	    printf (" (compact) ");
	  decode_tic6x_unwind_regmask ((word >> 4) & 0x1fff);
	  putchar ('\n');
	  printf (_("  Return register: %s\n"),
		  tic6x_unwind_regnames[word & 0xf]);
	}
      else
	printf (_("  [reserved (%d)]\n"), per_index);
      break;

    default:
      error (_("Unsupported architecture type %d encountered when decoding unwind table"),
	     elf_header.e_machine);
    }

  /* Decode the descriptors.  Not implemented.  */
}

static void
dump_arm_unwind (struct arm_unw_aux_info *aux, Elf_Internal_Shdr *exidx_sec)
{
  struct arm_section exidx_arm_sec, extab_arm_sec;
  unsigned int i, exidx_len;

  memset (&exidx_arm_sec, 0, sizeof (exidx_arm_sec));
  memset (&extab_arm_sec, 0, sizeof (extab_arm_sec));
  exidx_len = exidx_sec->sh_size / 8;

  for (i = 0; i < exidx_len; i++)
    {
      unsigned int exidx_fn, exidx_entry;
      struct absaddr fn_addr, entry_addr;
      bfd_vma fn;

      fputc ('\n', stdout);

      if (! get_unwind_section_word (aux, & exidx_arm_sec, exidx_sec,
				     8 * i, & exidx_fn, & fn_addr, NULL)
	  || ! get_unwind_section_word (aux, & exidx_arm_sec, exidx_sec,
					8 * i + 4, & exidx_entry, & entry_addr, NULL))
	{
	  arm_free_section (& exidx_arm_sec);
	  arm_free_section (& extab_arm_sec);
	  return;
	}

      /* ARM EHABI, Section 5:
	 An index table entry consists of 2 words.
         The first word contains a prel31 offset to the start of a function, with bit 31 clear.  */
      if (exidx_fn & 0x80000000)
	warn (_("corrupt index table entry: %x\n"), exidx_fn);

      fn = arm_expand_prel31 (exidx_fn, exidx_sec->sh_addr + 8 * i);

      arm_print_vma_and_name (aux, fn, fn_addr);
      fputs (": ", stdout);

      if (exidx_entry == 1)
	{
	  print_vma (exidx_entry, PREFIX_HEX);
	  fputs (" [cantunwind]\n", stdout);
	}
      else if (exidx_entry & 0x80000000)
	{
	  print_vma (exidx_entry, PREFIX_HEX);
	  fputc ('\n', stdout);
	  decode_arm_unwind (aux, exidx_entry, 4, 0, NULL, NULL);
	}
      else
	{
	  bfd_vma table, table_offset = 0;
	  Elf_Internal_Shdr *table_sec;

	  fputs ("@", stdout);
	  table = arm_expand_prel31 (exidx_entry, exidx_sec->sh_addr + 8 * i + 4);
	  print_vma (table, PREFIX_HEX);
	  printf ("\n");

	  /* Locate the matching .ARM.extab.  */
	  if (entry_addr.section != SHN_UNDEF
	      && entry_addr.section < elf_header.e_shnum)
	    {
	      table_sec = section_headers + entry_addr.section;
	      table_offset = entry_addr.offset;
	    }
	  else
	    {
	      table_sec = find_section_by_address (table);
	      if (table_sec != NULL)
		table_offset = table - table_sec->sh_addr;
	    }
	  if (table_sec == NULL)
	    {
	      warn (_("Could not locate .ARM.extab section containing 0x%lx.\n"),
		    (unsigned long) table);
	      continue;
	    }
	  decode_arm_unwind (aux, 0, 0, table_offset, table_sec,
			     &extab_arm_sec);
	}
    }

  printf ("\n");

  arm_free_section (&exidx_arm_sec);
  arm_free_section (&extab_arm_sec);
}

/* Used for both ARM and C6X unwinding tables.  */

static void
arm_process_unwind (FILE *file)
{
  struct arm_unw_aux_info aux;
  Elf_Internal_Shdr *unwsec = NULL;
  Elf_Internal_Shdr *strsec;
  Elf_Internal_Shdr *sec;
  unsigned long i;
  unsigned int sec_type;

  switch (elf_header.e_machine)
    {
    case EM_ARM:
      sec_type = SHT_ARM_EXIDX;
      break;

    case EM_TI_C6000:
      sec_type = SHT_C6000_UNWIND;
      break;

    default: 
      error (_("Unsupported architecture type %d encountered when processing unwind table"),
	     elf_header.e_machine);
      return;
    }

  if (string_table == NULL)
    return;

  memset (& aux, 0, sizeof (aux));
  aux.file = file;

  for (i = 0, sec = section_headers; i < elf_header.e_shnum; ++i, ++sec)
    {
      if (sec->sh_type == SHT_SYMTAB && sec->sh_link < elf_header.e_shnum)
	{
	  aux.symtab = GET_ELF_SYMBOLS (file, sec, & aux.nsyms);

	  strsec = section_headers + sec->sh_link;
	  assert (aux.strtab == NULL);
	  aux.strtab = get_data (NULL, file, strsec->sh_offset,
				 1, strsec->sh_size, _("string table"));
	  aux.strtab_size = aux.strtab != NULL ? strsec->sh_size : 0;
	}
      else if (sec->sh_type == sec_type)
	unwsec = sec;
    }

  if (unwsec == NULL)
    printf (_("\nThere are no unwind sections in this file.\n"));
  else
    for (i = 0, sec = section_headers; i < elf_header.e_shnum; ++i, ++sec)
      {
	if (sec->sh_type == sec_type)
	  {
	    printf (_("\nUnwind table index '%s' at offset 0x%lx contains %lu entries:\n"),
		    SECTION_NAME (sec),
		    (unsigned long) sec->sh_offset,
		    (unsigned long) (sec->sh_size / (2 * eh_addr_size)));

	    dump_arm_unwind (&aux, sec);
	  }
      }

  if (aux.symtab)
    free (aux.symtab);
  if (aux.strtab)
    free ((char *) aux.strtab);
}

static void
process_unwind (FILE * file)
{
  struct unwind_handler
  {
    int machtype;
    void (* handler)(FILE *);
  } handlers[] =
  {
    { EM_ARM, arm_process_unwind },
    { EM_IA_64, ia64_process_unwind },
    { EM_PARISC, hppa_process_unwind },
    { EM_TI_C6000, arm_process_unwind },
    { 0, 0 }
  };
  int i;

  if (!do_unwind)
    return;

  for (i = 0; handlers[i].handler != NULL; i++)
    if (elf_header.e_machine == handlers[i].machtype)
      return handlers[i].handler (file);

  printf (_("\nThe decoding of unwind sections for machine type %s is not currently supported.\n"),
	  get_machine_name (elf_header.e_machine));
}

static void
dynamic_section_mips_val (Elf_Internal_Dyn * entry)
{
  switch (entry->d_tag)
    {
    case DT_MIPS_FLAGS:
      if (entry->d_un.d_val == 0)
	printf (_("NONE"));
      else
	{
	  static const char * opts[] =
	  {
	    "QUICKSTART", "NOTPOT", "NO_LIBRARY_REPLACEMENT",
	    "NO_MOVE", "SGI_ONLY", "GUARANTEE_INIT", "DELTA_C_PLUS_PLUS",
	    "GUARANTEE_START_INIT", "PIXIE", "DEFAULT_DELAY_LOAD",
	    "REQUICKSTART", "REQUICKSTARTED", "CORD", "NO_UNRES_UNDEF",
	    "RLD_ORDER_SAFE"
	  };
	  unsigned int cnt;
	  int first = 1;

	  for (cnt = 0; cnt < ARRAY_SIZE (opts); ++cnt)
	    if (entry->d_un.d_val & (1 << cnt))
	      {
		printf ("%s%s", first ? "" : " ", opts[cnt]);
		first = 0;
	      }
	}
      break;

    case DT_MIPS_IVERSION:
      if (VALID_DYNAMIC_NAME (entry->d_un.d_val))
	printf (_("Interface Version: %s"), GET_DYNAMIC_NAME (entry->d_un.d_val));
      else
	printf (_("<corrupt: %" BFD_VMA_FMT "d>"), entry->d_un.d_ptr);
      break;

    case DT_MIPS_TIME_STAMP:
      {
	char timebuf[20];
	struct tm * tmp;

	time_t atime = entry->d_un.d_val;
	tmp = gmtime (&atime);
	snprintf (timebuf, sizeof (timebuf), "%04u-%02u-%02uT%02u:%02u:%02u",
		  tmp->tm_year + 1900, tmp->tm_mon + 1, tmp->tm_mday,
		  tmp->tm_hour, tmp->tm_min, tmp->tm_sec);
	printf (_("Time Stamp: %s"), timebuf);
      }
      break;

    case DT_MIPS_RLD_VERSION:
    case DT_MIPS_LOCAL_GOTNO:
    case DT_MIPS_CONFLICTNO:
    case DT_MIPS_LIBLISTNO:
    case DT_MIPS_SYMTABNO:
    case DT_MIPS_UNREFEXTNO:
    case DT_MIPS_HIPAGENO:
    case DT_MIPS_DELTA_CLASS_NO:
    case DT_MIPS_DELTA_INSTANCE_NO:
    case DT_MIPS_DELTA_RELOC_NO:
    case DT_MIPS_DELTA_SYM_NO:
    case DT_MIPS_DELTA_CLASSSYM_NO:
    case DT_MIPS_COMPACT_SIZE:
      print_vma (entry->d_un.d_ptr, DEC);
      break;

    default:
      print_vma (entry->d_un.d_ptr, PREFIX_HEX);
    }
    putchar ('\n');
}

static void
dynamic_section_parisc_val (Elf_Internal_Dyn * entry)
{
  switch (entry->d_tag)
    {
    case DT_HP_DLD_FLAGS:
      {
	static struct
	{
	  long int bit;
	  const char * str;
	}
	flags[] =
	{
	  { DT_HP_DEBUG_PRIVATE, "HP_DEBUG_PRIVATE" },
	  { DT_HP_DEBUG_CALLBACK, "HP_DEBUG_CALLBACK" },
	  { DT_HP_DEBUG_CALLBACK_BOR, "HP_DEBUG_CALLBACK_BOR" },
	  { DT_HP_NO_ENVVAR, "HP_NO_ENVVAR" },
	  { DT_HP_BIND_NOW, "HP_BIND_NOW" },
	  { DT_HP_BIND_NONFATAL, "HP_BIND_NONFATAL" },
	  { DT_HP_BIND_VERBOSE, "HP_BIND_VERBOSE" },
	  { DT_HP_BIND_RESTRICTED, "HP_BIND_RESTRICTED" },
	  { DT_HP_BIND_SYMBOLIC, "HP_BIND_SYMBOLIC" },
	  { DT_HP_RPATH_FIRST, "HP_RPATH_FIRST" },
	  { DT_HP_BIND_DEPTH_FIRST, "HP_BIND_DEPTH_FIRST" },
	  { DT_HP_GST, "HP_GST" },
	  { DT_HP_SHLIB_FIXED, "HP_SHLIB_FIXED" },
	  { DT_HP_MERGE_SHLIB_SEG, "HP_MERGE_SHLIB_SEG" },
	  { DT_HP_NODELETE, "HP_NODELETE" },
	  { DT_HP_GROUP, "HP_GROUP" },
	  { DT_HP_PROTECT_LINKAGE_TABLE, "HP_PROTECT_LINKAGE_TABLE" }
	};
	int first = 1;
	size_t cnt;
	bfd_vma val = entry->d_un.d_val;

	for (cnt = 0; cnt < ARRAY_SIZE (flags); ++cnt)
	  if (val & flags[cnt].bit)
	    {
	      if (! first)
		putchar (' ');
	      fputs (flags[cnt].str, stdout);
	      first = 0;
	      val ^= flags[cnt].bit;
	    }

	if (val != 0 || first)
	  {
	    if (! first)
	      putchar (' ');
	    print_vma (val, HEX);
	  }
      }
      break;

    default:
      print_vma (entry->d_un.d_ptr, PREFIX_HEX);
      break;
    }
  putchar ('\n');
}

#ifdef BFD64

/* VMS vs Unix time offset and factor.  */

#define VMS_EPOCH_OFFSET 35067168000000000LL
#define VMS_GRANULARITY_FACTOR 10000000

/* Display a VMS time in a human readable format.  */

static void
print_vms_time (bfd_int64_t vmstime)
{
  struct tm *tm;
  time_t unxtime;

  unxtime = (vmstime - VMS_EPOCH_OFFSET) / VMS_GRANULARITY_FACTOR;
  tm = gmtime (&unxtime);
  printf ("%04u-%02u-%02uT%02u:%02u:%02u",
          tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
          tm->tm_hour, tm->tm_min, tm->tm_sec);
}
#endif /* BFD64 */

static void
dynamic_section_ia64_val (Elf_Internal_Dyn * entry)
{
  switch (entry->d_tag)
    {
    case DT_IA_64_PLT_RESERVE:
      /* First 3 slots reserved.  */
      print_vma (entry->d_un.d_ptr, PREFIX_HEX);
      printf (" -- ");
      print_vma (entry->d_un.d_ptr + (3 * 8), PREFIX_HEX);
      break;

    case DT_IA_64_VMS_LINKTIME:
#ifdef BFD64
      print_vms_time (entry->d_un.d_val);
#endif
      break;

    case DT_IA_64_VMS_LNKFLAGS:
      print_vma (entry->d_un.d_ptr, PREFIX_HEX);
      if (entry->d_un.d_val & VMS_LF_CALL_DEBUG)
        printf (" CALL_DEBUG");
      if (entry->d_un.d_val & VMS_LF_NOP0BUFS)
        printf (" NOP0BUFS");
      if (entry->d_un.d_val & VMS_LF_P0IMAGE)
        printf (" P0IMAGE");
      if (entry->d_un.d_val & VMS_LF_MKTHREADS)
        printf (" MKTHREADS");
      if (entry->d_un.d_val & VMS_LF_UPCALLS)
        printf (" UPCALLS");
      if (entry->d_un.d_val & VMS_LF_IMGSTA)
        printf (" IMGSTA");
      if (entry->d_un.d_val & VMS_LF_INITIALIZE)
        printf (" INITIALIZE");
      if (entry->d_un.d_val & VMS_LF_MAIN)
        printf (" MAIN");
      if (entry->d_un.d_val & VMS_LF_EXE_INIT)
        printf (" EXE_INIT");
      if (entry->d_un.d_val & VMS_LF_TBK_IN_IMG)
        printf (" TBK_IN_IMG");
      if (entry->d_un.d_val & VMS_LF_DBG_IN_IMG)
        printf (" DBG_IN_IMG");
      if (entry->d_un.d_val & VMS_LF_TBK_IN_DSF)
        printf (" TBK_IN_DSF");
      if (entry->d_un.d_val & VMS_LF_DBG_IN_DSF)
        printf (" DBG_IN_DSF");
      if (entry->d_un.d_val & VMS_LF_SIGNATURES)
        printf (" SIGNATURES");
      if (entry->d_un.d_val & VMS_LF_REL_SEG_OFF)
        printf (" REL_SEG_OFF");
      break;

    default:
      print_vma (entry->d_un.d_ptr, PREFIX_HEX);
      break;
    }
  putchar ('\n');
}

static int
get_32bit_dynamic_section (FILE * file)
{
  Elf32_External_Dyn * edyn;
  Elf32_External_Dyn * ext;
  Elf_Internal_Dyn * entry;

  edyn = (Elf32_External_Dyn *) get_data (NULL, file, dynamic_addr, 1,
                                          dynamic_size, _("dynamic section"));
  if (!edyn)
    return 0;

/* SGI's ELF has more than one section in the DYNAMIC segment, and we
   might not have the luxury of section headers.  Look for the DT_NULL
   terminator to determine the number of entries.  */
  for (ext = edyn, dynamic_nent = 0;
       (char *) ext < (char *) edyn + dynamic_size;
       ext++)
    {
      dynamic_nent++;
      if (BYTE_GET (ext->d_tag) == DT_NULL)
	break;
    }

  dynamic_section = (Elf_Internal_Dyn *) cmalloc (dynamic_nent,
                                                  sizeof (* entry));
  if (dynamic_section == NULL)
    {
      error (_("Out of memory\n"));
      free (edyn);
      return 0;
    }

  for (ext = edyn, entry = dynamic_section;
       entry < dynamic_section + dynamic_nent;
       ext++, entry++)
    {
      entry->d_tag      = BYTE_GET (ext->d_tag);
      entry->d_un.d_val = BYTE_GET (ext->d_un.d_val);
    }

  free (edyn);

  return 1;
}

static int
get_64bit_dynamic_section (FILE * file)
{
  Elf64_External_Dyn * edyn;
  Elf64_External_Dyn * ext;
  Elf_Internal_Dyn * entry;

  edyn = (Elf64_External_Dyn *) get_data (NULL, file, dynamic_addr, 1,
                                          dynamic_size, _("dynamic section"));
  if (!edyn)
    return 0;

/* SGI's ELF has more than one section in the DYNAMIC segment, and we
   might not have the luxury of section headers.  Look for the DT_NULL
   terminator to determine the number of entries.  */
  for (ext = edyn, dynamic_nent = 0;
       (char *) ext < (char *) edyn + dynamic_size;
       ext++)
    {
      dynamic_nent++;
      if (BYTE_GET (ext->d_tag) == DT_NULL)
	break;
    }

  dynamic_section = (Elf_Internal_Dyn *) cmalloc (dynamic_nent,
                                                  sizeof (* entry));
  if (dynamic_section == NULL)
    {
      error (_("Out of memory\n"));
      free (edyn);
      return 0;
    }

  for (ext = edyn, entry = dynamic_section;
       entry < dynamic_section + dynamic_nent;
       ext++, entry++)
    {
      entry->d_tag      = BYTE_GET (ext->d_tag);
      entry->d_un.d_val = BYTE_GET (ext->d_un.d_val);
    }

  free (edyn);

  return 1;
}

static void
print_dynamic_flags (bfd_vma flags)
{
  int first = 1;

  while (flags)
    {
      bfd_vma flag;

      flag = flags & - flags;
      flags &= ~ flag;

      if (first)
	first = 0;
      else
	putc (' ', stdout);

      switch (flag)
	{
	case DF_ORIGIN:		fputs ("ORIGIN", stdout); break;
	case DF_SYMBOLIC:	fputs ("SYMBOLIC", stdout); break;
	case DF_TEXTREL:	fputs ("TEXTREL", stdout); break;
	case DF_BIND_NOW:	fputs ("BIND_NOW", stdout); break;
	case DF_STATIC_TLS:	fputs ("STATIC_TLS", stdout); break;
	default:		fputs (_("unknown"), stdout); break;
	}
    }
  puts ("");
}

/* Parse and display the contents of the dynamic section.  */

static int
process_dynamic_section (FILE * file)
{
  Elf_Internal_Dyn * entry;

  if (dynamic_size == 0)
    {
      if (do_dynamic)
	printf (_("\nThere is no dynamic section in this file.\n"));

      return 1;
    }

  if (is_32bit_elf)
    {
      if (! get_32bit_dynamic_section (file))
	return 0;
    }
  else if (! get_64bit_dynamic_section (file))
    return 0;

  /* Find the appropriate symbol table.  */
  if (dynamic_symbols == NULL)
    {
      for (entry = dynamic_section;
	   entry < dynamic_section + dynamic_nent;
	   ++entry)
	{
	  Elf_Internal_Shdr section;

	  if (entry->d_tag != DT_SYMTAB)
	    continue;

	  dynamic_info[DT_SYMTAB] = entry->d_un.d_val;

	  /* Since we do not know how big the symbol table is,
	     we default to reading in the entire file (!) and
	     processing that.  This is overkill, I know, but it
	     should work.  */
	  section.sh_offset = offset_from_vma (file, entry->d_un.d_val, 0);

	  if (archive_file_offset != 0)
	    section.sh_size = archive_file_size - section.sh_offset;
	  else
	    {
	      if (fseek (file, 0, SEEK_END))
		error (_("Unable to seek to end of file!\n"));

	      section.sh_size = ftell (file) - section.sh_offset;
	    }

	  if (is_32bit_elf)
	    section.sh_entsize = sizeof (Elf32_External_Sym);
	  else
	    section.sh_entsize = sizeof (Elf64_External_Sym);

	  dynamic_symbols = GET_ELF_SYMBOLS (file, &section, & num_dynamic_syms);
	  if (num_dynamic_syms < 1)
	    {
	      error (_("Unable to determine the number of symbols to load\n"));
	      continue;
	    }
	}
    }

  /* Similarly find a string table.  */
  if (dynamic_strings == NULL)
    {
      for (entry = dynamic_section;
	   entry < dynamic_section + dynamic_nent;
	   ++entry)
	{
	  unsigned long offset;
	  long str_tab_len;

	  if (entry->d_tag != DT_STRTAB)
	    continue;

	  dynamic_info[DT_STRTAB] = entry->d_un.d_val;

	  /* Since we do not know how big the string table is,
	     we default to reading in the entire file (!) and
	     processing that.  This is overkill, I know, but it
	     should work.  */

	  offset = offset_from_vma (file, entry->d_un.d_val, 0);

	  if (archive_file_offset != 0)
	    str_tab_len = archive_file_size - offset;
	  else
	    {
	      if (fseek (file, 0, SEEK_END))
		error (_("Unable to seek to end of file\n"));
	      str_tab_len = ftell (file) - offset;
	    }

	  if (str_tab_len < 1)
	    {
	      error
		(_("Unable to determine the length of the dynamic string table\n"));
	      continue;
	    }

	  dynamic_strings = (char *) get_data (NULL, file, offset, 1,
                                               str_tab_len,
                                               _("dynamic string table"));
	  dynamic_strings_length = dynamic_strings == NULL ? 0 : str_tab_len;
	  break;
	}
    }

  /* And find the syminfo section if available.  */
  if (dynamic_syminfo == NULL)
    {
      unsigned long syminsz = 0;

      for (entry = dynamic_section;
	   entry < dynamic_section + dynamic_nent;
	   ++entry)
	{
	  if (entry->d_tag == DT_SYMINENT)
	    {
	      /* Note: these braces are necessary to avoid a syntax
		 error from the SunOS4 C compiler.  */
	      assert (sizeof (Elf_External_Syminfo) == entry->d_un.d_val);
	    }
	  else if (entry->d_tag == DT_SYMINSZ)
	    syminsz = entry->d_un.d_val;
	  else if (entry->d_tag == DT_SYMINFO)
	    dynamic_syminfo_offset = offset_from_vma (file, entry->d_un.d_val,
						      syminsz);
	}

      if (dynamic_syminfo_offset != 0 && syminsz != 0)
	{
	  Elf_External_Syminfo * extsyminfo;
	  Elf_External_Syminfo * extsym;
	  Elf_Internal_Syminfo * syminfo;

	  /* There is a syminfo section.  Read the data.  */
	  extsyminfo = (Elf_External_Syminfo *)
              get_data (NULL, file, dynamic_syminfo_offset, 1, syminsz,
                        _("symbol information"));
	  if (!extsyminfo)
	    return 0;

	  dynamic_syminfo = (Elf_Internal_Syminfo *) malloc (syminsz);
	  if (dynamic_syminfo == NULL)
	    {
	      error (_("Out of memory\n"));
	      return 0;
	    }

	  dynamic_syminfo_nent = syminsz / sizeof (Elf_External_Syminfo);
	  for (syminfo = dynamic_syminfo, extsym = extsyminfo;
	       syminfo < dynamic_syminfo + dynamic_syminfo_nent;
	       ++syminfo, ++extsym)
	    {
	      syminfo->si_boundto = BYTE_GET (extsym->si_boundto);
	      syminfo->si_flags = BYTE_GET (extsym->si_flags);
	    }

	  free (extsyminfo);
	}
    }

  if (do_dynamic && dynamic_addr)
    printf (_("\nDynamic section at offset 0x%lx contains %u entries:\n"),
	    dynamic_addr, dynamic_nent);
  if (do_dynamic)
    printf (_("  Tag        Type                         Name/Value\n"));

  for (entry = dynamic_section;
       entry < dynamic_section + dynamic_nent;
       entry++)
    {
      if (do_dynamic)
	{
	  const char * dtype;

	  putchar (' ');
	  print_vma (entry->d_tag, FULL_HEX);
	  dtype = get_dynamic_type (entry->d_tag);
	  printf (" (%s)%*s", dtype,
		  ((is_32bit_elf ? 27 : 19)
		   - (int) strlen (dtype)),
		  " ");
	}

      switch (entry->d_tag)
	{
	case DT_FLAGS:
	  if (do_dynamic)
	    print_dynamic_flags (entry->d_un.d_val);
	  break;

	case DT_AUXILIARY:
	case DT_FILTER:
	case DT_CONFIG:
	case DT_DEPAUDIT:
	case DT_AUDIT:
	  if (do_dynamic)
	    {
	      switch (entry->d_tag)
		{
		case DT_AUXILIARY:
		  printf (_("Auxiliary library"));
		  break;

		case DT_FILTER:
		  printf (_("Filter library"));
		  break;

		case DT_CONFIG:
		  printf (_("Configuration file"));
		  break;

		case DT_DEPAUDIT:
		  printf (_("Dependency audit library"));
		  break;

		case DT_AUDIT:
		  printf (_("Audit library"));
		  break;
		}

	      if (VALID_DYNAMIC_NAME (entry->d_un.d_val))
		printf (": [%s]\n", GET_DYNAMIC_NAME (entry->d_un.d_val));
	      else
		{
		  printf (": ");
		  print_vma (entry->d_un.d_val, PREFIX_HEX);
		  putchar ('\n');
		}
	    }
	  break;

	case DT_FEATURE:
	  if (do_dynamic)
	    {
	      printf (_("Flags:"));

	      if (entry->d_un.d_val == 0)
		printf (_(" None\n"));
	      else
		{
		  unsigned long int val = entry->d_un.d_val;

		  if (val & DTF_1_PARINIT)
		    {
		      printf (" PARINIT");
		      val ^= DTF_1_PARINIT;
		    }
		  if (val & DTF_1_CONFEXP)
		    {
		      printf (" CONFEXP");
		      val ^= DTF_1_CONFEXP;
		    }
		  if (val != 0)
		    printf (" %lx", val);
		  puts ("");
		}
	    }
	  break;

	case DT_POSFLAG_1:
	  if (do_dynamic)
	    {
	      printf (_("Flags:"));

	      if (entry->d_un.d_val == 0)
		printf (_(" None\n"));
	      else
		{
		  unsigned long int val = entry->d_un.d_val;

		  if (val & DF_P1_LAZYLOAD)
		    {
		      printf (" LAZYLOAD");
		      val ^= DF_P1_LAZYLOAD;
		    }
		  if (val & DF_P1_GROUPPERM)
		    {
		      printf (" GROUPPERM");
		      val ^= DF_P1_GROUPPERM;
		    }
		  if (val != 0)
		    printf (" %lx", val);
		  puts ("");
		}
	    }
	  break;

	case DT_FLAGS_1:
	  if (do_dynamic)
	    {
	      printf (_("Flags:"));
	      if (entry->d_un.d_val == 0)
		printf (_(" None\n"));
	      else
		{
		  unsigned long int val = entry->d_un.d_val;

		  if (val & DF_1_NOW)
		    {
		      printf (" NOW");
		      val ^= DF_1_NOW;
		    }
		  if (val & DF_1_GLOBAL)
		    {
		      printf (" GLOBAL");
		      val ^= DF_1_GLOBAL;
		    }
		  if (val & DF_1_GROUP)
		    {
		      printf (" GROUP");
		      val ^= DF_1_GROUP;
		    }
		  if (val & DF_1_NODELETE)
		    {
		      printf (" NODELETE");
		      val ^= DF_1_NODELETE;
		    }
		  if (val & DF_1_LOADFLTR)
		    {
		      printf (" LOADFLTR");
		      val ^= DF_1_LOADFLTR;
		    }
		  if (val & DF_1_INITFIRST)
		    {
		      printf (" INITFIRST");
		      val ^= DF_1_INITFIRST;
		    }
		  if (val & DF_1_NOOPEN)
		    {
		      printf (" NOOPEN");
		      val ^= DF_1_NOOPEN;
		    }
		  if (val & DF_1_ORIGIN)
		    {
		      printf (" ORIGIN");
		      val ^= DF_1_ORIGIN;
		    }
		  if (val & DF_1_DIRECT)
		    {
		      printf (" DIRECT");
		      val ^= DF_1_DIRECT;
		    }
		  if (val & DF_1_TRANS)
		    {
		      printf (" TRANS");
		      val ^= DF_1_TRANS;
		    }
		  if (val & DF_1_INTERPOSE)
		    {
		      printf (" INTERPOSE");
		      val ^= DF_1_INTERPOSE;
		    }
		  if (val & DF_1_NODEFLIB)
		    {
		      printf (" NODEFLIB");
		      val ^= DF_1_NODEFLIB;
		    }
		  if (val & DF_1_NODUMP)
		    {
		      printf (" NODUMP");
		      val ^= DF_1_NODUMP;
		    }
		  if (val & DF_1_CONLFAT)
		    {
		      printf (" CONLFAT");
		      val ^= DF_1_CONLFAT;
		    }
		  if (val != 0)
		    printf (" %lx", val);
		  puts ("");
		}
	    }
	  break;

	case DT_PLTREL:
	  dynamic_info[entry->d_tag] = entry->d_un.d_val;
	  if (do_dynamic)
	    puts (get_dynamic_type (entry->d_un.d_val));
	  break;

	case DT_NULL	:
	case DT_NEEDED	:
	case DT_PLTGOT	:
	case DT_HASH	:
	case DT_STRTAB	:
	case DT_SYMTAB	:
	case DT_RELA	:
	case DT_INIT	:
	case DT_FINI	:
	case DT_SONAME	:
	case DT_RPATH	:
	case DT_SYMBOLIC:
	case DT_REL	:
	case DT_DEBUG	:
	case DT_TEXTREL	:
	case DT_JMPREL	:
	case DT_RUNPATH	:
	  dynamic_info[entry->d_tag] = entry->d_un.d_val;

	  if (do_dynamic)
	    {
	      char * name;

	      if (VALID_DYNAMIC_NAME (entry->d_un.d_val))
		name = GET_DYNAMIC_NAME (entry->d_un.d_val);
	      else
		name = NULL;

	      if (name)
		{
		  switch (entry->d_tag)
		    {
		    case DT_NEEDED:
		      printf (_("Shared library: [%s]"), name);

		      if (streq (name, program_interpreter))
			printf (_(" program interpreter"));
		      break;

		    case DT_SONAME:
		      printf (_("Library soname: [%s]"), name);
		      break;

		    case DT_RPATH:
		      printf (_("Library rpath: [%s]"), name);
		      break;

		    case DT_RUNPATH:
		      printf (_("Library runpath: [%s]"), name);
		      break;

		    default:
		      print_vma (entry->d_un.d_val, PREFIX_HEX);
		      break;
		    }
		}
	      else
		print_vma (entry->d_un.d_val, PREFIX_HEX);

	      putchar ('\n');
	    }
	  break;

	case DT_PLTRELSZ:
	case DT_RELASZ	:
	case DT_STRSZ	:
	case DT_RELSZ	:
	case DT_RELAENT	:
	case DT_SYMENT	:
	case DT_RELENT	:
	  dynamic_info[entry->d_tag] = entry->d_un.d_val;
	case DT_PLTPADSZ:
	case DT_MOVEENT	:
	case DT_MOVESZ	:
	case DT_INIT_ARRAYSZ:
	case DT_FINI_ARRAYSZ:
	case DT_GNU_CONFLICTSZ:
	case DT_GNU_LIBLISTSZ:
	  if (do_dynamic)
	    {
	      print_vma (entry->d_un.d_val, UNSIGNED);
	      printf (_(" (bytes)\n"));
	    }
	  break;

	case DT_VERDEFNUM:
	case DT_VERNEEDNUM:
	case DT_RELACOUNT:
	case DT_RELCOUNT:
	  if (do_dynamic)
	    {
	      print_vma (entry->d_un.d_val, UNSIGNED);
	      putchar ('\n');
	    }
	  break;

	case DT_SYMINSZ:
	case DT_SYMINENT:
	case DT_SYMINFO:
	case DT_USED:
	case DT_INIT_ARRAY:
	case DT_FINI_ARRAY:
	  if (do_dynamic)
	    {
	      if (entry->d_tag == DT_USED
		  && VALID_DYNAMIC_NAME (entry->d_un.d_val))
		{
		  char * name = GET_DYNAMIC_NAME (entry->d_un.d_val);

		  if (*name)
		    {
		      printf (_("Not needed object: [%s]\n"), name);
		      break;
		    }
		}

	      print_vma (entry->d_un.d_val, PREFIX_HEX);
	      putchar ('\n');
	    }
	  break;

	case DT_BIND_NOW:
	  /* The value of this entry is ignored.  */
	  if (do_dynamic)
	    putchar ('\n');
	  break;

	case DT_GNU_PRELINKED:
	  if (do_dynamic)
	    {
	      struct tm * tmp;
	      time_t atime = entry->d_un.d_val;

	      tmp = gmtime (&atime);
	      printf ("%04u-%02u-%02uT%02u:%02u:%02u\n",
		      tmp->tm_year + 1900, tmp->tm_mon + 1, tmp->tm_mday,
		      tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

	    }
	  break;

	case DT_GNU_HASH:
	  dynamic_info_DT_GNU_HASH = entry->d_un.d_val;
	  if (do_dynamic)
	    {
	      print_vma (entry->d_un.d_val, PREFIX_HEX);
	      putchar ('\n');
	    }
	  break;

	default:
	  if ((entry->d_tag >= DT_VERSYM) && (entry->d_tag <= DT_VERNEEDNUM))
	    version_info[DT_VERSIONTAGIDX (entry->d_tag)] =
	      entry->d_un.d_val;

	  if (do_dynamic)
	    {
	      switch (elf_header.e_machine)
		{
		case EM_MIPS:
		case EM_MIPS_RS3_LE:
		  dynamic_section_mips_val (entry);
		  break;
		case EM_PARISC:
		  dynamic_section_parisc_val (entry);
		  break;
		case EM_IA_64:
		  dynamic_section_ia64_val (entry);
		  break;
		default:
		  print_vma (entry->d_un.d_val, PREFIX_HEX);
		  putchar ('\n');
		}
	    }
	  break;
	}
    }

  return 1;
}

static char *
get_ver_flags (unsigned int flags)
{
  static char buff[32];

  buff[0] = 0;

  if (flags == 0)
    return _("none");

  if (flags & VER_FLG_BASE)
    strcat (buff, "BASE ");

  if (flags & VER_FLG_WEAK)
    {
      if (flags & VER_FLG_BASE)
	strcat (buff, "| ");

      strcat (buff, "WEAK ");
    }

  if (flags & VER_FLG_INFO)
    {
      if (flags & (VER_FLG_BASE|VER_FLG_WEAK))
	strcat (buff, "| ");

      strcat (buff, "INFO ");
    }

  if (flags & ~(VER_FLG_BASE | VER_FLG_WEAK | VER_FLG_INFO))
    strcat (buff, _("| <unknown>"));

  return buff;
}

/* Display the contents of the version sections.  */

static int
process_version_sections (FILE * file)
{
  Elf_Internal_Shdr * section;
  unsigned i;
  int found = 0;

  if (! do_version)
    return 1;

  for (i = 0, section = section_headers;
       i < elf_header.e_shnum;
       i++, section++)
    {
      switch (section->sh_type)
	{
	case SHT_GNU_verdef:
	  {
	    Elf_External_Verdef * edefs;
	    unsigned int idx;
	    unsigned int cnt;
	    char * endbuf;

	    found = 1;

	    printf
	      (_("\nVersion definition section '%s' contains %u entries:\n"),
	       SECTION_NAME (section), section->sh_info);

	    printf (_("  Addr: 0x"));
	    printf_vma (section->sh_addr);
	    printf (_("  Offset: %#08lx  Link: %u (%s)\n"),
		    (unsigned long) section->sh_offset, section->sh_link,
		    section->sh_link < elf_header.e_shnum
		    ? SECTION_NAME (section_headers + section->sh_link)
		    : _("<corrupt>"));

	    edefs = (Elf_External_Verdef *)
                get_data (NULL, file, section->sh_offset, 1,section->sh_size,
                          _("version definition section"));
	    if (!edefs)
	      break;
	    endbuf = (char *) edefs + section->sh_size;

	    for (idx = cnt = 0; cnt < section->sh_info; ++cnt)
	      {
		char * vstart;
		Elf_External_Verdef * edef;
		Elf_Internal_Verdef ent;
		Elf_External_Verdaux * eaux;
		Elf_Internal_Verdaux aux;
		int j;
		int isum;

		/* Check for negative or very large indicies.  */
		if ((unsigned char *) edefs + idx < (unsigned char *) edefs)
		  break;

		vstart = ((char *) edefs) + idx;
		if (vstart + sizeof (*edef) > endbuf)
		  break;

		edef = (Elf_External_Verdef *) vstart;

		ent.vd_version = BYTE_GET (edef->vd_version);
		ent.vd_flags   = BYTE_GET (edef->vd_flags);
		ent.vd_ndx     = BYTE_GET (edef->vd_ndx);
		ent.vd_cnt     = BYTE_GET (edef->vd_cnt);
		ent.vd_hash    = BYTE_GET (edef->vd_hash);
		ent.vd_aux     = BYTE_GET (edef->vd_aux);
		ent.vd_next    = BYTE_GET (edef->vd_next);

		printf (_("  %#06x: Rev: %d  Flags: %s"),
			idx, ent.vd_version, get_ver_flags (ent.vd_flags));

		printf (_("  Index: %d  Cnt: %d  "),
			ent.vd_ndx, ent.vd_cnt);

		/* Check for overflow.  */
		if ((unsigned char *)(vstart + ent.vd_aux) < (unsigned char *) vstart
		    || (unsigned char *)(vstart + ent.vd_aux) > (unsigned char *) endbuf)
		  break;

		vstart += ent.vd_aux;

		eaux = (Elf_External_Verdaux *) vstart;

		aux.vda_name = BYTE_GET (eaux->vda_name);
		aux.vda_next = BYTE_GET (eaux->vda_next);

		if (VALID_DYNAMIC_NAME (aux.vda_name))
		  printf (_("Name: %s\n"), GET_DYNAMIC_NAME (aux.vda_name));
		else
		  printf (_("Name index: %ld\n"), aux.vda_name);

		isum = idx + ent.vd_aux;

		for (j = 1; j < ent.vd_cnt; j++)
		  {
		    /* Check for overflow.  */
		    if ((unsigned char *)(vstart + aux.vda_next) < (unsigned char *) vstart
			|| (unsigned char *)(vstart + aux.vda_next) > (unsigned char *) endbuf)
		      break;

		    isum   += aux.vda_next;
		    vstart += aux.vda_next;

		    eaux = (Elf_External_Verdaux *) vstart;
		    if (vstart + sizeof (*eaux) > endbuf)
		      break;

		    aux.vda_name = BYTE_GET (eaux->vda_name);
		    aux.vda_next = BYTE_GET (eaux->vda_next);

		    if (VALID_DYNAMIC_NAME (aux.vda_name))
		      printf (_("  %#06x: Parent %d: %s\n"),
			      isum, j, GET_DYNAMIC_NAME (aux.vda_name));
		    else
		      printf (_("  %#06x: Parent %d, name index: %ld\n"),
			      isum, j, aux.vda_name);
		  }

		if (j < ent.vd_cnt)
		  printf (_("  Version def aux past end of section\n"));

		idx += ent.vd_next;
	      }

	    if (cnt < section->sh_info)
	      printf (_("  Version definition past end of section\n"));

	    free (edefs);
	  }
	  break;

	case SHT_GNU_verneed:
	  {
	    Elf_External_Verneed * eneed;
	    unsigned int idx;
	    unsigned int cnt;
	    char * endbuf;

	    found = 1;

	    printf (_("\nVersion needs section '%s' contains %u entries:\n"),
		    SECTION_NAME (section), section->sh_info);

	    printf (_(" Addr: 0x"));
	    printf_vma (section->sh_addr);
	    printf (_("  Offset: %#08lx  Link: %u (%s)\n"),
		    (unsigned long) section->sh_offset, section->sh_link,
		    section->sh_link < elf_header.e_shnum
		    ? SECTION_NAME (section_headers + section->sh_link)
		    : _("<corrupt>"));

	    eneed = (Elf_External_Verneed *) get_data (NULL, file,
                                                       section->sh_offset, 1,
                                                       section->sh_size,
                                                       _("Version Needs section"));
	    if (!eneed)
	      break;
	    endbuf = (char *) eneed + section->sh_size;

	    for (idx = cnt = 0; cnt < section->sh_info; ++cnt)
	      {
		Elf_External_Verneed * entry;
		Elf_Internal_Verneed ent;
		int j;
		int isum;
		char * vstart;

		if ((unsigned char *) eneed + idx < (unsigned char *) eneed)
		  break;

		vstart = ((char *) eneed) + idx;
		if (vstart + sizeof (*entry) > endbuf)
		  break;

		entry = (Elf_External_Verneed *) vstart;

		ent.vn_version = BYTE_GET (entry->vn_version);
		ent.vn_cnt     = BYTE_GET (entry->vn_cnt);
		ent.vn_file    = BYTE_GET (entry->vn_file);
		ent.vn_aux     = BYTE_GET (entry->vn_aux);
		ent.vn_next    = BYTE_GET (entry->vn_next);

		printf (_("  %#06x: Version: %d"), idx, ent.vn_version);

		if (VALID_DYNAMIC_NAME (ent.vn_file))
		  printf (_("  File: %s"), GET_DYNAMIC_NAME (ent.vn_file));
		else
		  printf (_("  File: %lx"), ent.vn_file);

		printf (_("  Cnt: %d\n"), ent.vn_cnt);

		/* Check for overflow.  */
		if ((unsigned char *)(vstart + ent.vn_aux) < (unsigned char *) vstart
		    || (unsigned char *)(vstart + ent.vn_aux) > (unsigned char *) endbuf)
		  break;

		vstart += ent.vn_aux;

		for (j = 0, isum = idx + ent.vn_aux; j < ent.vn_cnt; ++j)
		  {
		    Elf_External_Vernaux * eaux;
		    Elf_Internal_Vernaux aux;

		    if (vstart + sizeof (*eaux) > endbuf)
		      break;
		    eaux = (Elf_External_Vernaux *) vstart;

		    aux.vna_hash  = BYTE_GET (eaux->vna_hash);
		    aux.vna_flags = BYTE_GET (eaux->vna_flags);
		    aux.vna_other = BYTE_GET (eaux->vna_other);
		    aux.vna_name  = BYTE_GET (eaux->vna_name);
		    aux.vna_next  = BYTE_GET (eaux->vna_next);

		    if (VALID_DYNAMIC_NAME (aux.vna_name))
		      printf (_("  %#06x:   Name: %s"),
			      isum, GET_DYNAMIC_NAME (aux.vna_name));
		    else
		      printf (_("  %#06x:   Name index: %lx"),
			      isum, aux.vna_name);

		    printf (_("  Flags: %s  Version: %d\n"),
			    get_ver_flags (aux.vna_flags), aux.vna_other);

		    /* Check for overflow.  */
		    if ((unsigned char *)(vstart + aux.vna_next) < (unsigned char *) vstart
			|| (unsigned char *)(vstart + aux.vna_next) > (unsigned char *) endbuf)
		      break;

		    isum   += aux.vna_next;
		    vstart += aux.vna_next;
		  }

		if (j < ent.vn_cnt)
		  warn (_("Missing Version Needs auxillary information\n"));

		idx += ent.vn_next;
	      }

	    if (cnt < section->sh_info)
	      warn (_("Missing Version Needs information\n"));

	    free (eneed);
	  }
	  break;

	case SHT_GNU_versym:
	  {
	    Elf_Internal_Shdr * link_section;
	    int total;
	    int cnt;
	    unsigned char * edata;
	    unsigned short * data;
	    char * strtab;
	    Elf_Internal_Sym * symbols;
	    Elf_Internal_Shdr * string_sec;
	    unsigned long num_syms;
	    long off;

	    if (section->sh_link >= elf_header.e_shnum)
	      break;

	    link_section = section_headers + section->sh_link;
	    total = section->sh_size / sizeof (Elf_External_Versym);

	    if (link_section->sh_link >= elf_header.e_shnum)
	      break;

	    found = 1;

	    symbols = GET_ELF_SYMBOLS (file, link_section, & num_syms);
	    if (symbols == NULL)
	      break;

	    string_sec = section_headers + link_section->sh_link;

	    strtab = (char *) get_data (NULL, file, string_sec->sh_offset, 1,
                                        string_sec->sh_size,
                                        _("version string table"));
	    if (!strtab)
	      {
		free (symbols);
		break;
	      }

	    printf (_("\nVersion symbols section '%s' contains %d entries:\n"),
		    SECTION_NAME (section), total);

	    printf (_(" Addr: "));
	    printf_vma (section->sh_addr);
	    printf (_("  Offset: %#08lx  Link: %u (%s)\n"),
		    (unsigned long) section->sh_offset, section->sh_link,
		    SECTION_NAME (link_section));

	    off = offset_from_vma (file,
				   version_info[DT_VERSIONTAGIDX (DT_VERSYM)],
				   total * sizeof (short));
	    edata = (unsigned char *) get_data (NULL, file, off, total,
                                                sizeof (short),
                                                _("version symbol data"));
	    if (!edata)
	      {
		free (strtab);
		free (symbols);
		break;
	      }

	    data = (short unsigned int *) cmalloc (total, sizeof (short));

	    for (cnt = total; cnt --;)
	      data[cnt] = byte_get (edata + cnt * sizeof (short),
				    sizeof (short));

	    free (edata);

	    for (cnt = 0; cnt < total; cnt += 4)
	      {
		int j, nn;
		int check_def, check_need;
		char * name;

		printf ("  %03x:", cnt);

		for (j = 0; (j < 4) && (cnt + j) < total; ++j)
		  switch (data[cnt + j])
		    {
		    case 0:
		      fputs (_("   0 (*local*)    "), stdout);
		      break;

		    case 1:
		      fputs (_("   1 (*global*)   "), stdout);
		      break;

		    default:
		      nn = printf ("%4x%c", data[cnt + j] & VERSYM_VERSION,
				   data[cnt + j] & VERSYM_HIDDEN ? 'h' : ' ');

		      /* If this index value is greater than the size of the symbols
		         array, break to avoid an out-of-bounds read.  */
		      if ((unsigned long)(cnt + j) >= num_syms)
		        {
		          warn (_("invalid index into symbol array\n"));
		          break;
			}

		      check_def = 1;
		      check_need = 1;
		      if (symbols[cnt + j].st_shndx >= elf_header.e_shnum
			  || section_headers[symbols[cnt + j].st_shndx].sh_type
			     != SHT_NOBITS)
			{
			  if (symbols[cnt + j].st_shndx == SHN_UNDEF)
			    check_def = 0;
			  else
			    check_need = 0;
			}

		      if (check_need
			  && version_info[DT_VERSIONTAGIDX (DT_VERNEED)])
			{
			  Elf_Internal_Verneed ivn;
			  unsigned long offset;

			  offset = offset_from_vma
			    (file, version_info[DT_VERSIONTAGIDX (DT_VERNEED)],
			     sizeof (Elf_External_Verneed));

			  do
			    {
			      Elf_Internal_Vernaux ivna;
			      Elf_External_Verneed evn;
			      Elf_External_Vernaux evna;
			      unsigned long a_off;

			      if (get_data (&evn, file, offset, sizeof (evn), 1,
					    _("version need")) == NULL)
				break;
			      
			      ivn.vn_aux  = BYTE_GET (evn.vn_aux);
			      ivn.vn_next = BYTE_GET (evn.vn_next);

			      a_off = offset + ivn.vn_aux;

			      do
				{
				  if (get_data (&evna, file, a_off, sizeof (evna),
						1, _("version need aux (2)")) == NULL)
				    {
				      ivna.vna_next  = 0;
				      ivna.vna_other = 0;
				    }
				  else
				    {
				      ivna.vna_next  = BYTE_GET (evna.vna_next);
				      ivna.vna_other = BYTE_GET (evna.vna_other);
				    }

				  a_off += ivna.vna_next;
				}
			      while (ivna.vna_other != data[cnt + j]
				     && ivna.vna_next != 0);

			      if (ivna.vna_other == data[cnt + j])
				{
				  ivna.vna_name = BYTE_GET (evna.vna_name);

				  if (ivna.vna_name >= string_sec->sh_size)
				    name = _("*invalid*");
				  else
				    name = strtab + ivna.vna_name;
				  nn += printf ("(%s%-*s",
						name,
						12 - (int) strlen (name),
						")");
				  check_def = 0;
				  break;
				}

			      offset += ivn.vn_next;
			    }
			  while (ivn.vn_next);
			}

		      if (check_def && data[cnt + j] != 0x8001
			  && version_info[DT_VERSIONTAGIDX (DT_VERDEF)])
			{
			  Elf_Internal_Verdef ivd;
			  Elf_External_Verdef evd;
			  unsigned long offset;

			  offset = offset_from_vma
			    (file, version_info[DT_VERSIONTAGIDX (DT_VERDEF)],
			     sizeof evd);

			  do
			    {
			      if (get_data (&evd, file, offset, sizeof (evd), 1,
					    _("version def")) == NULL)
				{
				  ivd.vd_next = 0;
				  ivd.vd_ndx  = 0;
				}
			      else
				{
				  ivd.vd_next = BYTE_GET (evd.vd_next);
				  ivd.vd_ndx  = BYTE_GET (evd.vd_ndx);
				}

			      offset += ivd.vd_next;
			    }
			  while (ivd.vd_ndx != (data[cnt + j] & VERSYM_VERSION)
				 && ivd.vd_next != 0);

			  if (ivd.vd_ndx == (data[cnt + j] & VERSYM_VERSION))
			    {
			      Elf_External_Verdaux evda;
			      Elf_Internal_Verdaux ivda;

			      ivd.vd_aux = BYTE_GET (evd.vd_aux);

			      if (get_data (&evda, file,
					    offset - ivd.vd_next + ivd.vd_aux,
					    sizeof (evda), 1,
					    _("version def aux")) == NULL)
				break;

			      ivda.vda_name = BYTE_GET (evda.vda_name);

			      if (ivda.vda_name >= string_sec->sh_size)
				name = _("*invalid*");
			      else
				name = strtab + ivda.vda_name;
			      nn += printf ("(%s%-*s",
					    name,
					    12 - (int) strlen (name),
					    ")");
			    }
			}

		      if (nn < 18)
			printf ("%*c", 18 - nn, ' ');
		    }

		putchar ('\n');
	      }

	    free (data);
	    free (strtab);
	    free (symbols);
	  }
	  break;

	default:
	  break;
	}
    }

  if (! found)
    printf (_("\nNo version information found in this file.\n"));

  return 1;
}

static const char *
get_symbol_binding (unsigned int binding)
{
  static char buff[32];

  switch (binding)
    {
    case STB_LOCAL:	return "LOCAL";
    case STB_GLOBAL:	return "GLOBAL";
    case STB_WEAK:	return "WEAK";
    default:
      if (binding >= STB_LOPROC && binding <= STB_HIPROC)
	snprintf (buff, sizeof (buff), _("<processor specific>: %d"),
		  binding);
      else if (binding >= STB_LOOS && binding <= STB_HIOS)
	{
	  if (binding == STB_GNU_UNIQUE
	      && (elf_header.e_ident[EI_OSABI] == ELFOSABI_GNU
		  /* GNU is still using the default value 0.  */
		  || elf_header.e_ident[EI_OSABI] == ELFOSABI_NONE))
	    return "UNIQUE";
	  snprintf (buff, sizeof (buff), _("<OS specific>: %d"), binding);
	}
      else
	snprintf (buff, sizeof (buff), _("<unknown>: %d"), binding);
      return buff;
    }
}

static const char *
get_symbol_type (unsigned int type)
{
  static char buff[32];

  switch (type)
    {
    case STT_NOTYPE:	return "NOTYPE";
    case STT_OBJECT:	return "OBJECT";
    case STT_FUNC:	return "FUNC";
    case STT_SECTION:	return "SECTION";
    case STT_FILE:	return "FILE";
    case STT_COMMON:	return "COMMON";
    case STT_TLS:	return "TLS";
    case STT_RELC:      return "RELC";
    case STT_SRELC:     return "SRELC";
    default:
      if (type >= STT_LOPROC && type <= STT_HIPROC)
	{
	  if (elf_header.e_machine == EM_ARM && type == STT_ARM_TFUNC)
	    return "THUMB_FUNC";

	  if (elf_header.e_machine == EM_SPARCV9 && type == STT_REGISTER)
	    return "REGISTER";

	  if (elf_header.e_machine == EM_PARISC && type == STT_PARISC_MILLI)
	    return "PARISC_MILLI";

	  snprintf (buff, sizeof (buff), _("<processor specific>: %d"), type);
	}
      else if (type >= STT_LOOS && type <= STT_HIOS)
	{
	  if (elf_header.e_machine == EM_PARISC)
	    {
	      if (type == STT_HP_OPAQUE)
		return "HP_OPAQUE";
	      if (type == STT_HP_STUB)
		return "HP_STUB";
	    }

	  if (type == STT_GNU_IFUNC
	      && (elf_header.e_ident[EI_OSABI] == ELFOSABI_GNU
		  || elf_header.e_ident[EI_OSABI] == ELFOSABI_FREEBSD
		  /* GNU is still using the default value 0.  */
		  || elf_header.e_ident[EI_OSABI] == ELFOSABI_NONE))
	    return "IFUNC";

	  snprintf (buff, sizeof (buff), _("<OS specific>: %d"), type);
	}
      else
	snprintf (buff, sizeof (buff), _("<unknown>: %d"), type);
      return buff;
    }
}

static const char *
get_symbol_visibility (unsigned int visibility)
{
  switch (visibility)
    {
    case STV_DEFAULT:	return "DEFAULT";
    case STV_INTERNAL:	return "INTERNAL";
    case STV_HIDDEN:	return "HIDDEN";
    case STV_PROTECTED: return "PROTECTED";
    default: abort ();
    }
}

static const char *
get_mips_symbol_other (unsigned int other)
{
  switch (other)
    {
    case STO_OPTIONAL:
      return "OPTIONAL";
    case STO_MIPS_PLT:
      return "MIPS PLT";
    case STO_MIPS_PIC:
      return "MIPS PIC";
    case STO_MICROMIPS:
      return "MICROMIPS";
    case STO_MICROMIPS | STO_MIPS_PIC:
      return "MICROMIPS, MIPS PIC";
    case STO_MIPS16:
      return "MIPS16";
    default:
      return NULL;
    }
}

static const char *
get_ia64_symbol_other (unsigned int other)
{
  if (is_ia64_vms ())
    {
      static char res[32];

      res[0] = 0;

      /* Function types is for images and .STB files only.  */
      switch (elf_header.e_type)
        {
        case ET_DYN:
        case ET_EXEC:
          switch (VMS_ST_FUNC_TYPE (other))
            {
            case VMS_SFT_CODE_ADDR:
              strcat (res, " CA");
              break;
            case VMS_SFT_SYMV_IDX:
              strcat (res, " VEC");
              break;
            case VMS_SFT_FD:
              strcat (res, " FD");
              break;
            case VMS_SFT_RESERVE:
              strcat (res, " RSV");
              break;
            default:
              abort ();
            }
          break;
        default:
          break;
        }
      switch (VMS_ST_LINKAGE (other))
        {
        case VMS_STL_IGNORE:
          strcat (res, " IGN");
          break;
        case VMS_STL_RESERVE:
          strcat (res, " RSV");
          break;
        case VMS_STL_STD:
          strcat (res, " STD");
          break;
        case VMS_STL_LNK:
          strcat (res, " LNK");
          break;
        default:
          abort ();
        }

      if (res[0] != 0)
        return res + 1;
      else
        return res;
    }
  return NULL;
}

static const char *
get_symbol_other (unsigned int other)
{
  const char * result = NULL;
  static char buff [32];

  if (other == 0)
    return "";

  switch (elf_header.e_machine)
    {
    case EM_MIPS:
      result = get_mips_symbol_other (other);
      break;
    case EM_IA_64:
      result = get_ia64_symbol_other (other);
      break;
    default:
      break;
    }

  if (result)
    return result;

  snprintf (buff, sizeof buff, _("<other>: %x"), other);
  return buff;
}

static const char *
get_symbol_index_type (unsigned int type)
{
  static char buff[32];

  switch (type)
    {
    case SHN_UNDEF:	return "UND";
    case SHN_ABS:	return "ABS";
    case SHN_COMMON:	return "COM";
    default:
      if (type == SHN_IA_64_ANSI_COMMON
	  && elf_header.e_machine == EM_IA_64
	  && elf_header.e_ident[EI_OSABI] == ELFOSABI_HPUX)
	return "ANSI_COM";
      else if ((elf_header.e_machine == EM_X86_64
		|| elf_header.e_machine == EM_L1OM
		|| elf_header.e_machine == EM_K1OM)
	       && type == SHN_X86_64_LCOMMON)
	return "LARGE_COM";
      else if ((type == SHN_MIPS_SCOMMON
		&& elf_header.e_machine == EM_MIPS)
	       || (type == SHN_TIC6X_SCOMMON
		   && elf_header.e_machine == EM_TI_C6000))
	return "SCOM";
      else if (type == SHN_MIPS_SUNDEFINED
	       && elf_header.e_machine == EM_MIPS)
	return "SUND";
      else if (type >= SHN_LOPROC && type <= SHN_HIPROC)
	sprintf (buff, "PRC[0x%04x]", type & 0xffff);
      else if (type >= SHN_LOOS && type <= SHN_HIOS)
	sprintf (buff, "OS [0x%04x]", type & 0xffff);
      else if (type >= SHN_LORESERVE)
	sprintf (buff, "RSV[0x%04x]", type & 0xffff);
      else if (type >= elf_header.e_shnum)
	sprintf (buff, "bad section index[%3d]", type);
      else
	sprintf (buff, "%3d", type);
      break;
    }

  return buff;
}

static bfd_vma *
get_dynamic_data (FILE * file, unsigned int number, unsigned int ent_size)
{
  unsigned char * e_data;
  bfd_vma * i_data;

  e_data = (unsigned char *) cmalloc (number, ent_size);

  if (e_data == NULL)
    {
      error (_("Out of memory\n"));
      return NULL;
    }

  if (fread (e_data, ent_size, number, file) != number)
    {
      error (_("Unable to read in dynamic data\n"));
      return NULL;
    }

  i_data = (bfd_vma *) cmalloc (number, sizeof (*i_data));

  if (i_data == NULL)
    {
      error (_("Out of memory\n"));
      free (e_data);
      return NULL;
    }

  while (number--)
    i_data[number] = byte_get (e_data + number * ent_size, ent_size);

  free (e_data);

  return i_data;
}

static void
print_dynamic_symbol (bfd_vma si, unsigned long hn)
{
  Elf_Internal_Sym * psym;
  int n;
  const char* tmpsp = "     ";

  psym = dynamic_symbols + si;

  n = print_vma (si, DEC_5);
  if (n < 5)
    fputs (tmpsp + n, stdout);
  printf (" %3lu: ", hn);
  print_vma (psym->st_value, LONG_HEX);
  putchar (' ');
  print_vma (psym->st_size, DEC_5);

  printf (" %-7s", get_symbol_type (ELF_ST_TYPE (psym->st_info)));
  printf (" %-6s",  get_symbol_binding (ELF_ST_BIND (psym->st_info)));
  printf (" %-7s",  get_symbol_visibility (ELF_ST_VISIBILITY (psym->st_other)));
  /* Check to see if any other bits in the st_other field are set.
     Note - displaying this information disrupts the layout of the
     table being generated, but for the moment this case is very
     rare.  */
  if (psym->st_other ^ ELF_ST_VISIBILITY (psym->st_other))
    printf (" [%s] ", get_symbol_other (psym->st_other ^ ELF_ST_VISIBILITY (psym->st_other)));
  printf (" %3.3s ", get_symbol_index_type (psym->st_shndx));
  if (VALID_DYNAMIC_NAME (psym->st_name))
    print_symbol (25, GET_DYNAMIC_NAME (psym->st_name));
  else
    printf (_(" <corrupt: %14ld>"), psym->st_name);
  putchar ('\n');
}

/* Dump the symbol table.  */
static int
process_symbol_table (FILE * file)
{
  Elf_Internal_Shdr * section;
  bfd_vma nbuckets = 0;
  bfd_vma nchains = 0;
  bfd_vma * buckets = NULL;
  bfd_vma * chains = NULL;
  bfd_vma ngnubuckets = 0;
  bfd_vma * gnubuckets = NULL;
  bfd_vma * gnuchains = NULL;
  bfd_vma gnusymidx = 0;

  if (!do_syms && !do_dyn_syms && !do_histogram)
    return 1;

  if (dynamic_info[DT_HASH]
      && (do_histogram
	  || (do_using_dynamic
	      && !do_dyn_syms
	      && dynamic_strings != NULL)))
    {
      unsigned char nb[8];
      unsigned char nc[8];
      int hash_ent_size = 4;

      if ((elf_header.e_machine == EM_ALPHA
	   || elf_header.e_machine == EM_S390
	   || elf_header.e_machine == EM_S390_OLD)
	  && elf_header.e_ident[EI_CLASS] == ELFCLASS64)
	hash_ent_size = 8;

      if (fseek (file,
		 (archive_file_offset
		  + offset_from_vma (file, dynamic_info[DT_HASH],
				     sizeof nb + sizeof nc)),
		 SEEK_SET))
	{
	  error (_("Unable to seek to start of dynamic information\n"));
	  goto no_hash;
	}

      if (fread (nb, hash_ent_size, 1, file) != 1)
	{
	  error (_("Failed to read in number of buckets\n"));
	  goto no_hash;
	}

      if (fread (nc, hash_ent_size, 1, file) != 1)
	{
	  error (_("Failed to read in number of chains\n"));
	  goto no_hash;
	}

      nbuckets = byte_get (nb, hash_ent_size);
      nchains  = byte_get (nc, hash_ent_size);

      buckets = get_dynamic_data (file, nbuckets, hash_ent_size);
      chains  = get_dynamic_data (file, nchains, hash_ent_size);

    no_hash:
      if (buckets == NULL || chains == NULL)
	{
	  if (do_using_dynamic)
	    return 0;
	  free (buckets);
	  free (chains);
	  buckets = NULL;
	  chains = NULL;
	  nbuckets = 0;
	  nchains = 0;
	}
    }

  if (dynamic_info_DT_GNU_HASH
      && (do_histogram
	  || (do_using_dynamic
	      && !do_dyn_syms
	      && dynamic_strings != NULL)))
    {
      unsigned char nb[16];
      bfd_vma i, maxchain = 0xffffffff, bitmaskwords;
      bfd_vma buckets_vma;

      if (fseek (file,
		 (archive_file_offset
		  + offset_from_vma (file, dynamic_info_DT_GNU_HASH,
				     sizeof nb)),
		 SEEK_SET))
	{
	  error (_("Unable to seek to start of dynamic information\n"));
	  goto no_gnu_hash;
	}

      if (fread (nb, 16, 1, file) != 1)
	{
	  error (_("Failed to read in number of buckets\n"));
	  goto no_gnu_hash;
	}

      ngnubuckets = byte_get (nb, 4);
      gnusymidx = byte_get (nb + 4, 4);
      bitmaskwords = byte_get (nb + 8, 4);
      buckets_vma = dynamic_info_DT_GNU_HASH + 16;
      if (is_32bit_elf)
	buckets_vma += bitmaskwords * 4;
      else
	buckets_vma += bitmaskwords * 8;

      if (fseek (file,
		 (archive_file_offset
		  + offset_from_vma (file, buckets_vma, 4)),
		 SEEK_SET))
	{
	  error (_("Unable to seek to start of dynamic information\n"));
	  goto no_gnu_hash;
	}

      gnubuckets = get_dynamic_data (file, ngnubuckets, 4);

      if (gnubuckets == NULL)
	goto no_gnu_hash;

      for (i = 0; i < ngnubuckets; i++)
	if (gnubuckets[i] != 0)
	  {
	    if (gnubuckets[i] < gnusymidx)
	      return 0;

	    if (maxchain == 0xffffffff || gnubuckets[i] > maxchain)
	      maxchain = gnubuckets[i];
	  }

      if (maxchain == 0xffffffff)
	goto no_gnu_hash;

      maxchain -= gnusymidx;

      if (fseek (file,
		 (archive_file_offset
		  + offset_from_vma (file, buckets_vma
					   + 4 * (ngnubuckets + maxchain), 4)),
		 SEEK_SET))
	{
	  error (_("Unable to seek to start of dynamic information\n"));
	  goto no_gnu_hash;
	}

      do
	{
	  if (fread (nb, 4, 1, file) != 1)
	    {
	      error (_("Failed to determine last chain length\n"));
	      goto no_gnu_hash;
	    }

	  if (maxchain + 1 == 0)
	    goto no_gnu_hash;

	  ++maxchain;
	}
      while ((byte_get (nb, 4) & 1) == 0);

      if (fseek (file,
		 (archive_file_offset
		  + offset_from_vma (file, buckets_vma + 4 * ngnubuckets, 4)),
		 SEEK_SET))
	{
	  error (_("Unable to seek to start of dynamic information\n"));
	  goto no_gnu_hash;
	}

      gnuchains = get_dynamic_data (file, maxchain, 4);

    no_gnu_hash:
      if (gnuchains == NULL)
	{
	  free (gnubuckets);
	  gnubuckets = NULL;
	  ngnubuckets = 0;
	  if (do_using_dynamic)
	    return 0;
	}
    }

  if ((dynamic_info[DT_HASH] || dynamic_info_DT_GNU_HASH)
      && do_syms
      && do_using_dynamic
      && dynamic_strings != NULL)
    {
      unsigned long hn;

      if (dynamic_info[DT_HASH])
	{
	  bfd_vma si;

	  printf (_("\nSymbol table for image:\n"));
	  if (is_32bit_elf)
	    printf (_("  Num Buc:    Value  Size   Type   Bind Vis      Ndx Name\n"));
	  else
	    printf (_("  Num Buc:    Value          Size   Type   Bind Vis      Ndx Name\n"));

	  for (hn = 0; hn < nbuckets; hn++)
	    {
	      if (! buckets[hn])
		continue;

	      for (si = buckets[hn]; si < nchains && si > 0; si = chains[si])
		print_dynamic_symbol (si, hn);
	    }
	}

      if (dynamic_info_DT_GNU_HASH)
	{
	  printf (_("\nSymbol table of `.gnu.hash' for image:\n"));
	  if (is_32bit_elf)
	    printf (_("  Num Buc:    Value  Size   Type   Bind Vis      Ndx Name\n"));
	  else
	    printf (_("  Num Buc:    Value          Size   Type   Bind Vis      Ndx Name\n"));

	  for (hn = 0; hn < ngnubuckets; ++hn)
	    if (gnubuckets[hn] != 0)
	      {
		bfd_vma si = gnubuckets[hn];
		bfd_vma off = si - gnusymidx;

		do
		  {
		    print_dynamic_symbol (si, hn);
		    si++;
		  }
		while ((gnuchains[off++] & 1) == 0);
	      }
	}
    }
  else if (do_dyn_syms || (do_syms && !do_using_dynamic))
    {
      unsigned int i;

      for (i = 0, section = section_headers;
	   i < elf_header.e_shnum;
	   i++, section++)
	{
	  unsigned int si;
	  char * strtab = NULL;
	  unsigned long int strtab_size = 0;
	  Elf_Internal_Sym * symtab;
	  Elf_Internal_Sym * psym;
	  unsigned long num_syms;

	  if ((section->sh_type != SHT_SYMTAB
	       && section->sh_type != SHT_DYNSYM)
	      || (!do_syms
		  && section->sh_type == SHT_SYMTAB))
	    continue;

	  if (section->sh_entsize == 0)
	    {
	      printf (_("\nSymbol table '%s' has a sh_entsize of zero!\n"),
		      SECTION_NAME (section));
	      continue;
	    }

	  printf (_("\nSymbol table '%s' contains %lu entries:\n"),
		  SECTION_NAME (section),
		  (unsigned long) (section->sh_size / section->sh_entsize));

	  if (is_32bit_elf)
	    printf (_("   Num:    Value  Size Type    Bind   Vis      Ndx Name\n"));
	  else
	    printf (_("   Num:    Value          Size Type    Bind   Vis      Ndx Name\n"));

	  symtab = GET_ELF_SYMBOLS (file, section, & num_syms);
	  if (symtab == NULL)
	    continue;

	  if (section->sh_link == elf_header.e_shstrndx)
	    {
	      strtab = string_table;
	      strtab_size = string_table_length;
	    }
	  else if (section->sh_link < elf_header.e_shnum)
	    {
	      Elf_Internal_Shdr * string_sec;

	      string_sec = section_headers + section->sh_link;

	      strtab = (char *) get_data (NULL, file, string_sec->sh_offset,
                                          1, string_sec->sh_size,
                                          _("string table"));
	      strtab_size = strtab != NULL ? string_sec->sh_size : 0;
	    }

	  for (si = 0, psym = symtab; si < num_syms; si++, psym++)
	    {
	      printf ("%6d: ", si);
	      print_vma (psym->st_value, LONG_HEX);
	      putchar (' ');
	      print_vma (psym->st_size, DEC_5);
	      printf (" %-7s", get_symbol_type (ELF_ST_TYPE (psym->st_info)));
	      printf (" %-6s", get_symbol_binding (ELF_ST_BIND (psym->st_info)));
	      printf (" %-7s", get_symbol_visibility (ELF_ST_VISIBILITY (psym->st_other)));
	      /* Check to see if any other bits in the st_other field are set.
	         Note - displaying this information disrupts the layout of the
	         table being generated, but for the moment this case is very rare.  */
	      if (psym->st_other ^ ELF_ST_VISIBILITY (psym->st_other))
		printf (" [%s] ", get_symbol_other (psym->st_other ^ ELF_ST_VISIBILITY (psym->st_other)));
	      printf (" %4s ", get_symbol_index_type (psym->st_shndx));
	      print_symbol (25, psym->st_name < strtab_size
			    ? strtab + psym->st_name : _("<corrupt>"));

	      if (section->sh_type == SHT_DYNSYM
		  && version_info[DT_VERSIONTAGIDX (DT_VERSYM)] != 0)
		{
		  unsigned char data[2];
		  unsigned short vers_data;
		  unsigned long offset;
		  int is_nobits;
		  int check_def;

		  offset = offset_from_vma
		    (file, version_info[DT_VERSIONTAGIDX (DT_VERSYM)],
		     sizeof data + si * sizeof (vers_data));

		  if (get_data (&data, file, offset + si * sizeof (vers_data),
				sizeof (data), 1, _("version data")) == NULL)
		    break;

		  vers_data = byte_get (data, 2);

		  is_nobits = (psym->st_shndx < elf_header.e_shnum
			       && section_headers[psym->st_shndx].sh_type
				  == SHT_NOBITS);

		  check_def = (psym->st_shndx != SHN_UNDEF);

		  if ((vers_data & VERSYM_HIDDEN) || vers_data > 1)
		    {
		      if (version_info[DT_VERSIONTAGIDX (DT_VERNEED)]
			  && (is_nobits || ! check_def))
			{
			  Elf_External_Verneed evn;
			  Elf_Internal_Verneed ivn;
			  Elf_Internal_Vernaux ivna;

			  /* We must test both.  */
			  offset = offset_from_vma
			    (file, version_info[DT_VERSIONTAGIDX (DT_VERNEED)],
			     sizeof evn);

			  do
			    {
			      unsigned long vna_off;

			      if (get_data (&evn, file, offset, sizeof (evn), 1,
					    _("version need")) == NULL)
				{
				  ivna.vna_next = 0;
				  ivna.vna_other = 0;
				  ivna.vna_name = 0;
				  break;
				}

			      ivn.vn_aux  = BYTE_GET (evn.vn_aux);
			      ivn.vn_next = BYTE_GET (evn.vn_next);

			      vna_off = offset + ivn.vn_aux;

			      do
				{
				  Elf_External_Vernaux evna;

				  if (get_data (&evna, file, vna_off,
						sizeof (evna), 1,
						_("version need aux (3)")) == NULL)
				    {
				      ivna.vna_next = 0;
				      ivna.vna_other = 0;
				      ivna.vna_name = 0;
				    }
				  else
				    {
				      ivna.vna_other = BYTE_GET (evna.vna_other);
				      ivna.vna_next  = BYTE_GET (evna.vna_next);
				      ivna.vna_name  = BYTE_GET (evna.vna_name);
				    }

				  vna_off += ivna.vna_next;
				}
			      while (ivna.vna_other != vers_data
				     && ivna.vna_next != 0);

			      if (ivna.vna_other == vers_data)
				break;

			      offset += ivn.vn_next;
			    }
			  while (ivn.vn_next != 0);

			  if (ivna.vna_other == vers_data)
			    {
			      printf ("@%s (%d)",
				      ivna.vna_name < strtab_size
				      ? strtab + ivna.vna_name : _("<corrupt>"),
				      ivna.vna_other);
			      check_def = 0;
			    }
			  else if (! is_nobits)
			    error (_("bad dynamic symbol\n"));
			  else
			    check_def = 1;
			}

		      if (check_def)
			{
			  if (vers_data != 0x8001
			      && version_info[DT_VERSIONTAGIDX (DT_VERDEF)])
			    {
			      Elf_Internal_Verdef ivd;
			      Elf_Internal_Verdaux ivda;
			      Elf_External_Verdaux evda;
			      unsigned long off;

			      off = offset_from_vma
				(file,
				 version_info[DT_VERSIONTAGIDX (DT_VERDEF)],
				 sizeof (Elf_External_Verdef));

			      do
				{
				  Elf_External_Verdef evd;

				  if (get_data (&evd, file, off, sizeof (evd),
						1, _("version def")) == NULL)
				    {
				      ivd.vd_ndx = 0;
				      ivd.vd_aux = 0;
				      ivd.vd_next = 0;
				    }
				  else
				    {
				      ivd.vd_ndx = BYTE_GET (evd.vd_ndx);
				      ivd.vd_aux = BYTE_GET (evd.vd_aux);
				      ivd.vd_next = BYTE_GET (evd.vd_next);
				    }

				  off += ivd.vd_next;
				}
			      while (ivd.vd_ndx != (vers_data & VERSYM_VERSION)
				     && ivd.vd_next != 0);

			      off -= ivd.vd_next;
			      off += ivd.vd_aux;

			      if (get_data (&evda, file, off, sizeof (evda),
					    1, _("version def aux")) == NULL)
				break;

			      ivda.vda_name = BYTE_GET (evda.vda_name);

			      if (psym->st_name != ivda.vda_name)
				printf ((vers_data & VERSYM_HIDDEN)
					? "@%s" : "@@%s",
					ivda.vda_name < strtab_size
					? strtab + ivda.vda_name : _("<corrupt>"));
			    }
			}
		    }
		}

	      putchar ('\n');
	    }

	  free (symtab);
	  if (strtab != string_table)
	    free (strtab);
	}
    }
  else if (do_syms)
    printf
      (_("\nDynamic symbol information is not available for displaying symbols.\n"));

  if (do_histogram && buckets != NULL)
    {
      unsigned long * lengths;
      unsigned long * counts;
      unsigned long hn;
      bfd_vma si;
      unsigned long maxlength = 0;
      unsigned long nzero_counts = 0;
      unsigned long nsyms = 0;

      printf (_("\nHistogram for bucket list length (total of %lu buckets):\n"),
	      (unsigned long) nbuckets);
      printf (_(" Length  Number     %% of total  Coverage\n"));

      lengths = (unsigned long *) calloc (nbuckets, sizeof (*lengths));
      if (lengths == NULL)
	{
	  error (_("Out of memory\n"));
	  return 0;
	}
      for (hn = 0; hn < nbuckets; ++hn)
	{
	  for (si = buckets[hn]; si > 0 && si < nchains; si = chains[si])
	    {
	      ++nsyms;
	      if (maxlength < ++lengths[hn])
		++maxlength;
	    }
	}

      counts = (unsigned long *) calloc (maxlength + 1, sizeof (*counts));
      if (counts == NULL)
	{
	  error (_("Out of memory\n"));
	  return 0;
	}

      for (hn = 0; hn < nbuckets; ++hn)
	++counts[lengths[hn]];

      if (nbuckets > 0)
	{
	  unsigned long i;
	  printf ("      0  %-10lu (%5.1f%%)\n",
		  counts[0], (counts[0] * 100.0) / nbuckets);
	  for (i = 1; i <= maxlength; ++i)
	    {
	      nzero_counts += counts[i] * i;
	      printf ("%7lu  %-10lu (%5.1f%%)    %5.1f%%\n",
		      i, counts[i], (counts[i] * 100.0) / nbuckets,
		      (nzero_counts * 100.0) / nsyms);
	    }
	}

      free (counts);
      free (lengths);
    }

  if (buckets != NULL)
    {
      free (buckets);
      free (chains);
    }

  if (do_histogram && gnubuckets != NULL)
    {
      unsigned long * lengths;
      unsigned long * counts;
      unsigned long hn;
      unsigned long maxlength = 0;
      unsigned long nzero_counts = 0;
      unsigned long nsyms = 0;

      lengths = (unsigned long *) calloc (ngnubuckets, sizeof (*lengths));
      if (lengths == NULL)
	{
	  error (_("Out of memory\n"));
	  return 0;
	}

      printf (_("\nHistogram for `.gnu.hash' bucket list length (total of %lu buckets):\n"),
	      (unsigned long) ngnubuckets);
      printf (_(" Length  Number     %% of total  Coverage\n"));

      for (hn = 0; hn < ngnubuckets; ++hn)
	if (gnubuckets[hn] != 0)
	  {
	    bfd_vma off, length = 1;

	    for (off = gnubuckets[hn] - gnusymidx;
		 (gnuchains[off] & 1) == 0; ++off)
	      ++length;
	    lengths[hn] = length;
	    if (length > maxlength)
	      maxlength = length;
	    nsyms += length;
	  }

      counts = (unsigned long *) calloc (maxlength + 1, sizeof (*counts));
      if (counts == NULL)
	{
	  error (_("Out of memory\n"));
	  return 0;
	}

      for (hn = 0; hn < ngnubuckets; ++hn)
	++counts[lengths[hn]];

      if (ngnubuckets > 0)
	{
	  unsigned long j;
	  printf ("      0  %-10lu (%5.1f%%)\n",
		  counts[0], (counts[0] * 100.0) / ngnubuckets);
	  for (j = 1; j <= maxlength; ++j)
	    {
	      nzero_counts += counts[j] * j;
	      printf ("%7lu  %-10lu (%5.1f%%)    %5.1f%%\n",
		      j, counts[j], (counts[j] * 100.0) / ngnubuckets,
		      (nzero_counts * 100.0) / nsyms);
	    }
	}

      free (counts);
      free (lengths);
      free (gnubuckets);
      free (gnuchains);
    }

  return 1;
}

static int
process_syminfo (FILE * file ATTRIBUTE_UNUSED)
{
  unsigned int i;

  if (dynamic_syminfo == NULL
      || !do_dynamic)
    /* No syminfo, this is ok.  */
    return 1;

  /* There better should be a dynamic symbol section.  */
  if (dynamic_symbols == NULL || dynamic_strings == NULL)
    return 0;

  if (dynamic_addr)
    printf (_("\nDynamic info segment at offset 0x%lx contains %d entries:\n"),
	    dynamic_syminfo_offset, dynamic_syminfo_nent);

  printf (_(" Num: Name                           BoundTo     Flags\n"));
  for (i = 0; i < dynamic_syminfo_nent; ++i)
    {
      unsigned short int flags = dynamic_syminfo[i].si_flags;

      printf ("%4d: ", i);
      if (VALID_DYNAMIC_NAME (dynamic_symbols[i].st_name))
	print_symbol (30, GET_DYNAMIC_NAME (dynamic_symbols[i].st_name));
      else
	printf (_("<corrupt: %19ld>"), dynamic_symbols[i].st_name);
      putchar (' ');

      switch (dynamic_syminfo[i].si_boundto)
	{
	case SYMINFO_BT_SELF:
	  fputs ("SELF       ", stdout);
	  break;
	case SYMINFO_BT_PARENT:
	  fputs ("PARENT     ", stdout);
	  break;
	default:
	  if (dynamic_syminfo[i].si_boundto > 0
	      && dynamic_syminfo[i].si_boundto < dynamic_nent
	      && VALID_DYNAMIC_NAME (dynamic_section[dynamic_syminfo[i].si_boundto].d_un.d_val))
	    {
	      print_symbol (10, GET_DYNAMIC_NAME (dynamic_section[dynamic_syminfo[i].si_boundto].d_un.d_val));
	      putchar (' ' );
	    }
	  else
	    printf ("%-10d ", dynamic_syminfo[i].si_boundto);
	  break;
	}

      if (flags & SYMINFO_FLG_DIRECT)
	printf (" DIRECT");
      if (flags & SYMINFO_FLG_PASSTHRU)
	printf (" PASSTHRU");
      if (flags & SYMINFO_FLG_COPY)
	printf (" COPY");
      if (flags & SYMINFO_FLG_LAZYLOAD)
	printf (" LAZYLOAD");

      puts ("");
    }

  return 1;
}

/* Check to see if the given reloc needs to be handled in a target specific
   manner.  If so then process the reloc and return TRUE otherwise return
   FALSE.  */

static bfd_boolean
target_specific_reloc_handling (Elf_Internal_Rela * reloc,
				unsigned char *     start,
				Elf_Internal_Sym *  symtab)
{
  unsigned int reloc_type = get_reloc_type (reloc->r_info);

  switch (elf_header.e_machine)
    {
    case EM_MN10300:
    case EM_CYGNUS_MN10300:
      {
	static Elf_Internal_Sym * saved_sym = NULL;

	switch (reloc_type)
	  {
	  case 34: /* R_MN10300_ALIGN */
	    return TRUE;
	  case 33: /* R_MN10300_SYM_DIFF */
	    saved_sym = symtab + get_reloc_symindex (reloc->r_info);
	    return TRUE;
	  case 1: /* R_MN10300_32 */
	  case 2: /* R_MN10300_16 */
	    if (saved_sym != NULL)
	      {
		bfd_vma value;

		value = reloc->r_addend
		  + (symtab[get_reloc_symindex (reloc->r_info)].st_value
		     - saved_sym->st_value);

		byte_put (start + reloc->r_offset, value, reloc_type == 1 ? 4 : 2);

		saved_sym = NULL;
		return TRUE;
	      }
	    break;
	  default:
	    if (saved_sym != NULL)
	      error (_("Unhandled MN10300 reloc type found after SYM_DIFF reloc"));
	    break;
	  }
	break;
      }
    }

  return FALSE;
}

/* Returns TRUE iff RELOC_TYPE is a 32-bit absolute RELA relocation used in
   DWARF debug sections.  This is a target specific test.  Note - we do not
   go through the whole including-target-headers-multiple-times route, (as
   we have already done with <elf/h8.h>) because this would become very
   messy and even then this function would have to contain target specific
   information (the names of the relocs instead of their numeric values).
   FIXME: This is not the correct way to solve this problem.  The proper way
   is to have target specific reloc sizing and typing functions created by
   the reloc-macros.h header, in the same way that it already creates the
   reloc naming functions.  */

static bfd_boolean
is_32bit_abs_reloc (unsigned int reloc_type)
{
  switch (elf_header.e_machine)
    {
    case EM_386:
    case EM_486:
      return reloc_type == 1; /* R_386_32.  */
    case EM_68K:
      return reloc_type == 1; /* R_68K_32.  */
    case EM_860:
      return reloc_type == 1; /* R_860_32.  */
    case EM_960:
      return reloc_type == 2; /* R_960_32.  */
    case EM_AARCH64:
      return reloc_type == 258; /* R_AARCH64_ABS32 */
    case EM_ALPHA:
      return reloc_type == 1; /* R_ALPHA_REFLONG.  */
    case EM_ARC:
      return reloc_type == 1; /* R_ARC_32.  */
    case EM_ARM:
      return reloc_type == 2; /* R_ARM_ABS32 */
    case EM_AVR_OLD:
    case EM_AVR:
      return reloc_type == 1;
    case EM_ADAPTEVA_EPIPHANY:
      return reloc_type == 3;
    case EM_BLACKFIN:
      return reloc_type == 0x12; /* R_byte4_data.  */
    case EM_CRIS:
      return reloc_type == 3; /* R_CRIS_32.  */
    case EM_CR16:
      return reloc_type == 3; /* R_CR16_NUM32.  */
    case EM_CRX:
      return reloc_type == 15; /* R_CRX_NUM32.  */
    case EM_CYGNUS_FRV:
      return reloc_type == 1;
    case EM_CYGNUS_D10V:
    case EM_D10V:
      return reloc_type == 6; /* R_D10V_32.  */
    case EM_CYGNUS_D30V:
    case EM_D30V:
      return reloc_type == 12; /* R_D30V_32_NORMAL.  */
    case EM_DLX:
      return reloc_type == 3; /* R_DLX_RELOC_32.  */
    case EM_CYGNUS_FR30:
    case EM_FR30:
      return reloc_type == 3; /* R_FR30_32.  */
    case EM_H8S:
    case EM_H8_300:
    case EM_H8_300H:
      return reloc_type == 1; /* R_H8_DIR32.  */
    case EM_IA_64:
      return reloc_type == 0x65; /* R_IA64_SECREL32LSB.  */
    case EM_IP2K_OLD:
    case EM_IP2K:
      return reloc_type == 2; /* R_IP2K_32.  */
    case EM_IQ2000:
      return reloc_type == 2; /* R_IQ2000_32.  */
    case EM_LATTICEMICO32:
      return reloc_type == 3; /* R_LM32_32.  */
    case EM_M32C_OLD:
    case EM_M32C:
      return reloc_type == 3; /* R_M32C_32.  */
    case EM_M32R:
      return reloc_type == 34; /* R_M32R_32_RELA.  */
    case EM_MCORE:
      return reloc_type == 1; /* R_MCORE_ADDR32.  */
    case EM_CYGNUS_MEP:
      return reloc_type == 4; /* R_MEP_32.  */
    case EM_MICROBLAZE:
      return reloc_type == 1; /* R_MICROBLAZE_32.  */
    case EM_MIPS:
      return reloc_type == 2; /* R_MIPS_32.  */
    case EM_MMIX:
      return reloc_type == 4; /* R_MMIX_32.  */
    case EM_CYGNUS_MN10200:
    case EM_MN10200:
      return reloc_type == 1; /* R_MN10200_32.  */
    case EM_CYGNUS_MN10300:
    case EM_MN10300:
      return reloc_type == 1; /* R_MN10300_32.  */
    case EM_MOXIE:
      return reloc_type == 1; /* R_MOXIE_32.  */
    case EM_MSP430_OLD:
    case EM_MSP430:
      return reloc_type == 1; /* R_MSP43_32.  */
    case EM_MT:
      return reloc_type == 2; /* R_MT_32.  */
    case EM_ALTERA_NIOS2:
    case EM_NIOS32:
      return reloc_type == 1; /* R_NIOS_32.  */
    case EM_OPENRISC:
    case EM_OR32:
      return reloc_type == 1; /* R_OR32_32.  */
    case EM_PARISC:
      return (reloc_type == 1 /* R_PARISC_DIR32.  */
	      || reloc_type == 41); /* R_PARISC_SECREL32.  */
    case EM_PJ:
    case EM_PJ_OLD:
      return reloc_type == 1; /* R_PJ_DATA_DIR32.  */
    case EM_PPC64:
      return reloc_type == 1; /* R_PPC64_ADDR32.  */
    case EM_PPC:
      return reloc_type == 1; /* R_PPC_ADDR32.  */
    case EM_RL78:
      return reloc_type == 1; /* R_RL78_DIR32.  */
    case EM_RX:
      return reloc_type == 1; /* R_RX_DIR32.  */
    case EM_S370:
      return reloc_type == 1; /* R_I370_ADDR31.  */
    case EM_S390_OLD:
    case EM_S390:
      return reloc_type == 4; /* R_S390_32.  */
    case EM_SCORE:
      return reloc_type == 8; /* R_SCORE_ABS32.  */
    case EM_SH:
      return reloc_type == 1; /* R_SH_DIR32.  */
    case EM_SPARC32PLUS:
    case EM_SPARCV9:
    case EM_SPARC:
      return reloc_type == 3 /* R_SPARC_32.  */
	|| reloc_type == 23; /* R_SPARC_UA32.  */
    case EM_SPU:
      return reloc_type == 6; /* R_SPU_ADDR32 */
    case EM_TI_C6000:
      return reloc_type == 1; /* R_C6000_ABS32.  */
    case EM_TILEGX:
      return reloc_type == 2; /* R_TILEGX_32.  */
    case EM_TILEPRO:
      return reloc_type == 1; /* R_TILEPRO_32.  */
    case EM_CYGNUS_V850:
    case EM_V850:
      return reloc_type == 6; /* R_V850_ABS32.  */
    case EM_VAX:
      return reloc_type == 1; /* R_VAX_32.  */
    case EM_X86_64:
    case EM_L1OM:
    case EM_K1OM:
      return reloc_type == 10; /* R_X86_64_32.  */
    case EM_XC16X:
    case EM_C166:
      return reloc_type == 3; /* R_XC16C_ABS_32.  */
    case EM_XGATE:
      return reloc_type == 4; /* R_XGATE_32.  */
    case EM_XSTORMY16:
      return reloc_type == 1; /* R_XSTROMY16_32.  */
    case EM_XTENSA_OLD:
    case EM_XTENSA:
      return reloc_type == 1; /* R_XTENSA_32.  */
    default:
      error (_("Missing knowledge of 32-bit reloc types used in DWARF sections of machine number %d\n"),
	     elf_header.e_machine);
      abort ();
    }
}

/* Like is_32bit_abs_reloc except that it returns TRUE iff RELOC_TYPE is
   a 32-bit pc-relative RELA relocation used in DWARF debug sections.  */

static bfd_boolean
is_32bit_pcrel_reloc (unsigned int reloc_type)
{
  switch (elf_header.e_machine)
    {
    case EM_386:
    case EM_486:
      return reloc_type == 2;  /* R_386_PC32.  */
    case EM_68K:
      return reloc_type == 4;  /* R_68K_PC32.  */
    case EM_AARCH64:
      return reloc_type == 261; /* R_AARCH64_PREL32 */
    case EM_ADAPTEVA_EPIPHANY:
      return reloc_type == 6;
    case EM_ALPHA:
      return reloc_type == 10; /* R_ALPHA_SREL32.  */
    case EM_ARM:
      return reloc_type == 3;  /* R_ARM_REL32 */
    case EM_MICROBLAZE:
      return reloc_type == 2;  /* R_MICROBLAZE_32_PCREL.  */
    case EM_PARISC:
      return reloc_type == 9;  /* R_PARISC_PCREL32.  */
    case EM_PPC:
      return reloc_type == 26; /* R_PPC_REL32.  */
    case EM_PPC64:
      return reloc_type == 26; /* R_PPC64_REL32.  */
    case EM_S390_OLD:
    case EM_S390:
      return reloc_type == 5;  /* R_390_PC32.  */
    case EM_SH:
      return reloc_type == 2;  /* R_SH_REL32.  */
    case EM_SPARC32PLUS:
    case EM_SPARCV9:
    case EM_SPARC:
      return reloc_type == 6;  /* R_SPARC_DISP32.  */
    case EM_SPU:
      return reloc_type == 13; /* R_SPU_REL32.  */
    case EM_TILEGX:
      return reloc_type == 6; /* R_TILEGX_32_PCREL.  */
    case EM_TILEPRO:
      return reloc_type == 4; /* R_TILEPRO_32_PCREL.  */
    case EM_X86_64:
    case EM_L1OM:
    case EM_K1OM:
      return reloc_type == 2;  /* R_X86_64_PC32.  */
    case EM_XTENSA_OLD:
    case EM_XTENSA:
      return reloc_type == 14; /* R_XTENSA_32_PCREL.  */
    default:
      /* Do not abort or issue an error message here.  Not all targets use
	 pc-relative 32-bit relocs in their DWARF debug information and we
	 have already tested for target coverage in is_32bit_abs_reloc.  A
	 more helpful warning message will be generated by apply_relocations
	 anyway, so just return.  */
      return FALSE;
    }
}

/* Like is_32bit_abs_reloc except that it returns TRUE iff RELOC_TYPE is
   a 64-bit absolute RELA relocation used in DWARF debug sections.  */

static bfd_boolean
is_64bit_abs_reloc (unsigned int reloc_type)
{
  switch (elf_header.e_machine)
    {
    case EM_AARCH64:
      return reloc_type == 257;	/* R_AARCH64_ABS64.  */
    case EM_ALPHA:
      return reloc_type == 2; /* R_ALPHA_REFQUAD.  */
    case EM_IA_64:
      return reloc_type == 0x27; /* R_IA64_DIR64LSB.  */
    case EM_PARISC:
      return reloc_type == 80; /* R_PARISC_DIR64.  */
    case EM_PPC64:
      return reloc_type == 38; /* R_PPC64_ADDR64.  */
    case EM_SPARC32PLUS:
    case EM_SPARCV9:
    case EM_SPARC:
      return reloc_type == 54; /* R_SPARC_UA64.  */
    case EM_X86_64:
    case EM_L1OM:
    case EM_K1OM:
      return reloc_type == 1; /* R_X86_64_64.  */
    case EM_S390_OLD:
    case EM_S390:
      return reloc_type == 22;	/* R_S390_64.  */
    case EM_TILEGX:
      return reloc_type == 1; /* R_TILEGX_64.  */
    case EM_MIPS:
      return reloc_type == 18;	/* R_MIPS_64.  */
    default:
      return FALSE;
    }
}

/* Like is_32bit_pcrel_reloc except that it returns TRUE iff RELOC_TYPE is
   a 64-bit pc-relative RELA relocation used in DWARF debug sections.  */

static bfd_boolean
is_64bit_pcrel_reloc (unsigned int reloc_type)
{
  switch (elf_header.e_machine)
    {
    case EM_AARCH64:
      return reloc_type == 260;	/* R_AARCH64_PREL64.  */
    case EM_ALPHA:
      return reloc_type == 11; /* R_ALPHA_SREL64.  */
    case EM_IA_64:
      return reloc_type == 0x4f; /* R_IA64_PCREL64LSB.  */
    case EM_PARISC:
      return reloc_type == 72; /* R_PARISC_PCREL64.  */
    case EM_PPC64:
      return reloc_type == 44; /* R_PPC64_REL64.  */
    case EM_SPARC32PLUS:
    case EM_SPARCV9:
    case EM_SPARC:
      return reloc_type == 46; /* R_SPARC_DISP64.  */
    case EM_X86_64:
    case EM_L1OM:
    case EM_K1OM:
      return reloc_type == 24; /* R_X86_64_PC64.  */
    case EM_S390_OLD:
    case EM_S390:
      return reloc_type == 23;	/* R_S390_PC64.  */
    case EM_TILEGX:
      return reloc_type == 5;  /* R_TILEGX_64_PCREL.  */
    default:
      return FALSE;
    }
}

/* Like is_32bit_abs_reloc except that it returns TRUE iff RELOC_TYPE is
   a 24-bit absolute RELA relocation used in DWARF debug sections.  */

static bfd_boolean
is_24bit_abs_reloc (unsigned int reloc_type)
{
  switch (elf_header.e_machine)
    {
    case EM_CYGNUS_MN10200:
    case EM_MN10200:
      return reloc_type == 4; /* R_MN10200_24.  */
    default:
      return FALSE;
    }
}

/* Like is_32bit_abs_reloc except that it returns TRUE iff RELOC_TYPE is
   a 16-bit absolute RELA relocation used in DWARF debug sections.  */

static bfd_boolean
is_16bit_abs_reloc (unsigned int reloc_type)
{
  switch (elf_header.e_machine)
    {
    case EM_AVR_OLD:
    case EM_AVR:
      return reloc_type == 4; /* R_AVR_16.  */
    case EM_ADAPTEVA_EPIPHANY:
      return reloc_type == 5;
    case EM_CYGNUS_D10V:
    case EM_D10V:
      return reloc_type == 3; /* R_D10V_16.  */
    case EM_H8S:
    case EM_H8_300:
    case EM_H8_300H:
      return reloc_type == R_H8_DIR16;
    case EM_IP2K_OLD:
    case EM_IP2K:
      return reloc_type == 1; /* R_IP2K_16.  */
    case EM_M32C_OLD:
    case EM_M32C:
      return reloc_type == 1; /* R_M32C_16 */
    case EM_MSP430_OLD:
    case EM_MSP430:
      return reloc_type == 5; /* R_MSP430_16_BYTE.  */
    case EM_ALTERA_NIOS2:
    case EM_NIOS32:
      return reloc_type == 9; /* R_NIOS_16.  */
    case EM_TI_C6000:
      return reloc_type == 2; /* R_C6000_ABS16.  */
    case EM_XC16X:
    case EM_C166:
      return reloc_type == 2; /* R_XC16C_ABS_16.  */
    case EM_CYGNUS_MN10200:
    case EM_MN10200:
      return reloc_type == 2; /* R_MN10200_16.  */
    case EM_CYGNUS_MN10300:
    case EM_MN10300:
      return reloc_type == 2; /* R_MN10300_16.  */
    case EM_XGATE:
      return reloc_type == 3; /* R_XGATE_16.  */
    default:
      return FALSE;
    }
}

/* Returns TRUE iff RELOC_TYPE is a NONE relocation used for discarded
   relocation entries (possibly formerly used for SHT_GROUP sections).  */

static bfd_boolean
is_none_reloc (unsigned int reloc_type)
{
  switch (elf_header.e_machine)
    {
    case EM_68K:     /* R_68K_NONE.  */
    case EM_386:     /* R_386_NONE.  */
    case EM_SPARC32PLUS:
    case EM_SPARCV9:
    case EM_SPARC:   /* R_SPARC_NONE.  */
    case EM_MIPS:    /* R_MIPS_NONE.  */
    case EM_PARISC:  /* R_PARISC_NONE.  */
    case EM_ALPHA:   /* R_ALPHA_NONE.  */
    case EM_ADAPTEVA_EPIPHANY:
    case EM_PPC:     /* R_PPC_NONE.  */
    case EM_PPC64:   /* R_PPC64_NONE.  */
    case EM_ARM:     /* R_ARM_NONE.  */
    case EM_IA_64:   /* R_IA64_NONE.  */
    case EM_SH:      /* R_SH_NONE.  */
    case EM_S390_OLD:
    case EM_S390:    /* R_390_NONE.  */
    case EM_CRIS:    /* R_CRIS_NONE.  */
    case EM_X86_64:  /* R_X86_64_NONE.  */
    case EM_L1OM:    /* R_X86_64_NONE.  */
    case EM_K1OM:    /* R_X86_64_NONE.  */
    case EM_MN10300: /* R_MN10300_NONE.  */
    case EM_MOXIE:   /* R_MOXIE_NONE.  */
    case EM_M32R:    /* R_M32R_NONE.  */
    case EM_TI_C6000:/* R_C6000_NONE.  */
    case EM_TILEGX:  /* R_TILEGX_NONE.  */
    case EM_TILEPRO: /* R_TILEPRO_NONE.  */
    case EM_XC16X:
    case EM_C166:    /* R_XC16X_NONE.  */
      return reloc_type == 0;
    case EM_AARCH64:
      return reloc_type == 0 || reloc_type == 256;
    case EM_XTENSA_OLD:
    case EM_XTENSA:
      return (reloc_type == 0      /* R_XTENSA_NONE.  */
	      || reloc_type == 17  /* R_XTENSA_DIFF8.  */
	      || reloc_type == 18  /* R_XTENSA_DIFF16.  */
	      || reloc_type == 19  /* R_XTENSA_DIFF32.  */);
    }
  return FALSE;
}

/* Apply relocations to a section.
   Note: So far support has been added only for those relocations
   which can be found in debug sections.
   FIXME: Add support for more relocations ?  */

static void
apply_relocations (void * file,
		   Elf_Internal_Shdr * section,
		   unsigned char * start)
{
  Elf_Internal_Shdr * relsec;
  unsigned char * end = start + section->sh_size;

  if (elf_header.e_type != ET_REL)
    return;

  /* Find the reloc section associated with the section.  */
  for (relsec = section_headers;
       relsec < section_headers + elf_header.e_shnum;
       ++relsec)
    {
      bfd_boolean is_rela;
      unsigned long num_relocs;
      Elf_Internal_Rela * relocs;
      Elf_Internal_Rela * rp;
      Elf_Internal_Shdr * symsec;
      Elf_Internal_Sym * symtab;
      unsigned long num_syms;
      Elf_Internal_Sym * sym;

      if ((relsec->sh_type != SHT_RELA && relsec->sh_type != SHT_REL)
	  || relsec->sh_info >= elf_header.e_shnum
	  || section_headers + relsec->sh_info != section
	  || relsec->sh_size == 0
	  || relsec->sh_link >= elf_header.e_shnum)
	continue;

      is_rela = relsec->sh_type == SHT_RELA;

      if (is_rela)
	{
	  if (!slurp_rela_relocs ((FILE *) file, relsec->sh_offset,
                                  relsec->sh_size, & relocs, & num_relocs))
	    return;
	}
      else
	{
	  if (!slurp_rel_relocs ((FILE *) file, relsec->sh_offset,
                                 relsec->sh_size, & relocs, & num_relocs))
	    return;
	}

      /* SH uses RELA but uses in place value instead of the addend field.  */
      if (elf_header.e_machine == EM_SH)
	is_rela = FALSE;

      symsec = section_headers + relsec->sh_link;
      symtab = GET_ELF_SYMBOLS ((FILE *) file, symsec, & num_syms);

      for (rp = relocs; rp < relocs + num_relocs; ++rp)
	{
	  bfd_vma         addend;
	  unsigned int    reloc_type;
	  unsigned int    reloc_size;
	  unsigned char * rloc;
	  unsigned long   sym_index;

	  reloc_type = get_reloc_type (rp->r_info);

	  if (target_specific_reloc_handling (rp, start, symtab))
	    continue;
	  else if (is_none_reloc (reloc_type))
	    continue;
	  else if (is_32bit_abs_reloc (reloc_type)
		   || is_32bit_pcrel_reloc (reloc_type))
	    reloc_size = 4;
	  else if (is_64bit_abs_reloc (reloc_type)
		   || is_64bit_pcrel_reloc (reloc_type))
	    reloc_size = 8;
	  else if (is_24bit_abs_reloc (reloc_type))
	    reloc_size = 3;
	  else if (is_16bit_abs_reloc (reloc_type))
	    reloc_size = 2;
	  else
	    {
	      warn (_("unable to apply unsupported reloc type %d to section %s\n"),
		    reloc_type, SECTION_NAME (section));
	      continue;
	    }

	  rloc = start + rp->r_offset;
	  if ((rloc + reloc_size) > end)
	    {
	      warn (_("skipping invalid relocation offset 0x%lx in section %s\n"),
		    (unsigned long) rp->r_offset,
		    SECTION_NAME (section));
	      continue;
	    }

	  sym_index = (unsigned long) get_reloc_symindex (rp->r_info);
	  if (sym_index >= num_syms)
	    {
	      warn (_("skipping invalid relocation symbol index 0x%lx in section %s\n"),
		    sym_index, SECTION_NAME (section));
	      continue;
	    }
	  sym = symtab + sym_index;

	  /* If the reloc has a symbol associated with it,
	     make sure that it is of an appropriate type.

	     Relocations against symbols without type can happen.
	     Gcc -feliminate-dwarf2-dups may generate symbols
	     without type for debug info.

	     Icc generates relocations against function symbols
	     instead of local labels.

	     Relocations against object symbols can happen, eg when
	     referencing a global array.  For an example of this see
	     the _clz.o binary in libgcc.a.  */
	  if (sym != symtab
	      && ELF_ST_TYPE (sym->st_info) > STT_SECTION)
	    {
	      warn (_("skipping unexpected symbol type %s in %ld'th relocation in section %s\n"),
		    get_symbol_type (ELF_ST_TYPE (sym->st_info)),
		    (long int)(rp - relocs),
		    SECTION_NAME (relsec));
	      continue;
	    }

	  addend = 0;
	  if (is_rela)
	    addend += rp->r_addend;
	  /* R_XTENSA_32, R_PJ_DATA_DIR32 and R_D30V_32_NORMAL are
	     partial_inplace.  */
	  if (!is_rela
	      || (elf_header.e_machine == EM_XTENSA
		  && reloc_type == 1)
	      || ((elf_header.e_machine == EM_PJ
		   || elf_header.e_machine == EM_PJ_OLD)
		  && reloc_type == 1)
	      || ((elf_header.e_machine == EM_D30V
		   || elf_header.e_machine == EM_CYGNUS_D30V)
		  && reloc_type == 12))
	    addend += byte_get (rloc, reloc_size);

	  if (is_32bit_pcrel_reloc (reloc_type)
	      || is_64bit_pcrel_reloc (reloc_type))
	    {
	      /* On HPPA, all pc-relative relocations are biased by 8.  */
	      if (elf_header.e_machine == EM_PARISC)
		addend -= 8;
	      byte_put (rloc, (addend + sym->st_value) - rp->r_offset,
		        reloc_size);
	    }
	  else
	    byte_put (rloc, addend + sym->st_value, reloc_size);
	}

      free (symtab);
      free (relocs);
      break;
    }
}

#ifdef SUPPORT_DISASSEMBLY
static int
disassemble_section (Elf_Internal_Shdr * section, FILE * file)
{
  printf (_("\nAssembly dump of section %s\n"),
	  SECTION_NAME (section));

  /* XXX -- to be done --- XXX */

  return 1;
}
#endif

/* Reads in the contents of SECTION from FILE, returning a pointer
   to a malloc'ed buffer or NULL if something went wrong.  */

static char *
get_section_contents (Elf_Internal_Shdr * section, FILE * file)
{
  bfd_size_type num_bytes;

  num_bytes = section->sh_size;

  if (num_bytes == 0 || section->sh_type == SHT_NOBITS)
    {
      printf (_("\nSection '%s' has no data to dump.\n"),
	      SECTION_NAME (section));
      return NULL;
    }

  return  (char *) get_data (NULL, file, section->sh_offset, 1, num_bytes,
                             _("section contents"));
}


static void
dump_section_as_strings (Elf_Internal_Shdr * section, FILE * file)
{
  Elf_Internal_Shdr * relsec;
  bfd_size_type num_bytes;
  char * data;
  char * end;
  char * start;
  char * name = SECTION_NAME (section);
  bfd_boolean some_strings_shown;

  start = get_section_contents (section, file);
  if (start == NULL)
    return;

  printf (_("\nString dump of section '%s':\n"), name);

  /* If the section being dumped has relocations against it the user might
     be expecting these relocations to have been applied.  Check for this
     case and issue a warning message in order to avoid confusion.
     FIXME: Maybe we ought to have an option that dumps a section with
     relocs applied ?  */
  for (relsec = section_headers;
       relsec < section_headers + elf_header.e_shnum;
       ++relsec)
    {
      if ((relsec->sh_type != SHT_RELA && relsec->sh_type != SHT_REL)
	  || relsec->sh_info >= elf_header.e_shnum
	  || section_headers + relsec->sh_info != section
	  || relsec->sh_size == 0
	  || relsec->sh_link >= elf_header.e_shnum)
	continue;

      printf (_("  Note: This section has relocations against it, but these have NOT been applied to this dump.\n"));
      break;
    }

  num_bytes = section->sh_size;
  data = start;
  end  = start + num_bytes;
  some_strings_shown = FALSE;

  while (data < end)
    {
      while (!ISPRINT (* data))
	if (++ data >= end)
	  break;

      if (data < end)
	{
#ifndef __MSVCRT__
	  /* PR 11128: Use two separate invocations in order to work
             around bugs in the Solaris 8 implementation of printf.  */
	  printf ("  [%6tx]  ", data - start);
	  printf ("%s\n", data);
#else
	  printf ("  [%6Ix]  %s\n", (size_t) (data - start), data);
#endif
	  data += strlen (data);
	  some_strings_shown = TRUE;
	}
    }

  if (! some_strings_shown)
    printf (_("  No strings found in this section."));

  free (start);

  putchar ('\n');
}

static void
dump_section_as_bytes (Elf_Internal_Shdr * section,
		       FILE * file,
		       bfd_boolean relocate)
{
  Elf_Internal_Shdr * relsec;
  bfd_size_type bytes;
  bfd_vma addr;
  unsigned char * data;
  unsigned char * start;

  start = (unsigned char *) get_section_contents (section, file);
  if (start == NULL)
    return;

  printf (_("\nHex dump of section '%s':\n"), SECTION_NAME (section));

  if (relocate)
    {
      apply_relocations (file, section, start);
    }
  else
    {
      /* If the section being dumped has relocations against it the user might
	 be expecting these relocations to have been applied.  Check for this
	 case and issue a warning message in order to avoid confusion.
	 FIXME: Maybe we ought to have an option that dumps a section with
	 relocs applied ?  */
      for (relsec = section_headers;
	   relsec < section_headers + elf_header.e_shnum;
	   ++relsec)
	{
	  if ((relsec->sh_type != SHT_RELA && relsec->sh_type != SHT_REL)
	      || relsec->sh_info >= elf_header.e_shnum
	      || section_headers + relsec->sh_info != section
	      || relsec->sh_size == 0
	      || relsec->sh_link >= elf_header.e_shnum)
	    continue;

	  printf (_(" NOTE: This section has relocations against it, but these have NOT been applied to this dump.\n"));
	  break;
	}
    }

  addr = section->sh_addr;
  bytes = section->sh_size;
  data = start;

  while (bytes)
    {
      int j;
      int k;
      int lbytes;

      lbytes = (bytes > 16 ? 16 : bytes);

      printf ("  0x%8.8lx ", (unsigned long) addr);

      for (j = 0; j < 16; j++)
	{
	  if (j < lbytes)
	    printf ("%2.2x", data[j]);
	  else
	    printf ("  ");

	  if ((j & 3) == 3)
	    printf (" ");
	}

      for (j = 0; j < lbytes; j++)
	{
	  k = data[j];
	  if (k >= ' ' && k < 0x7f)
	    printf ("%c", k);
	  else
	    printf (".");
	}

      putchar ('\n');

      data  += lbytes;
      addr  += lbytes;
      bytes -= lbytes;
    }

  free (start);

  putchar ('\n');
}

/* Uncompresses a section that was compressed using zlib, in place.  */

static int
uncompress_section_contents (unsigned char **buffer ATTRIBUTE_UNUSED,
			     dwarf_size_type *size ATTRIBUTE_UNUSED)
{
#ifndef HAVE_ZLIB_H
  return FALSE;
#else
  dwarf_size_type compressed_size = *size;
  unsigned char * compressed_buffer = *buffer;
  dwarf_size_type uncompressed_size;
  unsigned char * uncompressed_buffer;
  z_stream strm;
  int rc;
  dwarf_size_type header_size = 12;

  /* Read the zlib header.  In this case, it should be "ZLIB" followed
     by the uncompressed section size, 8 bytes in big-endian order.  */
  if (compressed_size < header_size
      || ! streq ((char *) compressed_buffer, "ZLIB"))
    return 0;

  uncompressed_size = compressed_buffer[4]; uncompressed_size <<= 8;
  uncompressed_size += compressed_buffer[5]; uncompressed_size <<= 8;
  uncompressed_size += compressed_buffer[6]; uncompressed_size <<= 8;
  uncompressed_size += compressed_buffer[7]; uncompressed_size <<= 8;
  uncompressed_size += compressed_buffer[8]; uncompressed_size <<= 8;
  uncompressed_size += compressed_buffer[9]; uncompressed_size <<= 8;
  uncompressed_size += compressed_buffer[10]; uncompressed_size <<= 8;
  uncompressed_size += compressed_buffer[11];

  /* It is possible the section consists of several compressed
     buffers concatenated together, so we uncompress in a loop.  */
  strm.zalloc = NULL;
  strm.zfree = NULL;
  strm.opaque = NULL;
  strm.avail_in = compressed_size - header_size;
  strm.next_in = (Bytef *) compressed_buffer + header_size;
  strm.avail_out = uncompressed_size;
  uncompressed_buffer = (unsigned char *) xmalloc (uncompressed_size);

  rc = inflateInit (& strm);
  while (strm.avail_in > 0)
    {
      if (rc != Z_OK)
        goto fail;
      strm.next_out = ((Bytef *) uncompressed_buffer
                       + (uncompressed_size - strm.avail_out));
      rc = inflate (&strm, Z_FINISH);
      if (rc != Z_STREAM_END)
        goto fail;
      rc = inflateReset (& strm);
    }
  rc = inflateEnd (& strm);
  if (rc != Z_OK
      || strm.avail_out != 0)
    goto fail;

  free (compressed_buffer);
  *buffer = uncompressed_buffer;
  *size = uncompressed_size;
  return 1;

 fail:
  free (uncompressed_buffer);
  /* Indicate decompression failure.  */
  *buffer = NULL;
  return 0;
#endif  /* HAVE_ZLIB_H */
}

static int
load_specific_debug_section (enum dwarf_section_display_enum debug,
			     Elf_Internal_Shdr * sec, void * file)
{
  struct dwarf_section * section = &debug_displays [debug].section;
  char buf [64];

  /* If it is already loaded, do nothing.  */
  if (section->start != NULL)
    return 1;

  snprintf (buf, sizeof (buf), _("%s section data"), section->name);
  section->address = sec->sh_addr;
  section->start = (unsigned char *) get_data (NULL, (FILE *) file,
                                               sec->sh_offset, 1,
                                               sec->sh_size, buf);
  if (section->start == NULL)
    section->size = 0;
  else
    {
      section->size = sec->sh_size;
      if (uncompress_section_contents (&section->start, &section->size))
	sec->sh_size = section->size;
    }

  if (section->start == NULL)
    return 0;

  if (debug_displays [debug].relocate)
    apply_relocations ((FILE *) file, sec, section->start);

  return 1;
}

int
load_debug_section (enum dwarf_section_display_enum debug, void * file)
{
  struct dwarf_section * section = &debug_displays [debug].section;
  Elf_Internal_Shdr * sec;

  /* Locate the debug section.  */
  sec = find_section (section->uncompressed_name);
  if (sec != NULL)
    section->name = section->uncompressed_name;
  else
    {
      sec = find_section (section->compressed_name);
      if (sec != NULL)
	section->name = section->compressed_name;
    }
  if (sec == NULL)
    return 0;

  return load_specific_debug_section (debug, sec, (FILE *) file);
}

void
free_debug_section (enum dwarf_section_display_enum debug)
{
  struct dwarf_section * section = &debug_displays [debug].section;

  if (section->start == NULL)
    return;

  free ((char *) section->start);
  section->start = NULL;
  section->address = 0;
  section->size = 0;
}

static int
display_debug_section (Elf_Internal_Shdr * section, FILE * file)
{
  char * name = SECTION_NAME (section);
  bfd_size_type length;
  int result = 1;
  int i;

  length = section->sh_size;
  if (length == 0)
    {
      printf (_("\nSection '%s' has no debugging data.\n"), name);
      return 0;
    }
  if (section->sh_type == SHT_NOBITS)
    {
      /* There is no point in dumping the contents of a debugging section
	 which has the NOBITS type - the bits in the file will be random.
	 This can happen when a file containing a .eh_frame section is
	 stripped with the --only-keep-debug command line option.  */
      printf (_("section '%s' has the NOBITS type - its contents are unreliable.\n"), name);
      return 0;
    }

  if (const_strneq (name, ".gnu.linkonce.wi."))
    name = ".debug_info";

  /* See if we know how to display the contents of this section.  */
  for (i = 0; i < max; i++)
    if (streq (debug_displays[i].section.uncompressed_name, name)
        || streq (debug_displays[i].section.compressed_name, name))
      {
	struct dwarf_section * sec = &debug_displays [i].section;
	int secondary = (section != find_section (name));

	if (secondary)
	  free_debug_section ((enum dwarf_section_display_enum) i);

	if (streq (sec->uncompressed_name, name))
	  sec->name = sec->uncompressed_name;
	else
	  sec->name = sec->compressed_name;
	if (load_specific_debug_section ((enum dwarf_section_display_enum) i,
                                         section, file))
	  {
	    result &= debug_displays[i].display (sec, file);

	    if (secondary || (i != info && i != abbrev))
	      free_debug_section ((enum dwarf_section_display_enum) i);
	  }

	break;
      }

  if (i == max)
    {
      printf (_("Unrecognized debug section: %s\n"), name);
      result = 0;
    }

  return result;
}

/* Set DUMP_SECTS for all sections where dumps were requested
   based on section name.  */

static void
initialise_dumps_byname (void)
{
  struct dump_list_entry * cur;

  for (cur = dump_sects_byname; cur; cur = cur->next)
    {
      unsigned int i;
      int any;

      for (i = 0, any = 0; i < elf_header.e_shnum; i++)
	if (streq (SECTION_NAME (section_headers + i), cur->name))
	  {
	    request_dump_bynumber (i, cur->type);
	    any = 1;
	  }

      if (!any)
	warn (_("Section '%s' was not dumped because it does not exist!\n"),
	      cur->name);
    }
}

static void
process_section_contents (FILE * file)
{
  Elf_Internal_Shdr * section;
  unsigned int i;

  if (! do_dump)
    return;

  initialise_dumps_byname ();

  for (i = 0, section = section_headers;
       i < elf_header.e_shnum && i < num_dump_sects;
       i++, section++)
    {
#ifdef SUPPORT_DISASSEMBLY
      if (dump_sects[i] & DISASS_DUMP)
	disassemble_section (section, file);
#endif
      if (dump_sects[i] & HEX_DUMP)
	dump_section_as_bytes (section, file, FALSE);

      if (dump_sects[i] & RELOC_DUMP)
	dump_section_as_bytes (section, file, TRUE);

      if (dump_sects[i] & STRING_DUMP)
	dump_section_as_strings (section, file);

      if (dump_sects[i] & DEBUG_DUMP)
	display_debug_section (section, file);
    }

  /* Check to see if the user requested a
     dump of a section that does not exist.  */
  while (i++ < num_dump_sects)
    if (dump_sects[i])
      warn (_("Section %d was not dumped because it does not exist!\n"), i);
}

static void
process_mips_fpe_exception (int mask)
{
  if (mask)
    {
      int first = 1;
      if (mask & OEX_FPU_INEX)
	fputs ("INEX", stdout), first = 0;
      if (mask & OEX_FPU_UFLO)
	printf ("%sUFLO", first ? "" : "|"), first = 0;
      if (mask & OEX_FPU_OFLO)
	printf ("%sOFLO", first ? "" : "|"), first = 0;
      if (mask & OEX_FPU_DIV0)
	printf ("%sDIV0", first ? "" : "|"), first = 0;
      if (mask & OEX_FPU_INVAL)
	printf ("%sINVAL", first ? "" : "|");
    }
  else
    fputs ("0", stdout);
}

/* ARM EABI attributes section.  */
typedef struct
{
  int tag;
  const char * name;
  /* 0 = special, 1 = string, 2 = uleb123, > 0x80 == table lookup.  */
  int type;
  const char ** table;
} arm_attr_public_tag;

static const char * arm_attr_tag_CPU_arch[] =
  {"Pre-v4", "v4", "v4T", "v5T", "v5TE", "v5TEJ", "v6", "v6KZ", "v6T2",
   "v6K", "v7", "v6-M", "v6S-M", "v7E-M", "v8"};
static const char * arm_attr_tag_ARM_ISA_use[] = {"No", "Yes"};
static const char * arm_attr_tag_THUMB_ISA_use[] =
  {"No", "Thumb-1", "Thumb-2"};
static const char * arm_attr_tag_FP_arch[] =
  {"No", "VFPv1", "VFPv2", "VFPv3", "VFPv3-D16", "VFPv4", "VFPv4-D16",
   "FP for ARMv8"};
static const char * arm_attr_tag_WMMX_arch[] = {"No", "WMMXv1", "WMMXv2"};
static const char * arm_attr_tag_Advanced_SIMD_arch[] =
  {"No", "NEONv1", "NEONv1 with Fused-MAC", "NEON for ARMv8"};
static const char * arm_attr_tag_PCS_config[] =
  {"None", "Bare platform", "Linux application", "Linux DSO", "PalmOS 2004",
   "PalmOS (reserved)", "SymbianOS 2004", "SymbianOS (reserved)"};
static const char * arm_attr_tag_ABI_PCS_R9_use[] =
  {"V6", "SB", "TLS", "Unused"};
static const char * arm_attr_tag_ABI_PCS_RW_data[] =
  {"Absolute", "PC-relative", "SB-relative", "None"};
static const char * arm_attr_tag_ABI_PCS_RO_data[] =
  {"Absolute", "PC-relative", "None"};
static const char * arm_attr_tag_ABI_PCS_GOT_use[] =
  {"None", "direct", "GOT-indirect"};
static const char * arm_attr_tag_ABI_PCS_wchar_t[] =
  {"None", "??? 1", "2", "??? 3", "4"};
static const char * arm_attr_tag_ABI_FP_rounding[] = {"Unused", "Needed"};
static const char * arm_attr_tag_ABI_FP_denormal[] =
  {"Unused", "Needed", "Sign only"};
static const char * arm_attr_tag_ABI_FP_exceptions[] = {"Unused", "Needed"};
static const char * arm_attr_tag_ABI_FP_user_exceptions[] = {"Unused", "Needed"};
static const char * arm_attr_tag_ABI_FP_number_model[] =
  {"Unused", "Finite", "RTABI", "IEEE 754"};
static const char * arm_attr_tag_ABI_enum_size[] =
  {"Unused", "small", "int", "forced to int"};
static const char * arm_attr_tag_ABI_HardFP_use[] =
  {"As Tag_FP_arch", "SP only", "DP only", "SP and DP"};
static const char * arm_attr_tag_ABI_VFP_args[] =
  {"AAPCS", "VFP registers", "custom"};
static const char * arm_attr_tag_ABI_WMMX_args[] =
  {"AAPCS", "WMMX registers", "custom"};
static const char * arm_attr_tag_ABI_optimization_goals[] =
  {"None", "Prefer Speed", "Aggressive Speed", "Prefer Size",
    "Aggressive Size", "Prefer Debug", "Aggressive Debug"};
static const char * arm_attr_tag_ABI_FP_optimization_goals[] =
  {"None", "Prefer Speed", "Aggressive Speed", "Prefer Size",
    "Aggressive Size", "Prefer Accuracy", "Aggressive Accuracy"};
static const char * arm_attr_tag_CPU_unaligned_access[] = {"None", "v6"};
static const char * arm_attr_tag_FP_HP_extension[] =
  {"Not Allowed", "Allowed"};
static const char * arm_attr_tag_ABI_FP_16bit_format[] =
  {"None", "IEEE 754", "Alternative Format"};
static const char * arm_attr_tag_MPextension_use[] =
  {"Not Allowed", "Allowed"};
static const char * arm_attr_tag_DIV_use[] =
  {"Allowed in Thumb-ISA, v7-R or v7-M", "Not allowed",
    "Allowed in v7-A with integer division extension"};
static const char * arm_attr_tag_T2EE_use[] = {"Not Allowed", "Allowed"};
static const char * arm_attr_tag_Virtualization_use[] =
  {"Not Allowed", "TrustZone", "Virtualization Extensions",
    "TrustZone and Virtualization Extensions"};
static const char * arm_attr_tag_MPextension_use_legacy[] =
  {"Not Allowed", "Allowed"};

#define LOOKUP(id, name) \
  {id, #name, 0x80 | ARRAY_SIZE(arm_attr_tag_##name), arm_attr_tag_##name}
static arm_attr_public_tag arm_attr_public_tags[] =
{
  {4, "CPU_raw_name", 1, NULL},
  {5, "CPU_name", 1, NULL},
  LOOKUP(6, CPU_arch),
  {7, "CPU_arch_profile", 0, NULL},
  LOOKUP(8, ARM_ISA_use),
  LOOKUP(9, THUMB_ISA_use),
  LOOKUP(10, FP_arch),
  LOOKUP(11, WMMX_arch),
  LOOKUP(12, Advanced_SIMD_arch),
  LOOKUP(13, PCS_config),
  LOOKUP(14, ABI_PCS_R9_use),
  LOOKUP(15, ABI_PCS_RW_data),
  LOOKUP(16, ABI_PCS_RO_data),
  LOOKUP(17, ABI_PCS_GOT_use),
  LOOKUP(18, ABI_PCS_wchar_t),
  LOOKUP(19, ABI_FP_rounding),
  LOOKUP(20, ABI_FP_denormal),
  LOOKUP(21, ABI_FP_exceptions),
  LOOKUP(22, ABI_FP_user_exceptions),
  LOOKUP(23, ABI_FP_number_model),
  {24, "ABI_align_needed", 0, NULL},
  {25, "ABI_align_preserved", 0, NULL},
  LOOKUP(26, ABI_enum_size),
  LOOKUP(27, ABI_HardFP_use),
  LOOKUP(28, ABI_VFP_args),
  LOOKUP(29, ABI_WMMX_args),
  LOOKUP(30, ABI_optimization_goals),
  LOOKUP(31, ABI_FP_optimization_goals),
  {32, "compatibility", 0, NULL},
  LOOKUP(34, CPU_unaligned_access),
  LOOKUP(36, FP_HP_extension),
  LOOKUP(38, ABI_FP_16bit_format),
  LOOKUP(42, MPextension_use),
  LOOKUP(44, DIV_use),
  {64, "nodefaults", 0, NULL},
  {65, "also_compatible_with", 0, NULL},
  LOOKUP(66, T2EE_use),
  {67, "conformance", 1, NULL},
  LOOKUP(68, Virtualization_use),
  LOOKUP(70, MPextension_use_legacy)
};
#undef LOOKUP

static unsigned char *
display_arm_attribute (unsigned char * p)
{
  int tag;
  unsigned int len;
  int val;
  arm_attr_public_tag * attr;
  unsigned i;
  int type;

  tag = read_uleb128 (p, &len);
  p += len;
  attr = NULL;
  for (i = 0; i < ARRAY_SIZE (arm_attr_public_tags); i++)
    {
      if (arm_attr_public_tags[i].tag == tag)
	{
	  attr = &arm_attr_public_tags[i];
	  break;
	}
    }

  if (attr)
    {
      printf ("  Tag_%s: ", attr->name);
      switch (attr->type)
	{
	case 0:
	  switch (tag)
	    {
	    case 7: /* Tag_CPU_arch_profile.  */
	      val = read_uleb128 (p, &len);
	      p += len;
	      switch (val)
		{
		case 0: printf (_("None\n")); break;
		case 'A': printf (_("Application\n")); break;
		case 'R': printf (_("Realtime\n")); break;
		case 'M': printf (_("Microcontroller\n")); break;
		case 'S': printf (_("Application or Realtime\n")); break;
		default: printf ("??? (%d)\n", val); break;
		}
	      break;

	    case 24: /* Tag_align_needed.  */
	      val = read_uleb128 (p, &len);
	      p += len;
	      switch (val)
		{
		case 0: printf (_("None\n")); break;
		case 1: printf (_("8-byte\n")); break;
		case 2: printf (_("4-byte\n")); break;
		case 3: printf ("??? 3\n"); break;
		default:
		  if (val <= 12)
		    printf (_("8-byte and up to %d-byte extended\n"),
			    1 << val);
		  else
		    printf ("??? (%d)\n", val);
		  break;
		}
	      break;

	    case 25: /* Tag_align_preserved.  */
	      val = read_uleb128 (p, &len);
	      p += len;
	      switch (val)
		{
		case 0: printf (_("None\n")); break;
		case 1: printf (_("8-byte, except leaf SP\n")); break;
		case 2: printf (_("8-byte\n")); break;
		case 3: printf ("??? 3\n"); break;
		default:
		  if (val <= 12)
		    printf (_("8-byte and up to %d-byte extended\n"),
			    1 << val);
		  else
		    printf ("??? (%d)\n", val);
		  break;
		}
	      break;

	    case 32: /* Tag_compatibility.  */
	      val = read_uleb128 (p, &len);
	      p += len;
	      printf (_("flag = %d, vendor = %s\n"), val, p);
	      p += strlen ((char *) p) + 1;
	      break;

	    case 64: /* Tag_nodefaults.  */
	      p++;
	      printf (_("True\n"));
	      break;

	    case 65: /* Tag_also_compatible_with.  */
	      val = read_uleb128 (p, &len);
	      p += len;
	      if (val == 6 /* Tag_CPU_arch.  */)
		{
		  val = read_uleb128 (p, &len);
		  p += len;
		  if ((unsigned int)val >= ARRAY_SIZE (arm_attr_tag_CPU_arch))
		    printf ("??? (%d)\n", val);
		  else
		    printf ("%s\n", arm_attr_tag_CPU_arch[val]);
		}
	      else
		printf ("???\n");
	      while (*(p++) != '\0' /* NUL terminator.  */);
	      break;

	    default:
	      abort ();
	    }
	  return p;

	case 1:
	case 2:
	  type = attr->type;
	  break;

	default:
	  assert (attr->type & 0x80);
	  val = read_uleb128 (p, &len);
	  p += len;
	  type = attr->type & 0x7f;
	  if (val >= type)
	    printf ("??? (%d)\n", val);
	  else
	    printf ("%s\n", attr->table[val]);
	  return p;
	}
    }
  else
    {
      if (tag & 1)
	type = 1; /* String.  */
      else
	type = 2; /* uleb128.  */
      printf ("  Tag_unknown_%d: ", tag);
    }

  if (type == 1)
    {
      printf ("\"%s\"\n", p);
      p += strlen ((char *) p) + 1;
    }
  else
    {
      val = read_uleb128 (p, &len);
      p += len;
      printf ("%d (0x%x)\n", val, val);
    }

  return p;
}

static unsigned char *
display_gnu_attribute (unsigned char * p,
		       unsigned char * (* display_proc_gnu_attribute) (unsigned char *, int))
{
  int tag;
  unsigned int len;
  int val;
  int type;

  tag = read_uleb128 (p, &len);
  p += len;

  /* Tag_compatibility is the only generic GNU attribute defined at
     present.  */
  if (tag == 32)
    {
      val = read_uleb128 (p, &len);
      p += len;
      printf (_("flag = %d, vendor = %s\n"), val, p);
      p += strlen ((char *) p) + 1;
      return p;
    }

  if ((tag & 2) == 0 && display_proc_gnu_attribute)
    return display_proc_gnu_attribute (p, tag);

  if (tag & 1)
    type = 1; /* String.  */
  else
    type = 2; /* uleb128.  */
  printf ("  Tag_unknown_%d: ", tag);

  if (type == 1)
    {
      printf ("\"%s\"\n", p);
      p += strlen ((char *) p) + 1;
    }
  else
    {
      val = read_uleb128 (p, &len);
      p += len;
      printf ("%d (0x%x)\n", val, val);
    }

  return p;
}

static unsigned char *
display_power_gnu_attribute (unsigned char * p, int tag)
{
  int type;
  unsigned int len;
  int val;

  if (tag == Tag_GNU_Power_ABI_FP)
    {
      val = read_uleb128 (p, &len);
      p += len;
      printf ("  Tag_GNU_Power_ABI_FP: ");

      switch (val)
	{
	case 0:
	  printf (_("Hard or soft float\n"));
	  break;
	case 1:
	  printf (_("Hard float\n"));
	  break;
	case 2:
	  printf (_("Soft float\n"));
	  break;
	case 3:
	  printf (_("Single-precision hard float\n"));
	  break;
	default:
	  printf ("??? (%d)\n", val);
	  break;
	}
      return p;
   }

  if (tag == Tag_GNU_Power_ABI_Vector)
    {
      val = read_uleb128 (p, &len);
      p += len;
      printf ("  Tag_GNU_Power_ABI_Vector: ");
      switch (val)
	{
	case 0:
	  printf (_("Any\n"));
	  break;
	case 1:
	  printf (_("Generic\n"));
	  break;
	case 2:
	  printf ("AltiVec\n");
	  break;
	case 3:
	  printf ("SPE\n");
	  break;
	default:
	  printf ("??? (%d)\n", val);
	  break;
	}
      return p;
   }

  if (tag == Tag_GNU_Power_ABI_Struct_Return)
    {
      val = read_uleb128 (p, &len);
      p += len;
      printf ("  Tag_GNU_Power_ABI_Struct_Return: ");
      switch (val)
       {
       case 0:
         printf (_("Any\n"));
         break;
       case 1:
         printf ("r3/r4\n");
         break;
       case 2:
         printf (_("Memory\n"));
         break;
       default:
         printf ("??? (%d)\n", val);
         break;
       }
      return p;
    }

  if (tag & 1)
    type = 1; /* String.  */
  else
    type = 2; /* uleb128.  */
  printf ("  Tag_unknown_%d: ", tag);

  if (type == 1)
    {
      printf ("\"%s\"\n", p);
      p += strlen ((char *) p) + 1;
    }
  else
    {
      val = read_uleb128 (p, &len);
      p += len;
      printf ("%d (0x%x)\n", val, val);
    }

  return p;
}

static void
display_sparc_hwcaps (int mask)
{
  if (mask)
    {
      int first = 1;
      if (mask & ELF_SPARC_HWCAP_MUL32)
	fputs ("mul32", stdout), first = 0;
      if (mask & ELF_SPARC_HWCAP_DIV32)
	printf ("%sdiv32", first ? "" : "|"), first = 0;
      if (mask & ELF_SPARC_HWCAP_FSMULD)
	printf ("%sfsmuld", first ? "" : "|"), first = 0;
      if (mask & ELF_SPARC_HWCAP_V8PLUS)
	printf ("%sv8plus", first ? "" : "|"), first = 0;
      if (mask & ELF_SPARC_HWCAP_POPC)
	printf ("%spopc", first ? "" : "|"), first = 0;
      if (mask & ELF_SPARC_HWCAP_VIS)
	printf ("%svis", first ? "" : "|"), first = 0;
      if (mask & ELF_SPARC_HWCAP_VIS2)
	printf ("%svis2", first ? "" : "|"), first = 0;
      if (mask & ELF_SPARC_HWCAP_ASI_BLK_INIT)
	printf ("%sASIBlkInit", first ? "" : "|"), first = 0;
      if (mask & ELF_SPARC_HWCAP_FMAF)
	printf ("%sfmaf", first ? "" : "|"), first = 0;
      if (mask & ELF_SPARC_HWCAP_VIS3)
	printf ("%svis3", first ? "" : "|"), first = 0;
      if (mask & ELF_SPARC_HWCAP_HPC)
	printf ("%shpc", first ? "" : "|"), first = 0;
      if (mask & ELF_SPARC_HWCAP_RANDOM)
	printf ("%srandom", first ? "" : "|"), first = 0;
      if (mask & ELF_SPARC_HWCAP_TRANS)
	printf ("%strans", first ? "" : "|"), first = 0;
      if (mask & ELF_SPARC_HWCAP_FJFMAU)
	printf ("%sfjfmau", first ? "" : "|"), first = 0;
      if (mask & ELF_SPARC_HWCAP_IMA)
	printf ("%sima", first ? "" : "|"), first = 0;
      if (mask & ELF_SPARC_HWCAP_ASI_CACHE_SPARING)
	printf ("%scspare", first ? "" : "|"), first = 0;
    }
  else
    fputc('0', stdout);
  fputc('\n', stdout);
}

static unsigned char *
display_sparc_gnu_attribute (unsigned char * p, int tag)
{
  int type;
  unsigned int len;
  int val;

  if (tag == Tag_GNU_Sparc_HWCAPS)
    {
      val = read_uleb128 (p, &len);
      p += len;
      printf ("  Tag_GNU_Sparc_HWCAPS: ");

      display_sparc_hwcaps (val);
      return p;
   }

  if (tag & 1)
    type = 1; /* String.  */
  else
    type = 2; /* uleb128.  */
  printf ("  Tag_unknown_%d: ", tag);

  if (type == 1)
    {
      printf ("\"%s\"\n", p);
      p += strlen ((char *) p) + 1;
    }
  else
    {
      val = read_uleb128 (p, &len);
      p += len;
      printf ("%d (0x%x)\n", val, val);
    }

  return p;
}

static unsigned char *
display_mips_gnu_attribute (unsigned char * p, int tag)
{
  int type;
  unsigned int len;
  int val;

  if (tag == Tag_GNU_MIPS_ABI_FP)
    {
      val = read_uleb128 (p, &len);
      p += len;
      printf ("  Tag_GNU_MIPS_ABI_FP: ");

      switch (val)
	{
	case 0:
	  printf (_("Hard or soft float\n"));
	  break;
	case 1:
	  printf (_("Hard float (double precision)\n"));
	  break;
	case 2:
	  printf (_("Hard float (single precision)\n"));
	  break;
	case 3:
	  printf (_("Soft float\n"));
	  break;
	case 4:
	  printf (_("Hard float (MIPS32r2 64-bit FPU)\n"));
	  break;
	default:
	  printf ("??? (%d)\n", val);
	  break;
	}
      return p;
   }

  if (tag & 1)
    type = 1; /* String.  */
  else
    type = 2; /* uleb128.  */
  printf ("  Tag_unknown_%d: ", tag);

  if (type == 1)
    {
      printf ("\"%s\"\n", p);
      p += strlen ((char *) p) + 1;
    }
  else
    {
      val = read_uleb128 (p, &len);
      p += len;
      printf ("%d (0x%x)\n", val, val);
    }

  return p;
}

static unsigned char *
display_tic6x_attribute (unsigned char * p)
{
  int tag;
  unsigned int len;
  int val;

  tag = read_uleb128 (p, &len);
  p += len;

  switch (tag)
    {
    case Tag_ISA:
      val = read_uleb128 (p, &len);
      p += len;
      printf ("  Tag_ISA: ");

      switch (val)
	{
	case C6XABI_Tag_ISA_none:
	  printf (_("None\n"));
	  break;
	case C6XABI_Tag_ISA_C62X:
	  printf ("C62x\n");
	  break;
	case C6XABI_Tag_ISA_C67X:
	  printf ("C67x\n");
	  break;
	case C6XABI_Tag_ISA_C67XP:
	  printf ("C67x+\n");
	  break;
	case C6XABI_Tag_ISA_C64X:
	  printf ("C64x\n");
	  break;
	case C6XABI_Tag_ISA_C64XP:
	  printf ("C64x+\n");
	  break;
	case C6XABI_Tag_ISA_C674X:
	  printf ("C674x\n");
	  break;
	default:
	  printf ("??? (%d)\n", val);
	  break;
	}
      return p;

    case Tag_ABI_wchar_t:
      val = read_uleb128 (p, &len);
      p += len;
      printf ("  Tag_ABI_wchar_t: ");
      switch (val)
	{
	case 0:
	  printf (_("Not used\n"));
	  break;
	case 1:
	  printf (_("2 bytes\n"));
	  break;
	case 2:
	  printf (_("4 bytes\n"));
	  break;
	default:
	  printf ("??? (%d)\n", val);
	  break;
	}
      return p;

    case Tag_ABI_stack_align_needed:
      val = read_uleb128 (p, &len);
      p += len;
      printf ("  Tag_ABI_stack_align_needed: ");
      switch (val)
	{
	case 0:
	  printf (_("8-byte\n"));
	  break;
	case 1:
	  printf (_("16-byte\n"));
	  break;
	default:
	  printf ("??? (%d)\n", val);
	  break;
	}
      return p;

    case Tag_ABI_stack_align_preserved:
      val = read_uleb128 (p, &len);
      p += len;
      printf ("  Tag_ABI_stack_align_preserved: ");
      switch (val)
	{
	case 0:
	  printf (_("8-byte\n"));
	  break;
	case 1:
	  printf (_("16-byte\n"));
	  break;
	default:
	  printf ("??? (%d)\n", val);
	  break;
	}
      return p;

    case Tag_ABI_DSBT:
      val = read_uleb128 (p, &len);
      p += len;
      printf ("  Tag_ABI_DSBT: ");
      switch (val)
	{
	case 0:
	  printf (_("DSBT addressing not used\n"));
	  break;
	case 1:
	  printf (_("DSBT addressing used\n"));
	  break;
	default:
	  printf ("??? (%d)\n", val);
	  break;
	}
      return p;

    case Tag_ABI_PID:
      val = read_uleb128 (p, &len);
      p += len;
      printf ("  Tag_ABI_PID: ");
      switch (val)
	{
	case 0:
	  printf (_("Data addressing position-dependent\n"));
	  break;
	case 1:
	  printf (_("Data addressing position-independent, GOT near DP\n"));
	  break;
	case 2:
	  printf (_("Data addressing position-independent, GOT far from DP\n"));
	  break;
	default:
	  printf ("??? (%d)\n", val);
	  break;
	}
      return p;

    case Tag_ABI_PIC:
      val = read_uleb128 (p, &len);
      p += len;
      printf ("  Tag_ABI_PIC: ");
      switch (val)
	{
	case 0:
	  printf (_("Code addressing position-dependent\n"));
	  break;
	case 1:
	  printf (_("Code addressing position-independent\n"));
	  break;
	default:
	  printf ("??? (%d)\n", val);
	  break;
	}
      return p;

    case Tag_ABI_array_object_alignment:
      val = read_uleb128 (p, &len);
      p += len;
      printf ("  Tag_ABI_array_object_alignment: ");
      switch (val)
	{
	case 0:
	  printf (_("8-byte\n"));
	  break;
	case 1:
	  printf (_("4-byte\n"));
	  break;
	case 2:
	  printf (_("16-byte\n"));
	  break;
	default:
	  printf ("??? (%d)\n", val);
	  break;
	}
      return p;

    case Tag_ABI_array_object_align_expected:
      val = read_uleb128 (p, &len);
      p += len;
      printf ("  Tag_ABI_array_object_align_expected: ");
      switch (val)
	{
	case 0:
	  printf (_("8-byte\n"));
	  break;
	case 1:
	  printf (_("4-byte\n"));
	  break;
	case 2:
	  printf (_("16-byte\n"));
	  break;
	default:
	  printf ("??? (%d)\n", val);
	  break;
	}
      return p;

    case Tag_ABI_compatibility:
      val = read_uleb128 (p, &len);
      p += len;
      printf ("  Tag_ABI_compatibility: ");
      printf (_("flag = %d, vendor = %s\n"), val, p);
      p += strlen ((char *) p) + 1;
      return p;

    case Tag_ABI_conformance:
      printf ("  Tag_ABI_conformance: ");
      printf ("\"%s\"\n", p);
      p += strlen ((char *) p) + 1;
      return p;
    }

  printf ("  Tag_unknown_%d: ", tag);

  if (tag & 1)
    {
      printf ("\"%s\"\n", p);
      p += strlen ((char *) p) + 1;
    }
  else
    {
      val = read_uleb128 (p, &len);
      p += len;
      printf ("%d (0x%x)\n", val, val);
    }

  return p;
}

static int
process_attributes (FILE * file,
		    const char * public_name,
		    unsigned int proc_type,
		    unsigned char * (* display_pub_attribute) (unsigned char *),
		    unsigned char * (* display_proc_gnu_attribute) (unsigned char *, int))
{
  Elf_Internal_Shdr * sect;
  unsigned char * contents;
  unsigned char * p;
  unsigned char * end;
  bfd_vma section_len;
  bfd_vma len;
  unsigned i;

  /* Find the section header so that we get the size.  */
  for (i = 0, sect = section_headers;
       i < elf_header.e_shnum;
       i++, sect++)
    {
      if (sect->sh_type != proc_type && sect->sh_type != SHT_GNU_ATTRIBUTES)
	continue;

      contents = (unsigned char *) get_data (NULL, file, sect->sh_offset, 1,
                                             sect->sh_size, _("attributes"));
      if (contents == NULL)
	continue;

      p = contents;
      if (*p == 'A')
	{
	  len = sect->sh_size - 1;
	  p++;

	  while (len > 0)
	    {
	      int namelen;
	      bfd_boolean public_section;
	      bfd_boolean gnu_section;

	      section_len = byte_get (p, 4);
	      p += 4;

	      if (section_len > len)
		{
		  printf (_("ERROR: Bad section length (%d > %d)\n"),
			  (int) section_len, (int) len);
		  section_len = len;
		}

	      len -= section_len;
	      printf (_("Attribute Section: %s\n"), p);

	      if (public_name && streq ((char *) p, public_name))
		public_section = TRUE;
	      else
		public_section = FALSE;

	      if (streq ((char *) p, "gnu"))
		gnu_section = TRUE;
	      else
		gnu_section = FALSE;

	      namelen = strlen ((char *) p) + 1;
	      p += namelen;
	      section_len -= namelen + 4;

	      while (section_len > 0)
		{
		  int tag = *(p++);
		  int val;
		  bfd_vma size;

		  size = byte_get (p, 4);
		  if (size > section_len)
		    {
		      printf (_("ERROR: Bad subsection length (%d > %d)\n"),
			      (int) size, (int) section_len);
		      size = section_len;
		    }

		  section_len -= size;
		  end = p + size - 1;
		  p += 4;

		  switch (tag)
		    {
		    case 1:
		      printf (_("File Attributes\n"));
		      break;
		    case 2:
		      printf (_("Section Attributes:"));
		      goto do_numlist;
		    case 3:
		      printf (_("Symbol Attributes:"));
		    do_numlist:
		      for (;;)
			{
			  unsigned int j;

			  val = read_uleb128 (p, &j);
			  p += j;
			  if (val == 0)
			    break;
			  printf (" %d", val);
			}
		      printf ("\n");
		      break;
		    default:
		      printf (_("Unknown tag: %d\n"), tag);
		      public_section = FALSE;
		      break;
		    }

		  if (public_section)
		    {
		      while (p < end)
			p = display_pub_attribute (p);
		    }
		  else if (gnu_section)
		    {
		      while (p < end)
			p = display_gnu_attribute (p,
						   display_proc_gnu_attribute);
		    }
		  else
		    {
		      /* ??? Do something sensible, like dump hex.  */
		      printf (_("  Unknown section contexts\n"));
		      p = end;
		    }
		}
	    }
	}
      else
	printf (_("Unknown format '%c'\n"), *p);

      free (contents);
    }
  return 1;
}

static int
process_arm_specific (FILE * file)
{
  return process_attributes (file, "aeabi", SHT_ARM_ATTRIBUTES,
			     display_arm_attribute, NULL);
}

static int
process_power_specific (FILE * file)
{
  return process_attributes (file, NULL, SHT_GNU_ATTRIBUTES, NULL,
			     display_power_gnu_attribute);
}

static int
process_sparc_specific (FILE * file)
{
  return process_attributes (file, NULL, SHT_GNU_ATTRIBUTES, NULL,
			     display_sparc_gnu_attribute);
}

static int
process_tic6x_specific (FILE * file)
{
  return process_attributes (file, "c6xabi", SHT_C6000_ATTRIBUTES,
			     display_tic6x_attribute, NULL);
}

/* DATA points to the contents of a MIPS GOT that starts at VMA PLTGOT.
   Print the Address, Access and Initial fields of an entry at VMA ADDR
   and return the VMA of the next entry.  */

static bfd_vma
print_mips_got_entry (unsigned char * data, bfd_vma pltgot, bfd_vma addr)
{
  printf ("  ");
  print_vma (addr, LONG_HEX);
  printf (" ");
  if (addr < pltgot + 0xfff0)
    printf ("%6d(gp)", (int) (addr - pltgot - 0x7ff0));
  else
    printf ("%10s", "");
  printf (" ");
  if (data == NULL)
    printf ("%*s", is_32bit_elf ? 8 : 16, _("<unknown>"));
  else
    {
      bfd_vma entry;

      entry = byte_get (data + addr - pltgot, is_32bit_elf ? 4 : 8);
      print_vma (entry, LONG_HEX);
    }
  return addr + (is_32bit_elf ? 4 : 8);
}

/* DATA points to the contents of a MIPS PLT GOT that starts at VMA
   PLTGOT.  Print the Address and Initial fields of an entry at VMA
   ADDR and return the VMA of the next entry.  */

static bfd_vma
print_mips_pltgot_entry (unsigned char * data, bfd_vma pltgot, bfd_vma addr)
{
  printf ("  ");
  print_vma (addr, LONG_HEX);
  printf (" ");
  if (data == NULL)
    printf ("%*s", is_32bit_elf ? 8 : 16, _("<unknown>"));
  else
    {
      bfd_vma entry;

      entry = byte_get (data + addr - pltgot, is_32bit_elf ? 4 : 8);
      print_vma (entry, LONG_HEX);
    }
  return addr + (is_32bit_elf ? 4 : 8);
}

static int
process_mips_specific (FILE * file)
{
  Elf_Internal_Dyn * entry;
  size_t liblist_offset = 0;
  size_t liblistno = 0;
  size_t conflictsno = 0;
  size_t options_offset = 0;
  size_t conflicts_offset = 0;
  size_t pltrelsz = 0;
  size_t pltrel = 0;
  bfd_vma pltgot = 0;
  bfd_vma mips_pltgot = 0;
  bfd_vma jmprel = 0;
  bfd_vma local_gotno = 0;
  bfd_vma gotsym = 0;
  bfd_vma symtabno = 0;

  process_attributes (file, NULL, SHT_GNU_ATTRIBUTES, NULL,
		      display_mips_gnu_attribute);

  /* We have a lot of special sections.  Thanks SGI!  */
  if (dynamic_section == NULL)
    /* No information available.  */
    return 0;

  for (entry = dynamic_section; entry->d_tag != DT_NULL; ++entry)
    switch (entry->d_tag)
      {
      case DT_MIPS_LIBLIST:
	liblist_offset
	  = offset_from_vma (file, entry->d_un.d_val,
			     liblistno * sizeof (Elf32_External_Lib));
	break;
      case DT_MIPS_LIBLISTNO:
	liblistno = entry->d_un.d_val;
	break;
      case DT_MIPS_OPTIONS:
	options_offset = offset_from_vma (file, entry->d_un.d_val, 0);
	break;
      case DT_MIPS_CONFLICT:
	conflicts_offset
	  = offset_from_vma (file, entry->d_un.d_val,
			     conflictsno * sizeof (Elf32_External_Conflict));
	break;
      case DT_MIPS_CONFLICTNO:
	conflictsno = entry->d_un.d_val;
	break;
      case DT_PLTGOT:
	pltgot = entry->d_un.d_ptr;
	break;
      case DT_MIPS_LOCAL_GOTNO:
	local_gotno = entry->d_un.d_val;
	break;
      case DT_MIPS_GOTSYM:
	gotsym = entry->d_un.d_val;
	break;
      case DT_MIPS_SYMTABNO:
	symtabno = entry->d_un.d_val;
	break;
      case DT_MIPS_PLTGOT:
	mips_pltgot = entry->d_un.d_ptr;
	break;
      case DT_PLTREL:
	pltrel = entry->d_un.d_val;
	break;
      case DT_PLTRELSZ:
	pltrelsz = entry->d_un.d_val;
	break;
      case DT_JMPREL:
	jmprel = entry->d_un.d_ptr;
	break;
      default:
	break;
      }

  if (liblist_offset != 0 && liblistno != 0 && do_dynamic)
    {
      Elf32_External_Lib * elib;
      size_t cnt;

      elib = (Elf32_External_Lib *) get_data (NULL, file, liblist_offset,
                                              liblistno,
                                              sizeof (Elf32_External_Lib),
                                              _("liblist section data"));
      if (elib)
	{
	  printf (_("\nSection '.liblist' contains %lu entries:\n"),
		  (unsigned long) liblistno);
	  fputs (_("     Library              Time Stamp          Checksum   Version Flags\n"),
		 stdout);

	  for (cnt = 0; cnt < liblistno; ++cnt)
	    {
	      Elf32_Lib liblist;
	      time_t atime;
	      char timebuf[20];
	      struct tm * tmp;

	      liblist.l_name = BYTE_GET (elib[cnt].l_name);
	      atime = BYTE_GET (elib[cnt].l_time_stamp);
	      liblist.l_checksum = BYTE_GET (elib[cnt].l_checksum);
	      liblist.l_version = BYTE_GET (elib[cnt].l_version);
	      liblist.l_flags = BYTE_GET (elib[cnt].l_flags);

	      tmp = gmtime (&atime);
	      snprintf (timebuf, sizeof (timebuf),
			"%04u-%02u-%02uT%02u:%02u:%02u",
			tmp->tm_year + 1900, tmp->tm_mon + 1, tmp->tm_mday,
			tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

	      printf ("%3lu: ", (unsigned long) cnt);
	      if (VALID_DYNAMIC_NAME (liblist.l_name))
		print_symbol (20, GET_DYNAMIC_NAME (liblist.l_name));
	      else
		printf (_("<corrupt: %9ld>"), liblist.l_name);
	      printf (" %s %#10lx %-7ld", timebuf, liblist.l_checksum,
		      liblist.l_version);

	      if (liblist.l_flags == 0)
		puts (_(" NONE"));
	      else
		{
		  static const struct
		  {
		    const char * name;
		    int bit;
		  }
		  l_flags_vals[] =
		  {
		    { " EXACT_MATCH", LL_EXACT_MATCH },
		    { " IGNORE_INT_VER", LL_IGNORE_INT_VER },
		    { " REQUIRE_MINOR", LL_REQUIRE_MINOR },
		    { " EXPORTS", LL_EXPORTS },
		    { " DELAY_LOAD", LL_DELAY_LOAD },
		    { " DELTA", LL_DELTA }
		  };
		  int flags = liblist.l_flags;
		  size_t fcnt;

		  for (fcnt = 0; fcnt < ARRAY_SIZE (l_flags_vals); ++fcnt)
		    if ((flags & l_flags_vals[fcnt].bit) != 0)
		      {
			fputs (l_flags_vals[fcnt].name, stdout);
			flags ^= l_flags_vals[fcnt].bit;
		      }
		  if (flags != 0)
		    printf (" %#x", (unsigned int) flags);

		  puts ("");
		}
	    }

	  free (elib);
	}
    }

  if (options_offset != 0)
    {
      Elf_External_Options * eopt;
      Elf_Internal_Shdr * sect = section_headers;
      Elf_Internal_Options * iopt;
      Elf_Internal_Options * option;
      size_t offset;
      int cnt;

      /* Find the section header so that we get the size.  */
      while (sect->sh_type != SHT_MIPS_OPTIONS)
	++sect;

      eopt = (Elf_External_Options *) get_data (NULL, file, options_offset, 1,
                                                sect->sh_size, _("options"));
      if (eopt)
	{
	  iopt = (Elf_Internal_Options *)
              cmalloc ((sect->sh_size / sizeof (eopt)), sizeof (* iopt));
	  if (iopt == NULL)
	    {
	      error (_("Out of memory\n"));
	      return 0;
	    }

	  offset = cnt = 0;
	  option = iopt;

	  while (offset < sect->sh_size)
	    {
	      Elf_External_Options * eoption;

	      eoption = (Elf_External_Options *) ((char *) eopt + offset);

	      option->kind = BYTE_GET (eoption->kind);
	      option->size = BYTE_GET (eoption->size);
	      option->section = BYTE_GET (eoption->section);
	      option->info = BYTE_GET (eoption->info);

	      offset += option->size;

	      ++option;
	      ++cnt;
	    }

	  printf (_("\nSection '%s' contains %d entries:\n"),
		  SECTION_NAME (sect), cnt);

	  option = iopt;

	  while (cnt-- > 0)
	    {
	      size_t len;

	      switch (option->kind)
		{
		case ODK_NULL:
		  /* This shouldn't happen.  */
		  printf (" NULL       %d %lx", option->section, option->info);
		  break;
		case ODK_REGINFO:
		  printf (" REGINFO    ");
		  if (elf_header.e_machine == EM_MIPS)
		    {
		      /* 32bit form.  */
		      Elf32_External_RegInfo * ereg;
		      Elf32_RegInfo reginfo;

		      ereg = (Elf32_External_RegInfo *) (option + 1);
		      reginfo.ri_gprmask = BYTE_GET (ereg->ri_gprmask);
		      reginfo.ri_cprmask[0] = BYTE_GET (ereg->ri_cprmask[0]);
		      reginfo.ri_cprmask[1] = BYTE_GET (ereg->ri_cprmask[1]);
		      reginfo.ri_cprmask[2] = BYTE_GET (ereg->ri_cprmask[2]);
		      reginfo.ri_cprmask[3] = BYTE_GET (ereg->ri_cprmask[3]);
		      reginfo.ri_gp_value = BYTE_GET (ereg->ri_gp_value);

		      printf ("GPR %08lx  GP 0x%lx\n",
			      reginfo.ri_gprmask,
			      (unsigned long) reginfo.ri_gp_value);
		      printf ("            CPR0 %08lx  CPR1 %08lx  CPR2 %08lx  CPR3 %08lx\n",
			      reginfo.ri_cprmask[0], reginfo.ri_cprmask[1],
			      reginfo.ri_cprmask[2], reginfo.ri_cprmask[3]);
		    }
		  else
		    {
		      /* 64 bit form.  */
		      Elf64_External_RegInfo * ereg;
		      Elf64_Internal_RegInfo reginfo;

		      ereg = (Elf64_External_RegInfo *) (option + 1);
		      reginfo.ri_gprmask    = BYTE_GET (ereg->ri_gprmask);
		      reginfo.ri_cprmask[0] = BYTE_GET (ereg->ri_cprmask[0]);
		      reginfo.ri_cprmask[1] = BYTE_GET (ereg->ri_cprmask[1]);
		      reginfo.ri_cprmask[2] = BYTE_GET (ereg->ri_cprmask[2]);
		      reginfo.ri_cprmask[3] = BYTE_GET (ereg->ri_cprmask[3]);
		      reginfo.ri_gp_value   = BYTE_GET (ereg->ri_gp_value);

		      printf ("GPR %08lx  GP 0x",
			      reginfo.ri_gprmask);
		      printf_vma (reginfo.ri_gp_value);
		      printf ("\n");

		      printf ("            CPR0 %08lx  CPR1 %08lx  CPR2 %08lx  CPR3 %08lx\n",
			      reginfo.ri_cprmask[0], reginfo.ri_cprmask[1],
			      reginfo.ri_cprmask[2], reginfo.ri_cprmask[3]);
		    }
		  ++option;
		  continue;
		case ODK_EXCEPTIONS:
		  fputs (" EXCEPTIONS fpe_min(", stdout);
		  process_mips_fpe_exception (option->info & OEX_FPU_MIN);
		  fputs (") fpe_max(", stdout);
		  process_mips_fpe_exception ((option->info & OEX_FPU_MAX) >> 8);
		  fputs (")", stdout);

		  if (option->info & OEX_PAGE0)
		    fputs (" PAGE0", stdout);
		  if (option->info & OEX_SMM)
		    fputs (" SMM", stdout);
		  if (option->info & OEX_FPDBUG)
		    fputs (" FPDBUG", stdout);
		  if (option->info & OEX_DISMISS)
		    fputs (" DISMISS", stdout);
		  break;
		case ODK_PAD:
		  fputs (" PAD       ", stdout);
		  if (option->info & OPAD_PREFIX)
		    fputs (" PREFIX", stdout);
		  if (option->info & OPAD_POSTFIX)
		    fputs (" POSTFIX", stdout);
		  if (option->info & OPAD_SYMBOL)
		    fputs (" SYMBOL", stdout);
		  break;
		case ODK_HWPATCH:
		  fputs (" HWPATCH   ", stdout);
		  if (option->info & OHW_R4KEOP)
		    fputs (" R4KEOP", stdout);
		  if (option->info & OHW_R8KPFETCH)
		    fputs (" R8KPFETCH", stdout);
		  if (option->info & OHW_R5KEOP)
		    fputs (" R5KEOP", stdout);
		  if (option->info & OHW_R5KCVTL)
		    fputs (" R5KCVTL", stdout);
		  break;
		case ODK_FILL:
		  fputs (" FILL       ", stdout);
		  /* XXX Print content of info word?  */
		  break;
		case ODK_TAGS:
		  fputs (" TAGS       ", stdout);
		  /* XXX Print content of info word?  */
		  break;
		case ODK_HWAND:
		  fputs (" HWAND     ", stdout);
		  if (option->info & OHWA0_R4KEOP_CHECKED)
		    fputs (" R4KEOP_CHECKED", stdout);
		  if (option->info & OHWA0_R4KEOP_CLEAN)
		    fputs (" R4KEOP_CLEAN", stdout);
		  break;
		case ODK_HWOR:
		  fputs (" HWOR      ", stdout);
		  if (option->info & OHWA0_R4KEOP_CHECKED)
		    fputs (" R4KEOP_CHECKED", stdout);
		  if (option->info & OHWA0_R4KEOP_CLEAN)
		    fputs (" R4KEOP_CLEAN", stdout);
		  break;
		case ODK_GP_GROUP:
		  printf (" GP_GROUP  %#06lx  self-contained %#06lx",
			  option->info & OGP_GROUP,
			  (option->info & OGP_SELF) >> 16);
		  break;
		case ODK_IDENT:
		  printf (" IDENT     %#06lx  self-contained %#06lx",
			  option->info & OGP_GROUP,
			  (option->info & OGP_SELF) >> 16);
		  break;
		default:
		  /* This shouldn't happen.  */
		  printf (" %3d ???     %d %lx",
			  option->kind, option->section, option->info);
		  break;
		}

	      len = sizeof (* eopt);
	      while (len < option->size)
		if (((char *) option)[len] >= ' '
		    && ((char *) option)[len] < 0x7f)
		  printf ("%c", ((char *) option)[len++]);
		else
		  printf ("\\%03o", ((char *) option)[len++]);

	      fputs ("\n", stdout);
	      ++option;
	    }

	  free (eopt);
	}
    }

  if (conflicts_offset != 0 && conflictsno != 0)
    {
      Elf32_Conflict * iconf;
      size_t cnt;

      if (dynamic_symbols == NULL)
	{
	  error (_("conflict list found without a dynamic symbol table\n"));
	  return 0;
	}

      iconf = (Elf32_Conflict *) cmalloc (conflictsno, sizeof (* iconf));
      if (iconf == NULL)
	{
	  error (_("Out of memory\n"));
	  return 0;
	}

      if (is_32bit_elf)
	{
	  Elf32_External_Conflict * econf32;

	  econf32 = (Elf32_External_Conflict *)
              get_data (NULL, file, conflicts_offset, conflictsno,
                        sizeof (* econf32), _("conflict"));
	  if (!econf32)
	    return 0;

	  for (cnt = 0; cnt < conflictsno; ++cnt)
	    iconf[cnt] = BYTE_GET (econf32[cnt]);

	  free (econf32);
	}
      else
	{
	  Elf64_External_Conflict * econf64;

	  econf64 = (Elf64_External_Conflict *)
              get_data (NULL, file, conflicts_offset, conflictsno,
                        sizeof (* econf64), _("conflict"));
	  if (!econf64)
	    return 0;

	  for (cnt = 0; cnt < conflictsno; ++cnt)
	    iconf[cnt] = BYTE_GET (econf64[cnt]);

	  free (econf64);
	}

      printf (_("\nSection '.conflict' contains %lu entries:\n"),
	      (unsigned long) conflictsno);
      puts (_("  Num:    Index       Value  Name"));

      for (cnt = 0; cnt < conflictsno; ++cnt)
	{
	  Elf_Internal_Sym * psym = & dynamic_symbols[iconf[cnt]];

	  printf ("%5lu: %8lu  ", (unsigned long) cnt, iconf[cnt]);
	  print_vma (psym->st_value, FULL_HEX);
	  putchar (' ');
	  if (VALID_DYNAMIC_NAME (psym->st_name))
	    print_symbol (25, GET_DYNAMIC_NAME (psym->st_name));
	  else
	    printf (_("<corrupt: %14ld>"), psym->st_name);
	  putchar ('\n');
	}

      free (iconf);
    }

  if (pltgot != 0 && local_gotno != 0)
    {
      bfd_vma ent, local_end, global_end;
      size_t i, offset;
      unsigned char * data;
      int addr_size;

      ent = pltgot;
      addr_size = (is_32bit_elf ? 4 : 8);
      local_end = pltgot + local_gotno * addr_size;
      global_end = local_end + (symtabno - gotsym) * addr_size;

      offset = offset_from_vma (file, pltgot, global_end - pltgot);
      data = (unsigned char *) get_data (NULL, file, offset,
                                         global_end - pltgot, 1,
					 _("Global Offset Table data"));
      if (data == NULL)
	return 0;

      printf (_("\nPrimary GOT:\n"));
      printf (_(" Canonical gp value: "));
      print_vma (pltgot + 0x7ff0, LONG_HEX);
      printf ("\n\n");

      printf (_(" Reserved entries:\n"));
      printf (_("  %*s %10s %*s Purpose\n"),
	      addr_size * 2, _("Address"), _("Access"),
	      addr_size * 2, _("Initial"));
      ent = print_mips_got_entry (data, pltgot, ent);
      printf (_(" Lazy resolver\n"));
      if (data
	  && (byte_get (data + ent - pltgot, addr_size)
	      >> (addr_size * 8 - 1)) != 0)
	{
	  ent = print_mips_got_entry (data, pltgot, ent);
	  printf (_(" Module pointer (GNU extension)\n"));
	}
      printf ("\n");

      if (ent < local_end)
	{
	  printf (_(" Local entries:\n"));
	  printf ("  %*s %10s %*s\n",
		  addr_size * 2, _("Address"), _("Access"),
		  addr_size * 2, _("Initial"));
	  while (ent < local_end)
	    {
	      ent = print_mips_got_entry (data, pltgot, ent);
	      printf ("\n");
	    }
	  printf ("\n");
	}

      if (gotsym < symtabno)
	{
	  int sym_width;

	  printf (_(" Global entries:\n"));
	  printf ("  %*s %10s %*s %*s %-7s %3s %s\n",
		  addr_size * 2, _("Address"),
		  _("Access"),
		  addr_size * 2, _("Initial"),
		  addr_size * 2, _("Sym.Val."),
		  _("Type"),
		  /* Note for translators: "Ndx" = abbreviated form of "Index".  */
		  _("Ndx"), _("Name"));
	  
	  sym_width = (is_32bit_elf ? 80 : 160) - 28 - addr_size * 6 - 1;
	  for (i = gotsym; i < symtabno; i++)
	    {
	      Elf_Internal_Sym * psym;

	      psym = dynamic_symbols + i;
	      ent = print_mips_got_entry (data, pltgot, ent);
	      printf (" ");
	      print_vma (psym->st_value, LONG_HEX);
	      printf (" %-7s %3s ",
		      get_symbol_type (ELF_ST_TYPE (psym->st_info)),
		      get_symbol_index_type (psym->st_shndx));
	      if (VALID_DYNAMIC_NAME (psym->st_name))
		print_symbol (sym_width, GET_DYNAMIC_NAME (psym->st_name));
	      else
		printf (_("<corrupt: %14ld>"), psym->st_name);
	      printf ("\n");
	    }
	  printf ("\n");
	}

      if (data)
	free (data);
    }

  if (mips_pltgot != 0 && jmprel != 0 && pltrel != 0 && pltrelsz != 0)
    {
      bfd_vma ent, end;
      size_t offset, rel_offset;
      unsigned long count, i;
      unsigned char * data;
      int addr_size, sym_width;
      Elf_Internal_Rela * rels;

      rel_offset = offset_from_vma (file, jmprel, pltrelsz);
      if (pltrel == DT_RELA)
	{
	  if (!slurp_rela_relocs (file, rel_offset, pltrelsz, &rels, &count))
	    return 0;
	}
      else
	{
	  if (!slurp_rel_relocs (file, rel_offset, pltrelsz, &rels, &count))
	    return 0;
	}

      ent = mips_pltgot;
      addr_size = (is_32bit_elf ? 4 : 8);
      end = mips_pltgot + (2 + count) * addr_size;

      offset = offset_from_vma (file, mips_pltgot, end - mips_pltgot);
      data = (unsigned char *) get_data (NULL, file, offset, end - mips_pltgot,
                                         1, _("Procedure Linkage Table data"));
      if (data == NULL)
	return 0;

      printf ("\nPLT GOT:\n\n");
      printf (_(" Reserved entries:\n"));
      printf (_("  %*s %*s Purpose\n"),
	      addr_size * 2, _("Address"), addr_size * 2, _("Initial"));
      ent = print_mips_pltgot_entry (data, mips_pltgot, ent);
      printf (_(" PLT lazy resolver\n"));
      ent = print_mips_pltgot_entry (data, mips_pltgot, ent);
      printf (_(" Module pointer\n"));
      printf ("\n");

      printf (_(" Entries:\n"));
      printf ("  %*s %*s %*s %-7s %3s %s\n",
	      addr_size * 2, _("Address"),
	      addr_size * 2, _("Initial"),
	      addr_size * 2, _("Sym.Val."), _("Type"), _("Ndx"), _("Name"));
      sym_width = (is_32bit_elf ? 80 : 160) - 17 - addr_size * 6 - 1;
      for (i = 0; i < count; i++)
	{
	  Elf_Internal_Sym * psym;

	  psym = dynamic_symbols + get_reloc_symindex (rels[i].r_info);
	  ent = print_mips_pltgot_entry (data, mips_pltgot, ent);
	  printf (" ");
	  print_vma (psym->st_value, LONG_HEX);
	  printf (" %-7s %3s ",
		  get_symbol_type (ELF_ST_TYPE (psym->st_info)),
		  get_symbol_index_type (psym->st_shndx));
	  if (VALID_DYNAMIC_NAME (psym->st_name))
	    print_symbol (sym_width, GET_DYNAMIC_NAME (psym->st_name));
	  else
	    printf (_("<corrupt: %14ld>"), psym->st_name);
	  printf ("\n");
	}
      printf ("\n");

      if (data)
	free (data);
      free (rels);
    }

  return 1;
}

static int
process_gnu_liblist (FILE * file)
{
  Elf_Internal_Shdr * section;
  Elf_Internal_Shdr * string_sec;
  Elf32_External_Lib * elib;
  char * strtab;
  size_t strtab_size;
  size_t cnt;
  unsigned i;

  if (! do_arch)
    return 0;

  for (i = 0, section = section_headers;
       i < elf_header.e_shnum;
       i++, section++)
    {
      switch (section->sh_type)
	{
	case SHT_GNU_LIBLIST:
	  if (section->sh_link >= elf_header.e_shnum)
	    break;

	  elib = (Elf32_External_Lib *)
              get_data (NULL, file, section->sh_offset, 1, section->sh_size,
                        _("liblist section data"));

	  if (elib == NULL)
	    break;
	  string_sec = section_headers + section->sh_link;

	  strtab = (char *) get_data (NULL, file, string_sec->sh_offset, 1,
                                      string_sec->sh_size,
                                      _("liblist string table"));
	  if (strtab == NULL
	      || section->sh_entsize != sizeof (Elf32_External_Lib))
	    {
	      free (elib);
	      free (strtab);
	      break;
	    }
	  strtab_size = string_sec->sh_size;

	  printf (_("\nLibrary list section '%s' contains %lu entries:\n"),
		  SECTION_NAME (section),
		  (unsigned long) (section->sh_size / sizeof (Elf32_External_Lib)));

	  puts (_("     Library              Time Stamp          Checksum   Version Flags"));

	  for (cnt = 0; cnt < section->sh_size / sizeof (Elf32_External_Lib);
	       ++cnt)
	    {
	      Elf32_Lib liblist;
	      time_t atime;
	      char timebuf[20];
	      struct tm * tmp;

	      liblist.l_name = BYTE_GET (elib[cnt].l_name);
	      atime = BYTE_GET (elib[cnt].l_time_stamp);
	      liblist.l_checksum = BYTE_GET (elib[cnt].l_checksum);
	      liblist.l_version = BYTE_GET (elib[cnt].l_version);
	      liblist.l_flags = BYTE_GET (elib[cnt].l_flags);

	      tmp = gmtime (&atime);
	      snprintf (timebuf, sizeof (timebuf),
			"%04u-%02u-%02uT%02u:%02u:%02u",
			tmp->tm_year + 1900, tmp->tm_mon + 1, tmp->tm_mday,
			tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

	      printf ("%3lu: ", (unsigned long) cnt);
	      if (do_wide)
		printf ("%-20s", liblist.l_name < strtab_size
			? strtab + liblist.l_name : _("<corrupt>"));
	      else
		printf ("%-20.20s", liblist.l_name < strtab_size
			? strtab + liblist.l_name : _("<corrupt>"));
	      printf (" %s %#010lx %-7ld %-7ld\n", timebuf, liblist.l_checksum,
		      liblist.l_version, liblist.l_flags);
	    }

	  free (elib);
	  free (strtab);
	}
    }

  return 1;
}

static const char *
get_note_type (unsigned e_type)
{
  static char buff[64];

  if (elf_header.e_type == ET_CORE)
    switch (e_type)
      {
      case NT_AUXV:
	return _("NT_AUXV (auxiliary vector)");
      case NT_PRSTATUS:
	return _("NT_PRSTATUS (prstatus structure)");
      case NT_FPREGSET:
	return _("NT_FPREGSET (floating point registers)");
      case NT_PRPSINFO:
	return _("NT_PRPSINFO (prpsinfo structure)");
      case NT_TASKSTRUCT:
	return _("NT_TASKSTRUCT (task structure)");
      case NT_PRXFPREG:
	return _("NT_PRXFPREG (user_xfpregs structure)");
      case NT_PPC_VMX:
	return _("NT_PPC_VMX (ppc Altivec registers)");
      case NT_PPC_VSX:
	return _("NT_PPC_VSX (ppc VSX registers)");
      case NT_X86_XSTATE:
	return _("NT_X86_XSTATE (x86 XSAVE extended state)");
      case NT_S390_HIGH_GPRS:
	return _("NT_S390_HIGH_GPRS (s390 upper register halves)");
      case NT_S390_TIMER:
	return _("NT_S390_TIMER (s390 timer register)");
      case NT_S390_TODCMP:
	return _("NT_S390_TODCMP (s390 TOD comparator register)");
      case NT_S390_TODPREG:
	return _("NT_S390_TODPREG (s390 TOD programmable register)");
      case NT_S390_CTRS:
	return _("NT_S390_CTRS (s390 control registers)");
      case NT_S390_PREFIX:
	return _("NT_S390_PREFIX (s390 prefix register)");
      case NT_ARM_VFP:
	return _("NT_ARM_VFP (arm VFP registers)");
      case NT_PSTATUS:
	return _("NT_PSTATUS (pstatus structure)");
      case NT_FPREGS:
	return _("NT_FPREGS (floating point registers)");
      case NT_PSINFO:
	return _("NT_PSINFO (psinfo structure)");
      case NT_LWPSTATUS:
	return _("NT_LWPSTATUS (lwpstatus_t structure)");
      case NT_LWPSINFO:
	return _("NT_LWPSINFO (lwpsinfo_t structure)");
      case NT_WIN32PSTATUS:
	return _("NT_WIN32PSTATUS (win32_pstatus structure)");
      default:
	break;
      }
  else
    switch (e_type)
      {
      case NT_VERSION:
	return _("NT_VERSION (version)");
      case NT_ARCH:
	return _("NT_ARCH (architecture)");
      default:
	break;
      }

  snprintf (buff, sizeof (buff), _("Unknown note type: (0x%08x)"), e_type);
  return buff;
}

static const char *
get_gnu_elf_note_type (unsigned e_type)
{
  static char buff[64];

  switch (e_type)
    {
    case NT_GNU_ABI_TAG:
      return _("NT_GNU_ABI_TAG (ABI version tag)");
    case NT_GNU_HWCAP:
      return _("NT_GNU_HWCAP (DSO-supplied software HWCAP info)");
    case NT_GNU_BUILD_ID:
      return _("NT_GNU_BUILD_ID (unique build ID bitstring)");
    case NT_GNU_GOLD_VERSION:
      return _("NT_GNU_GOLD_VERSION (gold version)");
    default:
      break;
    }

  snprintf (buff, sizeof (buff), _("Unknown note type: (0x%08x)"), e_type);
  return buff;
}

static int
print_gnu_note (Elf_Internal_Note *pnote)
{
  switch (pnote->type)
    {
    case NT_GNU_BUILD_ID:
      {
	unsigned long i;

	printf (_("    Build ID: "));
	for (i = 0; i < pnote->descsz; ++i)
	  printf ("%02x", pnote->descdata[i] & 0xff);
	printf ("\n");
      }
      break;

    case NT_GNU_ABI_TAG:
      {
	unsigned long os, major, minor, subminor;
	const char *osname;

	os = byte_get ((unsigned char *) pnote->descdata, 4);
	major = byte_get ((unsigned char *) pnote->descdata + 4, 4);
	minor = byte_get ((unsigned char *) pnote->descdata + 8, 4);
	subminor = byte_get ((unsigned char *) pnote->descdata + 12, 4);

	switch (os)
	  {
	  case GNU_ABI_TAG_LINUX:
	    osname = "Linux";
	    break;
	  case GNU_ABI_TAG_HURD:
	    osname = "Hurd";
	    break;
	  case GNU_ABI_TAG_SOLARIS:
	    osname = "Solaris";
	    break;
	  case GNU_ABI_TAG_FREEBSD:
	    osname = "FreeBSD";
	    break;
	  case GNU_ABI_TAG_NETBSD:
	    osname = "NetBSD";
	    break;
	  default:
	    osname = "Unknown";
	    break;
	  }

	printf (_("    OS: %s, ABI: %ld.%ld.%ld\n"), osname,
		major, minor, subminor);
      }
      break;
    }

  return 1;
}

static const char *
get_netbsd_elfcore_note_type (unsigned e_type)
{
  static char buff[64];

  if (e_type == NT_NETBSDCORE_PROCINFO)
    {
      /* NetBSD core "procinfo" structure.  */
      return _("NetBSD procinfo structure");
    }

  /* As of Jan 2002 there are no other machine-independent notes
     defined for NetBSD core files.  If the note type is less
     than the start of the machine-dependent note types, we don't
     understand it.  */

  if (e_type < NT_NETBSDCORE_FIRSTMACH)
    {
      snprintf (buff, sizeof (buff), _("Unknown note type: (0x%08x)"), e_type);
      return buff;
    }

  switch (elf_header.e_machine)
    {
    /* On the Alpha, SPARC (32-bit and 64-bit), PT_GETREGS == mach+0
       and PT_GETFPREGS == mach+2.  */

    case EM_OLD_ALPHA:
    case EM_ALPHA:
    case EM_SPARC:
    case EM_SPARC32PLUS:
    case EM_SPARCV9:
      switch (e_type)
	{
	case NT_NETBSDCORE_FIRSTMACH + 0:
	  return _("PT_GETREGS (reg structure)");
	case NT_NETBSDCORE_FIRSTMACH + 2:
	  return _("PT_GETFPREGS (fpreg structure)");
	default:
	  break;
	}
      break;

    /* On all other arch's, PT_GETREGS == mach+1 and
       PT_GETFPREGS == mach+3.  */
    default:
      switch (e_type)
	{
	case NT_NETBSDCORE_FIRSTMACH + 1:
	  return _("PT_GETREGS (reg structure)");
	case NT_NETBSDCORE_FIRSTMACH + 3:
	  return _("PT_GETFPREGS (fpreg structure)");
	default:
	  break;
	}
    }

  snprintf (buff, sizeof (buff), "PT_FIRSTMACH+%d",
	    e_type - NT_NETBSDCORE_FIRSTMACH);
  return buff;
}

static const char *
get_stapsdt_note_type (unsigned e_type)
{
  static char buff[64];

  switch (e_type)
    {
    case NT_STAPSDT:
      return _("NT_STAPSDT (SystemTap probe descriptors)");

    default:
      break;
    }

  snprintf (buff, sizeof (buff), _("Unknown note type: (0x%08x)"), e_type);
  return buff;
}

static int
print_stapsdt_note (Elf_Internal_Note *pnote)
{
  int addr_size = is_32bit_elf ? 4 : 8;
  char *data = pnote->descdata;
  char *data_end = pnote->descdata + pnote->descsz;
  bfd_vma pc, base_addr, semaphore;
  char *provider, *probe, *arg_fmt;

  pc = byte_get ((unsigned char *) data, addr_size);
  data += addr_size;
  base_addr = byte_get ((unsigned char *) data, addr_size);
  data += addr_size;
  semaphore = byte_get ((unsigned char *) data, addr_size);
  data += addr_size;

  provider = data;
  data += strlen (data) + 1;
  probe = data;
  data += strlen (data) + 1;
  arg_fmt = data;
  data += strlen (data) + 1;

  printf (_("    Provider: %s\n"), provider);
  printf (_("    Name: %s\n"), probe);
  printf (_("    Location: "));
  print_vma (pc, FULL_HEX);
  printf (_(", Base: "));
  print_vma (base_addr, FULL_HEX);
  printf (_(", Semaphore: "));
  print_vma (semaphore, FULL_HEX);
  printf ("\n");
  printf (_("    Arguments: %s\n"), arg_fmt);

  return data == data_end;
}

static const char *
get_ia64_vms_note_type (unsigned e_type)
{
  static char buff[64];

  switch (e_type)
    {
    case NT_VMS_MHD:
      return _("NT_VMS_MHD (module header)");
    case NT_VMS_LNM:
      return _("NT_VMS_LNM (language name)");
    case NT_VMS_SRC:
      return _("NT_VMS_SRC (source files)");
    case NT_VMS_TITLE:
      return "NT_VMS_TITLE";
    case NT_VMS_EIDC:
      return _("NT_VMS_EIDC (consistency check)");
    case NT_VMS_FPMODE:
      return _("NT_VMS_FPMODE (FP mode)");
    case NT_VMS_LINKTIME:
      return "NT_VMS_LINKTIME";
    case NT_VMS_IMGNAM:
      return _("NT_VMS_IMGNAM (image name)");
    case NT_VMS_IMGID:
      return _("NT_VMS_IMGID (image id)");
    case NT_VMS_LINKID:
      return _("NT_VMS_LINKID (link id)");
    case NT_VMS_IMGBID:
      return _("NT_VMS_IMGBID (build id)");
    case NT_VMS_GSTNAM:
      return _("NT_VMS_GSTNAM (sym table name)");
    case NT_VMS_ORIG_DYN:
      return "NT_VMS_ORIG_DYN";
    case NT_VMS_PATCHTIME:
      return "NT_VMS_PATCHTIME";
    default:
      snprintf (buff, sizeof (buff), _("Unknown note type: (0x%08x)"), e_type);
      return buff;
    }
}

static int
print_ia64_vms_note (Elf_Internal_Note * pnote)
{
  switch (pnote->type)
    {
    case NT_VMS_MHD:
      if (pnote->descsz > 36)
        {
          size_t l = strlen (pnote->descdata + 34);
          printf (_("    Creation date  : %.17s\n"), pnote->descdata);
          printf (_("    Last patch date: %.17s\n"), pnote->descdata + 17);
          printf (_("    Module name    : %s\n"), pnote->descdata + 34);
          printf (_("    Module version : %s\n"), pnote->descdata + 34 + l + 1);
        }
      else
        printf (_("    Invalid size\n"));
      break;
    case NT_VMS_LNM:
      printf (_("   Language: %s\n"), pnote->descdata);
      break;
#ifdef BFD64
    case NT_VMS_FPMODE:
      printf (_("   Floating Point mode: "));
      printf ("0x%016" BFD_VMA_FMT "x\n",
              (bfd_vma)byte_get ((unsigned char *)pnote->descdata, 8));
      break;
    case NT_VMS_LINKTIME:
      printf (_("   Link time: "));
      print_vms_time
        ((bfd_int64_t) byte_get ((unsigned char *)pnote->descdata, 8));
      printf ("\n");
      break;
    case NT_VMS_PATCHTIME:
      printf (_("   Patch time: "));
      print_vms_time
        ((bfd_int64_t) byte_get ((unsigned char *)pnote->descdata, 8));
      printf ("\n");
      break;
    case NT_VMS_ORIG_DYN:
      printf (_("   Major id: %u,  minor id: %u\n"),
              (unsigned) byte_get ((unsigned char *)pnote->descdata, 4),
              (unsigned) byte_get ((unsigned char *)pnote->descdata + 4, 4));
      printf (_("   Last modified  : "));
      print_vms_time
        ((bfd_int64_t) byte_get ((unsigned char *)pnote->descdata + 8, 8));
      printf (_("\n   Link flags  : "));
      printf ("0x%016" BFD_VMA_FMT "x\n",
              (bfd_vma)byte_get ((unsigned char *)pnote->descdata + 16, 8));
      printf (_("   Header flags: 0x%08x\n"),
              (unsigned)byte_get ((unsigned char *)pnote->descdata + 24, 4));
      printf (_("   Image id    : %s\n"), pnote->descdata + 32);
      break;
#endif
    case NT_VMS_IMGNAM:
      printf (_("    Image name: %s\n"), pnote->descdata);
      break;
    case NT_VMS_GSTNAM:
      printf (_("    Global symbol table name: %s\n"), pnote->descdata);
      break;
    case NT_VMS_IMGID:
      printf (_("    Image id: %s\n"), pnote->descdata);
      break;
    case NT_VMS_LINKID:
      printf (_("    Linker id: %s\n"), pnote->descdata);
      break;
    default:
      break;
    }
  return 1;
}

/* Note that by the ELF standard, the name field is already null byte
   terminated, and namesz includes the terminating null byte.
   I.E. the value of namesz for the name "FSF" is 4.

   If the value of namesz is zero, there is no name present.  */
static int
process_note (Elf_Internal_Note * pnote)
{
  const char * name = pnote->namesz ? pnote->namedata : "(NONE)";
  const char * nt;

  if (pnote->namesz == 0)
    /* If there is no note name, then use the default set of
       note type strings.  */
    nt = get_note_type (pnote->type);

  else if (const_strneq (pnote->namedata, "GNU"))
    /* GNU-specific object file notes.  */
    nt = get_gnu_elf_note_type (pnote->type);

  else if (const_strneq (pnote->namedata, "NetBSD-CORE"))
    /* NetBSD-specific core file notes.  */
    nt = get_netbsd_elfcore_note_type (pnote->type);

  else if (strneq (pnote->namedata, "SPU/", 4))
    {
      /* SPU-specific core file notes.  */
      nt = pnote->namedata + 4;
      name = "SPU";
    }

  else if (const_strneq (pnote->namedata, "IPF/VMS"))
    /* VMS/ia64-specific file notes.  */
    nt = get_ia64_vms_note_type (pnote->type);

  else if (const_strneq (pnote->namedata, "stapsdt"))
    nt = get_stapsdt_note_type (pnote->type);

  else
    /* Don't recognize this note name; just use the default set of
       note type strings.  */
    nt = get_note_type (pnote->type);

  printf ("  %-20s 0x%08lx\t%s\n", name, pnote->descsz, nt);

  if (const_strneq (pnote->namedata, "IPF/VMS"))
    return print_ia64_vms_note (pnote);
  else if (const_strneq (pnote->namedata, "GNU"))
    return print_gnu_note (pnote);
  else if (const_strneq (pnote->namedata, "stapsdt"))
    return print_stapsdt_note (pnote);
  else
    return 1;
}


static int
process_corefile_note_segment (FILE * file, bfd_vma offset, bfd_vma length)
{
  Elf_External_Note * pnotes;
  Elf_External_Note * external;
  int res = 1;

  if (length <= 0)
    return 0;

  pnotes = (Elf_External_Note *) get_data (NULL, file, offset, 1, length,
                                           _("notes"));
  if (pnotes == NULL)
    return 0;

  external = pnotes;

  printf (_("\nNotes at offset 0x%08lx with length 0x%08lx:\n"),
	  (unsigned long) offset, (unsigned long) length);
  printf (_("  %-20s %10s\tDescription\n"), _("Owner"), _("Data size"));

  while (external < (Elf_External_Note *) ((char *) pnotes + length))
    {
      Elf_External_Note * next;
      Elf_Internal_Note inote;
      char * temp = NULL;

      if (!is_ia64_vms ())
        {
          inote.type     = BYTE_GET (external->type);
          inote.namesz   = BYTE_GET (external->namesz);
          inote.namedata = external->name;
          inote.descsz   = BYTE_GET (external->descsz);
          inote.descdata = inote.namedata + align_power (inote.namesz, 2);
          inote.descpos  = offset + (inote.descdata - (char *) pnotes);

          next = (Elf_External_Note *) (inote.descdata + align_power (inote.descsz, 2));
        }
      else
        {
          Elf64_External_VMS_Note *vms_external;

          vms_external = (Elf64_External_VMS_Note *)external;
          inote.type     = BYTE_GET (vms_external->type);
          inote.namesz   = BYTE_GET (vms_external->namesz);
          inote.namedata = vms_external->name;
          inote.descsz   = BYTE_GET (vms_external->descsz);
          inote.descdata = inote.namedata + align_power (inote.namesz, 3);
          inote.descpos  = offset + (inote.descdata - (char *) pnotes);

          next = (Elf_External_Note *)
            (inote.descdata + align_power (inote.descsz, 3));
        }

      if (   ((char *) next > ((char *) pnotes) + length)
	  || ((char *) next <  (char *) pnotes))
	{
	  warn (_("corrupt note found at offset %lx into core notes\n"),
		(unsigned long) ((char *) external - (char *) pnotes));
	  warn (_(" type: %lx, namesize: %08lx, descsize: %08lx\n"),
		inote.type, inote.namesz, inote.descsz);
	  break;
	}

      external = next;

      /* Prevent out-of-bounds indexing.  */
      if (inote.namedata + inote.namesz > (char *) pnotes + length
	  || inote.namedata + inote.namesz < inote.namedata)
        {
          warn (_("corrupt note found at offset %lx into core notes\n"),
                (unsigned long) ((char *) external - (char *) pnotes));
          warn (_(" type: %lx, namesize: %08lx, descsize: %08lx\n"),
                inote.type, inote.namesz, inote.descsz);
          break;
        }

      /* Verify that name is null terminated.  It appears that at least
	 one version of Linux (RedHat 6.0) generates corefiles that don't
	 comply with the ELF spec by failing to include the null byte in
	 namesz.  */
      if (inote.namedata[inote.namesz - 1] != '\0')
	{
	  temp = (char *) malloc (inote.namesz + 1);

	  if (temp == NULL)
	    {
	      error (_("Out of memory\n"));
	      res = 0;
	      break;
	    }

	  strncpy (temp, inote.namedata, inote.namesz);
	  temp[inote.namesz] = 0;

	  /* warn (_("'%s' NOTE name not properly null terminated\n"), temp);  */
	  inote.namedata = temp;
	}

      res &= process_note (& inote);

      if (temp != NULL)
	{
	  free (temp);
	  temp = NULL;
	}
    }

  free (pnotes);

  return res;
}

static int
process_corefile_note_segments (FILE * file)
{
  Elf_Internal_Phdr * segment;
  unsigned int i;
  int res = 1;

  if (! get_program_headers (file))
      return 0;

  for (i = 0, segment = program_headers;
       i < elf_header.e_phnum;
       i++, segment++)
    {
      if (segment->p_type == PT_NOTE)
	res &= process_corefile_note_segment (file,
					      (bfd_vma) segment->p_offset,
					      (bfd_vma) segment->p_filesz);
    }

  return res;
}

static int
process_note_sections (FILE * file)
{
  Elf_Internal_Shdr * section;
  unsigned long i;
  int res = 1;

  for (i = 0, section = section_headers;
       i < elf_header.e_shnum && section != NULL;
       i++, section++)
    if (section->sh_type == SHT_NOTE)
      res &= process_corefile_note_segment (file,
					    (bfd_vma) section->sh_offset,
					    (bfd_vma) section->sh_size);

  return res;
}

static int
process_notes (FILE * file)
{
  /* If we have not been asked to display the notes then do nothing.  */
  if (! do_notes)
    return 1;

  if (elf_header.e_type != ET_CORE)
    return process_note_sections (file);

  /* No program headers means no NOTE segment.  */
  if (elf_header.e_phnum > 0)
    return process_corefile_note_segments (file);

  printf (_("No note segments present in the core file.\n"));
  return 1;
}

static int
process_arch_specific (FILE * file)
{
  if (! do_arch)
    return 1;

  switch (elf_header.e_machine)
    {
    case EM_ARM:
      return process_arm_specific (file);
    case EM_MIPS:
    case EM_MIPS_RS3_LE:
      return process_mips_specific (file);
      break;
    case EM_PPC:
      return process_power_specific (file);
      break;
    case EM_SPARC:
    case EM_SPARC32PLUS:
    case EM_SPARCV9:
      return process_sparc_specific (file);
      break;
    case EM_TI_C6000:
      return process_tic6x_specific (file);
      break;
    default:
      break;
    }
  return 1;
}

static int
get_file_header (FILE * file)
{
  /* Read in the identity array.  */
  if (fread (elf_header.e_ident, EI_NIDENT, 1, file) != 1)
    return 0;

  /* Determine how to read the rest of the header.  */
  switch (elf_header.e_ident[EI_DATA])
    {
    default: /* fall through */
    case ELFDATANONE: /* fall through */
    case ELFDATA2LSB:
      byte_get = byte_get_little_endian;
      byte_put = byte_put_little_endian;
      break;
    case ELFDATA2MSB:
      byte_get = byte_get_big_endian;
      byte_put = byte_put_big_endian;
      break;
    }

  /* For now we only support 32 bit and 64 bit ELF files.  */
  is_32bit_elf = (elf_header.e_ident[EI_CLASS] != ELFCLASS64);

  /* Read in the rest of the header.  */
  if (is_32bit_elf)
    {
      Elf32_External_Ehdr ehdr32;

      if (fread (ehdr32.e_type, sizeof (ehdr32) - EI_NIDENT, 1, file) != 1)
	return 0;

      elf_header.e_type      = BYTE_GET (ehdr32.e_type);
      elf_header.e_machine   = BYTE_GET (ehdr32.e_machine);
      elf_header.e_version   = BYTE_GET (ehdr32.e_version);
      elf_header.e_entry     = BYTE_GET (ehdr32.e_entry);
      elf_header.e_phoff     = BYTE_GET (ehdr32.e_phoff);
      elf_header.e_shoff     = BYTE_GET (ehdr32.e_shoff);
      elf_header.e_flags     = BYTE_GET (ehdr32.e_flags);
      elf_header.e_ehsize    = BYTE_GET (ehdr32.e_ehsize);
      elf_header.e_phentsize = BYTE_GET (ehdr32.e_phentsize);
      elf_header.e_phnum     = BYTE_GET (ehdr32.e_phnum);
      elf_header.e_shentsize = BYTE_GET (ehdr32.e_shentsize);
      elf_header.e_shnum     = BYTE_GET (ehdr32.e_shnum);
      elf_header.e_shstrndx  = BYTE_GET (ehdr32.e_shstrndx);
    }
  else
    {
      Elf64_External_Ehdr ehdr64;

      /* If we have been compiled with sizeof (bfd_vma) == 4, then
	 we will not be able to cope with the 64bit data found in
	 64 ELF files.  Detect this now and abort before we start
	 overwriting things.  */
      if (sizeof (bfd_vma) < 8)
	{
	  error (_("This instance of readelf has been built without support for a\n\
64 bit data type and so it cannot read 64 bit ELF files.\n"));
	  return 0;
	}

      if (fread (ehdr64.e_type, sizeof (ehdr64) - EI_NIDENT, 1, file) != 1)
	return 0;

      elf_header.e_type      = BYTE_GET (ehdr64.e_type);
      elf_header.e_machine   = BYTE_GET (ehdr64.e_machine);
      elf_header.e_version   = BYTE_GET (ehdr64.e_version);
      elf_header.e_entry     = BYTE_GET (ehdr64.e_entry);
      elf_header.e_phoff     = BYTE_GET (ehdr64.e_phoff);
      elf_header.e_shoff     = BYTE_GET (ehdr64.e_shoff);
      elf_header.e_flags     = BYTE_GET (ehdr64.e_flags);
      elf_header.e_ehsize    = BYTE_GET (ehdr64.e_ehsize);
      elf_header.e_phentsize = BYTE_GET (ehdr64.e_phentsize);
      elf_header.e_phnum     = BYTE_GET (ehdr64.e_phnum);
      elf_header.e_shentsize = BYTE_GET (ehdr64.e_shentsize);
      elf_header.e_shnum     = BYTE_GET (ehdr64.e_shnum);
      elf_header.e_shstrndx  = BYTE_GET (ehdr64.e_shstrndx);
    }

  if (elf_header.e_shoff)
    {
      /* There may be some extensions in the first section header.  Don't
	 bomb if we can't read it.  */
      if (is_32bit_elf)
	get_32bit_section_headers (file, 1);
      else
	get_64bit_section_headers (file, 1);
    }

  return 1;
}

/* Process one ELF object file according to the command line options.
   This file may actually be stored in an archive.  The file is
   positioned at the start of the ELF object.  */

static int
process_object (char * file_name, FILE * file)
{
  unsigned int i;

  if (! get_file_header (file))
    {
      error (_("%s: Failed to read file header\n"), file_name);
      return 1;
    }

  /* Initialise per file variables.  */
  for (i = ARRAY_SIZE (version_info); i--;)
    version_info[i] = 0;

  for (i = ARRAY_SIZE (dynamic_info); i--;)
    dynamic_info[i] = 0;
  dynamic_info_DT_GNU_HASH = 0;

  /* Process the file.  */
  if (show_name)
    printf (_("\nFile: %s\n"), file_name);

  /* Initialise the dump_sects array from the cmdline_dump_sects array.
     Note we do this even if cmdline_dump_sects is empty because we
     must make sure that the dump_sets array is zeroed out before each
     object file is processed.  */
  if (num_dump_sects > num_cmdline_dump_sects)
    memset (dump_sects, 0, num_dump_sects * sizeof (* dump_sects));

  if (num_cmdline_dump_sects > 0)
    {
      if (num_dump_sects == 0)
	/* A sneaky way of allocating the dump_sects array.  */
	request_dump_bynumber (num_cmdline_dump_sects, 0);

      assert (num_dump_sects >= num_cmdline_dump_sects);
      memcpy (dump_sects, cmdline_dump_sects,
	      num_cmdline_dump_sects * sizeof (* dump_sects));
    }

  if (! process_file_header ())
    return 1;

  if (! process_section_headers (file))
    {
      /* Without loaded section headers we cannot process lots of
	 things.  */
      do_unwind = do_version = do_dump = do_arch = 0;

      if (! do_using_dynamic)
	do_syms = do_dyn_syms = do_reloc = 0;
    }

  if (! process_section_groups (file))
    {
      /* Without loaded section groups we cannot process unwind.  */
      do_unwind = 0;
    }

  if (process_program_headers (file))
    process_dynamic_section (file);

  process_relocs (file);

  process_unwind (file);

  process_symbol_table (file);

  process_syminfo (file);

  process_version_sections (file);

  process_section_contents (file);

  process_notes (file);

  process_gnu_liblist (file);

  process_arch_specific (file);

  if (program_headers)
    {
      free (program_headers);
      program_headers = NULL;
    }

  if (section_headers)
    {
      free (section_headers);
      section_headers = NULL;
    }

  if (string_table)
    {
      free (string_table);
      string_table = NULL;
      string_table_length = 0;
    }

  if (dynamic_strings)
    {
      free (dynamic_strings);
      dynamic_strings = NULL;
      dynamic_strings_length = 0;
    }

  if (dynamic_symbols)
    {
      free (dynamic_symbols);
      dynamic_symbols = NULL;
      num_dynamic_syms = 0;
    }

  if (dynamic_syminfo)
    {
      free (dynamic_syminfo);
      dynamic_syminfo = NULL;
    }

  if (dynamic_section)
    {
      free (dynamic_section);
      dynamic_section = NULL;
    }

  if (section_headers_groups)
    {
      free (section_headers_groups);
      section_headers_groups = NULL;
    }

  if (section_groups)
    {
      struct group_list * g;
      struct group_list * next;

      for (i = 0; i < group_count; i++)
	{
	  for (g = section_groups [i].root; g != NULL; g = next)
	    {
	      next = g->next;
	      free (g);
	    }
	}

      free (section_groups);
      section_groups = NULL;
    }

  free_debug_memory ();

  return 0;
}

/* Process an ELF archive.
   On entry the file is positioned just after the ARMAG string.  */

static int
process_archive (char * file_name, FILE * file, bfd_boolean is_thin_archive)
{
  struct archive_info arch;
  struct archive_info nested_arch;
  size_t got;
  int ret;

  show_name = 1;

  /* The ARCH structure is used to hold information about this archive.  */
  arch.file_name = NULL;
  arch.file = NULL;
  arch.index_array = NULL;
  arch.sym_table = NULL;
  arch.longnames = NULL;

  /* The NESTED_ARCH structure is used as a single-item cache of information
     about a nested archive (when members of a thin archive reside within
     another regular archive file).  */
  nested_arch.file_name = NULL;
  nested_arch.file = NULL;
  nested_arch.index_array = NULL;
  nested_arch.sym_table = NULL;
  nested_arch.longnames = NULL;

  if (setup_archive (&arch, file_name, file, is_thin_archive, do_archive_index) != 0)
    {
      ret = 1;
      goto out;
    }

  if (do_archive_index)
    {
      if (arch.sym_table == NULL)
	error (_("%s: unable to dump the index as none was found\n"), file_name);
      else
	{
	  unsigned int i, l;
	  unsigned long current_pos;

	  printf (_("Index of archive %s: (%ld entries, 0x%lx bytes in the symbol table)\n"),
		  file_name, (long) arch.index_num, arch.sym_size);
	  current_pos = ftell (file);

	  for (i = l = 0; i < arch.index_num; i++)
	    {
	      if ((i == 0) || ((i > 0) && (arch.index_array[i] != arch.index_array[i - 1])))
	        {
	          char * member_name;

		  member_name = get_archive_member_name_at (&arch, arch.index_array[i], &nested_arch);

                  if (member_name != NULL)
                    {
	              char * qualified_name = make_qualified_name (&arch, &nested_arch, member_name);

                      if (qualified_name != NULL)
                        {
		          printf (_("Contents of binary %s at offset "), qualified_name);
			  (void) print_vma (arch.index_array[i], PREFIX_HEX);
			  putchar ('\n');
		          free (qualified_name);
		        }
		    }
		}

	      if (l >= arch.sym_size)
		{
		  error (_("%s: end of the symbol table reached before the end of the index\n"),
			 file_name);
		  break;
		}
	      printf ("\t%s\n", arch.sym_table + l);
	      l += strlen (arch.sym_table + l) + 1;
	    }

	  if (arch.uses_64bit_indicies)
	    l = (l + 7) & ~ 7;
	  else
	    l += l & 1;

	  if (l < arch.sym_size)
	    error (_("%s: %ld bytes remain in the symbol table, but without corresponding entries in the index table\n"),
		   file_name, arch.sym_size - l);

	  if (fseek (file, current_pos, SEEK_SET) != 0)
	    {
	      error (_("%s: failed to seek back to start of object files in the archive\n"), file_name);
	      ret = 1;
	      goto out;
	    }
	}

      if (!do_dynamic && !do_syms && !do_reloc && !do_unwind && !do_sections
	  && !do_segments && !do_header && !do_dump && !do_version
	  && !do_histogram && !do_debugging && !do_arch && !do_notes
	  && !do_section_groups && !do_dyn_syms)
	{
	  ret = 0; /* Archive index only.  */
	  goto out;
	}
    }

  ret = 0;

  while (1)
    {
      char * name;
      size_t namelen;
      char * qualified_name;

      /* Read the next archive header.  */
      if (fseek (file, arch.next_arhdr_offset, SEEK_SET) != 0)
        {
          error (_("%s: failed to seek to next archive header\n"), file_name);
          return 1;
        }
      got = fread (&arch.arhdr, 1, sizeof arch.arhdr, file);
      if (got != sizeof arch.arhdr)
        {
          if (got == 0)
	    break;
          error (_("%s: failed to read archive header\n"), file_name);
          ret = 1;
          break;
        }
      if (memcmp (arch.arhdr.ar_fmag, ARFMAG, 2) != 0)
        {
          error (_("%s: did not find a valid archive header\n"), arch.file_name);
          ret = 1;
          break;
        }

      arch.next_arhdr_offset += sizeof arch.arhdr;

      archive_file_size = strtoul (arch.arhdr.ar_size, NULL, 10);
      if (archive_file_size & 01)
        ++archive_file_size;

      name = get_archive_member_name (&arch, &nested_arch);
      if (name == NULL)
	{
	  error (_("%s: bad archive file name\n"), file_name);
	  ret = 1;
	  break;
	}
      namelen = strlen (name);

      qualified_name = make_qualified_name (&arch, &nested_arch, name);
      if (qualified_name == NULL)
	{
	  error (_("%s: bad archive file name\n"), file_name);
	  ret = 1;
	  break;
	}

      if (is_thin_archive && arch.nested_member_origin == 0)
        {
          /* This is a proxy for an external member of a thin archive.  */
          FILE * member_file;
          char * member_file_name = adjust_relative_path (file_name, name, namelen);
          if (member_file_name == NULL)
            {
              ret = 1;
              break;
            }

          member_file = fopen (member_file_name, "rb");
          if (member_file == NULL)
            {
              error (_("Input file '%s' is not readable.\n"), member_file_name);
              free (member_file_name);
              ret = 1;
              break;
            }

          archive_file_offset = arch.nested_member_origin;

          ret |= process_object (qualified_name, member_file);

          fclose (member_file);
          free (member_file_name);
        }
      else if (is_thin_archive)
        {
          /* This is a proxy for a member of a nested archive.  */
          archive_file_offset = arch.nested_member_origin + sizeof arch.arhdr;

          /* The nested archive file will have been opened and setup by
             get_archive_member_name.  */
          if (fseek (nested_arch.file, archive_file_offset, SEEK_SET) != 0)
            {
              error (_("%s: failed to seek to archive member.\n"), nested_arch.file_name);
              ret = 1;
              break;
            }

          ret |= process_object (qualified_name, nested_arch.file);
        }
      else
        {
          archive_file_offset = arch.next_arhdr_offset;
          arch.next_arhdr_offset += archive_file_size;

          ret |= process_object (qualified_name, file);
        }

      if (dump_sects != NULL)
	{
	  free (dump_sects);
	  dump_sects = NULL;
	  num_dump_sects = 0;
	}

      free (qualified_name);
    }

 out:
  if (nested_arch.file != NULL)
    fclose (nested_arch.file);
  release_archive (&nested_arch);
  release_archive (&arch);

  return ret;
}

static int
process_file (char * file_name)
{
  FILE * file;
  struct stat statbuf;
  char armag[SARMAG];
  int ret;

  if (stat (file_name, &statbuf) < 0)
    {
      if (errno == ENOENT)
	error (_("'%s': No such file\n"), file_name);
      else
	error (_("Could not locate '%s'.  System error message: %s\n"),
	       file_name, strerror (errno));
      return 1;
    }

  if (! S_ISREG (statbuf.st_mode))
    {
      error (_("'%s' is not an ordinary file\n"), file_name);
      return 1;
    }

  file = fopen (file_name, "rb");
  if (file == NULL)
    {
      error (_("Input file '%s' is not readable.\n"), file_name);
      return 1;
    }

  if (fread (armag, SARMAG, 1, file) != 1)
    {
      error (_("%s: Failed to read file's magic number\n"), file_name);
      fclose (file);
      return 1;
    }

  if (memcmp (armag, ARMAG, SARMAG) == 0)
    ret = process_archive (file_name, file, FALSE);
  else if (memcmp (armag, ARMAGT, SARMAG) == 0)
    ret = process_archive (file_name, file, TRUE);
  else
    {
      if (do_archive_index)
	error (_("File %s is not an archive so its index cannot be displayed.\n"),
	       file_name);

      rewind (file);
      archive_file_size = archive_file_offset = 0;
      ret = process_object (file_name, file);
    }

  fclose (file);

  return ret;
}

#ifdef SUPPORT_DISASSEMBLY
/* Needed by the i386 disassembler.  For extra credit, someone could
   fix this so that we insert symbolic addresses here, esp for GOT/PLT
   symbols.  */

void
print_address (unsigned int addr, FILE * outfile)
{
  fprintf (outfile,"0x%8.8x", addr);
}

/* Needed by the i386 disassembler.  */
void
db_task_printsym (unsigned int addr)
{
  print_address (addr, stderr);
}
#endif

int
main (int argc, char ** argv)
{
  int err;

#if defined (HAVE_SETLOCALE) && defined (HAVE_LC_MESSAGES)
  setlocale (LC_MESSAGES, "");
#endif
#if defined (HAVE_SETLOCALE)
  setlocale (LC_CTYPE, "");
#endif
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);

  expandargv (&argc, &argv);

  parse_args (argc, argv);

  if (num_dump_sects > 0)
    {
      /* Make a copy of the dump_sects array.  */
      cmdline_dump_sects = (dump_type *)
          malloc (num_dump_sects * sizeof (* dump_sects));
      if (cmdline_dump_sects == NULL)
	error (_("Out of memory allocating dump request table.\n"));
      else
	{
	  memcpy (cmdline_dump_sects, dump_sects,
		  num_dump_sects * sizeof (* dump_sects));
	  num_cmdline_dump_sects = num_dump_sects;
	}
    }

  if (optind < (argc - 1))
    show_name = 1;

  err = 0;
  while (optind < argc)
    err |= process_file (argv[optind++]);

  if (dump_sects != NULL)
    free (dump_sects);
  if (cmdline_dump_sects != NULL)
    free (cmdline_dump_sects);

  return err;
}
