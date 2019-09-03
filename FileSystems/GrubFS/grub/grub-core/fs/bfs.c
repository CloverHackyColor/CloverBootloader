/* bfs.c - The Bee File System.  */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2010,2011  Free Software Foundation, Inc.
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
/*
  Based on the book "Practical File System Design by Dominic Giampaolo
  with corrections and completitions based on Haiku code.
*/

#include <Uefi/UefiSpec.h>
#include <grub/misc.h>
#include <grub/err.h>
#include <grub/file.h>
#include <grub/mm.h>
#include <grub/disk.h>
#include <grub/dl.h>
#include <grub/types.h>
#include <grub/i18n.h>

GRUB_MOD_LICENSE ("GPLv3+");

#ifdef MODE_AFS
#define BTREE_ALIGN 4
#define SUPERBLOCK  2
#else
#define BTREE_ALIGN 8
#define SUPERBLOCK  1
#endif

#define grub_bfs_to_cpu16 grub_le_to_cpu16
#define grub_bfs_to_cpu32 grub_le_to_cpu32
#define grub_bfs_to_cpu64 grub_le_to_cpu64
#define grub_cpu_to_bfs32_compile_time grub_cpu_to_le32_compile_time

#ifdef MODE_AFS
#define grub_bfs_to_cpu_treehead grub_bfs_to_cpu32
#else
#define grub_bfs_to_cpu_treehead grub_bfs_to_cpu16
#endif

#ifdef MODE_AFS
#define SUPER_BLOCK_MAGIC1 0x41465331
#else
#define SUPER_BLOCK_MAGIC1 0x42465331
#endif
#define SUPER_BLOCK_MAGIC2 0xdd121031
#define SUPER_BLOCK_MAGIC3 0x15b6830e
#define POINTER_INVALID 0xffffffffffffffffULL

#define ATTR_TYPE      0160000
#define ATTR_REG       0100000
#define ATTR_DIR       0040000
#define ATTR_LNK       0120000

#define DOUBLE_INDIRECT_SHIFT 2

#define LOG_EXTENT_SIZE 3
struct grub_bfs_extent
{
  grub_uint32_t ag;
  grub_uint16_t start;
  grub_uint16_t len;
} GRUB_PACKED;

struct grub_bfs_superblock
{
  char label[32];
  grub_uint32_t magic1;
  grub_uint32_t unused1;
  grub_uint32_t bsize;
  grub_uint32_t log2_bsize;
  grub_uint8_t unused[20];
  grub_uint32_t magic2;
  grub_uint32_t unused2;
  grub_uint32_t log2_ag_size;
  grub_uint8_t unused3[32];
  grub_uint32_t magic3;
  struct grub_bfs_extent root_dir;
} GRUB_PACKED;

struct grub_bfs_inode
{
  grub_uint8_t unused[20];
  grub_uint32_t mode;
  grub_uint32_t flags;
#ifdef MODE_AFS
  grub_uint8_t unused2[12];
#else
  grub_uint8_t unused2[8];
#endif
  grub_uint64_t mtime;
  grub_uint8_t unused3[8];
  struct grub_bfs_extent attr;
  grub_uint8_t unused4[12];

  union
  {
    struct
    {
      struct grub_bfs_extent direct[12];
      grub_uint64_t max_direct_range;
      struct grub_bfs_extent indirect;
      grub_uint64_t max_indirect_range;
      struct grub_bfs_extent double_indirect;
      grub_uint64_t max_double_indirect_range;
      grub_uint64_t size;
      grub_uint32_t pad[4];
    } GRUB_PACKED;
    char inplace_link[144];
  } GRUB_PACKED;
  grub_uint8_t small_data[0];
} GRUB_PACKED;

enum
{
  LONG_SYMLINK = 0x40
};

struct grub_bfs_small_data_element_header
{
  grub_uint32_t type;
  grub_uint16_t name_len;
  grub_uint16_t value_len;
} GRUB_PACKED;

struct grub_bfs_btree_header
{
  grub_uint32_t magic;
#ifdef MODE_AFS
  grub_uint64_t root;
  grub_uint32_t level;
  grub_uint32_t node_size;
  grub_uint32_t unused;
#else
  grub_uint32_t node_size;
  grub_uint32_t level;
  grub_uint32_t unused;
  grub_uint64_t root;
#endif
  grub_uint32_t unused2[2];
} GRUB_PACKED;

struct grub_bfs_btree_node
{
  grub_uint64_t unused;
  grub_uint64_t right;
  grub_uint64_t overflow;
#ifdef MODE_AFS
  grub_uint32_t count_keys;
  grub_uint32_t total_key_len;
#else
  grub_uint16_t count_keys;
  grub_uint16_t total_key_len;
#endif
} GRUB_PACKED;

