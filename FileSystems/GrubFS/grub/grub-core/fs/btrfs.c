/* btrfs.c - B-tree file system.  */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2010,2011,2012,2013  Free Software Foundation, Inc.
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
#include <grub/file.h>
#include <grub/mm.h>
#include <grub/misc.h>
#include <grub/disk.h>
#include <grub/dl.h>
#include <grub/types.h>
#include <grub/lib/crc.h>
#include <grub/deflate.h>
#include <minilzo.h>
#include <grub/i18n.h>
#include <grub/btrfs.h>

GRUB_MOD_LICENSE ("GPLv3+");

#define GRUB_BTRFS_SIGNATURE "_BHRfS_M"

/* From http://www.oberhumer.com/opensource/lzo/lzofaq.php
 * LZO will expand incompressible data by a little amount. I still haven't
 * computed the exact values, but I suggest using these formulas for
 * a worst-case expansion calculation:
 *
 * output_block_size = input_block_size + (input_block_size / 16) + 64 + 3
 *  */
#define GRUB_BTRFS_LZO_BLOCK_SIZE 4096
#define GRUB_BTRFS_LZO_BLOCK_MAX_CSIZE (GRUB_BTRFS_LZO_BLOCK_SIZE + \
				     (GRUB_BTRFS_LZO_BLOCK_SIZE / 16) + 64 + 3)

typedef grub_uint8_t grub_btrfs_checksum_t[0x20];
typedef grub_uint16_t grub_btrfs_uuid_t[8];

struct grub_btrfs_device
{
  grub_uint64_t device_id;
  grub_uint64_t size;
  grub_uint8_t dummy[0x62 - 0x10];
} GRUB_PACKED;

struct grub_btrfs_superblock
{
  grub_btrfs_checksum_t checksum;
  grub_btrfs_uuid_t uuid;
  grub_uint8_t dummy[0x10];
  grub_uint8_t signature[sizeof (GRUB_BTRFS_SIGNATURE) - 1];
  grub_uint64_t generation;
  grub_uint64_t root_tree;
  grub_uint64_t chunk_tree;
  grub_uint8_t dummy2[0x20];
  grub_uint64_t root_dir_objectid;
  grub_uint8_t dummy3[0x41];
  struct grub_btrfs_device this_device;
  char label[0x100];
  grub_uint8_t dummy4[0x100];
  grub_uint8_t bootstrap_mapping[0x800];
} GRUB_PACKED;

struct btrfs_header
{
  grub_btrfs_checksum_t checksum;
  grub_btrfs_uuid_t uuid;
  grub_uint8_t dummy[0x30];
  grub_uint32_t nitems;
  grub_uint8_t level;
} GRUB_PACKED;

struct grub_btrfs_device_desc
{
  grub_device_t dev;
  grub_uint64_t id;
};

struct grub_btrfs_data
{
  struct grub_btrfs_superblock sblock;
  grub_uint64_t tree;
  grub_uint64_t inode;

  struct grub_btrfs_device_desc *devices_attached;
  unsigned n_devices_attached;
  unsigned n_devices_allocated;

  /* Cached extent data.  */
  grub_uint64_t extstart;
  grub_uint64_t extend;
  grub_uint64_t extino;
  grub_uint64_t exttree;
  grub_size_t extsize;
  struct grub_btrfs_extent_data *extent;
};

struct grub_btrfs_chunk_item
{
  grub_uint64_t size;
  grub_uint64_t dummy;
  grub_uint64_t stripe_length;
  grub_uint64_t type;
#define GRUB_BTRFS_CHUNK_TYPE_BITS_DONTCARE 0x07
#define GRUB_BTRFS_CHUNK_TYPE_SINGLE        0x00
#define GRUB_BTRFS_CHUNK_TYPE_RAID0         0x08
#define GRUB_BTRFS_CHUNK_TYPE_RAID1         0x10
#define GRUB_BTRFS_CHUNK_TYPE_DUPLICATED    0x20
#define GRUB_BTRFS_CHUNK_TYPE_RAID10        0x40
  grub_uint8_t dummy2[0xc];
  grub_uint16_t nstripes;
  grub_uint16_t nsubstripes;
} GRUB_PACKED;

struct grub_btrfs_chunk_stripe
{
  grub_uint64_t device_id;
  grub_uint64_t offset;
  grub_btrfs_uuid_t device_uuid;
} GRUB_PACKED;

struct grub_btrfs_leaf_node
{
  struct grub_btrfs_key key;
  grub_uint32_t offset;
  grub_uint32_t size;
} GRUB_PACKED;

struct grub_btrfs_internal_node
{
  struct grub_btrfs_key key;
  grub_uint64_t addr;
  grub_uint64_t dummy;
} GRUB_PACKED;

struct grub_btrfs_dir_item
{
  struct grub_btrfs_key key;
  grub_uint8_t dummy[8];
  grub_uint16_t m;
  grub_uint16_t n;
#define GRUB_BTRFS_DIR_ITEM_TYPE_REGULAR 1
#define GRUB_BTRFS_DIR_ITEM_TYPE_DIRECTORY 2
#define GRUB_BTRFS_DIR_ITEM_TYPE_SYMLINK 7
  grub_uint8_t type;
  char name[0];
} GRUB_PACKED;

struct grub_btrfs_leaf_descriptor
{
  unsigned depth;
  unsigned allocated;
  struct
  {
    grub_disk_addr_t addr;
    unsigned iter;
    unsigned maxiter;
    int leaf;
  } *data;
};

struct grub_btrfs_time
{
  grub_int64_t sec;
  grub_uint32_t nanosec;
} __attribute__ ((aligned (4)));

struct grub_btrfs_inode
{
  grub_uint8_t dummy1[0x10];
  grub_uint64_t size;
  grub_uint8_t dummy2[0x70];
  struct grub_btrfs_time mtime;
} GRUB_PACKED;

struct grub_btrfs_extent_data
{
  grub_uint64_t dummy;
  grub_uint64_t size;
  grub_uint8_t compression;
  grub_uint8_t encryption;
  grub_uint16_t encoding;
  grub_uint8_t type;
  union
  {
    char inl[0];
    struct
    {
      grub_uint64_t laddr;
      grub_uint64_t compressed_size;
      grub_uint64_t offset;
      grub_uint64_t filled;
    };
  };
} GRUB_PACKED;

#define GRUB_BTRFS_EXTENT_INLINE 0
#define GRUB_BTRFS_EXTENT_REGULAR 1

#define GRUB_BTRFS_COMPRESSION_NONE 0
#define GRUB_BTRFS_COMPRESSION_ZLIB 1
#define GRUB_BTRFS_COMPRESSION_LZO  2

#define GRUB_BTRFS_OBJECT_ID_CHUNK 0x100

static grub_disk_addr_t superblock_sectors[] = { 64 * 2, 64 * 1024 * 2,
  256 * 1048576 * 2, 1048576ULL * 1048576ULL * 2
};

static grub_err_t
grub_btrfs_read_logical (struct grub_btrfs_data *data,
			 grub_disk_addr_t addr, void *buf, grub_size_t size,
			 int recursion_depth);

