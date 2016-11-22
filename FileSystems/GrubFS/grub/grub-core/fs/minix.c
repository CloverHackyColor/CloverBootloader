/* minix.c - The minix filesystem, version 1 and 2.  */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2004,2005,2006,2007,2008  Free Software Foundation, Inc.
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

#ifdef MODE_MINIX3
#define GRUB_MINIX_MAGIC	0x4D5A
#elif defined(MODE_MINIX2)
#define GRUB_MINIX_MAGIC	0x2468
#define GRUB_MINIX_MAGIC_30	0x2478
#else
#define GRUB_MINIX_MAGIC	0x137F
#define GRUB_MINIX_MAGIC_30	0x138F
#endif

#define GRUB_MINIX_INODE_DIR_BLOCKS	7
#define GRUB_MINIX_LOG2_BSIZE	1
#define GRUB_MINIX_ROOT_INODE	1
#define GRUB_MINIX_MAX_SYMLNK_CNT	8
#define GRUB_MINIX_SBLOCK	2

#define GRUB_MINIX_IFDIR	0040000U
#define GRUB_MINIX_IFLNK	0120000U

#ifdef MODE_BIGENDIAN
#define grub_cpu_to_minix16_compile_time grub_cpu_to_be16_compile_time
#define grub_minix_to_cpu16 grub_be_to_cpu16
#define grub_minix_to_cpu32 grub_be_to_cpu32
#else
#define grub_cpu_to_minix16_compile_time grub_cpu_to_le16_compile_time
#define grub_minix_to_cpu16 grub_le_to_cpu16
#define grub_minix_to_cpu32 grub_le_to_cpu32
#endif

#if defined(MODE_MINIX2) || defined(MODE_MINIX3)
typedef grub_uint32_t grub_minix_uintn_t;
#define grub_minix_to_cpu_n grub_minix_to_cpu32
#else
typedef grub_uint16_t grub_minix_uintn_t;
#define grub_minix_to_cpu_n grub_minix_to_cpu16
#endif

#define GRUB_MINIX_INODE_BLKSZ(data) sizeof (grub_minix_uintn_t)
#ifdef MODE_MINIX3
typedef grub_uint32_t grub_minix_ino_t;
#define grub_minix_to_cpu_ino grub_minix_to_cpu32
#else
typedef grub_uint16_t grub_minix_ino_t;
#define grub_minix_to_cpu_ino grub_minix_to_cpu16
#endif

#define GRUB_MINIX_INODE_SIZE(data) (grub_minix_to_cpu32 (data->inode.size))
#define GRUB_MINIX_INODE_MODE(data) (grub_minix_to_cpu16 (data->inode.mode))
#define GRUB_MINIX_INODE_DIR_ZONES(data,blk) (grub_minix_to_cpu_n   \
					      (data->inode.dir_zones[blk]))
#define GRUB_MINIX_INODE_INDIR_ZONE(data)  (grub_minix_to_cpu_n \
					    (data->inode.indir_zone))
#define GRUB_MINIX_INODE_DINDIR_ZONE(data) (grub_minix_to_cpu_n \
					    (data->inode.double_indir_zone))

#ifndef MODE_MINIX3
#define GRUB_MINIX_LOG2_ZONESZ	(GRUB_MINIX_LOG2_BSIZE				\
				 + grub_minix_to_cpu16 (data->sblock.log2_zone_size))
#endif
#define GRUB_MINIX_ZONESZ	((grub_uint64_t) data->block_size <<			\
				 (GRUB_DISK_SECTOR_BITS + grub_minix_to_cpu16 (data->sblock.log2_zone_size)))

#ifdef MODE_MINIX3
#define GRUB_MINIX_ZONE2SECT(zone) ((zone) * data->block_size)
#else
#define GRUB_MINIX_ZONE2SECT(zone) ((zone) << GRUB_MINIX_LOG2_ZONESZ)
#endif


