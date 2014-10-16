/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2010  Free Software Foundation, Inc.
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
#include <grub/relocator_private.h>
#include <grub/memory.h>
#include <grub/ieee1275/ieee1275.h>

/* Helper for grub_relocator_firmware_get_max_events.  */
static int
count (grub_uint64_t addr __attribute__ ((unused)),
       grub_uint64_t len __attribute__ ((unused)),
       grub_memory_type_t type __attribute__ ((unused)), void *data)
{
  int *counter = data;

  (*counter)++;
  return 0;
}

unsigned 
grub_relocator_firmware_get_max_events (void)
{
  int counter = 0;

  if (grub_ieee1275_test_flag (GRUB_IEEE1275_FLAG_FORCE_CLAIM))
    return 0;
  grub_machine_mmap_iterate (count, &counter);
  return 2 * counter;
}

/* Context for grub_relocator_firmware_fill_events.  */
struct grub_relocator_firmware_fill_events_ctx
{
  struct grub_relocator_mmap_event *events;
  int counter;
};

/* Helper for grub_relocator_firmware_fill_events.  */
static int
grub_relocator_firmware_fill_events_iter (grub_uint64_t addr,
					  grub_uint64_t len,
					  grub_memory_type_t type, void *data)
{
  struct grub_relocator_firmware_fill_events_ctx *ctx = data;

  if (type != GRUB_MEMORY_AVAILABLE)
    return 0;

  if (grub_ieee1275_test_flag (GRUB_IEEE1275_FLAG_NO_PRE1_5M_CLAIM))
    {
      if (addr + len <= 0x180000)
	return 0;

      if (addr < 0x180000)
	{
	  len = addr + len - 0x180000;
	  addr = 0x180000;
	}
    }

  ctx->events[ctx->counter].type = REG_FIRMWARE_START;
  ctx->events[ctx->counter].pos = addr;
  ctx->counter++;
  ctx->events[ctx->counter].type = REG_FIRMWARE_END;
  ctx->events[ctx->counter].pos = addr + len;
  ctx->counter++;

  return 0;
}

unsigned 
grub_relocator_firmware_fill_events (struct grub_relocator_mmap_event *events)
{
  struct grub_relocator_firmware_fill_events_ctx ctx = {
    .events = events,
    .counter = 0
  };

  if (grub_ieee1275_test_flag (GRUB_IEEE1275_FLAG_FORCE_CLAIM))
    return 0;
  grub_machine_mmap_iterate (grub_relocator_firmware_fill_events_iter, &ctx);
  return ctx.counter;
}

int
grub_relocator_firmware_alloc_region (grub_addr_t start, grub_size_t size)
{
  grub_err_t err;
  err = grub_claimmap (start, size);
  grub_errno = 0;
  return (err == 0);
}

void
grub_relocator_firmware_free_region (grub_addr_t start, grub_size_t size)
{
  grub_ieee1275_release (start, size);
}