static grub_err_t
read_sblock (grub_disk_t disk, struct grub_btrfs_superblock *sb)
{
  unsigned i;
  grub_err_t err = GRUB_ERR_NONE;
  for (i = 0; i < ARRAY_SIZE (superblock_sectors); i++)
    {
      struct grub_btrfs_superblock sblock;
      /* Don't try additional superblocks beyond device size.  */
      if (i && (grub_le_to_cpu64 (sblock.this_device.size)
		>> GRUB_DISK_SECTOR_BITS) <= superblock_sectors[i])
	break;
      err = grub_disk_read (disk, superblock_sectors[i], 0,
			    sizeof (sblock), &sblock);
      if (err == GRUB_ERR_OUT_OF_RANGE)
	break;

      if (grub_memcmp ((char *) sblock.signature, GRUB_BTRFS_SIGNATURE,
		       sizeof (GRUB_BTRFS_SIGNATURE) - 1) != 0)
	break;
      if (i == 0 || grub_le_to_cpu64 (sblock.generation)
	  > grub_le_to_cpu64 (sb->generation))
	grub_memcpy (sb, &sblock, sizeof (sblock));
    }

  if ((err == GRUB_ERR_OUT_OF_RANGE || !err) && i == 0)
    return grub_error (GRUB_ERR_BAD_FS, "not a Btrfs filesystem");

  if (err == GRUB_ERR_OUT_OF_RANGE)
    grub_errno = err = GRUB_ERR_NONE;

  return err;
}

static int
key_cmp (const struct grub_btrfs_key *a, const struct grub_btrfs_key *b)
{
  if (grub_le_to_cpu64 (a->object_id) < grub_le_to_cpu64 (b->object_id))
    return -1;
  if (grub_le_to_cpu64 (a->object_id) > grub_le_to_cpu64 (b->object_id))
    return +1;

  if (a->type < b->type)
    return -1;
  if (a->type > b->type)
    return +1;

  if (grub_le_to_cpu64 (a->offset) < grub_le_to_cpu64 (b->offset))
    return -1;
  if (grub_le_to_cpu64 (a->offset) > grub_le_to_cpu64 (b->offset))
    return +1;
  return 0;
}

static void
free_iterator (struct grub_btrfs_leaf_descriptor *desc)
{
  grub_free (desc->data);
}

static grub_err_t
save_ref (struct grub_btrfs_leaf_descriptor *desc,
	  grub_disk_addr_t addr, unsigned i, unsigned m, int l)
{
  desc->depth++;
  if (desc->allocated < desc->depth)
    {
      void *newdata;
      desc->allocated *= 2;
      newdata = grub_realloc (desc->data, sizeof (desc->data[0])
			      * desc->allocated);
      if (!newdata)
	return grub_errno;
      desc->data = newdata;
    }
  desc->data[desc->depth - 1].addr = addr;
  desc->data[desc->depth - 1].iter = i;
  desc->data[desc->depth - 1].maxiter = m;
  desc->data[desc->depth - 1].leaf = l;
  return GRUB_ERR_NONE;
}

static int
next (struct grub_btrfs_data *data,
      struct grub_btrfs_leaf_descriptor *desc,
      grub_disk_addr_t * outaddr, grub_size_t * outsize,
      struct grub_btrfs_key *key_out)
{
  grub_err_t err;
  struct grub_btrfs_leaf_node leaf;

  for (; desc->depth > 0; desc->depth--)
    {
      desc->data[desc->depth - 1].iter++;
      if (desc->data[desc->depth - 1].iter
	  < desc->data[desc->depth - 1].maxiter)
	break;
    }
  if (desc->depth == 0)
    return 0;
  while (!desc->data[desc->depth - 1].leaf)
    {
      struct grub_btrfs_internal_node node;
      struct btrfs_header head;

      err = grub_btrfs_read_logical (data, desc->data[desc->depth - 1].iter
				     * sizeof (node)
				     + sizeof (struct btrfs_header)
				     + desc->data[desc->depth - 1].addr,
				     &node, sizeof (node), 0);
      if (err)
	return -err;

      err = grub_btrfs_read_logical (data, grub_le_to_cpu64 (node.addr),
				     &head, sizeof (head), 0);
      if (err)
	return -err;

      save_ref (desc, grub_le_to_cpu64 (node.addr), 0,
		grub_le_to_cpu32 (head.nitems), !head.level);
    }
  err = grub_btrfs_read_logical (data, desc->data[desc->depth - 1].iter
				 * sizeof (leaf)
				 + sizeof (struct btrfs_header)
				 + desc->data[desc->depth - 1].addr, &leaf,
				 sizeof (leaf), 0);
  if (err)
    return -err;
  *outsize = grub_le_to_cpu32 (leaf.size);
  *outaddr = desc->data[desc->depth - 1].addr + sizeof (struct btrfs_header)
    + grub_le_to_cpu32 (leaf.offset);
  *key_out = leaf.key;
  return 1;
}

static grub_err_t
lower_bound (struct grub_btrfs_data *data,
	     const struct grub_btrfs_key *key_in,
	     struct grub_btrfs_key *key_out,
	     grub_uint64_t root,
	     grub_disk_addr_t *outaddr, grub_size_t *outsize,
	     struct grub_btrfs_leaf_descriptor *desc,
	     int recursion_depth)
{
  grub_disk_addr_t addr = grub_le_to_cpu64 (root);
  int depth = -1;

  if (desc)
    {
      desc->allocated = 16;
      desc->depth = 0;
      desc->data = grub_malloc (sizeof (desc->data[0]) * desc->allocated);
      if (!desc->data)
	return grub_errno;
    }

  /* > 2 would work as well but be robust and allow a bit more just in case.
   */
  if (recursion_depth > 10)
    return grub_error (GRUB_ERR_BAD_FS, "too deep btrfs virtual nesting");

  grub_dprintf ("btrfs",
		"retrieving %" PRIxGRUB_UINT64_T
		" %x %" PRIxGRUB_UINT64_T "\n",
		key_in->object_id, key_in->type, key_in->offset);

  while (1)
    {
      grub_err_t err;
      struct btrfs_header head;

    reiter:
      depth++;
      /* FIXME: preread few nodes into buffer. */
      err = grub_btrfs_read_logical (data, addr, &head, sizeof (head),
				     recursion_depth + 1);
      if (err)
	return err;
      addr += sizeof (head);
      if (head.level)
	{
	  unsigned i;
	  struct grub_btrfs_internal_node node, node_last;
	  int have_last = 0;
	  grub_memset (&node_last, 0, sizeof (node_last));
	  for (i = 0; i < grub_le_to_cpu32 (head.nitems); i++)
	    {
	      err = grub_btrfs_read_logical (data, addr + i * sizeof (node),
					     &node, sizeof (node),
					     recursion_depth + 1);
	      if (err)
		return err;

	      grub_dprintf ("btrfs",
			    "internal node (depth %d) %" PRIxGRUB_UINT64_T
			    " %x %" PRIxGRUB_UINT64_T "\n", depth,
			    node.key.object_id, node.key.type,
			    node.key.offset);

	      if (key_cmp (&node.key, key_in) == 0)
		{
		  err = GRUB_ERR_NONE;
		  if (desc)
		    err = save_ref (desc, addr - sizeof (head), i,
				    grub_le_to_cpu32 (head.nitems), 0);
		  if (err)
		    return err;
		  addr = grub_le_to_cpu64 (node.addr);
		  goto reiter;
		}
	      if (key_cmp (&node.key, key_in) > 0)
		break;
	      node_last = node;
	      have_last = 1;
	    }
	  if (have_last)
	    {
	      err = GRUB_ERR_NONE;
	      if (desc)
		err = save_ref (desc, addr - sizeof (head), i - 1,
				grub_le_to_cpu32 (head.nitems), 0);
	      if (err)
		return err;
	      addr = grub_le_to_cpu64 (node_last.addr);
	      goto reiter;
	    }
	  *outsize = 0;
	  *outaddr = 0;
	  grub_memset (key_out, 0, sizeof (*key_out));
	  if (desc)
	    return save_ref (desc, addr - sizeof (head), -1,
			     grub_le_to_cpu32 (head.nitems), 0);
	  return GRUB_ERR_NONE;
	}
      {
	unsigned i;
	struct grub_btrfs_leaf_node leaf, leaf_last;
	int have_last = 0;
	for (i = 0; i < grub_le_to_cpu32 (head.nitems); i++)
	  {
	    err = grub_btrfs_read_logical (data, addr + i * sizeof (leaf),
					   &leaf, sizeof (leaf),
					   recursion_depth + 1);
	    if (err)
	      return err;

	    grub_dprintf ("btrfs",
			  "leaf (depth %d) %" PRIxGRUB_UINT64_T
			  " %x %" PRIxGRUB_UINT64_T "\n", depth,
			  leaf.key.object_id, leaf.key.type, leaf.key.offset);

	    if (key_cmp (&leaf.key, key_in) == 0)
	      {
		grub_memcpy (key_out, &leaf.key, sizeof (*key_out));
		*outsize = grub_le_to_cpu32 (leaf.size);
		*outaddr = addr + grub_le_to_cpu32 (leaf.offset);
		if (desc)
		  return save_ref (desc, addr - sizeof (head), i,
				   grub_le_to_cpu32 (head.nitems), 1);
		return GRUB_ERR_NONE;
	      }

	    if (key_cmp (&leaf.key, key_in) > 0)
	      break;

	    have_last = 1;
	    leaf_last = leaf;
	  }

	if (have_last)
	  {
	    grub_memcpy (key_out, &leaf_last.key, sizeof (*key_out));
	    *outsize = grub_le_to_cpu32 (leaf_last.size);
	    *outaddr = addr + grub_le_to_cpu32 (leaf_last.offset);
	    if (desc)
	      return save_ref (desc, addr - sizeof (head), i - 1,
			       grub_le_to_cpu32 (head.nitems), 1);
	    return GRUB_ERR_NONE;
	  }
	*outsize = 0;
	*outaddr = 0;
	grub_memset (key_out, 0, sizeof (*key_out));
	if (desc)
	  return save_ref (desc, addr - sizeof (head), -1,
			   grub_le_to_cpu32 (head.nitems), 1);
	return GRUB_ERR_NONE;
      }
    }
}

