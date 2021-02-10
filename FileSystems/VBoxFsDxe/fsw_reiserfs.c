/**
 * \file fsw_reiserfs.c
 * ReiserFS file system driver code.
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

#include "fsw_reiserfs.h"


// functions

static fsw_status_t fsw_reiserfs_volume_mount(struct fsw_reiserfs_volume *vol);
static void         fsw_reiserfs_volume_free(struct fsw_reiserfs_volume *vol);
static fsw_status_t fsw_reiserfs_volume_stat(struct fsw_reiserfs_volume *vol, struct fsw_volume_stat *sb);

static fsw_status_t fsw_reiserfs_dnode_fill(struct fsw_reiserfs_volume *vol, struct fsw_reiserfs_dnode *dno);
static void         fsw_reiserfs_dnode_free(struct fsw_reiserfs_volume *vol, struct fsw_reiserfs_dnode *dno);
static fsw_status_t fsw_reiserfs_dnode_stat(struct fsw_reiserfs_volume *vol, struct fsw_reiserfs_dnode *dno,
                                        struct fsw_dnode_stat_str *sb);
static fsw_status_t fsw_reiserfs_get_extent(struct fsw_reiserfs_volume *vol, struct fsw_reiserfs_dnode *dno,
                                        struct fsw_extent *extent);

static fsw_status_t fsw_reiserfs_dir_lookup(struct fsw_reiserfs_volume *vol, struct fsw_reiserfs_dnode *dno,
                                        struct fsw_string *lookup_name, struct fsw_reiserfs_dnode **child_dno);
static fsw_status_t fsw_reiserfs_dir_read(struct fsw_reiserfs_volume *vol, struct fsw_reiserfs_dnode *dno,
                                      struct fsw_shandle *shand, struct fsw_reiserfs_dnode **child_dno);

static fsw_status_t fsw_reiserfs_readlink(struct fsw_reiserfs_volume *vol, struct fsw_reiserfs_dnode *dno,
                                      struct fsw_string *link);

static fsw_status_t fsw_reiserfs_item_search(struct fsw_reiserfs_volume *vol,
                                             fsw_u32 dir_id, fsw_u32 objectid, fsw_u64 offset,
                                             struct fsw_reiserfs_item *item);
static fsw_status_t fsw_reiserfs_item_next(struct fsw_reiserfs_volume *vol,
                                           struct fsw_reiserfs_item *item);
static void fsw_reiserfs_item_release(struct fsw_reiserfs_volume *vol,
                                      struct fsw_reiserfs_item *item);

//
// Dispatch Table
//

struct fsw_fstype_table   FSW_FSTYPE_TABLE_NAME(reiserfs) = {
    { FSW_STRING_TYPE_ISO88591, 8, 8, "reiserfs" },
    sizeof(struct fsw_reiserfs_volume),
    sizeof(struct fsw_reiserfs_dnode),
    
    fsw_reiserfs_volume_mount,
    fsw_reiserfs_volume_free,
    fsw_reiserfs_volume_stat,
    fsw_reiserfs_dnode_fill,
    fsw_reiserfs_dnode_free,
    fsw_reiserfs_dnode_stat,
    fsw_reiserfs_get_extent,
    fsw_reiserfs_dir_lookup,
    fsw_reiserfs_dir_read,
    fsw_reiserfs_readlink,
};

// misc data

static fsw_u32  superblock_offsets[3] = {
    REISERFS_DISK_OFFSET_IN_BYTES     >> REISERFS_SUPERBLOCK_BLOCKSIZEBITS,
    REISERFS_OLD_DISK_OFFSET_IN_BYTES >> REISERFS_SUPERBLOCK_BLOCKSIZEBITS,
    0
};

/**
 * Mount an reiserfs volume. Reads the superblock and constructs the
 * root directory dnode.
 */

