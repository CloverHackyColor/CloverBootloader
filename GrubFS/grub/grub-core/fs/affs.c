/* affs.c - Amiga Fast FileSystem.  */
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

/* The affs bootblock.  */
struct grub_affs_bblock
{
  grub_uint8_t type[3];
  grub_uint8_t flags;
  grub_uint32_t checksum;
  grub_uint32_t rootblock;
} GRUB_PACKED;

/* Set if the filesystem is a AFFS filesystem.  Otherwise this is an
   OFS filesystem.  */
#define GRUB_AFFS_FLAG_FFS	1

/* The affs rootblock.  */
struct grub_affs_rblock
{
  grub_uint32_t type;
  grub_uint8_t unused1[8];
  grub_uint32_t htsize;
  grub_uint32_t unused2;
  grub_uint32_t checksum;
  grub_uint32_t hashtable[1];
} GRUB_PACKED;

struct grub_affs_time
{
  grub_int32_t day;
  grub_uint32_t min;
  grub_uint32_t hz;
} GRUB_PACKED;

/* The second part of a file header block.  */
struct grub_affs_file
{
  grub_uint8_t unused1[12];
  grub_uint32_t size;
  grub_uint8_t unused2[92];
  struct grub_affs_time mtime;
  grub_uint8_t namelen;
  grub_uint8_t name[30];
  grub_uint8_t unused3[5];
  grub_uint32_t hardlink;
  grub_uint32_t unused4[6];
  grub_uint32_t next;
  grub_uint32_t parent;
  grub_uint32_t extension;
  grub_uint32_t type;
} GRUB_PACKED;

/* The location of `struct grub_affs_file' relative to the end of a
   file header block.  */
#define	GRUB_AFFS_FILE_LOCATION		200

/* The offset in both the rootblock and the file header block for the
   hashtable, symlink and block pointers (all synonyms).  */
#define GRUB_AFFS_HASHTABLE_OFFSET	24
#define GRUB_AFFS_BLOCKPTR_OFFSET	24
#define GRUB_AFFS_SYMLINK_OFFSET	24

enum
  {
    GRUB_AFFS_FILETYPE_DIR = 2,
    GRUB_AFFS_FILETYPE_SYMLINK = 3,
    GRUB_AFFS_FILETYPE_HARDLINK = 0xfffffffc,
    GRUB_AFFS_FILETYPE_REG = 0xfffffffd
  };

#define AFFS_MAX_LOG_BLOCK_SIZE 4
#define AFFS_MAX_SUPERBLOCK 1



struct grub_fshelp_node
{
  struct grub_affs_data *data;
  grub_uint32_t block;
  struct grub_fshelp_node *parent;
  struct grub_affs_file di;
  grub_uint32_t *block_cache;
  grub_uint32_t last_block_cache;
};

/* Information about a "mounted" affs filesystem.  */
struct grub_affs_data
{
  struct grub_affs_bblock bblock;
  struct grub_fshelp_node diropen;
  grub_disk_t disk;

  /* Log blocksize in sectors.  */
  int log_blocksize;

  /* The number of entries in the hashtable.  */
  unsigned int htsize;
};

static grub_dl_t my_mod;


static grub_disk_addr_t
grub_affs_read_block (grub_fshelp_node_t node, grub_disk_addr_t fileblock)
{
  grub_uint32_t target, curblock;
  grub_uint32_t pos;
  struct grub_affs_file file;
  struct grub_affs_data *data = node->data;
  grub_uint64_t mod;

  if (!node->block_cache)
    {
      node->block_cache = grub_malloc (((grub_be_to_cpu32 (node->di.size)
					 >> (9 + node->data->log_blocksize))
					/ data->htsize + 2)
				       * sizeof (node->block_cache[0]));
      if (!node->block_cache)
	return -1;
      node->last_block_cache = 0;
      node->block_cache[0] = node->block;
    }

  /* Files are at most 2G on AFFS, so no need for 64-bit division.  */
  target = (grub_uint32_t) fileblock / data->htsize;
  mod = (grub_uint32_t) fileblock % data->htsize;
  /* Find the block that points to the fileblock we are looking up by
     following the chain until the right table is reached.  */
  for (curblock = node->last_block_cache + 1; curblock < target + 1; curblock++)
    {
      grub_disk_read (data->disk,
		      (((grub_uint64_t) node->block_cache[curblock - 1] + 1)
		       << data->log_blocksize) - 1,
		      GRUB_DISK_SECTOR_SIZE - GRUB_AFFS_FILE_LOCATION,
		      sizeof (file), &file);
      if (grub_errno)
	return 0;

      node->block_cache[curblock] = grub_be_to_cpu32 (file.extension);
      node->last_block_cache = curblock;
    }

  /* Translate the fileblock to the block within the right table.  */
  grub_disk_read (data->disk, (grub_uint64_t) node->block_cache[target]
		  << data->log_blocksize,
		  GRUB_AFFS_BLOCKPTR_OFFSET
		  + (data->htsize - mod - 1) * sizeof (pos),
		  sizeof (pos), &pos);
  if (grub_errno)
    return 0;

  return grub_be_to_cpu32 (pos);
}