/* Context for find_device.  */
struct find_device_ctx
{
  struct grub_btrfs_data *data;
  grub_uint64_t id;
  grub_device_t dev_found;
};

/* Helper for find_device.  */
static int
find_device_iter (const char *name, void *data)
{
  struct find_device_ctx *ctx = data;
  grub_device_t dev;
  grub_err_t err;
  struct grub_btrfs_superblock sb;

  dev = grub_device_open (name);
  if (!dev)
    return 0;
  if (!dev->disk)
    {
      grub_device_close (dev);
      return 0;
    }
  err = read_sblock (dev->disk, &sb);
  if (err == GRUB_ERR_BAD_FS)
    {
      grub_device_close (dev);
      grub_errno = GRUB_ERR_NONE;
      return 0;
    }
  if (err)
    {
      grub_device_close (dev);
      grub_print_error ();
      return 0;
    }
  if (grub_memcmp (ctx->data->sblock.uuid, sb.uuid, sizeof (sb.uuid)) != 0
      || sb.this_device.device_id != ctx->id)
    {
      grub_device_close (dev);
      return 0;
    }

  ctx->dev_found = dev;
  return 1;
}

static grub_device_t
find_device (struct grub_btrfs_data *data, grub_uint64_t id, int do_rescan)
{
  struct find_device_ctx ctx = {
    .data = data,
    .id = id,
    .dev_found = NULL
  };
  unsigned i;

  for (i = 0; i < data->n_devices_attached; i++)
    if (id == data->devices_attached[i].id)
      return data->devices_attached[i].dev;
  if (do_rescan)
    grub_device_iterate (find_device_iter, &ctx);
  if (!ctx.dev_found)
    {
      grub_error (GRUB_ERR_BAD_FS,
		  N_("couldn't find a necessary member device "
		     "of multi-device filesystem"));
      return NULL;
    }
  data->n_devices_attached++;
  if (data->n_devices_attached > data->n_devices_allocated)
    {
      void *tmp;
      data->n_devices_allocated = 2 * data->n_devices_attached + 1;
      data->devices_attached
	= grub_realloc (tmp = data->devices_attached,
			data->n_devices_allocated
			* sizeof (data->devices_attached[0]));
      if (!data->devices_attached)
	{
	  grub_device_close (ctx.dev_found);
	  data->devices_attached = tmp;
	  return NULL;
	}
    }
  data->devices_attached[data->n_devices_attached - 1].id = id;
  data->devices_attached[data->n_devices_attached - 1].dev = ctx.dev_found;
  return ctx.dev_found;
}

