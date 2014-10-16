/* sfs.c - Amiga Smart FileSystem.  */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2005,2006,2007,2008,2009  Free Software Foundation, Inc.
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
#include <grub/fshelp.h>
#include <grub/charset.h>

GRUB_MOD_LICENSE ("GPLv3+");

/* The common header for a block.  */
struct grub_sfs_bheader
{
  grub_uint8_t magic[4];
  grub_uint32_t chksum;
  grub_uint32_t ipointtomyself;
} GRUB_PACKED;

/* The sfs rootblock.  */
struct grub_sfs_rblock
{
  struct grub_sfs_bheader header;
  grub_uint32_t version;
  grub_uint32_t createtime;
  grub_uint8_t flags;
  grub_uint8_t unused1[31];
  grub_uint32_t blocksize;
  grub_uint8_t unused2[40];
  grub_uint8_t unused3[8];
  grub_uint32_t rootobject;
  grub_uint32_t btree;
} GRUB_PACKED;

enum
  {
    FLAGS_CASE_SENSITIVE = 0x80
  };

/* A SFS object container.  */
struct grub_sfs_obj
{
  grub_uint8_t unused1[4];
  grub_uint32_t nodeid;
  grub_uint8_t unused2[4];
  union
  {
    struct
    {
      grub_uint32_t first_block;
      grub_uint32_t size;
    } GRUB_PACKED file;
    struct
    {
      grub_uint32_t hashtable;
      grub_uint32_t dir_objc;
    } GRUB_PACKED dir;
  } file_dir;
  grub_uint32_t mtime;
  grub_uint8_t type;
  grub_uint8_t filename[1];
  grub_uint8_t comment[1];
} GRUB_PACKED;

#define	GRUB_SFS_TYPE_DELETED	32
#define	GRUB_SFS_TYPE_SYMLINK	64
#define	GRUB_SFS_TYPE_DIR	128

/* A SFS object container.  */
struct grub_sfs_objc
{
  struct grub_sfs_bheader header;
  grub_uint32_t parent;
  grub_uint32_t next;
  grub_uint32_t prev;
  /* The amount of objects depends on the blocksize.  */
  struct grub_sfs_obj objects[1];
} GRUB_PACKED;

struct grub_sfs_btree_node
{
  grub_uint32_t key;
  grub_uint32_t data;
} GRUB_PACKED;

struct grub_sfs_btree_extent
{
  grub_uint32_t key;
  grub_uint32_t next;
  grub_uint32_t prev;
  grub_uint16_t size;
} GRUB_PACKED;

struct grub_sfs_btree
{
  struct grub_sfs_bheader header;
  grub_uint16_t nodes;
  grub_uint8_t leaf;
  grub_uint8_t nodesize;
  /* Normally this can be kind of node, but just extents are
     supported.  */
  struct grub_sfs_btree_node node[1];
} GRUB_PACKED;



struct cache_entry
{
  grub_uint32_t off;
  grub_uint32_t block;
};

struct grub_fshelp_node
{
  struct grub_sfs_data *data;
  grub_uint32_t block;
  grub_uint32_t size;
  grub_uint32_t mtime;
  grub_uint32_t cache_off;
  grub_uint32_t next_extent;
  grub_size_t cache_allocated;
  grub_size_t cache_size;
  struct cache_entry *cache;
};

/* Information about a "mounted" sfs filesystem.  */
struct grub_sfs_data
{
  struct grub_sfs_rblock rblock;
  struct grub_fshelp_node diropen;
  grub_disk_t disk;

  /* Log of blocksize in sectors.  */
  int log_blocksize;

  int fshelp_flags;

  /* Label of the filesystem.  */
  char *label;
};

static grub_dl_t my_mod;


/* Lookup the extent starting with BLOCK in the filesystem described
   by DATA.  Return the extent size in SIZE and the following extent
   in NEXTEXT.  */
