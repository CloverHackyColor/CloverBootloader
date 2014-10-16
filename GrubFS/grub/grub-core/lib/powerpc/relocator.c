/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2009,2010  Free Software Foundation, Inc.
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

#include <grub/mm.h>
#include <grub/misc.h>

#include <grub/types.h>
#include <grub/types.h>
#include <grub/err.h>
#include <grub/cache.h>

#include <grub/powerpc/relocator.h>
#include <grub/relocator_private.h>

extern grub_uint8_t grub_relocator_forward_start;
extern grub_uint8_t grub_relocator_forward_end;
extern grub_uint8_t grub_relocator_backward_start;
extern grub_uint8_t grub_relocator_backward_end;

#define REGW_SIZEOF (2 * sizeof (grub_uint32_t))
#define JUMP_SIZEOF (sizeof (grub_uint32_t))

#define RELOCATOR_SRC_SIZEOF(x) (&grub_relocator_##x##_end \
				 - &grub_relocator_##x##_start)
#define RELOCATOR_SIZEOF(x)	(RELOCATOR_SRC_SIZEOF(x) \
				 + REGW_SIZEOF * 3)
grub_size_t grub_relocator_align = sizeof (grub_uint32_t);
grub_size_t grub_relocator_forward_size;
grub_size_t grub_relocator_backward_size;
grub_size_t grub_relocator_jumper_size = JUMP_SIZEOF + REGW_SIZEOF;

void
grub_cpu_relocator_init (void)
{
  grub_relocator_forward_size = RELOCATOR_SIZEOF(forward);
  grub_relocator_backward_size = RELOCATOR_SIZEOF(backward);
}

static void
write_reg (int regn, grub_uint32_t val, void **target)
{
  /* lis r, val >> 16  */
  *(grub_uint32_t *) *target = 
    ((0x3c00 | (regn << 5)) << 16) | (val >> 16);
  *target = ((grub_uint32_t *) *target) + 1;
  /* ori r, r, val & 0xffff. */
  *(grub_uint32_t *) *target = (((0x6000 | regn << 5 | regn) << 16)
				| (val & 0xffff));
  *target = ((grub_uint32_t *) *target) + 1;
}

static void
write_jump (void **target)
{
  /* blr.  */
  *(grub_uint32_t *) *target = 0x4e800020;
  *target = ((grub_uint32_t *) *target) + 1;
}

void
grub_cpu_relocator_jumper (void *rels, grub_addr_t addr)
{
  write_reg (GRUB_PPC_JUMP_REGISTER, addr, &rels);
  write_jump (&rels);
}

void
grub_cpu_relocator_backward (void *ptr0, void *src, void *dest,
			     grub_size_t size)
{
  void *ptr = ptr0;
  write_reg (8, (grub_uint32_t) src, &ptr);
  write_reg (9, (grub_uint32_t) dest, &ptr);
  write_reg (10, (grub_uint32_t) size, &ptr);
  grub_memcpy (ptr, &grub_relocator_backward_start,
	       RELOCATOR_SRC_SIZEOF (backward));
}

void
grub_cpu_relocator_forward (void *ptr0, void *src, void *dest,
			     grub_size_t size)
{
  void *ptr = ptr0;
  write_reg (8, (grub_uint32_t) src, &ptr);
  write_reg (9, (grub_uint32_t) dest, &ptr);
  write_reg (10, (grub_uint32_t) size, &ptr);
  grub_memcpy (ptr, &grub_relocator_forward_start, 
	       RELOCATOR_SRC_SIZEOF (forward));
}

grub_err_t
grub_relocator32_boot (struct grub_relocator *rel,
		       struct grub_relocator32_state state)
{
  void *ptr;
  grub_err_t err;
  void *relst;
  grub_size_t relsize;
  grub_size_t stateset_size = 32 * REGW_SIZEOF + JUMP_SIZEOF;
  unsigned i;
  grub_relocator_chunk_t ch;

  err = grub_relocator_alloc_chunk_align (rel, &ch, 0,
					  (0xffffffff - stateset_size)
					  + 1, stateset_size,
					  sizeof (grub_uint32_t),
					  GRUB_RELOCATOR_PREFERENCE_NONE, 0);
  if (err)
    return err;

  ptr = get_virtual_current_address (ch);
  for (i = 0; i < 32; i++)
    write_reg (i, state.gpr[i], &ptr);
  write_jump (&ptr);

  err = grub_relocator_prepare_relocs (rel, get_physical_target_address (ch),
				       &relst, &relsize);
  if (err)
    return err;

  grub_arch_sync_caches ((void *) relst, relsize);

  ((void (*) (void)) relst) ();

  /* Not reached.  */
  return GRUB_ERR_NONE;
}
