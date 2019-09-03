/* cbfs.c - cbfs and tar filesystem.  */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2007,2008,2009,2013 Free Software Foundation, Inc.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <grub/misc.h>
#include <grub/disk.h>
#include <grub/archelp.h>

#include <grub/file.h>
#include <grub/mm.h>
#include <grub/dl.h>
#include <grub/i18n.h>
#include <grub/cbfs_core.h>

GRUB_MOD_LICENSE ("GPLv3+");


struct grub_archelp_data
{
  grub_disk_t disk;
  grub_off_t hofs, next_hofs;
  grub_off_t dofs;
  grub_off_t size;
  grub_off_t cbfs_start;
  grub_off_t cbfs_end;
  grub_off_t cbfs_align;
};

static grub_err_t
grub_cbfs_find_file (struct grub_archelp_data *data, char **name,
		     grub_int32_t *mtime,
		     grub_uint32_t *mode)
{
  grub_size_t offset;
  for (;;
       data->dofs = data->hofs + offset,
	 data->next_hofs = ALIGN_UP (data->dofs + data->size, data->cbfs_align))
    {
      struct cbfs_file hd;
      grub_size_t namesize;

      data->hofs = data->next_hofs;

      if (data->hofs >= data->cbfs_end)
	{
	  *mode = GRUB_ARCHELP_ATTR_END;
	  return GRUB_ERR_NONE;
	}

      if (grub_disk_read (data->disk, 0, data->hofs, sizeof (hd), &hd))
	return grub_errno;

      if (grub_memcmp (hd.magic, CBFS_FILE_MAGIC, sizeof (hd.magic)) != 0)
	{
	  *mode = GRUB_ARCHELP_ATTR_END;
	  return GRUB_ERR_NONE;
	}
      data->size = grub_be_to_cpu32 (hd.len);
      (void) mtime;
      offset = grub_be_to_cpu32 (hd.offset);

      if (mode)
	*mode = GRUB_ARCHELP_ATTR_FILE | GRUB_ARCHELP_ATTR_NOTIME;

      namesize = offset;
      if (namesize >= sizeof (hd))
	namesize -= sizeof (hd);
      if (namesize == 0)
	continue;
      *name = grub_malloc (namesize + 1);
      if (*name == NULL)
	return grub_errno;

      if (grub_disk_read (data->disk, 0, data->hofs + sizeof (hd),
			  namesize, *name))
	{
	  grub_free (*name);
	  return grub_errno;
	}

      if ((*name)[0] == '\0')
	{
	  grub_free (*name);
	  *name = NULL;
	  continue;
	}

      (*name)[namesize] = 0;

      data->dofs = data->hofs + offset;
      data->next_hofs = ALIGN_UP (data->dofs + data->size, data->cbfs_align);
      return GRUB_ERR_NONE;
    }
}

static void
grub_cbfs_rewind (struct grub_archelp_data *data)
{
  data->next_hofs = data->cbfs_start;
}

static struct grub_archelp_ops arcops =
  {
    .find_file = grub_cbfs_find_file,
    .rewind = grub_cbfs_rewind
  };

static int
validate_head (struct cbfs_header *head)
{
  return (head->magic == grub_cpu_to_be32_compile_time (CBFS_HEADER_MAGIC)
	  && (head->version
	      == grub_cpu_to_be32_compile_time (CBFS_HEADER_VERSION1)
	      || head->version
	      == grub_cpu_to_be32_compile_time (CBFS_HEADER_VERSION2))
	  && (grub_be_to_cpu32 (head->bootblocksize)
	      < grub_be_to_cpu32 (head->romsize))
	  && (grub_be_to_cpu32 (head->offset)
	      < grub_be_to_cpu32 (head->romsize))
	  && (grub_be_to_cpu32 (head->offset)
	      + grub_be_to_cpu32 (head->bootblocksize)
	      < grub_be_to_cpu32 (head->romsize))
	  && head->align != 0
	  && (head->align & (head->align - 1)) == 0
	  && head->romsize != 0);
}

