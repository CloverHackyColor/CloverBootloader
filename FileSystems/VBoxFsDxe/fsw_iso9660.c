/* $Id: fsw_iso9660.c 33540 2010-10-28 09:27:05Z vboxsync $ */
/** @file
 * fsw_iso9660.c - ISO9660 file system driver code.
 *
 * Current limitations:
 *  - Files must be in one extent (i.e. Level 2)
 *  - No Joliet or Rock Ridge extensions
 *  - No interleaving
 *  - inode number generation strategy fails on volumes > 2 GB
 *  - No blocksizes != 2048
 *  - No High Sierra or anything else != 'CD001'
 *  - No volume sets with directories pointing at other volumes
 *  - No extended attribute records
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

/*-
 * This code is based on:
 *
 * Copyright (c) 2006 Christoph Pfisterer
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *  * Neither the name of Christoph Pfisterer nor the names of the
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "fsw_iso9660.h"
#include <Protocol/MsgLog.h>

#ifndef DEBUG_ISO
#define DEBUG_ISO 1
#endif

#if DEBUG_ISO == 2
#define DBG(...)	AsciiPrint(__VA_ARGS__)
#elif DEBUG_ISO == 1
#define DBG(...)	BootLog(__VA_ARGS__)
#else
#define DBG(...)	
#endif

//#define MsgLog(x...) if(msgCursor){AsciiSPrint(msgCursor, BOOTER_LOG_SIZE, x); while(*msgCursor){msgCursor++;}}

extern CHAR8     *msgCursor;
extern MESSAGE_LOG_PROTOCOL *Msg;
// functions

static fsw_status_t fsw_iso9660_volume_mount(struct fsw_iso9660_volume *vol);
static void         fsw_iso9660_volume_free(struct fsw_iso9660_volume *vol);
static fsw_status_t fsw_iso9660_volume_stat(struct fsw_iso9660_volume *vol, struct fsw_volume_stat *sb);

static fsw_status_t fsw_iso9660_dnode_fill(struct fsw_iso9660_volume *vol, struct fsw_iso9660_dnode *dno);
static void         fsw_iso9660_dnode_free(struct fsw_iso9660_volume *vol, struct fsw_iso9660_dnode *dno);
static fsw_status_t fsw_iso9660_dnode_stat(struct fsw_iso9660_volume *vol, struct fsw_iso9660_dnode *dno,
                                           struct fsw_dnode_stat_str *sb);
static fsw_status_t fsw_iso9660_get_extent(struct fsw_iso9660_volume *vol, struct fsw_iso9660_dnode *dno,
                                           struct fsw_extent *extent);

static fsw_status_t fsw_iso9660_dir_lookup(struct fsw_iso9660_volume *vol, struct fsw_iso9660_dnode *dno,
                                           struct fsw_string *lookup_name, struct fsw_iso9660_dnode **child_dno);
static fsw_status_t fsw_iso9660_dir_read(struct fsw_iso9660_volume *vol, struct fsw_iso9660_dnode *dno,
                                         struct fsw_shandle *shand, struct fsw_iso9660_dnode **child_dno);
static fsw_status_t fsw_iso9660_read_dirrec(struct fsw_iso9660_volume *vol, struct fsw_shandle *shand, struct iso9660_dirrec_buffer *dirrec_buffer);

static fsw_status_t fsw_iso9660_readlink(struct fsw_iso9660_volume *vol, struct fsw_iso9660_dnode *dno,
                                         struct fsw_string *link);

static fsw_status_t rr_find_sp(struct iso9660_dirrec *dirrec, struct fsw_rock_ridge_susp_sp **psp);
static fsw_status_t rr_find_nm(struct fsw_iso9660_volume *vol, struct iso9660_dirrec *dirrec, int off, struct fsw_string *str);
static fsw_status_t rr_read_ce(struct fsw_iso9660_volume *vol, union fsw_rock_ridge_susp_ce *ce, fsw_u8 *begin);
//static void dump_dirrec(struct iso9660_dirrec *dirrec);
//
// Dispatch Table
//

struct fsw_fstype_table   FSW_FSTYPE_TABLE_NAME(iso9660) = {
    { FSW_STRING_TYPE_ISO88591, 4, 4, "iso9660" },
    sizeof(struct fsw_iso9660_volume),
    sizeof(struct fsw_iso9660_dnode),

    fsw_iso9660_volume_mount,
    fsw_iso9660_volume_free,
    fsw_iso9660_volume_stat,
    fsw_iso9660_dnode_fill,
    fsw_iso9660_dnode_free,
    fsw_iso9660_dnode_stat,
    fsw_iso9660_get_extent,
    fsw_iso9660_dir_lookup,
    fsw_iso9660_dir_read,
    fsw_iso9660_readlink,
};

static fsw_status_t rr_find_sp(struct iso9660_dirrec *dirrec, struct fsw_rock_ridge_susp_sp **psp)
{
    fsw_u8 *r;
    int off = 0;
    struct fsw_rock_ridge_susp_sp *sp;
    r = (fsw_u8 *)((fsw_u8 *)dirrec + sizeof(*dirrec) + dirrec->file_identifier_length);
    off = (int)(r - (fsw_u8 *)dirrec);
    while(off < dirrec->dirrec_length)
    {
        if (*r == 'S')
        {
            sp = (struct fsw_rock_ridge_susp_sp *)r;
            if(    sp->e.sig[0] == 'S'
                && sp->e.sig[1] == 'P'
                && sp->magic[0] == 0xbe
                && sp->magic[1] == 0xef)
            {
                *psp = sp;
                return FSW_SUCCESS;
            }
        }
        r++;
        off = (int)(r - (fsw_u8 *)dirrec);
    }
    *psp = NULL;
    return FSW_NOT_FOUND;
}

static fsw_status_t rr_find_nm(struct fsw_iso9660_volume *vol, struct iso9660_dirrec *dirrec, int off, struct fsw_string *str)
{
    fsw_u8 *r, *begin;
    int fCe = 0;
    struct fsw_rock_ridge_susp_nm *nm;
    int limit = dirrec->dirrec_length;
    begin = (fsw_u8 *)dirrec;
    r = (fsw_u8 *)dirrec + off;
    str->data = NULL;
    str->len = 0;
    str->size = 0;
    str->type = 0;
    while(off < limit)
    {
        if (r[0] == 'C' && r[1] == 'E' && r[2] == 28)
        {
            int rc;
            int ce_off;
            union fsw_rock_ridge_susp_ce *ce;
            if (fCe == 0)
                fsw_alloc_zero(ISO9660_BLOCKSIZE, (void *)&begin);
            fCe = 1;
        //    DEBUG((DEBUG_WARN, "%a:%d we found CE before NM or its continuation\n", __FILE__, __LINE__));
            ce = (union fsw_rock_ridge_susp_ce *)r;
            limit = ISOINT(ce->X.len);
            ce_off = ISOINT(ce->X.offset);
            rc = rr_read_ce(vol, ce, begin);
            if (rc != FSW_SUCCESS)
            {
                fsw_free(begin);
                return rc;
            }
            begin += ce_off;
            r = begin;
        }
        if (r[0] == 'N' && r[1] == 'M')
        {
            nm = (struct fsw_rock_ridge_susp_nm *)r;
            if(    nm->e.sig[0] == 'N'
                && nm->e.sig[1] == 'M')
            {
                int len = 0;
                fsw_u8 *tmp = NULL;
                if (nm->flags & RR_NM_CURR)
                {
                     fsw_memdup(&str->data, ".", 1);
                     str->len = 1;
                     goto done;
                }
                if (nm->flags & RR_NM_PARE)
                {
                     fsw_memdup(&str->data, "..", 2);
                     str->len = 2;
                     goto done;
                }
                len = nm->e.len - sizeof(struct fsw_rock_ridge_susp_nm) + 1;
                fsw_alloc_zero(str->len + len, (void **)&tmp);
                if (str->data != NULL)
                {
                    fsw_memcpy(tmp, str->data, str->len);
                    fsw_free(str->data);
                }
           //     DEBUG((DEBUG_INFO, "dst:%p src:%p len:%d\n", tmp + str->len, &nm->name[0], len));
                fsw_memcpy(tmp + str->len, &nm->name[0], len);
                str->data = tmp;
                str->len += len;

                if ((nm->flags & RR_NM_CONT) == 0)  //; -- as nms42 noted
                    goto done;
            }
        }
        r++;
        off = (int)(r - (fsw_u8 *)begin);
    }
    if(fCe == 1)
        fsw_free(begin);
    return FSW_NOT_FOUND;
done:
    str->type = FSW_STRING_TYPE_ISO88591;
    str->size = str->len;
    if(fCe == 1)
        fsw_free(begin);
    return FSW_SUCCESS;
}

static fsw_status_t rr_read_ce(struct fsw_iso9660_volume *vol, union fsw_rock_ridge_susp_ce *ce, fsw_u8 *begin)
{
    int rc;
//    int i;
//    fsw_u8 *r = begin + ISOINT(ce->X.offset);
//    int len = ISOINT(ce->X.len);
    rc = vol->g.host_table->read_block(&vol->g, ISOINT(ce->X.block_loc), begin);
    if (rc != FSW_SUCCESS)
        return rc;
/*    for (i = 0; i < len; ++i)
    {
        DEBUG((DEBUG_INFO, "%d: (%d:%x)%c ", i, r[i], r[i], r[i]));
    }*/
    return FSW_SUCCESS;
}
/*
static void dump_dirrec(struct iso9660_dirrec *dirrec)
{
    int i;
    fsw_u8 *r = (fsw_u8 *)dirrec + dirrec->file_identifier_length;
    int len = dirrec->dirrec_length;
    for (i = dirrec->file_identifier_length; i < len; ++i)
    {
        DEBUG((DEBUG_INFO, "%d: (%d:%x)%c ", i, r[i], r[i], r[i]));
    }
}*/
/**
 * Mount an ISO9660 volume. Reads the superblock and constructs the
 * root directory dnode.
 */