struct grub_bfs_data
{
  struct grub_bfs_superblock sb;
  struct grub_bfs_inode ino;
};

/* Context for grub_bfs_dir.  */
struct grub_bfs_dir_ctx
{
  grub_device_t device;
  grub_fs_dir_hook_t hook;
  void *hook_data;
  struct grub_bfs_superblock sb;
};

static grub_err_t
read_extent (grub_disk_t disk,
	     const struct grub_bfs_superblock *sb,
	     const struct grub_bfs_extent *in,
	     grub_off_t off, grub_off_t byteoff, void *buf, grub_size_t len)
{
#ifdef MODE_AFS
  return grub_disk_read (disk, ((grub_bfs_to_cpu32 (in->ag)
				 << (grub_bfs_to_cpu32 (sb->log2_ag_size)
				     - GRUB_DISK_SECTOR_BITS))
				+ ((grub_bfs_to_cpu16 (in->start) + off)
				   << (grub_bfs_to_cpu32 (sb->log2_bsize)
				       - GRUB_DISK_SECTOR_BITS))),
			 byteoff, len, buf);
#else
  return grub_disk_read (disk, (((grub_bfs_to_cpu32 (in->ag)
				  << grub_bfs_to_cpu32 (sb->log2_ag_size))
				 + grub_bfs_to_cpu16 (in->start) + off)
				<< (grub_bfs_to_cpu32 (sb->log2_bsize)
				    - GRUB_DISK_SECTOR_BITS)),
			 byteoff, len, buf);
#endif
}

#ifdef MODE_AFS
#define RANGE_SHIFT grub_bfs_to_cpu32 (sb->log2_bsize)
#else
#define RANGE_SHIFT 0
#endif