static grub_err_t
grub_sfs_read_extent (struct grub_sfs_data *data, unsigned int block,
		      grub_uint32_t *size, grub_uint32_t *nextext)
{
  char *treeblock;
  struct grub_sfs_btree *tree;
  int i;
  grub_uint32_t next;

  treeblock = grub_malloc (GRUB_DISK_SECTOR_SIZE << data->log_blocksize);
  if (!block)
    return 0;

  next = grub_be_to_cpu32 (data->rblock.btree);
  tree = (struct grub_sfs_btree *) treeblock;

  /* Handle this level in the btree.  */
  do
    {
      grub_disk_read (data->disk,
		      ((grub_disk_addr_t) next) << data->log_blocksize,
		      0, GRUB_DISK_SECTOR_SIZE << data->log_blocksize,
		      treeblock);
      if (grub_errno)
	{
	  grub_free (treeblock);
	  return grub_errno;
	}

      for (i = grub_be_to_cpu16 (tree->nodes) - 1; i >= 0; i--)
	{

#define EXTNODE(tree, index)						\
	((struct grub_sfs_btree_node *) (((char *) &(tree)->node[0])	\
					 + (index) * (tree)->nodesize))

	  /* Follow the tree down to the leaf level.  */
	  if ((grub_be_to_cpu32 (EXTNODE(tree, i)->key) <= block)
	      && !tree->leaf)
	    {
	      next = grub_be_to_cpu32 (EXTNODE (tree, i)->data);
	      break;
	    }

	  /* If the leaf level is reached, just find the correct extent.  */
	  if (grub_be_to_cpu32 (EXTNODE (tree, i)->key) == block && tree->leaf)
	    {
	      struct grub_sfs_btree_extent *extent;
	      extent = (struct grub_sfs_btree_extent *) EXTNODE (tree, i);

	      /* We found a correct leaf.  */
	      *size = grub_be_to_cpu16 (extent->size);
	      *nextext = grub_be_to_cpu32 (extent->next);

	      grub_free (treeblock);
	      return 0;
	    }

#undef EXTNODE

	}
    } while (!tree->leaf);

  grub_free (treeblock);

  return grub_error (GRUB_ERR_FILE_READ_ERROR, "SFS extent not found");
}

static grub_disk_addr_t
grub_sfs_read_block (grub_fshelp_node_t node, grub_disk_addr_t fileblock)
{
  grub_uint32_t blk;
  grub_uint32_t size = 0;
  grub_uint32_t next = 0;
  grub_disk_addr_t off;
  struct grub_sfs_data *data = node->data;

  /* In case of the first block we don't have to lookup the
     extent, the minimum size is always 1.  */
  if (fileblock == 0)
    return node->block;

  if (!node->cache)
    {
      grub_size_t cache_size;
      /* Assume half-max extents (32768 sectors).  */
      cache_size = ((node->size >> (data->log_blocksize + GRUB_DISK_SECTOR_BITS
				    + 15))
		    + 3);
      if (cache_size < 8)
	cache_size = 8;

      node->cache_off = 0;
      node->next_extent = node->block;
      node->cache_size = 0;

      node->cache = grub_malloc (sizeof (node->cache[0]) * cache_size);
      if (!node->cache)
	{
	  grub_errno = 0;
	  node->cache_allocated = 0;
	}
      else
	{
	  node->cache_allocated = cache_size;
	  node->cache[0].off = 0;
	  node->cache[0].block = node->block;
	}
    }

  if (fileblock < node->cache_off)
    {
      unsigned int i = 0;
      int j, lg;
      for (lg = 0; node->cache_size >> lg; lg++);

      for (j = lg - 1; j >= 0; j--)
	if ((i | (1 << j)) < node->cache_size
	    && node->cache[(i | (1 << j))].off <= fileblock)
	  i |= (1 << j);
      return node->cache[i].block + fileblock - node->cache[i].off;
    }

  off = node->cache_off;
  blk = node->next_extent;

  while (blk)
    {
      grub_err_t err;

      err = grub_sfs_read_extent (node->data, blk, &size, &next);
      if (err)
	return 0;

      if (node->cache && node->cache_size >= node->cache_allocated)
	{
	  struct cache_entry *e = node->cache;
	  e = grub_realloc (node->cache,node->cache_allocated * 2
			    * sizeof (e[0]));
	  if (!e)
	    {
	      grub_errno = 0;
	      grub_free (node->cache);
	      node->cache = 0;
	    }
	  else
	    {
	      node->cache_allocated *= 2;
	      node->cache = e;
	    }
	}

      if (node->cache)
	{
	  node->cache_off = off + size;
	  node->next_extent = next;
	  node->cache[node->cache_size].off = off;
	  node->cache[node->cache_size].block = blk;
	  node->cache_size++;
	}

      if (fileblock - off < size)
	return fileblock - off + blk;

      off += size;

      blk = next;
    }

  grub_error (GRUB_ERR_FILE_READ_ERROR,
	      "reading a SFS block outside the extent");

  return 0;
}


/* Read LEN bytes from the file described by DATA starting with byte
   POS.  Return the amount of read bytes in READ.  */
