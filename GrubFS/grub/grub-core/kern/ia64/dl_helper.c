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

#pragma GCC diagnostic ignored "-Wcast-align"

#define MASK20 ((1 << 20) - 1)
#define MASK3 (~(grub_addr_t) 3)

void
grub_ia64_add_value_to_slot_20b (grub_addr_t addr, grub_uint32_t value)
{
  grub_uint32_t val;
  switch (addr & 3)
    {
    case 0:
      val = grub_le_to_cpu32 (grub_get_unaligned32 (((grub_uint8_t *)
						     (addr & MASK3) + 2)));
      val = (((((val & MASK20) + value) & MASK20) << 2) 
	    | (val & ~(MASK20 << 2)));
      grub_set_unaligned32 (((grub_uint8_t *) (addr & MASK3) + 2),
			    grub_cpu_to_le32 (val));
      break;
    case 1:
      val = grub_le_to_cpu32 (grub_get_unaligned32 (((grub_uint8_t *)
						     (addr & MASK3) + 7)));
      val = ((((((val >> 3) & MASK20) + value) & MASK20) << 3)
	    | (val & ~(MASK20 << 3)));
      grub_set_unaligned32 (((grub_uint8_t *) (addr & MASK3) + 7),
			    grub_cpu_to_le32 (val));
      break;
    case 2:
      val = grub_le_to_cpu32 (grub_get_unaligned32 (((grub_uint8_t *)
						     (addr & MASK3) + 12)));
      val = ((((((val >> 4) & MASK20) + value) & MASK20) << 4)
	    | (val & ~(MASK20 << 4)));
      grub_set_unaligned32 (((grub_uint8_t *) (addr & MASK3) + 12),
			    grub_cpu_to_le32 (val));
      break;
    }
}

#define MASKF21 ( ((1 << 23) - 1) & ~((1 << 7) | (1 << 8)) )

static grub_uint32_t
add_value_to_slot_21_real (grub_uint32_t a, grub_uint32_t value)
{
  grub_uint32_t high, mid, low, c;
  low  = (a & 0x00007f);
  mid  = (a & 0x7fc000) >> 7;
  high = (a & 0x003e00) << 7;
  c = (low | mid | high) + value;
  return (c & 0x7f) | ((c << 7) & 0x7fc000) | ((c >> 7) & 0x0003e00); //0x003e00
}

void
grub_ia64_add_value_to_slot_21 (grub_addr_t addr, grub_uint32_t value)
{
  grub_uint32_t val;
  switch (addr & 3)
    {
    case 0:
      val = grub_le_to_cpu32 (grub_get_unaligned32 (((grub_uint8_t *)
						     (addr & MASK3) + 2)));
      val = ((add_value_to_slot_21_real (((val >> 2) & MASKF21), value)
	      & MASKF21) << 2) | (val & ~(MASKF21 << 2));
      grub_set_unaligned32 (((grub_uint8_t *) (addr & MASK3) + 2),
			    grub_cpu_to_le32 (val));
      break;
    case 1:
      val = grub_le_to_cpu32 (grub_get_unaligned32 (((grub_uint8_t *)
						     (addr & MASK3) + 7)));
      val = ((add_value_to_slot_21_real (((val >> 3) & MASKF21), value)
	      & MASKF21) << 3) | (val & ~(MASKF21 << 3));
      grub_set_unaligned32 (((grub_uint8_t *) (addr & MASK3) + 7),
			    grub_cpu_to_le32 (val));
      break;
    case 2:
      val = grub_le_to_cpu32 (grub_get_unaligned32 (((grub_uint8_t *)
						     (addr & MASK3) + 12)));
      val = ((add_value_to_slot_21_real (((val >> 4) & MASKF21), value)
	      & MASKF21) << 4) | (val & ~(MASKF21 << 4));
      grub_set_unaligned32 (((grub_uint8_t *) (addr & MASK3) + 12),
			    grub_cpu_to_le32 (val));
      break;
    }
}

static const grub_uint8_t nopm[5] =
  {
    /* [MLX]       nop.m 0x0 */
    0x05, 0x00, 0x00, 0x00, 0x01
  };

