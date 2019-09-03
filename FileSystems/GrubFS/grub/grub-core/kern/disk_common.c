/* This function performs three tasks:
   - Make sectors disk relative from partition relative.
   - Normalize offset to be less than the sector size.
   - Verify that the range is inside the partition.  */
static grub_err_t
grub_disk_adjust_range (grub_disk_t disk, grub_disk_addr_t *sector,
			grub_off_t *offset, grub_size_t size)
{
  grub_partition_t part;
  *sector += *offset >> GRUB_DISK_SECTOR_BITS;
  *offset &= GRUB_DISK_SECTOR_SIZE - 1;

  for (part = disk->partition; part; part = part->parent)
    {
      grub_disk_addr_t start;
      grub_uint64_t len;

      start = part->start;
      len = part->len;

      if (*sector >= len
	  || len - *sector < ((*offset + size + GRUB_DISK_SECTOR_SIZE - 1)
			      >> GRUB_DISK_SECTOR_BITS))
	return grub_error (GRUB_ERR_OUT_OF_RANGE,
			   N_("attempt to read or write outside of partition"));

      *sector += start;
    }

  if (disk->total_sectors != GRUB_DISK_SIZE_UNKNOWN
      && ((disk->total_sectors << (disk->log_sector_size - GRUB_DISK_SECTOR_BITS)) <= *sector
	  || ((*offset + size + GRUB_DISK_SECTOR_SIZE - 1)
	  >> GRUB_DISK_SECTOR_BITS) > (disk->total_sectors
				       << (disk->log_sector_size
					   - GRUB_DISK_SECTOR_BITS)) - *sector))
    return grub_error (GRUB_ERR_OUT_OF_RANGE,
		       N_("attempt to read or write outside of disk `%s'"), disk->name);

  return GRUB_ERR_NONE;
}

static inline grub_disk_addr_t
transform_sector (grub_disk_t disk, grub_disk_addr_t sector)
{
  return sector >> (disk->log_sector_size - GRUB_DISK_SECTOR_BITS);
}

static unsigned
grub_disk_cache_get_index (unsigned long dev_id, unsigned long disk_id,
			   grub_disk_addr_t sector)
{
  return ((dev_id * 524287UL + disk_id * 2606459UL
	   + ((unsigned) (sector >> GRUB_DISK_CACHE_BITS)))
	  % GRUB_DISK_CACHE_NUM);
}
