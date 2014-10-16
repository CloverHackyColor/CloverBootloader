/* lzopio.c - decompression support for lzop */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2011  Free Software Foundation, Inc.
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
#include <grub/file.h>
#include <grub/fs.h>
#include <grub/dl.h>
#include <grub/crypto.h>
#include <minilzo.h>

GRUB_MOD_LICENSE ("GPLv3+");

#define LZOP_MAGIC "\x89\x4c\x5a\x4f\x00\x0d\x0a\x1a\x0a"
#define LZOP_MAGIC_SIZE 9
#define LZOP_CHECK_SIZE 4
#define LZOP_NEW_LIB 0x0940

/* Header flags - copied from conf.h of LZOP source code.  */
#define F_ADLER32_D	0x00000001L
#define F_ADLER32_C	0x00000002L
#define F_STDIN		0x00000004L
#define F_STDOUT	0x00000008L
#define F_NAME_DEFAULT	0x00000010L
#define F_DOSISH	0x00000020L
#define F_H_EXTRA_FIELD	0x00000040L
#define F_H_GMTDIFF	0x00000080L
#define F_CRC32_D	0x00000100L
#define F_CRC32_C	0x00000200L
#define F_MULTIPART	0x00000400L
#define F_H_FILTER	0x00000800L
#define F_H_CRC32	0x00001000L
#define F_H_PATH	0x00002000L
#define F_MASK		0x00003FFFL

struct block_header
{
  grub_uint32_t usize;
  grub_uint32_t csize;
  grub_uint32_t ucheck;
  grub_uint32_t ccheck;
  unsigned char *cdata;
  unsigned char *udata;
};

struct grub_lzopio
{
  grub_file_t file;
  int has_ccheck;
  int has_ucheck;
  const gcry_md_spec_t *ucheck_fun;
  const gcry_md_spec_t *ccheck_fun;
  grub_off_t saved_off;		/* Rounded down to block boundary.  */
  grub_off_t start_block_off;
  struct block_header block;
};

typedef struct grub_lzopio *grub_lzopio_t;
static struct grub_fs grub_lzopio_fs;

/* Some helper functions. On errors memory allocated by those function is free
 * either on close() so no risk of leaks. This makes functions simpler.  */

/* Read block header from file, after successful exit file points to
 * beginning of block data.  */
static int
read_block_header (struct grub_lzopio *lzopio)
{
  lzopio->saved_off += lzopio->block.usize;

  /* Free cached block data if any.  */
  grub_free (lzopio->block.udata);
  grub_free (lzopio->block.cdata);
  lzopio->block.udata = NULL;
  lzopio->block.cdata = NULL;

  if (grub_file_read (lzopio->file, &lzopio->block.usize,
		      sizeof (lzopio->block.usize)) !=
      sizeof (lzopio->block.usize))
    return -1;

  lzopio->block.usize = grub_be_to_cpu32 (lzopio->block.usize);

  /* Last block has uncompressed data size == 0 and no other fields.  */
  if (lzopio->block.usize == 0)
    {
      if (grub_file_tell (lzopio->file) == grub_file_size (lzopio->file))
	return 0;
      else
	return -1;
    }

  /* Read compressed data block size.  */
  if (grub_file_read (lzopio->file, &lzopio->block.csize,
		      sizeof (lzopio->block.csize)) !=
      sizeof (lzopio->block.csize))
    return -1;

  lzopio->block.csize = grub_be_to_cpu32 (lzopio->block.csize);

  /* Corrupted.  */
  if (lzopio->block.csize > lzopio->block.usize)
    return -1;

  /* Read checksum of uncompressed data.  */
  if (lzopio->has_ucheck)
    {
      if (grub_file_read (lzopio->file, &lzopio->block.ucheck,
			  sizeof (lzopio->block.ucheck)) !=
	  sizeof (lzopio->block.ucheck))
	return -1;

      lzopio->block.ucheck = lzopio->block.ucheck;
    }

  /* Read checksum of compressed data.  */
  if (lzopio->has_ccheck)
    {
      /* Incompressible data block.  */
      if (lzopio->block.csize == lzopio->block.usize)
	{
	  lzopio->block.ccheck = lzopio->block.ucheck;
	}
      else
	{
	  if (grub_file_read (lzopio->file, &lzopio->block.ccheck,
			      sizeof (lzopio->block.ccheck)) !=
	      sizeof (lzopio->block.ccheck))
	    return -1;

	  lzopio->block.ccheck = lzopio->block.ccheck;
	}
    }

  return 0;
}