static fsw_status_t fsw_iso9660_volume_mount(struct fsw_iso9660_volume *vol)
{
    fsw_status_t    status;
    void            *buffer;
    fsw_u32         blockno;
    struct iso9660_volume_descriptor *voldesc;
    struct iso9660_primary_volume_descriptor *pvoldesc;
    fsw_u32         voldesc_type;
    int             i;
    struct fsw_string s;
 //   struct iso9660_dirrec rootdir;
    struct iso9660_dirrec *rootdir_p;
    int sua_pos;
    char *sig;
//    int skip;
    struct fsw_rock_ridge_susp_entry *entry;

    // read through the Volume Descriptor Set
    fsw_set_blocksize(vol, ISO9660_BLOCKSIZE, ISO9660_BLOCKSIZE);
    blockno = ISO9660_SUPERBLOCK_BLOCKNO;

    do {
      DBG("iso9660: check blockno=%d\n", blockno);
        status = fsw_block_get(vol, blockno, 0, &buffer);
        if (status)
            return status;

        voldesc = (struct iso9660_volume_descriptor *)buffer;
        voldesc_type = voldesc->volume_descriptor_type;
        if (fsw_memeq(voldesc->standard_identifier, "CD001", 5)) {
            // descriptor follows ISO 9660 standard
            if (voldesc_type == 1 && voldesc->volume_descriptor_version == 1) {
                // suitable Primary Volume Descriptor found
              DBG("iso9660: suitable Primary Volume Descriptor found\n");
                if (vol->primary_voldesc) {
                    fsw_free(vol->primary_voldesc);
                    vol->primary_voldesc = NULL;
                }
                status = fsw_memdup((void **)&vol->primary_voldesc, voldesc, ISO9660_BLOCKSIZE);
            }
        } else if (!fsw_memeq(voldesc->standard_identifier, "CD", 2)) {
            // completely alien standard identifier, stop reading
            voldesc_type = 255;
        }

        fsw_block_release(vol, blockno, buffer);
        blockno++;
    } while (!status && voldesc_type != 255);
    if (status)
        return status;

    // get information from Primary Volume Descriptor
    if (vol->primary_voldesc == NULL)
        return FSW_UNSUPPORTED;
    pvoldesc = vol->primary_voldesc;
    if (ISOINT(pvoldesc->logical_block_size) != 2048)
        return FSW_UNSUPPORTED;

    // get volume name
    for (i = 32; i > 0; i--)
        if (pvoldesc->volume_identifier[i-1] != ' ')
            break;
    s.type = FSW_STRING_TYPE_ISO88591;
    s.size = s.len = i;
    s.data = pvoldesc->volume_identifier;
    status = fsw_strdup_coerce(&vol->g.label, vol->g.host_string_type, &s);
    if (status)
        return status;

    // setup the root dnode
    status = fsw_dnode_create_root(vol, ISO9660_SUPERBLOCK_BLOCKNO << ISO9660_BLOCKSIZE_BITS, &vol->g.root);
    if (status)
        return status;
    fsw_memcpy(&vol->g.root->dirrec, &pvoldesc->root_directory, sizeof(struct iso9660_dirrec));

    if (   pvoldesc->escape[0] == 0x25
        && pvoldesc->escape[1] == 0x2f
        && (   pvoldesc->escape[2] == 0x40
            || pvoldesc->escape[2] == 0x43
            || pvoldesc->escape[2] == 0x45))
    {
 //       FSW_MSG_DEBUG((FSW_MSGSTR("fsw_iso9660_volume_mount: success (joliet!!!)\n")));
 //     DBG("fsw_iso9660_volume_mount: success (joliet!!!)\n");
        vol->fJoliet = 1;
    }


    //rootdir = pvoldesc->root_directory;
    rootdir_p = &pvoldesc->root_directory;
//  fsw_memcpy(&rootdir, &pvoldesc->root_directory, sizeof(struct iso9660_dirrec));
    sua_pos = (sizeof(struct iso9660_dirrec)) + rootdir_p->file_identifier_length + (rootdir_p->file_identifier_length % 2) - 2;
    //int sua_size = rootdir.dirrec_length - rootdir.file_identifier_length;
    //FSW_MSG_DEBUG((FSW_MSGSTR("fsw_iso9660_volume_mount: success (SUA(pos:%x, sz:%d)!!!)\n"), sua_pos, sua_size));

#if 1
    /*status = */fsw_block_get(vol, ISOINT(rootdir_p->extent_location), 0, &buffer);
    sig = (char *)buffer + sua_pos;
//    skip = 0;
    entry = (struct fsw_rock_ridge_susp_entry *)sig;
    if (   entry->sig[0] == 'S'
        && entry->sig[1] == 'P') {
        struct fsw_rock_ridge_susp_sp *sp = (struct fsw_rock_ridge_susp_sp *)entry;
        if (sp->magic[0] == 0xbe && sp->magic[1] == 0xef) {
            vol->fRockRidge = 1;
        } else {
 //           FSW_MSG_DEBUG((FSW_MSGSTR("fsw_iso9660_volume_mount: SP magic isn't valid\n")));
          DBG("fsw_iso9660_volume_mount: SP magic isn't valid\n");
        }
//        skip = sp->skip;
    }
#endif
    // release volume descriptors
    fsw_free(vol->primary_voldesc);
    vol->primary_voldesc = NULL;


//    FSW_MSG_DEBUG((FSW_MSGSTR("fsw_iso9660_volume_mount: success\n")));
  DBG("fsw_iso9660_volume_mount: success\n");

    return FSW_SUCCESS;
}

