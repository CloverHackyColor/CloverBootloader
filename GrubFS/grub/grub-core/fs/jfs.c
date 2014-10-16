/* jfs.c - JFS.  */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2004,2005,2006,2007,2008,2009  Free Software Foundation, Inc.
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
#include <grub/charset.h>
#include <grub/i18n.h>

GRUB_MOD_LICENSE ("GPLv3+");

#define GRUB_JFS_MAX_SYMLNK_CNT	8
#define GRUB_JFS_FILETYPE_MASK	0170000
#define GRUB_JFS_FILETYPE_REG	0100000
#define GRUB_JFS_FILETYPE_LNK	0120000
#define GRUB_JFS_FILETYPE_DIR	0040000

#define GRUB_JFS_SBLOCK		64
#define GRUB_JFS_AGGR_INODE	2
#define GRUB_JFS_FS1_INODE_BLK	104

#define GRUB_JFS_TREE_LEAF	2

struct grub_jfs_sblock
{
  /* The magic for JFS.  It should contain the string "JFS1".  */
  grub_uint8_t magic[4];
  grub_uint32_t version;
  grub_uint64_t ag_size;

  /* The size of a filesystem block in bytes.  XXX: currently only
     4096 was tested.  */
  grub_uint32_t blksz;
  grub_uint16_t log2_blksz;
  grub_uint8_t unused[14];
  grub_uint32_t flags;
  grub_uint8_t unused3[61];
  char volname[11];
  grub_uint8_t unused2[24];
  grub_uint8_t uuid[16];
  char volname2[16];
};

struct grub_jfs_extent
{
  /* The length of the extent in filesystem blocks.  */
  grub_uint16_t length;
  grub_uint8_t length2;

  /* The physical offset of the first block on the disk.  */
  grub_uint8_t blk1;
  grub_uint32_t blk2;
} GRUB_PACKED;

#define GRUB_JFS_IAG_INODES_OFFSET 3072
#define GRUB_JFS_IAG_INODES_COUNT 128

struct grub_jfs_iag
{
  grub_uint8_t unused[GRUB_JFS_IAG_INODES_OFFSET];
  struct grub_jfs_extent inodes[GRUB_JFS_IAG_INODES_COUNT];
} GRUB_PACKED;


/* The head of the tree used to find extents.  */
struct grub_jfs_treehead
{
  grub_uint64_t next;
  grub_uint64_t prev;

  grub_uint8_t flags;
  grub_uint8_t unused;

  grub_uint16_t count;
  grub_uint16_t max;
  grub_uint8_t unused2[10];
} GRUB_PACKED;

/* A node in the extent tree.  */
struct grub_jfs_tree_extent
{
  grub_uint8_t flags;
  grub_uint16_t unused;

  /* The offset is the key used to lookup an extent.  */
  grub_uint8_t offset1;
  grub_uint32_t offset2;

  struct grub_jfs_extent extent;
} GRUB_PACKED;

/* The tree of directory entries.  */
struct grub_jfs_tree_dir
{
  /* Pointers to the previous and next tree headers of other nodes on
     this level.  */
  grub_uint64_t nextb;
  grub_uint64_t prevb;

  grub_uint8_t flags;

  /* The amount of dirents in this node.  */
  grub_uint8_t count;
  grub_uint8_t freecnt;
  grub_uint8_t freelist;
  grub_uint8_t maxslot;

  /* The location of the sorted array of pointers to dirents.  */
  grub_uint8_t sindex;
  grub_uint8_t unused[10];
} GRUB_PACKED;

/* An internal node in the dirents tree.  */
struct grub_jfs_internal_dirent
{
  struct grub_jfs_extent ex;
  grub_uint8_t next;
  grub_uint8_t len;
  grub_uint16_t namepart[11];
} GRUB_PACKED;

/* A leaf node in the dirents tree.  */
struct grub_jfs_leaf_dirent
{
  /* The inode for this dirent.  */
  grub_uint32_t inode;
  grub_uint8_t next;

  /* The size of the name.  */
  grub_uint8_t len;
  grub_uint16_t namepart[11];
  grub_uint32_t index;
} GRUB_PACKED;

/* A leaf in the dirents tree.  This one is used if the previously
   dirent was not big enough to store the name.  */
