/* xfs.c - XFS.  */
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

GRUB_MOD_LICENSE ("GPLv3+");

#define	XFS_INODE_EXTENTS	9

#define XFS_INODE_FORMAT_INO	1
#define XFS_INODE_FORMAT_EXT	2
#define XFS_INODE_FORMAT_BTREE	3


struct grub_xfs_sblock
{
  grub_uint8_t magic[4];
  grub_uint32_t bsize;
  grub_uint8_t unused1[24];
  grub_uint16_t uuid[8];
  grub_uint8_t unused2[8];
  grub_uint64_t rootino;
  grub_uint8_t unused3[20];
  grub_uint32_t agsize;
  grub_uint8_t unused4[20];
  grub_uint8_t label[12];
  grub_uint8_t log2_bsize;
  grub_uint8_t log2_sect;
  grub_uint8_t log2_inode;
  grub_uint8_t log2_inop;
  grub_uint8_t log2_agblk;
  grub_uint8_t unused6[67];
  grub_uint8_t log2_dirblk;
} GRUB_PACKED;

struct grub_xfs_dir_header
{
  grub_uint8_t count;
  grub_uint8_t smallino;
  union
  {
    grub_uint32_t i4;
    grub_uint64_t i8;
  } GRUB_PACKED parent;
} GRUB_PACKED;

struct grub_xfs_dir_entry
{
  grub_uint8_t len;
  grub_uint16_t offset;
  char name[1];
  /* Inode number follows, 32 bits.  */
} GRUB_PACKED;

struct grub_xfs_dir2_entry
{
  grub_uint64_t inode;
  grub_uint8_t len;
} GRUB_PACKED;

typedef grub_uint32_t grub_xfs_extent[4];

struct grub_xfs_btree_node
{
  grub_uint8_t magic[4];
  grub_uint16_t level;
  grub_uint16_t numrecs;
  grub_uint64_t left;
  grub_uint64_t right;
  grub_uint64_t keys[1];
}  GRUB_PACKED;

struct grub_xfs_btree_root
{
  grub_uint16_t level;
  grub_uint16_t numrecs;
  grub_uint64_t keys[1];
}  GRUB_PACKED;

struct grub_xfs_time
{
  grub_uint32_t sec;
  grub_uint32_t nanosec;
}  GRUB_PACKED;

struct grub_xfs_inode
{
  grub_uint8_t magic[2];
  grub_uint16_t mode;
  grub_uint8_t version;
  grub_uint8_t format;
  grub_uint8_t unused2[26];
  struct grub_xfs_time atime;
  struct grub_xfs_time mtime;
  struct grub_xfs_time ctime;
  grub_uint64_t size;
  grub_uint64_t nblocks;
  grub_uint32_t extsize;
  grub_uint32_t nextents;
  grub_uint16_t unused3;
  grub_uint8_t fork_offset;
  grub_uint8_t unused4[17];
  union
  {
    char raw[156];
    struct dir
    {
      struct grub_xfs_dir_header dirhead;
      struct grub_xfs_dir_entry direntry[1];
    } dir;
    grub_xfs_extent extents[XFS_INODE_EXTENTS];
    struct grub_xfs_btree_root btree;
  } GRUB_PACKED data;
} GRUB_PACKED;

struct grub_xfs_dirblock_tail
{
  grub_uint32_t leaf_count;
  grub_uint32_t leaf_stale;
} GRUB_PACKED;

struct grub_fshelp_node
{
  struct grub_xfs_data *data;
  grub_uint64_t ino;
  int inode_read;
  struct grub_xfs_inode inode;
};

struct grub_xfs_data
{
  struct grub_xfs_sblock sblock;
  grub_disk_t disk;
  int pos;
  int bsize;
  grub_uint32_t agsize;
  struct grub_fshelp_node diropen;
};

static grub_dl_t my_mod;



/* Filetype information as used in inodes.  */
#define FILETYPE_INO_MASK	0170000
#define FILETYPE_INO_REG	0100000
#define FILETYPE_INO_DIRECTORY	0040000
#define FILETYPE_INO_SYMLINK	0120000

