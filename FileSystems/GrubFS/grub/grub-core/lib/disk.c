/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2002,2003,2004,2006,2007,2008,2009,2010  Free Software Foundation, Inc.
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

#include <grub/disk.h>
#include <grub/err.h>
#include <grub/mm.h>
#include <grub/types.h>
#include <grub/partition.h>
#include <grub/misc.h>
#include <grub/time.h>
#include <grub/file.h>
#include <grub/i18n.h>

GRUB_MOD_LICENSE ("GPLv3+");

#include "../kern/disk_common.c"

static void
grub_disk_cache_invalidate (unsigned long dev_id, unsigned long disk_id,
			    grub_disk_addr_t sector)
{
  unsigned cache_index;
  struct grub_disk_cache *cache;

  sector &= ~((grub_disk_addr_t) GRUB_DISK_CACHE_SIZE - 1);
  cache_index = grub_disk_cache_get_index (dev_id, disk_id, sector);
  cache = grub_disk_cache_table + cache_index;

  if (cache->dev_id == dev_id && cache->disk_id == disk_id
      && cache->sector == sector && cache->data)
    {
      cache->lock = 1;
      grub_free (cache->data);
      cache->data = 0;
      cache->lock = 0;
    }
}

grub_err_t
grub_disk_write (grub_disk_t disk, grub_disk_addr_t sector,
		 grub_off_t offset, grub_size_t size, const void *buf)
{
  unsigned real_offset;
  grub_disk_addr_t aligned_sector;

  grub_dprintf ("disk", "Writing `%s'...\n", disk->name);

  if (grub_disk_adjust_range (disk, &sector, &offset, size) != GRUB_ERR_NONE)
    return -1;

  aligned_sector = (sector & ~((1ULL << (disk->log_sector_size
					 - GRUB_DISK_SECTOR_BITS)) - 1));
  real_offset = offset + ((sector - aligned_sector) << GRUB_DISK_SECTOR_BITS);
  sector = aligned_sector;

  while (size)
    {
      if (real_offset != 0 || (size < (1U << disk->log_sector_size)
			       && size != 0))
	{
	  char *tmp_buf;
	  grub_size_t len;
	  grub_partition_t part;

	  tmp_buf = grub_malloc (1U << disk->log_sector_size);
	  if (!tmp_buf)
	    return grub_errno;

	  part = disk->partition;
	  disk->partition = 0;
	  if (grub_disk_read (disk, sector,
			      0, (1U << disk->log_sector_size), tmp_buf)
	      != GRUB_ERR_NONE)
	    {
	      disk->partition = part;
	      grub_free (tmp_buf);
	      goto finish;
	    }
	  disk->partition = part;

	  len = (1U << disk->log_sector_size) - real_offset;
	  if (len > size)
	    len = size;

	  grub_memcpy (tmp_buf + real_offset, buf, len);

	  grub_disk_cache_invalidate (disk->dev->id, disk->id, sector);

	  if ((disk->dev->write) (disk, transform_sector (disk, sector),
				  1, tmp_buf) != GRUB_ERR_NONE)
	    {
	      grub_free (tmp_buf);
	      goto finish;
	    }

	  grub_free (tmp_buf);

	  sector += (1U << (disk->log_sector_size - GRUB_DISK_SECTOR_BITS));
	  buf = (const char *) buf + len;
	  size -= len;
	  real_offset = 0;
	}
      else
	{
	  grub_size_t len;
	  grub_size_t n;

	  len = size & ~((1ULL << disk->log_sector_size) - 1);
	  n = size >> disk->log_sector_size;

	  if (n > (disk->max_agglomerate
		   << (GRUB_DISK_CACHE_BITS + GRUB_DISK_SECTOR_BITS
		       - disk->log_sector_size)))
	    n = (disk->max_agglomerate
		 << (GRUB_DISK_CACHE_BITS + GRUB_DISK_SECTOR_BITS
		     - disk->log_sector_size));

	  if ((disk->dev->write) (disk, transform_sector (disk, sector),
				  n, buf) != GRUB_ERR_NONE)
	    goto finish;

	  while (n--)
	    {
	      grub_disk_cache_invalidate (disk->dev->id, disk->id, sector);
	      sector += (1U << (disk->log_sector_size - GRUB_DISK_SECTOR_BITS));
	    }

	  buf = (const char *) buf + len;
	  size -= len;
	}
    }

 finish:

  return grub_errno;
}

GRUB_MOD_INIT(disk)
{
  grub_disk_write_weak = grub_disk_write;
}

GRUB_MOD_FINI(disk)
{
  grub_disk_write_weak = NULL;
}
