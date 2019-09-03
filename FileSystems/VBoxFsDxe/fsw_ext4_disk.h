/**
 * \file fsw_ext4_disk.h
 * ext4 file system on-disk structures.
 */

/*-
 * Copyright (c) 2012 Stefan Agner
 * Portions Copyright (c) 2006 Christoph Pfisterer
 * Portions Copyright (c) 1991-2012 by various Linux kernel contributors
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef _FSW_EXT4_DISK_H_
#define _FSW_EXT4_DISK_H_

// types

typedef fsw_s8  __s8;
typedef fsw_u8  __u8;
typedef fsw_s16 __s16;
typedef fsw_u16 __u16;
typedef fsw_s32 __s32;
typedef fsw_u32 __u32;
typedef fsw_s64 __s64;
typedef fsw_u64 __u64;

typedef __u16   __le16;
typedef __u32   __le32;
typedef __u64   __le64;

//
// from Linux kernel, fs/ext4/ext4.h
//

/*
 * Special inode numbers
 */
#define	EXT4_BAD_INO		 1	/* Bad blocks inode */
#define EXT4_ROOT_INO		 2	/* Root inode */
#define EXT4_USR_QUOTA_INO	 3	/* User quota inode */
#define EXT4_GRP_QUOTA_INO	 4	/* Group quota inode */
#define EXT4_BOOT_LOADER_INO	 5	/* Boot loader inode */
#define EXT4_UNDEL_DIR_INO	 6	/* Undelete directory inode */
#define EXT4_RESIZE_INO		 7	/* Reserved group descriptors inode */
#define EXT4_JOURNAL_INO	 8	/* Journal inode */

/*
 * The second extended file system magic number
 */
#define EXT4_SUPER_MAGIC        0xEF53

/*
 * Macro-instructions used to manage several block sizes
 */
#define EXT4_MIN_BLOCK_SIZE             1024
#define EXT4_MAX_BLOCK_SIZE             4096
#define EXT4_MIN_BLOCK_LOG_SIZE           10
#define EXT4_BLOCK_SIZE(s)              (EXT4_MIN_BLOCK_SIZE << (s)->s_log_block_size)
#define EXT4_ADDR_PER_BLOCK(s)          (EXT4_BLOCK_SIZE(s) / sizeof (__u32))
#define EXT4_BLOCK_SIZE_BITS(s)         ((s)->s_log_block_size + 10)
#define EXT4_INODE_SIZE(s)      (((s)->s_rev_level == EXT4_GOOD_OLD_REV) ? \
                                 EXT4_GOOD_OLD_INODE_SIZE : \
                                 (s)->s_inode_size)

/*
 * Structure of a blocks group descriptor
 */
struct ext4_group_desc
{
	__le32	bg_block_bitmap_lo;	/* Blocks bitmap block */
	__le32	bg_inode_bitmap_lo;	/* Inodes bitmap block */
	__le32	bg_inode_table_lo;	/* Inodes table block */
	__le16	bg_free_blocks_count_lo;/* Free blocks count */
	__le16	bg_free_inodes_count_lo;/* Free inodes count */
	__le16	bg_used_dirs_count_lo;	/* Directories count */
	__le16	bg_flags;		/* EXT4_BG_flags (INODE_UNINIT, etc) */
	__le32  bg_exclude_bitmap_lo;   /* Exclude bitmap for snapshots */
	__le16  bg_block_bitmap_csum_lo;/* crc32c(s_uuid+grp_num+bbitmap) LE */
	__le16  bg_inode_bitmap_csum_lo;/* crc32c(s_uuid+grp_num+ibitmap) LE */
	__le16  bg_itable_unused_lo;	/* Unused inodes count */
	__le16  bg_checksum;		/* crc16(sb_uuid+group+desc) */
	__le32	bg_block_bitmap_hi;	/* Blocks bitmap block MSB */
	__le32	bg_inode_bitmap_hi;	/* Inodes bitmap block MSB */
	__le32	bg_inode_table_hi;	/* Inodes table block MSB */
	__le16	bg_free_blocks_count_hi;/* Free blocks count MSB */
	__le16	bg_free_inodes_count_hi;/* Free inodes count MSB */
	__le16	bg_used_dirs_count_hi;	/* Directories count MSB */
	__le16  bg_itable_unused_hi;    /* Unused inodes count MSB */
	__le32  bg_exclude_bitmap_hi;   /* Exclude bitmap block MSB */
	__le16  bg_block_bitmap_csum_hi;/* crc32c(s_uuid+grp_num+bbitmap) BE */
	__le16  bg_inode_bitmap_csum_hi;/* crc32c(s_uuid+grp_num+ibitmap) BE */
	__u32   bg_reserved;
};


