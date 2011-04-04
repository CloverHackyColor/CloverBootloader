/**
 * \file fsw_ext2_disk.h
 * ext2 file system on-disk structures.
 */

/*-
 * Copyright (c) 2006 Christoph Pfisterer
 * Portions Copyright (c) 1991-2006 by various Linux kernel contributors
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

#ifndef _FSW_EXT2_DISK_H_
#define _FSW_EXT2_DISK_H_

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
// from Linux kernel, include/linux/ext2_fs.h
//

/*
 * Special inode numbers
 */
#define EXT2_BAD_INO             1      /* Bad blocks inode */
#define EXT2_ROOT_INO            2      /* Root inode */
#define EXT2_BOOT_LOADER_INO     5      /* Boot loader inode */
#define EXT2_UNDEL_DIR_INO       6      /* Undelete directory inode */

/*
 * The second extended file system magic number
 */
#define EXT2_SUPER_MAGIC        0xEF53

/*
 * Macro-instructions used to manage several block sizes
 */
#define EXT2_MIN_BLOCK_SIZE             1024
#define EXT2_MAX_BLOCK_SIZE             4096
#define EXT2_MIN_BLOCK_LOG_SIZE           10
#define EXT2_BLOCK_SIZE(s)              (EXT2_MIN_BLOCK_SIZE << (s)->s_log_block_size)
#define EXT2_ADDR_PER_BLOCK(s)          (EXT2_BLOCK_SIZE(s) / sizeof (__u32))
#define EXT2_BLOCK_SIZE_BITS(s)         ((s)->s_log_block_size + 10)
#define EXT2_INODE_SIZE(s)      (((s)->s_rev_level == EXT2_GOOD_OLD_REV) ? \
                                 EXT2_GOOD_OLD_INODE_SIZE : \
                                 (s)->s_inode_size)
#define EXT2_FIRST_INO(s)       (((s)->s_rev_level == EXT2_GOOD_OLD_REV) ? \
                                 EXT2_GOOD_OLD_FIRST_INO : \
                                 (s)->s_first_ino)

/*
 * Structure of a blocks group descriptor
 */
struct ext2_group_desc
{
    __le32  bg_block_bitmap;        /* Blocks bitmap block */
    __le32  bg_inode_bitmap;        /* Inodes bitmap block */
    __le32  bg_inode_table;         /* Inodes table block */
    __le16  bg_free_blocks_count;   /* Free blocks count */
    __le16  bg_free_inodes_count;   /* Free inodes count */
    __le16  bg_used_dirs_count;     /* Directories count */
    __le16  bg_pad;
    __le32  bg_reserved[3];
};

/*
 * Macro-instructions used to manage group descriptors
 */
#define EXT2_BLOCKS_PER_GROUP(s)       ((s)->s_blocks_per_group)
#define EXT2_DESC_PER_BLOCK(s)         (EXT2_BLOCK_SIZE(s) / sizeof (struct ext2_group_desc))
#define EXT2_INODES_PER_GROUP(s)       ((s)->s_inodes_per_group)

/*
 * Constants relative to the data blocks
 */
#define EXT2_NDIR_BLOCKS                12
#define EXT2_IND_BLOCK                  EXT2_NDIR_BLOCKS
#define EXT2_DIND_BLOCK                 (EXT2_IND_BLOCK + 1)
#define EXT2_TIND_BLOCK                 (EXT2_DIND_BLOCK + 1)
#define EXT2_N_BLOCKS                   (EXT2_TIND_BLOCK + 1)

/*
 * Inode flags
 */
