/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2012  Free Software Foundation, Inc.
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

/* HFS+ is documented at http://developer.apple.com/technotes/tn/tn1150.html */

#include <grub/hfsplus.h>
#include <grub/dl.h>
#include <grub/misc.h>
#include <grub/mm.h>
#include <grub/deflate.h>
#include <grub/file.h>

GRUB_MOD_LICENSE ("GPLv3+");

/* big-endian.  */
struct grub_hfsplus_compress_header1
{
  grub_uint32_t header_size;
  grub_uint32_t end_descriptor_offset;
  grub_uint32_t total_compressed_size_including_seek_blocks_and_header2;
  grub_uint32_t value_0x32;
  grub_uint8_t unused[0xf0];
} GRUB_PACKED;

/* big-endian.  */
struct grub_hfsplus_compress_header2
{
  grub_uint32_t total_compressed_size_including_seek_blocks;
} GRUB_PACKED;

/* little-endian.  */
struct grub_hfsplus_compress_header3
{
  grub_uint32_t num_chunks;
} GRUB_PACKED;

/* little-endian.  */
struct grub_hfsplus_compress_block_descriptor
{
  grub_uint32_t offset;
  grub_uint32_t size;
};

struct grub_hfsplus_compress_end_descriptor
{
  grub_uint8_t always_the_same[50];
} GRUB_PACKED;

struct grub_hfsplus_attr_header
{
  grub_uint8_t unused[3];
  grub_uint8_t type;
  grub_uint32_t unknown[1];
  grub_uint64_t size;
} GRUB_PACKED;

struct grub_hfsplus_compress_attr
{
  grub_uint32_t magic;
  grub_uint32_t type;
  grub_uint32_t uncompressed_inline_size;
  grub_uint32_t always_0;
} GRUB_PACKED;

enum
  {
    HFSPLUS_COMPRESSION_INLINE = 3,
    HFSPLUS_COMPRESSION_RESOURCE = 4
  };

static int
grub_hfsplus_cmp_attrkey (struct grub_hfsplus_key *keya,
			  struct grub_hfsplus_key_internal *keyb)
{
  struct grub_hfsplus_attrkey *attrkey_a = &keya->attrkey;
  struct grub_hfsplus_attrkey_internal *attrkey_b = &keyb->attrkey;
  grub_uint32_t aparent = grub_be_to_cpu32 (attrkey_a->cnid);
  grub_size_t len;
  int diff;

  if (aparent > attrkey_b->cnid)
    return 1;
  if (aparent < attrkey_b->cnid)
    return -1;

  len = grub_be_to_cpu16 (attrkey_a->namelen);
  if (len > attrkey_b->namelen)
    len = attrkey_b->namelen;
  /* Since it's big-endian memcmp gives the same result as manually comparing
     uint16_t but may be faster.  */
  diff = grub_memcmp (attrkey_a->name, attrkey_b->name,
		      len * sizeof (attrkey_a->name[0]));
  if (diff == 0) {
    diff = grub_be_to_cpu16 (attrkey_a->namelen) - attrkey_b->namelen;
  }
  return diff;
}

#define HFSPLUS_COMPRESS_BLOCK_SIZE 65536

static grub_ssize_t
hfsplus_read_compressed_real (struct grub_hfsplus_file *node,
			      grub_off_t pos, grub_size_t len, char *buf)
{
  char *tmp_buf = 0;
  grub_size_t len0 = len;

  if (node->compressed == 1) {
    grub_memcpy (buf, node->cbuf + pos, len);
    if (grub_file_progress_hook && node->file) {
      grub_file_progress_hook (0, 0, len, node->file);
    }
    return len;
  }

  while (len) {
    grub_uint32_t block = pos / HFSPLUS_COMPRESS_BLOCK_SIZE;
    grub_size_t curlen = HFSPLUS_COMPRESS_BLOCK_SIZE
    - (pos % HFSPLUS_COMPRESS_BLOCK_SIZE);

    if (curlen > len)
      curlen = len;

    if (node->cbuf_block != block) {
      grub_uint32_t sz = grub_le_to_cpu32 (node->compress_index[block].size);
      grub_size_t ts;
      if (!tmp_buf) {
        tmp_buf = grub_malloc (HFSPLUS_COMPRESS_BLOCK_SIZE);
      }
      if (!tmp_buf) {
        return -1;
      }
      if (grub_hfsplus_read_file (node, 0, 0,
                                  grub_le_to_cpu32 (node->compress_index[block].start) + 0x104,
                                  sz, tmp_buf)
          != (grub_ssize_t) sz) {
        grub_free (tmp_buf);
        return -1;
      }
      ts = HFSPLUS_COMPRESS_BLOCK_SIZE;
      if (ts > node->size - (pos & ~(HFSPLUS_COMPRESS_BLOCK_SIZE))) {
        ts = node->size - (pos & ~(HFSPLUS_COMPRESS_BLOCK_SIZE));
      }
      if (grub_zlib_decompress (tmp_buf, sz, 0,
                                node->cbuf, ts) != (grub_ssize_t) ts) {
        if (!grub_errno) {
          grub_error (GRUB_ERR_BAD_COMPRESSED_DATA,
                      "premature end of compressed");
        }

        grub_free (tmp_buf);
        return -1;
      }
      node->cbuf_block = block;
    }
    grub_memcpy (buf, node->cbuf + (pos % HFSPLUS_COMPRESS_BLOCK_SIZE),
                 curlen);
    if (grub_file_progress_hook && node->file) {
      grub_file_progress_hook (0, 0, curlen, node->file);
    }
    buf += curlen;
    pos += curlen;
    len -= curlen;
  }
  grub_free (tmp_buf);
  return len0;
}