/**
 * Free the volume data structure. Called by the core after an unmount or after
 * an unsuccessful mount to release the memory used by the file system type specific
 * part of the volume structure.
 */

static void fsw_iso9660_volume_free(struct fsw_iso9660_volume *vol)
{
    if (vol->primary_voldesc)
        fsw_free(vol->primary_voldesc);
}

/**
 * Get in-depth information on a volume.
 */

static fsw_status_t fsw_iso9660_volume_stat(struct fsw_iso9660_volume *vol, struct fsw_volume_stat *sb)
{
    sb->total_bytes = 0; //(fsw_u64)vol->sb->s_blocks_count      * vol->g.log_blocksize;
    sb->free_bytes  = 0;
    return FSW_SUCCESS;
}

/**
 * Get full information on a dnode from disk. This function is called by the core
 * whenever it needs to access fields in the dnode structure that may not
 * be filled immediately upon creation of the dnode. In the case of iso9660, we
 * delay fetching of the inode structure until dnode_fill is called. The size and
 * type fields are invalid until this function has been called.
 */

static fsw_status_t fsw_iso9660_dnode_fill(struct fsw_iso9660_volume *vol, struct fsw_iso9660_dnode *dno)
{
    // get info from the directory record
    dno->g.size = ISOINT(dno->dirrec.data_length);
    if (dno->dirrec.file_flags & 0x02)
        dno->g.type = FSW_DNODE_TYPE_DIR;
    else
        dno->g.type = FSW_DNODE_TYPE_FILE;

    return FSW_SUCCESS;
}

