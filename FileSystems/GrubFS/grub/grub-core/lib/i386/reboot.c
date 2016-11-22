/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2011  Free Software Foundation, Inc.
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

#include <grub/relocator.h>
#include <grub/cpu/relocator.h>
#include <grub/misc.h>
#include <grub/mm.h>
#include <grub/cpu/reboot.h>
#include <grub/i386/floppy.h>

void
grub_reboot (void)
{
  struct grub_relocator *relocator = NULL;
  grub_relocator_chunk_t ch;
  grub_err_t err;
  void *buf;
  struct grub_relocator16_state state;
  grub_uint16_t segment;

  relocator = grub_relocator_new ();
  if (!relocator)
    while (1);
  err = grub_relocator_alloc_chunk_align (relocator, &ch, 0x1000, 0x1000,
					  grub_reboot_end - grub_reboot_start,
					  16, GRUB_RELOCATOR_PREFERENCE_NONE,
					  0);
  if (err)
    while (1);
  buf = get_virtual_current_address (ch);
  grub_memcpy (buf, grub_reboot_start, grub_reboot_end - grub_reboot_start);

  segment = ((grub_addr_t) get_physical_target_address (ch)) >> 4;
  state.gs = state.fs = state.es = state.ds = state.ss = segment;
  state.sp = 0;
  state.cs = segment;
  state.ip = 0;
  state.a20 = 0;

  grub_stop_floppy ();
  
  err = grub_relocator16_boot (relocator, state);

  while (1);
}
