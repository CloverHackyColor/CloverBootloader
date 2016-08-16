/* ufs.c - Unix File System */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2004,2005,2007,2008,2009  Free Software Foundation, Inc.
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
#include <grub/i18n.h>

GRUB_MOD_LICENSE ("GPLv3+");

#ifdef MODE_UFS2
#define GRUB_UFS_MAGIC		0x19540119
#else
#define GRUB_UFS_MAGIC		0x11954
#endif

#define GRUB_UFS_INODE		2
#define GRUB_UFS_FILETYPE_DIR	4
#define GRUB_UFS_FILETYPE_LNK	10
#define GRUB_UFS_MAX_SYMLNK_CNT	8

#define GRUB_UFS_DIRBLKS	12
#define GRUB_UFS_INDIRBLKS	3

#define GRUB_UFS_ATTR_TYPE      0160000
#define GRUB_UFS_ATTR_FILE	0100000
#define GRUB_UFS_ATTR_DIR	0040000
#define GRUB_UFS_ATTR_LNK       0120000

#define GRUB_UFS_VOLNAME_LEN	32

#ifdef MODE_BIGENDIAN
#define grub_ufs_to_cpu16 grub_be_to_cpu16
#define grub_ufs_to_cpu32 grub_be_to_cpu32
#define grub_ufs_to_cpu64 grub_be_to_cpu64
#define grub_cpu_to_ufs32_compile_time grub_cpu_to_be32_compile_time
#else
#define grub_ufs_to_cpu16 grub_le_to_cpu16
#define grub_ufs_to_cpu32 grub_le_to_cpu32
#define grub_ufs_to_cpu64 grub_le_to_cpu64
#define grub_cpu_to_ufs32_compile_time grub_cpu_to_le32_compile_time
#endif

#ifdef MODE_UFS2
typedef grub_uint64_t grub_ufs_blk_t;
static inline grub_disk_addr_t
grub_ufs_to_cpu_blk (grub_ufs_blk_t blk)
{
  return grub_ufs_to_cpu64 (blk);
}
#else
typedef grub_uint32_t grub_ufs_blk_t;
static inline grub_disk_addr_t
grub_ufs_to_cpu_blk (grub_ufs_blk_t blk)
{
  return grub_ufs_to_cpu32 (blk);
}
#endif

/* Calculate in which group the inode can be found.  */
#define UFS_BLKSZ(sblock) (grub_ufs_to_cpu32 (sblock->bsize))
#define UFS_LOG_BLKSZ(sblock) (data->log2_blksz)

#ifdef MODE_UFS2
#define INODE_ENDIAN(data,field,bits1,bits2) grub_ufs_to_cpu##bits2 (data->inode.field)
#else
#define INODE_ENDIAN(data,field,bits1,bits2) grub_ufs_to_cpu##bits1 (data->inode.field)
#endif

#define INODE_SIZE(data) grub_ufs_to_cpu64 (data->inode.size)
#define INODE_MODE(data) grub_ufs_to_cpu16 (data->inode.mode)
#ifdef MODE_UFS2
#define LOG_INODE_BLKSZ 3
#else
#define LOG_INODE_BLKSZ 2
#endif
#ifdef MODE_UFS2
#define UFS_INODE_PER_BLOCK 2
#else
#define UFS_INODE_PER_BLOCK 4
#endif
#define INODE_DIRBLOCKS(data,blk) INODE_ENDIAN \
                                   (data,blocks.dir_blocks[blk],32,64)
#define INODE_INDIRBLOCKS(data,blk) INODE_ENDIAN \
                                     (data,blocks.indir_blocks[blk],32,64)

/* The blocks on which the superblock can be found.  */
static int sblocklist[] = { 128, 16, 0, 512, -1 };

struct grub_ufs_sblock
{
  grub_uint8_t unused[16];
  /* The offset of the inodes in the cylinder group.  */
  grub_uint32_t inoblk_offs;

  grub_uint8_t unused2[4];