static inline int
GRUB_XFS_INO_AGBITS(struct grub_xfs_data *data)
{
  return ((data)->sblock.log2_agblk + (data)->sblock.log2_inop);
}

static inline grub_uint64_t
GRUB_XFS_INO_INOINAG (struct grub_xfs_data *data,
		      grub_uint64_t ino)
{
  return (grub_be_to_cpu64 (ino) & ((1LL << GRUB_XFS_INO_AGBITS (data)) - 1));
}

static inline grub_uint64_t
GRUB_XFS_INO_AG (struct grub_xfs_data *data,
		 grub_uint64_t ino)
{
  return (grub_be_to_cpu64 (ino) >> GRUB_XFS_INO_AGBITS (data));
}

static inline grub_disk_addr_t
GRUB_XFS_FSB_TO_BLOCK (struct grub_xfs_data *data, grub_disk_addr_t fsb)
{
  return ((fsb >> data->sblock.log2_agblk) * data->agsize
	  + (fsb & ((1LL << data->sblock.log2_agblk) - 1)));
}

static inline grub_uint64_t
GRUB_XFS_EXTENT_OFFSET (grub_xfs_extent *exts, int ex)
{
  return ((grub_be_to_cpu32 (exts[ex][0]) & ~(1 << 31)) << 23
	  | grub_be_to_cpu32 (exts[ex][1]) >> 9);
}

static inline grub_uint64_t
GRUB_XFS_EXTENT_BLOCK (grub_xfs_extent *exts, int ex)
{
  return ((grub_uint64_t) (grub_be_to_cpu32 (exts[ex][1])
			   & (0x1ff)) << 43
	  | (grub_uint64_t) grub_be_to_cpu32 (exts[ex][2]) << 11
	  | grub_be_to_cpu32 (exts[ex][3]) >> 21);
}

static inline grub_uint64_t
GRUB_XFS_EXTENT_SIZE (grub_xfs_extent *exts, int ex)
{
  return (grub_be_to_cpu32 (exts[ex][3]) & ((1 << 21) - 1));
}

static inline int
GRUB_XFS_ROUND_TO_DIRENT (int pos)
{
  return ((((pos) + 8 - 1) / 8) * 8);
}

static inline int
GRUB_XFS_NEXT_DIRENT (int pos, int len)
{
  return (pos) + GRUB_XFS_ROUND_TO_DIRENT (8 + 1 + len + 2);
}


static inline grub_uint64_t
grub_xfs_inode_block (struct grub_xfs_data *data,
		      grub_uint64_t ino)
{
  long long int inoinag = GRUB_XFS_INO_INOINAG (data, ino);
  long long ag = GRUB_XFS_INO_AG (data, ino);
  long long block;

  block = (inoinag >> data->sblock.log2_inop) + ag * data->agsize;
  block <<= (data->sblock.log2_bsize - GRUB_DISK_SECTOR_BITS);
  return block;
}


static inline int
grub_xfs_inode_offset (struct grub_xfs_data *data,
		       grub_uint64_t ino)
{
  int inoag = GRUB_XFS_INO_INOINAG (data, ino);
  return ((inoag & ((1 << data->sblock.log2_inop) - 1)) <<
	  data->sblock.log2_inode);
}


static grub_err_t
grub_xfs_read_inode (struct grub_xfs_data *data, grub_uint64_t ino,
		     struct grub_xfs_inode *inode)
{
  grub_uint64_t block = grub_xfs_inode_block (data, ino);
  int offset = grub_xfs_inode_offset (data, ino);

  /* Read the inode.  */
  if (grub_disk_read (data->disk, block, offset,
		      1 << data->sblock.log2_inode, inode))
    return grub_errno;

  if (grub_strncmp ((char *) inode->magic, "IN", 2))
    return grub_error (GRUB_ERR_BAD_FS, "not a correct XFS inode");

  return 0;
}