/**
 * Free the dnode data structure. Called by the core when deallocating a dnode
 * structure to release the memory used by the file system type specific part
 * of the dnode structure.
 */

static void fsw_iso9660_dnode_free(struct fsw_iso9660_volume *vol, struct fsw_iso9660_dnode *dno)
{
}

/**
 * Get in-depth information on a dnode. The core makes sure that fsw_iso9660_dnode_fill
 * has been called on the dnode before this function is called. Note that some
 * data is not directly stored into the structure, but passed to a host-specific
 * callback that converts it to the host-specific format.
 */

static fsw_status_t fsw_iso9660_dnode_stat(struct fsw_iso9660_volume *vol, struct fsw_iso9660_dnode *dno,
                                           struct fsw_dnode_stat_str *sb)
{
    sb->used_bytes = (dno->g.size + (ISO9660_BLOCKSIZE-1)) & ~(ISO9660_BLOCKSIZE-1);
    /*
    sb->store_time_posix(sb, FSW_DNODE_STAT_CTIME, dno->raw->i_ctime);
    sb->store_time_posix(sb, FSW_DNODE_STAT_ATIME, dno->raw->i_atime);
    sb->store_time_posix(sb, FSW_DNODE_STAT_MTIME, dno->raw->i_mtime);
    sb->store_attr_posix(sb, dno->raw->i_mode);
    */

