/**
 * \file fsw_ext2.c
 * ext2 file system driver code.
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

#include "fsw_ext2.h"


// functions

static fsw_status_t fsw_ext2_volume_mount(struct fsw_ext2_volume *vol);
static void         fsw_ext2_volume_free(struct fsw_ext2_volume *vol);
static fsw_status_t fsw_ext2_volume_stat(struct fsw_ext2_volume *vol, struct fsw_volume_stat *sb);

static fsw_status_t fsw_ext2_dnode_fill(struct fsw_ext2_volume *vol, struct fsw_ext2_dnode *dno);
static void         fsw_ext2_dnode_free(struct fsw_ext2_volume *vol, struct fsw_ext2_dnode *dno);
static fsw_status_t fsw_ext2_dnode_stat(struct fsw_ext2_volume *vol, struct fsw_ext2_dnode *dno,
                                        struct fsw_dnode_stat_str *sb);
static fsw_status_t fsw_ext2_get_extent(struct fsw_ext2_volume *vol, struct fsw_ext2_dnode *dno,
                                        struct fsw_extent *extent);

static fsw_status_t fsw_ext2_dir_lookup(struct fsw_ext2_volume *vol, struct fsw_ext2_dnode *dno,
                                        struct fsw_string *lookup_name, struct fsw_ext2_dnode **child_dno);
static fsw_status_t fsw_ext2_dir_read(struct fsw_ext2_volume *vol, struct fsw_ext2_dnode *dno,
                                      struct fsw_shandle *shand, struct fsw_ext2_dnode **child_dno);
static fsw_status_t fsw_ext2_read_dentry(struct fsw_shandle *shand, struct ext2_dir_entry *entry);

static fsw_status_t fsw_ext2_readlink(struct fsw_ext2_volume *vol, struct fsw_ext2_dnode *dno,
                                      struct fsw_string *link);

//
// Dispatch Table
//

struct fsw_fstype_table   FSW_FSTYPE_TABLE_NAME(ext2) = {
    { FSW_STRING_TYPE_ISO88591, 4, 4, "ext2" },
    sizeof(struct fsw_ext2_volume),
    sizeof(struct fsw_ext2_dnode),
    
    fsw_ext2_volume_mount,
    fsw_ext2_volume_free,
    fsw_ext2_volume_stat,
    fsw_ext2_dnode_fill,
    fsw_ext2_dnode_free,
    fsw_ext2_dnode_stat,
    fsw_ext2_get_extent,
    fsw_ext2_dir_lookup,
    fsw_ext2_dir_read,
    fsw_ext2_readlink,
};

/**
 * Mount an ext2 volume. Reads the superblock and constructs the
 * root directory dnode.
 */

