/* xzio.c - decompression support for xz */
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

#include <grub/err.h>
#include <grub/mm.h>
#include <grub/misc.h>
#include <grub/file.h>
#include <grub/fs.h>
#include <grub/dl.h>

GRUB_MOD_LICENSE ("GPLv3+");

#include "xz_config.h"
#include "xz.h"
#include "xz_stream.h"

#define XZBUFSIZ 0x2000
#define VLI_MAX_DIGITS 9
#define XZ_STREAM_FOOTER_SIZE 12

struct grub_xzio
{
  grub_file_t file;
  struct xz_buf buf;
  struct xz_dec *dec;
  grub_uint8_t inbuf[XZBUFSIZ];
  grub_uint8_t outbuf[XZBUFSIZ];
  grub_off_t saved_offset;
};

typedef struct grub_xzio *grub_xzio_t;
static struct grub_fs grub_xzio_fs;

static grub_size_t
decode_vli (const grub_uint8_t buf[], grub_size_t size_max,
	    grub_uint64_t *num)
{
  if (size_max == 0)
    return 0;

  if (size_max > VLI_MAX_DIGITS)
    size_max = VLI_MAX_DIGITS;

  *num = buf[0] & 0x7F;
  grub_size_t i = 0;

  while (buf[i++] & 0x80)
    {
      if (i >= size_max || buf[i] == 0x00)
	return 0;

      *num |= (uint64_t) (buf[i] & 0x7F) << (i * 7);
    }

  return i;
}

static grub_ssize_t
read_vli (grub_file_t file, grub_uint64_t *num)
{
  grub_uint8_t buf[VLI_MAX_DIGITS];
  grub_ssize_t read_bytes;
  grub_size_t dec;

  read_bytes = grub_file_read (file, buf, VLI_MAX_DIGITS);
  if (read_bytes < 0)
    return -1;

  dec = decode_vli (buf, read_bytes, num);
  grub_file_seek (file, file->offset - (read_bytes - dec));
  return dec;
}

/* Function xz_dec_run() should consume header and ask for more (XZ_OK)
 * else file is corrupted (or options not supported) or not xz.  */
static int
test_header (grub_file_t file)
{
  grub_xzio_t xzio = file->data;
  enum xz_ret ret;

  xzio->buf.in_size = grub_file_read (xzio->file, xzio->inbuf,
				      STREAM_HEADER_SIZE);

  if (xzio->buf.in_size != STREAM_HEADER_SIZE)
    return 0;

  ret = xz_dec_run (xzio->dec, &xzio->buf);

  if (ret == XZ_FORMAT_ERROR)
    return 0;

  if (ret != XZ_OK)
    return 0;

  return 1;
}

/* Try to find out size of uncompressed data,
 * also do some footer sanity checks.  */
static int
test_footer (grub_file_t file)
{
  grub_xzio_t xzio = file->data;
  grub_uint8_t footer[FOOTER_MAGIC_SIZE];
  grub_uint32_t backsize;
  grub_uint8_t imarker;
  grub_uint64_t uncompressed_size_total = 0;
  grub_uint64_t uncompressed_size;
  grub_uint64_t records;

  grub_file_seek (xzio->file, xzio->file->size - FOOTER_MAGIC_SIZE);
  if (grub_file_read (xzio->file, footer, FOOTER_MAGIC_SIZE)
      != FOOTER_MAGIC_SIZE
      || grub_memcmp (footer, FOOTER_MAGIC, FOOTER_MAGIC_SIZE) != 0)
    goto ERROR;

  grub_file_seek (xzio->file, xzio->file->size - 8);
  if (grub_file_read (xzio->file, &backsize, sizeof (backsize))
      != sizeof (backsize))
    goto ERROR;

  /* Calculate real backward size.  */
  backsize = (grub_le_to_cpu32 (backsize) + 1) * 4;

  /* Set file to the beginning of stream index.  */
  grub_file_seek (xzio->file,
		  xzio->file->size - XZ_STREAM_FOOTER_SIZE - backsize);

  /* Test index marker.  */
  if (grub_file_read (xzio->file, &imarker, sizeof (imarker))
      != sizeof (imarker) && imarker != 0x00)
    goto ERROR;

  if (read_vli (xzio->file, &records) <= 0)
    goto ERROR;

  for (; records != 0; records--)
    {
      if (read_vli (xzio->file, &uncompressed_size) <= 0)	/* Ignore unpadded.  */
	goto ERROR;
      if (read_vli (xzio->file, &uncompressed_size) <= 0)	/* Uncompressed.  */
	goto ERROR;

      uncompressed_size_total += uncompressed_size;
    }

  file->size = uncompressed_size_total;
  grub_file_seek (xzio->file, STREAM_HEADER_SIZE);
  return 1;

ERROR:
  return 0;
}