static grub_err_t
read_bfs_file (grub_disk_t disk,
	       const struct grub_bfs_superblock *sb,
	       const struct grub_bfs_inode *ino,
	       grub_off_t off, void *buf, grub_size_t len,
	       grub_disk_read_hook_t read_hook, void *read_hook_data)
{
  if (len == 0)
    return GRUB_ERR_NONE;

  if (off + len > grub_bfs_to_cpu64 (ino->size))
    return grub_error (GRUB_ERR_OUT_OF_RANGE,
		       N_("attempt to read past the end of file"));

  if (off < (grub_bfs_to_cpu64 (ino->max_direct_range) << RANGE_SHIFT))
    {
      unsigned i;
      grub_uint64_t pos = 0;
      for (i = 0; i < ARRAY_SIZE (ino->direct); i++)
	{
	  grub_uint64_t newpos;
	  newpos = pos + (((grub_uint64_t) grub_bfs_to_cpu16 (ino->direct[i].len))
			  << grub_bfs_to_cpu32 (sb->log2_bsize));
	  if (newpos > off)
	    {
	      grub_size_t read_size;
	      grub_err_t err;
	      read_size = newpos - off;
	      if (read_size > len)
		read_size = len;
	      disk->read_hook = read_hook;
	      disk->read_hook_data = read_hook_data;
	      err = read_extent (disk, sb, &ino->direct[i], 0, off - pos,
				 buf, read_size);
	      disk->read_hook = 0;
	      if (err)
		return err;
	      off += read_size;
	      len -= read_size;
	      buf = (char *) buf + read_size;
	      if (len == 0)
		return GRUB_ERR_NONE;
	    }
	  pos = newpos;
	}
    }

  if (off < (grub_bfs_to_cpu64 (ino->max_direct_range) << RANGE_SHIFT))
    return grub_error (GRUB_ERR_BAD_FS, "incorrect direct blocks");

  if (off < (grub_bfs_to_cpu64 (ino->max_indirect_range) << RANGE_SHIFT))
    {
      unsigned i;
      struct grub_bfs_extent *entries;
      grub_size_t nentries;
      grub_err_t err;
      grub_uint64_t pos = (grub_bfs_to_cpu64 (ino->max_direct_range)
			   << RANGE_SHIFT);
      nentries = (((grub_size_t) grub_bfs_to_cpu16 (ino->indirect.len))
		  << (grub_bfs_to_cpu32 (sb->log2_bsize) - LOG_EXTENT_SIZE));
      entries = grub_malloc (nentries << LOG_EXTENT_SIZE);
      if (!entries)
	return grub_errno;
      err = read_extent (disk, sb, &ino->indirect, 0, 0,
			 entries, nentries << LOG_EXTENT_SIZE);
      for (i = 0; i < nentries; i++)
	{
	  grub_uint64_t newpos;
	  newpos = pos + (((grub_uint64_t) grub_bfs_to_cpu16 (entries[i].len))
			  << grub_bfs_to_cpu32 (sb->log2_bsize));
	  if (newpos > off)
	    {
	      grub_size_t read_size;
	      read_size = newpos - off;
	      if (read_size > len)
		read_size = len;
	      disk->read_hook = read_hook;
	      disk->read_hook_data = read_hook_data;
	      err = read_extent (disk, sb, &entries[i], 0, off - pos,
				 buf, read_size);
	      disk->read_hook = 0;
	      if (err)
		{
		  grub_free (entries);
		  return err;
		}
	      off += read_size;
	      len -= read_size;
	      buf = (char *) buf + read_size;
	      if (len == 0)
		{
		  grub_free (entries);
		  return GRUB_ERR_NONE;
		}
	    }
	  pos = newpos;
	}
      grub_free (entries);
    }

  if (off < (grub_bfs_to_cpu64 (ino->max_indirect_range) << RANGE_SHIFT))
    return grub_error (GRUB_ERR_BAD_FS, "incorrect indirect blocks");

  {
    struct grub_bfs_extent *l1_entries, *l2_entries;
    grub_size_t nl1_entries, nl2_entries;
    grub_off_t last_l1n = ~0ULL;
    grub_err_t err;
    nl1_entries = (((grub_uint64_t) grub_bfs_to_cpu16 (ino->double_indirect.len))
		   << (grub_bfs_to_cpu32 (sb->log2_bsize) - LOG_EXTENT_SIZE));
    l1_entries = grub_malloc (nl1_entries << LOG_EXTENT_SIZE);
    if (!l1_entries)
      return grub_errno;
    nl2_entries = 0;
    l2_entries = grub_malloc (1 << (DOUBLE_INDIRECT_SHIFT
				    + grub_bfs_to_cpu32 (sb->log2_bsize)));
    if (!l2_entries)
      {
	grub_free (l1_entries);
	return grub_errno;
      }
    err = read_extent (disk, sb, &ino->double_indirect, 0, 0,
		       l1_entries, nl1_entries << LOG_EXTENT_SIZE);
    if (err)
      {
	grub_free (l1_entries);
	grub_free (l2_entries);
	return err;
      }

    while (len > 0)
      {
	grub_off_t boff, l2n, l1n;
	grub_size_t read_size;
	grub_off_t double_indirect_offset;
	double_indirect_offset = off
	  - grub_bfs_to_cpu64 (ino->max_indirect_range);
	boff = (double_indirect_offset
		& ((1 << (grub_bfs_to_cpu32 (sb->log2_bsize)
			  + DOUBLE_INDIRECT_SHIFT)) - 1));
	l2n = ((double_indirect_offset >> (grub_bfs_to_cpu32 (sb->log2_bsize)
					   + DOUBLE_INDIRECT_SHIFT))
	       & ((1 << (grub_bfs_to_cpu32 (sb->log2_bsize) - LOG_EXTENT_SIZE
			 + DOUBLE_INDIRECT_SHIFT)) - 1));
	l1n =
	  (double_indirect_offset >>
	   (2 * grub_bfs_to_cpu32 (sb->log2_bsize) - LOG_EXTENT_SIZE +
	    2 * DOUBLE_INDIRECT_SHIFT));
	if (l1n > nl1_entries)
	  {
	    grub_free (l1_entries);
	    grub_free (l2_entries);
	    return grub_error (GRUB_ERR_BAD_FS,
			       "incorrect double-indirect block");
	  }
	if (l1n != last_l1n)
	  {
	    nl2_entries = (((grub_uint64_t) grub_bfs_to_cpu16 (l1_entries[l1n].len))
			   << (grub_bfs_to_cpu32 (sb->log2_bsize)
			       - LOG_EXTENT_SIZE));
	    if (nl2_entries > (1U << (grub_bfs_to_cpu32 (sb->log2_bsize)
				      - LOG_EXTENT_SIZE
				      + DOUBLE_INDIRECT_SHIFT)))
	      nl2_entries = (1 << (grub_bfs_to_cpu32 (sb->log2_bsize)
				   - LOG_EXTENT_SIZE
				   + DOUBLE_INDIRECT_SHIFT));
	    err = read_extent (disk, sb, &l1_entries[l1n], 0, 0,
			       l2_entries, nl2_entries << LOG_EXTENT_SIZE);
	    if (err)
	      {
		grub_free (l1_entries);
		grub_free (l2_entries);
		return err;
	      }
	    last_l1n = l1n;
	  }
	if (l2n > nl2_entries)
	  {
	    grub_free (l1_entries);
	    grub_free (l2_entries);
	    return grub_error (GRUB_ERR_BAD_FS,
			       "incorrect double-indirect block");
	  }

	read_size = (1 << (grub_bfs_to_cpu32 (sb->log2_bsize)
			   + DOUBLE_INDIRECT_SHIFT)) - boff;
	if (read_size > len)
	  read_size = len;
	disk->read_hook = read_hook;
	disk->read_hook_data = read_hook_data;
	err = read_extent (disk, sb, &l2_entries[l2n], 0, boff,
			   buf, read_size);
	disk->read_hook = 0;
	if (err)
	  {
	    grub_free (l1_entries);
	    grub_free (l2_entries);
	    return err;
	  }
	off += read_size;
	len -= read_size;
	buf = (char *) buf + read_size;
      }
    return GRUB_ERR_NONE;
  }
}

