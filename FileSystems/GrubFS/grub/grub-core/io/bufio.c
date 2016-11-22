/* bufio.c - buffered io access */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2008  Free Software Foundation, Inc.
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

#include <grub/err.h>
#include <grub/types.h>
#include <grub/mm.h>
#include <grub/misc.h>
#include <grub/fs.h>
#include <grub/bufio.h>
#include <grub/dl.h>

GRUB_MOD_LICENSE ("GPLv3+");

#define GRUB_BUFIO_DEF_SIZE	8192
#define GRUB_BUFIO_MAX_SIZE	1048576

struct grub_bufio
{
  grub_file_t file;
  grub_size_t block_size;
  grub_size_t buffer_len;
  grub_off_t buffer_at;
  char buffer[0];
};
typedef struct grub_bufio *grub_bufio_t;

static struct grub_fs grub_bufio_fs;

grub_file_t
grub_bufio_open (grub_file_t io, int size)
{
  grub_file_t file;
  grub_bufio_t bufio = 0;

  file = (grub_file_t) grub_zalloc (sizeof (*file));
  if (! file)
    return 0;

  if (size == 0)
    size = GRUB_BUFIO_DEF_SIZE;
  else if (size > GRUB_BUFIO_MAX_SIZE)
    size = GRUB_BUFIO_MAX_SIZE;

  if ((size < 0) || ((unsigned) size > io->size))
    size = ((io->size > GRUB_BUFIO_MAX_SIZE) ? GRUB_BUFIO_MAX_SIZE :
            io->size);

  bufio = grub_zalloc (sizeof (struct grub_bufio) + size);
  if (! bufio)
    {
      grub_free (file);
      return 0;
    }

  bufio->file = io;
  bufio->block_size = size;

  file->device = io->device;
  file->size = io->size;
  file->data = bufio;
  file->fs = &grub_bufio_fs;
  file->not_easily_seekable = io->not_easily_seekable;

  return file;
}

grub_file_t
grub_buffile_open (const char *name, int size)
{
  grub_file_t io, file;

  io = grub_file_open (name);
  if (! io)
    return 0;

  file = grub_bufio_open (io, size);
  if (! file)
    {
      grub_file_close (io);
      return 0;
    }

  return file;
}

static grub_ssize_t
grub_bufio_read (grub_file_t file, char *buf, grub_size_t len)
{
  grub_size_t res = 0;
  grub_off_t next_buf;
  grub_bufio_t bufio = file->data;
  grub_ssize_t really_read;

  if (file->size == GRUB_FILE_SIZE_UNKNOWN)
    file->size = bufio->file->size;

  /* First part: use whatever we already have in the buffer.  */
  if ((file->offset >= bufio->buffer_at) &&
      (file->offset < bufio->buffer_at + bufio->buffer_len))
    {
      grub_size_t n;
      grub_uint64_t pos;

      pos = file->offset - bufio->buffer_at;
      n = bufio->buffer_len - pos;
      if (n > len)
        n = len;

      grub_memcpy (buf, &bufio->buffer[pos], n);
      len -= n;
      res += n;

      buf += n;
    }
  if (len == 0)
    return res;

  /* Need to read some more.  */
  next_buf = (file->offset + res + len - 1) & ~((grub_off_t) bufio->block_size - 1);
  /* Now read between file->offset + res and bufio->buffer_at.  */
  if (file->offset + res < next_buf)
    {
      grub_size_t read_now;
      read_now = next_buf - (file->offset + res);
      grub_file_seek (bufio->file, file->offset + res);
      really_read = grub_file_read (bufio->file, buf, read_now);
      if (really_read < 0)
	return -1;
      if (file->size == GRUB_FILE_SIZE_UNKNOWN)
	file->size = bufio->file->size;
      len -= really_read;
      buf += really_read;
      res += really_read;

      /* Partial read. File ended unexpectedly. Save the last chunk in buffer.
       */
      if (really_read != (grub_ssize_t) read_now)
	{
	  bufio->buffer_len = really_read;
	  if (bufio->buffer_len > bufio->block_size)
	    bufio->buffer_len = bufio->block_size;
	  bufio->buffer_at = file->offset + res - bufio->buffer_len;
	  grub_memcpy (&bufio->buffer[0], buf - bufio->buffer_len,
		       bufio->buffer_len);
	  return res;
	}
    }

  /* Read into buffer.  */
  grub_file_seek (bufio->file, next_buf);
  really_read = grub_file_read (bufio->file, bufio->buffer,
				bufio->block_size);
  if (really_read < 0)
    return -1;
  bufio->buffer_at = next_buf;
  bufio->buffer_len = really_read;

  if (file->size == GRUB_FILE_SIZE_UNKNOWN)
    file->size = bufio->file->size;

  if (len > bufio->buffer_len)
    len = bufio->buffer_len;
  grub_memcpy (buf, &bufio->buffer[file->offset + res - next_buf], len);
  res += len;

  return res;
}

static grub_err_t
grub_bufio_close (grub_file_t file)
{
  grub_bufio_t bufio = file->data;

  grub_file_close (bufio->file);
  grub_free (bufio);

  file->device = 0;

  return grub_errno;
}

static struct grub_fs grub_bufio_fs =
  {
    .name = "bufio",
    .dir = 0,
    .open = 0,
    .read = grub_bufio_read,
    .close = grub_bufio_close,
    .label = 0,
    .next = 0
  };
