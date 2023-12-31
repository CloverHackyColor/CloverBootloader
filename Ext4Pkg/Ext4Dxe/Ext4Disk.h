/** @file
  Raw filesystem data structures

  Copyright (c) 2021 - 2023 Pedro Falcato All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent

  Layout of an EXT2/3/4 filesystem:
  (note: this driver has been developed using
   https://www.kernel.org/doc/html/latest/filesystems/ext4/index.html as
   documentation).

  An ext2/3/4 filesystem (here on out referred to as simply an ext4 filesystem,
  due to the similarities) is composed of various concepts:

  1) Superblock
     The superblock is the structure near (1024 bytes offset from the start)
     the start of the partition, and describes the filesystem in general.
     Here, we get to know the size of the filesystem's blocks, which features
     it supports or not, whether it's been cleanly unmounted, how many blocks
     we have, etc.

  2) Block groups
     EXT4 filesystems are divided into block groups, and each block group covers
     s_blocks_per_group(8 * Block Size) blocks. Each block group has an
     associated block group descriptor; these are present directly after the
     superblock. Each block group descriptor contains the location of the
     inode table, and the inode and block bitmaps (note these bitmaps are only
     a block long, which gets us the 8 * Block Size formula covered previously).

  3) Blocks
     The ext4 filesystem is divided in blocks, of size s_log_block_size ^ 1024.
     Blocks can be allocated using individual block groups's bitmaps. Note
     that block 0 is invalid and its presence on extents/block tables means
     it's part of a file hole, and that particular location must be read as
     a block full of zeros.

  4) Inodes
     The ext4 filesystem divides files/directories into inodes (originally
     index nodes). Each file/socket/symlink/directory/etc (here on out referred
     to as a file, since there is no distinction under the ext4 filesystem) is
     stored as a /nameless/ inode, that is stored in some block group's inode
     table. Each inode has s_inode_size size (or GOOD_OLD_INODE_SIZE if it's
     an old filesystem), and holds various metadata about the file. Since the
     largest inode structure right now is ~160 bytes, the rest of the inode
     contains inline extended attributes. Inodes' data is stored using either
     data blocks (under ext2/3) or extents (under ext4).

  5) Extents
     Ext4 inodes store data in extents. These let N contiguous logical blocks
     that are represented by N contiguous physical blocks be represented by a
     single extent structure, which minimizes filesystem metadata bloat and
     speeds up block mapping (particularly due to the fact that high-quality
     ext4 implementations like linux's try /really/ hard to make the file
     contiguous, so it's common to have files with almost 0 fragmentation).
     Inodes that use extents store them in a tree, and the top of the tree
     is stored on i_data. The tree's leaves always start with an
     EXT4_EXTENT_HEADER and contain EXT4_EXTENT_INDEX on eh_depth != 0 and
     EXT4_EXTENT on eh_depth = 0; these entries are always sorted by logical
     block.

  6) Directories
     Ext4 directories are files that store name -> inode mappings for the
     logical directory; this is where files get their names, which means ext4
     inodes do not themselves have names, since they can be linked (present)
     multiple times with different names. Directories can store entries in two
     different ways:
       1) Classical linear directories: They store entries as a mostly-linked
          mostly-list of EXT4_DIR_ENTRY.
       2) Hash tree directories: These are used for larger directories, with
          hundreds of entries, and are designed in a backwards compatible way.
          These are not yet implemented in the Ext4Dxe driver.

  7) Journal
     Ext3/4 filesystems have a journal to help protect the filesystem against
     system crashes. This is not yet implemented in Ext4Dxe but is described
     in detail in the Linux kernel's documentation.
**/

#ifndef EXT4_DISK_H_
#define EXT4_DISK_H_

#include <Uefi.h>

#define EXT4_SUPERBLOCK_OFFSET  1024U

#define EXT4_SIGNATURE  0xEF53U

#define EXT4_FS_STATE_UNMOUNTED           0x1
#define EXT4_FS_STATE_ERRORS_DETECTED     0x2
#define EXT4_FS_STATE_RECOVERING_ORPHANS  0x4

#define EXT4_ERRORS_CONTINUE  1
#define EXT4_ERRORS_RO        2
#define EXT4_ERRORS_PANIC     3

#define EXT4_LINUX_ID     0
#define EXT4_GNU_HURD_ID  1
#define EXT4_MASIX_ID     2
#define EXT4_FREEBSD_ID   3
#define EXT4_LITES_ID     4