/*
 * Macro-instructions used to manage group descriptors
 */
#define EXT4_MIN_DESC_SIZE              32
#define EXT4_MIN_DESC_SIZE_64BIT        64
#define EXT4_MAX_DESC_SIZE              EXT4_MIN_BLOCK_SIZE
#define EXT4_DESC_SIZE(s)              ((s)->s_desc_size)
#define EXT4_BLOCKS_PER_GROUP(s)       ((s)->s_blocks_per_group)
#define EXT4_DESC_PER_BLOCK(s)         (EXT4_BLOCK_SIZE(s) / EXT4_DESC_SIZE(s))
#define EXT4_INODES_PER_GROUP(s)       ((s)->s_inodes_per_group)

/*
 * Constants relative to the data blocks
 */
#define EXT4_NDIR_BLOCKS                12
#define EXT4_IND_BLOCK                  EXT4_NDIR_BLOCKS
#define EXT4_DIND_BLOCK                 (EXT4_IND_BLOCK + 1)
#define EXT4_TIND_BLOCK                 (EXT4_DIND_BLOCK + 1)
#define EXT4_N_BLOCKS                   (EXT4_TIND_BLOCK + 1)

/*
 * Inode flags
 */
#define EXT4_SECRM_FL                   0x00000001 /* Secure deletion */
#define EXT4_UNRM_FL                    0x00000002 /* Undelete */
#define EXT4_COMPR_FL                   0x00000004 /* Compress file */
#define EXT4_SYNC_FL                    0x00000008 /* Synchronous updates */
#define EXT4_IMMUTABLE_FL               0x00000010 /* Immutable file */
#define EXT4_APPEND_FL                  0x00000020 /* writes to file may only append */
#define EXT4_NODUMP_FL                  0x00000040 /* do not dump file */
#define EXT4_NOATIME_FL                 0x00000080 /* do not update atime */
/* Reserved for compression usage... */
#define EXT4_DIRTY_FL                   0x00000100
#define EXT4_COMPRBLK_FL                0x00000200 /* One or more compressed clusters */
#define EXT4_NOCOMP_FL                  0x00000400 /* Don't compress */
#define EXT4_ECOMPR_FL                  0x00000800 /* Compression error */
/* End compression flags --- maybe not all used */      
#define EXT4_INDEX_FL                   0x00001000 /* hash-indexed directory */
#define EXT4_IMAGIC_FL                  0x00002000 /* AFS directory */
#define EXT4_JOURNAL_DATA_FL            0x00004000 /* Reserved for ext3 */
#define EXT4_NOTAIL_FL                  0x00008000 /* file tail should not be merged */
#define EXT4_DIRSYNC_FL                 0x00010000 /* dirsync behaviour (directories only) */
#define EXT4_TOPDIR_FL                  0x00020000 /* Top of directory hierarchies*/
#define EXT4_HUGE_FILE_FL               0x00040000 /* Set to each huge file */
#define EXT4_EXTENTS_FL                 0x00080000 /* Inode uses extents */
#define EXT4_EA_INODE_FL                0x00200000 /* Inode used for large EA */
#define EXT4_EOFBLOCKS_FL               0x00400000 /* Blocks allocated beyond EOF */
#define EXT4_RESERVED_FL                0x80000000 /* reserved for ext4 lib */

#define EXT4_FL_USER_VISIBLE		0x004BDFFF /* User visible flags */
#define EXT4_FL_USER_MODIFIABLE		0x004B80FF /* User modifiable flags */


/*
 * Structure of an inode on the disk
 */
