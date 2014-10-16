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
#include <grub/arm/reloc.h>

/*
 * R_ARM_ABS32
 *
 * Simple relocation of 32-bit value (in literal pool)
 */
grub_err_t
grub_arm_reloc_abs32 (Elf32_Word *target, Elf32_Addr sym_addr)
{
  Elf32_Addr tmp;

  tmp = grub_le_to_cpu32 (*target);
  tmp += sym_addr;
  *target = grub_cpu_to_le32 (tmp);

  return GRUB_ERR_NONE;
}

/********************************************************************
 * Thumb (T32) relocations:                                         *
 *                                                                  *
 * 32-bit Thumb instructions can be 16-bit aligned, and are fetched *
 * little-endian, requiring some additional fiddling.               *
 ********************************************************************/

grub_int32_t
grub_arm_thm_call_get_offset (grub_uint16_t *target)
{
  grub_uint32_t sign, j1, j2;
  grub_uint32_t insword;
  grub_int32_t offset;

  /* Extract instruction word in alignment-safe manner */
  insword = (grub_le_to_cpu16 (*target) << 16)
    | (grub_le_to_cpu16(*(target + 1)));

  /* Extract bitfields from instruction words */
  sign = (insword >> 26) & 1;
  j1 = (insword >> 13) & 1;
  j2 = (insword >> 11) & 1;
  offset = (sign << 24) | ((~(j1 ^ sign) & 1) << 23) |
    ((~(j2 ^ sign) & 1) << 22) |
    ((insword & 0x03ff0000) >> 4) | ((insword & 0x000007ff) << 1);

  /* Sign adjust and calculate offset */
  if (offset & (1 << 24))
    offset -= (1 << 25);

  return offset;
}

grub_err_t
grub_arm_thm_call_set_offset (grub_uint16_t *target, grub_int32_t offset)
{
  grub_uint32_t sign, j1, j2;
  const grub_uint32_t insmask = 0xf800d000;
  grub_uint32_t insword;
  int is_blx;

  /* Extract instruction word in alignment-safe manner */
  insword = (grub_le_to_cpu16 (*target) << 16)
    | (grub_le_to_cpu16(*(target + 1)));

  if (((insword >> 12) & 0xd) == 0xc)
    is_blx = 1;
  else
    is_blx = 0;

  if (!is_blx && !(offset & 1))
    return grub_error (GRUB_ERR_BAD_MODULE, "bl/b.w targettting ARM");

  /* Transform blx into bl if necessarry.  */
  if (is_blx && (offset & 1))
    insword |= (1 << 12);

  /* Reassemble instruction word */
  sign = (offset >> 24) & 1;
  j1 = sign ^ (~(offset >> 23) & 1);
  j2 = sign ^ (~(offset >> 22) & 1);
  insword = (insword & insmask) |
    (sign << 26) |
    (((offset >> 12) & 0x03ff) << 16) |
    (j1 << 13) | (j2 << 11) | ((offset >> 1) & 0x07ff);

  /* Write instruction word back in alignment-safe manner */
  *target = grub_cpu_to_le16 ((insword >> 16) & 0xffff);
  *(target + 1) = grub_cpu_to_le16 (insword & 0xffff);

  grub_dprintf ("dl", "    *insword = 0x%08x", insword);

  return GRUB_ERR_NONE;
}

grub_int32_t
grub_arm_thm_jump19_get_offset (grub_uint16_t *target)
{
  grub_int32_t offset;
  grub_uint32_t insword;

  /* Extract instruction word in alignment-safe manner */
  insword = (grub_le_to_cpu16 (*target) << 16)
    | (grub_le_to_cpu16(*(target + 1)));

  /* Extract and sign extend offset */
  offset = ((insword >> 26) & 1) << 19
    | ((insword >> 11) & 1) << 18
    | ((insword >> 13) & 1) << 17
    | ((insword >> 16) & 0x3f) << 11
    | (insword & 0x7ff);
  offset <<= 1;
  if (offset & (1 << 20))
    offset -= (1 << 21);

  return offset;
}

void
grub_arm_thm_jump19_set_offset (grub_uint16_t *target, grub_int32_t offset)
{
  grub_uint32_t insword;
  const grub_uint32_t insmask = 0xfbc0d000;

  offset >>= 1;
  offset &= 0xfffff;

  /* Extract instruction word in alignment-safe manner */
  insword = grub_le_to_cpu16 ((*target)) << 16
    | grub_le_to_cpu16 (*(target + 1));

  /* Reassemble instruction word and write back */
  insword &= insmask;
  insword |= ((offset >> 19) & 1) << 26
    | ((offset >> 18) & 1) << 11
    | ((offset >> 17) & 1) << 13
    | ((offset >> 11) & 0x3f) << 16
    | (offset & 0x7ff);
  *target = grub_cpu_to_le16 (insword >> 16);
  *(target + 1) = grub_cpu_to_le16 (insword & 0xffff);
}

int
grub_arm_thm_jump19_check_offset (grub_int32_t offset)
{
  if ((offset > 1048574) || (offset < -1048576))
    return 0;
  return 1;
}


/***********************************************************
 * ARM (A32) relocations:                                  *
 *                                                         *
 * ARM instructions are 32-bit in size and 32-bit aligned. *
 ***********************************************************/

grub_int32_t
grub_arm_jump24_get_offset (grub_uint32_t *target)
{
  grub_int32_t offset;
  grub_uint32_t insword;

  insword = grub_le_to_cpu32 (*target);

  offset = (insword & 0x00ffffff) << 2;
  if (offset & 0x02000000)
    offset -= 0x04000000;
  return offset;
}

int
grub_arm_jump24_check_offset (grub_int32_t offset)
{
  if (offset >= 0x02000000 || offset < -0x02000000)
    return 0;
  return 1;
}

void
grub_arm_jump24_set_offset (grub_uint32_t *target,
			    grub_int32_t offset)
{
  grub_uint32_t insword;

  insword = grub_le_to_cpu32 (*target);

  insword &= 0xff000000;
  insword |= (offset >> 2) & 0x00ffffff;

  *target = grub_cpu_to_le32 (insword);
}