static grub_err_t
read_b_node (grub_disk_t disk,
	     const struct grub_bfs_superblock *sb,
	     const struct grub_bfs_inode *ino,
	     grub_uint64_t node_off,
	     struct grub_bfs_btree_node **node,
	     char **key_data, grub_uint16_t **keylen_idx,
	     grub_unaligned_uint64_t **key_values)
{
  void *ret;
  struct grub_bfs_btree_node node_head;
  grub_size_t total_size;
  grub_err_t err;

  *node = NULL;
  *key_data = NULL;
  *keylen_idx = NULL;
  *key_values = NULL;

  err = read_bfs_file (disk, sb, ino, node_off, &node_head, sizeof (node_head),
		       0, 0);
  if (err)
    return err;

  total_size = ALIGN_UP (sizeof (node_head) +
			 grub_bfs_to_cpu_treehead
			 (node_head.total_key_len),
			 BTREE_ALIGN) +
    grub_bfs_to_cpu_treehead (node_head.count_keys) *
    sizeof (grub_uint16_t)
    + grub_bfs_to_cpu_treehead (node_head.count_keys) *
    sizeof (grub_uint64_t);

  ret = grub_malloc (total_size);
  if (!ret)
    return grub_errno;

  err = read_bfs_file (disk, sb, ino, node_off, ret, total_size, 0, 0);
  if (err)
    {
      grub_free (ret);
      return err;
    }

  *node = ret;
  *key_data = (char *) ret + sizeof (node_head);
  *keylen_idx = (grub_uint16_t *) ret
    + ALIGN_UP (sizeof (node_head) +
		grub_bfs_to_cpu_treehead (node_head.total_key_len),
		BTREE_ALIGN) / 2;
  *key_values = (grub_unaligned_uint64_t *)
    (*keylen_idx +
     grub_bfs_to_cpu_treehead (node_head.count_keys));

  return GRUB_ERR_NONE;
}

static int
iterate_in_b_tree (grub_disk_t disk,
		   const struct grub_bfs_superblock *sb,
		   const struct grub_bfs_inode *ino,
		   int (*hook) (const char *name, grub_uint64_t value,
				struct grub_bfs_dir_ctx *ctx),
		   struct grub_bfs_dir_ctx *ctx)
{
  struct grub_bfs_btree_header head;
  grub_err_t err;
  int level;
  grub_uint64_t node_off;

  err = read_bfs_file (disk, sb, ino, 0, &head, sizeof (head), 0, 0);
  if (err)
    return 0;
  node_off = grub_bfs_to_cpu64 (head.root);

  level = grub_bfs_to_cpu32 (head.level) - 1;
  while (level--)
    {
      struct grub_bfs_btree_node node;
      grub_uint64_t key_value;
      err = read_bfs_file (disk, sb, ino, node_off, &node, sizeof (node),
			   0, 0);
      if (err)
	return 0;
      err = read_bfs_file (disk, sb, ino, node_off
			   + ALIGN_UP (sizeof (node) +
				       grub_bfs_to_cpu_treehead (node.
								 total_key_len),
				       BTREE_ALIGN) +
			   grub_bfs_to_cpu_treehead (node.count_keys) *
			   sizeof (grub_uint16_t), &key_value,
			   sizeof (grub_uint64_t), 0, 0);
      if (err)
	return 0;

      node_off = grub_bfs_to_cpu64 (key_value);
    }

  while (1)
    {
      struct grub_bfs_btree_node *node;
      char *key_data;
      grub_uint16_t *keylen_idx;
      grub_unaligned_uint64_t *key_values;
      unsigned i;
      grub_uint16_t start = 0, end = 0;

      err = read_b_node (disk, sb, ino,
			 node_off,
			 &node,
			 &key_data, 
			 &keylen_idx,
			 &key_values);

      if (err)
	return 0;
      
      for (i = 0; i < grub_bfs_to_cpu_treehead (node->count_keys); i++)
	{
	  char c;
	  start = end;
	  end = grub_bfs_to_cpu16 (keylen_idx[i]);
	  if (grub_bfs_to_cpu_treehead (node->total_key_len) <= end)
	    end = grub_bfs_to_cpu_treehead (node->total_key_len);
	  c = key_data[end];
	  key_data[end] = 0;
	  if (hook (key_data + start, grub_bfs_to_cpu64 (key_values[i].val),
		    ctx))
	    {
	      grub_free (node);
	      return 1;
	    }
	    key_data[end] = c;
	  }
	node_off = grub_bfs_to_cpu64 (node->right);
	grub_free (node);
	if (node_off == POINTER_INVALID)
	  return 0;
    }
}