struct grub_jfs_leaf_next_dirent
{
  grub_uint8_t next;
  grub_uint8_t len;
  grub_uint16_t namepart[15];
} GRUB_PACKED;

struct grub_jfs_time
{
  grub_int32_t sec;
  grub_int32_t nanosec;
} GRUB_PACKED;

struct grub_jfs_inode
{
  grub_uint32_t stamp;
  grub_uint32_t fileset;
  grub_uint32_t inode;
  grub_uint8_t unused[12];
  grub_uint64_t size;
  grub_uint8_t unused2[20];
  grub_uint32_t mode;
  struct grub_jfs_time atime;
  struct grub_jfs_time ctime;
  struct grub_jfs_time mtime;
  grub_uint8_t unused3[48];
  grub_uint8_t unused4[96];

  union
  {
    /* The tree describing the extents of the file.  */
    struct GRUB_PACKED
    {
      struct grub_jfs_treehead tree;
      struct grub_jfs_tree_extent extents[16];
    } file;
    union
    {
      /* The tree describing the dirents.  */
      struct
      {
	grub_uint8_t unused[16];
	grub_uint8_t flags;

	/* Amount of dirents in this node.  */
	grub_uint8_t count;
	grub_uint8_t freecnt;
	grub_uint8_t freelist;
	grub_uint32_t idotdot;
	grub_uint8_t sorted[8];
      } header;
      struct grub_jfs_leaf_dirent dirents[8];
    } GRUB_PACKED dir;
    /* Fast symlink.  */
    struct
    {
      grub_uint8_t unused[32];
      grub_uint8_t path[256];
    } symlink;
  } GRUB_PACKED;
} GRUB_PACKED;

struct grub_jfs_data
{
  struct grub_jfs_sblock sblock;
  grub_disk_t disk;
  struct grub_jfs_inode fileset;
  struct grub_jfs_inode currinode;
  int caseins;
  int pos;
  int linknest;
  int namecomponentlen;
} GRUB_PACKED;

struct grub_jfs_diropen
{
  int index;
  union
  {
    struct grub_jfs_tree_dir header;
    struct grub_jfs_leaf_dirent dirent[0];
    struct grub_jfs_leaf_next_dirent next_dirent[0];
    grub_uint8_t sorted[0];
  } GRUB_PACKED *dirpage;
  struct grub_jfs_data *data;
  struct grub_jfs_inode *inode;
  int count;
  grub_uint8_t *sorted;
  struct grub_jfs_leaf_dirent *leaf;
  struct grub_jfs_leaf_next_dirent *next_leaf;

  /* The filename and inode of the last read dirent.  */
  /* On-disk name is at most 255 UTF-16 codepoints.
     Every UTF-16 codepoint is at most 4 UTF-8 bytes.
   */
  char name[256 * GRUB_MAX_UTF8_PER_UTF16 + 1];
  grub_uint32_t ino;
} GRUB_PACKED;


static grub_dl_t my_mod;

static grub_err_t grub_jfs_lookup_symlink (struct grub_jfs_data *data, grub_uint32_t ino);

static grub_int64_t
getblk (struct grub_jfs_treehead *treehead,
	struct grub_jfs_tree_extent *extents,
	struct grub_jfs_data *data,
	grub_uint64_t blk)
{
  int found = -1;
  int i;

  for (i = 0; i < grub_le_to_cpu16 (treehead->count) - 2; i++)
    {
      if (treehead->flags & GRUB_JFS_TREE_LEAF)
	{
	  /* Read the leafnode.  */
	  if (grub_le_to_cpu32 (extents[i].offset2) <= blk
	      && ((grub_le_to_cpu16 (extents[i].extent.length))
		  + (extents[i].extent.length2 << 16)
		  + grub_le_to_cpu32 (extents[i].offset2)) > blk)
	    return (blk - grub_le_to_cpu32 (extents[i].offset2)
		    + grub_le_to_cpu32 (extents[i].extent.blk2));
	}
      else
	if (blk >= grub_le_to_cpu32 (extents[i].offset2))
	  found = i;
    }

  if (found != -1)
    {
      grub_int64_t ret = -1;
      struct
      {
	struct grub_jfs_treehead treehead;
	struct grub_jfs_tree_extent extents[254];
      } *tree;

      tree = grub_zalloc (sizeof (*tree));
      if (!tree)
	return -1;

      if (!grub_disk_read (data->disk,
			   ((grub_disk_addr_t) grub_le_to_cpu32 (extents[found].extent.blk2))
			   << (grub_le_to_cpu16 (data->sblock.log2_blksz)
			       - GRUB_DISK_SECTOR_BITS), 0,
			   sizeof (*tree), (char *) tree))
	ret = getblk (&tree->treehead, &tree->extents[0], data, blk);
      grub_free (tree);
      return ret;
    }

  return -1;
}