static grub_err_t
grub_btrfs_read_logical (struct grub_btrfs_data *data, grub_disk_addr_t addr,
			 void *buf, grub_size_t size, int recursion_depth)
{
  while (size > 0)
    {
      grub_uint8_t *ptr;
      struct grub_btrfs_key *key;
      struct grub_btrfs_chunk_item *chunk;
      grub_uint64_t csize;
      grub_err_t err = 0;
      struct grub_btrfs_key key_out;
      int challoc = 0;
      grub_device_t dev;
      struct grub_btrfs_key key_in;
      grub_size_t chsize;
      grub_disk_addr_t chaddr;

      grub_dprintf ("btrfs", "searching for laddr %" PRIxGRUB_UINT64_T "\n",
		    addr);
      for (ptr = data->sblock.bootstrap_mapping;
	   ptr < data->sblock.bootstrap_mapping
	   + sizeof (data->sblock.bootstrap_mapping)
	   - sizeof (struct grub_btrfs_key);)
	{
	  key = (struct grub_btrfs_key *) ptr;
	  if (key->type != GRUB_BTRFS_ITEM_TYPE_CHUNK)
	    break;
	  chunk = (struct grub_btrfs_chunk_item *) (key + 1);
	  grub_dprintf ("btrfs",
			"%" PRIxGRUB_UINT64_T " %" PRIxGRUB_UINT64_T " \n",
			grub_le_to_cpu64 (key->offset),
			grub_le_to_cpu64 (chunk->size));
	  if (grub_le_to_cpu64 (key->offset) <= addr
	      && addr < grub_le_to_cpu64 (key->offset)
	      + grub_le_to_cpu64 (chunk->size))
	    goto chunk_found;
	  ptr += sizeof (*key) + sizeof (*chunk)
	    + sizeof (struct grub_btrfs_chunk_stripe)
	    * grub_le_to_cpu16 (chunk->nstripes);
	}

      key_in.object_id = grub_cpu_to_le64_compile_time (GRUB_BTRFS_OBJECT_ID_CHUNK);
      key_in.type = GRUB_BTRFS_ITEM_TYPE_CHUNK;
      key_in.offset = grub_cpu_to_le64 (addr);
      err = lower_bound (data, &key_in, &key_out,
			 data->sblock.chunk_tree,
			 &chaddr, &chsize, NULL, recursion_depth);
      if (err)
	return err;
      key = &key_out;
      if (key->type != GRUB_BTRFS_ITEM_TYPE_CHUNK
	  || !(grub_le_to_cpu64 (key->offset) <= addr))
	return grub_error (GRUB_ERR_BAD_FS,
			   "couldn't find the chunk descriptor");

      chunk = grub_malloc (chsize);
      if (!chunk)
	return grub_errno;

      challoc = 1;
      err = grub_btrfs_read_logical (data, chaddr, chunk, chsize,
				     recursion_depth);
      if (err)
	{
	  grub_free (chunk);
	  return err;
	}

    chunk_found:
      {
	grub_uint64_t stripen;
	grub_uint64_t stripe_offset;
	grub_uint64_t off = addr - grub_le_to_cpu64 (key->offset);
	unsigned redundancy = 1;
	unsigned i, j;

	if (grub_le_to_cpu64 (chunk->size) <= off)
	  {
	    grub_dprintf ("btrfs", "no chunk\n");
	    return grub_error (GRUB_ERR_BAD_FS,
			       "couldn't find the chunk descriptor");
	  }

	grub_dprintf ("btrfs", "chunk 0x%" PRIxGRUB_UINT64_T
		      "+0x%" PRIxGRUB_UINT64_T
		      " (%d stripes (%d substripes) of %"
		      PRIxGRUB_UINT64_T ")\n",
		      grub_le_to_cpu64 (key->offset),
		      grub_le_to_cpu64 (chunk->size),
		      grub_le_to_cpu16 (chunk->nstripes),
		      grub_le_to_cpu16 (chunk->nsubstripes),
		      grub_le_to_cpu64 (chunk->stripe_length));

	switch (grub_le_to_cpu64 (chunk->type)
		& ~GRUB_BTRFS_CHUNK_TYPE_BITS_DONTCARE)
	  {
	  case GRUB_BTRFS_CHUNK_TYPE_SINGLE:
	    {
	      grub_uint64_t stripe_length;
	      grub_dprintf ("btrfs", "single\n");
	      stripe_length = grub_divmod64 (grub_le_to_cpu64 (chunk->size),
					     grub_le_to_cpu16 (chunk->nstripes),
					     NULL);
	      stripen = grub_divmod64 (off, stripe_length, &stripe_offset);
	      csize = (stripen + 1) * stripe_length - off;
	      break;
	    }
	  case GRUB_BTRFS_CHUNK_TYPE_DUPLICATED:
	  case GRUB_BTRFS_CHUNK_TYPE_RAID1:
	    {
	      grub_dprintf ("btrfs", "RAID1\n");
	      stripen = 0;
	      stripe_offset = off;
	      csize = grub_le_to_cpu64 (chunk->size) - off;
	      redundancy = 2;
	      break;
	    }
	  case GRUB_BTRFS_CHUNK_TYPE_RAID0:
	    {
	      grub_uint64_t middle, high;
	      grub_uint64_t low;
	      grub_dprintf ("btrfs", "RAID0\n");
	      middle = grub_divmod64 (off,
				      grub_le_to_cpu64 (chunk->stripe_length),
				      &low);

	      high = grub_divmod64 (middle, grub_le_to_cpu16 (chunk->nstripes),
				    &stripen);
	      stripe_offset =
		low + grub_le_to_cpu64 (chunk->stripe_length) * high;
	      csize = grub_le_to_cpu64 (chunk->stripe_length) - low;
	      break;
	    }
	  case GRUB_BTRFS_CHUNK_TYPE_RAID10:
	    {
	      grub_uint64_t middle, high;
	      grub_uint64_t low;
	      middle = grub_divmod64 (off,
				      grub_le_to_cpu64 (chunk->stripe_length),
				      &low);

	      high = grub_divmod64 (middle,
				    grub_le_to_cpu16 (chunk->nstripes)
				    / grub_le_to_cpu16 (chunk->nsubstripes),
				    &stripen);
	      stripen *= grub_le_to_cpu16 (chunk->nsubstripes);
	      redundancy = grub_le_to_cpu16 (chunk->nsubstripes);
	      stripe_offset = low + grub_le_to_cpu64 (chunk->stripe_length)
		* high;
	      csize = grub_le_to_cpu64 (chunk->stripe_length) - low;
	      break;
	    }
	  default:
	    grub_dprintf ("btrfs", "unsupported RAID\n");
	    return grub_error (GRUB_ERR_NOT_IMPLEMENTED_YET,
			       "unsupported RAID flags %" PRIxGRUB_UINT64_T,
			       grub_le_to_cpu64 (chunk->type));
	  }
	if (csize == 0)
	  return grub_error (GRUB_ERR_BUG,
			     "couldn't find the chunk descriptor");
	if (csize > (grub_uint64_t) size)
	  csize = size;

	for (j = 0; j < 2; j++)
	  {
	    for (i = 0; i < redundancy; i++)
	      {
		struct grub_btrfs_chunk_stripe *stripe;
		grub_disk_addr_t paddr;

		stripe = (struct grub_btrfs_chunk_stripe *) (chunk + 1);
		/* Right now the redundancy handling is easy.
		   With RAID5-like it will be more difficult.  */
		stripe += stripen + i;

		paddr = grub_le_to_cpu64 (stripe->offset) + stripe_offset;

		grub_dprintf ("btrfs", "chunk 0x%" PRIxGRUB_UINT64_T
			      "+0x%" PRIxGRUB_UINT64_T
			      " (%d stripes (%d substripes) of %"
			      PRIxGRUB_UINT64_T ") stripe %" PRIxGRUB_UINT64_T
			      " maps to 0x%" PRIxGRUB_UINT64_T "\n",
			      grub_le_to_cpu64 (key->offset),
			      grub_le_to_cpu64 (chunk->size),
			      grub_le_to_cpu16 (chunk->nstripes),
			      grub_le_to_cpu16 (chunk->nsubstripes),
			      grub_le_to_cpu64 (chunk->stripe_length),
			      stripen, stripe->offset);
		grub_dprintf ("btrfs", "reading paddr 0x%" PRIxGRUB_UINT64_T
			      " for laddr 0x%" PRIxGRUB_UINT64_T "\n", paddr,
			      addr);

		dev = find_device (data, stripe->device_id, j);
		if (!dev)
		  {
		    err = grub_errno;
		    grub_errno = GRUB_ERR_NONE;
		    continue;
		  }

		err = grub_disk_read (dev->disk, paddr >> GRUB_DISK_SECTOR_BITS,
				      paddr & (GRUB_DISK_SECTOR_SIZE - 1),
				      csize, buf);
		if (!err)
		  break;
		grub_errno = GRUB_ERR_NONE;
	      }
	    if (i != redundancy)
	      break;
	  }
	if (err)
	  return grub_errno = err;
      }
      size -= csize;
      buf = (grub_uint8_t *) buf + csize;
      addr += csize;
      if (challoc)
	grub_free (chunk);
    }
  return GRUB_ERR_NONE;
}

static struct grub_btrfs_data *
grub_btrfs_mount (grub_device_t dev)
{
  struct grub_btrfs_data *data;
  grub_err_t err;

  if (!dev->disk)
    {
      grub_error (GRUB_ERR_BAD_FS, "not BtrFS");
      return NULL;
    }

  data = grub_zalloc (sizeof (*data));
  if (!data)
    return NULL;

  err = read_sblock (dev->disk, &data->sblock);
  if (err)
    {
      grub_free (data);
      return NULL;
    }

  data->n_devices_allocated = 16;
  data->devices_attached = grub_malloc (sizeof (data->devices_attached[0])
					* data->n_devices_allocated);
  if (!data->devices_attached)
    {
      grub_free (data);
      return NULL;
    }
  data->n_devices_attached = 1;
  data->devices_attached[0].dev = dev;
  data->devices_attached[0].id = data->sblock.this_device.device_id;

  return data;
}