static grub_file_t
grub_xzio_open (grub_file_t io,
		const char *name __attribute__ ((unused)))
{
  grub_file_t file;
  grub_xzio_t xzio;

  file = (grub_file_t) grub_zalloc (sizeof (*file));
  if (!file)
    return 0;

  xzio = grub_zalloc (sizeof (*xzio));
  if (!xzio)
    {
      grub_free (file);
      return 0;
    }

  xzio->file = io;

  file->device = io->device;
  file->data = xzio;
  file->fs = &grub_xzio_fs;
  file->size = GRUB_FILE_SIZE_UNKNOWN;
  file->not_easily_seekable = 1;

  if (grub_file_tell (xzio->file) != 0)
    grub_file_seek (xzio->file, 0);

  /* Allocated 64KiB for dictionary.
   * Decoder will relocate if bigger is needed.  */
  xzio->dec = xz_dec_init (1 << 16);
  if (!xzio->dec)
    {
      grub_free (file);
      grub_free (xzio);
      return 0;
    }

  xzio->buf.in = xzio->inbuf;
  xzio->buf.out = xzio->outbuf;
  xzio->buf.out_size = XZBUFSIZ;

  /* FIXME: don't test footer on not easily seekable files.  */
  if (!test_header (file) || !test_footer (file))
    {
      grub_errno = GRUB_ERR_NONE;
      grub_file_seek (io, 0);
      xz_dec_end (xzio->dec);
      grub_free (xzio);
      grub_free (file);

      return io;
    }

  return file;
}

static grub_ssize_t
grub_xzio_read (grub_file_t file, char *buf, grub_size_t len)
{
  grub_ssize_t ret = 0;
  grub_ssize_t readret;
  enum xz_ret xzret;
  grub_xzio_t xzio = file->data;
  grub_off_t current_offset;

  /* If seek backward need to reset decoder and start from beginning of file.
     TODO Possible improvement by jumping blocks.  */
  if (file->offset < xzio->saved_offset)
    {
      xz_dec_reset (xzio->dec);
      xzio->saved_offset = 0;
      xzio->buf.out_pos = 0;
      xzio->buf.in_pos = 0;
      xzio->buf.in_size = 0;
      grub_file_seek (xzio->file, 0);
    }

  current_offset = xzio->saved_offset;

  while (len > 0)
    {
      xzio->buf.out_size = file->offset + ret + len - current_offset;
      if (xzio->buf.out_size > XZBUFSIZ)
	xzio->buf.out_size = XZBUFSIZ;
      /* Feed input.  */
      if (xzio->buf.in_pos == xzio->buf.in_size)
	{
	  readret = grub_file_read (xzio->file, xzio->inbuf, XZBUFSIZ);
	  if (readret < 0)
	    return -1;
	  xzio->buf.in_size = readret;
	  xzio->buf.in_pos = 0;
	}

      xzret = xz_dec_run (xzio->dec, &xzio->buf);
      switch (xzret)
	{
	case XZ_MEMLIMIT_ERROR:
	case XZ_FORMAT_ERROR:
	case XZ_OPTIONS_ERROR:
	case XZ_DATA_ERROR:
	case XZ_BUF_ERROR:
	  grub_error (GRUB_ERR_BAD_COMPRESSED_DATA,
		      N_("xz file corrupted or unsupported block options"));
	  return -1;
	default:
	  break;
	}

      {
	grub_off_t new_offset = current_offset + xzio->buf.out_pos;
	
	if (file->offset <= new_offset)
	  /* Store first chunk of data in buffer.  */
	  {
	    grub_size_t delta = new_offset - (file->offset + ret);
	    grub_memmove (buf, xzio->buf.out + (xzio->buf.out_pos - delta),
			  delta);
	    len -= delta;
	    buf += delta;
	    ret += delta;
	  }
	current_offset = new_offset;
      }
      xzio->buf.out_pos = 0;

      if (xzret == XZ_STREAM_END)	/* Stream end, EOF.  */
	break;
    }

  if (ret >= 0)
    xzio->saved_offset = file->offset + ret;

  return ret;
}

/* Release everything, including the underlying file object.  */
static grub_err_t
grub_xzio_close (grub_file_t file)
{
  grub_xzio_t xzio = file->data;

  xz_dec_end (xzio->dec);

  grub_file_close (xzio->file);
  grub_free (xzio);

  /* Device must not be closed twice.  */
  file->device = 0;
  file->name = 0;
  return grub_errno;
}

static struct grub_fs grub_xzio_fs = {
  .name = "xzio",
  .dir = 0,
  .open = 0,
  .read = grub_xzio_read,
  .close = grub_xzio_close,
  .label = 0,
  .next = 0
};

GRUB_MOD_INIT (xzio)
{
  grub_file_filter_register (GRUB_FILE_FILTER_XZIO, grub_xzio_open);
}

GRUB_MOD_FINI (xzio)
{
  grub_file_filter_unregister (GRUB_FILE_FILTER_XZIO);
}