  /* The start of the cylinder group.  */
  grub_uint32_t cylg_offset;
  grub_uint32_t cylg_mask;

  grub_uint32_t mtime;
  grub_uint8_t unused4[12];

  /* The size of a block in bytes.  */
  grub_int32_t bsize;
  grub_uint8_t unused5[48];

  /* The size of filesystem blocks to disk blocks.  */
  grub_uint32_t log2_blksz;
  grub_uint8_t unused6[40];
  grub_uint32_t uuidhi;
  grub_uint32_t uuidlow;
  grub_uint8_t unused7[32];

  /* Inodes stored per cylinder group.  */
  grub_uint32_t ino_per_group;

  /* The frags per cylinder group.  */
  grub_uint32_t frags_per_group;

  grub_uint8_t unused8[488];

  /* Volume name for UFS2.  */
  grub_uint8_t volume_name[GRUB_UFS_VOLNAME_LEN];
  grub_uint8_t unused9[360];

  grub_uint64_t mtime2;
  grub_uint8_t unused10[292];

  /* Magic value to check if this is really a UFS filesystem.  */
  grub_uint32_t magic;
};

#ifdef MODE_UFS2
/* UFS inode.  */
struct grub_ufs_inode
{
  grub_uint16_t mode;
  grub_uint16_t nlinks;
  grub_uint32_t uid;
  grub_uint32_t gid;
  grub_uint32_t blocksize;
  grub_uint64_t size;
  grub_int64_t nblocks;
  grub_uint64_t atime;
  grub_uint64_t mtime;
  grub_uint64_t ctime;
  grub_uint64_t create_time;
  grub_uint32_t atime_usec;
  grub_uint32_t mtime_usec;
  grub_uint32_t ctime_usec;
  grub_uint32_t create_time_sec;
  grub_uint32_t gen;
  grub_uint32_t kernel_flags;
  grub_uint32_t flags;
  grub_uint32_t extsz;
  grub_uint64_t ext[2];
  union
  {
    struct
    {
      grub_uint64_t dir_blocks[GRUB_UFS_DIRBLKS];
      grub_uint64_t indir_blocks[GRUB_UFS_INDIRBLKS];
    } blocks;
    grub_uint8_t symlink[(GRUB_UFS_DIRBLKS + GRUB_UFS_INDIRBLKS) * 8];
  };

  grub_uint8_t unused[24];
} GRUB_PACKED;
#else
/* UFS inode.  */
struct grub_ufs_inode
{
  grub_uint16_t mode;
  grub_uint16_t nlinks;
  grub_uint16_t uid;
  grub_uint16_t gid;
  grub_uint64_t size;
  grub_uint32_t atime;
  grub_uint32_t atime_usec;
  grub_uint32_t mtime;
  grub_uint32_t mtime_usec;
  grub_uint32_t ctime;
  grub_uint32_t ctime_usec;
  union
  {
    struct
    {
      grub_uint32_t dir_blocks[GRUB_UFS_DIRBLKS];
      grub_uint32_t indir_blocks[GRUB_UFS_INDIRBLKS];
    } blocks;
    grub_uint8_t symlink[(GRUB_UFS_DIRBLKS + GRUB_UFS_INDIRBLKS) * 4];
  };
  grub_uint32_t flags;
  grub_uint32_t nblocks;
  grub_uint32_t gen;
  grub_uint32_t unused;
  grub_uint8_t pad[12];
} GRUB_PACKED;
#endif

/* Directory entry.  */
struct grub_ufs_dirent
{
  grub_uint32_t ino;
  grub_uint16_t direntlen;
  union
  {
    grub_uint16_t namelen;
    struct
    {
      grub_uint8_t filetype_bsd;
      grub_uint8_t namelen_bsd;
    };
  };
} GRUB_PACKED;

/* Information about a "mounted" ufs filesystem.  */
struct grub_ufs_data
{
  struct grub_ufs_sblock sblock;
  grub_disk_t disk;
  struct grub_ufs_inode inode;
  int ino;
  int linknest;
  int log2_blksz;
};