static fsw_status_t fsw_ext2_volume_mount(struct fsw_ext2_volume *vol)
{
    fsw_status_t    status;
    void            *buffer;
    fsw_u32         blocksize;
    fsw_u32         groupcnt, groupno, gdesc_per_block, gdesc_bno, gdesc_index;
    struct ext2_group_desc *gdesc;
    int             i;
    struct fsw_string s;
    
    // allocate memory to keep the superblock around
    status = fsw_alloc(sizeof(struct ext2_super_block), &vol->sb);
    if (status)
        return status;
    
    // read the superblock into its buffer
    fsw_set_blocksize(vol, EXT2_SUPERBLOCK_BLOCKSIZE, EXT2_SUPERBLOCK_BLOCKSIZE);
    status = fsw_block_get(vol, EXT2_SUPERBLOCK_BLOCKNO, 0, &buffer);
    if (status)
        return status;
    fsw_memcpy(vol->sb, buffer, sizeof(struct ext2_super_block));
    fsw_block_release(vol, EXT2_SUPERBLOCK_BLOCKNO, buffer);
    
    // check the superblock
    if (vol->sb->s_magic != EXT2_SUPER_MAGIC)
        return FSW_UNSUPPORTED;
    if (vol->sb->s_rev_level != EXT2_GOOD_OLD_REV &&
        vol->sb->s_rev_level != EXT2_DYNAMIC_REV)
        return FSW_UNSUPPORTED;
    if (vol->sb->s_rev_level == EXT2_DYNAMIC_REV &&
        (vol->sb->s_feature_incompat & ~(EXT2_FEATURE_INCOMPAT_FILETYPE | EXT3_FEATURE_INCOMPAT_RECOVER)))
        return FSW_UNSUPPORTED;
    
    /*
     if (vol->sb->s_rev_level == EXT2_DYNAMIC_REV &&
         (vol->sb->s_feature_incompat & EXT3_FEATURE_INCOMPAT_RECOVER))
     Print(L"Ext2 WARNING: This ext3 file system needs recovery, trying to use it anyway.\n");
     */
    
    // set real blocksize
    blocksize = EXT2_BLOCK_SIZE(vol->sb);
    fsw_set_blocksize(vol, blocksize, blocksize);
    
    // get other info from superblock
    vol->ind_bcnt = EXT2_ADDR_PER_BLOCK(vol->sb);
    vol->dind_bcnt = vol->ind_bcnt * vol->ind_bcnt;
    vol->inode_size = EXT2_INODE_SIZE(vol->sb);
    
    for (i = 0; i < 16; i++)
        if (vol->sb->s_volume_name[i] == 0)
            break;
    s.type = FSW_STRING_TYPE_ISO88591;
    s.size = s.len = i;
    s.data = vol->sb->s_volume_name;
    status = fsw_strdup_coerce(&vol->g.label, vol->g.host_string_type, &s);
    if (status)
        return status;
    
    // read the group descriptors to get inode table offsets
    groupcnt = ((vol->sb->s_inodes_count - 2) / vol->sb->s_inodes_per_group) + 1;
    gdesc_per_block = (vol->g.phys_blocksize / sizeof(struct ext2_group_desc));
    
    status = fsw_alloc(sizeof(fsw_u32) * groupcnt, &vol->inotab_bno);
    if (status)
        return status;
    for (groupno = 0; groupno < groupcnt; groupno++) {
        // get the block group descriptor
        gdesc_bno = (vol->sb->s_first_data_block + 1) + groupno / gdesc_per_block;
        gdesc_index = groupno % gdesc_per_block;
        status = fsw_block_get(vol, gdesc_bno, 1, (void **)&buffer);
        if (status)
            return status;
        gdesc = ((struct ext2_group_desc *)(buffer)) + gdesc_index;
        vol->inotab_bno[groupno] = gdesc->bg_inode_table;
        fsw_block_release(vol, gdesc_bno, buffer);
    }
    
    // setup the root dnode
    status = fsw_dnode_create_root(vol, EXT2_ROOT_INO, &vol->g.root);
    if (status)
        return status;
    
    FSW_MSG_DEBUG((FSW_MSGSTR("fsw_ext2_volume_mount: success, blocksize %d\n"), blocksize));
    
    return FSW_SUCCESS;
}

/**
 * Free the volume data structure. Called by the core after an unmount or after
 * an unsuccessful mount to release the memory used by the file system type specific
 * part of the volume structure.
 */

static void fsw_ext2_volume_free(struct fsw_ext2_volume *vol)
{
    if (vol->sb)
        fsw_free(vol->sb);
    if (vol->inotab_bno)
        fsw_free(vol->inotab_bno);
}

/**
 * Get in-depth information on a volume.
 */

static fsw_status_t fsw_ext2_volume_stat(struct fsw_ext2_volume *vol, struct fsw_volume_stat *sb)
{
    sb->total_bytes = (fsw_u64)vol->sb->s_blocks_count      * vol->g.log_blocksize;
    sb->free_bytes  = (fsw_u64)vol->sb->s_free_blocks_count * vol->g.log_blocksize;
    return FSW_SUCCESS;
}

/**
 * Get full information on a dnode from disk. This function is called by the core
 * whenever it needs to access fields in the dnode structure that may not
 * be filled immediately upon creation of the dnode. In the case of ext2, we
 * delay fetching of the inode structure until dnode_fill is called. The size and
 * type fields are invalid until this function has been called.
 */