#ifdef MODE_MINIX3
struct grub_minix_sblock
{
  grub_uint32_t inode_cnt;
  grub_uint16_t zone_cnt;
  grub_uint16_t inode_bmap_size;
  grub_uint16_t zone_bmap_size;
  grub_uint16_t first_data_zone;
  grub_uint16_t log2_zone_size;
  grub_uint16_t pad;
  grub_uint32_t max_file_size;
  grub_uint32_t zones;
  grub_uint16_t magic;
  
  grub_uint16_t pad2;
  grub_uint16_t block_size;
  grub_uint8_t disk_version; 
};
#else
struct grub_minix_sblock
{
  grub_uint16_t inode_cnt;
  grub_uint16_t zone_cnt;
  grub_uint16_t inode_bmap_size;
  grub_uint16_t zone_bmap_size;
  grub_uint16_t first_data_zone;
  grub_uint16_t log2_zone_size;
  grub_uint32_t max_file_size;
  grub_uint16_t magic;
};
#endif

#if defined(MODE_MINIX3) || defined(MODE_MINIX2)
struct grub_minix_inode
{
  grub_uint16_t mode;
  grub_uint16_t nlinks;
  grub_uint16_t uid;
  grub_uint16_t gid;
  grub_uint32_t size;
  grub_uint32_t atime;
  grub_uint32_t mtime;
  grub_uint32_t ctime;
  grub_uint32_t dir_zones[7];
  grub_uint32_t indir_zone;
  grub_uint32_t double_indir_zone;
  grub_uint32_t triple_indir_zone;
};
#else
struct grub_minix_inode
{
  grub_uint16_t mode;
  grub_uint16_t uid;
  grub_uint32_t size;
  grub_uint32_t mtime;
  grub_uint8_t gid;
  grub_uint8_t nlinks;
  grub_uint16_t dir_zones[7];
  grub_uint16_t indir_zone;
  grub_uint16_t double_indir_zone;
};

#endif

#if defined(MODE_MINIX3)
#define MAX_MINIX_FILENAME_SIZE 60
#else
#define MAX_MINIX_FILENAME_SIZE 30
#endif

/* Information about a "mounted" minix filesystem.  */
struct grub_minix_data
{
  struct grub_minix_sblock sblock;
  struct grub_minix_inode inode;
  grub_minix_ino_t ino;
  int linknest;
  grub_disk_t disk;
  int filename_size;
  grub_size_t block_size;
};

static grub_dl_t my_mod;

static grub_err_t grub_minix_find_file (struct grub_minix_data *data,
					const char *path);

  /* Read the block pointer in ZONE, on the offset NUM.  */
static grub_minix_uintn_t
grub_get_indir (struct grub_minix_data *data, 
		 grub_minix_uintn_t zone,
		 grub_minix_uintn_t num)
{
  grub_minix_uintn_t indirn;
  grub_disk_read (data->disk,
		  GRUB_MINIX_ZONE2SECT(zone),
		  sizeof (grub_minix_uintn_t) * num,
		  sizeof (grub_minix_uintn_t), (char *) &indirn);
  return grub_minix_to_cpu_n (indirn);
}