static grub_disk_addr_t
grub_xfs_read_block (grub_fshelp_node_t node, grub_disk_addr_t fileblock)
{
  struct grub_xfs_btree_node *leaf = 0;
  int ex, nrec;
  grub_xfs_extent *exts;
  grub_uint64_t ret = 0;

  if (node->inode.format == XFS_INODE_FORMAT_BTREE)
    {
      const grub_uint64_t *keys;
      int recoffset;

      leaf = grub_malloc (node->data->bsize);
      if (leaf == 0)
        return 0;

      nrec = grub_be_to_cpu16 (node->inode.data.btree.numrecs);
      keys = &node->inode.data.btree.keys[0];
      if (node->inode.fork_offset)
	recoffset = (node->inode.fork_offset - 1) / 2;
      else
	recoffset = ((1 << node->data->sblock.log2_inode)
		     - ((char *) &node->inode.data.btree.keys
			- (char *) &node->inode))
	  / (2 * sizeof (grub_uint64_t));
      do
        {
          int i;

          for (i = 0; i < nrec; i++)
            {
              if (fileblock < grub_be_to_cpu64 (keys[i]))
                break;
            }

          /* Sparse block.  */
          if (i == 0)
            {
              grub_free (leaf);
              return 0;
            }
          if (grub_disk_read (node->data->disk,
                              GRUB_XFS_FSB_TO_BLOCK (node->data, grub_be_to_cpu64 (keys[i - 1 + recoffset])) << (node->data->sblock.log2_bsize - GRUB_DISK_SECTOR_BITS),
                              0, node->data->bsize, leaf))
            return 0;

          if (grub_strncmp ((char *) leaf->magic, "BMAP", 4))
            {
              grub_free (leaf);
              grub_error (GRUB_ERR_BAD_FS, "not a correct XFS BMAP node");
              return 0;
            }

          nrec = grub_be_to_cpu16 (leaf->numrecs);
          keys = &leaf->keys[0];
	  recoffset = ((node->data->bsize - ((char *) &leaf->keys
					     - (char *) leaf))
		       / (2 * sizeof (grub_uint64_t)));
	}
      while (leaf->level);
      exts = (grub_xfs_extent *) keys;
    }
  else if (node->inode.format == XFS_INODE_FORMAT_EXT)
    {
      nrec = grub_be_to_cpu32 (node->inode.nextents);
      exts = &node->inode.data.extents[0];
    }
  else
    {
      grub_error (GRUB_ERR_NOT_IMPLEMENTED_YET,
		  "XFS does not support inode format %d yet",
		  node->inode.format);
      return 0;
    }

  /* Iterate over each extent to figure out which extent has
     the block we are looking for.  */
  for (ex = 0; ex < nrec; ex++)
    {
      grub_uint64_t start = GRUB_XFS_EXTENT_BLOCK (exts, ex);
      grub_uint64_t offset = GRUB_XFS_EXTENT_OFFSET (exts, ex);
      grub_uint64_t size = GRUB_XFS_EXTENT_SIZE (exts, ex);

      /* Sparse block.  */
      if (fileblock < offset)
        break;
      else if (fileblock < offset + size)
        {
          ret = (fileblock - offset + start);
          break;
        }
    }

  grub_free (leaf);

  return GRUB_XFS_FSB_TO_BLOCK(node->data, ret);
}


/* Read LEN bytes from the file described by DATA starting with byte
   POS.  Return the amount of read bytes in READ.  */
static grub_ssize_t
grub_xfs_read_file (grub_fshelp_node_t node,
		     grub_disk_read_hook_t read_hook, void *read_hook_data,
		     grub_off_t pos, grub_size_t len, char *buf)
{
  return grub_fshelp_read_file (node->data->disk, node,
				read_hook, read_hook_data,
				pos, len, buf, grub_xfs_read_block,
				grub_be_to_cpu64 (node->inode.size),
				node->data->sblock.log2_bsize
				- GRUB_DISK_SECTOR_BITS, 0);
}


static char *
grub_xfs_read_symlink (grub_fshelp_node_t node)
{
  int size = grub_be_to_cpu64 (node->inode.size);

  switch (node->inode.format)
    {
    case XFS_INODE_FORMAT_INO:
      return grub_strndup (node->inode.data.raw, size);

    case XFS_INODE_FORMAT_EXT:
      {
	char *symlink;
	grub_ssize_t numread;

	symlink = grub_malloc (size + 1);
	if (!symlink)
	  return 0;

	numread = grub_xfs_read_file (node, 0, 0, 0, size, symlink);
	if (numread != size)
	  {
	    grub_free (symlink);
	    return 0;
	  }
	symlink[size] = '\0';
	return symlink;
      }
    }

  return 0;
}


