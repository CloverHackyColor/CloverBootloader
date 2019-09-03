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

#include <grub/file.h>
#include <grub/dl.h>

GRUB_MOD_LICENSE ("GPLv3+");

struct grub_offset_file
{
  grub_file_t parent;
  grub_off_t off;
};

static grub_ssize_t
grub_offset_read (grub_file_t file, char *buf, grub_size_t len)
{
  struct grub_offset_file *data = file->data;
  if (grub_file_seek (data->parent, data->off + file->offset) == (grub_off_t) -1)
    return -1;
  return grub_file_read (data->parent, buf, len);
}

static grub_err_t
grub_offset_close (grub_file_t file)
{
  struct grub_offset_file *data = file->data;

  if (data->parent)
    grub_file_close (data->parent);

  /* No need to close the same device twice.  */
  file->device = 0;

  return 0;
}

static struct grub_fs grub_offset_fs = {
  .name = "offset",
  .dir = 0,
  .open = 0,
  .read = grub_offset_read,
  .close = grub_offset_close,
  .label = 0,
  .next = 0
};

void
grub_file_offset_close (grub_file_t file)
{
  struct grub_offset_file *off_data = file->data;
  off_data->parent = NULL;
  grub_file_close (file);
}

grub_file_t
grub_file_offset_open (grub_file_t parent, grub_off_t start, grub_off_t size)
{
  struct grub_offset_file *off_data;
  grub_file_t off_file, last_off_file;
  grub_file_filter_id_t filter;

  off_file = grub_zalloc (sizeof (*off_file));
  off_data = grub_zalloc (sizeof (*off_data));
  if (!off_file || !off_data)
    {
      grub_free (off_file);
      grub_free (off_data);
      return 0;
    }

  off_data->off = start;
  off_data->parent = parent;

  off_file->device = parent->device;
  off_file->data = off_data;
  off_file->fs = &grub_offset_fs;
  off_file->size = size;

  last_off_file = NULL;
  for (filter = GRUB_FILE_FILTER_COMPRESSION_FIRST;
       off_file && filter <= GRUB_FILE_FILTER_COMPRESSION_LAST; filter++)
    if (grub_file_filters_enabled[filter])
      {
	last_off_file = off_file;
	off_file = grub_file_filters_enabled[filter] (off_file, parent->name);
      }

  if (!off_file)
    {
      off_data->parent = NULL;
      grub_file_close (last_off_file);
      return 0;
    }
  return off_file;
}