#define EXT4_GOOD_OLD_REV  0
#define EXT4_DYNAMIC_REV   1

#define EXT4_CHECKSUM_CRC32C  0x1

#define EXT4_FEATURE_COMPAT_DIR_PREALLOC   0x01
#define EXT4_FEATURE_COMPAT_IMAGIC_INODES  0x02
#define EXT3_FEATURE_COMPAT_HAS_JOURNAL    0x04
#define EXT4_FEATURE_COMPAT_EXT_ATTR       0x08
#define EXT4_FEATURE_COMPAT_RESIZE_INO     0x10
#define EXT4_FEATURE_COMPAT_DIR_INDEX      0x20

#define EXT4_FEATURE_INCOMPAT_COMPRESSION  0x00001
#define EXT4_FEATURE_INCOMPAT_FILETYPE     0x00002
#define EXT4_FEATURE_INCOMPAT_RECOVER      0x00004
#define EXT4_FEATURE_INCOMPAT_JOURNAL_DEV  0x00008
#define EXT4_FEATURE_INCOMPAT_META_BG      0x00010
#define EXT4_FEATURE_INCOMPAT_EXTENTS      0x00040
#define EXT4_FEATURE_INCOMPAT_64BIT        0x00080
#define EXT4_FEATURE_INCOMPAT_MMP          0x00100
#define EXT4_FEATURE_INCOMPAT_FLEX_BG      0x00200
#define EXT4_FEATURE_INCOMPAT_EA_INODE     0x00400
// It's not clear whether or not this feature (below) is used right now
#define EXT4_FEATURE_INCOMPAT_DIRDATA      0x01000
#define EXT4_FEATURE_INCOMPAT_CSUM_SEED    0x02000
#define EXT4_FEATURE_INCOMPAT_LARGEDIR     0x04000
#define EXT4_FEATURE_INCOMPAT_INLINE_DATA  0x08000
#define EXT4_FEATURE_INCOMPAT_ENCRYPT      0x10000

#define EXT4_FEATURE_RO_COMPAT_SPARSE_SUPER   0x0001
#define EXT4_FEATURE_RO_COMPAT_LARGE_FILE     0x0002
#define EXT4_FEATURE_RO_COMPAT_BTREE_DIR      0x0004// Unused
#define EXT4_FEATURE_RO_COMPAT_HUGE_FILE      0x0008
#define EXT4_FEATURE_RO_COMPAT_GDT_CSUM       0x0010
#define EXT4_FEATURE_RO_COMPAT_DIR_NLINK      0x0020
#define EXT4_FEATURE_RO_COMPAT_EXTRA_ISIZE    0x0040
#define EXT4_FEATURE_RO_COMPAT_HAS_SNAPSHOT   0x0080// Not implemented in ext4
#define EXT4_FEATURE_RO_COMPAT_QUOTA          0x0100
#define EXT4_FEATURE_RO_COMPAT_BIGALLOC       0x0200
#define EXT4_FEATURE_RO_COMPAT_METADATA_CSUM  0x0400
#define EXT4_FEATURE_RO_COMPAT_REPLICA        0x0800// Not used

// We explicitly don't recognise this, so we get read only.
#define EXT4_FEATURE_RO_COMPAT_READONLY  0x1000
#define EXT4_FEATURE_RO_COMPAT_PROJECT   0x2000

/* Important notes about the features
 * Absolutely needed features:
 *    1) Every incompat, because we might want to mount root filesystems
 *    2) Relevant RO_COMPATs(I'm not sure of what to do wrt quota, project)
 **/

#define EXT4_INO_TYPE_FIFO       0x1000
#define EXT4_INO_TYPE_CHARDEV    0x2000
#define EXT4_INO_TYPE_DIR        0x4000
#define EXT4_INO_TYPE_BLOCKDEV   0x6000
#define EXT4_INO_TYPE_REGFILE    0x8000
#define EXT4_INO_TYPE_SYMLINK    0xA000
#define EXT4_INO_TYPE_UNIX_SOCK  0xC000