static struct grub_archelp_data *
grub_cbfs_mount (grub_disk_t disk)
{
  struct cbfs_file hd;
  struct grub_archelp_data *data;
  grub_uint32_t ptr;
  grub_off_t header_off;
  struct cbfs_header head;

  if (grub_disk_read (disk, grub_disk_get_size (disk) - 1,
		      GRUB_DISK_SECTOR_SIZE - sizeof (ptr),
		      sizeof (ptr), &ptr))
    goto fail;

  ptr = grub_cpu_to_le32 (ptr);
  header_off = (grub_disk_get_size (disk) << GRUB_DISK_SECTOR_BITS)
    + (grub_int32_t) ptr;

  if (grub_disk_read (disk, 0, header_off,
		      sizeof (head), &head))
    goto fail;

  if (!validate_head (&head))
    goto fail;

  data = (struct grub_archelp_data *) grub_zalloc (sizeof (*data));
  if (!data)
    goto fail;

  data->cbfs_start = (grub_disk_get_size (disk) << GRUB_DISK_SECTOR_BITS)
    - (grub_be_to_cpu32 (head.romsize) - grub_be_to_cpu32 (head.offset));
  data->cbfs_end = (grub_disk_get_size (disk) << GRUB_DISK_SECTOR_BITS)
    - grub_be_to_cpu32 (head.bootblocksize);
  data->cbfs_align = grub_be_to_cpu32 (head.align);

  if (data->cbfs_start >= (grub_disk_get_size (disk) << GRUB_DISK_SECTOR_BITS))
    goto fail;
  if (data->cbfs_end > (grub_disk_get_size (disk) << GRUB_DISK_SECTOR_BITS))
    data->cbfs_end = (grub_disk_get_size (disk) << GRUB_DISK_SECTOR_BITS);

  data->next_hofs = data->cbfs_start;

  if (grub_disk_read (disk, 0, data->cbfs_start, sizeof (hd), &hd))
    goto fail;

  if (grub_memcmp (hd.magic, CBFS_FILE_MAGIC, sizeof (CBFS_FILE_MAGIC) - 1))
    goto fail;

  data->disk = disk;

  return data;

fail:
  grub_error (GRUB_ERR_BAD_FS, "not a cbfs filesystem");
  return 0;
}

static grub_err_t
grub_cbfs_dir (grub_device_t device, const char *path_in,
	       grub_fs_dir_hook_t hook, void *hook_data)
{
  struct grub_archelp_data *data;
  grub_err_t err;

  data = grub_cbfs_mount (device->disk);
  if (!data)
    return grub_errno;

  err = grub_archelp_dir (data, &arcops,
			  path_in, hook, hook_data);

  grub_free (data);

  return err;
}

static grub_err_t
grub_cbfs_open (grub_file_t file, const char *name_in)
{
  struct grub_archelp_data *data;
  grub_err_t err;

  data = grub_cbfs_mount (file->device->disk);
  if (!data)
    return grub_errno;

  err = grub_archelp_open (data, &arcops, name_in);
  if (err)
    {
      grub_free (data);
    }
  else
    {
      file->data = data;
      file->size = data->size;
    }
  return err;
}

static grub_ssize_t
grub_cbfs_read (grub_file_t file, char *buf, grub_size_t len)
{
  struct grub_archelp_data *data;
  grub_ssize_t ret;

  data = file->data;
  data->disk->read_hook = file->read_hook;
  data->disk->read_hook_data = file->read_hook_data;

  ret = (grub_disk_read (data->disk, 0, data->dofs + file->offset,
			 len, buf)) ? -1 : (grub_ssize_t) len;
  data->disk->read_hook = 0;

  return ret;
}

static grub_err_t
grub_cbfs_close (grub_file_t file)
{
  struct grub_archelp_data *data;

  data = file->data;
  grub_free (data);

  return grub_errno;
}

#if (defined (__i386__) || defined (__x86_64__)) && !defined (GRUB_UTIL) \
  && !defined (GRUB_MACHINE_EMU) && !defined (GRUB_MACHINE_XEN)

static char *cbfsdisk_addr;
static grub_off_t cbfsdisk_size = 0;