static struct grub_affs_data *
grub_affs_mount (grub_disk_t disk)
{
  struct grub_affs_data *data;
  grub_uint32_t *rootblock = 0;
  struct grub_affs_rblock *rblock = 0;
  int log_blocksize = 0;
  int bsnum = 0;

  data = grub_zalloc (sizeof (struct grub_affs_data));
  if (!data)
    return 0;

  for (bsnum = 0; bsnum < AFFS_MAX_SUPERBLOCK + 1; bsnum++)
    {
      /* Read the bootblock.  */
      grub_disk_read (disk, bsnum, 0, sizeof (struct grub_affs_bblock),
		      &data->bblock);
      if (grub_errno)
	goto fail;

      /* Make sure this is an affs filesystem.  */
      if (grub_strncmp ((char *) (data->bblock.type), "DOS", 3) != 0
	  /* Test if the filesystem is a OFS filesystem.  */
	  || !(data->bblock.flags & GRUB_AFFS_FLAG_FFS))
	continue;

      /* No sane person uses more than 8KB for a block.  At least I hope
	 for that person because in that case this won't work.  */
      if (!rootblock)
	rootblock = grub_malloc (GRUB_DISK_SECTOR_SIZE
				 << AFFS_MAX_LOG_BLOCK_SIZE);
      if (!rootblock)
	goto fail;

      rblock = (struct grub_affs_rblock *) rootblock;

      /* The filesystem blocksize is not stored anywhere in the filesystem
	 itself.  One way to determine it is try reading blocks for the
	 rootblock until the checksum is correct.  */
      for (log_blocksize = 0; log_blocksize <= AFFS_MAX_LOG_BLOCK_SIZE;
	   log_blocksize++)
	{
	  grub_uint32_t *currblock = rootblock;
	  unsigned int i;
	  grub_uint32_t checksum = 0;

	  /* Read the rootblock.  */
	  grub_disk_read (disk,
			  (grub_uint64_t) grub_be_to_cpu32 (data->bblock.rootblock)
			  << log_blocksize, 0,
			  GRUB_DISK_SECTOR_SIZE << log_blocksize, rootblock);
	  if (grub_errno == GRUB_ERR_OUT_OF_RANGE)
	    {
	      grub_errno = 0;
	      break;
	    }
	  if (grub_errno)
	    goto fail;

	  if (rblock->type != grub_cpu_to_be32_compile_time (2)
	      || rblock->htsize == 0
	      || currblock[(GRUB_DISK_SECTOR_SIZE << log_blocksize)
			   / sizeof (*currblock) - 1]
	      != grub_cpu_to_be32_compile_time (1))
	    continue;

	  for (i = 0; i < (GRUB_DISK_SECTOR_SIZE << log_blocksize)
		 / sizeof (*currblock);
	       i++)
	    checksum += grub_be_to_cpu32 (currblock[i]);

	  if (checksum == 0)
	    {
	      data->log_blocksize = log_blocksize;
	      data->disk = disk;
	      data->htsize = grub_be_to_cpu32 (rblock->htsize);
	      data->diropen.data = data;
	      data->diropen.block = grub_be_to_cpu32 (data->bblock.rootblock);
	      data->diropen.parent = NULL;
	      grub_memcpy (&data->diropen.di, rootblock,
			   sizeof (data->diropen.di));
	      grub_free (rootblock);

	      return data;
	    }
	}
    }

 fail:
  if (grub_errno == GRUB_ERR_NONE || grub_errno == GRUB_ERR_OUT_OF_RANGE)
    grub_error (GRUB_ERR_BAD_FS, "not an AFFS filesystem");

  grub_free (data);
  grub_free (rootblock);
  return 0;
}