static void
grub_btrfs_unmount (struct grub_btrfs_data *data)
{
  unsigned i;
  /* The device 0 is closed one layer upper.  */
  for (i = 1; i < data->n_devices_attached; i++)
    grub_device_close (data->devices_attached[i].dev);
  grub_free (data->devices_attached);
  grub_free (data->extent);
  grub_free (data);
}

static grub_err_t
grub_btrfs_read_inode (struct grub_btrfs_data *data,
		       struct grub_btrfs_inode *inode, grub_uint64_t num,
		       grub_uint64_t tree)
{
  struct grub_btrfs_key key_in, key_out;
  grub_disk_addr_t elemaddr;
  grub_size_t elemsize;
  grub_err_t err;

  key_in.object_id = num;
  key_in.type = GRUB_BTRFS_ITEM_TYPE_INODE_ITEM;
  key_in.offset = 0;

  err = lower_bound (data, &key_in, &key_out, tree, &elemaddr, &elemsize, NULL,
		     0);
  if (err)
    return err;
  if (num != key_out.object_id
      || key_out.type != GRUB_BTRFS_ITEM_TYPE_INODE_ITEM)
    return grub_error (GRUB_ERR_BAD_FS, "inode not found");

  return grub_btrfs_read_logical (data, elemaddr, inode, sizeof (*inode), 0);
}

static grub_ssize_t
grub_btrfs_lzo_decompress(char *ibuf, grub_size_t isize, grub_off_t off,
			  char *obuf, grub_size_t osize)
{
  grub_uint32_t total_size, cblock_size;
  grub_size_t ret = 0;
  char *ibuf0 = ibuf;

  total_size = grub_le_to_cpu32 (grub_get_unaligned32 (ibuf));
  ibuf += sizeof (total_size);

  if (isize < total_size)
    return -1;

  /* Jump forward to first block with requested data.  */
  while (off >= GRUB_BTRFS_LZO_BLOCK_SIZE)
    {
      /* Don't let following uint32_t cross the page boundary.  */
      if (((ibuf - ibuf0) & 0xffc) == 0xffc)
	ibuf = ((ibuf - ibuf0 + 3) & ~3) + ibuf0;

      cblock_size = grub_le_to_cpu32 (grub_get_unaligned32 (ibuf));
      ibuf += sizeof (cblock_size);

      if (cblock_size > GRUB_BTRFS_LZO_BLOCK_MAX_CSIZE)
	return -1;

      off -= GRUB_BTRFS_LZO_BLOCK_SIZE;
      ibuf += cblock_size;
    }

  while (osize > 0)
    {
      lzo_uint usize = GRUB_BTRFS_LZO_BLOCK_SIZE;

      /* Don't let following uint32_t cross the page boundary.  */
      if (((ibuf - ibuf0) & 0xffc) == 0xffc)
	ibuf = ((ibuf - ibuf0 + 3) & ~3) + ibuf0;

      cblock_size = grub_le_to_cpu32 (grub_get_unaligned32 (ibuf));
      ibuf += sizeof (cblock_size);

      if (cblock_size > GRUB_BTRFS_LZO_BLOCK_MAX_CSIZE)
	return -1;

      /* Block partially filled with requested data.  */
      if (off > 0 || osize < GRUB_BTRFS_LZO_BLOCK_SIZE)
	{
	  grub_size_t to_copy = GRUB_BTRFS_LZO_BLOCK_SIZE - off;
	  grub_uint8_t *buf;

	  if (to_copy > osize)
	    to_copy = osize;

	  buf = grub_malloc (GRUB_BTRFS_LZO_BLOCK_SIZE);
	  if (!buf)
	    return -1;

	  if (lzo1x_decompress_safe ((lzo_bytep)ibuf, cblock_size, buf, &usize,
	      NULL) != LZO_E_OK)
	    {
	      grub_free (buf);
	      return -1;
	    }

	  if (to_copy > usize)
	    to_copy = usize;
	  grub_memcpy(obuf, buf + off, to_copy);

	  osize -= to_copy;
	  ret += to_copy;
	  obuf += to_copy;
	  ibuf += cblock_size;
	  off = 0;

	  grub_free (buf);
	  continue;
	}

      /* Decompress whole block directly to output buffer.  */
      if (lzo1x_decompress_safe ((lzo_bytep)ibuf, cblock_size, (lzo_bytep)obuf,
	  &usize, NULL) != LZO_E_OK)
	return -1;

      osize -= usize;
      ret += usize;
      obuf += usize;
      ibuf += cblock_size;
    }

  return ret;
}