static enum grub_fshelp_filetype
grub_xfs_mode_to_filetype (grub_uint16_t mode)
{
  if ((grub_be_to_cpu16 (mode)
       & FILETYPE_INO_MASK) == FILETYPE_INO_DIRECTORY)
    return GRUB_FSHELP_DIR;
  else if ((grub_be_to_cpu16 (mode)
	    & FILETYPE_INO_MASK) == FILETYPE_INO_SYMLINK)
    return GRUB_FSHELP_SYMLINK;
  else if ((grub_be_to_cpu16 (mode)
	    & FILETYPE_INO_MASK) == FILETYPE_INO_REG)
    return GRUB_FSHELP_REG;
  return GRUB_FSHELP_UNKNOWN;
}


/* Context for grub_xfs_iterate_dir.  */
struct grub_xfs_iterate_dir_ctx
{
  grub_fshelp_iterate_dir_hook_t hook;
  void *hook_data;
  struct grub_fshelp_node *diro;
};

/* Helper for grub_xfs_iterate_dir.  */
static int iterate_dir_call_hook (grub_uint64_t ino, const char *filename,
				  struct grub_xfs_iterate_dir_ctx *ctx)
{
  struct grub_fshelp_node *fdiro;
  grub_err_t err;

  fdiro = grub_malloc (sizeof (struct grub_fshelp_node)
		       - sizeof (struct grub_xfs_inode)
		       + (1 << ctx->diro->data->sblock.log2_inode) + 1);
  if (!fdiro)
    {
      grub_print_error ();
      return 0;
    }

  /* The inode should be read, otherwise the filetype can
     not be determined.  */
  fdiro->ino = ino;
  fdiro->inode_read = 1;
  fdiro->data = ctx->diro->data;
  err = grub_xfs_read_inode (ctx->diro->data, ino, &fdiro->inode);
  if (err)
    {
      grub_print_error ();
      return 0;
    }

  return ctx->hook (filename, grub_xfs_mode_to_filetype (fdiro->inode.mode),
		    fdiro, ctx->hook_data);
}