static int
bfs_strcmp (const char *a, const char *b, grub_size_t alen, grub_size_t blen)
{
  char ac, bc;
  while (blen && alen)
    {
      if (*a != *b)
	break;

      a++;
      b++;
      alen--;
      blen--;
    }

  ac = alen ? *a : 0;
  bc = blen ? *b : 0;

#ifdef MODE_AFS
  return (int) (grub_int8_t) ac - (int) (grub_int8_t) bc;
#else
  return (int) (grub_uint8_t) ac - (int) (grub_uint8_t) bc;
#endif
}

static grub_err_t
find_in_b_tree (grub_disk_t disk,
		const struct grub_bfs_superblock *sb,
		const struct grub_bfs_inode *ino, const char *name,
		grub_size_t name_len,
		grub_uint64_t * res)
{
  struct grub_bfs_btree_header head;
  grub_err_t err;
  int level;
  grub_uint64_t node_off;

  err = read_bfs_file (disk, sb, ino, 0, &head, sizeof (head), 0, 0);
  if (err)
    return err;
  node_off = grub_bfs_to_cpu64 (head.root);

  level = grub_bfs_to_cpu32 (head.level) - 1;
  while (1)
    {
      struct grub_bfs_btree_node *node;
      char *key_data;
      grub_uint16_t *keylen_idx;
      grub_unaligned_uint64_t *key_values;
      int lg, j;
      unsigned i;

      err = read_b_node (disk, sb, ino, node_off, &node, &key_data, &keylen_idx, &key_values);
      if (err)
	return err;

      if (node->count_keys == 0)
	{
	  grub_free (node);
	  return grub_error (GRUB_ERR_FILE_NOT_FOUND, N_("file `%s' not found"),
			     name);
	}

      for (lg = 0; grub_bfs_to_cpu_treehead (node->count_keys) >> lg; lg++);

      i = 0;

      for (j = lg - 1; j >= 0; j--)
	{
	  int cmp;
	  grub_uint16_t start = 0, end = 0;
	  if ((i | (1 << j)) >= grub_bfs_to_cpu_treehead (node->count_keys))
	    continue;
	  start = grub_bfs_to_cpu16 (keylen_idx[(i | (1 << j)) - 1]);
	  end = grub_bfs_to_cpu16 (keylen_idx[(i | (1 << j))]);
	  if (grub_bfs_to_cpu_treehead (node->total_key_len) <= end)
	    end = grub_bfs_to_cpu_treehead (node->total_key_len);
	  cmp = bfs_strcmp (key_data + start, name, end - start, name_len);
	  if (cmp == 0 && level == 0)
	    {
	      *res = grub_bfs_to_cpu64 (key_values[i | (1 << j)].val);
	      grub_free (node);
	      return GRUB_ERR_NONE;
	    }
#ifdef MODE_AFS
	  if (cmp <= 0)
#else
	  if (cmp < 0)
#endif
	    i |= (1 << j);
	}
      if (i == 0)
	{
	  grub_uint16_t end = 0;
	  int cmp;
	  end = grub_bfs_to_cpu16 (keylen_idx[0]);
	  if (grub_bfs_to_cpu_treehead (node->total_key_len) <= end)
	    end = grub_bfs_to_cpu_treehead (node->total_key_len);
	  cmp = bfs_strcmp (key_data, name, end, name_len);
	  if (cmp == 0 && level == 0)
	    {
	      *res = grub_bfs_to_cpu64 (key_values[0].val);
	      grub_free (node);
	      return GRUB_ERR_NONE;
	    }
#ifdef MODE_AFS
	  if (cmp > 0 && level != 0)
#else
	    if (cmp >= 0 && level != 0)
#endif
	      {
		node_off = grub_bfs_to_cpu64 (key_values[0].val);
		level--;
		grub_free (node);
		continue;
	      }
	    else if (level != 0
		     && grub_bfs_to_cpu_treehead (node->count_keys) >= 2)
	      {
		node_off = grub_bfs_to_cpu64 (key_values[1].val);
		level--;
		grub_free (node);
		continue;
	      }	      
	  }
	else if (level != 0
		 && i + 1 < grub_bfs_to_cpu_treehead (node->count_keys))
	  {
	    node_off = grub_bfs_to_cpu64 (key_values[i + 1].val);
	    level--;
	    grub_free (node);
	    continue;
	  }
	if (node->overflow != POINTER_INVALID)
	  {
	    node_off = grub_bfs_to_cpu64 (node->overflow);
	    /* This level-- isn't specified but is needed.  */
	    level--;
	    grub_free (node);
	    continue;
	  }
	grub_free (node);
	return grub_error (GRUB_ERR_FILE_NOT_FOUND, N_("file `%s' not found"),
			   name);
    }
}