static grub_ssize_t
grub_btrfs_extent_read (struct grub_btrfs_data *data,
			grub_uint64_t ino, grub_uint64_t tree,
			grub_off_t pos0, char *buf, grub_size_t len)
{
  grub_off_t pos = pos0;
  while (len)
    {
      grub_size_t csize;
      grub_err_t err;
      grub_off_t extoff;
      if (!data->extent || data->extstart > pos || data->extino != ino
	  || data->exttree != tree || data->extend <= pos)
	{
	  struct grub_btrfs_key key_in, key_out;
	  grub_disk_addr_t elemaddr;
	  grub_size_t elemsize;

	  grub_free (data->extent);
	  key_in.object_id = ino;
	  key_in.type = GRUB_BTRFS_ITEM_TYPE_EXTENT_ITEM;
	  key_in.offset = grub_cpu_to_le64 (pos);
	  err = lower_bound (data, &key_in, &key_out, tree,
			     &elemaddr, &elemsize, NULL, 0);
	  if (err)
	    return -1;
	  if (key_out.object_id != ino
	      || key_out.type != GRUB_BTRFS_ITEM_TYPE_EXTENT_ITEM)
	    {
	      grub_error (GRUB_ERR_BAD_FS, "extent not found");
	      return -1;
	    }
	  if ((grub_ssize_t) elemsize < ((char *) &data->extent->inl
					 - (char *) data->extent))
	    {
	      grub_error (GRUB_ERR_BAD_FS, "extent descriptor is too short");
	      return -1;
	    }
	  data->extstart = grub_le_to_cpu64 (key_out.offset);
	  data->extsize = elemsize;
	  data->extent = grub_malloc (elemsize);
	  data->extino = ino;
	  data->exttree = tree;
	  if (!data->extent)
	    return grub_errno;

	  err = grub_btrfs_read_logical (data, elemaddr, data->extent,
					 elemsize, 0);
	  if (err)
	    return err;

	  data->extend = data->extstart + grub_le_to_cpu64 (data->extent->size);
	  if (data->extent->type == GRUB_BTRFS_EXTENT_REGULAR
	      && (char *) &data->extent + elemsize
	      >= (char *) &data->extent->filled + sizeof (data->extent->filled))
	    data->extend =
	      data->extstart + grub_le_to_cpu64 (data->extent->filled);

	  grub_dprintf ("btrfs", "regular extent 0x%" PRIxGRUB_UINT64_T "+0x%"
			PRIxGRUB_UINT64_T "\n",
			grub_le_to_cpu64 (key_out.offset),
			grub_le_to_cpu64 (data->extent->size));
	  if (data->extend <= pos)
	    {
	      grub_error (GRUB_ERR_BAD_FS, "extent not found");
	      return -1;
	    }
	}
      csize = data->extend - pos;
      extoff = pos - data->extstart;
      if (csize > len)
	csize = len;

      if (data->extent->encryption)
	{
	  grub_error (GRUB_ERR_NOT_IMPLEMENTED_YET,
		      "encryption not supported");
	  return -1;
	}

      if (data->extent->compression != GRUB_BTRFS_COMPRESSION_NONE
	  && data->extent->compression != GRUB_BTRFS_COMPRESSION_ZLIB
	  && data->extent->compression != GRUB_BTRFS_COMPRESSION_LZO)
	{
	  grub_error (GRUB_ERR_NOT_IMPLEMENTED_YET,
		      "compression type 0x%x not supported",
		      data->extent->compression);
	  return -1;
	}

      if (data->extent->encoding)
	{
	  grub_error (GRUB_ERR_NOT_IMPLEMENTED_YET, "encoding not supported");
	  return -1;
	}

      switch (data->extent->type)
	{
	case GRUB_BTRFS_EXTENT_INLINE:
	  if (data->extent->compression == GRUB_BTRFS_COMPRESSION_ZLIB)
	    {
	      if (grub_zlib_decompress (data->extent->inl, data->extsize -
					((grub_uint8_t *) data->extent->inl
					 - (grub_uint8_t *) data->extent),
					extoff, buf, csize)
		  != (grub_ssize_t) csize)
		{
		  if (!grub_errno)
		    grub_error (GRUB_ERR_BAD_COMPRESSED_DATA,
				"premature end of compressed");
		  return -1;
		}
	    }
	  else if (data->extent->compression == GRUB_BTRFS_COMPRESSION_LZO)
	    {
	      if (grub_btrfs_lzo_decompress(data->extent->inl, data->extsize -
					   ((grub_uint8_t *) data->extent->inl
					    - (grub_uint8_t *) data->extent),
					   extoff, buf, csize)
		  != (grub_ssize_t) csize)
		return -1;
	    }
	  else
	    grub_memcpy (buf, data->extent->inl + extoff, csize);
	  break;
	case GRUB_BTRFS_EXTENT_REGULAR:
	  if (!data->extent->laddr)
	    {
	      grub_memset (buf, 0, csize);
	      break;
	    }

	  if (data->extent->compression != GRUB_BTRFS_COMPRESSION_NONE)
	    {
	      char *tmp;
	      grub_uint64_t zsize;
	      grub_ssize_t ret;

	      zsize = grub_le_to_cpu64 (data->extent->compressed_size);
	      tmp = grub_malloc (zsize);
	      if (!tmp)
		return -1;
	      err = grub_btrfs_read_logical (data,
					     grub_le_to_cpu64 (data->extent->laddr),
					     tmp, zsize, 0);
	      if (err)
		{
		  grub_free (tmp);
		  return -1;
		}

	      if (data->extent->compression == GRUB_BTRFS_COMPRESSION_ZLIB)
		ret = grub_zlib_decompress (tmp, zsize, extoff
				    + grub_le_to_cpu64 (data->extent->offset),
				    buf, csize);
	      else if (data->extent->compression == GRUB_BTRFS_COMPRESSION_LZO)
		ret = grub_btrfs_lzo_decompress (tmp, zsize, extoff
				    + grub_le_to_cpu64 (data->extent->offset),
				    buf, csize);
	      else
		ret = -1;

	      grub_free (tmp);

	      if (ret != (grub_ssize_t) csize)
		{
		  if (!grub_errno)
		    grub_error (GRUB_ERR_BAD_COMPRESSED_DATA,
				"premature end of compressed");
		  return -1;
		}

	      break;
	    }
	  err = grub_btrfs_read_logical (data,
					 grub_le_to_cpu64 (data->extent->laddr)
					 + grub_le_to_cpu64 (data->extent->offset)
					 + extoff, buf, csize, 0);
	  if (err)
	    return -1;
	  break;
	default:
	  grub_error (GRUB_ERR_NOT_IMPLEMENTED_YET,
		      "unsupported extent type 0x%x", data->extent->type);
	  return -1;
	}
      buf += csize;
      pos += csize;
      len -= csize;
    }
  return pos - pos0;
}

static grub_err_t
get_root (struct grub_btrfs_data *data, struct grub_btrfs_key *key,
	  grub_uint64_t *tree, grub_uint8_t *type)
{
  grub_err_t err;
  grub_disk_addr_t elemaddr;
  grub_size_t elemsize;
  struct grub_btrfs_key key_out, key_in;
  struct grub_btrfs_root_item ri;

  key_in.object_id = grub_cpu_to_le64_compile_time (GRUB_BTRFS_ROOT_VOL_OBJECTID);
  key_in.offset = 0;
  key_in.type = GRUB_BTRFS_ITEM_TYPE_ROOT_ITEM;
  err = lower_bound (data, &key_in, &key_out,
		     data->sblock.root_tree,
		     &elemaddr, &elemsize, NULL, 0);
  if (err)
    return err;
  if (key_in.object_id != key_out.object_id
      || key_in.type != key_out.type
      || key_in.offset != key_out.offset)
    return grub_error (GRUB_ERR_BAD_FS, "no root");
  err = grub_btrfs_read_logical (data, elemaddr, &ri,
				 sizeof (ri), 0);
  if (err)
    return err;
  key->type = GRUB_BTRFS_ITEM_TYPE_DIR_ITEM;
  key->offset = 0;
  key->object_id = grub_cpu_to_le64_compile_time (GRUB_BTRFS_OBJECT_ID_CHUNK);
  *tree = ri.tree;
  *type = GRUB_BTRFS_DIR_ITEM_TYPE_DIRECTORY;
  return GRUB_ERR_NONE;
}