static grub_dl_t my_mod;

/* Forward declaration.  */
static grub_err_t grub_ufs_find_file (struct grub_ufs_data *data,
				      const char *path);


static grub_disk_addr_t
grub_ufs_get_file_block (struct grub_ufs_data *data, grub_disk_addr_t blk)
{
  unsigned long indirsz;
  int log2_blksz, log_indirsz;

  /* Direct.  */
  if (blk < GRUB_UFS_DIRBLKS)
    return INODE_DIRBLOCKS (data, blk);

  log2_blksz = grub_ufs_to_cpu32 (data->sblock.log2_blksz);

  blk -= GRUB_UFS_DIRBLKS;

  log_indirsz = data->log2_blksz - LOG_INODE_BLKSZ;
  indirsz = 1 << log_indirsz;

  /* Single indirect block.  */
  if (blk < indirsz)
    {
      grub_ufs_blk_t indir;
      grub_disk_read (data->disk,
		      ((grub_disk_addr_t) INODE_INDIRBLOCKS (data, 0))
		      << log2_blksz,
		      blk * sizeof (indir), sizeof (indir), &indir);
      return indir;
    }
  blk -= indirsz;

  /* Double indirect block.  */
  if (blk < (grub_disk_addr_t) indirsz * (grub_disk_addr_t) indirsz)
    {
      grub_ufs_blk_t indir;

      grub_disk_read (data->disk,
		      ((grub_disk_addr_t) INODE_INDIRBLOCKS (data, 1))
		      << log2_blksz,
		      (blk >> log_indirsz) * sizeof (indir),
		      sizeof (indir), &indir);
      grub_disk_read (data->disk,
		      grub_ufs_to_cpu_blk (indir) << log2_blksz,
		      (blk & ((1 << log_indirsz) - 1)) * sizeof (indir),
		      sizeof (indir), &indir);

      return indir;
    }

  blk -= (grub_disk_addr_t) indirsz * (grub_disk_addr_t) indirsz;

  /* Triple indirect block.  */
  if (!(blk >> (3 * log_indirsz)))
    {
      grub_ufs_blk_t indir;

      grub_disk_read (data->disk,
		      ((grub_disk_addr_t) INODE_INDIRBLOCKS (data, 2))
		      << log2_blksz,
		      (blk >> (2 * log_indirsz)) * sizeof (indir),
		      sizeof (indir), &indir);
      grub_disk_read (data->disk,
		      grub_ufs_to_cpu_blk (indir) << log2_blksz,
		      ((blk >> log_indirsz)
		       & ((1 << log_indirsz) - 1)) * sizeof (indir),
		      sizeof (indir), &indir);

      grub_disk_read (data->disk,
		      grub_ufs_to_cpu_blk (indir) << log2_blksz,
		      (blk & ((1 << log_indirsz) - 1)) * sizeof (indir),
		      sizeof (indir), &indir);

      return indir;
    }

  grub_error (GRUB_ERR_BAD_FS,
	      "ufs does not support quadruple indirect blocks");
  return 0;
}


/* Read LEN bytes from the file described by DATA starting with byte
   POS.  Return the amount of read bytes in READ.  */