#define EXT2_SECRM_FL                   0x00000001 /* Secure deletion */
#define EXT2_UNRM_FL                    0x00000002 /* Undelete */
#define EXT2_COMPR_FL                   0x00000004 /* Compress file */
#define EXT2_SYNC_FL                    0x00000008 /* Synchronous updates */
#define EXT2_IMMUTABLE_FL               0x00000010 /* Immutable file */
#define EXT2_APPEND_FL                  0x00000020 /* writes to file may only append */
#define EXT2_NODUMP_FL                  0x00000040 /* do not dump file */
#define EXT2_NOATIME_FL                 0x00000080 /* do not update atime */
/* Reserved for compression usage... */
#define EXT2_DIRTY_FL                   0x00000100
#define EXT2_COMPRBLK_FL                0x00000200 /* One or more compressed clusters */
#define EXT2_NOCOMP_FL                  0x00000400 /* Don't compress */
#define EXT2_ECOMPR_FL                  0x00000800 /* Compression error */
/* End compression flags --- maybe not all used */      
#define EXT2_BTREE_FL                   0x00001000 /* btree format dir */
#define EXT2_INDEX_FL                   0x00001000 /* hash-indexed directory */
#define EXT2_IMAGIC_FL                  0x00002000 /* AFS directory */
#define EXT2_JOURNAL_DATA_FL            0x00004000 /* Reserved for ext3 */
#define EXT2_NOTAIL_FL                  0x00008000 /* file tail should not be merged */
#define EXT2_DIRSYNC_FL                 0x00010000 /* dirsync behaviour (directories only) */
#define EXT2_TOPDIR_FL                  0x00020000 /* Top of directory hierarchies*/
#define EXT2_RESERVED_FL                0x80000000 /* reserved for ext2 lib */

#define EXT2_FL_USER_VISIBLE            0x0003DFFF /* User visible flags */
#define EXT2_FL_USER_MODIFIABLE         0x000380FF /* User modifiable flags */

/*
 * Structure of an inode on the disk
 */
struct ext2_inode {
    __le16  i_mode;         /* 0: File mode */
    __le16  i_uid;          /* 2: Low 16 bits of Owner Uid */
    __le32  i_size;         /* 4: Size in bytes */
    __le32  i_atime;        /* 8: Access time */
    __le32  i_ctime;        /* 12: Creation time */
    __le32  i_mtime;        /* 16: Modification time */
    __le32  i_dtime;        /* 20: Deletion Time */
    __le16  i_gid;          /* 24: Low 16 bits of Group Id */
    __le16  i_links_count;  /* 26: Links count */
    __le32  i_blocks;       /* 28: Blocks count */
    __le32  i_flags;        /* 32: File flags */
    union {
        struct {
            __le32  l_i_reserved1;
        } linux1;
        struct {
            __le32  h_i_translator;
        } hurd1;
        struct {
            __le32  m_i_reserved1;
        } masix1;
    } osd1;                         /* 36: OS dependent 1 */
    __le32  i_block[EXT2_N_BLOCKS];/* 40: Pointers to blocks */
    __le32  i_generation;   /* 100: File version (for NFS) */
    __le32  i_file_acl;     /* 104: File ACL */
    __le32  i_dir_acl;      /* 108: Directory ACL */
    __le32  i_faddr;        /* 112: Fragment address */
    union {
        struct {
            __u8    l_i_frag;       /* 116: Fragment number */
            __u8    l_i_fsize;      /* 117: Fragment size */
            __u16   i_pad1;
            __le16  l_i_uid_high;   /* 120: these 2 fields    */
            __le16  l_i_gid_high;   /* 122: were reserved2[0] */
            __u32   l_i_reserved2;
        } linux2;
        struct {
            __u8    h_i_frag;       /* Fragment number */
            __u8    h_i_fsize;      /* Fragment size */
            __le16  h_i_mode_high;
            __le16  h_i_uid_high;
            __le16  h_i_gid_high;
            __le32  h_i_author;
        } hurd2;
        struct {
            __u8    m_i_frag;       /* Fragment number */
            __u8    m_i_fsize;      /* Fragment size */
            __u16   m_pad1;
            __u32   m_i_reserved2[2];
        } masix2;
    } osd2;                         /* OS dependent 2 */
};

#define i_size_high     i_dir_acl

/*
 * Structure of the super block
 */