static grub_err_t
find_path (struct grub_btrfs_data *data,
	   const char *path, struct grub_btrfs_key *key,
	   grub_uint64_t *tree, grub_uint8_t *type)
{
  const char *slash = path;
  grub_err_t err;
  grub_disk_addr_t elemaddr;
  grub_size_t elemsize;
  grub_size_t allocated = 0;
  struct grub_btrfs_dir_item *direl = NULL;
  struct grub_btrfs_key key_out;
  const char *ctoken;
  grub_size_t ctokenlen;
  char *path_alloc = NULL;
  char *origpath = NULL;
  unsigned symlinks_max = 32;

  err = get_root (data, key, tree, type);
  if (err)
    return err;

  origpath = grub_strdup (path);
  if (!origpath)
    return grub_errno;

  while (1)
    {
      while (path[0] == '/')
	path++;
      if (!path[0])
	break;
      slash = grub_strchr (path, '/');
      if (!slash)
	slash = path + grub_strlen (path);
      ctoken = path;
      ctokenlen = slash - path;

      if (*type != GRUB_BTRFS_DIR_ITEM_TYPE_DIRECTORY)
	{
	  grub_free (path_alloc);
	  grub_free (origpath);
	  return grub_error (GRUB_ERR_BAD_FILE_TYPE, N_("not a directory"));
	}

      if (ctokenlen == 1 && ctoken[0] == '.')
	{
	  path = slash;
	  continue;
	}
      if (ctokenlen == 2 && ctoken[0] == '.' && ctoken[1] == '.')
	{
	  key->type = GRUB_BTRFS_ITEM_TYPE_INODE_REF;
	  key->offset = -1;

	  err = lower_bound (data, key, &key_out, *tree, &elemaddr, &elemsize,
			     NULL, 0);
	  if (err)
	    {
	      grub_free (direl);
	      grub_free (path_alloc);
	      grub_free (origpath);
	      return err;
	    }

	  if (key_out.type != key->type
	      || key->object_id != key_out.object_id)
	    {
	      grub_free (direl);
	      grub_free (path_alloc);
	      err = grub_error (GRUB_ERR_FILE_NOT_FOUND, N_("file `%s' not found"), origpath);
	      grub_free (origpath);
	      return err;
	    }

	  *type = GRUB_BTRFS_DIR_ITEM_TYPE_DIRECTORY;
	  key->object_id = key_out.offset;

	  path = slash;

	  continue;
	}

      key->type = GRUB_BTRFS_ITEM_TYPE_DIR_ITEM;
      key->offset = grub_cpu_to_le64 (~grub_getcrc32c (1, ctoken, ctokenlen));

      err = lower_bound (data, key, &key_out, *tree, &elemaddr, &elemsize,
			 NULL, 0);
      if (err)
	{
	  grub_free (direl);
	  grub_free (path_alloc);
	  grub_free (origpath);
	  return err;
	}
      if (key_cmp (key, &key_out) != 0)
	{
	  grub_free (direl);
	  grub_free (path_alloc);
	  err = grub_error (GRUB_ERR_FILE_NOT_FOUND, N_("file `%s' not found"), origpath);
	  grub_free (origpath);
	  return err;
	}

      struct grub_btrfs_dir_item *cdirel;
      if (elemsize > allocated)
	{
	  allocated = 2 * elemsize;
	  grub_free (direl);
	  direl = grub_malloc (allocated + 1);
	  if (!direl)
	    {
	      grub_free (path_alloc);
	      grub_free (origpath);
	      return grub_errno;
	    }
	}

      err = grub_btrfs_read_logical (data, elemaddr, direl, elemsize, 0);
      if (err)
	{
	  grub_free (direl);
	  grub_free (path_alloc);
	  grub_free (origpath);
	  return err;
	}

      for (cdirel = direl;
	   (grub_uint8_t *) cdirel - (grub_uint8_t *) direl
	   < (grub_ssize_t) elemsize;
	   cdirel = (void *) ((grub_uint8_t *) (direl + 1)
			      + grub_le_to_cpu16 (cdirel->n)
			      + grub_le_to_cpu16 (cdirel->m)))
	{
	  if (ctokenlen == grub_le_to_cpu16 (cdirel->n)
	      && grub_memcmp (cdirel->name, ctoken, ctokenlen) == 0)
	    break;
	}
      if ((grub_uint8_t *) cdirel - (grub_uint8_t *) direl
	  >= (grub_ssize_t) elemsize)
	{
	  grub_free (direl);
	  grub_free (path_alloc);
	  err = grub_error (GRUB_ERR_FILE_NOT_FOUND, N_("file `%s' not found"), origpath);
	  grub_free (origpath);
	  return err;
	}

      path = slash;
      if (cdirel->type == GRUB_BTRFS_DIR_ITEM_TYPE_SYMLINK)
	{
	  struct grub_btrfs_inode inode;
	  char *tmp;
	  if (--symlinks_max == 0)
	    {
	      grub_free (direl);
	      grub_free (path_alloc);
	      grub_free (origpath);
	      return grub_error (GRUB_ERR_SYMLINK_LOOP,
				 N_("too deep nesting of symlinks"));
	    }

	  err = grub_btrfs_read_inode (data, &inode,
				       cdirel->key.object_id, *tree);
	  if (err)
	    {
	      grub_free (direl);
	      grub_free (path_alloc);
	      grub_free (origpath);
	      return err;
	    }
	  tmp = grub_malloc (grub_le_to_cpu64 (inode.size)
			     + grub_strlen (path) + 1);
	  if (!tmp)
	    {
	      grub_free (direl);
	      grub_free (path_alloc);
	      grub_free (origpath);
	      return grub_errno;
	    }

	  if (grub_btrfs_extent_read (data, cdirel->key.object_id,
				      *tree, 0, tmp,
				      grub_le_to_cpu64 (inode.size))
	      != (grub_ssize_t) grub_le_to_cpu64 (inode.size))
	    {
	      grub_free (direl);
	      grub_free (path_alloc);
	      grub_free (origpath);
	      grub_free (tmp);
	      return grub_errno;
	    }
	  grub_memcpy (tmp + grub_le_to_cpu64 (inode.size), path,
		       grub_strlen (path) + 1);
	  grub_free (path_alloc);
	  path = path_alloc = tmp;
	  if (path[0] == '/')
	    {
	      err = get_root (data, key, tree, type);
	      if (err)
		return err;
	    }
	  continue;
	}
      *type = cdirel->type;

      switch (cdirel->key.type)
	{
	case GRUB_BTRFS_ITEM_TYPE_ROOT_ITEM:
	  {
	    struct grub_btrfs_root_item ri;
	    err = lower_bound (data, &cdirel->key, &key_out,
			       data->sblock.root_tree,
			       &elemaddr, &elemsize, NULL, 0);
	    if (err)
	      {
		grub_free (direl);
		grub_free (path_alloc);
		grub_free (origpath);
		return err;
	      }
	    if (cdirel->key.object_id != key_out.object_id
		|| cdirel->key.type != key_out.type)
	      {
		grub_free (direl);
		grub_free (path_alloc);
		err = grub_error (GRUB_ERR_FILE_NOT_FOUND, N_("file `%s' not found"), origpath);
		grub_free (origpath);
		return err;
	      }
	    err = grub_btrfs_read_logical (data, elemaddr, &ri,
					   sizeof (ri), 0);
	    if (err)
	      {
		grub_free (direl);
		grub_free (path_alloc);
		grub_free (origpath);
		return err;
	      }
	    key->type = GRUB_BTRFS_ITEM_TYPE_DIR_ITEM;
	    key->offset = 0;
	    key->object_id = grub_cpu_to_le64_compile_time (GRUB_BTRFS_OBJECT_ID_CHUNK);
	    *tree = ri.tree;
	    break;
	  }
	case GRUB_BTRFS_ITEM_TYPE_INODE_ITEM:
	  if (*slash && *type == GRUB_BTRFS_DIR_ITEM_TYPE_REGULAR)
	    {
	      grub_free (direl);
	      grub_free (path_alloc);
	      err = grub_error (GRUB_ERR_FILE_NOT_FOUND, N_("file `%s' not found"), origpath);
	      grub_free (origpath);
	      return err;
	    }
	  *key = cdirel->key;
	  if (*type == GRUB_BTRFS_DIR_ITEM_TYPE_DIRECTORY)
	    key->type = GRUB_BTRFS_ITEM_TYPE_DIR_ITEM;
	  break;
	default:
	  grub_free (path_alloc);
	  grub_free (origpath);
	  grub_free (direl);
	  return grub_error (GRUB_ERR_BAD_FS, "unrecognised object type 0x%x",
			     cdirel->key.type);
	}
    }

  grub_free (direl);
  grub_free (origpath);
  grub_free (path_alloc);
  return GRUB_ERR_NONE;
}