struct ext4_inode {
	__le16	i_mode;		/* File mode */
	__le16	i_uid;		/* Low 16 bits of Owner Uid */
	__le32	i_size_lo;	/* Size in bytes */
	__le32	i_atime;	/* Access time */
	__le32	i_ctime;	/* Inode Change time */
	__le32	i_mtime;	/* Modification time */
	__le32	i_dtime;	/* Deletion Time */
	__le16	i_gid;		/* Low 16 bits of Group Id */
	__le16	i_links_count;	/* Links count */
	__le32	i_blocks_lo;	/* Blocks count */
	__le32	i_flags;	/* File flags */
	union {
		struct {
			__le32  l_i_version;
		} linux1;
		struct {
			__u32  h_i_translator;
		} hurd1;
		struct {
			__u32  m_i_reserved1;
		} masix1;
	} osd1;				/* OS dependent 1 */
	__le32	i_block[EXT4_N_BLOCKS];/* Pointers to blocks */
	__le32	i_generation;	/* File version (for NFS) */
	__le32	i_file_acl_lo;	/* File ACL */
	__le32	i_size_high;
	__le32	i_obso_faddr;	/* Obsoleted fragment address */
	union {
		struct {
			__le16	l_i_blocks_high; /* were l_i_reserved1 */
			__le16	l_i_file_acl_high;
			__le16	l_i_uid_high;	/* these 2 fields */
			__le16	l_i_gid_high;	/* were reserved2[0] */
			__le16	l_i_checksum_lo;/* crc32c(uuid+inum+inode) LE */
			__le16	l_i_reserved;
		} linux2;
		struct {
			__le16	h_i_reserved1;	/* Obsoleted fragment number/size which are removed in ext4 */
			__u16	h_i_mode_high;
			__u16	h_i_uid_high;
			__u16	h_i_gid_high;
			__u32	h_i_author;
		} hurd2;
		struct {
			__le16	h_i_reserved1;	/* Obsoleted fragment number/size which are removed in ext4 */
			__le16	m_i_file_acl_high;
			__u32	m_i_reserved2[2];
		} masix2;
	} osd2;				/* OS dependent 2 */
	__le16	i_extra_isize;
	__le16	i_checksum_hi;	/* crc32c(uuid+inum+inode) BE */
	__le32  i_ctime_extra;  /* extra Change time      (nsec << 2 | epoch) */
	__le32  i_mtime_extra;  /* extra Modification time(nsec << 2 | epoch) */
	__le32  i_atime_extra;  /* extra Access time      (nsec << 2 | epoch) */
	__le32  i_crtime;       /* File Creation time */
	__le32  i_crtime_extra; /* extra FileCreationtime (nsec << 2 | epoch) */
	__le32  i_version_hi;	/* high 32 bits for 64-bit version */
};


/*
 * Inode flags used for atomic set/get
 */
enum {
	EXT4_INODE_SECRM	= 0,	/* Secure deletion */
	EXT4_INODE_UNRM		= 1,	/* Undelete */
	EXT4_INODE_COMPR	= 2,	/* Compress file */
	EXT4_INODE_SYNC		= 3,	/* Synchronous updates */
	EXT4_INODE_IMMUTABLE	= 4,	/* Immutable file */
	EXT4_INODE_APPEND	= 5,	/* writes to file may only append */
	EXT4_INODE_NODUMP	= 6,	/* do not dump file */
	EXT4_INODE_NOATIME	= 7,	/* do not update atime */
/* Reserved for compression usage... */
	EXT4_INODE_DIRTY	= 8,
	EXT4_INODE_COMPRBLK	= 9,	/* One or more compressed clusters */
	EXT4_INODE_NOCOMPR	= 10,	/* Don't compress */
	EXT4_INODE_ECOMPR	= 11,	/* Compression error */
/* End compression flags --- maybe not all used */
	EXT4_INODE_INDEX	= 12,	/* hash-indexed directory */
	EXT4_INODE_IMAGIC	= 13,	/* AFS directory */
	EXT4_INODE_JOURNAL_DATA	= 14,	/* file data should be journaled */
	EXT4_INODE_NOTAIL	= 15,	/* file tail should not be merged */
	EXT4_INODE_DIRSYNC	= 16,	/* dirsync behaviour (directories only) */
	EXT4_INODE_TOPDIR	= 17,	/* Top of directory hierarchies*/
	EXT4_INODE_HUGE_FILE	= 18,	/* Set to each huge file */
	EXT4_INODE_EXTENTS	= 19,	/* Inode uses extents */
	EXT4_INODE_EA_INODE	= 21,	/* Inode used for large EA */
	EXT4_INODE_EOFBLOCKS	= 22,	/* Blocks allocated beyond EOF */
	EXT4_INODE_RESERVED	= 31,	/* reserved for ext4 lib */
};