static char *
grub_affs_read_symlink (grub_fshelp_node_t node)
{
  struct grub_affs_data *data = node->data;
  grub_uint8_t *latin1, *utf8;
  const grub_size_t symlink_size = ((GRUB_DISK_SECTOR_SIZE
				     << data->log_blocksize) - GRUB_AFFS_SYMLINK_OFFSET);

  latin1 = grub_malloc (symlink_size + 1);
  if (!latin1)
    return 0;

  grub_disk_read (data->disk,
		  (grub_uint64_t) node->block << data->log_blocksize,
		  GRUB_AFFS_SYMLINK_OFFSET,
		  symlink_size, latin1);
  if (grub_errno)
    {
      grub_free (latin1);
      return 0;
    }
  latin1[symlink_size] = 0;
  utf8 = grub_malloc (symlink_size * GRUB_MAX_UTF8_PER_LATIN1 + 1);
  if (!utf8)
    {
      grub_free (latin1);
      return 0;
    }
  *grub_latin1_to_utf8 (utf8, latin1, symlink_size) = '\0';
  grub_dprintf ("affs", "Symlink: `%s'\n", utf8);
  grub_free (latin1);
  if (utf8[0] == ':')
    utf8[0] = '/';
  return (char *) utf8;
}


/* Helper for grub_affs_iterate_dir.  */
static int
grub_affs_create_node (grub_fshelp_node_t dir,
		       grub_fshelp_iterate_dir_hook_t hook, void *hook_data,
		       struct grub_fshelp_node **node,
		       grub_uint32_t **hashtable,
		       grub_uint32_t block, const struct grub_affs_file *fil)
{
  struct grub_affs_data *data = dir->data;
  int type = GRUB_FSHELP_REG;
  grub_uint8_t name_u8[sizeof (fil->name) * GRUB_MAX_UTF8_PER_LATIN1 + 1];
  grub_size_t len;
  unsigned int nest;

  *node = grub_zalloc (sizeof (**node));
  if (!*node)
    {
      grub_free (*hashtable);
      return 1;
    }

  (*node)->data = data;
  (*node)->block = block;
  (*node)->parent = dir;

  len = fil->namelen;
  if (len > sizeof (fil->name))
    len = sizeof (fil->name);
  *grub_latin1_to_utf8 (name_u8, fil->name, len) = '\0';
  
  (*node)->di = *fil;
  for (nest = 0; nest < 8; nest++)
    {
      switch ((*node)->di.type)
	{
	case grub_cpu_to_be32_compile_time (GRUB_AFFS_FILETYPE_REG):
	  type = GRUB_FSHELP_REG;
	  break;
	case grub_cpu_to_be32_compile_time (GRUB_AFFS_FILETYPE_DIR):
	  type = GRUB_FSHELP_DIR;
	  break;
	case grub_cpu_to_be32_compile_time (GRUB_AFFS_FILETYPE_SYMLINK):
	  type = GRUB_FSHELP_SYMLINK;
	  break;
	case grub_cpu_to_be32_compile_time (GRUB_AFFS_FILETYPE_HARDLINK):
	  {
	    grub_err_t err;
	    (*node)->block = grub_be_to_cpu32 ((*node)->di.hardlink);
	    err = grub_disk_read (data->disk,
				  (((grub_uint64_t) (*node)->block + 1) << data->log_blocksize)
				  - 1,
				  GRUB_DISK_SECTOR_SIZE - GRUB_AFFS_FILE_LOCATION,
				  sizeof ((*node)->di), (char *) &(*node)->di);
	    if (err)
	      return 1;
	    continue;
	  }
	default:
	  return 0;
	}
      break;
    }

  if (nest == 8)
    return 0;

  type |= GRUB_FSHELP_CASE_INSENSITIVE;

  if (hook ((char *) name_u8, type, *node, hook_data))
    {
      grub_free (*hashtable);
      *node = 0;
      return 1;
    }
  *node = 0;
  return 0;
}