static int
grub_xfs_iterate_dir (grub_fshelp_node_t dir,
		      grub_fshelp_iterate_dir_hook_t hook, void *hook_data)
{
  struct grub_fshelp_node *diro = (struct grub_fshelp_node *) dir;
  struct grub_xfs_iterate_dir_ctx ctx = {
    .hook = hook,
    .hook_data = hook_data,
    .diro = diro
  };

  switch (diro->inode.format)
    {
    case XFS_INODE_FORMAT_INO:
      {
	struct grub_xfs_dir_entry *de = &diro->inode.data.dir.direntry[0];
	int smallino = !diro->inode.data.dir.dirhead.smallino;
	int i;
	grub_uint64_t parent;

	/* If small inode numbers are used to pack the direntry, the
	   parent inode number is small too.  */
	if (smallino)
	  {
	    parent = grub_be_to_cpu32 (diro->inode.data.dir.dirhead.parent.i4);
	    parent = grub_cpu_to_be64 (parent);
	    /* The header is a bit smaller than usual.  */
	    de = (struct grub_xfs_dir_entry *) ((char *) de - 4);
	  }
	else
	  {
	    parent = diro->inode.data.dir.dirhead.parent.i8;
	  }

	/* Synthesize the direntries for `.' and `..'.  */
	if (iterate_dir_call_hook (diro->ino, ".", &ctx))
	  return 1;

	if (iterate_dir_call_hook (parent, "..", &ctx))
	  return 1;

	for (i = 0; i < diro->inode.data.dir.dirhead.count; i++)
	  {
	    grub_uint64_t ino;
	    grub_uint8_t *inopos = (((grub_uint8_t *) de)
			    + sizeof (struct grub_xfs_dir_entry)
			    + de->len - 1);
	    grub_uint8_t c;

	    /* inopos might be unaligned.  */
	    if (smallino)
	      ino = (((grub_uint32_t) inopos[0]) << 24)
		| (((grub_uint32_t) inopos[1]) << 16)
		| (((grub_uint32_t) inopos[2]) << 8)
		| (((grub_uint32_t) inopos[3]) << 0);
	    else
	      ino = (((grub_uint64_t) inopos[0]) << 56)
		| (((grub_uint64_t) inopos[1]) << 48)
		| (((grub_uint64_t) inopos[2]) << 40)
		| (((grub_uint64_t) inopos[3]) << 32)
		| (((grub_uint64_t) inopos[4]) << 24)
		| (((grub_uint64_t) inopos[5]) << 16)
		| (((grub_uint64_t) inopos[6]) << 8)
		| (((grub_uint64_t) inopos[7]) << 0);
	    ino = grub_cpu_to_be64 (ino);

	    c = de->name[de->len];
	    de->name[de->len] = '\0';
	    if (iterate_dir_call_hook (ino, de->name, &ctx))
	      return 1;
	    de->name[de->len] = c;

	    de = ((struct grub_xfs_dir_entry *)
		  (((char *) de)+ sizeof (struct grub_xfs_dir_entry) + de->len
		   + ((smallino ? sizeof (grub_uint32_t)
		       : sizeof (grub_uint64_t))) - 1));
	  }
	break;
      }

    case XFS_INODE_FORMAT_BTREE:
    case XFS_INODE_FORMAT_EXT:
      {
	grub_ssize_t numread;
	char *dirblock;
	grub_uint64_t blk;
        int dirblk_size, dirblk_log2;

        dirblk_log2 = (dir->data->sblock.log2_bsize
                       + dir->data->sblock.log2_dirblk);
        dirblk_size = 1 << dirblk_log2;

	dirblock = grub_malloc (dirblk_size);
	if (! dirblock)
	  return 0;

	/* Iterate over every block the directory has.  */
	for (blk = 0;
	     blk < (grub_be_to_cpu64 (dir->inode.size)
		    >> dirblk_log2);
	     blk++)
	  {
	    /* The header is skipped, the first direntry is stored
	       from byte 16.  */
	    int pos = 16;
	    int entries;
	    int tail_start = (dirblk_size
			      - sizeof (struct grub_xfs_dirblock_tail));

	    struct grub_xfs_dirblock_tail *tail;
	    tail = (struct grub_xfs_dirblock_tail *) &dirblock[tail_start];

	    numread = grub_xfs_read_file (dir, 0, 0,
					  blk << dirblk_log2,
					  dirblk_size, dirblock);
	    if (numread != dirblk_size)
	      return 0;

	    entries = (grub_be_to_cpu32 (tail->leaf_count)
		       - grub_be_to_cpu32 (tail->leaf_stale));

	    /* Iterate over all entries within this block.  */
	    while (pos < (dirblk_size
			  - (int) sizeof (struct grub_xfs_dir2_entry)))
	      {
		struct grub_xfs_dir2_entry *direntry;
		grub_uint8_t *freetag;
		char *filename;

		direntry = (struct grub_xfs_dir2_entry *) &dirblock[pos];
		freetag = (grub_uint8_t *) direntry;

		if (grub_get_unaligned16 (freetag) == 0XFFFF)
		  {
		    grub_uint8_t *skip = (freetag + sizeof (grub_uint16_t));

		    /* This entry is not used, go to the next one.  */
		    pos += grub_be_to_cpu16 (grub_get_unaligned16 (skip));

		    continue;
		  }

		filename = &dirblock[pos + sizeof (*direntry)];
		/* The byte after the filename is for the tag, which
		   is not used by GRUB.  So it can be overwritten.  */
		filename[direntry->len] = '\0';

		if (iterate_dir_call_hook (direntry->inode, filename, &ctx))
		  {
		    grub_free (dirblock);
		    return 1;
		  }

		/* Check if last direntry in this block is
		   reached.  */
		entries--;
		if (!entries)
		  break;

		/* Select the next directory entry.  */
		pos = GRUB_XFS_NEXT_DIRENT (pos, direntry->len);
		pos = GRUB_XFS_ROUND_TO_DIRENT (pos);
	      }
	  }
	grub_free (dirblock);
	break;
      }

    default:
      grub_error (GRUB_ERR_NOT_IMPLEMENTED_YET,
		  "XFS does not support inode format %d yet",
		  diro->inode.format);
    }
  return 0;
}


