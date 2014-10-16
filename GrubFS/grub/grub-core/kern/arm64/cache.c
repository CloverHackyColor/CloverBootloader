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

#include <grub/cache.h>
#include <grub/misc.h>

static grub_int64_t dlinesz;
static grub_int64_t ilinesz;

/* Prototypes for asm functions. */
void grub_arch_clean_dcache_range (grub_addr_t beg, grub_addr_t end,
				   grub_uint64_t line_size);
void grub_arch_invalidate_icache_range (grub_addr_t beg, grub_addr_t end,
					grub_uint64_t line_size);

static void
probe_caches (void)
{
  grub_uint64_t cache_type;

  /* Read Cache Type Register */
  asm volatile ("mrs 	%0, ctr_el0": "=r"(cache_type));

  dlinesz = 4 << ((cache_type >> 16) & 0xf);
  ilinesz = 4 << (cache_type & 0xf);

  grub_dprintf("cache", "D$ line size: %lld\n", (long long) dlinesz);
  grub_dprintf("cache", "I$ line size: %lld\n", (long long) ilinesz);
}

void
grub_arch_sync_caches (void *address, grub_size_t len)
{
  grub_uint64_t start, end, max_align;

  if (dlinesz == 0)
    probe_caches();
  if (dlinesz == 0)
    grub_fatal ("Unknown cache line size!");

  max_align = dlinesz > ilinesz ? dlinesz : ilinesz;

  start = ALIGN_DOWN ((grub_uint64_t) address, max_align);
  end = ALIGN_UP ((grub_uint64_t) address + len, max_align);

  grub_arch_clean_dcache_range (start, end, dlinesz);
  grub_arch_invalidate_icache_range (start, end, ilinesz);
}