static fsw_status_t fsw_ext2_dnode_fill(struct fsw_ext2_volume *vol, struct fsw_ext2_dnode *dno)
{
    fsw_status_t    status;
    fsw_u32         groupno, ino_in_group, ino_bno, ino_index;
    fsw_u8          *buffer;
    
    if (dno->raw)
        return FSW_SUCCESS;
    
    FSW_MSG_DEBUG((FSW_MSGSTR("fsw_ext2_dnode_fill: inode %d\n"), dno->g.dnode_id));
    
    // read the inode block
    groupno = (dno->g.dnode_id - 1) / vol->sb->s_inodes_per_group;
    ino_in_group = (dno->g.dnode_id - 1) % vol->sb->s_inodes_per_group;
    ino_bno = vol->inotab_bno[groupno] +
        ino_in_group / (vol->g.phys_blocksize / vol->inode_size);
    ino_index = ino_in_group % (vol->g.phys_blocksize / vol->inode_size);
    status = fsw_block_get(vol, ino_bno, 2, (void **)&buffer);
    if (status)
        return status;
    
    // keep our inode around
    status = fsw_memdup((void **)&dno->raw, buffer + ino_index * vol->inode_size, vol->inode_size);
    fsw_block_release(vol, ino_bno, buffer);
    if (status)
        return status;
    
    // get info from the inode
    dno->g.size = dno->raw->i_size;
    // TODO: check docs for 64-bit sized files
    if (S_ISREG(dno->raw->i_mode))
        dno->g.type = FSW_DNODE_TYPE_FILE;
    else if (S_ISDIR(dno->raw->i_mode))
        dno->g.type = FSW_DNODE_TYPE_DIR;
    else if (S_ISLNK(dno->raw->i_mode))
        dno->g.type = FSW_DNODE_TYPE_SYMLINK;
    else
        dno->g.type = FSW_DNODE_TYPE_SPECIAL;
    
    return FSW_SUCCESS;
}

/**
 * Free the dnode data structure. Called by the core when deallocating a dnode
 * structure to release the memory used by the file system type specific part
 * of the dnode structure.
 */

static void fsw_ext2_dnode_free(struct fsw_ext2_volume *vol, struct fsw_ext2_dnode *dno)
{
    if (dno->raw)
        fsw_free(dno->raw);
}

/**
 * Get in-depth information on a dnode. The core makes sure that fsw_ext2_dnode_fill
 * has been called on the dnode before this function is called. Note that some
 * data is not directly stored into the structure, but passed to a host-specific
 * callback that converts it to the host-specific format.
 */

static fsw_status_t fsw_ext2_dnode_stat(struct fsw_ext2_volume *vol, struct fsw_ext2_dnode *dno,
                                        struct fsw_dnode_stat_str *sb)
{
    sb->used_bytes = dno->raw->i_blocks * 512;   // very, very strange...
    sb->store_time_posix(sb, FSW_DNODE_STAT_CTIME, dno->raw->i_ctime);
    sb->store_time_posix(sb, FSW_DNODE_STAT_ATIME, dno->raw->i_atime);
    sb->store_time_posix(sb, FSW_DNODE_STAT_MTIME, dno->raw->i_mtime);
    sb->store_attr_posix(sb, dno->raw->i_mode);
    
    return FSW_SUCCESS;
}

/**
 * Retrieve file data mapping information. This function is called by the core when
 * fsw_shandle_read needs to know where on the disk the required piece of the file's
 * data can be found. The core makes sure that fsw_ext2_dnode_fill has been called
 * on the dnode before. Our task here is to get the physical disk block number for
 * the requested logical block number.
 *
 * The ext2 file system does not use extents, but stores a list of block numbers
 * using the usual direct, indirect, double-indirect, triple-indirect scheme. To
 * optimize access, this function checks if the following file blocks are mapped
 * to consecutive disk blocks and returns a combined extent if possible.
 */

static fsw_status_t fsw_ext2_get_extent(struct fsw_ext2_volume *vol, struct fsw_ext2_dnode *dno,
                                        struct fsw_extent *extent)
{
    fsw_status_t    status;
    fsw_u32         bno, release_bno, buf_bcnt, file_bcnt;
    fsw_u32         *buffer;
    int             path[5], i;
    
    // Preconditions: The caller has checked that the requested logical block
    //  is within the file's size. The dnode has complete information, i.e.
    //  fsw_ext2_dnode_read_info was called successfully on it.
    
    extent->type = FSW_EXTENT_TYPE_PHYSBLOCK;
    extent->log_count = 1;
    bno = extent->log_start;
    