    return FSW_SUCCESS;
}

/**
 * Retrieve file data mapping information. This function is called by the core when
 * fsw_shandle_read needs to know where on the disk the required piece of the file's
 * data can be found. The core makes sure that fsw_iso9660_dnode_fill has been called
 * on the dnode before. Our task here is to get the physical disk block number for
 * the requested logical block number.
 */

static fsw_status_t fsw_iso9660_get_extent(struct fsw_iso9660_volume *vol, struct fsw_iso9660_dnode *dno,
                                           struct fsw_extent *extent)
{
    // Preconditions: The caller has checked that the requested logical block
    //  is within the file's size. The dnode has complete information, i.e.
    //  fsw_iso9660_dnode_read_info was called successfully on it.

    extent->type = FSW_EXTENT_TYPE_PHYSBLOCK;
    extent->phys_start = ISOINT(dno->dirrec.extent_location);
    extent->log_start = 0;
    extent->log_count = (ISOINT(dno->dirrec.data_length) + (ISO9660_BLOCKSIZE-1)) >> ISO9660_BLOCKSIZE_BITS;
    return FSW_SUCCESS;
}

/**
 * Lookup a directory's child dnode by name. This function is called on a directory
 * to retrieve the directory entry with the given name. A dnode is constructed for
 * this entry and returned. The core makes sure that fsw_iso9660_dnode_fill has been called
 * and the dnode is actually a directory.
 */

static fsw_status_t fsw_iso9660_dir_lookup(struct fsw_iso9660_volume *vol, struct fsw_iso9660_dnode *dno,
                                           struct fsw_string *lookup_name, struct fsw_iso9660_dnode **child_dno_out)
{
    fsw_status_t    status;
    struct fsw_shandle shand;
    struct iso9660_dirrec_buffer dirrec_buffer;
    struct iso9660_dirrec *dirrec = &dirrec_buffer.dirrec;