/* Inode flags */
#define EXT4_SECRM_FL         0x00000001
#define EXT4_UNRM_FL          0x00000002
#define EXT4_COMPR_FL         0x00000004
#define EXT4_SYNC_FL          0x00000008
#define EXT4_IMMUTABLE_FL     0x00000010
#define EXT4_APPEND_FL        0x00000020
#define EXT4_NODUMP_FL        0x00000040
#define EXT4_NOATIME_FL       0x00000080
#define EXT4_DIRTY_FL         0x00000100
#define EXT4_COMPRBLK_FL      0x00000200
#define EXT4_NOCOMPR_FL       0x00000400
#define EXT4_ENCRYPT_FL       0x00000800
#define EXT4_BTREE_FL         0x00001000
#define EXT4_INDEX_FL         0x00002000
#define EXT4_JOURNAL_DATA_FL  0x00004000
#define EXT4_NOTAIL_FL        0x00008000
#define EXT4_DIRSYNC_FL       0x00010000
#define EXT4_TOPDIR_FL        0x00020000
#define EXT4_HUGE_FILE_FL     0x00040000
#define EXT4_EXTENTS_FL       0x00080000
#define EXT4_VERITY_FL        0x00100000
#define EXT4_EA_INODE_FL      0x00200000
#define EXT4_RESERVED_FL      0x80000000

/* File type flags that are stored in the directory entries */
#define EXT4_FT_UNKNOWN   0
#define EXT4_FT_REG_FILE  1
#define EXT4_FT_DIR       2
#define EXT4_FT_CHRDEV    3
#define EXT4_FT_BLKDEV    4
#define EXT4_FT_FIFO      5
#define EXT4_FT_SOCK      6
#define EXT4_FT_SYMLINK   7

typedef struct {
  UINT32    s_inodes_count;
  UINT32    s_blocks_count;
  UINT32    s_r_blocks_count;
  UINT32    s_free_blocks_count;
  UINT32    s_free_inodes_count;
  UINT32    s_first_data_block;
  UINT32    s_log_block_size;
  UINT32    s_log_frag_size;
  UINT32    s_blocks_per_group;
  UINT32    s_frags_per_group;
  UINT32    s_inodes_per_group;
  UINT32    s_mtime;
  UINT32    s_wtime;
  UINT16    s_mnt_count;
  UINT16    s_max_mnt_count;
  UINT16    s_magic;
  UINT16    s_state;
  UINT16    s_errors;
  UINT16    s_minor_rev_level;
  UINT32    s_lastcheck;
  UINT32    s_check_interval;
  UINT32    s_creator_os;
  UINT32    s_rev_level;
  UINT16    s_def_resuid;
  UINT16    s_def_resgid;

  /* Every field after this comment is revision >= 1 */

  UINT32    s_first_ino;
  UINT16    s_inode_size;
  UINT16    s_block_group_nr;
  UINT32    s_feature_compat;
  UINT32    s_feature_incompat;
  UINT32    s_feature_ro_compat;
  UINT8     s_uuid[16];
  UINT8     s_volume_name[16];
  UINT8     s_last_mounted[64];
  UINT32    s_algo_bitmap;
  UINT8     s_prealloc_blocks;
  UINT8     s_prealloc_dir_blocks;
  UINT16    unused;
  UINT8     s_journal_uuid[16];
  UINT32    s_journal_inum;
  UINT32    s_journal_dev;
  UINT32    s_last_orphan;
  UINT32    s_hash_seed[4];
  UINT8     s_def_hash_version;
  UINT8     s_jnl_backup_type;
  UINT16    s_desc_size;
  UINT32    s_default_mount_options;
  UINT32    s_first_meta_bg;
  UINT32    s_mkfs_time;
  UINT32    s_jnl_blocks[17];
  UINT32    s_blocks_count_hi;
  UINT32    s_r_blocks_count_hi;
  UINT32    s_free_blocks_count_hi;
  UINT16    s_min_extra_isize;
  UINT16    s_want_extra_isize;
  UINT32    s_flags;
  UINT16    s_raid_stride;
  UINT16    s_mmp_interval;
  UINT64    s_mmp_block;
  UINT32    s_raid_stride_width;
  UINT8     s_log_groups_per_flex;
  UINT8     s_checksum_type; // Only valid value is 1 - CRC32C
  UINT16    s_reserved_pad;
  UINT64    s_kbytes_written;

  // Snapshot stuff isn't used in Linux and isn't implemented here
  UINT32    s_snapshot_inum;
  UINT32    s_snapshot_id;
  UINT64    s_snapshot_r_blocks_count;
  UINT32    s_snapshot_list;
  UINT32    s_error_count;
  UINT32    s_first_error_time;
  UINT32    s_first_error_ino;
  UINT64    s_first_error_block;
  UINT8     s_first_error_func[32];
  UINT32    s_first_error_line;
  UINT32    s_last_error_time;
  UINT32    s_last_error_ino;
  UINT32    s_last_error_line;
  UINT64    s_last_error_block;
  UINT8     s_last_error_func[32];
  UINT8     s_mount_opts[64];
  UINT32    s_usr_quota_inum;
  UINT32    s_grp_quota_inum;
  UINT32    s_overhead_blocks;
  UINT32    s_backup_bgs[2]; // sparse_super2
  UINT8     s_encrypt_algos[4];
  UINT8     s_encrypt_pw_salt[16];
  UINT32    s_lpf_ino;
  UINT32    s_prj_quota_inum;
  UINT32    s_checksum_seed;
  UINT32    s_reserved[98];
  UINT32    s_checksum;
} EXT4_SUPERBLOCK;