struct ext2_super_block {
    __le32  s_inodes_count;         /* Inodes count */
    __le32  s_blocks_count;         /* Blocks count */
    __le32  s_r_blocks_count;       /* Reserved blocks count */
    __le32  s_free_blocks_count;    /* Free blocks count */
    __le32  s_free_inodes_count;    /* Free inodes count */
    __le32  s_first_data_block;     /* First Data Block */
    __le32  s_log_block_size;       /* Block size */
    __le32  s_log_frag_size;        /* Fragment size */
    __le32  s_blocks_per_group;     /* # Blocks per group */
    __le32  s_frags_per_group;      /* # Fragments per group */
    __le32  s_inodes_per_group;     /* # Inodes per group */
    __le32  s_mtime;                /* Mount time */
    __le32  s_wtime;                /* Write time */
    __le16  s_mnt_count;            /* Mount count */
    __le16  s_max_mnt_count;        /* Maximal mount count */
    __le16  s_magic;                /* Magic signature */
    __le16  s_state;                /* File system state */
    __le16  s_errors;               /* Behaviour when detecting errors */
    __le16  s_minor_rev_level;      /* minor revision level */
    __le32  s_lastcheck;            /* time of last check */
    __le32  s_checkinterval;        /* max. time between checks */
    __le32  s_creator_os;           /* OS */
    __le32  s_rev_level;            /* Revision level */
    __le16  s_def_resuid;           /* Default uid for reserved blocks */
    __le16  s_def_resgid;           /* Default gid for reserved blocks */
    /*
     * These fields are for EXT2_DYNAMIC_REV superblocks only.
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
    __le32  s_first_ino;            /* First non-reserved inode */
    __le16  s_inode_size;           /* size of inode structure */
    __le16  s_block_group_nr;       /* block group # of this superblock */
    __le32  s_feature_compat;       /* compatible feature set */
    __le32  s_feature_incompat;     /* incompatible feature set */
    __le32  s_feature_ro_compat;    /* readonly-compatible feature set */
    __u8    s_uuid[16];             /* 128-bit uuid for volume */
    char    s_volume_name[16];      /* volume name */
    char    s_last_mounted[64];     /* directory where last mounted */
    __le32  s_algorithm_usage_bitmap; /* For compression */
    /*
     * Performance hints.  Directory preallocation should only
     * happen if the EXT2_COMPAT_PREALLOC flag is on.
     */
    __u8    s_prealloc_blocks;      /* Nr of blocks to try to preallocate*/
    __u8    s_prealloc_dir_blocks;  /* Nr to preallocate for dirs */
    __u16   s_padding1;
    /*
     * Journaling support valid if EXT3_FEATURE_COMPAT_HAS_JOURNAL set.
     */
    __u8    s_journal_uuid[16];     /* uuid of journal superblock */
    __u32   s_journal_inum;         /* inode number of journal file */
    __u32   s_journal_dev;          /* device number of journal file */
    __u32   s_last_orphan;          /* start of list of inodes to delete */
    __u32   s_hash_seed[4];         /* HTREE hash seed */
    __u8    s_def_hash_version;     /* Default hash version to use */
    __u8    s_reserved_char_pad;
    __u16   s_reserved_word_pad;
    __le32  s_default_mount_opts;
    __le32  s_first_meta_bg;        /* First metablock block group */
    __u32   s_reserved[190];        /* Padding to the end of the block */
};

/*
 * Revision levels
 */
#define EXT2_GOOD_OLD_REV       0       /* The good old (original) format */
#define EXT2_DYNAMIC_REV        1       /* V2 format w/ dynamic inode sizes */

#define EXT2_CURRENT_REV        EXT2_GOOD_OLD_REV
#define EXT2_MAX_SUPP_REV       EXT2_DYNAMIC_REV

#define EXT2_GOOD_OLD_INODE_SIZE 128

/*
 * Feature set definitions
 */

#define EXT2_HAS_COMPAT_FEATURE(sb,mask)                        \
        ( EXT2_SB(sb)->s_es->s_feature_compat & cpu_to_le32(mask) )
#define EXT2_HAS_RO_COMPAT_FEATURE(sb,mask)                     \
        ( EXT2_SB(sb)->s_es->s_feature_ro_compat & cpu_to_le32(mask) )
#define EXT2_HAS_INCOMPAT_FEATURE(sb,mask)                      \
        ( EXT2_SB(sb)->s_es->s_feature_incompat & cpu_to_le32(mask) )
#define EXT2_SET_COMPAT_FEATURE(sb,mask)                        \
        EXT2_SB(sb)->s_es->s_feature_compat |= cpu_to_le32(mask)