static grub_minix_uintn_t
grub_minix_get_file_block (struct grub_minix_data *data, unsigned int blk)
{
  grub_minix_uintn_t indir;
  const grub_uint32_t block_per_zone = (GRUB_MINIX_ZONESZ
					/ GRUB_MINIX_INODE_BLKSZ (data));

  /* Direct block.  */
  if (blk < GRUB_MINIX_INODE_DIR_BLOCKS)
    return GRUB_MINIX_INODE_DIR_ZONES (data, blk);

  /* Indirect block.  */
  blk -= GRUB_MINIX_INODE_DIR_BLOCKS;
  if (blk < block_per_zone)
    {
      indir = grub_get_indir (data, GRUB_MINIX_INODE_INDIR_ZONE (data), blk);
      return indir;
    }

  /* Double indirect block.  */
  blk -= block_per_zone;
  if (blk < block_per_zone * block_per_zone)
    {
      indir = grub_get_indir (data, GRUB_MINIX_INODE_DINDIR_ZONE (data),
			      blk / block_per_zone);

      indir = grub_get_indir (data, indir, blk % block_per_zone);

      return indir;
    }

#if defined (MODE_MINIX3) || defined (MODE_MINIX2)
  blk -= block_per_zone * block_per_zone;
  if (blk < ((grub_uint64_t) block_per_zone * (grub_uint64_t) block_per_zone
	     * (grub_uint64_t) block_per_zone))
    {
      indir = grub_get_indir (data, grub_minix_to_cpu_n (data->inode.triple_indir_zone),
			      (blk / block_per_zone) / block_per_zone);
      indir = grub_get_indir (data, indir, (blk / block_per_zone) % block_per_zone);
      indir = grub_get_indir (data, indir, blk % block_per_zone);

      return indir;
    }
#endif

  /* This should never happen.  */
  grub_error (GRUB_ERR_OUT_OF_RANGE, "file bigger than maximum size");

  return 0;
}


/* Read LEN bytes from the file described by DATA starting with byte
   POS.  Return the amount of read bytes in READ.  */
static grub_ssize_t
grub_minix_read_file (struct grub_minix_data *data,
		      grub_disk_read_hook_t read_hook, void *read_hook_data,
		      grub_off_t pos, grub_size_t len, char *buf)
{
  grub_uint32_t i;
  grub_uint32_t blockcnt;
  grub_uint32_t posblock;
  grub_uint32_t blockoff;

  /* Adjust len so it we can't read past the end of the file.  */
  if (len + pos > GRUB_MINIX_INODE_SIZE (data))
    len = GRUB_MINIX_INODE_SIZE (data) - pos;
  if (len == 0)
    return 0;

  /* Files are at most 2G/4G - 1 bytes on minixfs. Avoid 64-bit division.  */
  blockcnt = ((grub_uint32_t) ((len + pos - 1)
	       >> GRUB_DISK_SECTOR_BITS)) / data->block_size + 1;
  posblock = (((grub_uint32_t) pos)
	      / (data->block_size << GRUB_DISK_SECTOR_BITS));
  blockoff = (((grub_uint32_t) pos)
	      % (data->block_size << GRUB_DISK_SECTOR_BITS));

  for (i = posblock; i < blockcnt; i++)
    {
      grub_disk_addr_t blknr;
      grub_uint64_t blockend = data->block_size << GRUB_DISK_SECTOR_BITS;
      grub_off_t skipfirst = 0;

      blknr = grub_minix_get_file_block (data, i);
      if (grub_errno)
	return -1;

      /* Last block.  */
      if (i == blockcnt - 1)
	{
	  /* len + pos < 4G (checked above), so it doesn't overflow.  */
	  blockend = (((grub_uint32_t) (len + pos))
		      % (data->block_size << GRUB_DISK_SECTOR_BITS));

	  if (!blockend)
	    blockend = data->block_size << GRUB_DISK_SECTOR_BITS;
	}

      /* First block.  */
      if (i == posblock)
	{
	  skipfirst = blockoff;
	  blockend -= skipfirst;
	}

      data->disk->read_hook = read_hook;
      data->disk->read_hook_data = read_hook_data;
      grub_disk_read (data->disk,
		      GRUB_MINIX_ZONE2SECT(blknr),
		      skipfirst, blockend, buf);
      data->disk->read_hook = 0;
      if (grub_errno)
	return -1;

      buf += (data->block_size << GRUB_DISK_SECTOR_BITS) - skipfirst;
    }

  return len;
}


/* Read inode INO from the mounted filesystem described by DATA.  This
   inode is used by default now.  */