static int
grub_affs_iterate_dir (grub_fshelp_node_t dir,
		       grub_fshelp_iterate_dir_hook_t hook, void *hook_data)
{
  unsigned int i;
  struct grub_affs_file file;
  struct grub_fshelp_node *node = 0;
  struct grub_affs_data *data = dir->data;
  grub_uint32_t *hashtable;

  /* Create the directory entries for `.' and `..'.  */
  node = grub_zalloc (sizeof (*node));
  if (!node)
    return 1;
    
  *node = *dir;
  if (hook (".", GRUB_FSHELP_DIR, node, hook_data))
    return 1;
  if (dir->parent)
    {
      node = grub_zalloc (sizeof (*node));
      if (!node)
	return 1;
      *node = *dir->parent;
      if (hook ("..", GRUB_FSHELP_DIR, node, hook_data))
	return 1;
    }

  hashtable = grub_zalloc (data->htsize * sizeof (*hashtable));
  if (!hashtable)
    return 1;

  grub_disk_read (data->disk,
		  (grub_uint64_t) dir->block << data->log_blocksize,
		  GRUB_AFFS_HASHTABLE_OFFSET,
		  data->htsize * sizeof (*hashtable), (char *) hashtable);
  if (grub_errno)
    goto fail;

  for (i = 0; i < data->htsize; i++)
    {
      grub_uint32_t next;

      if (!hashtable[i])
	continue;

      /* Every entry in the hashtable can be chained.  Read the entire
	 chain.  */
      next = grub_be_to_cpu32 (hashtable[i]);

      while (next)
	{
	  grub_disk_read (data->disk,
			  (((grub_uint64_t) next + 1) << data->log_blocksize)
			  - 1,
			  GRUB_DISK_SECTOR_SIZE - GRUB_AFFS_FILE_LOCATION,
			  sizeof (file), (char *) &file);
	  if (grub_errno)
	    goto fail;

	  if (grub_affs_create_node (dir, hook, hook_data, &node, &hashtable,
				     next, &file))
	    return 1;

	  next = grub_be_to_cpu32 (file.next);
	}
    }

  grub_free (hashtable);
  return 0;

 fail:
  grub_free (node);
  grub_free (hashtable);
  return 0;
}


/* Open a file named NAME and initialize FILE.  */
static grub_err_t
grub_affs_open (struct grub_file *file, const char *name)
{
  struct grub_affs_data *data;
  struct grub_fshelp_node *fdiro = 0;

  grub_dl_ref (my_mod);

  data = grub_affs_mount (file->device->disk);
  if (!data)
    goto fail;

  grub_fshelp_find_file (name, &data->diropen, &fdiro, grub_affs_iterate_dir,
			 grub_affs_read_symlink, GRUB_FSHELP_REG);
  if (grub_errno)
    goto fail;

  file->size = grub_be_to_cpu32 (fdiro->di.size);
  data->diropen = *fdiro;
  grub_free (fdiro);

  file->data = data;
  file->offset = 0;

  return 0;

 fail:
  if (data && fdiro != &data->diropen)
    grub_free (fdiro);
  grub_free (data);

  grub_dl_unref (my_mod);

  return grub_errno;
}

static grub_err_t
grub_affs_close (grub_file_t file)
{
  struct grub_affs_data *data =
    (struct grub_affs_data *) file->data;

  grub_free (data->diropen.block_cache);
  grub_free (file->data);

  grub_dl_unref (my_mod);

  return GRUB_ERR_NONE;
}

/* Read LEN bytes data from FILE into BUF.  */
static grub_ssize_t
grub_affs_read (grub_file_t file, char *buf, grub_size_t len)
{
  struct grub_affs_data *data =
    (struct grub_affs_data *) file->data;

  return grub_fshelp_read_file (data->diropen.data->disk, &data->diropen,
				file->read_hook, file->read_hook_data,
				file->offset, len, buf, grub_affs_read_block,
				grub_be_to_cpu32 (data->diropen.di.size),
				data->log_blocksize, 0);
}

static grub_int32_t
aftime2ctime (const struct grub_affs_time *t)
{
  return grub_be_to_cpu32 (t->day) * 86400
    + grub_be_to_cpu32 (t->min) * 60
    + grub_be_to_cpu32 (t->hz) / 50
    + 8 * 365 * 86400 + 86400 * 2;
}

/* Context for grub_affs_dir.  */
struct grub_affs_dir_ctx
{
  grub_fs_dir_hook_t hook;
  void *hook_data;
};