static fsw_status_t fsw_reiserfs_volume_mount(struct fsw_reiserfs_volume *vol)
{
    fsw_status_t    status;
    void            *buffer;
    fsw_u32         blocksize;
    int             i;
    struct fsw_string s;
    
    // allocate memory to keep the superblock around
    status = fsw_alloc(sizeof(struct reiserfs_super_block), &vol->sb);
    if (status)
        return status;
    
    // read the superblock into its buffer
    fsw_set_blocksize(vol, REISERFS_SUPERBLOCK_BLOCKSIZE, REISERFS_SUPERBLOCK_BLOCKSIZE);
    for (i = 0; superblock_offsets[i]; i++) {
        status = fsw_block_get(vol, superblock_offsets[i], 0, &buffer);
        if (status)
            return status;
        fsw_memcpy(vol->sb, buffer, sizeof(struct reiserfs_super_block));
        fsw_block_release(vol, superblock_offsets[i], buffer);
        
        // check for one of the magic strings
        if (fsw_memeq(vol->sb->s_v1.s_magic,
                      REISERFS_SUPER_MAGIC_STRING, 8)) {
            vol->version = REISERFS_VERSION_1;
            break;
        } else if (fsw_memeq(vol->sb->s_v1.s_magic,
                             REISER2FS_SUPER_MAGIC_STRING, 9)) {
            vol->version = REISERFS_VERSION_2;
            break;
        } else if (fsw_memeq(vol->sb->s_v1.s_magic,
                             REISER2FS_JR_SUPER_MAGIC_STRING, 9)) {
            vol->version = vol->sb->s_v1.s_version;
            if (vol->version == REISERFS_VERSION_1 || vol->version == REISERFS_VERSION_2)
                break;
        }
    }
    if (superblock_offsets[i] == 0)
        return FSW_UNSUPPORTED;
    
    // check the superblock
    if (vol->sb->s_v1.s_root_block == (__le32)-1)   // unfinished 'reiserfsck --rebuild-tree'
        return FSW_VOLUME_CORRUPTED;
    
    /*
    if (vol->sb->s_rev_level != EXT2_GOOD_OLD_REV &&
        vol->sb->s_rev_level != EXT2_DYNAMIC_REV)
        return FSW_UNSUPPORTED;
    if (vol->sb->s_rev_level == EXT2_DYNAMIC_REV &&
        (vol->sb->s_feature_incompat & ~(EXT2_FEATURE_INCOMPAT_FILETYPE | EXT3_FEATURE_INCOMPAT_RECOVER)))
        return FSW_UNSUPPORTED;
    */
    
    // set real blocksize
    blocksize = vol->sb->s_v1.s_blocksize;
    fsw_set_blocksize(vol, blocksize, blocksize);
    
    // get other info from superblock
    /*
    vol->ind_bcnt = EXT2_ADDR_PER_BLOCK(vol->sb);
    vol->dind_bcnt = vol->ind_bcnt * vol->ind_bcnt;
    vol->inode_size = EXT2_INODE_SIZE(vol->sb);
    */
    
    for (i = 0; i < 16; i++)
        if (vol->sb->s_label[i] == 0)
            break;
    s.type = FSW_STRING_TYPE_ISO88591;
    s.size = s.len = i;
    s.data = vol->sb->s_label;
    status = fsw_strdup_coerce(&vol->g.label, vol->g.host_string_type, &s);
    if (status)
        return status;
    
    // setup the root dnode
    status = fsw_dnode_create_root(vol, REISERFS_ROOT_OBJECTID, &vol->g.root);
    if (status)
        return status;
    vol->g.root->dir_id = REISERFS_ROOT_PARENT_OBJECTID;
    
    FSW_MSG_DEBUG((FSW_MSGSTR("fsw_reiserfs_volume_mount: success, blocksize %d tree height %d\n"),
                   blocksize, vol->sb->s_v1.s_tree_height));
    
    return FSW_SUCCESS;
}

/**
 * Free the volume data structure. Called by the core after an unmount or after
 * an unsuccessful mount to release the memory used by the file system type specific
 * part of the volume structure.
 */

static void fsw_reiserfs_volume_free(struct fsw_reiserfs_volume *vol)
{
    if (vol->sb)
        fsw_free(vol->sb);
}

/**
 * Get in-depth information on a volume.
 */

static fsw_status_t fsw_reiserfs_volume_stat(struct fsw_reiserfs_volume *vol, struct fsw_volume_stat *sb)
{
    sb->total_bytes = (fsw_u64)vol->sb->s_v1.s_block_count * vol->g.log_blocksize;
    sb->free_bytes  = (fsw_u64)vol->sb->s_v1.s_free_blocks * vol->g.log_blocksize;
    return FSW_SUCCESS;
}

/**
 * Get full information on a dnode from disk. This function is called by the core
 * whenever it needs to access fields in the dnode structure that may not
 * be filled immediately upon creation of the dnode. In the case of reiserfs, we
 * delay fetching of the stat data until dnode_fill is called. The size and
 * type fields are invalid until this function has been called.
 */