/* Get the block number for the block BLK in the node INODE in the
   mounted filesystem DATA.  */
static grub_int64_t
grub_jfs_blkno (struct grub_jfs_data *data, struct grub_jfs_inode *inode,
		grub_uint64_t blk)
{
  return getblk (&inode->file.tree, &inode->file.extents[0], data, blk);
}


static grub_err_t
grub_jfs_read_inode (struct grub_jfs_data *data, grub_uint32_t ino,
		     struct grub_jfs_inode *inode)
{
  struct grub_jfs_extent iag_inodes[GRUB_JFS_IAG_INODES_COUNT];
  grub_uint32_t iagnum = ino / 4096;
  unsigned inoext = (ino % 4096) / 32;
  unsigned inonum = (ino % 4096) % 32;
  grub_uint64_t iagblk;
  grub_uint64_t inoblk;

  iagblk = grub_jfs_blkno (data, &data->fileset, iagnum + 1);
  if (grub_errno)
    return grub_errno;

  /* Read in the IAG.  */
  if (grub_disk_read (data->disk,
		      iagblk << (grub_le_to_cpu16 (data->sblock.log2_blksz)
				 - GRUB_DISK_SECTOR_BITS),
		      GRUB_JFS_IAG_INODES_OFFSET,
		      sizeof (iag_inodes), &iag_inodes))
    return grub_errno;

  inoblk = grub_le_to_cpu32 (iag_inodes[inoext].blk2);
  inoblk <<= (grub_le_to_cpu16 (data->sblock.log2_blksz)
	      - GRUB_DISK_SECTOR_BITS);
  inoblk += inonum;

  if (grub_disk_read (data->disk, inoblk, 0,
		      sizeof (struct grub_jfs_inode), inode))
    return grub_errno;

  return 0;
}


static struct grub_jfs_data *
grub_jfs_mount (grub_disk_t disk)
{
  struct grub_jfs_data *data = 0;

  data = grub_malloc (sizeof (struct grub_jfs_data));
  if (!data)
    return 0;

  /* Read the superblock.  */
  if (grub_disk_read (disk, GRUB_JFS_SBLOCK, 0,
		      sizeof (struct grub_jfs_sblock), &data->sblock))
    goto fail;

  if (grub_strncmp ((char *) (data->sblock.magic), "JFS1", 4))
    {
      grub_error (GRUB_ERR_BAD_FS, "not a JFS filesystem");
      goto fail;
    }

  if (data->sblock.blksz == 0
      || grub_le_to_cpu32 (data->sblock.blksz)
      != (1U << grub_le_to_cpu16 (data->sblock.log2_blksz))
      || grub_le_to_cpu16 (data->sblock.log2_blksz) < GRUB_DISK_SECTOR_BITS)
    {
      grub_error (GRUB_ERR_BAD_FS, "not a JFS filesystem");
      goto fail;
    }

  data->disk = disk;
  data->pos = 0;
  data->linknest = 0;

  /* Read the inode of the first fileset.  */
  if (grub_disk_read (data->disk, GRUB_JFS_FS1_INODE_BLK, 0,
		      sizeof (struct grub_jfs_inode), &data->fileset))
    goto fail;

  if (data->sblock.flags & grub_cpu_to_le32_compile_time (0x00200000))
    data->namecomponentlen = 11;
  else
    data->namecomponentlen = 13;

  if (data->sblock.flags & grub_cpu_to_le32_compile_time (0x40000000))
    data->caseins = 1;
  else
    data->caseins = 0;

  return data;

 fail:
  grub_free (data);

  if (grub_errno == GRUB_ERR_OUT_OF_RANGE)
    grub_error (GRUB_ERR_BAD_FS, "not a JFS filesystem");

  return 0;
}