STATIC_ASSERT (
  sizeof (EXT4_SUPERBLOCK) == 1024,
  "ext4 superblock struct has incorrect size"
  );

typedef struct {
  UINT32    bg_block_bitmap_lo;
  UINT32    bg_inode_bitmap_lo;
  UINT32    bg_inode_table_lo;
  UINT16    bg_free_blocks_count_lo;
  UINT16    bg_free_inodes_count_lo;
  UINT16    bg_used_dirs_count_lo;
  UINT16    bg_flags;
  UINT32    bg_exclude_bitmap_lo;
  UINT16    bg_block_bitmap_csum_lo;
  UINT16    bg_inode_bitmap_csum_lo;
  UINT16    bg_itable_unused_lo;
  UINT16    bg_checksum;
  UINT32    bg_block_bitmap_hi;
  UINT32    bg_inode_bitmap_hi;
  UINT32    bg_inode_table_hi;
  UINT16    bg_free_blocks_count_hi;
  UINT16    bg_free_inodes_count_hi;
  UINT16    bg_used_dirs_count_hi;
  UINT16    bg_itable_unused_hi;
  UINT32    bg_exclude_bitmap_hi;
  UINT16    bg_block_bitmap_csum_hi;
  UINT16    bg_inode_bitmap_csum_hi;
  UINT32    bg_reserved;
} EXT4_BLOCK_GROUP_DESC;

#define EXT4_OLD_BLOCK_DESC_SIZE    32
#define EXT4_64BIT_BLOCK_DESC_SIZE  64

STATIC_ASSERT (
  sizeof (EXT4_BLOCK_GROUP_DESC) == EXT4_64BIT_BLOCK_DESC_SIZE,
  "ext4 block group descriptor struct has incorrect size"
  );

#define EXT4_DBLOCKS                12
#define EXT4_IND_BLOCK              12
#define EXT4_DIND_BLOCK             13
#define EXT4_TIND_BLOCK             14
#define EXT4_NR_BLOCKS              15
#define EXT4_FAST_SYMLINK_MAX_SIZE  EXT4_NR_BLOCKS * sizeof(UINT32)

#define EXT4_GOOD_OLD_INODE_SIZE  128U

typedef struct _Ext4_I_OSD2_Linux {
  UINT16    l_i_blocks_high;
  UINT16    l_i_file_acl_high;
  UINT16    l_i_uid_high;
  UINT16    l_i_gid_high;
  UINT16    l_i_checksum_lo;
  UINT16    l_i_reserved;
} EXT4_OSD2_LINUX;

typedef struct _Ext4_I_OSD2_Hurd {
  UINT16    h_i_reserved1;
  UINT16    h_i_mode_high;
  UINT16    h_i_uid_high;
  UINT16    h_i_gid_high;
  UINT32    h_i_author;
} EXT4_OSD2_HURD;

typedef union {
  // Note: Toolchain-specific defines (such as "linux") stops us from using
  // simpler names down here.
  EXT4_OSD2_LINUX    data_linux;
  EXT4_OSD2_HURD     data_hurd;
} EXT4_OSD2;

typedef struct _Ext4Inode {
  UINT16       i_mode;
  UINT16       i_uid;
  UINT32       i_size_lo;
  UINT32       i_atime;
  UINT32       i_ctime;
  UINT32       i_mtime;
  UINT32       i_dtime;
  UINT16       i_gid;
  UINT16       i_links;
  UINT32       i_blocks;
  UINT32       i_flags;
  UINT32       i_os_spec;
  UINT32       i_data[EXT4_NR_BLOCKS];
  UINT32       i_generation;
  UINT32       i_file_acl;
  UINT32       i_size_hi;
  UINT32       i_faddr;

  EXT4_OSD2    i_osd2;

  UINT16       i_extra_isize;
  UINT16       i_checksum_hi;
  UINT32       i_ctime_extra;
  UINT32       i_mtime_extra;
  UINT32       i_atime_extra;
  UINT32       i_crtime;
  UINT32       i_crtime_extra;
  UINT32       i_version_hi;
  UINT32       i_projid;
} EXT4_INODE;

