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

#include <grub/misc.h>
#include <grub/disk.h>
#include <grub/archelp.h>

#include <grub/file.h>
#include <grub/mm.h>
#include <grub/dl.h>
#include <grub/i18n.h>

GRUB_MOD_LICENSE ("GPLv3+");

/* tar support */
#define MAGIC	"ustar"
struct head
{
  char name[100];
  char mode[8];
  char uid[8];
  char gid[8];
  char size[12];
  char mtime[12];
  char chksum[8];
  char typeflag;
  char linkname[100];
  char magic[6];
  char version[2];
  char uname[32];
  char gname[32];
  char devmajor[8];
  char devminor[8];
  char prefix[155];
} GRUB_PACKED;

static inline unsigned long long
read_number (const char *str, grub_size_t size)
{
  unsigned long long ret = 0;
  while (size-- && *str >= '0' && *str <= '7')
    ret = (ret << 3) | (*str++ & 0xf);
  return ret;
}

struct grub_archelp_data
{
  grub_disk_t disk;
  grub_off_t hofs, next_hofs;
  grub_off_t dofs;
  grub_off_t size;
  char *linkname;
  grub_size_t linkname_alloc;
};

static grub_err_t
grub_cpio_find_file (struct grub_archelp_data *data, char **name,
		     grub_int32_t *mtime,
		     grub_uint32_t *mode)
{
  struct head hd;
  int reread = 0, have_longname = 0, have_longlink = 0;

  data->hofs = data->next_hofs;

  for (reread = 0; reread < 3; reread++)
    {
      if (grub_disk_read (data->disk, 0, data->hofs, sizeof (hd), &hd))
	return grub_errno;

      if (!hd.name[0] && !hd.prefix[0])
	{
	  *mode = GRUB_ARCHELP_ATTR_END;
	  return GRUB_ERR_NONE;
	}

      if (grub_memcmp (hd.magic, MAGIC, sizeof (MAGIC) - 1))
	return grub_error (GRUB_ERR_BAD_FS, "invalid tar archive");

      if (hd.typeflag == 'L')
	{
	  grub_err_t err;
	  grub_size_t namesize = read_number (hd.size, sizeof (hd.size));
	  *name = grub_malloc (namesize + 1);
	  if (*name == NULL)
	    return grub_errno;
	  err = grub_disk_read (data->disk, 0,
				data->hofs + GRUB_DISK_SECTOR_SIZE, namesize,
				*name);
	  (*name)[namesize] = 0;
	  if (err)
	    return err;
	  data->hofs += GRUB_DISK_SECTOR_SIZE
	    + ((namesize + GRUB_DISK_SECTOR_SIZE - 1) &
	       ~(GRUB_DISK_SECTOR_SIZE - 1));
	  have_longname = 1;
	  continue;
	}

      if (hd.typeflag == 'K')
	{
	  grub_err_t err;
	  grub_size_t linksize = read_number (hd.size, sizeof (hd.size));
	  if (data->linkname_alloc < linksize + 1)
	    {
	      char *n;
	      n = grub_malloc (2 * (linksize + 1));
	      if (!n)
		return grub_errno;
	      grub_free (data->linkname);
	      data->linkname = n;
	      data->linkname_alloc = 2 * (linksize + 1);
	    }

	  err = grub_disk_read (data->disk, 0,
				data->hofs + GRUB_DISK_SECTOR_SIZE, linksize,
				data->linkname);
	  if (err)
	    return err;
	  data->linkname[linksize] = 0;
	  data->hofs += GRUB_DISK_SECTOR_SIZE
	    + ((linksize + GRUB_DISK_SECTOR_SIZE - 1) &
	       ~(GRUB_DISK_SECTOR_SIZE - 1));
	  have_longlink = 1;
	  continue;
	}

      if (!have_longname)
	{
	  grub_size_t extra_size = 0;

	  while (extra_size < sizeof (hd.prefix)
		 && hd.prefix[extra_size])
	    extra_size++;
	  *name = grub_malloc (sizeof (hd.name) + extra_size + 2);
	  if (*name == NULL)
	    return grub_errno;
	  if (hd.prefix[0])
	    {
	      grub_memcpy (*name, hd.prefix, extra_size);
	      (*name)[extra_size++] = '/';
	    }
	  grub_memcpy (*name + extra_size, hd.name, sizeof (hd.name));
	  (*name)[extra_size + sizeof (hd.name)] = 0;
	}

      data->size = read_number (hd.size, sizeof (hd.size));
      data->dofs = data->hofs + GRUB_DISK_SECTOR_SIZE;
      data->next_hofs = data->dofs + ((data->size + GRUB_DISK_SECTOR_SIZE - 1) &
			   ~(GRUB_DISK_SECTOR_SIZE - 1));
      if (mtime)
	*mtime = read_number (hd.mtime, sizeof (hd.mtime));
      if (mode)
	{
	  *mode = read_number (hd.mode, sizeof (hd.mode));
	  switch (hd.typeflag)
	    {
	      /* Hardlink.  */
	    case '1':
	      /* Symlink.  */
	    case '2':
	      *mode |= GRUB_ARCHELP_ATTR_LNK;
	      break;
	    case '0':
	      *mode |= GRUB_ARCHELP_ATTR_FILE;
	      break;
	    case '5':
	      *mode |= GRUB_ARCHELP_ATTR_DIR;
	      break;
	    }
	}
      if (!have_longlink)
	{
	  if (data->linkname_alloc < 101)
	    {
	      char *n;
	      n = grub_malloc (101);
	      if (!n)
		return grub_errno;
	      grub_free (data->linkname);
	      data->linkname = n;
	      data->linkname_alloc = 101;
	    }
	  grub_memcpy (data->linkname, hd.linkname, sizeof (hd.linkname));
	  data->linkname[100] = 0;
	}
      return GRUB_ERR_NONE;
    }
  return GRUB_ERR_NONE;
}

static char *
grub_cpio_get_link_target (struct grub_archelp_data *data)
{
  return grub_strdup (data->linkname);
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

  if (grub_memcmp (hd.magic, MAGIC, sizeof (MAGIC) - 1))
    goto fail;

  data = (struct grub_archelp_data *) grub_zalloc (sizeof (*data));
  if (!data)
    goto fail;

  data->disk = disk;

  return data;

fail:
  grub_error (GRUB_ERR_BAD_FS, "not a tarfs filesystem");
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

  grub_free (data->linkname);
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
      grub_free (data->linkname);
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
  grub_free (data->linkname);
  grub_free (data);

  return grub_errno;
}

static struct grub_fs grub_cpio_fs = {
  .name = "tarfs",
  .dir = grub_cpio_dir,
  .open = grub_cpio_open,
  .read = grub_cpio_read,
  .close = grub_cpio_close,
#ifdef GRUB_UTIL
  .reserved_first_sector = 0,
  .blocklist_install = 0,
#endif
};

GRUB_MOD_INIT (tar)
{
  grub_fs_register (&grub_cpio_fs);
}

GRUB_MOD_FINI (tar)
{
  grub_fs_unregister (&grub_cpio_fs);
}