    // Preconditions: The caller has checked that dno is a directory node.

    // setup handle to read the directory
    status = fsw_shandle_open(dno, &shand);
    if (status)
        return status;

    // scan the directory for the file
    while (1) {
        // read next entry
        status = fsw_iso9660_read_dirrec(vol, &shand, &dirrec_buffer);
        if (status)
            goto errorexit;
        if (dirrec->dirrec_length == 0) {
            // end of directory reached
            status = FSW_NOT_FOUND;
            goto errorexit;
        }

        // skip . and ..
        if (dirrec->file_identifier_length == 1 &&
            (dirrec->file_identifier[0] == 0 || dirrec->file_identifier[0] == 1))
            continue;

        // compare name
        if (fsw_streq(lookup_name, &dirrec_buffer.name))  // TODO: compare case-insensitively
            break;
    }

    // setup a dnode for the child item
    status = fsw_dnode_create(dno, dirrec_buffer.ino, FSW_DNODE_TYPE_UNKNOWN, &dirrec_buffer.name, child_dno_out);
    if (status == FSW_SUCCESS)
        fsw_memcpy(&(*child_dno_out)->dirrec, dirrec, sizeof(struct iso9660_dirrec));

errorexit:
    fsw_shandle_close(&shand);
    return status;
}

/**
 * Get the next directory entry when reading a directory. This function is called during
 * directory iteration to retrieve the next directory entry. A dnode is constructed for
 * the entry and returned. The core makes sure that fsw_iso9660_dnode_fill has been called
 * and the dnode is actually a directory. The shandle provided by the caller is used to
 * record the position in the directory between calls.
 */

static fsw_status_t fsw_iso9660_dir_read(struct fsw_iso9660_volume *vol, struct fsw_iso9660_dnode *dno,
                                         struct fsw_shandle *shand, struct fsw_iso9660_dnode **child_dno_out)
{
    fsw_status_t    status;
    struct iso9660_dirrec_buffer dirrec_buffer;
    struct iso9660_dirrec *dirrec = &dirrec_buffer.dirrec;

    // Preconditions: The caller has checked that dno is a directory node. The caller
    //  has opened a storage handle to the directory's storage and keeps it around between
    //  calls.
    /* (vasily) directory nodes are 4096 bytes that is two logical blocks so read dir operation
     * should read both blocks.
     */

    while (1) {
        // read next entry
        if (shand->pos >= dno->g.size)
            return FSW_NOT_FOUND; // end of directory
        status = fsw_iso9660_read_dirrec(vol, shand, &dirrec_buffer);
        if (status)
            return status;
        if (dirrec->dirrec_length == 0)
        {
            // try the next block
            shand->pos =(shand->pos & ~(vol->g.log_blocksize - 1)) + vol->g.log_blocksize;
            continue;
        }

        // skip . and ..
        if (dirrec->file_identifier_length == 1 &&
            (dirrec->file_identifier[0] == 0 || dirrec->file_identifier[0] == 1))
            continue;
        break;
    }

    // setup a dnode for the child item
    status = fsw_dnode_create(dno, dirrec_buffer.ino, FSW_DNODE_TYPE_UNKNOWN, &dirrec_buffer.name, child_dno_out);
    if (status == FSW_SUCCESS)
        fsw_memcpy(&(*child_dno_out)->dirrec, dirrec, sizeof(struct iso9660_dirrec));

    return status;
}

/**
 * Read a directory entry from the directory's raw data. This internal function is used
 * to read a raw iso9660 directory entry into memory. The shandle's position pointer is adjusted
 * to point to the next entry.
 */