grub_ssize_t
grub_hfsplus_read_compressed(struct grub_hfsplus_file *node,
                             grub_off_t pos, grub_size_t len, char *buf)
{
  return hfsplus_read_compressed_real(node, pos, len, buf);
}

static grub_err_t 
hfsplus_open_compressed_real (struct grub_hfsplus_file *node)
{
  grub_err_t err;
  struct grub_hfsplus_btnode *attr_node;
  grub_off_t attr_off;
  struct grub_hfsplus_key_internal key;
  struct grub_hfsplus_attr_header *attr_head;
  struct grub_hfsplus_compress_attr *cmp_head;
#define c grub_cpu_to_be16_compile_time
  const grub_uint16_t compress_attr_name[] =
  {
    c('c'), c('o'), c('m'), c('.'), c('a'), c('p'), c('p'), c('l'), c('e'),
    c('.'), c('d'), c('e'), c('c'), c('m'), c('p'), c('f'), c('s') };
#undef c
  if (node->size)
    return 0;

  key.attrkey.cnid = node->fileid;
  key.attrkey.namelen = sizeof (compress_attr_name) / sizeof (compress_attr_name[0]);
  key.attrkey.name = compress_attr_name;

  err = grub_hfsplus_btree_search (&node->data->attr_tree, &key,
                                   grub_hfsplus_cmp_attrkey,
                                   &attr_node, &attr_off);
  if (err || !attr_node) {
    grub_errno = 0;
    return 0;
  }

  attr_head = (struct grub_hfsplus_attr_header *)
  ((char *) grub_hfsplus_btree_recptr (&node->data->attr_tree,
                                       attr_node, attr_off)
   + sizeof (struct grub_hfsplus_attrkey) + sizeof (compress_attr_name));
  if (attr_head->type != 0x10
      || !(attr_head->size & grub_cpu_to_be64_compile_time(~0xfULL))) {
    grub_free (attr_node);
    return 0;
  }
  cmp_head = (struct grub_hfsplus_compress_attr *) (attr_head + 1);
  if (cmp_head->magic != grub_cpu_to_be32_compile_time (0x66706d63)) {
    grub_free (attr_node);
    return 0;
  }
  node->size = grub_le_to_cpu32 (cmp_head->uncompressed_inline_size);

  if (cmp_head->type == grub_cpu_to_le32_compile_time (HFSPLUS_COMPRESSION_RESOURCE)) {
    grub_uint32_t index_size;
    node->compressed = 2;

    if (grub_hfsplus_read_file (node, 0, 0,
                                0x104, sizeof (index_size),
                                (char *) &index_size)
        != 4) {
      node->compressed = 0;
      grub_free (attr_node);
      grub_errno = 0;
      return 0;
    }
    node->compress_index_size = grub_le_to_cpu32 (index_size);
    node->compress_index = grub_malloc (node->compress_index_size
                                        * sizeof (node->compress_index[0]));
    if (!node->compress_index) {
      node->compressed = 0;
      grub_free (attr_node);
      return grub_errno;
    }
    if (grub_hfsplus_read_file (node, 0, 0,
                                0x104 + sizeof (index_size),
                                node->compress_index_size
                                * sizeof (node->compress_index[0]),
                                (char *) node->compress_index) !=
        (grub_ssize_t) (node->compress_index_size
                           * sizeof (node->compress_index[0]))) {
          node->compressed = 0;
          grub_free (attr_node);
          grub_free (node->compress_index);
          grub_errno = 0;
          return 0;
        }

    node->cbuf_block = -1;

    node->cbuf = grub_malloc (HFSPLUS_COMPRESS_BLOCK_SIZE);
    grub_free (attr_node);
    if (!node->cbuf) {
      node->compressed = 0;
      grub_free (node->compress_index);
      return grub_errno;
    }
    return 0;
  }
  if (cmp_head->type != HFSPLUS_COMPRESSION_INLINE) {
    grub_free (attr_node);
    return 0;
  }

  node->cbuf = grub_malloc (node->size);
  if (!node->cbuf) {
    return grub_errno;
  }

  if (grub_zlib_decompress ((char *) (cmp_head + 1),
                            grub_cpu_to_be64 (attr_head->size)
                            - sizeof (*cmp_head), 0,
                            node->cbuf, node->size) !=
      (grub_ssize_t) node->size) {
    if (!grub_errno) {
      grub_error (GRUB_ERR_BAD_COMPRESSED_DATA,
                  "premature end of compressed");
    }
    return grub_errno;
  }
  node->compressed = 1;
  return 0;
}

grub_err_t
grub_hfsplus_open_compressed (struct grub_hfsplus_file *node)
{
    return hfsplus_open_compressed_real (node);
}