#ifdef GRUB_UTIL
static grub_uint8_t jump[0x20] =
  {
    /* [MMI]       add r15=r15,r1;; */
    0x0b, 0x78, 0x3c, 0x02, 0x00, 0x20,
    /* ld8 r16=[r15],8 */
    0x00, 0x41, 0x3c, 0x30, 0x28, 0xc0,
    /* mov r14=r1;; */
    0x01, 0x08, 0x00, 0x84,
    /* 	[MIB]       ld8 r1=[r15] */
    0x11, 0x08, 0x00, 0x1e, 0x18, 0x10,
    /* mov b6=r16 */
    0x60, 0x80, 0x04, 0x80, 0x03, 0x00, 
    /* br.few b6;; */
    0x60, 0x00, 0x80, 0x00       	            
  };
#else
static const grub_uint8_t jump[0x20] =
  {
    /* ld8 r16=[r15],8 */
    0x02, 0x80, 0x20, 0x1e, 0x18, 0x14,
    /* mov r14=r1;; */
    0xe0, 0x00, 0x04, 0x00, 0x42, 0x00,
    /* nop.i 0x0 */
    0x00, 0x00, 0x04, 0x00,
    /* ld8 r1=[r15] */
    0x11, 0x08, 0x00, 0x1e, 0x18, 0x10,
    /* mov b6=r16 */
    0x60, 0x80, 0x04, 0x80, 0x03, 0x00,
    /* br.few b6;; */
    0x60, 0x00, 0x80, 0x00
  };
#endif

void
grub_ia64_make_trampoline (struct grub_ia64_trampoline *tr, grub_uint64_t addr)
{
  grub_memcpy (tr->nop, nopm, sizeof (tr->nop));
  tr->addr_hi[0] = ((addr & 0xc00000) >> 16);
  tr->addr_hi[1] = (addr >> 24) & 0xff;
  tr->addr_hi[2] = (addr >> 32) & 0xff;
  tr->addr_hi[3] = (addr >> 40) & 0xff;
  tr->addr_hi[4] = (addr >> 48) & 0xff;
  tr->addr_hi[5] = (addr >> 56) & 0xff;
  tr->e0 = 0xe0;
  tr->addr_lo[0] = ((addr & 0x000f) << 4) | 0x01;
  tr->addr_lo[1] = (((addr & 0x0070) >> 4) | ((addr & 0x070000) >> 11)
		    | ((addr & 0x200000) >> 17));
  tr->addr_lo[2] = ((addr & 0x1f80) >> 5) | ((addr & 0x180000) >> 19);
  tr->addr_lo[3] = ((addr & 0xe000) >> 13) | 0x60;
  grub_memcpy (tr->jump, jump, sizeof (tr->jump));
}

grub_err_t
grub_ia64_dl_get_tramp_got_size (const void *ehdr, grub_size_t *tramp,
				 grub_size_t *got)
{
  const Elf64_Ehdr *e = ehdr;
  grub_size_t cntt = 0, cntg = 0;
  const Elf64_Shdr *s;
  unsigned i;

  for (i = 0, s = (Elf64_Shdr *) ((char *) e + grub_le_to_cpu64 (e->e_shoff));
       i < grub_le_to_cpu16 (e->e_shnum);
       i++, s = (Elf64_Shdr *) ((char *) s + grub_le_to_cpu16 (e->e_shentsize)))
    if (s->sh_type == grub_cpu_to_le32_compile_time (SHT_RELA))
      {
	Elf64_Rela *rel, *max;

	for (rel = (Elf64_Rela *) ((char *) e + grub_le_to_cpu64 (s->sh_offset)),
	       max = rel + grub_le_to_cpu64 (s->sh_size) / grub_le_to_cpu64 (s->sh_entsize);
	     rel < max; rel++)
	  switch (ELF64_R_TYPE (grub_le_to_cpu64 (rel->r_info)))
	    {
	    case R_IA64_PCREL21B:
	      cntt++;
	      break;
	    case R_IA64_LTOFF_FPTR22:
	    case R_IA64_LTOFF22X:
	    case R_IA64_LTOFF22:
	      cntg++;
	      break;
	    }
      }
  *tramp = cntt * sizeof (struct grub_ia64_trampoline);
  *got = cntg * sizeof (grub_uint64_t);

  return GRUB_ERR_NONE;
}