/*
 * Structure of the super block
 */
struct ext4_super_block {
/*00*/	__le32	s_inodes_count;		/* Inodes count */
	__le32	s_blocks_count_lo;	/* Blocks count */
	__le32	s_r_blocks_count_lo;	/* Reserved blocks count */
	__le32	s_free_blocks_count_lo;	/* Free blocks count */
/*10*/	__le32	s_free_inodes_count;	/* Free inodes count */
	__le32	s_first_data_block;	/* First Data Block */
	__le32	s_log_block_size;	/* Block size */
	__le32	s_log_cluster_size;	/* Allocation cluster size */
/*20*/	__le32	s_blocks_per_group;	/* # Blocks per group */
	__le32	s_clusters_per_group;	/* # Clusters per group */
	__le32	s_inodes_per_group;	/* # Inodes per group */
	__le32	s_mtime;		/* Mount time */
/*30*/	__le32	s_wtime;		/* Write time */
	__le16	s_mnt_count;		/* Mount count */
	__le16	s_max_mnt_count;	/* Maximal mount count */
	__le16	s_magic;		/* Magic signature */
	__le16	s_state;		/* File system state */
	__le16	s_errors;		/* Behaviour when detecting errors */
	__le16	s_minor_rev_level;	/* minor revision level */
/*40*/	__le32	s_lastcheck;		/* time of last check */
	__le32	s_checkinterval;	/* max. time between checks */
	__le32	s_creator_os;		/* OS */
	__le32	s_rev_level;		/* Revision level */
/*50*/	__le16	s_def_resuid;		/* Default uid for reserved blocks */
	__le16	s_def_resgid;		/* Default gid for reserved blocks */
	/*
	 * These fields are for EXT4_DYNAMIC_REV superblocks only.
	 *
	 * Note: the difference between the compatible feature set and
	 * the incompatible feature set is that if there is a bit set
	 * in the incompatible feature set that the kernel doesn't
	 * know about, it should refuse to mount the filesystem.
	 *
	 * e2fsck's requirements are more strict; if it doesn't know
	 * about a feature in either the compatible or incompatible
	 * feature set, it must abort and not try to meddle with
	 * things it doesn't understand...
	 */
	__le32	s_first_ino;		/* First non-reserved inode */
	__le16  s_inode_size;		/* size of inode structure */
	__le16	s_block_group_nr;	/* block group # of this superblock */
	__le32	s_feature_compat;	/* compatible feature set */
/*60*/	__le32	s_feature_incompat;	/* incompatible feature set */
	__le32	s_feature_ro_compat;	/* readonly-compatible feature set */
/*68*/	__u8	s_uuid[16];		/* 128-bit uuid for volume */
/*78*/	char	s_volume_name[16];	/* volume name */
/*88*/	char	s_last_mounted[64];	/* directory where last mounted */
/*C8*/	__le32	s_algorithm_usage_bitmap; /* For compression */
	/*
	 * Performance hints.  Directory preallocation should only
	 * happen if the EXT4_FEATURE_COMPAT_DIR_PREALLOC flag is on.
	 */
	__u8	s_prealloc_blocks;	/* Nr of blocks to try to preallocate*/
	__u8	s_prealloc_dir_blocks;	/* Nr to preallocate for dirs */
	__le16	s_reserved_gdt_blocks;	/* Per group desc for online growth */
	/*
	 * Journaling support valid if EXT4_FEATURE_COMPAT_HAS_JOURNAL set.
	 */
/*D0*/	__u8	s_journal_uuid[16];	/* uuid of journal superblock */
/*E0*/	__le32	s_journal_inum;		/* inode number of journal file */
	__le32	s_journal_dev;		/* device number of journal file */
	__le32	s_last_orphan;		/* start of list of inodes to delete */
	__le32	s_hash_seed[4];		/* HTREE hash seed */
	__u8	s_def_hash_version;	/* Default hash version to use */
	__u8	s_jnl_backup_type;
	__le16  s_desc_size;		/* size of group descriptor */
/*100*/	__le32	s_default_mount_opts;
	__le32	s_first_meta_bg;	/* First metablock block group */
	__le32	s_mkfs_time;		/* When the filesystem was created */
	__le32	s_jnl_blocks[17];	/* Backup of the journal inode */
	/* 64bit support valid if EXT4_FEATURE_COMPAT_64BIT */
/*150*/	__le32	s_blocks_count_hi;	/* Blocks count */
	__le32	s_r_blocks_count_hi;	/* Reserved blocks count */
	__le32	s_free_blocks_count_hi;	/* Free blocks count */
	__le16	s_min_extra_isize;	/* All inodes have at least # bytes */
	__le16	s_want_extra_isize; 	/* New inodes should reserve # bytes */
	__le32	s_flags;		/* Miscellaneous flags */
	__le16  s_raid_stride;		/* RAID stride */
	__le16  s_mmp_update_interval;  /* # seconds to wait in MMP checking */
	__le64  s_mmp_block;            /* Block for multi-mount protection */
	__le32  s_raid_stripe_width;    /* blocks on all data disks (N*stride)*/
	__u8	s_log_groups_per_flex;  /* FLEX_BG group size */
	__u8	s_checksum_type;	/* metadata checksum algorithm used */
	__le16  s_reserved_pad;
	__le64	s_kbytes_written;	/* nr of lifetime kilobytes written */
	__le32	s_snapshot_inum;	/* Inode number of active snapshot */
	__le32	s_snapshot_id;		/* sequential ID of active snapshot */
	__le64	s_snapshot_r_blocks_count; /* reserved blocks for active
					      snapshot's future use */
	__le32	s_snapshot_list;	/* inode number of the head of the
					   on-disk snapshot list */
#define EXT4_S_ERR_START offsetof(struct ext4_super_block, s_error_count)
	__le32	s_error_count;		/* number of fs errors */
	__le32	s_first_error_time;	/* first time an error happened */
	__le32	s_first_error_ino;	/* inode involved in first error */
	__le64	s_first_error_block;	/* block involved of first error */
	__u8	s_first_error_func[32];	/* function where the error happened */
	__le32	s_first_error_line;	/* line number where error happened */
	__le32	s_last_error_time;	/* most recent time of an error */
	__le32	s_last_error_ino;	/* inode involved in last error */
	__le32	s_last_error_line;	/* line number where error happened */
	__le64	s_last_error_block;	/* block involved of last error */
	__u8	s_last_error_func[32];	/* function where the error happened */
#define EXT4_S_ERR_END offsetof(struct ext4_super_block, s_mount_opts)
	__u8	s_mount_opts[64];
	__le32	s_usr_quota_inum;	/* inode for tracking user quota */
	__le32	s_grp_quota_inum;	/* inode for tracking group quota */
	__le32	s_overhead_clusters;	/* overhead blocks/clusters in fs */
	__le32	s_reserved[108];	/* Padding to the end of the block */
	__le32	s_checksum;		/* crc32c(superblock) */
};