static grub_ssize_t
grub_sfs_read_file (grub_fshelp_node_t node,
		    grub_disk_read_hook_t read_hook, void *read_hook_data,
		    grub_off_t pos, grub_size_t len, char *buf)
{
  return grub_fshelp_read_file (node->data->disk, node,
				read_hook, read_hook_data,
				pos, len, buf, grub_sfs_read_block,
				node->size, node->data->log_blocksize, 0);
}


static struct grub_sfs_data *
grub_sfs_mount (grub_disk_t disk)
{
  struct grub_sfs_data *data;
  struct grub_sfs_objc *rootobjc;
  char *rootobjc_data = 0;
  grub_uint32_t blk;

  data = grub_malloc (sizeof (*data));
  if (!data)
    return 0;

  /* Read the rootblock.  */
  grub_disk_read (disk, 0, 0, sizeof (struct grub_sfs_rblock),
		  &data->rblock);
  if (grub_errno)
    goto fail;

  /* Make sure this is a sfs filesystem.  */
  if (grub_strncmp ((char *) (data->rblock.header.magic), "SFS", 4)
      || data->rblock.blocksize == 0
      || (data->rblock.blocksize & (data->rblock.blocksize - 1)) != 0
      || (data->rblock.blocksize & grub_cpu_to_be32_compile_time (0xf00001ff)))
    {
      grub_error (GRUB_ERR_BAD_FS, "not a SFS filesystem");
      goto fail;
    }

  for (data->log_blocksize = 9;
       (1U << data->log_blocksize) < grub_be_to_cpu32 (data->rblock.blocksize);
       data->log_blocksize++);
  data->log_blocksize -= GRUB_DISK_SECTOR_BITS;
  if (data->rblock.flags & FLAGS_CASE_SENSITIVE)
    data->fshelp_flags = 0;
  else
    data->fshelp_flags = GRUB_FSHELP_CASE_INSENSITIVE;
  rootobjc_data = grub_malloc (GRUB_DISK_SECTOR_SIZE << data->log_blocksize);
  if (! rootobjc_data)
    goto fail;

  /* Read the root object container.  */
  grub_disk_read (disk, ((grub_disk_addr_t) grub_be_to_cpu32 (data->rblock.rootobject))
		  << data->log_blocksize, 0,
		  GRUB_DISK_SECTOR_SIZE << data->log_blocksize, rootobjc_data);
  if (grub_errno)
    goto fail;

  rootobjc = (struct grub_sfs_objc *) rootobjc_data;

  blk = grub_be_to_cpu32 (rootobjc->objects[0].file_dir.dir.dir_objc);
  data->diropen.size = 0;
  data->diropen.block = blk;
  data->diropen.data = data;
  data->diropen.cache = 0;
  data->disk = disk;
  data->label = grub_strdup ((char *) (rootobjc->objects[0].filename));

  grub_free (rootobjc_data);
  return data;

 fail:
  if (grub_errno == GRUB_ERR_OUT_OF_RANGE)
    grub_error (GRUB_ERR_BAD_FS, "not an SFS filesystem");

  grub_free (data);
  grub_free (rootobjc_data);
  return 0;
}


static char *
grub_sfs_read_symlink (grub_fshelp_node_t node)
{
  struct grub_sfs_data *data = node->data;
  char *symlink;
  char *block;

  block = grub_malloc (GRUB_DISK_SECTOR_SIZE << data->log_blocksize);
  if (!block)
    return 0;

  grub_disk_read (data->disk, ((grub_disk_addr_t) node->block)
		  << data->log_blocksize,
		  0, GRUB_DISK_SECTOR_SIZE << data->log_blocksize, block);
  if (grub_errno)
    {
      grub_free (block);
      return 0;
    }

  /* This is just a wild guess, but it always worked for me.  How the
     SLNK block looks like is not documented in the SFS docs.  */
  symlink = grub_malloc (((GRUB_DISK_SECTOR_SIZE << data->log_blocksize)
			  - 24) * GRUB_MAX_UTF8_PER_LATIN1 + 1);
  if (!symlink)
    {
      grub_free (block);
      return 0;
    }
  *grub_latin1_to_utf8 ((grub_uint8_t *) symlink, (grub_uint8_t *) &block[24],
			(GRUB_DISK_SECTOR_SIZE << data->log_blocksize) - 24) = '\0';
  grub_free (block);
  return symlink;
}