static struct grub_jfs_diropen *
grub_jfs_opendir (struct grub_jfs_data *data, struct grub_jfs_inode *inode)
{
  struct grub_jfs_internal_dirent *de;
  struct grub_jfs_diropen *diro;
  grub_disk_addr_t blk;

  de = (struct grub_jfs_internal_dirent *) inode->dir.dirents;

  if (!((grub_le_to_cpu32 (inode->mode)
	 & GRUB_JFS_FILETYPE_MASK) == GRUB_JFS_FILETYPE_DIR))
    {
      grub_error (GRUB_ERR_BAD_FILE_TYPE, N_("not a directory"));
      return 0;
    }

  diro = grub_zalloc (sizeof (struct grub_jfs_diropen));
  if (!diro)
    return 0;

  diro->data = data;
  diro->inode = inode;

  /* Check if the entire tree is contained within the inode.  */
  if (inode->file.tree.flags & GRUB_JFS_TREE_LEAF)
    {
      diro->leaf = inode->dir.dirents;
      diro->next_leaf = (struct grub_jfs_leaf_next_dirent *) de;
      diro->sorted = inode->dir.header.sorted;
      diro->count = inode->dir.header.count;

      return diro;
    }

  diro->dirpage = grub_malloc (grub_le_to_cpu32 (data->sblock.blksz));
  if (!diro->dirpage)
    {
      grub_free (diro);
      return 0;
    }

  blk = grub_le_to_cpu32 (de[inode->dir.header.sorted[0]].ex.blk2);
  blk <<= (grub_le_to_cpu16 (data->sblock.log2_blksz) - GRUB_DISK_SECTOR_BITS);

  /* Read in the nodes until we are on the leaf node level.  */
  do
    {
      int index;
      if (grub_disk_read (data->disk, blk, 0,
			  grub_le_to_cpu32 (data->sblock.blksz),
			  diro->dirpage->sorted))
	{
	  grub_free (diro->dirpage);
	  grub_free (diro);
	  return 0;
	}

      de = (struct grub_jfs_internal_dirent *) diro->dirpage->dirent;
      index = diro->dirpage->sorted[diro->dirpage->header.sindex * 32];
      blk = (grub_le_to_cpu32 (de[index].ex.blk2)
	     << (grub_le_to_cpu16 (data->sblock.log2_blksz)
		 - GRUB_DISK_SECTOR_BITS));
    } while (!(diro->dirpage->header.flags & GRUB_JFS_TREE_LEAF));

  diro->leaf = diro->dirpage->dirent;
  diro->next_leaf = diro->dirpage->next_dirent;
  diro->sorted = &diro->dirpage->sorted[diro->dirpage->header.sindex * 32];
  diro->count = diro->dirpage->header.count;

  return diro;
}


static void
grub_jfs_closedir (struct grub_jfs_diropen *diro)
{
  if (!diro)
    return;
  grub_free (diro->dirpage);
  grub_free (diro);
}

static void
le_to_cpu16_copy (grub_uint16_t *out, grub_uint16_t *in, grub_size_t len)
{
  while (len--)
    *out++ = grub_le_to_cpu16 (*in++);
}


