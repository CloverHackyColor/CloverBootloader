/* dl_helper.c - relocation helper functions for modules and grub-mkimage */
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
#include <grub/arm64/reloc.h>

/*
 * grub_arm64_reloc_xxxx26():
 *
 * JUMP26/CALL26 relocations for B and BL instructions.
 */

int
grub_arm_64_check_xxxx26_offset (grub_int64_t offset)
{
  const grub_ssize_t offset_low = -(1 << 27), offset_high = (1 << 27) - 1;

  if ((offset < offset_low) || (offset > offset_high))
    return 0;
  return 1;
}

void
grub_arm64_set_xxxx26_offset (grub_uint32_t *place, grub_int64_t offset)
{
  const grub_uint32_t insmask = grub_cpu_to_le32_compile_time (0xfc000000);

  grub_dprintf ("dl", "  reloc_xxxx64 %p %c= 0x%llx\n",
		place, offset > 0 ? '+' : '-',
		offset < 0 ? (long long) -(unsigned long long) offset : offset);

  *place &= insmask;
  *place |= grub_cpu_to_le32 (offset >> 2) & ~insmask;
}