    // try direct block pointers in the inode
    if (bno < EXT2_NDIR_BLOCKS) {
        path[0] = bno;
        path[1] = -1;
    } else {
        bno -= EXT2_NDIR_BLOCKS;
        
        // try indirect block
        if (bno < vol->ind_bcnt) {
            path[0] = EXT2_IND_BLOCK;
            path[1] = bno;
            path[2] = -1;
        } else {
            bno -= vol->ind_bcnt;
        
            // try double-indirect block
            if (bno < vol->dind_bcnt) {
                path[0] = EXT2_DIND_BLOCK;
                path[1] = bno / vol->ind_bcnt;
                path[2] = bno % vol->ind_bcnt;
                path[3] = -1;
            } else {
                bno -= vol->dind_bcnt;
                
                // use the triple-indirect block
                path[0] = EXT2_TIND_BLOCK;
                path[1] = bno / vol->dind_bcnt;
                path[2] = (bno / vol->ind_bcnt) % vol->ind_bcnt;
                path[3] = bno % vol->ind_bcnt;
                path[4] = -1;
            }
        }
    }
    
    // follow the indirection path
    buffer = dno->raw->i_block;
    buf_bcnt = EXT2_NDIR_BLOCKS;
    release_bno = 0;
    for (i = 0; ; i++) {
        bno = buffer[path[i]];
        if (bno == 0) {
            extent->type = FSW_EXTENT_TYPE_SPARSE;
            if (release_bno)
                fsw_block_release(vol, release_bno, buffer);
            return FSW_SUCCESS;
        }
        if (path[i+1] < 0)
            break;
        
        if (release_bno)
            fsw_block_release(vol, release_bno, buffer);
        status = fsw_block_get(vol, bno, 1, (void **)&buffer);
        if (status)
            return status;
        release_bno = bno;
        buf_bcnt = vol->ind_bcnt;
    }
    extent->phys_start = bno;
    
    // check if the following blocks can be aggregated into one extent
    file_bcnt = (fsw_u32)((dno->g.size + vol->g.log_blocksize - 1) & (vol->g.log_blocksize - 1));
    while (path[i]           + extent->log_count < buf_bcnt &&    // indirect block has more block pointers
           extent->log_start + extent->log_count < file_bcnt) {   // file has more blocks
        if (buffer[path[i] + extent->log_count] == buffer[path[i] + extent->log_count - 1] + 1)
            extent->log_count++;
        else
            break;
    }
    
    if (release_bno)
        fsw_block_release(vol, release_bno, buffer);
    return FSW_SUCCESS;
}

/**
 * Lookup a directory's child dnode by name. This function is called on a directory
 * to retrieve the directory entry with the given name. A dnode is constructed for
 * this entry and returned. The core makes sure that fsw_ext2_dnode_fill has been called
 * and the dnode is actually a directory.
 */

static fsw_status_t fsw_ext2_dir_lookup(struct fsw_ext2_volume *vol, struct fsw_ext2_dnode *dno,
                                        struct fsw_string *lookup_name, struct fsw_ext2_dnode **child_dno_out)
{
    fsw_status_t    status;
    struct fsw_shandle shand;
    fsw_u32         child_ino;
    struct ext2_dir_entry entry;
    struct fsw_string entry_name;
    
    // Preconditions: The caller has checked that dno is a directory node.
    
    entry_name.type = FSW_STRING_TYPE_ISO88591;
    
    // setup handle to read the directory
    status = fsw_shandle_open(dno, &shand);
    if (status)
        return status;
    
    // scan the directory for the file
    child_ino = 0;
    while (child_ino == 0) {
        // read next entry
        status = fsw_ext2_read_dentry(&shand, &entry);
        if (status)
            goto errorexit;
        if (entry.inode == 0) {
            // end of directory reached
            status = FSW_NOT_FOUND;
            goto errorexit;
        }
        
        // compare name
        entry_name.len = entry_name.size = entry.name_len;
        entry_name.data = entry.name;
        if (fsw_streq(lookup_name, &entry_name)) {
            child_ino = entry.inode;
            break;
        }
    }
    
    // setup a dnode for the child item
    status = fsw_dnode_create(dno, child_ino, FSW_DNODE_TYPE_UNKNOWN, &entry_name, child_dno_out);
    
errorexit:
    fsw_shandle_close(&shand);
    return status;
}

/**
 * Get the next directory entry when reading a directory. This function is called during
 * directory iteration to retrieve the next directory entry. A dnode is constructed for
 * the entry and returned. The core makes sure that fsw_ext2_dnode_fill has been called
 * and the dnode is actually a directory. The shandle provided by the caller is used to
 * record the position in the directory between calls.
 */