/* Read in the next dirent from the directory described by DIRO.  */
static grub_err_t
grub_jfs_getent (struct grub_jfs_diropen *diro)
{
  int strpos = 0;
  struct grub_jfs_leaf_dirent *leaf;
  struct grub_jfs_leaf_next_dirent *next_leaf;
  int len;
  int nextent;
  grub_uint16_t filename[256];

  /* The last node, read in more.  */
  if (diro->index == diro->count)
    {
      grub_disk_addr_t next;

      /* If the inode contains the entry tree or if this was the last
	 node, there is nothing to read.  */
      if ((diro->inode->file.tree.flags & GRUB_JFS_TREE_LEAF)
	  || !grub_le_to_cpu64 (diro->dirpage->header.nextb))
	return GRUB_ERR_OUT_OF_RANGE;

      next = grub_le_to_cpu64 (diro->dirpage->header.nextb);
      next <<= (grub_le_to_cpu16 (diro->data->sblock.log2_blksz)
		- GRUB_DISK_SECTOR_BITS);

      if (grub_disk_read (diro->data->disk, next, 0,
			  grub_le_to_cpu32 (diro->data->sblock.blksz),
			  diro->dirpage->sorted))
	return grub_errno;

      diro->leaf = diro->dirpage->dirent;
      diro->next_leaf = diro->dirpage->next_dirent;
      diro->sorted = &diro->dirpage->sorted[diro->dirpage->header.sindex * 32];
      diro->count = diro->dirpage->header.count;
      diro->index = 0;
    }

  leaf = &diro->leaf[diro->sorted[diro->index]];
  next_leaf = &diro->next_leaf[diro->index];

  len = leaf->len;
  if (!len)
    {
      diro->index++;
      return grub_jfs_getent (diro);
    }

  le_to_cpu16_copy (filename + strpos, leaf->namepart, len < diro->data->namecomponentlen ? len
		    : diro->data->namecomponentlen);
  strpos += len < diro->data->namecomponentlen ? len
    : diro->data->namecomponentlen;
  diro->ino = grub_le_to_cpu32 (leaf->inode);
  len -= diro->data->namecomponentlen;

  /* Move down to the leaf level.  */
  nextent = leaf->next;
  if (leaf->next != 255)
    do
      {
 	next_leaf = &diro->next_leaf[nextent];
	le_to_cpu16_copy (filename + strpos, next_leaf->namepart, len < 15 ? len : 15);
	strpos += len < 15 ? len : 15;

	len -= 15;
	nextent = next_leaf->next;
      } while (next_leaf->next != 255 && len > 0);

  diro->index++;

  /* Convert the temporary UTF16 filename to UTF8.  */
  *grub_utf16_to_utf8 ((grub_uint8_t *) (diro->name), filename, strpos) = '\0';

  return 0;
}


/* Read LEN bytes from the file described by DATA starting with byte
   POS.  Return the amount of read bytes in READ.  */
static grub_ssize_t
grub_jfs_read_file (struct grub_jfs_data *data,
		    grub_disk_read_hook_t read_hook, void *read_hook_data,
		    grub_off_t pos, grub_size_t len, char *buf)
{
  grub_off_t i;
  grub_off_t blockcnt;

  blockcnt = (len + pos + grub_le_to_cpu32 (data->sblock.blksz) - 1)
    >> grub_le_to_cpu16 (data->sblock.log2_blksz);

  for (i = pos >> grub_le_to_cpu16 (data->sblock.log2_blksz); i < blockcnt; i++)
    {
      grub_disk_addr_t blknr;
      grub_uint32_t blockoff = pos & (grub_le_to_cpu32 (data->sblock.blksz) - 1);
      grub_uint32_t blockend = grub_le_to_cpu32 (data->sblock.blksz);

      grub_uint64_t skipfirst = 0;

      blknr = grub_jfs_blkno (data, &data->currinode, i);
      if (grub_errno)
	return -1;

      /* Last block.  */
      if (i == blockcnt - 1)
	{
	  blockend = (len + pos) & (grub_le_to_cpu32 (data->sblock.blksz) - 1);

	  if (!blockend)
	    blockend = grub_le_to_cpu32 (data->sblock.blksz);
	}

      /* First block.  */
      if (i == (pos >> grub_le_to_cpu16 (data->sblock.log2_blksz)))
	{
	  skipfirst = blockoff;
	  blockend -= skipfirst;
	}

      data->disk->read_hook = read_hook;
      data->disk->read_hook_data = read_hook_data;
      grub_disk_read (data->disk,
		      blknr << (grub_le_to_cpu16 (data->sblock.log2_blksz)
				- GRUB_DISK_SECTOR_BITS),
		      skipfirst, blockend, buf);

      data->disk->read_hook = 0;
      if (grub_errno)
	return -1;

      buf += grub_le_to_cpu32 (data->sblock.blksz) - skipfirst;
    }

  return len;
}


/* Find the file with the pathname PATH on the filesystem described by
   DATA.  */