/*
 * Revision levels
 */
#define EXT4_GOOD_OLD_REV       0       /* The good old (original) format */
#define EXT4_DYNAMIC_REV        1       /* V2 format w/ dynamic inode sizes */

#define EXT4_CURRENT_REV        EXT4_GOOD_OLD_REV
#define EXT4_MAX_SUPP_REV       EXT4_DYNAMIC_REV

#define EXT4_GOOD_OLD_INODE_SIZE 128

/*
 * Feature set definitions (only the once we need for read support)
 */
#define EXT4_FEATURE_RO_COMPAT_SPARSE_SUPER     0x0001

#define EXT4_FEATURE_INCOMPAT_COMPRESSION	0x0001
#define EXT4_FEATURE_INCOMPAT_FILETYPE		0x0002
#define EXT4_FEATURE_INCOMPAT_RECOVER		0x0004 /* Needs recovery */
#define EXT4_FEATURE_INCOMPAT_JOURNAL_DEV	0x0008 /* Journal device */
#define EXT4_FEATURE_INCOMPAT_META_BG		0x0010
#define EXT4_FEATURE_INCOMPAT_EXTENTS		0x0040 /* extents support */
#define EXT4_FEATURE_INCOMPAT_64BIT		0x0080
#define EXT4_FEATURE_INCOMPAT_MMP               0x0100
#define EXT4_FEATURE_INCOMPAT_FLEX_BG		0x0200
#define EXT4_FEATURE_INCOMPAT_EA_INODE		0x0400 /* EA in inode */
#define EXT4_FEATURE_INCOMPAT_DIRDATA		0x1000 /* data in dirent */
#define EXT4_FEATURE_INCOMPAT_BG_USE_META_CSUM	0x2000 /* use crc32c for bg */
#define EXT4_FEATURE_INCOMPAT_LARGEDIR		0x4000 /* >2GB or 3-lvl htree */
#define EXT4_FEATURE_INCOMPAT_INLINEDATA	0x8000 /* data in inode */