static fsw_status_t fsw_ext2_dir_read(struct fsw_ext2_volume *vol, struct fsw_ext2_dnode *dno,
                                      struct fsw_shandle *shand, struct fsw_ext2_dnode **child_dno_out)
{
    fsw_status_t    status;
    struct ext2_dir_entry entry;
    struct fsw_string entry_name;
    
    // Preconditions: The caller has checked that dno is a directory node. The caller
    //  has opened a storage handle to the directory's storage and keeps it around between
    //  calls.
    
    while (1) {
        // read next entry
        status = fsw_ext2_read_dentry(shand, &entry);
        if (status)
            return status;
        if (entry.inode == 0)   // end of directory
            return FSW_NOT_FOUND;
        
        // skip . and ..
        if ((entry.name_len == 1 && entry.name[0] == '.') ||
            (entry.name_len == 2 && entry.name[0] == '.' && entry.name[1] == '.'))
            continue;
        break;
    }
    
    // setup name
    entry_name.type = FSW_STRING_TYPE_ISO88591;
    entry_name.len = entry_name.size = entry.name_len;
    entry_name.data = entry.name;
    
    // setup a dnode for the child item
    status = fsw_dnode_create(dno, entry.inode, FSW_DNODE_TYPE_UNKNOWN, &entry_name, child_dno_out);
    
    return status;
}

/**
 * Read a directory entry from the directory's raw data. This internal function is used
 * to read a raw ext2 directory entry into memory. The shandle's position pointer is adjusted
 * to point to the next entry.
 */

static fsw_status_t fsw_ext2_read_dentry(struct fsw_shandle *shand, struct ext2_dir_entry *entry)
{
    fsw_status_t    status;
    fsw_u32         buffer_size;
    
    while (1) {
        // read dir_entry header (fixed length)
        buffer_size = 8;
        status = fsw_shandle_read(shand, &buffer_size, entry);
        if (status)
            return status;
        
        if (buffer_size < 8 || entry->rec_len == 0) {
            // end of directory reached
            entry->inode = 0;
            return FSW_SUCCESS;
        }
        if (entry->rec_len < 8)
            return FSW_VOLUME_CORRUPTED;
        if (entry->inode != 0) {
            // this entry is used
            if (entry->rec_len < 8 + entry->name_len)
                return FSW_VOLUME_CORRUPTED;
            break;
        }
        
        // valid, but unused entry, skip it
        shand->pos += entry->rec_len - 8;
    }
    
    // read file name (variable length)
    buffer_size = entry->name_len;
    status = fsw_shandle_read(shand, &buffer_size, entry->name);
    if (status)
        return status;
    if (buffer_size < entry->name_len)
        return FSW_VOLUME_CORRUPTED;
    
    // skip any remaining padding
    shand->pos += entry->rec_len - (8 + entry->name_len);
    
    return FSW_SUCCESS;
}

/**
 * Get the target path of a symbolic link. This function is called when a symbolic
 * link needs to be resolved. The core makes sure that the fsw_ext2_dnode_fill has been
 * called on the dnode and that it really is a symlink.
 *
 * For ext2, the target path can be stored inline in the inode structure (in the space
 * otherwise occupied by the block pointers) or in the inode's data. There is no flag
 * indicating this, only the number of blocks entry (i_blocks) can be used as an
 * indication. The check used here comes from the Linux kernel.
 */

static fsw_status_t fsw_ext2_readlink(struct fsw_ext2_volume *vol, struct fsw_ext2_dnode *dno,
                                      struct fsw_string *link_target)
{
    fsw_status_t    status;
    int             ea_blocks;
    struct fsw_string s;
    
    if (dno->g.size > FSW_PATH_MAX)
        return FSW_VOLUME_CORRUPTED;
    
    ea_blocks = dno->raw->i_file_acl ? (vol->g.log_blocksize >> 9) : 0;
    
    if (dno->raw->i_blocks - ea_blocks == 0) {
        // "fast" symlink, path is stored inside the inode
        s.type = FSW_STRING_TYPE_ISO88591;
        s.size = s.len = (int)dno->g.size;
        s.data = dno->raw->i_block;
        status = fsw_strdup_coerce(link_target, vol->g.host_string_type, &s);
    } else {
        // "slow" symlink, path is stored in normal inode data
        status = fsw_dnode_readlink_data(dno, link_target);
    }
    
    return status;
}

// EOF