static int
grub_cbfsdisk_iterate (grub_disk_dev_iterate_hook_t hook, void *hook_data,
		       grub_disk_pull_t pull)
{
  if (pull != GRUB_DISK_PULL_NONE)
    return 0;

  return hook ("cbfsdisk", hook_data);
}

static grub_err_t
grub_cbfsdisk_open (const char *name, grub_disk_t disk)
{
  if (grub_strcmp (name, "cbfsdisk"))
      return grub_error (GRUB_ERR_UNKNOWN_DEVICE, "not a cbfsdisk");

  disk->total_sectors = cbfsdisk_size / GRUB_DISK_SECTOR_SIZE;
  disk->max_agglomerate = GRUB_DISK_MAX_MAX_AGGLOMERATE;
  disk->id = 0;

  return GRUB_ERR_NONE;
}

static void
grub_cbfsdisk_close (grub_disk_t disk __attribute((unused)))
{
}

static grub_err_t
grub_cbfsdisk_read (grub_disk_t disk __attribute((unused)),
		    grub_disk_addr_t sector,
		    grub_size_t size, char *buf)
{
  grub_memcpy (buf, cbfsdisk_addr + (sector << GRUB_DISK_SECTOR_BITS),
	       size << GRUB_DISK_SECTOR_BITS);
  return 0;
}

static grub_err_t
grub_cbfsdisk_write (grub_disk_t disk __attribute__ ((unused)),
		     grub_disk_addr_t sector __attribute__ ((unused)),
		     grub_size_t size __attribute__ ((unused)),
		     const char *buf __attribute__ ((unused)))
{
  return grub_error (GRUB_ERR_NOT_IMPLEMENTED_YET,
		     "rom flashing isn't implemented yet");
}

static struct grub_disk_dev grub_cbfsdisk_dev =
  {
    .name = "cbfsdisk",
    .id = GRUB_DISK_DEVICE_CBFSDISK_ID,
    .iterate = grub_cbfsdisk_iterate,
    .open = grub_cbfsdisk_open,
    .close = grub_cbfsdisk_close,
    .read = grub_cbfsdisk_read,
    .write = grub_cbfsdisk_write,
    .next = 0
  };

static void
init_cbfsdisk (void)
{
  grub_uint32_t ptr;
  struct cbfs_header *head;

  ptr = *(grub_uint32_t *) 0xfffffffc;
  head = (struct cbfs_header *) (grub_addr_t) ptr;

  if (!validate_head (head))
    return;

  cbfsdisk_size = ALIGN_UP (grub_be_to_cpu32 (head->romsize),
			    GRUB_DISK_SECTOR_SIZE);
  cbfsdisk_addr = (void *) (grub_addr_t) (0x100000000ULL - cbfsdisk_size);

  grub_disk_dev_register (&grub_cbfsdisk_dev);
}

static void
fini_cbfsdisk (void)
{
  if (! cbfsdisk_size)
    return;
  grub_disk_dev_unregister (&grub_cbfsdisk_dev);
}

#endif

static struct grub_fs grub_cbfs_fs = {
  .name = "cbfs",
  .dir = grub_cbfs_dir,
  .open = grub_cbfs_open,
  .read = grub_cbfs_read,
  .close = grub_cbfs_close,
#ifdef GRUB_UTIL
  .reserved_first_sector = 0,
  .blocklist_install = 0,
#endif
};

GRUB_MOD_INIT (cbfs)
{
#if (defined (__i386__) || defined (__x86_64__)) && !defined (GRUB_UTIL) && !defined (GRUB_MACHINE_EMU) && !defined (GRUB_MACHINE_XEN)
  init_cbfsdisk ();
#endif
  grub_fs_register (&grub_cbfs_fs);
}

GRUB_MOD_FINI (cbfs)
{
  grub_fs_unregister (&grub_cbfs_fs);
#if (defined (__i386__) || defined (__x86_64__)) && !defined (GRUB_UTIL) && !defined (GRUB_MACHINE_EMU) && !defined (GRUB_MACHINE_XEN)
  fini_cbfsdisk ();
#endif
}