static grub_err_t
grub_btrfs_dir (grub_device_t device, const char *path,
		grub_fs_dir_hook_t hook, void *hook_data)
{
  struct grub_btrfs_data *data = grub_btrfs_mount (device);
  struct grub_btrfs_key key_in, key_out;
  grub_err_t err;
  grub_disk_addr_t elemaddr;
  grub_size_t elemsize;
  grub_size_t allocated = 0;
  struct grub_btrfs_dir_item *direl = NULL;
  struct grub_btrfs_leaf_descriptor desc;
  int r = 0;
  grub_uint64_t tree;
  grub_uint8_t type;

  if (!data)
    return grub_errno;

  err = find_path (data, path, &key_in, &tree, &type);
  if (err)
    {
      grub_btrfs_unmount (data);
      return err;
    }
  if (type != GRUB_BTRFS_DIR_ITEM_TYPE_DIRECTORY)
    {
      grub_btrfs_unmount (data);
      return grub_error (GRUB_ERR_BAD_FILE_TYPE, N_("not a directory"));
    }

  err = lower_bound (data, &key_in, &key_out, tree,
		     &elemaddr, &elemsize, &desc, 0);
  if (err)
    {
      grub_btrfs_unmount (data);
      return err;
    }
  if (key_out.type != GRUB_BTRFS_ITEM_TYPE_DIR_ITEM
      || key_out.object_id != key_in.object_id)
    {
      r = next (data, &desc, &elemaddr, &elemsize, &key_out);
      if (r <= 0)
	goto out;
    }
  do
    {
      struct grub_btrfs_dir_item *cdirel;
      if (key_out.type != GRUB_BTRFS_ITEM_TYPE_DIR_ITEM
	  || key_out.object_id != key_in.object_id)
	{
	  r = 0;
	  break;
	}
      if (elemsize > allocated)
	{
	  allocated = 2 * elemsize;
	  grub_free (direl);
	  direl = grub_malloc (allocated + 1);
	  if (!direl)
	    {
	      r = -grub_errno;
	      break;
	    }
	}

      err = grub_btrfs_read_logical (data, elemaddr, direl, elemsize, 0);
      if (err)
	{
	  r = -err;
	  break;
	}

      for (cdirel = direl;
	   (grub_uint8_t *) cdirel - (grub_uint8_t *) direl
	   < (grub_ssize_t) elemsize;
	   cdirel = (void *) ((grub_uint8_t *) (direl + 1)
			      + grub_le_to_cpu16 (cdirel->n)
			      + grub_le_to_cpu16 (cdirel->m)))
	{
	  char c;
	  struct grub_btrfs_inode inode;
	  struct grub_dirhook_info info;
	  err = grub_btrfs_read_inode (data, &inode, cdirel->key.object_id,
				       tree);
	  grub_memset (&info, 0, sizeof (info));
	  if (err)
	    grub_errno = GRUB_ERR_NONE;
	  else
	    {
	      info.mtime = grub_le_to_cpu64 (inode.mtime.sec);
	      info.mtimeset = 1;
	    }
	  c = cdirel->name[grub_le_to_cpu16 (cdirel->n)];
	  cdirel->name[grub_le_to_cpu16 (cdirel->n)] = 0;
	  info.dir = (cdirel->type == GRUB_BTRFS_DIR_ITEM_TYPE_DIRECTORY);
	  if (hook (cdirel->name, &info, hook_data))
	    goto out;
	  cdirel->name[grub_le_to_cpu16 (cdirel->n)] = c;
	}
      r = next (data, &desc, &elemaddr, &elemsize, &key_out);
    }
  while (r > 0);

out:
  grub_free (direl);

  free_iterator (&desc);
  grub_btrfs_unmount (data);

  return -r;
}

static grub_err_t
grub_btrfs_open (struct grub_file *file, const char *name)
{
  struct grub_btrfs_data *data = grub_btrfs_mount (file->device);
  grub_err_t err;
  struct grub_btrfs_inode inode;
  grub_uint8_t type;
  struct grub_btrfs_key key_in;

  if (!data)
    return grub_errno;

  err = find_path (data, name, &key_in, &data->tree, &type);
  if (err)
    {
      grub_btrfs_unmount (data);
      return err;
    }
  if (type != GRUB_BTRFS_DIR_ITEM_TYPE_REGULAR)
    {
      grub_btrfs_unmount (data);
      return grub_error (GRUB_ERR_BAD_FILE_TYPE, N_("not a regular file"));
    }

  data->inode = key_in.object_id;
  err = grub_btrfs_read_inode (data, &inode, data->inode, data->tree);
  if (err)
    {
      grub_btrfs_unmount (data);
      return err;
    }

  file->data = data;
  file->size = grub_le_to_cpu64 (inode.size);

  return err;
}

static grub_err_t
grub_btrfs_close (grub_file_t file)
{
  grub_btrfs_unmount (file->data);

  return GRUB_ERR_NONE;
}

static grub_ssize_t
grub_btrfs_read (grub_file_t file, char *buf, grub_size_t len)
{
  struct grub_btrfs_data *data = file->data;

  return grub_btrfs_extent_read (data, data->inode,
				 data->tree, file->offset, buf, len);
}

static grub_err_t
grub_btrfs_uuid (grub_device_t device, char **uuid)
{
  struct grub_btrfs_data *data;

  *uuid = NULL;

  data = grub_btrfs_mount (device);
  if (!data)
    return grub_errno;

  *uuid = grub_xasprintf ("%04x%04x-%04x-%04x-%04x-%04x%04x%04x",
			  grub_be_to_cpu16 (data->sblock.uuid[0]),
			  grub_be_to_cpu16 (data->sblock.uuid[1]),
			  grub_be_to_cpu16 (data->sblock.uuid[2]),
			  grub_be_to_cpu16 (data->sblock.uuid[3]),
			  grub_be_to_cpu16 (data->sblock.uuid[4]),
			  grub_be_to_cpu16 (data->sblock.uuid[5]),
			  grub_be_to_cpu16 (data->sblock.uuid[6]),
			  grub_be_to_cpu16 (data->sblock.uuid[7]));

  grub_btrfs_unmount (data);

  return grub_errno;
}

static grub_err_t
grub_btrfs_label (grub_device_t device, char **label)
{
  struct grub_btrfs_data *data;

  *label = NULL;

  data = grub_btrfs_mount (device);
  if (!data)
    return grub_errno;

  *label = grub_strndup (data->sblock.label, sizeof (data->sblock.label));

  grub_btrfs_unmount (data);

  return grub_errno;
}

#ifdef GRUB_UTIL
static grub_err_t
grub_btrfs_embed (grub_device_t device __attribute__ ((unused)),
		  unsigned int *nsectors,
		  unsigned int max_nsectors,
		  grub_embed_type_t embed_type,
		  grub_disk_addr_t **sectors)
{
  unsigned i;

  if (embed_type != GRUB_EMBED_PCBIOS)
    return grub_error (GRUB_ERR_NOT_IMPLEMENTED_YET,
		       "BtrFS currently supports only PC-BIOS embedding");

  if (64 * 2 - 1 < *nsectors)
    return grub_error (GRUB_ERR_OUT_OF_RANGE,
		       N_("your core.img is unusually large.  "
			  "It won't fit in the embedding area"));

  *nsectors = 64 * 2 - 1;
  if (*nsectors > max_nsectors)
    *nsectors = max_nsectors;
  *sectors = grub_malloc (*nsectors * sizeof (**sectors));
  if (!*sectors)
    return grub_errno;
  for (i = 0; i < *nsectors; i++)
    (*sectors)[i] = i + 1;

  return GRUB_ERR_NONE;
}
#endif

static struct grub_fs grub_btrfs_fs = {
  .name = "btrfs",
  .dir = grub_btrfs_dir,
  .open = grub_btrfs_open,
  .read = grub_btrfs_read,
  .close = grub_btrfs_close,
  .uuid = grub_btrfs_uuid,
  .label = grub_btrfs_label,
#ifdef GRUB_UTIL
  .embed = grub_btrfs_embed,
  .reserved_first_sector = 1,
  .blocklist_install = 0,
#endif
};

GRUB_MOD_INIT (btrfs)
{
  grub_fs_register (&grub_btrfs_fs);
}

GRUB_MOD_FINI (btrfs)
{
  grub_fs_unregister (&grub_btrfs_fs);
}
