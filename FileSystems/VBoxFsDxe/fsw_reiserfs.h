/**
 * \file fsw_reiserfs.h
 * ReiserFS file system driver header.
 */

/*-
 * Copyright (c) 2006 Christoph Pfisterer
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

#ifndef _FSW_REISERFS_H_
#define _FSW_REISERFS_H_

#define VOLSTRUCTNAME fsw_reiserfs_volume
#define DNODESTRUCTNAME fsw_reiserfs_dnode
#include "fsw_core.h"

#include "fsw_reiserfs_disk.h"


//! Block size (in shift bits) to be used when reading the reiserfs superblock.
#define REISERFS_SUPERBLOCK_BLOCKSIZEBITS  12
//! Block size (in bytes) to be used when reading the reiserfs superblock.
#define REISERFS_SUPERBLOCK_BLOCKSIZE  (1<<REISERFS_SUPERBLOCK_BLOCKSIZEBITS)


/**
 * ReiserFS: Results from a tree search.
 */

struct fsw_reiserfs_item {
    int valid;
    
    // the found item
    struct item_head ih;
    fsw_u64 item_offset;
    fsw_u32 item_type;
    
    fsw_u8 *item_data;
    
    // path information
    fsw_u32 path_bno[MAX_HEIGHT];
    fsw_u32 path_index[MAX_HEIGHT];
    
    // block release information
    fsw_u32 block_bno;
    void *block_buffer;
};


/**
 * ReiserFS: Volume structure with reiserfs-specific data.
 */

struct fsw_reiserfs_volume {
    struct fsw_volume g;            //!< Generic volume structure
    
    struct reiserfs_super_block *sb;  //!< Full raw reiserfs superblock structure
    int version;                    //!< Flag for 3.5 or 3.6 format
};

/**
 * ReiserFS: Dnode structure with reiserfs-specific data.
 */

struct fsw_reiserfs_dnode {
    struct fsw_dnode g;             //!< Generic dnode structure
    
    fsw_u32 dir_id;                 //!< Locality ID for the reiserfs tree (parent dir id)
    struct stat_data_v1 *sd_v1;     //!< Full stat_data, version 1
    struct stat_data *sd_v2;        //!< Full stat_data, version 2
};


#endif