static grub_err_t
grub_minix_read_inode (struct grub_minix_data *data, grub_minix_ino_t ino)
{
  struct grub_minix_sblock *sblock = &data->sblock;

  /* Block in which the inode is stored.  */
  grub_disk_addr_t block;
  data->ino = ino;

  /* The first inode in minix is inode 1.  */
  ino--;
  block = GRUB_MINIX_ZONE2SECT (2 + grub_minix_to_cpu16 (sblock->inode_bmap_size)
				+ grub_minix_to_cpu16 (sblock->zone_bmap_size));
  block += ino / (GRUB_DISK_SECTOR_SIZE / sizeof (struct grub_minix_inode));
  int offs = (ino % (GRUB_DISK_SECTOR_SIZE
		     / sizeof (struct grub_minix_inode))
	      * sizeof (struct grub_minix_inode));
  
  grub_disk_read (data->disk, block, offs,
		  sizeof (struct grub_minix_inode), &data->inode);

  return GRUB_ERR_NONE;
}


/* Lookup the symlink the current inode points to.  INO is the inode
   number of the directory the symlink is relative to.  */
static grub_err_t
grub_minix_lookup_symlink (struct grub_minix_data *data, grub_minix_ino_t ino)
{
  char *symlink;
  grub_size_t sz = GRUB_MINIX_INODE_SIZE (data);

  if (++data->linknest > GRUB_MINIX_MAX_SYMLNK_CNT)
    return grub_error (GRUB_ERR_SYMLINK_LOOP, N_("too deep nesting of symlinks"));

  symlink = grub_malloc (sz + 1);
  if (!symlink)
    return grub_errno;
  if (grub_minix_read_file (data, 0, 0, 0, sz, symlink) < 0)
    return grub_errno;

  symlink[sz] = '\0';

  /* The symlink is an absolute path, go back to the root inode.  */
  if (symlink[0] == '/')
    ino = GRUB_MINIX_ROOT_INODE;

  /* Now load in the old inode.  */
  if (grub_minix_read_inode (data, ino))
    return grub_errno;

  grub_minix_find_file (data, symlink);

  return grub_errno;
}


/* Find the file with the pathname PATH on the filesystem described by
   DATA.  */
static grub_err_t
grub_minix_find_file (struct grub_minix_data *data, const char *path)
{
  const char *name;
  const char *next = path;
  unsigned int pos = 0;
  grub_minix_ino_t dirino;

  while (1)
    {
      name = next;
      /* Skip the first slash.  */
      while (*name == '/')
	name++;
      if (!*name)
	return GRUB_ERR_NONE;

      if ((GRUB_MINIX_INODE_MODE (data)
	   & GRUB_MINIX_IFDIR) != GRUB_MINIX_IFDIR)
	return grub_error (GRUB_ERR_BAD_FILE_TYPE, N_("not a directory"));

      /* Extract the actual part from the pathname.  */
      for (next = name; *next && *next != '/'; next++);

      for (pos = 0; ; )
	{
	  grub_minix_ino_t ino;
	  char filename[MAX_MINIX_FILENAME_SIZE + 1];

	  if (pos >= GRUB_MINIX_INODE_SIZE (data))
	    {
	      grub_error (GRUB_ERR_FILE_NOT_FOUND, N_("file `%s' not found"), path);
	      return grub_errno;
	    }

	  if (grub_minix_read_file (data, 0, 0, pos, sizeof (ino),
				    (char *) &ino) < 0)
	    return grub_errno;
	  if (grub_minix_read_file (data, 0, 0, pos + sizeof (ino),
				    data->filename_size, (char *) filename)< 0)
	    return grub_errno;

	  pos += sizeof (ino) + data->filename_size;

	  filename[data->filename_size] = '\0';

	  /* Check if the current direntry matches the current part of the
	     pathname.  */
	  if (grub_strncmp (name, filename, next - name) == 0
	      && filename[next - name] == '\0')
	    {
	      dirino = data->ino;
	      grub_minix_read_inode (data, grub_minix_to_cpu_ino (ino));

	      /* Follow the symlink.  */
	      if ((GRUB_MINIX_INODE_MODE (data)
		   & GRUB_MINIX_IFLNK) == GRUB_MINIX_IFLNK)
		{
		  grub_minix_lookup_symlink (data, dirino);
		  if (grub_errno)
		    return grub_errno;
		}

	      break;
	    }
	}
    }
}