static grub_ssize_t
grub_ufs_read_file (struct grub_ufs_data *data,
		    grub_disk_read_hook_t read_hook, void *read_hook_data,
		    grub_off_t pos, grub_size_t len, char *buf)
{
  struct grub_ufs_sblock *sblock = &data->sblock;
  grub_off_t i;
  grub_off_t blockcnt;

  /* Adjust len so it we can't read past the end of the file.  */
  if (len + pos > INODE_SIZE (data))
    len = INODE_SIZE (data) - pos;

  blockcnt = (len + pos + UFS_BLKSZ (sblock) - 1) >> UFS_LOG_BLKSZ (sblock);

  for (i = pos >> UFS_LOG_BLKSZ (sblock); i < blockcnt; i++)
    {
      grub_disk_addr_t blknr;
      grub_off_t blockoff;
      grub_off_t blockend = UFS_BLKSZ (sblock);

      int skipfirst = 0;

      blockoff = pos & (UFS_BLKSZ (sblock) - 1);

      blknr = grub_ufs_get_file_block (data, i);
      if (grub_errno)
	return -1;

      /* Last block.  */
      if (i == blockcnt - 1)
	{
	  blockend = (len + pos) & (UFS_BLKSZ (sblock) - 1);

	  if (!blockend)
	    blockend = UFS_BLKSZ (sblock);
	}

      /* First block.  */
      if (i == (pos >> UFS_LOG_BLKSZ (sblock)))
	{
	  skipfirst = blockoff;
	  blockend -= skipfirst;
	}

      /* XXX: If the block number is 0 this block is not stored on
	 disk but is zero filled instead.  */
      if (blknr)
	{
	  data->disk->read_hook = read_hook;
	  data->disk->read_hook_data = read_hook_data;
	  grub_disk_read (data->disk,
			  blknr << grub_ufs_to_cpu32 (data->sblock.log2_blksz),
			  skipfirst, blockend, buf);
	  data->disk->read_hook = 0;
	  if (grub_errno)
	    return -1;
	}
      else
	grub_memset (buf, UFS_BLKSZ (sblock) - skipfirst, 0);

      buf += UFS_BLKSZ (sblock) - skipfirst;
    }

  return len;
}

/* Read inode INO from the mounted filesystem described by DATA.  This
   inode is used by default now.  */
static grub_err_t
grub_ufs_read_inode (struct grub_ufs_data *data, int ino, char *inode)
{
  struct grub_ufs_sblock *sblock = &data->sblock;

  /* Determine the group the inode is in.  */
  int group = ino / grub_ufs_to_cpu32 (sblock->ino_per_group);

  /* Determine the inode within the group.  */
  int grpino = ino % grub_ufs_to_cpu32 (sblock->ino_per_group);

  /* The first block of the group.  */
  int grpblk = group * (grub_ufs_to_cpu32 (sblock->frags_per_group));

#ifndef MODE_UFS2
  grpblk += grub_ufs_to_cpu32 (sblock->cylg_offset)
    * (group & (~grub_ufs_to_cpu32 (sblock->cylg_mask)));
#endif

  if (!inode)
    {
      inode = (char *) &data->inode;
      data->ino = ino;
    }

  grub_disk_read (data->disk,
		  ((grub_ufs_to_cpu32 (sblock->inoblk_offs) + grpblk)
		   << grub_ufs_to_cpu32 (data->sblock.log2_blksz))
		  + grpino / UFS_INODE_PER_BLOCK,
		  (grpino % UFS_INODE_PER_BLOCK)
		  * sizeof (struct grub_ufs_inode),
		  sizeof (struct grub_ufs_inode),
		  inode);

  return grub_errno;
}


/* Lookup the symlink the current inode points to.  INO is the inode
   number of the directory the symlink is relative to.  */
static grub_err_t
grub_ufs_lookup_symlink (struct grub_ufs_data *data, int ino)
{
  char *symlink;
  grub_size_t sz = INODE_SIZE (data);

  if (++data->linknest > GRUB_UFS_MAX_SYMLNK_CNT)
    return grub_error (GRUB_ERR_SYMLINK_LOOP, N_("too deep nesting of symlinks"));

  symlink = grub_malloc (sz + 1);
  if (!symlink)
    return grub_errno;
  /* Normally we should just check that data->inode.nblocks == 0.
     However old Linux doesn't maintain nblocks correctly and so it's always
     0. If size is bigger than inline space then the symlink is surely not
     inline.  */
  /* Check against zero is paylindromic, no need to swap.  */
  if (data->inode.nblocks == 0
      && INODE_SIZE (data) <= sizeof (data->inode.symlink))
    grub_strcpy (symlink, (char *) data->inode.symlink);
  else
    {
      if (grub_ufs_read_file (data, 0, 0, 0, sz, symlink) < 0)
	{
	  grub_free(symlink);
	  return grub_errno;
	}
    }
  symlink[sz] = '\0';

  /* The symlink is an absolute path, go back to the root inode.  */
  if (symlink[0] == '/')
    ino = GRUB_UFS_INODE;

  /* Now load in the old inode.  */
  if (grub_ufs_read_inode (data, ino, 0))
    {
      grub_free (symlink);
      return grub_errno;
    }

  grub_ufs_find_file (data, symlink);

  grub_free (symlink);

  return grub_errno;
}