static grub_err_t
hop_level (grub_disk_t disk,
	   const struct grub_bfs_superblock *sb,
	   struct grub_bfs_inode *ino, const char *name,
	   const char *name_end)
{
  grub_err_t err;
  grub_uint64_t res = 0;

  if (((grub_bfs_to_cpu32 (ino->mode) & ATTR_TYPE) != ATTR_DIR))
    return grub_error (GRUB_ERR_BAD_FILE_TYPE, N_("not a directory"));

  err = find_in_b_tree (disk, sb, ino, name, name_end - name, &res);
  if (err)
    return err;

  return grub_disk_read (disk, res
			 << (grub_bfs_to_cpu32 (sb->log2_bsize)
			     - GRUB_DISK_SECTOR_BITS), 0,
			 sizeof (*ino), (char *) ino);
}

static grub_err_t
find_file (const char *path, grub_disk_t disk,
	   const struct grub_bfs_superblock *sb, struct grub_bfs_inode *ino)
{
  const char *ptr, *next = path;
  char *alloc = NULL;
  char *wptr;
  grub_err_t err;
  struct grub_bfs_inode old_ino;
  unsigned symlinks_max = 32;

  err = read_extent (disk, sb, &sb->root_dir, 0, 0, ino,
		     sizeof (*ino));
  if (err)
    return err;

  while (1)
    {
      ptr = next;
      while (*ptr == '/')
	ptr++;
      if (*ptr == 0)
	{
	  grub_free (alloc);
	  return GRUB_ERR_NONE;
	}
      for (next = ptr; *next && *next != '/'; next++);
      grub_memcpy (&old_ino, ino, sizeof (old_ino));
      err = hop_level (disk, sb, ino, ptr, next);
      if (err)
	return err;

      if (((grub_bfs_to_cpu32 (ino->mode) & ATTR_TYPE) == ATTR_LNK))
	{
	  char *old_alloc = alloc;
	  if (--symlinks_max == 0)
	    {
	      grub_free (alloc);
	      return grub_error (GRUB_ERR_SYMLINK_LOOP,
				 N_("too deep nesting of symlinks"));
	    }

#ifndef MODE_AFS
	  if (grub_bfs_to_cpu32 (ino->flags) & LONG_SYMLINK)
#endif
	    {
	      grub_size_t symsize = grub_bfs_to_cpu64 (ino->size);
	      alloc = grub_malloc (grub_strlen (next)
				   + symsize + 1);
	      if (!alloc)
		{
		  grub_free (alloc);
		  return grub_errno;
		}
	      grub_free (old_alloc);
	      err = read_bfs_file (disk, sb, ino, 0, alloc, symsize, 0, 0);
	      if (err)
		{
		  grub_free (alloc);
		  return err;
		}
	      alloc[symsize] = 0;
	    }
#ifndef MODE_AFS
	  else
	    {
	      alloc = grub_malloc (grub_strlen (next)
				   + sizeof (ino->inplace_link) + 1);
	      if (!alloc)
		{
		  grub_free (alloc);
		  return grub_errno;
		}
	      grub_free (old_alloc);
	      grub_memcpy (alloc, ino->inplace_link,
			   sizeof (ino->inplace_link));
	      alloc[sizeof (ino->inplace_link)] = 0;
	    }
#endif
	  if (alloc[0] == '/')
	    {
	      err = read_extent (disk, sb, &sb->root_dir, 0, 0, ino,
				 sizeof (*ino));
	      if (err)
		{
		  grub_free (alloc);
		  return err;
		}
	    }
	  else
	    grub_memcpy (ino, &old_ino, sizeof (old_ino));
	  wptr = alloc + grub_strlen (alloc);
	  if (next)
	    wptr = grub_stpcpy (wptr, next);
	  *wptr = 0;
	  next = alloc;
	  continue;
	}
    }
}

