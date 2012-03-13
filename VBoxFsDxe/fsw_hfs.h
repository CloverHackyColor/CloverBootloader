/* $Id: fsw_hfs.h 29125 2010-05-06 09:43:05Z vboxsync $ */
/** @file
 * fsw_hfs.h - HFS file system driver header.
 */

/*
 * Copyright (C) 2010 Oracle Corporation
 *
 * This file is part of VirtualBox Open Source Edition (OSE), as
 * available from http://www.virtualbox.org. This file is free software;
 * you can redistribute it and/or modify it under the terms of the GNU
 * General Public License (GPL) as published by the Free Software
 * Foundation, in version 2 as it comes in the "COPYING" file of the
 * VirtualBox OSE distribution. VirtualBox OSE is distributed in the
 * hope that it will be useful, but WITHOUT ANY WARRANTY of any kind.
 */

#ifndef _FSW_HFS_H_
#define _FSW_HFS_H_

#define VOLSTRUCTNAME fsw_hfs_volume
#define DNODESTRUCTNAME fsw_hfs_dnode

#include "fsw_core.h"


//! Block size for HFS volumes.
#define HFS_BLOCKSIZE            512

//! Block number where the HFS superblock resides.
#define HFS_SUPERBLOCK_BLOCKNO   2

/* Make world look Applish enough for the system header describing HFS layout  */
#define __APPLE_API_PRIVATE
#define __APPLE_API_UNSTABLE

#define u_int8_t  fsw_u8
#define u_int16_t fsw_u16
#define u_int32_t fsw_u32
#define u_int64_t fsw_u64
#define int8_t    fsw_s8
#define int16_t   fsw_s16
#define int32_t   fsw_s32
#define int64_t   fsw_s64

#include "hfs_format.h"

#undef u_int8_t
#undef u_int16_t
#undef u_int32_t
#undef u_int64_t
#undef int8_t
#undef int16_t
#undef int32_t
#undef int64_t

#pragma pack(1)
#ifdef _MSC_VER
/* vasily: disable warning for non-standard anonymous struct/union
 * declarations
 */
# pragma warning (disable:4201)
# define inline __inline
#endif

struct hfs_dirrec {
    fsw_u8      _dummy;
};

struct fsw_hfs_key
{
  union
  {
    struct HFSPlusExtentKey  ext_key;
    struct HFSPlusCatalogKey cat_key;
    fsw_u16                  key_len; /* Length is at the beginning of all keys */
  };
};

#pragma pack()

typedef enum {
    /* Regular HFS */
    FSW_HFS_PLAIN = 0,
    /* HFS+ */
    FSW_HFS_PLUS,
    /* HFS+ embedded to HFS */
    FSW_HFS_PLUS_EMB
} fsw_hfs_kind;

/**
 * HFS: Dnode structure with HFS-specific data.
 */

struct fsw_hfs_dnode
{
  struct fsw_dnode          g;          //!< Generic dnode structure
  HFSPlusExtentRecord       extents;
  fsw_u32                   ctime;
  fsw_u32                   mtime;
  fsw_u64                   used_bytes;
};

/**
 * HFS: In-memory B-tree structure.
 */
struct fsw_hfs_btree
{
    fsw_u32                  root_node;
    fsw_u32                  node_size;
    struct fsw_hfs_dnode*    file;
};


/**
 * HFS: In-memory volume structure with HFS-specific data.
 */

struct fsw_hfs_volume
{
    struct fsw_volume            g;            //!< Generic volume structure

    struct HFSPlusVolumeHeader   *primary_voldesc;  //!< Volume Descriptor
    struct fsw_hfs_btree          catalog_tree;     // Catalog tree
    struct fsw_hfs_btree          extents_tree;     // Extents overflow tree
    struct fsw_hfs_dnode          root_file;
    int                           case_sensitive;
    fsw_u32                       block_size_shift;
    fsw_hfs_kind                  hfs_kind;
    fsw_u32                       emb_block_off;
};

/* Endianess swappers */
static inline fsw_u16
swab16(fsw_u16 x)
{
    return (x<<8 | ((x & 0xff00)>>8));
}

static inline fsw_u32
swab32(fsw_u32 x)
{
    return x<<24 | x>>24 |
            (x & (fsw_u32)0x0000ff00UL)<<8 |
            (x & (fsw_u32)0x00ff0000UL)>>8;
}


static inline fsw_u64
swab64(fsw_u64 x)
{
    return x<<56 | x>>56 |
            (x & (fsw_u64)0x000000000000ff00ULL)<<40 |
            (x & (fsw_u64)0x0000000000ff0000ULL)<<24 |
            (x & (fsw_u64)0x00000000ff000000ULL)<< 8 |
            (x & (fsw_u64)0x000000ff00000000ULL)>> 8 |
            (x & (fsw_u64)0x0000ff0000000000ULL)>>24 |
            (x & (fsw_u64)0x00ff000000000000ULL)>>40;
}

static inline fsw_u16
be16_to_cpu(fsw_u16 x)
{
    return swab16(x);
}

static inline fsw_u16
cpu_to_be16(fsw_u16 x)
{
    return swab16(x);
}


static inline fsw_u32
cpu_to_be32(fsw_u32 x)
{
    return swab32(x);
}

static inline fsw_u32
be32_to_cpu(fsw_u32 x)
{
    return swab32(x);
}

static inline fsw_u64
be64_to_cpu(fsw_u64 x)
{
    return swab64(x);
}

#endif