#define EXT4_FEATURE_INCOMPAT_SUPP	(EXT4_FEATURE_INCOMPAT_FILETYPE| \
					 EXT4_FEATURE_INCOMPAT_RECOVER| \
					 EXT4_FEATURE_INCOMPAT_META_BG| \
					 EXT4_FEATURE_INCOMPAT_EXTENTS| \
					 EXT4_FEATURE_INCOMPAT_64BIT| \
					 EXT4_FEATURE_INCOMPAT_FLEX_BG| \
					 EXT4_FEATURE_INCOMPAT_MMP)

/*
 * Structure of a directory entry
 */
#define EXT4_NAME_LEN 255

struct ext4_dir_entry {
    __le32  inode;                  /* Inode number */
    __le16  rec_len;                /* Directory entry length */
    __u8    name_len;               /* Name length */
    __u8    file_type;
    char    name[EXT4_NAME_LEN];    /* File name */
};
// NOTE: The original Linux kernel header defines ext4_dir_entry with the original
//  layout and ext4_dir_entry_2 with the revised layout. We simply use the revised one.

/*
 * Ext2 directory file types.  Only the low 3 bits are used.  The
 * other bits are reserved for now.
 */
enum {
    EXT4_FT_UNKNOWN,
    EXT4_FT_REG_FILE,
    EXT4_FT_DIR,
    EXT4_FT_CHRDEV,
    EXT4_FT_BLKDEV,
    EXT4_FT_FIFO,
    EXT4_FT_SOCK,
    EXT4_FT_SYMLINK,
    EXT4_FT_MAX
};

/*
 * ext4_inode has i_block array (60 bytes total).
 * The first 12 bytes store ext4_extent_header;
 * the remainder stores an array of ext4_extent.
 * For non-inode extent blocks, ext4_extent_tail
 * follows the array.
 */

/*
 * This is the extent tail on-disk structure.
 * All other extent structures are 12 bytes long.  It turns out that
 * block_size % 12 >= 4 for at least all powers of 2 greater than 512, which
 * covers all valid ext4 block sizes.  Therefore, this tail structure can be
 * crammed into the end of the block without having to rebalance the tree.
 */
struct ext4_extent_tail {
	__le32	et_checksum;	/* crc32c(uuid+inum+extent_block) */
};

/*
 * This is the extent on-disk structure.
 * It's used at the bottom of the tree.
 */
struct ext4_extent {
	__le32	ee_block;	/* first logical block extent covers */
	__le16	ee_len;		/* number of blocks covered by extent */
	__le16	ee_start_hi;	/* high 16 bits of physical block */
	__le32	ee_start_lo;	/* low 32 bits of physical block */
};

/*
 * This is index on-disk structure.
 * It's used at all the levels except the bottom.
 */
struct ext4_extent_idx {
	__le32	ei_block;	/* index covers logical blocks from 'block' */
	__le32	ei_leaf_lo;	/* pointer to the physical block of the next *
				 * level. leaf or next index could be there */
	__le16	ei_leaf_hi;	/* high 16 bits of physical block */
	__u16	ei_unused;
};


/*
 * Each block (leaves and indexes), even inode-stored has header.
 */
struct ext4_extent_header {
	__le16	eh_magic;	/* probably will support different formats */
	__le16	eh_entries;	/* number of valid entries */
	__le16	eh_max;		/* capacity of store in entries */
	__le16	eh_depth;	/* has tree real underlying blocks? */
	__le32	eh_generation;	/* generation of the tree */
};

#define EXT4_EXT_MAGIC		(0xf30a)


#endif