/* Find the file with the pathname PATH on the filesystem described by
   DATA.  */
static grub_err_t
grub_ufs_find_file (struct grub_ufs_data *data, const char *path)
{
  const char *name;
  const char *next = path;
  unsigned int pos = 0;
  int dirino;
  char *filename;

  /* We reject filenames longer than the one we're looking
     for without reading, so this allocation is enough.  */
  filename = grub_malloc (grub_strlen (path) + 2);
  if (!filename)
    return grub_errno;

  while (1)
    {
      struct grub_ufs_dirent dirent;

      name = next;
      /* Skip the first slash.  */
      while (*name == '/')
	name++;
      if (*name == 0)
	{
	  grub_free (filename);
	  return GRUB_ERR_NONE;
	}

      if ((INODE_MODE(data) & GRUB_UFS_ATTR_TYPE)
	  != GRUB_UFS_ATTR_DIR)
	{
	  grub_error (GRUB_ERR_BAD_FILE_TYPE,
		      N_("not a directory"));
	  goto fail;
	}

      /* Extract the actual part from the pathname.  */
      for (next = name; *next && *next != '/'; next++);
      for (pos = 0;  ; pos += grub_ufs_to_cpu16 (dirent.direntlen))
	{
	  int namelen;

	  if (pos >= INODE_SIZE (data))
	    {
	      grub_error (GRUB_ERR_FILE_NOT_FOUND,
			  N_("file `%s' not found"),
			  path);
	      goto fail;
	    }

	  if (grub_ufs_read_file (data, 0, 0, pos, sizeof (dirent),
				  (char *) &dirent) < 0)
	    goto fail;

#ifdef MODE_UFS2
	  namelen = dirent.namelen_bsd;
#else
	  namelen = grub_ufs_to_cpu16 (dirent.namelen);
#endif
	  if (namelen < next - name)
	    continue;

	  if (grub_ufs_read_file (data, 0, 0, pos + sizeof (dirent),
				  next - name + (namelen != next - name),
				  filename) < 0)
	    goto fail;

	  if (grub_strncmp (name, filename, next - name) == 0
	      && (namelen == next - name || filename[next - name] == '\0'))
	    {
	      dirino = data->ino;
	      grub_ufs_read_inode (data, grub_ufs_to_cpu32 (dirent.ino), 0);
	      
	      if ((INODE_MODE(data) & GRUB_UFS_ATTR_TYPE)
		  == GRUB_UFS_ATTR_LNK)
		{
		  grub_ufs_lookup_symlink (data, dirino);
		  if (grub_errno)
		    goto fail;
		}

	      break;
	    }
	}
    }
 fail:
  grub_free (filename);
  return grub_errno;
}


/* Mount the filesystem on the disk DISK.  */
static struct grub_ufs_data *
grub_ufs_mount (grub_disk_t disk)
{
  struct grub_ufs_data *data;
  int *sblklist = sblocklist;

  data = grub_malloc (sizeof (struct grub_ufs_data));
  if (!data)
    return 0;

  /* Find a UFS sblock.  */
  while (*sblklist != -1)
    {
      grub_disk_read (disk, *sblklist, 0, sizeof (struct grub_ufs_sblock),
		      &data->sblock);
      if (grub_errno)
	goto fail;

      /* No need to byteswap bsize in this check. It works the same on both
	 endiannesses.  */
      if (data->sblock.magic == grub_cpu_to_ufs32_compile_time (GRUB_UFS_MAGIC)
	  && data->sblock.bsize != 0
	  && ((data->sblock.bsize & (data->sblock.bsize - 1)) == 0)
	  && data->sblock.ino_per_group != 0)
	{
	  for (data->log2_blksz = 0; 
	       (1U << data->log2_blksz) < grub_ufs_to_cpu32 (data->sblock.bsize);
	       data->log2_blksz++);

	  data->disk = disk;
	  data->linknest = 0;
	  return data;
	}
      sblklist++;
    }

 fail:

  if (grub_errno == GRUB_ERR_NONE || grub_errno == GRUB_ERR_OUT_OF_RANGE)
    {
#ifdef MODE_UFS2
      grub_error (GRUB_ERR_BAD_FS, "not an ufs2 filesystem");
#else
      grub_error (GRUB_ERR_BAD_FS, "not an ufs1 filesystem");
#endif
    }

  grub_free (data);

  return 0;
}