#define EXT2_SET_RO_COMPAT_FEATURE(sb,mask)                     \
        EXT2_SB(sb)->s_es->s_feature_ro_compat |= cpu_to_le32(mask)
#define EXT2_SET_INCOMPAT_FEATURE(sb,mask)                      \
        EXT2_SB(sb)->s_es->s_feature_incompat |= cpu_to_le32(mask)
#define EXT2_CLEAR_COMPAT_FEATURE(sb,mask)                      \
        EXT2_SB(sb)->s_es->s_feature_compat &= ~cpu_to_le32(mask)
#define EXT2_CLEAR_RO_COMPAT_FEATURE(sb,mask)                   \
        EXT2_SB(sb)->s_es->s_feature_ro_compat &= ~cpu_to_le32(mask)
#define EXT2_CLEAR_INCOMPAT_FEATURE(sb,mask)                    \
        EXT2_SB(sb)->s_es->s_feature_incompat &= ~cpu_to_le32(mask)

#define EXT2_FEATURE_COMPAT_DIR_PREALLOC        0x0001
#define EXT2_FEATURE_COMPAT_IMAGIC_INODES       0x0002
#define EXT3_FEATURE_COMPAT_HAS_JOURNAL         0x0004
#define EXT2_FEATURE_COMPAT_EXT_ATTR            0x0008
#define EXT2_FEATURE_COMPAT_RESIZE_INO          0x0010
#define EXT2_FEATURE_COMPAT_DIR_INDEX           0x0020
#define EXT2_FEATURE_COMPAT_ANY                 0xffffffff

#define EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER     0x0001
#define EXT2_FEATURE_RO_COMPAT_LARGE_FILE       0x0002
#define EXT2_FEATURE_RO_COMPAT_BTREE_DIR        0x0004
#define EXT2_FEATURE_RO_COMPAT_ANY              0xffffffff

#define EXT2_FEATURE_INCOMPAT_COMPRESSION       0x0001
#define EXT2_FEATURE_INCOMPAT_FILETYPE          0x0002
#define EXT3_FEATURE_INCOMPAT_RECOVER           0x0004
#define EXT3_FEATURE_INCOMPAT_JOURNAL_DEV       0x0008
#define EXT2_FEATURE_INCOMPAT_META_BG           0x0010
#define EXT2_FEATURE_INCOMPAT_ANY               0xffffffff

/*
#define EXT2_FEATURE_COMPAT_SUPP        EXT2_FEATURE_COMPAT_EXT_ATTR
#define EXT2_FEATURE_INCOMPAT_SUPP      (EXT2_FEATURE_INCOMPAT_FILETYPE| \
                                         EXT2_FEATURE_INCOMPAT_META_BG)
#define EXT2_FEATURE_RO_COMPAT_SUPP     (EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER| \
                                         EXT2_FEATURE_RO_COMPAT_LARGE_FILE| \
                                         EXT2_FEATURE_RO_COMPAT_BTREE_DIR)
#define EXT2_FEATURE_RO_COMPAT_UNSUPPORTED      ~EXT2_FEATURE_RO_COMPAT_SUPP
#define EXT2_FEATURE_INCOMPAT_UNSUPPORTED       ~EXT2_FEATURE_INCOMPAT_SUPP
*/

/*
 * Structure of a directory entry
 */
#define EXT2_NAME_LEN 255

struct ext2_dir_entry {
    __le32  inode;                  /* Inode number */
    __le16  rec_len;                /* Directory entry length */
    __u8    name_len;               /* Name length */
    __u8    file_type;
    char    name[EXT2_NAME_LEN];    /* File name */
};
// NOTE: The original Linux kernel header defines ext2_dir_entry with the original
//  layout and ext2_dir_entry_2 with the revised layout. We simply use the revised one.

/*
 * Ext2 directory file types.  Only the low 3 bits are used.  The
 * other bits are reserved for now.
 */
enum {
    EXT2_FT_UNKNOWN,
    EXT2_FT_REG_FILE,
    EXT2_FT_DIR,
    EXT2_FT_CHRDEV,
    EXT2_FT_BLKDEV,
    EXT2_FT_FIFO,
    EXT2_FT_SOCK,
    EXT2_FT_SYMLINK,
    EXT2_FT_MAX
};


#endif