static grub_err_t
grub_jfs_find_file (struct grub_jfs_data *data, const char *path,
		    grub_uint32_t start_ino)
{
  const char *name;
  const char *next = path;
  struct grub_jfs_diropen *diro = NULL;

  if (grub_jfs_read_inode (data, start_ino, &data->currinode))
    return grub_errno;

  while (1)
    {
      name = next;
      while (*name == '/')
	name++;
      if (name[0] == 0)
	return GRUB_ERR_NONE;
      for (next = name; *next && *next != '/'; next++);

      if (name[0] == '.' && name + 1 == next)
	continue;

      if (name[0] == '.' && name[1] == '.' && name + 2 == next)
	{
	  grub_uint32_t ino = grub_le_to_cpu32 (data->currinode.dir.header.idotdot);

	  if (grub_jfs_read_inode (data, ino, &data->currinode))
	    return grub_errno;

	  continue;
	}

      diro = grub_jfs_opendir (data, &data->currinode);
      if (!diro)
	return grub_errno;

      for (;;)
	{
	  if (grub_jfs_getent (diro) == GRUB_ERR_OUT_OF_RANGE)
	    {
	      grub_jfs_closedir (diro);
	      return grub_error (GRUB_ERR_FILE_NOT_FOUND, N_("file `%s' not found"), path);
	    }

	  /* Check if the current direntry matches the current part of the
	     pathname.  */
	  if ((data->caseins ? grub_strncasecmp (name, diro->name, next - name) == 0
	       : grub_strncmp (name, diro->name, next - name) == 0) && !diro->name[next - name])
	    {
	      grub_uint32_t ino = diro->ino;
	      grub_uint32_t dirino = grub_le_to_cpu32 (data->currinode.inode);

	      grub_jfs_closedir (diro);
	      diro = 0;

	      if (grub_jfs_read_inode (data, ino, &data->currinode))
		break;

	      /* Check if this is a symlink.  */
	      if ((grub_le_to_cpu32 (data->currinode.mode)
		   & GRUB_JFS_FILETYPE_MASK) == GRUB_JFS_FILETYPE_LNK)
		{
		  grub_jfs_lookup_symlink (data, dirino);
		  if (grub_errno)
		    return grub_errno;
		}

	      break;
	    }
	}
    }
}


static grub_err_t
grub_jfs_lookup_symlink (struct grub_jfs_data *data, grub_uint32_t ino)
{
  grub_size_t size = grub_le_to_cpu64 (data->currinode.size);
  char *symlink;

  if (++data->linknest > GRUB_JFS_MAX_SYMLNK_CNT)
    return grub_error (GRUB_ERR_SYMLINK_LOOP, N_("too deep nesting of symlinks"));

  symlink = grub_malloc (size + 1);
  if (!symlink)
    return grub_errno;
  if (size <= sizeof (data->currinode.symlink.path))
    grub_memcpy (symlink, (char *) (data->currinode.symlink.path), size);
  else if (grub_jfs_read_file (data, 0, 0, 0, size, symlink) < 0)
    {
      grub_free (symlink);
      return grub_errno;
    }

  symlink[size] = '\0';

  /* The symlink is an absolute path, go back to the root inode.  */
  if (symlink[0] == '/')
    ino = 2;

  grub_jfs_find_file (data, symlink, ino);

  grub_free (symlink);

  return grub_errno;
}


static grub_err_t
grub_jfs_dir (grub_device_t device, const char *path,
	      grub_fs_dir_hook_t hook, void *hook_data)
{
  struct grub_jfs_data *data = 0;
  struct grub_jfs_diropen *diro = 0;

  grub_dl_ref (my_mod);

  data = grub_jfs_mount (device->disk);
  if (!data)
    goto fail;

  if (grub_jfs_find_file (data, path, GRUB_JFS_AGGR_INODE))
    goto fail;

  diro = grub_jfs_opendir (data, &data->currinode);
  if (!diro)
    goto fail;

  /* Iterate over the dirents in the directory that was found.  */
  while (grub_jfs_getent (diro) != GRUB_ERR_OUT_OF_RANGE)
    {
      struct grub_jfs_inode inode;
      struct grub_dirhook_info info;
      grub_memset (&info, 0, sizeof (info));

      if (grub_jfs_read_inode (data, diro->ino, &inode))
	goto fail;

      info.dir = (grub_le_to_cpu32 (inode.mode)
		  & GRUB_JFS_FILETYPE_MASK) == GRUB_JFS_FILETYPE_DIR;
      info.mtimeset = 1;
      info.mtime = grub_le_to_cpu32 (inode.mtime.sec);
      if (hook (diro->name, &info, hook_data))
	goto fail;
    }

  /* XXX: GRUB_ERR_OUT_OF_RANGE is used for the last dirent.  */
  if (grub_errno == GRUB_ERR_OUT_OF_RANGE)
    grub_errno = 0;

 fail:
  grub_jfs_closedir (diro);
  grub_free (data);

  grub_dl_unref (my_mod);

  return grub_errno;
}