/* Mount the filesystem on the disk DISK.  */
static struct grub_minix_data *
grub_minix_mount (grub_disk_t disk)
{
  struct grub_minix_data *data;

  data = grub_malloc (sizeof (struct grub_minix_data));
  if (!data)
    return 0;

  /* Read the superblock.  */
  grub_disk_read (disk, GRUB_MINIX_SBLOCK, 0,
		  sizeof (struct grub_minix_sblock),&data->sblock);
  if (grub_errno)
    goto fail;

  if (data->sblock.magic == grub_cpu_to_minix16_compile_time (GRUB_MINIX_MAGIC))
    {
#if !defined(MODE_MINIX3)
      data->filename_size = 14;
#else
      data->filename_size = 60;
#endif
    }
#if !defined(MODE_MINIX3)
  else if (data->sblock.magic
	   == grub_cpu_to_minix16_compile_time (GRUB_MINIX_MAGIC_30))
    data->filename_size = 30;
#endif
  else
    goto fail;

  /* 20 means 1G zones. We could support up to 31 but already 1G isn't
     supported by anything else.  */
  if (grub_minix_to_cpu16 (data->sblock.log2_zone_size) >= 20)
    goto fail;

  data->disk = disk;
  data->linknest = 0;
#ifdef MODE_MINIX3
  /* These tests are endian-independent. No need to byteswap.  */
  if (data->sblock.block_size == 0xffff)
    data->block_size = 2;
  else
    {
      if ((data->sblock.block_size == grub_cpu_to_minix16_compile_time (0x200))
	  || (data->sblock.block_size == 0)
	  || (data->sblock.block_size & grub_cpu_to_minix16_compile_time (0x1ff)))
	goto fail;
      data->block_size = grub_minix_to_cpu16 (data->sblock.block_size)
	>> GRUB_DISK_SECTOR_BITS;
    }
#else
  data->block_size = 2;
#endif

  return data;

 fail:
  grub_free (data);
#if defined(MODE_MINIX3)
  grub_error (GRUB_ERR_BAD_FS, "not a minix3 filesystem");
#elif defined(MODE_MINIX2)
  grub_error (GRUB_ERR_BAD_FS, "not a minix2 filesystem");
#else
  grub_error (GRUB_ERR_BAD_FS, "not a minix filesystem");
#endif
  return 0;
}

static grub_err_t
grub_minix_dir (grub_device_t device, const char *path,
		grub_fs_dir_hook_t hook, void *hook_data)
{
  struct grub_minix_data *data = 0;
  unsigned int pos = 0;

  data = grub_minix_mount (device->disk);
  if (!data)
    return grub_errno;

  grub_minix_read_inode (data, GRUB_MINIX_ROOT_INODE);
  if (grub_errno)
    goto fail;

  grub_minix_find_file (data, path);
  if (grub_errno)
    goto fail;

  if ((GRUB_MINIX_INODE_MODE (data) & GRUB_MINIX_IFDIR) != GRUB_MINIX_IFDIR)
    {
      grub_error (GRUB_ERR_BAD_FILE_TYPE, N_("not a directory"));
      goto fail;
    }

  while (pos < GRUB_MINIX_INODE_SIZE (data))
    {
      grub_minix_ino_t ino;
      char filename[MAX_MINIX_FILENAME_SIZE + 1];
      grub_minix_ino_t dirino = data->ino;
      struct grub_dirhook_info info;
      grub_memset (&info, 0, sizeof (info));


      if (grub_minix_read_file (data, 0, 0, pos, sizeof (ino),
				(char *) &ino) < 0)
	return grub_errno;

      if (grub_minix_read_file (data, 0, 0, pos + sizeof (ino),
				data->filename_size,
				(char *) filename) < 0)
	return grub_errno;
      filename[data->filename_size] = '\0';
      if (!ino)
	{
	  pos += sizeof (ino) + data->filename_size;
	  continue;
	}

      grub_minix_read_inode (data, grub_minix_to_cpu_ino (ino));
      info.dir = ((GRUB_MINIX_INODE_MODE (data)
		   & GRUB_MINIX_IFDIR) == GRUB_MINIX_IFDIR);
      info.mtimeset = 1;
      info.mtime = grub_minix_to_cpu32 (data->inode.mtime);

      if (hook (filename, &info, hook_data) ? 1 : 0)
	break;

      /* Load the old inode back in.  */
      grub_minix_read_inode (data, dirino);

      pos += sizeof (ino) + data->filename_size;
    }

 fail:
  grub_free (data);
  return grub_errno;
}