/* Helper for grub_sfs_iterate_dir.  */
static int
grub_sfs_create_node (struct grub_fshelp_node **node,
		      struct grub_sfs_data *data,
		      const char *name,
		      grub_uint32_t block, grub_uint32_t size, int type,
		      grub_uint32_t mtime,
		      grub_fshelp_iterate_dir_hook_t hook, void *hook_data)
{
  grub_size_t len = grub_strlen (name);
  grub_uint8_t *name_u8;
  int ret;
  *node = grub_malloc (sizeof (**node));
  if (!*node)
    return 1;
  name_u8 = grub_malloc (len * GRUB_MAX_UTF8_PER_LATIN1 + 1);
  if (!name_u8)
    {
      grub_free (*node);
      return 1;
    }

  (*node)->data = data;
  (*node)->size = size;
  (*node)->block = block;
  (*node)->mtime = mtime;
  (*node)->cache = 0;
  (*node)->cache_off = 0;
  (*node)->next_extent = block;
  (*node)->cache_size = 0;
  (*node)->cache_allocated = 0;

  *grub_latin1_to_utf8 (name_u8, (const grub_uint8_t *) name, len) = '\0';

  ret = hook ((char *) name_u8, type | data->fshelp_flags, *node, hook_data);
  grub_free (name_u8);
  return ret;
}

static int
grub_sfs_iterate_dir (grub_fshelp_node_t dir,
		      grub_fshelp_iterate_dir_hook_t hook, void *hook_data)
{
  struct grub_fshelp_node *node = 0;
  struct grub_sfs_data *data = dir->data;
  char *objc_data;
  struct grub_sfs_objc *objc;
  unsigned int next = dir->block;
  grub_uint32_t pos;

  objc_data = grub_malloc (GRUB_DISK_SECTOR_SIZE << data->log_blocksize);
  if (!objc_data)
    goto fail;

  /* The Object container can consist of multiple blocks, iterate over
     every block.  */
  while (next)
    {
      grub_disk_read (data->disk, ((grub_disk_addr_t) next)
		      << data->log_blocksize, 0,
		      GRUB_DISK_SECTOR_SIZE << data->log_blocksize, objc_data);
      if (grub_errno)
	goto fail;

      objc = (struct grub_sfs_objc *) objc_data;

      pos = (char *) &objc->objects[0] - (char *) objc;

      /* Iterate over all entries in this block.  */
      while (pos + sizeof (struct grub_sfs_obj)
	     < (1U << (GRUB_DISK_SECTOR_BITS + data->log_blocksize)))
	{
	  struct grub_sfs_obj *obj;
	  obj = (struct grub_sfs_obj *) ((char *) objc + pos);
	  const char *filename = (const char *) obj->filename;
	  grub_size_t len;
	  enum grub_fshelp_filetype type;
	  grub_uint32_t block;

	  /* The filename and comment dynamically increase the size of
	     the object.  */
	  len = grub_strlen (filename);
	  len += grub_strlen (filename + len + 1);

	  pos += sizeof (*obj) + len;
	  /* Round up to a multiple of two bytes.  */
	  pos = ((pos + 1) >> 1) << 1;

	  if (filename[0] == 0)
	    continue;

	  /* First check if the file was not deleted.  */
	  if (obj->type & GRUB_SFS_TYPE_DELETED)
	    continue;
	  else if (obj->type & GRUB_SFS_TYPE_SYMLINK)
	    type = GRUB_FSHELP_SYMLINK;
	  else if (obj->type & GRUB_SFS_TYPE_DIR)
	    type = GRUB_FSHELP_DIR;
	  else
	    type = GRUB_FSHELP_REG;

	  if (type == GRUB_FSHELP_DIR)
	    block = grub_be_to_cpu32 (obj->file_dir.dir.dir_objc);
	  else
	    block = grub_be_to_cpu32 (obj->file_dir.file.first_block);

	  if (grub_sfs_create_node (&node, data, filename, block,
				    grub_be_to_cpu32 (obj->file_dir.file.size),
				    type, grub_be_to_cpu32 (obj->mtime),
				    hook, hook_data))
	    {
	      grub_free (objc_data);
	      return 1;
	    }
	}

      next = grub_be_to_cpu32 (objc->next);
    }

 fail:
  grub_free (objc_data);
  return 0;
}