static grub_err_t
grub_ufs_dir (grub_device_t device, const char *path,
	      grub_fs_dir_hook_t hook, void *hook_data)
{
  struct grub_ufs_data *data;
  unsigned int pos = 0;

  data = grub_ufs_mount (device->disk);
  if (!data)
    return grub_errno;

  grub_ufs_read_inode (data, GRUB_UFS_INODE, 0);
  if (grub_errno)
    return grub_errno;

  if (!path || path[0] != '/')
    {
      grub_error (GRUB_ERR_BAD_FILENAME, N_("invalid file name `%s'"), path);
      return grub_errno;
    }

  grub_ufs_find_file (data, path);
  if (grub_errno)
    goto fail;

  if ((INODE_MODE (data) & GRUB_UFS_ATTR_TYPE) != GRUB_UFS_ATTR_DIR)
    {
      grub_error (GRUB_ERR_BAD_FILE_TYPE, N_("not a directory"));
      goto fail;
    }

  while (pos < INODE_SIZE (data))
    {
      struct grub_ufs_dirent dirent;
      int namelen;

      if (grub_ufs_read_file (data, 0, 0, pos, sizeof (dirent),
			      (char *) &dirent) < 0)
	break;

      if (dirent.direntlen == 0)
	break;

#ifdef MODE_UFS2
      namelen = dirent.namelen_bsd;
#else
      namelen = grub_ufs_to_cpu16 (dirent.namelen);
#endif

      char *filename = grub_malloc (namelen + 1);
      if (!filename)
	goto fail;
      struct grub_dirhook_info info;
      struct grub_ufs_inode inode;

      grub_memset (&info, 0, sizeof (info));

      if (grub_ufs_read_file (data, 0, 0, pos + sizeof (dirent),
			      namelen, filename) < 0)
	{
	  grub_free (filename);
	  break;
	}

      filename[namelen] = '\0';
      grub_ufs_read_inode (data, grub_ufs_to_cpu32 (dirent.ino),
			   (char *) &inode);

      info.dir = ((grub_ufs_to_cpu16 (inode.mode) & GRUB_UFS_ATTR_TYPE)
		  == GRUB_UFS_ATTR_DIR);
#ifdef MODE_UFS2
      info.mtime = grub_ufs_to_cpu64 (inode.mtime);
#else
      info.mtime = grub_ufs_to_cpu32 (inode.mtime);
#endif
      info.mtimeset = 1;

      if (hook (filename, &info, hook_data))
	{
	  grub_free (filename);
	  break;
	}

      grub_free (filename);

      pos += grub_ufs_to_cpu16 (dirent.direntlen);
    }

 fail:
  grub_free (data);

  return grub_errno;
}


/* Open a file named NAME and initialize FILE.  */
static grub_err_t
grub_ufs_open (struct grub_file *file, const char *name)
{
  struct grub_ufs_data *data;
  data = grub_ufs_mount (file->device->disk);
  if (!data)
    return grub_errno;

  grub_ufs_read_inode (data, 2, 0);
  if (grub_errno)
    {
      grub_free (data);
      return grub_errno;
    }

  if (!name || name[0] != '/')
    {
      grub_error (GRUB_ERR_BAD_FILENAME, N_("invalid file name `%s'"), name);
      return grub_errno;
    }

  grub_ufs_find_file (data, name);
  if (grub_errno)
    {
      grub_free (data);
      return grub_errno;
    }

  file->data = data;
  file->size = INODE_SIZE (data);

  return GRUB_ERR_NONE;
}