static grub_err_t
mount (grub_disk_t disk, struct grub_bfs_superblock *sb)
{
  grub_err_t err;
  err = grub_disk_read (disk, SUPERBLOCK, 0, sizeof (*sb), sb);
  if (err == GRUB_ERR_OUT_OF_RANGE)
    return grub_error (GRUB_ERR_BAD_FS, 
#ifdef MODE_AFS
		       "not an AFS filesystem"
#else
		       "not a BFS filesystem"
#endif
		       );
  if (err)
    return err;
  if (sb->magic1 != grub_cpu_to_bfs32_compile_time (SUPER_BLOCK_MAGIC1)
      || sb->magic2 != grub_cpu_to_bfs32_compile_time (SUPER_BLOCK_MAGIC2)
      || sb->magic3 != grub_cpu_to_bfs32_compile_time (SUPER_BLOCK_MAGIC3)
      || sb->bsize == 0
      || (grub_bfs_to_cpu32 (sb->bsize)
	  != (1U << grub_bfs_to_cpu32 (sb->log2_bsize)))
      || grub_bfs_to_cpu32 (sb->log2_bsize) < GRUB_DISK_SECTOR_BITS)
    return grub_error (GRUB_ERR_BAD_FS, 
#ifdef MODE_AFS
		       "not an AFS filesystem"
#else
		       "not a BFS filesystem"
#endif
		       );
  return GRUB_ERR_NONE;
}

/* Helper for grub_bfs_dir.  */
static int
grub_bfs_dir_iter (const char *name, grub_uint64_t value,
		   struct grub_bfs_dir_ctx *ctx)
{
  grub_err_t err2;
  struct grub_bfs_inode ino;
  struct grub_dirhook_info info;

  err2 = grub_disk_read (ctx->device->disk, value
			 << (grub_bfs_to_cpu32 (ctx->sb.log2_bsize)
			     - GRUB_DISK_SECTOR_BITS), 0,
			 sizeof (ino), (char *) &ino);
  if (err2)
    {
      grub_print_error ();
      return 0;
    }

  info.mtimeset = 1;
#ifdef MODE_AFS
  info.mtime =
    grub_divmod64 (grub_bfs_to_cpu64 (ino.mtime), 1000000, 0);
#else
  info.mtime = grub_bfs_to_cpu64 (ino.mtime) >> 16;
#endif
  info.dir = ((grub_bfs_to_cpu32 (ino.mode) & ATTR_TYPE) == ATTR_DIR);
  return ctx->hook (name, &info, ctx->hook_data);
}

static grub_err_t
grub_bfs_dir (grub_device_t device, const char *path,
	      grub_fs_dir_hook_t hook, void *hook_data)
{
  struct grub_bfs_dir_ctx ctx = {
    .device = device,
    .hook = hook,
    .hook_data = hook_data
  };
  grub_err_t err;

  err = mount (device->disk, &ctx.sb);
  if (err)
    return err;

  {
    struct grub_bfs_inode ino;
    err = find_file (path, device->disk, &ctx.sb, &ino);
    if (err)
      return err;
    if (((grub_bfs_to_cpu32 (ino.mode) & ATTR_TYPE) != ATTR_DIR))
      return grub_error (GRUB_ERR_BAD_FILE_TYPE, N_("not a directory"));
    iterate_in_b_tree (device->disk, &ctx.sb, &ino, grub_bfs_dir_iter,
		       &ctx);
  }

  return grub_errno;
}

static grub_err_t
grub_bfs_open (struct grub_file *file, const char *name)
{
  struct grub_bfs_superblock sb;
  grub_err_t err;

  err = mount (file->device->disk, &sb);
  if (err)
    return err;

  {
    struct grub_bfs_inode ino;
    struct grub_bfs_data *data;
    err = find_file (name, file->device->disk, &sb, &ino);
    if (err)
      return err;
    if (((grub_bfs_to_cpu32 (ino.mode) & ATTR_TYPE) != ATTR_REG))
      return grub_error (GRUB_ERR_BAD_FILE_TYPE, N_("not a regular file"));

    data = grub_zalloc (sizeof (struct grub_bfs_data));
    if (!data)
      return grub_errno;
    data->sb = sb;
    grub_memcpy (&data->ino, &ino, sizeof (data->ino));
    file->data = data;
    file->size = grub_bfs_to_cpu64 (ino.size);
  }

  return GRUB_ERR_NONE;
}

static grub_err_t
grub_bfs_close (grub_file_t file)
{
  grub_free (file->data);

  return GRUB_ERR_NONE;
}

static grub_ssize_t
grub_bfs_read (grub_file_t file, char *buf, grub_size_t len)
{
  grub_err_t err;
  struct grub_bfs_data *data = file->data;

  err = read_bfs_file (file->device->disk, &data->sb,
		       &data->ino, file->offset, buf, len,
		       file->read_hook, file->read_hook_data);
  if (err)
    return -1;
  return len;
}

static grub_err_t
grub_bfs_label (grub_device_t device, char **label)
{
  struct grub_bfs_superblock sb;
  grub_err_t err;

  *label = 0;

  err = mount (device->disk, &sb);
  if (err)
    return err;

  *label = grub_strndup (sb.label, sizeof (sb.label));
  return GRUB_ERR_NONE;
}