/* Read block data into memory. File must be set to beginning of block data.
 * Can't be called on last block.  */
static int
read_block_data (struct grub_lzopio *lzopio)
{
  lzopio->block.cdata = grub_malloc (lzopio->block.csize);
  if (!lzopio->block.cdata)
    return -1;

  if (grub_file_read (lzopio->file, lzopio->block.cdata, lzopio->block.csize)
      != (grub_ssize_t) lzopio->block.csize)
    return -1;

  if (lzopio->ccheck_fun)
    {
      grub_uint8_t computed_hash[GRUB_CRYPTO_MAX_MDLEN];

      if (lzopio->ccheck_fun->mdlen > GRUB_CRYPTO_MAX_MDLEN)
	return -1;

      grub_crypto_hash (lzopio->ccheck_fun, computed_hash,
			lzopio->block.cdata,
			lzopio->block.csize);

      if (grub_memcmp
	  (computed_hash, &lzopio->block.ccheck,
	   sizeof (lzopio->block.ccheck)) != 0)
	return -1;
    }

  return 0;
}

/* Read block data, uncompressed and also store it in memory.  */
/* XXX Investigate possibility of in-place decompression to reduce memory
 * footprint. Or try to uncompress directly to buf if possible.  */
static int
uncompress_block (struct grub_lzopio *lzopio)
{
  lzo_uint usize = lzopio->block.usize;

  if (read_block_data (lzopio) < 0)
    return -1;

  /* Incompressible data. */
  if (lzopio->block.csize == lzopio->block.usize)
    {
      lzopio->block.udata = lzopio->block.cdata;
      lzopio->block.cdata = NULL;
    }
  else
    {
      lzopio->block.udata = grub_malloc (lzopio->block.usize);
      if (!lzopio->block.udata)
	return -1;

      if (lzo1x_decompress_safe (lzopio->block.cdata, lzopio->block.csize,
				 lzopio->block.udata, &usize, NULL)
	  != LZO_E_OK)
	return -1;

      if (lzopio->ucheck_fun)
	{
	  grub_uint8_t computed_hash[GRUB_CRYPTO_MAX_MDLEN];

	  if (lzopio->ucheck_fun->mdlen > GRUB_CRYPTO_MAX_MDLEN)
	    return -1;

	  grub_crypto_hash (lzopio->ucheck_fun, computed_hash,
			    lzopio->block.udata,
			    lzopio->block.usize);

	  if (grub_memcmp
	      (computed_hash, &lzopio->block.ucheck,
	       sizeof (lzopio->block.ucheck)) != 0)
	    return -1;
	}

      /* Compressed data can be free now.  */
      grub_free (lzopio->block.cdata);
      lzopio->block.cdata = NULL;
    }

  return 0;
}

/* Jump to next block and read its header.  */
static int
jump_block (struct grub_lzopio *lzopio)
{
  /* only jump if block was not decompressed (and read from disk) */
  if (!lzopio->block.udata)
    {
      grub_off_t off = grub_file_tell (lzopio->file) + lzopio->block.csize;

      if (grub_file_seek (lzopio->file, off) == ((grub_off_t) - 1))
	return -1;
    }

  return read_block_header (lzopio);
}

static int
calculate_uncompressed_size (grub_file_t file)
{
  grub_lzopio_t lzopio = file->data;
  grub_off_t usize_total = 0;

  if (read_block_header (lzopio) < 0)
    return -1;

  /* FIXME: Don't do this for not easily seekable files.  */
  while (lzopio->block.usize != 0)
    {
      usize_total += lzopio->block.usize;

      if (jump_block (lzopio) < 0)
	return -1;
    }

  file->size = usize_total;

  return 0;
}