static struct grub_xfs_data *
grub_xfs_mount (grub_disk_t disk)
{
  struct grub_xfs_data *data = 0;

  data = grub_zalloc (sizeof (struct grub_xfs_data));
  if (!data)
    return 0;

  /* Read the superblock.  */
  if (grub_disk_read (disk, 0, 0,
		      sizeof (struct grub_xfs_sblock), &data->sblock))
    goto fail;

  if (grub_strncmp ((char *) (data->sblock.magic), "XFSB", 4)
      || data->sblock.log2_bsize < GRUB_DISK_SECTOR_BITS
      || ((int) data->sblock.log2_bsize
	  + (int) data->sblock.log2_dirblk) >= 27)
    {
      grub_error (GRUB_ERR_BAD_FS, "not a XFS filesystem");
      goto fail;
    }

  data = grub_realloc (data,
		       sizeof (struct grub_xfs_data)
		       - sizeof (struct grub_xfs_inode)
		       + (1 << data->sblock.log2_inode) + 1);

  if (! data)
    goto fail;

  data->diropen.data = data;
  data->diropen.ino = data->sblock.rootino;
  data->diropen.inode_read = 1;
  data->bsize = grub_be_to_cpu32 (data->sblock.bsize);
  data->agsize = grub_be_to_cpu32 (data->sblock.agsize);

  data->disk = disk;
  data->pos = 0;

  grub_xfs_read_inode (data, data->diropen.ino, &data->diropen.inode);

  return data;
 fail:

  if (grub_errno == GRUB_ERR_OUT_OF_RANGE)
    grub_error (GRUB_ERR_BAD_FS, "not an XFS filesystem");

  grub_free (data);

  return 0;
}


/* Context for grub_xfs_dir.  */
struct grub_xfs_dir_ctx
{
  grub_fs_dir_hook_t hook;
  void *hook_data;
};

/* Helper for grub_xfs_dir.  */
static int
grub_xfs_dir_iter (const char *filename, enum grub_fshelp_filetype filetype,
		   grub_fshelp_node_t node, void *data)
{
  struct grub_xfs_dir_ctx *ctx = data;
  struct grub_dirhook_info info;

  grub_memset (&info, 0, sizeof (info));
  if (node->inode_read)
    {
      info.mtimeset = 1;
      info.mtime = grub_be_to_cpu32 (node->inode.mtime.sec);
    }
  info.dir = ((filetype & GRUB_FSHELP_TYPE_MASK) == GRUB_FSHELP_DIR);
  grub_free (node);
  return ctx->hook (filename, &info, ctx->hook_data);
}

static grub_err_t
grub_xfs_dir (grub_device_t device, const char *path,
	      grub_fs_dir_hook_t hook, void *hook_data)
{
  struct grub_xfs_dir_ctx ctx = { hook, hook_data };
  struct grub_xfs_data *data = 0;
  struct grub_fshelp_node *fdiro = 0;

  grub_dl_ref (my_mod);

  data = grub_xfs_mount (device->disk);
  if (!data)
    goto mount_fail;

  grub_fshelp_find_file (path, &data->diropen, &fdiro, grub_xfs_iterate_dir,
			 grub_xfs_read_symlink, GRUB_FSHELP_DIR);
  if (grub_errno)
    goto fail;

  grub_xfs_iterate_dir (fdiro, grub_xfs_dir_iter, &ctx);

 fail:
  if (fdiro != &data->diropen)
    grub_free (fdiro);
  grub_free (data);

 mount_fail:

  grub_dl_unref (my_mod);

  return grub_errno;
}