#ifndef MODE_AFS
static grub_ssize_t
read_bfs_attr (grub_disk_t disk,
	       const struct grub_bfs_superblock *sb,
	       struct grub_bfs_inode *ino,
	       const char *name, void *buf, grub_size_t len)
{
  grub_uint8_t *ptr = (grub_uint8_t *) ino->small_data;
  grub_uint8_t *end = ((grub_uint8_t *) ino + grub_bfs_to_cpu32 (sb->bsize));

  while (ptr + sizeof (struct grub_bfs_small_data_element_header) < end)
    {
      struct grub_bfs_small_data_element_header *el;
      char *el_name;
      grub_uint8_t *data;
      el = (struct grub_bfs_small_data_element_header *) ptr;
      if (el->name_len == 0)
	break;
      el_name = (char *) (el + 1);
      data = (grub_uint8_t *) el_name + grub_bfs_to_cpu16 (el->name_len) + 3;
      ptr = data + grub_bfs_to_cpu16 (el->value_len) + 1;
      if (grub_memcmp (name, el_name, grub_bfs_to_cpu16 (el->name_len)) == 0
	  && name[el->name_len] == 0)
	{
	  grub_size_t copy;
	  copy = len;
	  if (grub_bfs_to_cpu16 (el->value_len) > copy)
	    copy = grub_bfs_to_cpu16 (el->value_len);
	  grub_memcpy (buf, data, copy);
	  return copy;
	}
    }

  if (ino->attr.len != 0)
    {
      grub_size_t read;
      grub_err_t err;
      grub_uint64_t res;

      err = read_extent (disk, sb, &ino->attr, 0, 0, ino,
			 grub_bfs_to_cpu32 (sb->bsize));
      if (err)
	return -1;

      err = find_in_b_tree (disk, sb, ino, name, grub_strlen (name), &res);
      if (err)
	return -1;
      grub_disk_read (disk, res
		      << (grub_bfs_to_cpu32 (sb->log2_bsize)
			  - GRUB_DISK_SECTOR_BITS), 0,
		      grub_bfs_to_cpu32 (sb->bsize), (char *) ino);
      read = grub_bfs_to_cpu64 (ino->size);
      if (read > len)
	read = len;

      err = read_bfs_file (disk, sb, ino, 0, buf, read, 0, 0);
      if (err)
	return -1;
      return read;
    }
  return -1;
}

static grub_err_t
grub_bfs_uuid (grub_device_t device, char **uuid)
{
  struct grub_bfs_superblock sb;
  grub_err_t err;
  struct grub_bfs_inode *ino;
  grub_uint64_t vid;

  *uuid = 0;

  err = mount (device->disk, &sb);
  if (err)
    return err;

  ino = grub_malloc (grub_bfs_to_cpu32 (sb.bsize));
  if (!ino)
    return grub_errno;

  err = read_extent (device->disk, &sb, &sb.root_dir, 0, 0,
		     ino, grub_bfs_to_cpu32 (sb.bsize));
  if (err)
    {
      grub_free (ino);
      return err;
    }
  if (read_bfs_attr (device->disk, &sb, ino, "be:volume_id",
		     &vid, sizeof (vid)) == sizeof (vid))
    *uuid =
      grub_xasprintf ("%016" PRIxGRUB_UINT64_T, grub_bfs_to_cpu64 (vid));

  grub_free (ino);

  return GRUB_ERR_NONE;
}
#endif

static struct grub_fs grub_bfs_fs = {
#ifdef MODE_AFS
  .name = "afs",
#else
  .name = "bfs",
#endif
  .dir = grub_bfs_dir,
  .open = grub_bfs_open,
  .read = grub_bfs_read,
  .close = grub_bfs_close,
  .label = grub_bfs_label,
#ifndef MODE_AFS
  .uuid = grub_bfs_uuid,
#endif
#ifdef GRUB_UTIL
  .reserved_first_sector = 1,
  .blocklist_install = 1,
#endif
};

#ifdef MODE_AFS
GRUB_MOD_INIT (afs)
#else
GRUB_MOD_INIT (bfs)
#endif
{
  COMPILE_TIME_ASSERT (1 << LOG_EXTENT_SIZE ==
		       sizeof (struct grub_bfs_extent));
  grub_fs_register (&grub_bfs_fs);
}

#ifdef MODE_AFS
GRUB_MOD_FINI (afs)
#else
GRUB_MOD_FINI (bfs)
#endif
{
  grub_fs_unregister (&grub_bfs_fs);
}

#ifdef __clang__
#undef memset

void *memset(void *Destination, int Value, int Count)
{
    CHAR8 *Ptr = Destination;
    
    while (Count--)
        *Ptr++ = Value;
    
    return Destination;
}
#endif