struct lzop_header
{
  grub_uint8_t magic[LZOP_MAGIC_SIZE];
  grub_uint16_t lzop_version;
  grub_uint16_t lib_version;
  grub_uint16_t lib_version_ext;
  grub_uint8_t method;
  grub_uint8_t level;
  grub_uint32_t flags;
  /* grub_uint32_t filter; */ /* No filters support. Rarely used anyway.  */
  grub_uint32_t mode;
  grub_uint32_t mtime_lo;
  grub_uint32_t mtime_hi;
  grub_uint8_t name_len;
} GRUB_PACKED;

static int
test_header (grub_file_t file)
{
  grub_lzopio_t lzopio = file->data;
  struct lzop_header header;
  grub_uint32_t flags, checksum;
  const gcry_md_spec_t *hcheck;
  grub_uint8_t *context = NULL;
  grub_uint8_t *name = NULL;

  if (grub_file_read (lzopio->file, &header, sizeof (header)) != sizeof (header))
    return 0;

  if (grub_memcmp (header.magic, LZOP_MAGIC, LZOP_MAGIC_SIZE) != 0)
    return 0;

  if (grub_be_to_cpu16(header.lib_version) < LZOP_NEW_LIB)
    return 0;

  /* Too new version, should upgrade minilzo?  */
  if (grub_be_to_cpu16 (header.lib_version_ext) > MINILZO_VERSION)
    return 0;

  flags = grub_be_to_cpu32 (header.flags);

  if (flags & F_CRC32_D)
    {
      lzopio->has_ucheck = 1;
      lzopio->ucheck_fun = grub_crypto_lookup_md_by_name ("crc32");
    }
  else if (flags & F_ADLER32_D)
    {
      lzopio->has_ucheck = 1;
      lzopio->ucheck_fun = grub_crypto_lookup_md_by_name ("adler32");
    }

  if (flags & F_CRC32_C)
    {
      lzopio->has_ccheck = 1;
      lzopio->ccheck_fun = grub_crypto_lookup_md_by_name ("crc32");
    }
  else if (flags & F_ADLER32_C)
    {
      lzopio->has_ccheck = 1;
      lzopio->ccheck_fun = grub_crypto_lookup_md_by_name ("adler32");
    }

  if (flags & F_H_CRC32)
    hcheck = grub_crypto_lookup_md_by_name ("crc32");
  else
    hcheck = grub_crypto_lookup_md_by_name ("adler32");

  if (hcheck) {
    context = grub_malloc(hcheck->contextsize);
    if (! context)
      return 0;

    hcheck->init(context);

    /* MAGIC is not included in check calculation.  */
    hcheck->write(context, &header.lzop_version, sizeof(header)- LZOP_MAGIC_SIZE);
  }

  if (header.name_len != 0)
    {
      name = grub_malloc (header.name_len);
      if (! name)
	{
	  grub_free (context);
	  return 0;
	}

      if (grub_file_read (lzopio->file, name, header.name_len) !=
	  header.name_len)
	{
	  grub_free(name);
	  goto CORRUPTED;
	}

      if (hcheck)
	hcheck->write(context, name, header.name_len);

      grub_free(name);
    }

  if (hcheck)
    hcheck->final(context);

  if (grub_file_read (lzopio->file, &checksum, sizeof (checksum)) !=
      sizeof (checksum))
    goto CORRUPTED;

  if (hcheck && grub_memcmp (&checksum, hcheck->read(context), sizeof(checksum)) != 0)
    goto CORRUPTED;

  lzopio->start_block_off = grub_file_tell (lzopio->file);

  if (calculate_uncompressed_size (file) < 0)
    goto CORRUPTED;

  /* Get back to start block.  */
  grub_file_seek (lzopio->file, lzopio->start_block_off);

  /* Read first block - grub_lzopio_read() expects valid block.  */
  if (read_block_header (lzopio) < 0)
    goto CORRUPTED;

  lzopio->saved_off = 0;
  return 1;

CORRUPTED:
  grub_free(name);

  return 0;
}