/* Open a file named NAME and initialize FILE.  */
static grub_err_t
grub_minix_open (struct grub_file *file, const char *name)
{
  struct grub_minix_data *data;
  data = grub_minix_mount (file->device->disk);
  if (!data)
    return grub_errno;

  /* Open the inode op the root directory.  */
  grub_minix_read_inode (data, GRUB_MINIX_ROOT_INODE);
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

  /* Traverse the directory tree to the node that should be
     opened.  */
  grub_minix_find_file (data, name);
  if (grub_errno)
    {
      grub_free (data);
      return grub_errno;
    }

  file->data = data;
  file->size = GRUB_MINIX_INODE_SIZE (data);

  return GRUB_ERR_NONE;
}


static grub_ssize_t
grub_minix_read (grub_file_t file, char *buf, grub_size_t len)
{
  struct grub_minix_data *data =
    (struct grub_minix_data *) file->data;

  return grub_minix_read_file (data, file->read_hook, file->read_hook_data,
			       file->offset, len, buf);
}


static grub_err_t
grub_minix_close (grub_file_t file)
{
  grub_free (file->data);

  return GRUB_ERR_NONE;
}



static struct grub_fs grub_minix_fs =
  {
#ifdef MODE_BIGENDIAN
#if defined(MODE_MINIX3)
    .name = "minix3_be",
#elif defined(MODE_MINIX2)
    .name = "minix2_be",
#else
    .name = "minix_be",
#endif
#else
#if defined(MODE_MINIX3)
    .name = "minix3",
#elif defined(MODE_MINIX2)
    .name = "minix2",
#else
    .name = "minix",
#endif
#endif
    .dir = grub_minix_dir,
    .open = grub_minix_open,
    .read = grub_minix_read,
    .close = grub_minix_close,
#ifdef GRUB_UTIL
    .reserved_first_sector = 1,
    .blocklist_install = 1,
#endif
    .next = 0
  };

#ifdef MODE_BIGENDIAN
#if defined(MODE_MINIX3)
GRUB_MOD_INIT(minix3_be)
#elif defined(MODE_MINIX2)
GRUB_MOD_INIT(minix2_be)
#else
GRUB_MOD_INIT(minix_be)
#endif
#else
#if defined(MODE_MINIX3)
GRUB_MOD_INIT(minix3)
#elif defined(MODE_MINIX2)
GRUB_MOD_INIT(minix2)
#else
GRUB_MOD_INIT(minix)
#endif
#endif
{
  grub_fs_register (&grub_minix_fs);
  my_mod = mod;
}

#ifdef MODE_BIGENDIAN
#if defined(MODE_MINIX3)
GRUB_MOD_FINI(minix3_be)
#elif defined(MODE_MINIX2)
GRUB_MOD_FINI(minix2_be)
#else
GRUB_MOD_FINI(minix_be)
#endif
#else
#if defined(MODE_MINIX3)
GRUB_MOD_FINI(minix3)
#elif defined(MODE_MINIX2)
GRUB_MOD_FINI(minix2)
#else
GRUB_MOD_FINI(minix)
#endif
#endif
{
  grub_fs_unregister (&grub_minix_fs);
}