static grub_ssize_t
grub_ufs_read (grub_file_t file, char *buf, grub_size_t len)
{
  struct grub_ufs_data *data =
    (struct grub_ufs_data *) file->data;

  return grub_ufs_read_file (data, file->read_hook, file->read_hook_data,
			     file->offset, len, buf);
}


static grub_err_t
grub_ufs_close (grub_file_t file)
{
  grub_free (file->data);

  return GRUB_ERR_NONE;
}

static grub_err_t
grub_ufs_label (grub_device_t device, char **label)
{
  struct grub_ufs_data *data = 0;

  grub_dl_ref (my_mod);

  *label = 0;

  data = grub_ufs_mount (device->disk);
  if (data)
    *label = grub_strdup ((char *) data->sblock.volume_name);

  grub_dl_unref (my_mod);

  grub_free (data);

  return grub_errno;
}

static grub_err_t
grub_ufs_uuid (grub_device_t device, char **uuid)
{
  struct grub_ufs_data *data;
  grub_disk_t disk = device->disk;

  grub_dl_ref (my_mod);

  data = grub_ufs_mount (disk);
  if (data && (data->sblock.uuidhi != 0 || data->sblock.uuidlow != 0))
    *uuid = grub_xasprintf ("%08x%08x",
			   (unsigned) grub_ufs_to_cpu32 (data->sblock.uuidhi),
			   (unsigned) grub_ufs_to_cpu32 (data->sblock.uuidlow));
  else
    *uuid = NULL;

  grub_dl_unref (my_mod);

  grub_free (data);

  return grub_errno;
}


/* Get mtime.  */
static grub_err_t
grub_ufs_mtime (grub_device_t device, grub_int32_t *tm)
{
  struct grub_ufs_data *data = 0;

  grub_dl_ref (my_mod);

  data = grub_ufs_mount (device->disk);
  if (!data)
    *tm = 0;
  else
    {
      *tm = grub_ufs_to_cpu32 (data->sblock.mtime);
#ifdef MODE_UFS2
      if (*tm < (grub_int64_t) grub_ufs_to_cpu64 (data->sblock.mtime2))
	*tm = grub_ufs_to_cpu64 (data->sblock.mtime2);
#endif
    }

  grub_dl_unref (my_mod);

  grub_free (data);

  return grub_errno;
}



static struct grub_fs grub_ufs_fs =
  {
#ifdef MODE_UFS2
    .name = "ufs2",
#else
#ifdef MODE_BIGENDIAN
    .name = "ufs1_be",
#else
    .name = "ufs1",
#endif
#endif
    .dir = grub_ufs_dir,
    .open = grub_ufs_open,
    .read = grub_ufs_read,
    .close = grub_ufs_close,
    .label = grub_ufs_label,
    .uuid = grub_ufs_uuid,
    .mtime = grub_ufs_mtime,
    /* FIXME: set reserved_first_sector.  */
#ifdef GRUB_UTIL
    .blocklist_install = 1,
#endif
    .next = 0
  };

#ifdef MODE_UFS2
GRUB_MOD_INIT(ufs2)
#else
#ifdef MODE_BIGENDIAN
GRUB_MOD_INIT(ufs1_be)
#else
GRUB_MOD_INIT(ufs1)
#endif
#endif
{
  grub_fs_register (&grub_ufs_fs);
  my_mod = mod;
}

#ifdef MODE_UFS2
GRUB_MOD_FINI(ufs2)
#else
#ifdef MODE_BIGENDIAN
GRUB_MOD_FINI(ufs1_be)
#else
GRUB_MOD_FINI(ufs1)
#endif
#endif
{
  grub_fs_unregister (&grub_ufs_fs);
}