static grub_file_t
grub_lzopio_open (grub_file_t io,
		  const char *name __attribute__ ((unused)))
{
  grub_file_t file;
  grub_lzopio_t lzopio;

  file = (grub_file_t) grub_zalloc (sizeof (*file));
  if (!file)
    return 0;

  lzopio = grub_zalloc (sizeof (*lzopio));
  if (!lzopio)
    {
      grub_free (file);
      return 0;
    }

  lzopio->file = io;

  file->device = io->device;
  file->data = lzopio;
  file->fs = &grub_lzopio_fs;
  file->size = GRUB_FILE_SIZE_UNKNOWN;
  file->not_easily_seekable = 1;

  if (grub_file_tell (lzopio->file) != 0)
    grub_file_seek (lzopio->file, 0);

  if (!test_header (file))
    {
      grub_errno = GRUB_ERR_NONE;
      grub_file_seek (io, 0);
      grub_free (lzopio);
      grub_free (file);

      return io;
    }

  return file;
}

static grub_ssize_t
grub_lzopio_read (grub_file_t file, char *buf, grub_size_t len)
{
  grub_lzopio_t lzopio = file->data;
  grub_ssize_t ret = 0;
  grub_off_t off;

  /* Backward seek before last read block.  */
  if (lzopio->saved_off > grub_file_tell (file))
    {
      grub_file_seek (lzopio->file, lzopio->start_block_off);

      if (read_block_header (lzopio) < 0)
	goto CORRUPTED;

      lzopio->saved_off = 0;
    }

  /* Forward to first block with requested data.  */
  while (lzopio->saved_off + lzopio->block.usize <= grub_file_tell (file))
    {
      /* EOF, could be possible files with unknown size.  */
      if (lzopio->block.usize == 0)
	return 0;

      if (jump_block (lzopio) < 0)
	goto CORRUPTED;
    }

  off = grub_file_tell (file) - lzopio->saved_off;

  while (len != 0 && lzopio->block.usize != 0)
    {
      grub_size_t to_copy;

      /* Block not decompressed yet.  */
      if (!lzopio->block.udata && uncompress_block (lzopio) < 0)
	goto CORRUPTED;

      /* Copy requested data into buffer.  */
      to_copy = lzopio->block.usize - off;
      if (to_copy > len)
	to_copy = len;
      grub_memcpy (buf, lzopio->block.udata + off, to_copy);

      len -= to_copy;
      buf += to_copy;
      ret += to_copy;
      off = 0;

      /* Read next block if needed.  */
      if (len > 0 && read_block_header (lzopio) < 0)
	goto CORRUPTED;
    }

  return ret;

CORRUPTED:
  grub_error (GRUB_ERR_BAD_COMPRESSED_DATA, N_("lzop file corrupted"));
  return -1;
}

/* Release everything, including the underlying file object.  */
static grub_err_t
grub_lzopio_close (grub_file_t file)
{
  grub_lzopio_t lzopio = file->data;

  grub_file_close (lzopio->file);
  grub_free (lzopio->block.cdata);
  grub_free (lzopio->block.udata);
  grub_free (lzopio);

  /* Device must not be closed twice.  */
  file->device = 0;
  file->name = 0;
  return grub_errno;
}

static struct grub_fs grub_lzopio_fs = {
  .name = "lzopio",
  .dir = 0,
  .open = 0,
  .read = grub_lzopio_read,
  .close = grub_lzopio_close,
  .label = 0,
  .next = 0
};

GRUB_MOD_INIT (lzopio)
{
  grub_file_filter_register (GRUB_FILE_FILTER_LZOPIO, grub_lzopio_open);
}

GRUB_MOD_FINI (lzopio)
{
  grub_file_filter_unregister (GRUB_FILE_FILTER_LZOPIO);
}