/* Open a file named NAME and initialize FILE.  */
static grub_err_t
grub_xfs_open (struct grub_file *file, const char *name)
{
  struct grub_xfs_data *data;
  struct grub_fshelp_node *fdiro = 0;

  grub_dl_ref (my_mod);

  data = grub_xfs_mount (file->device->disk);
  if (!data)
    goto mount_fail;

  grub_fshelp_find_file (name, &data->diropen, &fdiro, grub_xfs_iterate_dir,
			 grub_xfs_read_symlink, GRUB_FSHELP_REG);
  if (grub_errno)
    goto fail;

  if (!fdiro->inode_read)
    {
      grub_xfs_read_inode (data, fdiro->ino, &fdiro->inode);
      if (grub_errno)
	goto fail;
    }

  if (fdiro != &data->diropen)
    {
      grub_memcpy (&data->diropen, fdiro,
		   sizeof (struct grub_fshelp_node)
		   - sizeof (struct grub_xfs_inode)
		   + (1 << data->sblock.log2_inode));
      grub_free (fdiro);
    }

  file->size = grub_be_to_cpu64 (data->diropen.inode.size);
  file->data = data;
  file->offset = 0;

  return 0;

 fail:
  if (fdiro != &data->diropen)
    grub_free (fdiro);
  grub_free (data);

 mount_fail:
  grub_dl_unref (my_mod);

  return grub_errno;
}


static grub_ssize_t
grub_xfs_read (grub_file_t file, char *buf, grub_size_t len)
{
  struct grub_xfs_data *data =
    (struct grub_xfs_data *) file->data;

  return grub_xfs_read_file (&data->diropen,
			     file->read_hook, file->read_hook_data,
			     file->offset, len, buf);
}


static grub_err_t
grub_xfs_close (grub_file_t file)
{
  grub_free (file->data);

  grub_dl_unref (my_mod);

  return GRUB_ERR_NONE;
}


static grub_err_t
grub_xfs_label (grub_device_t device, char **label)
{
  struct grub_xfs_data *data;
  grub_disk_t disk = device->disk;

  grub_dl_ref (my_mod);

  data = grub_xfs_mount (disk);
  if (data)
    *label = grub_strndup ((char *) (data->sblock.label), 12);
  else
    *label = 0;

  grub_dl_unref (my_mod);

  grub_free (data);

  return grub_errno;
}

static grub_err_t
grub_xfs_uuid (grub_device_t device, char **uuid)
{
  struct grub_xfs_data *data;
  grub_disk_t disk = device->disk;

  grub_dl_ref (my_mod);

  data = grub_xfs_mount (disk);
  if (data)
    {
      *uuid = grub_xasprintf ("%04x%04x-%04x-%04x-%04x-%04x%04x%04x",
			     grub_be_to_cpu16 (data->sblock.uuid[0]),
			     grub_be_to_cpu16 (data->sblock.uuid[1]),
			     grub_be_to_cpu16 (data->sblock.uuid[2]),
			     grub_be_to_cpu16 (data->sblock.uuid[3]),
			     grub_be_to_cpu16 (data->sblock.uuid[4]),
			     grub_be_to_cpu16 (data->sblock.uuid[5]),
			     grub_be_to_cpu16 (data->sblock.uuid[6]),
			     grub_be_to_cpu16 (data->sblock.uuid[7]));
    }
  else
    *uuid = NULL;

  grub_dl_unref (my_mod);

  grub_free (data);

  return grub_errno;
}



static struct grub_fs grub_xfs_fs =
  {
    .name = "xfs",
    .dir = grub_xfs_dir,
    .open = grub_xfs_open,
    .read = grub_xfs_read,
    .close = grub_xfs_close,
    .label = grub_xfs_label,
    .uuid = grub_xfs_uuid,
#ifdef GRUB_UTIL
    .reserved_first_sector = 0,
    .blocklist_install = 1,
#endif
    .next = 0
  };

GRUB_MOD_INIT(xfs)
{
  grub_fs_register (&grub_xfs_fs);
  my_mod = mod;
}

GRUB_MOD_FINI(xfs)
{
  grub_fs_unregister (&grub_xfs_fs);
}