static fsw_status_t fsw_reiserfs_dnode_fill(struct fsw_reiserfs_volume *vol, struct fsw_reiserfs_dnode *dno)
{
    fsw_status_t    status;
    fsw_u32         item_len, mode;
    struct fsw_reiserfs_item item;
    
    if (dno->sd_v1 || dno->sd_v2)
        return FSW_SUCCESS;
    
    FSW_MSG_DEBUG((FSW_MSGSTR("fsw_reiserfs_dnode_fill: object %d/%d\n"), dno->dir_id, dno->g.dnode_id));
    
    // find stat data item in reiserfs tree
    status = fsw_reiserfs_item_search(vol, dno->dir_id, dno->g.dnode_id, 0, &item);
    if (status == FSW_NOT_FOUND) {
        FSW_MSG_ASSERT((FSW_MSGSTR("fsw_reiserfs_dnode_fill: cannot find stat_data for object %d/%d\n"),
                        dno->dir_id, dno->g.dnode_id));
        return FSW_VOLUME_CORRUPTED;
    }
    if (status)
        return status;
    if (item.item_offset != 0) {
        FSW_MSG_ASSERT((FSW_MSGSTR("fsw_reiserfs_dnode_fill: got item that's not stat_data\n")));
        fsw_reiserfs_item_release(vol, &item);
        return FSW_VOLUME_CORRUPTED;
    }
    item_len = item.ih.ih_item_len;
    
    // get data in appropriate version
    if (item.ih.ih_version == KEY_FORMAT_3_5 && item_len == SD_V1_SIZE) {
        // have stat_data_v1 structure
        status = fsw_memdup((void **)&dno->sd_v1, item.item_data, item_len);
        fsw_reiserfs_item_release(vol, &item);
        if (status)
            return status;
        
        // get info from the inode
        dno->g.size = dno->sd_v1->sd_size;
        mode = dno->sd_v1->sd_mode;
        
    } else if (item.ih.ih_version == KEY_FORMAT_3_6 && item_len == SD_V2_SIZE) {
        // have stat_data_v2 structure
        status = fsw_memdup((void **)&dno->sd_v2, item.item_data, item_len);
        fsw_reiserfs_item_release(vol, &item);
        if (status)
            return status;
        
        // get info from the inode
        dno->g.size = dno->sd_v2->sd_size;
        mode = dno->sd_v2->sd_mode;
        
    } else {
        FSW_MSG_ASSERT((FSW_MSGSTR("fsw_reiserfs_dnode_fill: version %d(%d) and size %d(%d) not recognized for stat_data\n"),
                        item.ih.ih_version, KEY_FORMAT_3_6, item_len, SD_V2_SIZE));
        fsw_reiserfs_item_release(vol, &item);
        return FSW_VOLUME_CORRUPTED;
    }
    
    // get node type from mode field
    if (S_ISREG(mode))
        dno->g.type = FSW_DNODE_TYPE_FILE;
    else if (S_ISDIR(mode))
        dno->g.type = FSW_DNODE_TYPE_DIR;
    else if (S_ISLNK(mode))
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

static void fsw_reiserfs_dnode_free(struct fsw_reiserfs_volume *vol, struct fsw_reiserfs_dnode *dno)
{
    if (dno->sd_v1)
        fsw_free(dno->sd_v1);
    if (dno->sd_v2)
        fsw_free(dno->sd_v2);
}

/**
 * Get in-depth information on a dnode. The core makes sure that fsw_reiserfs_dnode_fill
 * has been called on the dnode before this function is called. Note that some
 * data is not directly stored into the structure, but passed to a host-specific
 * callback that converts it to the host-specific format.
 */

static fsw_status_t fsw_reiserfs_dnode_stat(struct fsw_reiserfs_volume *vol, struct fsw_reiserfs_dnode *dno,
                                            struct fsw_dnode_stat_str *sb)
{
    if (dno->sd_v1) {
        if (dno->g.type == FSW_DNODE_TYPE_SPECIAL)
            sb->used_bytes = 0;
        else
            sb->used_bytes = dno->sd_v1->u.sd_blocks * vol->g.log_blocksize;
        sb->store_time_posix(sb, FSW_DNODE_STAT_CTIME, dno->sd_v1->sd_ctime);
        sb->store_time_posix(sb, FSW_DNODE_STAT_ATIME, dno->sd_v1->sd_atime);
        sb->store_time_posix(sb, FSW_DNODE_STAT_MTIME, dno->sd_v1->sd_mtime);
        sb->store_attr_posix(sb, dno->sd_v1->sd_mode);
    } else if (dno->sd_v2) {
        sb->used_bytes = dno->sd_v2->sd_blocks * vol->g.log_blocksize;
        sb->store_time_posix(sb, FSW_DNODE_STAT_CTIME, dno->sd_v2->sd_ctime);
        sb->store_time_posix(sb, FSW_DNODE_STAT_ATIME, dno->sd_v2->sd_atime);
        sb->store_time_posix(sb, FSW_DNODE_STAT_MTIME, dno->sd_v2->sd_mtime);
        sb->store_attr_posix(sb, dno->sd_v2->sd_mode);
    }
    
    return FSW_SUCCESS;
}

/**
 * Retrieve file data mapping information. This function is called by the core when
 * fsw_shandle_read needs to know where on the disk the required piece of the file's
 * data can be found. The core makes sure that fsw_reiserfs_dnode_fill has been called
 * on the dnode before. Our task here is to get the physical disk block number for
 * the requested logical block number.
 */

static fsw_status_t fsw_reiserfs_get_extent(struct fsw_reiserfs_volume *vol, struct fsw_reiserfs_dnode *dno,
                                            struct fsw_extent *extent)
{
    fsw_status_t    status;
    fsw_u64         search_offset, intra_offset;
    struct fsw_reiserfs_item item;
    fsw_u32         intra_bno, nr_item;
    
    // Preconditions: The caller has checked that the requested logical block
    //  is within the file's size. The dnode has complete information, i.e.
    //  fsw_reiserfs_dnode_read_info was called successfully on it.
    
    FSW_MSG_DEBUG((FSW_MSGSTR("fsw_reiserfs_get_extent: mapping block %d of object %d/%d\n"),
                   extent->log_start, dno->dir_id, dno->g.dnode_id));
    
    extent->type = FSW_EXTENT_TYPE_SPARSE;
    extent->log_count = 1;
    
    // get the item for the requested block
    search_offset = (fsw_u64)extent->log_start * vol->g.log_blocksize + 1;
    status = fsw_reiserfs_item_search(vol, dno->dir_id, dno->g.dnode_id, search_offset, &item);
    if (status)
        return status;
    if (item.item_offset == 0) {
        fsw_reiserfs_item_release(vol, &item);
        return FSW_SUCCESS;       // no data items found, assume all-sparse file
    }
    intra_offset = search_offset - item.item_offset;
    
    // check the kind of block
    if (item.item_type == TYPE_INDIRECT || item.item_type == V1_INDIRECT_UNIQUENESS) {
        // indirect item, contains block numbers
        
        if (intra_offset & (vol->g.log_blocksize - 1)) {
            FSW_MSG_ASSERT((FSW_MSGSTR("fsw_reiserfs_get_extent: intra_offset not block-aligned for indirect block\n")));
            goto bail;
        }
        intra_bno = (fsw_u32)FSW_U64_DIV(intra_offset, vol->g.log_blocksize);
        nr_item = item.ih.ih_item_len / sizeof(fsw_u32);
        if (intra_bno >= nr_item) {
            FSW_MSG_ASSERT((FSW_MSGSTR("fsw_reiserfs_get_extent: indirect block too small\n")));
            goto bail;
        }
        extent->type = FSW_EXTENT_TYPE_PHYSBLOCK;
        extent->phys_start = ((fsw_u32 *)item.item_data)[intra_bno];
        
        // TODO: check if the following blocks can be aggregated into one extent
        
        fsw_reiserfs_item_release(vol, &item);
        return FSW_SUCCESS;
        
    } else if (item.item_type == TYPE_DIRECT || item.item_type == V1_DIRECT_UNIQUENESS) {
        // direct item, contains file data
        
        // TODO: Check if direct items always start on block boundaries. If not, we may have
        //  to do extra work here.
        
        if (intra_offset != 0) {
            FSW_MSG_ASSERT((FSW_MSGSTR("fsw_reiserfs_get_extent: intra_offset not aligned for direct block\n")));
            goto bail;
        }
        
        extent->type = FSW_EXTENT_TYPE_BUFFER;
        status = fsw_memdup(&extent->buffer, item.item_data, item.ih.ih_item_len);
        fsw_reiserfs_item_release(vol, &item);
        if (status)
            return status;
        
        return FSW_SUCCESS;
        
    }
    
bail:
    fsw_reiserfs_item_release(vol, &item);
    return FSW_VOLUME_CORRUPTED;
    
    /*    
    // check if the following blocks can be aggregated into one extent
    file_bcnt = (fsw_u32)((dno->g.size + vol->g.log_blocksize - 1) & (vol->g.log_blocksize - 1));
    while (path[i]           + extent->log_count < buf_bcnt &&    // indirect block has more block pointers
           extent->log_start + extent->log_count < file_bcnt) {   // file has more blocks
        if (buffer[path[i] + extent->log_count] == buffer[path[i] + extent->log_count - 1] + 1)
            extent->log_count++;
        else
            break;
    }
    */
}

/**
 * Lookup a directory's child dnode by name. This function is called on a directory
 * to retrieve the directory entry with the given name. A dnode is constructed for
 * this entry and returned. The core makes sure that fsw_reiserfs_dnode_fill has been called
 * and the dnode is actually a directory.
 */

static fsw_status_t fsw_reiserfs_dir_lookup(struct fsw_reiserfs_volume *vol, struct fsw_reiserfs_dnode *dno,
                                            struct fsw_string *lookup_name, struct fsw_reiserfs_dnode **child_dno_out)
{
    fsw_status_t    status;
    struct fsw_reiserfs_item item;
    fsw_u32         nr_item, i, name_offset, next_name_offset, name_len;
    fsw_u32         child_dir_id;
    struct reiserfs_de_head *dhead;
    struct fsw_string entry_name;
    
    // Preconditions: The caller has checked that dno is a directory node.
    
    // BIG TODOS: Use the hash function to start with the item containing the entry.
    //  Use binary search within the item.
    
    entry_name.type = FSW_STRING_TYPE_ISO88591;
    
    // get the item for that position
    status = fsw_reiserfs_item_search(vol, dno->dir_id, dno->g.dnode_id, FIRST_ITEM_OFFSET, &item);
    if (status)
        return status;
    if (item.item_offset == 0) {
        fsw_reiserfs_item_release(vol, &item);
        return FSW_NOT_FOUND;       // empty directory or something
    }
    
    for(;;) {
        
        // search the directory item
        dhead = (struct reiserfs_de_head *)item.item_data;
        nr_item = item.ih.u.ih_entry_count;
        next_name_offset = item.ih.ih_item_len;
        for (i = 0; i < nr_item; i++, dhead++, next_name_offset = name_offset) {
            // get the name
            name_offset = dhead->deh_location;
            name_len = next_name_offset - name_offset;
            while (name_len > 0 && item.item_data[name_offset + name_len - 1] == 0)
                name_len--;
            
            entry_name.len = entry_name.size = name_len;
            entry_name.data = item.item_data + name_offset;
            
            // compare name
            if (fsw_streq(lookup_name, &entry_name)) {
                // found the entry we're looking for!
                
                // setup a dnode for the child item
                status = fsw_dnode_create(dno, dhead->deh_objectid, FSW_DNODE_TYPE_UNKNOWN, &entry_name, child_dno_out);
                child_dir_id = dhead->deh_dir_id;
                fsw_reiserfs_item_release(vol, &item);
                if (status)
                    return status;
                (*child_dno_out)->dir_id = child_dir_id;
                
                return FSW_SUCCESS;
            }
        }
        
        // We didn't find the next directory entry in this item. Look for the next
        // item of the directory.
        
        status = fsw_reiserfs_item_next(vol, &item);
        if (status)
            return status;
        
    }
}

/**
 * Get the next directory entry when reading a directory. This function is called during
 * directory iteration to retrieve the next directory entry. A dnode is constructed for
 * the entry and returned. The core makes sure that fsw_reiserfs_dnode_fill has been called
 * and the dnode is actually a directory. The shandle provided by the caller is used to
 * record the position in the directory between calls.
 */

static fsw_status_t fsw_reiserfs_dir_read(struct fsw_reiserfs_volume *vol, struct fsw_reiserfs_dnode *dno,
                                          struct fsw_shandle *shand, struct fsw_reiserfs_dnode **child_dno_out)
{
    fsw_status_t    status;
    struct fsw_reiserfs_item item;
    fsw_u32         nr_item, i, name_offset, next_name_offset, name_len;
    fsw_u32         child_dir_id;
    struct reiserfs_de_head *dhead;
    struct fsw_string entry_name;
    
    // Preconditions: The caller has checked that dno is a directory node. The caller
    //  has opened a storage handle to the directory's storage and keeps it around between
    //  calls.
    
    // BIG TODOS: Use binary search within the item.
    
    // adjust pointer to first entry if necessary
    if (shand->pos == 0)
        shand->pos = FIRST_ITEM_OFFSET;
    
    // get the item for that position
    status = fsw_reiserfs_item_search(vol, dno->dir_id, dno->g.dnode_id, shand->pos, &item);
    if (status)
        return status;
    if (item.item_offset == 0) {
        fsw_reiserfs_item_release(vol, &item);
        return FSW_NOT_FOUND;       // empty directory or something
    }
    
    for(;;) {
        
        // search the directory item
        dhead = (struct reiserfs_de_head *)item.item_data;
        nr_item = item.ih.u.ih_entry_count;
        for (i = 0; i < nr_item; i++, dhead++) {
            if (dhead->deh_offset < shand->pos)
                continue;  // not yet past the last entry returned
            if (dhead->deh_offset == DOT_OFFSET || dhead->deh_offset == DOT_DOT_OFFSET)
                continue;  // never report . or ..
            
            // get the name
            name_offset = dhead->deh_location;
            if (i == 0)
                next_name_offset = item.ih.ih_item_len;
            else
                next_name_offset = dhead[-1].deh_location;
            name_len = next_name_offset - name_offset;
            while (name_len > 0 && item.item_data[name_offset + name_len - 1] == 0)
                name_len--;
            
            entry_name.type = FSW_STRING_TYPE_ISO88591;
            entry_name.len = entry_name.size = name_len;
            entry_name.data = item.item_data + name_offset;
            
            if (fsw_streq_cstr(&entry_name, ".reiserfs_priv"))
                continue;  // never report this special file
            
            // found the next entry!
            shand->pos = dhead->deh_offset + 1;
            
            // setup a dnode for the child item
            status = fsw_dnode_create(dno, dhead->deh_objectid, FSW_DNODE_TYPE_UNKNOWN, &entry_name, child_dno_out);
            child_dir_id = dhead->deh_dir_id;
            fsw_reiserfs_item_release(vol, &item);
            if (status)
                return status;
            (*child_dno_out)->dir_id = child_dir_id;
            
            return FSW_SUCCESS;
        }
        
        // We didn't find the next directory entry in this item. Look for the next
        // item of the directory.
        
        status = fsw_reiserfs_item_next(vol, &item);
        if (status)
            return status;
        
    }
}

/**
 * Get the target path of a symbolic link. This function is called when a symbolic
 * link needs to be resolved. The core makes sure that the fsw_reiserfs_dnode_fill has been
 * called on the dnode and that it really is a symlink.
 */

static fsw_status_t fsw_reiserfs_readlink(struct fsw_reiserfs_volume *vol, struct fsw_reiserfs_dnode *dno,
                                          struct fsw_string *link_target)
{
    return fsw_dnode_readlink_data(dno, link_target);
}

/**
 * Compare an on-disk tree key against the search key.
 */

static int fsw_reiserfs_compare_key(struct reiserfs_key *key,
                                    fsw_u32 dir_id, fsw_u32 objectid, fsw_u64 offset)
{
    fsw_u32 key_type;
    fsw_u64 key_offset;
    
    if (key->k_dir_id > dir_id)
        return FIRST_GREATER;
    if (key->k_dir_id < dir_id)
        return SECOND_GREATER;
    
    if (key->k_objectid > objectid)
        return FIRST_GREATER;
    if (key->k_objectid < objectid)
        return SECOND_GREATER;
    
    // determine format of the on-disk key
    key_type = (fsw_u32)FSW_U64_SHR(key->u.k_offset_v2.v, 60);
    if (key_type != TYPE_DIRECT && key_type != TYPE_INDIRECT && key_type != TYPE_DIRENTRY) {
        // detected 3.5 format (_v1)
        key_offset = key->u.k_offset_v1.k_offset;
    } else {
        // detected 3.6 format (_v2)
        key_offset = key->u.k_offset_v2.v & (~0ULL >> 4);
    }
    if (key_offset > offset)
        return FIRST_GREATER;
    if (key_offset < offset)
        return SECOND_GREATER;
    return KEYS_IDENTICAL;
}

/**
 * Find an item by key in the reiserfs tree.
 */

static fsw_status_t fsw_reiserfs_item_search(struct fsw_reiserfs_volume *vol,
                                            fsw_u32 dir_id, fsw_u32 objectid, fsw_u64 offset,
                                            struct fsw_reiserfs_item *item)
{
    fsw_status_t    status;
    int             comp_result;
    fsw_u32         tree_bno, next_tree_bno, tree_level, nr_item, i;
    fsw_u8          *buffer;
    struct block_head *bhead;
    struct reiserfs_key *key;
    struct item_head *ihead;
    
    FSW_MSG_DEBUG((FSW_MSGSTR("fsw_reiserfs_item_search: searching %d/%d/%lld\n"), dir_id, objectid, offset));
    
    // BIG TODOS: Use binary search within the item.
    //  Remember tree path for "get next item" function.
    
    item->valid = 0;
    item->block_bno = 0;
    
    // walk the tree
    tree_bno = vol->sb->s_v1.s_root_block;
    for (tree_level = vol->sb->s_v1.s_tree_height - 1; ; tree_level--) {
        
        // get the current tree block into memory
        status = fsw_block_get(vol, tree_bno, tree_level, (void **)&buffer);
        if (status)
            return status;
        bhead = (struct block_head *)buffer;
        if (bhead->blk_level != tree_level) {
            FSW_MSG_ASSERT((FSW_MSGSTR("fsw_reiserfs_item_search: tree block %d has not expected level %d\n"), tree_bno, tree_level));
            fsw_block_release(vol, tree_bno, buffer);
            return FSW_VOLUME_CORRUPTED;
        }
        nr_item = bhead->blk_nr_item;
        FSW_MSG_DEBUGV((FSW_MSGSTR("fsw_reiserfs_item_search: visiting block %d level %d items %d\n"), tree_bno, tree_level, nr_item));
        item->path_bno[tree_level] = tree_bno;
        
        // check if we have reached a leaf block
        if (tree_level == DISK_LEAF_NODE_LEVEL)
            break;
        
        // search internal node block, look for the path to follow
        key = (struct reiserfs_key *)(buffer + BLKH_SIZE);
        for (i = 0; i < nr_item; i++, key++) {
            if (fsw_reiserfs_compare_key(key, dir_id, objectid, offset) == FIRST_GREATER)
                break;
        }
        item->path_index[tree_level] = i;
        next_tree_bno = ((struct disk_child *)(buffer + BLKH_SIZE + nr_item * KEY_SIZE))[i].dc_block_number;
        fsw_block_release(vol, tree_bno, buffer);
        tree_bno = next_tree_bno;
    }
    
    // search leaf node block, look for our data
    ihead = (struct item_head *)(buffer + BLKH_SIZE);
    for (i = 0; i < nr_item; i++, ihead++) {
        comp_result = fsw_reiserfs_compare_key(&ihead->ih_key, dir_id, objectid, offset);
        if (comp_result == KEYS_IDENTICAL)
            break;
        if (comp_result == FIRST_GREATER) {
            // Current key is greater than the search key. Use the last key before this
            // one as the preliminary result.
            if (i == 0) {
                fsw_block_release(vol, tree_bno, buffer);
                return FSW_NOT_FOUND;
            }
            i--, ihead--;
            break;
        }
    }
    if (i >= nr_item) {
        // Go back to the last key, it was smaller than the search key.
        // NOTE: The first key of the next leaf block is guaranteed to be greater than
        //  our search key.
        i--, ihead--;
    }
    item->path_index[tree_level] = i;
    // Since we may have a key that is smaller than the search key, verify that
    // it is for the same object.
    if (ihead->ih_key.k_dir_id != dir_id || ihead->ih_key.k_objectid != objectid) {
        fsw_block_release(vol, tree_bno, buffer);
        return FSW_NOT_FOUND;   // Found no key for this object at all
    }
    
    // return results
    fsw_memcpy(&item->ih, ihead, sizeof(struct item_head));
    item->item_type = (fsw_u32)FSW_U64_SHR(ihead->ih_key.u.k_offset_v2.v, 60);
    if (item->item_type != TYPE_DIRECT &&
        item->item_type != TYPE_INDIRECT &&
        item->item_type != TYPE_DIRENTRY) {
        // 3.5 format (_v1)
        item->item_type = ihead->ih_key.u.k_offset_v1.k_uniqueness;
        item->item_offset = ihead->ih_key.u.k_offset_v1.k_offset;
    } else {
        // 3.6 format (_v2)
        item->item_offset = ihead->ih_key.u.k_offset_v2.v & (~0ULL >> 4);
    }
    item->item_data = buffer + ihead->ih_item_location;
    item->valid = 1;
    
    // add information for block release
    item->block_bno = tree_bno;
    item->block_buffer = buffer;
    
    FSW_MSG_DEBUG((FSW_MSGSTR("fsw_reiserfs_item_search: found %d/%d/%lld (%d)\n"),
                   ihead->ih_key.k_dir_id, ihead->ih_key.k_objectid, item->item_offset, item->item_type));
    return FSW_SUCCESS;
}

/**
 * Find the next item in the reiserfs tree for an already-found item.
 */

static fsw_status_t fsw_reiserfs_item_next(struct fsw_reiserfs_volume *vol,
                                          struct fsw_reiserfs_item *item)
{
    fsw_status_t    status;
    fsw_u32         dir_id, objectid;
//    fsw_u64         offset;
    fsw_u32         tree_bno, next_tree_bno, tree_level, nr_item, nr_ptr_item;
    fsw_u8          *buffer;
    struct block_head *bhead;
    struct item_head *ihead;
    
    if (!item->valid)
        return FSW_NOT_FOUND;
    fsw_reiserfs_item_release(vol, item);   // TODO: maybe delay this and/or use the cached block!
    
    dir_id = item->ih.ih_key.k_dir_id;
    objectid = item->ih.ih_key.k_objectid;
//    offset = item->item_offset;
    
    FSW_MSG_DEBUG((FSW_MSGSTR("fsw_reiserfs_item_next: next for %d/%d/%lld\n"), dir_id, objectid, item->item_offset));
    
    // find a node that has more items, moving up until we find one
    
    for (tree_level = DISK_LEAF_NODE_LEVEL; tree_level < vol->sb->s_v1.s_tree_height; tree_level++) {
        
        // get the current tree block into memory
        tree_bno = item->path_bno[tree_level];
        status = fsw_block_get(vol, tree_bno, tree_level, (void **)&buffer);
        if (status)
            return status;
        bhead = (struct block_head *)buffer;
        if (bhead->blk_level != tree_level) {
            FSW_MSG_ASSERT((FSW_MSGSTR("fsw_reiserfs_item_next: tree block %d has not expected level %d\n"), tree_bno, tree_level));
            fsw_block_release(vol, tree_bno, buffer);
            return FSW_VOLUME_CORRUPTED;
        }
        nr_item = bhead->blk_nr_item;
        FSW_MSG_DEBUGV((FSW_MSGSTR("fsw_reiserfs_item_next: visiting block %d level %d items %d\n"), tree_bno, tree_level, nr_item));
        
        nr_ptr_item = nr_item + ((tree_level > DISK_LEAF_NODE_LEVEL) ? 1 : 0);  // internal nodes have (nr_item) keys and (nr_item+1) pointers
        item->path_index[tree_level]++;
        if (item->path_index[tree_level] >= nr_ptr_item) {
            item->path_index[tree_level] = 0;
            fsw_block_release(vol, tree_bno, buffer);
            continue;  // this node doesn't have any more items, move up one level
        }
        
        // we have a new path to follow, move down to the leaf node again
        while (tree_level > DISK_LEAF_NODE_LEVEL) {
            // get next pointer from current block
            next_tree_bno = ((struct disk_child *)(buffer + BLKH_SIZE + nr_item * KEY_SIZE))[item->path_index[tree_level]].dc_block_number;
            fsw_block_release(vol, tree_bno, buffer);
            tree_bno = next_tree_bno;
            tree_level--;
            
            // get the current tree block into memory
            status = fsw_block_get(vol, tree_bno, tree_level, (void **)&buffer);
            if (status)
                return status;
            bhead = (struct block_head *)buffer;
            if (bhead->blk_level != tree_level) {
                FSW_MSG_ASSERT((FSW_MSGSTR("fsw_reiserfs_item_next: tree block %d has not expected level %d\n"), tree_bno, tree_level));
                fsw_block_release(vol, tree_bno, buffer);
                return FSW_VOLUME_CORRUPTED;
            }
            nr_item = bhead->blk_nr_item;
            FSW_MSG_DEBUGV((FSW_MSGSTR("fsw_reiserfs_item_next: visiting block %d level %d items %d\n"), tree_bno, tree_level, nr_item));
            item->path_bno[tree_level] = tree_bno;
        }
        
        // get the item from the leaf node
        ihead = ((struct item_head *)(buffer + BLKH_SIZE)) + item->path_index[tree_level];
        
        // We now have the item that follows the previous one in the tree. Check that it
        // belongs to the same object.
        if (ihead->ih_key.k_dir_id != dir_id || ihead->ih_key.k_objectid != objectid) {
            fsw_block_release(vol, tree_bno, buffer);
            return FSW_NOT_FOUND;   // Found no next key for this object
        }
        
        // return results
        fsw_memcpy(&item->ih, ihead, sizeof(struct item_head));
        item->item_type = (fsw_u32)FSW_U64_SHR(ihead->ih_key.u.k_offset_v2.v, 60);
        if (item->item_type != TYPE_DIRECT &&
            item->item_type != TYPE_INDIRECT &&
            item->item_type != TYPE_DIRENTRY) {
            // 3.5 format (_v1)
            item->item_type = ihead->ih_key.u.k_offset_v1.k_uniqueness;
            item->item_offset = ihead->ih_key.u.k_offset_v1.k_offset;
        } else {
            // 3.6 format (_v2)
            item->item_offset = ihead->ih_key.u.k_offset_v2.v & (~0ULL >> 4);
        }
        item->item_data = buffer + ihead->ih_item_location;
        item->valid = 1;
        
        // add information for block release
        item->block_bno = tree_bno;
        item->block_buffer = buffer;
        
        FSW_MSG_DEBUG((FSW_MSGSTR("fsw_reiserfs_item_next: found %d/%d/%lld (%d)\n"),
                       ihead->ih_key.k_dir_id, ihead->ih_key.k_objectid, item->item_offset, item->item_type));
        return FSW_SUCCESS;
    }
    
    // we went to the highest level node and there still were no more items...
    return FSW_NOT_FOUND;
}

/**
 * Release the disk block still referenced by an item search result.
 */

static void fsw_reiserfs_item_release(struct fsw_reiserfs_volume *vol,
                                      struct fsw_reiserfs_item *item)
{
    if (!item->valid)
        return;
    
    if (item->block_bno > 0) {
        fsw_block_release(vol, item->block_bno, item->block_buffer);
        item->block_bno = 0;
    }
}

// EOF
