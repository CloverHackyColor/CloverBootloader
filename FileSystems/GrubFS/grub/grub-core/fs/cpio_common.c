/* cpio.c - cpio and tar filesystem.  */
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

#include <grub/file.h>
#include <grub/mm.h>
#include <grub/misc.h>
#include <grub/disk.h>
#include <grub/dl.h>
#include <grub/i18n.h>
#include <grub/archelp.h>

GRUB_MOD_LICENSE ("GPLv3+");

struct grub_archelp_data
{
  grub_disk_t disk;
  grub_off_t hofs;
  grub_off_t next_hofs;
  grub_off_t dofs;
  grub_off_t size;
};

static grub_err_t
grub_cpio_find_file (struct grub_archelp_data *data, char **name,
		     grub_int32_t *mtime, grub_uint32_t *mode)
{
  struct head hd;
  grub_size_t namesize;
  grub_uint32_t modeval;

  data->hofs = data->next_hofs;

  if (grub_disk_read (data->disk, 0, data->hofs, sizeof (hd), &hd))
    return grub_errno;

  if (grub_memcmp (hd.magic, MAGIC, sizeof (hd.magic)) != 0
#ifdef MAGIC2
      && grub_memcmp (hd.magic, MAGIC2, sizeof (hd.magic)) != 0
#endif
      )
    return grub_error (GRUB_ERR_BAD_FS, "invalid cpio archive");
  data->size = read_number (hd.filesize, ARRAY_SIZE (hd.filesize));
  if (mtime)
    *mtime = read_number (hd.mtime, ARRAY_SIZE (hd.mtime));
  modeval = read_number (hd.mode, ARRAY_SIZE (hd.mode));
  namesize = read_number (hd.namesize, ARRAY_SIZE (hd.namesize));

  if (mode)
    *mode = modeval;

  *name = grub_malloc (namesize + 1);
  if (*name == NULL)
    return grub_errno;

  if (grub_disk_read (data->disk, 0, data->hofs + sizeof (hd),
		      namesize, *name))
    {
      grub_free (*name);
      return grub_errno;
    }
  (*name)[namesize] = 0;

  if (data->size == 0 && modeval == 0 && namesize == 11
      && grub_memcmp(*name, "TRAILER!!!", 11) == 0)
    {
      *mode = GRUB_ARCHELP_ATTR_END;
      grub_free (*name);
      return GRUB_ERR_NONE;
    }

  data->dofs = data->hofs + ALIGN_CPIO (sizeof (hd) + namesize);
  data->next_hofs = data->dofs + ALIGN_CPIO (data->size);
  return GRUB_ERR_NONE;
}

static char *
grub_cpio_get_link_target (struct grub_archelp_data *data)
{
  char *ret;
  grub_err_t err;

  if (data->size == 0)
    return grub_strdup ("");
  ret = grub_malloc (data->size + 1);
  if (!ret)
    return NULL;

  err = grub_disk_read (data->disk, 0, data->dofs, data->size, 
			ret);
  if (err)
    {
      grub_free (ret);
      return NULL;
    }
  ret[data->size] = '\0';
  return ret;
}

static void
grub_cpio_rewind (struct grub_archelp_data *data)
{
  data->next_hofs = 0;
}

static struct grub_archelp_ops arcops =
  {
    .find_file = grub_cpio_find_file,
    .get_link_target = grub_cpio_get_link_target,
    .rewind = grub_cpio_rewind
  };

static struct grub_archelp_data *
grub_cpio_mount (grub_disk_t disk)
{
  struct head hd;
  struct grub_archelp_data *data;

  if (grub_disk_read (disk, 0, 0, sizeof (hd), &hd))
    goto fail;

  if (grub_memcmp (hd.magic, MAGIC, sizeof (MAGIC) - 1)
#ifdef MAGIC2
      && grub_memcmp (hd.magic, MAGIC2, sizeof (MAGIC2) - 1)
#endif
      )
    goto fail;

  data = (struct grub_archelp_data *) grub_zalloc (sizeof (*data));
  if (!data)
    goto fail;

  data->disk = disk;

  return data;

fail:
  grub_error (GRUB_ERR_BAD_FS, "not a " FSNAME " filesystem");
  return 0;
}

static grub_err_t
grub_cpio_dir (grub_device_t device, const char *path_in,
	       grub_fs_dir_hook_t hook, void *hook_data)
{
  struct grub_archelp_data *data;
  grub_err_t err;

  data = grub_cpio_mount (device->disk);
  if (!data)
    return grub_errno;

  err = grub_archelp_dir (data, &arcops,
			  path_in, hook, hook_data);

  grub_free (data);

  return err;
}

static grub_err_t
grub_cpio_open (grub_file_t file, const char *name_in)
{
  struct grub_archelp_data *data;
  grub_err_t err;

  data = grub_cpio_mount (file->device->disk);
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
grub_cpio_read (grub_file_t file, char *buf, grub_size_t len)
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
grub_cpio_close (grub_file_t file)
{
  struct grub_archelp_data *data;

  data = file->data;
  grub_free (data);

  return grub_errno;
}

static struct grub_fs grub_cpio_fs = {
  .name = FSNAME,
  .dir = grub_cpio_dir,
  .open = grub_cpio_open,
  .read = grub_cpio_read,
  .close = grub_cpio_close,
#ifdef GRUB_UTIL
  .reserved_first_sector = 0,
  .blocklist_install = 0,
#endif
};