/* Open a file named NAME and initialize FILE.  */
static grub_err_t
grub_sfs_open (struct grub_file *file, const char *name)
{
  struct grub_sfs_data *data;
  struct grub_fshelp_node *fdiro = 0;

  grub_dl_ref (my_mod);

  data = grub_sfs_mount (file->device->disk);
  if (!data)
    goto fail;

  grub_fshelp_find_file (name, &data->diropen, &fdiro, grub_sfs_iterate_dir,
			 grub_sfs_read_symlink, GRUB_FSHELP_REG);
  if (grub_errno)
    goto fail;

  file->size = fdiro->size;
  data->diropen = *fdiro;
  grub_free (fdiro);

  file->data = data;
  file->offset = 0;

  return 0;

 fail:
  if (data && fdiro != &data->diropen)
    grub_free (fdiro);
  if (data)
    grub_free (data->label);
  grub_free (data);

  grub_dl_unref (my_mod);

  return grub_errno;
}


static grub_err_t
grub_sfs_close (grub_file_t file)
{
  struct grub_sfs_data *data = (struct grub_sfs_data *) file->data;

  grub_free (data->diropen.cache);
  grub_free (data->label);
  grub_free (data);

  grub_dl_unref (my_mod);

  return GRUB_ERR_NONE;
}


/* Read LEN bytes data from FILE into BUF.  */
static grub_ssize_t
grub_sfs_read (grub_file_t file, char *buf, grub_size_t len)
{
  struct grub_sfs_data *data = (struct grub_sfs_data *) file->data;

  return grub_sfs_read_file (&data->diropen,
			     file->read_hook, file->read_hook_data,
			     file->offset, len, buf);
}


/* Context for grub_sfs_dir.  */
struct grub_sfs_dir_ctx
{
  grub_fs_dir_hook_t hook;
  void *hook_data;
};

/* Helper for grub_sfs_dir.  */
static int
grub_sfs_dir_iter (const char *filename, enum grub_fshelp_filetype filetype,
		   grub_fshelp_node_t node, void *data)
{
  struct grub_sfs_dir_ctx *ctx = data;
  struct grub_dirhook_info info;

  grub_memset (&info, 0, sizeof (info));
  info.dir = ((filetype & GRUB_FSHELP_TYPE_MASK) == GRUB_FSHELP_DIR);
  info.mtime = node->mtime + 8 * 365 * 86400 + 86400 * 2;
  info.mtimeset = 1;
  grub_free (node->cache);
  grub_free (node);
  return ctx->hook (filename, &info, ctx->hook_data);
}

static grub_err_t
grub_sfs_dir (grub_device_t device, const char *path,
	      grub_fs_dir_hook_t hook, void *hook_data)
{
  struct grub_sfs_dir_ctx ctx = { hook, hook_data };
  struct grub_sfs_data *data = 0;
  struct grub_fshelp_node *fdiro = 0;

  grub_dl_ref (my_mod);

  data = grub_sfs_mount (device->disk);
  if (!data)
    goto fail;

  grub_fshelp_find_file (path, &data->diropen, &fdiro, grub_sfs_iterate_dir,
			grub_sfs_read_symlink, GRUB_FSHELP_DIR);
  if (grub_errno)
    goto fail;

  grub_sfs_iterate_dir (fdiro, grub_sfs_dir_iter, &ctx);

 fail:
  if (data && fdiro != &data->diropen)
    grub_free (fdiro);
  if (data)
    grub_free (data->label);
  grub_free (data);

  grub_dl_unref (my_mod);

  return grub_errno;
}


static grub_err_t
grub_sfs_label (grub_device_t device, char **label)
{
  struct grub_sfs_data *data;
  grub_disk_t disk = device->disk;

  data = grub_sfs_mount (disk);
  if (data)
    {
      grub_size_t len = grub_strlen (data->label);
      *label = grub_malloc (len * GRUB_MAX_UTF8_PER_LATIN1 + 1);
      if (*label)
	*grub_latin1_to_utf8 ((grub_uint8_t *) *label,
			      (const grub_uint8_t *) data->label,
			      len) = '\0';
      grub_free (data->label);
    }
  grub_free (data);

  return grub_errno;
}


static struct grub_fs grub_sfs_fs =
  {
    .name = "sfs",
    .dir = grub_sfs_dir,
    .open = grub_sfs_open,
    .read = grub_sfs_read,
    .close = grub_sfs_close,
    .label = grub_sfs_label,
#ifdef GRUB_UTIL
    .reserved_first_sector = 0,
    .blocklist_install = 1,
#endif
    .next = 0
  };

GRUB_MOD_INIT(sfs)
{
  grub_fs_register (&grub_sfs_fs);
  my_mod = mod;
}

GRUB_MOD_FINI(sfs)
{
  grub_fs_unregister (&grub_sfs_fs);
}