#define EXT4_NAME_MAX  255

typedef struct {
  // offset 0x0: inode number (if 0, unused entry, should skip.)
  UINT32    inode;
  // offset 0x4: Directory entry's length.
  //             Note: rec_len >= name_len + EXT4_MIN_DIR_ENTRY_LEN and rec_len % 4 == 0.
  UINT16    rec_len;
  // offset 0x6: Directory entry's name's length
  UINT8     name_len;
  // offset 0x7: Directory entry's file type indicator
  UINT8     file_type;
  // offset 0x8: name[name_len]: Variable length character array; not null-terminated.
  CHAR8     name[EXT4_NAME_MAX];
  // Further notes on names:
  // 1) We use EXT4_NAME_MAX here instead of flexible arrays for ease of use around the driver.
  //
  // 2) ext4 directories are defined, as the documentation puts it, as:
  // "a directory is more or less a flat file that maps an arbitrary byte string
  // (usually ASCII) to an inode number on the filesystem". So, they are not
  // necessarily encoded with ASCII, UTF-8, or any of the sort. We must treat it
  // as a bag of bytes. When interacting with EFI interfaces themselves (which expect UCS-2)
  // we skip any directory entry that is not valid UTF-8.
} EXT4_DIR_ENTRY;

#define EXT4_MIN_DIR_ENTRY_LEN  8

// This on-disk structure is present at the bottom of the extent tree
typedef struct {
  // First logical block
  UINT32    ee_block;
  // Length of the extent, in blocks
  UINT16    ee_len;
  // The physical (filesystem-relative) block is split between the high 16 bits
  // and the low 32 bits - this forms a 48-bit block number
  UINT16    ee_start_hi;
  UINT32    ee_start_lo;
} EXT4_EXTENT;

// This on-disk structure is present at all levels except the bottom
typedef struct {
  // This index covers logical blocks from 'ei_block'
  UINT32    ei_block;
  // Block of the next level of the extent tree, similarly split in a high and
  // low portion.
  UINT32    ei_leaf_lo;
  UINT16    ei_leaf_hi;

  UINT16    ei_unused;
} EXT4_EXTENT_INDEX;

typedef struct {
  // Needs to be EXT4_EXTENT_HEADER_MAGIC
  UINT16    eh_magic;
  // Number of entries
  UINT16    eh_entries;
  // Maximum number of entries that could follow this header
  UINT16    eh_max;
  // Depth of this node in the tree - the tree can be at most 5 levels deep
  UINT16    eh_depth;
  // Unused by standard ext4
  UINT32    eh_generation;
} EXT4_EXTENT_HEADER;

#define EXT4_EXTENT_HEADER_MAGIC  0xF30A

// Specified by ext4 docs and backed by a bunch of math
#define EXT4_EXTENT_TREE_MAX_DEPTH  5

typedef struct {
  // CRC32C of UUID + inode number + igeneration + extent block
  UINT32    eb_checksum;
} EXT4_EXTENT_TAIL;

/**
 * EXT4 has this feature called uninitialized extents:
 * An extent has a maximum of 32768 blocks (2^15 or 1 << 15).
 * When we find an extent with > 32768 blocks, this extent is called
 * uninitialized. Long story short, it's an extent that behaves as a file hole
 * but has blocks already allocated.
 */
#define EXT4_EXTENT_MAX_INITIALIZED  (1 << 15)

typedef UINT64  EXT4_BLOCK_NR;
typedef UINT32  EXT2_BLOCK_NR;
typedef UINT32  EXT4_INO_NR;

/* Special inode numbers */
#define EXT4_ROOT_INODE_NR         2
#define EXT4_USR_QUOTA_INODE_NR    3
#define EXT4_GRP_QUOTA_INODE_NR    4
#define EXT4_BOOT_LOADER_INODE_NR  5
#define EXT4_UNDEL_DIR_INODE_NR    6
#define EXT4_RESIZE_INODE_NR       7
#define EXT4_JOURNAL_INODE_NR      8

/* First non-reserved inode for old ext4 filesystems */
#define EXT4_GOOD_OLD_FIRST_INODE_NR  11

#define EXT4_BLOCK_FILE_HOLE  0

#endif