static fsw_status_t fsw_iso9660_read_dirrec(struct fsw_iso9660_volume *vol, struct fsw_shandle *shand, struct iso9660_dirrec_buffer *dirrec_buffer)
{
    fsw_status_t    status;
    fsw_u32         i, buffer_size, remaining_size, name_len;
    struct fsw_rock_ridge_susp_sp *sp = NULL;
    struct iso9660_dirrec *dirrec = &dirrec_buffer->dirrec;
    int sp_off;
    int rc;

    dirrec_buffer->ino = (ISOINT(((struct fsw_iso9660_dnode *)shand->dnode)->dirrec.extent_location)
                          << ISO9660_BLOCKSIZE_BITS)
        + (fsw_u32)shand->pos;

    // read fixed size part of directory record
    buffer_size = 33;
    status = fsw_shandle_read(shand, &buffer_size, dirrec);
    if (status)
    {
    //    DEBUG((DEBUG_INFO, "%a:%d \n", __FILE__, __LINE__));
        return status;
    }

    if (buffer_size < 33 || dirrec->dirrec_length == 0) {
        // end of directory reached
        fsw_u8 *r;
        r = (fsw_u8 *)dirrec;
 //       DEBUG((DEBUG_INFO, "%a:%d bs:%d dl:%d\n", __FILE__, __LINE__, buffer_size, dirrec->dirrec_length));
        for(i = 0; i < buffer_size; ++i)
        {
            DEBUG((DEBUG_INFO, "r[%d]:%c", i, r[i]));
        }
        dirrec->dirrec_length = 0;
        return FSW_SUCCESS;
    }
    if (dirrec->dirrec_length < 33 ||
        dirrec->dirrec_length < 33 + dirrec->file_identifier_length)
        return FSW_VOLUME_CORRUPTED;

//    DEBUG((DEBUG_INFO, "%a:%d, dirrec_length: %d\n", __FILE__, __LINE__, dirrec->dirrec_length));

    // read variable size part of directory record
    buffer_size = remaining_size = dirrec->dirrec_length - 33;
    status = fsw_shandle_read(shand, &buffer_size, dirrec->file_identifier);
    if (status)
        return status;
    if (buffer_size < remaining_size)
        return FSW_VOLUME_CORRUPTED;

//     dump_dirrec(dirrec);
     if (vol->fRockRidge) {
         sp_off = sizeof(*dirrec) + dirrec->file_identifier_length;
         rc = rr_find_sp(dirrec, &sp);
         if (   rc == FSW_SUCCESS
             && sp != NULL) {
            sp_off = (int)((fsw_u8 *)&sp[1] - (fsw_u8*)dirrec + sp->skip);
         }
         rc = rr_find_nm(vol, dirrec, sp_off,  &dirrec_buffer->name);
         if (rc == FSW_SUCCESS)
            return FSW_SUCCESS;
    }

    // setup name
    name_len = dirrec->file_identifier_length;
    for (i = name_len - 1; i > 0; i--) {
        if (dirrec->file_identifier[i] == ';') {
            name_len = i;   // cut the ISO9660 version number off
            break;
        }
    }
    if (name_len > 0 && dirrec->file_identifier[name_len-1] == '.')
        name_len--;   // also cut the extension separator if the extension is empty
    dirrec_buffer->name.type = FSW_STRING_TYPE_ISO88591;
    dirrec_buffer->name.len = dirrec_buffer->name.size = name_len;
    dirrec_buffer->name.data = dirrec->file_identifier;
//    DEBUG((DEBUG_INFO, "%a:%d: dirrec_buffer->name.data:%a\n", __FILE__, __LINE__, dirrec_buffer->name.data));
    return FSW_SUCCESS;
}

/**
 * Get the target path of a symbolic link. This function is called when a symbolic
 * link needs to be resolved. The core makes sure that the fsw_iso9660_dnode_fill has been
 * called on the dnode and that it really is a symlink.
 *
 * For iso9660, the target path can be stored inline in the inode structure (in the space
 * otherwise occupied by the block pointers) or in the inode's data. There is no flag
 * indicating this, only the number of blocks entry (i_blocks) can be used as an
 * indication. The check used here comes from the Linux kernel.
 */

static fsw_status_t fsw_iso9660_readlink(struct fsw_iso9660_volume *vol, struct fsw_iso9660_dnode *dno,
                                         struct fsw_string *link_target)
{
    fsw_status_t    status;

    if (dno->g.size > FSW_PATH_MAX)
        return FSW_VOLUME_CORRUPTED;

    status = fsw_dnode_readlink_data(dno, link_target);

    return status;
}

// EOF