/* Helper for grub_affs_dir.  */
static int
grub_affs_dir_iter (const char *filename, enum grub_fshelp_filetype filetype,
		    grub_fshelp_node_t node, void *data)
{
  struct grub_affs_dir_ctx *ctx = data;
  struct grub_dirhook_info info;

  grub_memset (&info, 0, sizeof (info));
  info.dir = ((filetype & GRUB_FSHELP_TYPE_MASK) == GRUB_FSHELP_DIR);
  info.mtimeset = 1;
  info.mtime = aftime2ctime (&node->di.mtime);
  grub_free (node);
  return ctx->hook (filename, &info, ctx->hook_data);
}

static grub_err_t
grub_affs_dir (grub_device_t device, const char *path,
	       grub_fs_dir_hook_t hook, void *hook_data)
{
  struct grub_affs_dir_ctx ctx = { hook, hook_data };
  struct grub_affs_data *data = 0;
  struct grub_fshelp_node *fdiro = 0;

  grub_dl_ref (my_mod);

  data = grub_affs_mount (device->disk);
  if (!data)
    goto fail;

  grub_fshelp_find_file (path, &data->diropen, &fdiro, grub_affs_iterate_dir,
			 grub_affs_read_symlink, GRUB_FSHELP_DIR);
  if (grub_errno)
    goto fail;

  grub_affs_iterate_dir (fdiro, grub_affs_dir_iter, &ctx);

 fail:
  if (data && fdiro != &data->diropen)
    grub_free (fdiro);
  grub_free (data);

  grub_dl_unref (my_mod);

  return grub_errno;
}


static grub_err_t
grub_affs_label (grub_device_t device, char **label)
{
  struct grub_affs_data *data;
  struct grub_affs_file file;
  grub_disk_t disk = device->disk;

  grub_dl_ref (my_mod);

  data = grub_affs_mount (disk);
  if (data)
    {
      grub_size_t len;
      /* The rootblock maps quite well on a file header block, it's
	 something we can use here.  */
      grub_disk_read (data->disk,
		      (((grub_uint64_t)
			grub_be_to_cpu32 (data->bblock.rootblock) + 1)
		       << data->log_blocksize) - 1,
		      GRUB_DISK_SECTOR_SIZE - GRUB_AFFS_FILE_LOCATION,
		      sizeof (file), &file);
      if (grub_errno)
	return grub_errno;

      len = file.namelen;
      if (len > sizeof (file.name))
	len = sizeof (file.name);
      *label = grub_malloc (len * GRUB_MAX_UTF8_PER_LATIN1 + 1);
      if (*label)
	*grub_latin1_to_utf8 ((grub_uint8_t *) *label, file.name, len) = '\0';
    }
  else
    *label = 0;

  grub_dl_unref (my_mod);

  grub_free (data);

  return grub_errno;
}

static grub_err_t
grub_affs_mtime (grub_device_t device, grub_int32_t *t)
{
  struct grub_affs_data *data;
  grub_disk_t disk = device->disk;
  struct grub_affs_time af_time;

  *t = 0;

  grub_dl_ref (my_mod);

  data = grub_affs_mount (disk);
  if (!data)
    {
      grub_dl_unref (my_mod);
      return grub_errno;
    }

  grub_disk_read (data->disk,
		  (((grub_uint64_t)
		    grub_be_to_cpu32 (data->bblock.rootblock) + 1)
		   << data->log_blocksize) - 1,
		  GRUB_DISK_SECTOR_SIZE - 40,
		  sizeof (af_time), &af_time);
  if (grub_errno)
    {
      grub_dl_unref (my_mod);
      grub_free (data);
      return grub_errno;
    }

  *t = aftime2ctime (&af_time);
  grub_dl_unref (my_mod);

  grub_free (data);

  return GRUB_ERR_NONE;
}


static struct grub_fs grub_affs_fs =
  {
    .name = "affs",
    .dir = grub_affs_dir,
    .open = grub_affs_open,
    .read = grub_affs_read,
    .close = grub_affs_close,
    .label = grub_affs_label,
    .mtime = grub_affs_mtime,

#ifdef GRUB_UTIL
    .reserved_first_sector = 0,
    .blocklist_install = 1,
#endif
    .next = 0
  };

GRUB_MOD_INIT(affs)
{
  grub_fs_register (&grub_affs_fs);
  my_mod = mod;
}

GRUB_MOD_FINI(affs)
{
  grub_fs_unregister (&grub_affs_fs);
}