/* Open a file named NAME and initialize FILE.  */
static grub_err_t
grub_jfs_open (struct grub_file *file, const char *name)
{
  struct grub_jfs_data *data;

  grub_dl_ref (my_mod);

  data = grub_jfs_mount (file->device->disk);
  if (!data)
    goto fail;

  grub_jfs_find_file (data, name, GRUB_JFS_AGGR_INODE);
  if (grub_errno)
    goto fail;

  /* It is only possible for open regular files.  */
  if (! ((grub_le_to_cpu32 (data->currinode.mode)
	  & GRUB_JFS_FILETYPE_MASK) == GRUB_JFS_FILETYPE_REG))
    {
      grub_error (GRUB_ERR_BAD_FILE_TYPE, N_("not a regular file"));
      goto fail;
    }

  file->data = data;
  file->size = grub_le_to_cpu64 (data->currinode.size);

  return 0;

 fail:

  grub_dl_unref (my_mod);

  grub_free (data);

  return grub_errno;
}


static grub_ssize_t
grub_jfs_read (grub_file_t file, char *buf, grub_size_t len)
{
  struct grub_jfs_data *data =
    (struct grub_jfs_data *) file->data;

  return grub_jfs_read_file (data, file->read_hook, file->read_hook_data,
			     file->offset, len, buf);
}


static grub_err_t
grub_jfs_close (grub_file_t file)
{
  grub_free (file->data);

  grub_dl_unref (my_mod);

  return GRUB_ERR_NONE;
}

static grub_err_t
grub_jfs_uuid (grub_device_t device, char **uuid)
{
  struct grub_jfs_data *data;
  grub_disk_t disk = device->disk;

  grub_dl_ref (my_mod);

  data = grub_jfs_mount (disk);
  if (data)
    {
      *uuid = grub_xasprintf ("%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-"
			     "%02x%02x%02x%02x%02x%02x",
			     data->sblock.uuid[0], data->sblock.uuid[1],
			     data->sblock.uuid[2], data->sblock.uuid[3],
			     data->sblock.uuid[4], data->sblock.uuid[5],
			     data->sblock.uuid[6], data->sblock.uuid[7],
			     data->sblock.uuid[8], data->sblock.uuid[9],
			     data->sblock.uuid[10], data->sblock.uuid[11],
			     data->sblock.uuid[12], data->sblock.uuid[13],
			     data->sblock.uuid[14], data->sblock.uuid[15]);
    }
  else
    *uuid = NULL;

  grub_dl_unref (my_mod);

  grub_free (data);

  return grub_errno;
}

static grub_err_t
grub_jfs_label (grub_device_t device, char **label)
{
  struct grub_jfs_data *data;
  data = grub_jfs_mount (device->disk);

  if (data)
    {
      if (data->sblock.volname2[0] < ' ')
	{
	  char *ptr;
	  ptr = data->sblock.volname + sizeof (data->sblock.volname) - 1;
	  while (ptr >= data->sblock.volname && *ptr == ' ')
	    ptr--;
	  *label = grub_strndup (data->sblock.volname,
				 ptr - data->sblock.volname + 1);
	}
      else
	*label = grub_strndup (data->sblock.volname2,
			       sizeof (data->sblock.volname2));
    }
  else
    *label = 0;

  grub_free (data);

  return grub_errno;
}


static struct grub_fs grub_jfs_fs =
  {
    .name = "jfs",
    .dir = grub_jfs_dir,
    .open = grub_jfs_open,
    .read = grub_jfs_read,
    .close = grub_jfs_close,
    .label = grub_jfs_label,
    .uuid = grub_jfs_uuid,
#ifdef GRUB_UTIL
    .reserved_first_sector = 1,
    .blocklist_install = 1,
#endif
    .next = 0
  };

GRUB_MOD_INIT(jfs)
{
  grub_fs_register (&grub_jfs_fs);
  my_mod = mod;
}

GRUB_MOD_FINI(jfs)
{
  grub_fs_unregister (&grub_jfs_fs);
}
