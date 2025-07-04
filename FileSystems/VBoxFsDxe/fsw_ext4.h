/**
 * \file fsw_ext4.h
 * ext4 file system driver header.
 */

/*-
 * Copyright (c) 2012 Stefan Agner
 * Portions Copyright (c) 2006 Christoph Pfisterer
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

#ifndef _FSW_EXT4_H_
#define _FSW_EXT4_H_

#define VOLSTRUCTNAME fsw_ext4_volume
#define DNODESTRUCTNAME fsw_ext4_dnode
#include "fsw_core.h"

#include "fsw_ext4_disk.h"


//! Block size to be used when reading the ext4 superblock.
#define EXT4_SUPERBLOCK_BLOCKSIZE  1024
//! Block number where the (master copy of the) ext4 superblock resides.
#define EXT4_SUPERBLOCK_BLOCKNO       1


/**
 * ext4: Volume structure with ext2-specific data.
 */

struct fsw_ext4_volume {
    struct fsw_volume g;            //!< Generic volume structure
    
    struct ext4_super_block *sb;    //!< Full raw ext2 superblock structure
    fsw_u64     *inotab_bno;        //!< Block numbers of the inode tables
    fsw_u32     ind_bcnt;           //!< Number of blocks addressable through an indirect block
    fsw_u32     dind_bcnt;          //!< Number of blocks addressable through a double-indirect block
    fsw_u32     inode_size;         //!< Size of inode structure in bytes
};

/**
 * ext2: Dnode structure with ext2-specific data.
 */

struct fsw_ext4_dnode {
    struct fsw_dnode g;             //!< Generic dnode structure
    
    struct ext4_inode *raw;         //!< Full raw inode structure
};


#endif
