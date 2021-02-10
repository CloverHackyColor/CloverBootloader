/* $Id: fsw_hfs.c 33540 2010-10-28 09:27:05Z vboxsync $ */
/** @file
 * fsw_hfs.c - HFS file system driver code, see
 *
 *   http://developer.apple.com/technotes/tn/tn1150.html
 *
 * Current limitations:
 *  - Doesn't support permissions
 *  - Complete Unicode case-insensitiveness disabled (large tables)
 *  - links -> now supported symlinks and hardlinks. No aliasis.
 *  - Only supports pure HFS+ (i.e. no HFS, or HFS+ embedded to HFS)
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
// improved by nms42: VolumeName, links. 2014
// Cleanup, tested and corrected by Slice

#include "fsw_hfs.h"

#include <Library/MemLogLib.h>
#include <Library/PrintLib.h>


#define VBOXHFS_BTREE_BINSEARCH 0
#define DEBUG_HFS 0

#if DEBUG_HFS==2
#define DBG(...)  AsciiPrint(__VA_ARGS__)
#elif DEBUG_HFS==1
#define DBG(...) MemLog(TRUE, 1, __VA_ARGS__)
#else
#define DBG(...)
#endif

#if DEBUG_HFS == 2
#define CONCAT(x,y) x##y
#define DPRINT(x) Print(CONCAT(L,x))
#define DPRINT2(x,y) Print(CONCAT(L,x), y)
#define BP(msg) DPRINT(msg)
#else
#define CONCAT(x,y) x##y
#define DPRINT(x)
#define DPRINT2(x,y)
#define BP(msg)
#endif

#if defined(__GNUC__) && __GNUC__ >= 9
#pragma GCC diagnostic ignored "-Waddress-of-packed-member"
#endif

static int hardlink = 0;

static fsw_status_t fsw_hfs_volume_mount(struct fsw_hfs_volume *vol);
static void         fsw_hfs_volume_free(struct fsw_hfs_volume *vol);
static fsw_status_t fsw_hfs_volume_stat(struct fsw_hfs_volume *vol, struct fsw_volume_stat *sb);

static fsw_status_t fsw_hfs_dnode_fill(struct fsw_hfs_volume *vol, struct fsw_hfs_dnode *dno);
static void         fsw_hfs_dnode_free(struct fsw_hfs_volume *vol, struct fsw_hfs_dnode *dno);
static fsw_status_t fsw_hfs_dnode_stat(struct fsw_hfs_volume *vol, struct fsw_hfs_dnode *dno,
                                           struct fsw_dnode_stat_str *sb);
static fsw_status_t fsw_hfs_get_extent(struct fsw_hfs_volume *vol, struct fsw_hfs_dnode *dno,
                                           struct fsw_extent *extent);

static fsw_status_t fsw_hfs_dir_lookup(struct fsw_hfs_volume *vol, struct fsw_hfs_dnode *dno,
                                           struct fsw_string *lookup_name, struct fsw_hfs_dnode **child_dno);
static fsw_status_t fsw_hfs_dir_lookup_id(struct fsw_hfs_volume *vol,
                                          fsw_u32               lookup_id,
                                          file_info_t           *file_info);
static fsw_status_t fsw_hfs_dir_read(struct fsw_hfs_volume *vol, struct fsw_hfs_dnode *dno,
                                         struct fsw_shandle *shand, struct fsw_hfs_dnode **child_dno);
#if 0
static fsw_status_t fsw_hfs_read_dirrec(struct fsw_shandle *shand, struct hfs_dirrec_buffer *dirrec_buffer);
#endif

static fsw_status_t fsw_hfs_readlink(struct fsw_hfs_volume *vol,
                                     struct fsw_hfs_dnode *dno,
                                     struct fsw_string *link);

//
// Dispatch Table
//

struct fsw_fstype_table   FSW_FSTYPE_TABLE_NAME(hfs) = {
    { FSW_STRING_TYPE_ISO88591, 4, 4, "hfs" },
    sizeof(struct fsw_hfs_volume),
    sizeof(struct fsw_hfs_dnode),

    fsw_hfs_volume_mount, // volume open
    fsw_hfs_volume_free,  // volume close
    fsw_hfs_volume_stat,  // volume info: total_bytes, free_bytes
    fsw_hfs_dnode_fill,   //return FSW_SUCCESS;
    fsw_hfs_dnode_free,	  // empty
    fsw_hfs_dnode_stat,	 //size and times
    fsw_hfs_get_extent,	 // get the physical disk block number for the requested logical block number
    fsw_hfs_dir_lookup,  //retrieve the directory entry with the given name
    fsw_hfs_dir_read,	// next directory entry when reading a directory
    fsw_hfs_readlink,   // ;
};

static fsw_s32
fsw_hfs_read_block (struct fsw_hfs_dnode    *dno,
                    fsw_u32                 log_bno,
                    fsw_u32                 off,
                    fsw_s32                 len,
                    fsw_u8                  *buf)
{
  fsw_status_t          status;
  struct fsw_extent     extent;
  fsw_u32               phys_bno;
  fsw_u8                *buffer;
  
  extent.log_start = log_bno;
  status = fsw_hfs_get_extent(dno->g.vol, dno, &extent);
  if (status)
    return status;
  
  phys_bno = extent.phys_start;
  //Slice - increase cache level from 0 to 3
  status = fsw_block_get(dno->g.vol, phys_bno, 3, (void **)&buffer);
  if (status)
    return status;
  
  fsw_memcpy(buf, buffer + off, len);
  fsw_block_release(dno->g.vol, phys_bno, buffer);
  return FSW_SUCCESS;
}

/* Read data from HFS file. */
static fsw_s32
fsw_hfs_read_file (struct fsw_hfs_dnode    * dno,
                   fsw_u64                   pos,
                   fsw_s32                   len,
                   fsw_u8                  * buf)
{
  fsw_status_t          status;
  fsw_u32               log_bno;
  fsw_u32               block_size_bits = dno->g.vol->block_size_shift;
  fsw_u32               block_size = (1 << block_size_bits);
  fsw_u32               block_size_mask = block_size - 1;
  fsw_s32               read = 0;
  
  while (len > 0) {
    fsw_u32 off = (fsw_u32) (pos & block_size_mask);
    fsw_s32 next_len = len;
    
    log_bno = (fsw_u32)RShiftU64(pos, block_size_bits);
    
    if ((next_len >= 0) && ((fsw_u32)next_len > block_size)) {
      next_len = block_size;
    }
    status = fsw_hfs_read_block(dno, log_bno, off, next_len, buf);
    if (status)
      return -1;
    buf  += next_len;
    pos  += next_len;
    len  -= next_len;
    read += next_len;
  }
  hardlink = 0; //this is static value that me must initialize sometimes 
  return read;
}

static fsw_s32
fsw_hfs_compute_shift(fsw_u32 size)
{
  fsw_s32 i;
  
  for (i = 0; i < 32; i++) {
    if ((size >> i) == 0)
      return i - 1;
  }
  return 0;
}

/**
 * Mount an HFS+ volume. Reads the superblock and constructs the
 * root directory dnode.
 */

static fsw_status_t fsw_hfs_volume_mount(struct fsw_hfs_volume *vol)
{
  fsw_status_t           status, rv;
  void                  *buffer = NULL;
  HFSPlusVolumeHeader   *voldesc;
  fsw_u32                blockno;
  struct fsw_string      s;
  HFSMasterDirectoryBlock* mdb;
  INTN i;
  
  rv = FSW_UNSUPPORTED;
  
  vol->primary_voldesc = NULL;
  fsw_set_blocksize(vol, HFS_BLOCKSIZE, HFS_BLOCKSIZE);
  blockno = HFS_SUPERBLOCK_BLOCKNO;
  
#define CHECK(s)         \
        if (status)  {   \
            rv = status; \
            break;       \
        }
  
  vol->emb_block_off = 0;
  vol->hfs_kind = 0;
  do {
    fsw_u16       signature;
    BTHeaderRec   tree_header;
    fsw_s32       r;
    fsw_u32       block_size;
    
    status = fsw_block_get(vol, blockno, 0, &buffer);
    CHECK(status);
    voldesc = (HFSPlusVolumeHeader *)buffer;
    mdb = (HFSMasterDirectoryBlock*)buffer;
    signature = be16_to_cpu(voldesc->signature);
    
    if ((signature == kHFSPlusSigWord) || (signature == kHFSXSigWord)) { //H+ or HX
      if (vol->hfs_kind == 0) {
//        DBG("found HFS+\n");
        vol->hfs_kind = FSW_HFS_PLUS;
      }
    } else if (signature == kHFSSigWord) {// 'BD'
      //            HFSMasterDirectoryBlock* mdb = (HFSMasterDirectoryBlock*)buffer;
      //VolumeName = mdb->drVN 28bytes
      if (be16_to_cpu(mdb->drEmbedSigWord) == kHFSPlusSigWord) {
        DBG("found HFS+ inside HFS, untested\n");
        vol->hfs_kind = FSW_HFS_PLUS_EMB;
        vol->emb_block_off = be32_to_cpu(mdb->drEmbedExtent.startBlock);
        fsw_block_release (vol, blockno, buffer);
        blockno += vol->emb_block_off;
        /* retry */
        continue;
      } else {
        DBG("found plain HFS, unsupported\n");
        vol->hfs_kind = FSW_HFS_PLAIN;
      }
      rv = FSW_UNSUPPORTED;
      break;
    } else {
      rv = FSW_UNSUPPORTED;
      break;
    }
    
    status = fsw_memdup((void **)&vol->primary_voldesc, voldesc,
                        sizeof(HFSPlusVolumeHeader));
    CHECK(status);
    
    block_size = be32_to_cpu(voldesc->blockSize);
    vol->block_size_shift = fsw_hfs_compute_shift(block_size);
//    DBG("vol block_size=%d\n", block_size);
    fsw_block_release(vol, blockno, buffer);
    buffer = NULL;
    voldesc = NULL;
    fsw_set_blocksize(vol, block_size, block_size);
    
    /* get volume name */
    for (i = kHFSMaxVolumeNameChars; i > 0; i--)
      if (mdb->drVN[i-1] != ' ')
        break;
    
    s.type = FSW_STRING_TYPE_ISO88591;
    s.size = s.len = (int)i;
    s.data = &mdb->drVN; //"HFS+ volume";
    
    //fsw_status_t fsw_strdup_coerce(struct fsw_string *dest, int type, struct fsw_string *src)
    status = fsw_strdup_coerce(&vol->g.label, vol->g.host_string_type, &s);
    CHECK(status);
    DBG("HW Volume label:");
    for (i=0; i<s.len; i++) {
      DBG("%02x ", ((fsw_u16*)vol->g.label.data)[i]);
    }
    DBG("\n");

    
    /* Setup catalog dnode */
    status = fsw_dnode_create_root(vol, kHFSCatalogFileID, &vol->catalog_tree.file);
    CHECK(status);
    //Slice - why copy structure?
    fsw_memcpy (vol->catalog_tree.file->extents,
                vol->primary_voldesc->catalogFile.extents,
                sizeof vol->catalog_tree.file->extents);
    
    vol->catalog_tree.file->g.size =
    be64_to_cpu_ua(&vol->primary_voldesc->catalogFile.logicalSize);
    
    /* Setup extents overflow file */
    /*status = */fsw_dnode_create_root(vol, kHFSExtentsFileID, &vol->extents_tree.file);
    fsw_memcpy (vol->extents_tree.file->extents,
                vol->primary_voldesc->extentsFile.extents,
                sizeof vol->extents_tree.file->extents);
    vol->extents_tree.file->g.size =
    be64_to_cpu_ua(&vol->primary_voldesc->extentsFile.logicalSize);
    
    /* Setup the root dnode */
    status = fsw_dnode_create_root(vol, kHFSRootFolderID, &vol->g.root);
    CHECK(status);
    
    /*
     * Read catalog file, we know that first record is in the first node, right after
     * the node descriptor.
     */
    r = fsw_hfs_read_file(vol->catalog_tree.file,
                          sizeof (BTNodeDescriptor),
                          sizeof (BTHeaderRec), (fsw_u8 *) &tree_header);
    if (r <= 0) {
      status = FSW_VOLUME_CORRUPTED;
      DBG("bad read catalog file\n");
      break;
    }
    vol->case_sensitive =
    (signature == kHFSXSigWord) &&
    (tree_header.keyCompareType == kHFSBinaryCompare);
    vol->catalog_tree.root_node = be32_to_cpu (tree_header.rootNode);
    vol->catalog_tree.node_size = be16_to_cpu (tree_header.nodeSize);
    //nms42
    /* Take Volume Name before tree_header overwritten */
    {
      fsw_u32 firstLeafNum;
      fsw_u64 catfOffset;
      fsw_u8 cbuff[sizeof (BTNodeDescriptor) + sizeof (HFSPlusCatalogKey)];
      
      firstLeafNum = be32_to_cpu(tree_header.firstLeafNode);
      catfOffset = firstLeafNum * vol->catalog_tree.node_size;
      
      r = fsw_hfs_read_file(vol->catalog_tree.file, catfOffset, sizeof (cbuff), cbuff);
      
      if (r == sizeof (cbuff)) {
        BTNodeDescriptor *btnd;
        HFSPlusCatalogKey *ck;
        
        btnd = (BTNodeDescriptor*) cbuff;
        ck = (HFSPlusCatalogKey*) (cbuff + sizeof(BTNodeDescriptor));
   //     DBG(" btnd->kind ==%d expected -1\n", btnd->kind);
  //      DBG(" ck->parentID ==%d expected 1\n", be32_to_cpu (ck->parentID));
        if (btnd->kind == kBTLeafNode &&
            be32_to_cpu_ua ((fsw_u32 *)(((UINT8 *)ck) + OFFSET_OF(HFSPlusCatalogKey, parentID))) == kHFSRootParentID) {
          struct fsw_string vn;
          
          vn.type = FSW_STRING_TYPE_UTF16_BE;
          vn.len = be16_to_cpu (ck->nodeName.length);
          vn.size = vn.len * sizeof (fsw_u16);
          vn.data = ck->nodeName.unicode;
          fsw_strfree (&vol->g.label);
          status = fsw_strdup_coerce(&vol->g.label, FSW_STRING_TYPE_UTF16, &vn);
          DBG("Volume label:");
          for (i=0; i<vn.len; i++) {
            DBG("%02x ", ((fsw_u16*)vol->g.label.data)[i]);
          }
          DBG("\n");
          CHECK(status);
        }
      }
    }
    
    /* Read extents overflow file */
    r = fsw_hfs_read_file(vol->extents_tree.file,
                          sizeof (BTNodeDescriptor),
                          sizeof (BTHeaderRec), (fsw_u8 *) &tree_header);
    if (r != sizeof (BTHeaderRec)) {
      status = FSW_VOLUME_CORRUPTED;
      DBG("extents overflow file CORRUPTED\n");
      break;
    }
    
    vol->extents_tree.root_node = be32_to_cpu (tree_header.rootNode);
    vol->extents_tree.node_size = be16_to_cpu (tree_header.nodeSize);
    
    rv = FSW_SUCCESS;
    break;
  } while (1);
  
#undef CHECK
  
  if (buffer != NULL) {
    fsw_block_release(vol, blockno, buffer);
  }
  
  return rv;
}

/**
 * Free the volume data structure. Called by the core after an unmount or after
 * an unsuccessful mount to release the memory used by the file system type specific
 * part of the volume structure.
 */

static void fsw_hfs_volume_free(struct fsw_hfs_volume *vol)
{
  if (vol->primary_voldesc) {
    fsw_free(vol->primary_voldesc);
    vol->primary_voldesc = NULL;
  }
  if (vol->catalog_tree.file) {
    fsw_dnode_release((struct fsw_dnode*)(vol->catalog_tree.file));
    vol->catalog_tree.file = NULL;
  }
  if (vol->extents_tree.file) {
    fsw_dnode_release((struct fsw_dnode*)(vol->extents_tree.file));
    vol->extents_tree.file = NULL;
  }
}

/**
 * Get in-depth information on a volume.
 */

static fsw_status_t fsw_hfs_volume_stat(struct fsw_hfs_volume *vol, struct fsw_volume_stat *sb)
{
  sb->total_bytes = LShiftU64(be32_to_cpu(vol->primary_voldesc->totalBlocks), vol->block_size_shift);
  sb->free_bytes = LShiftU64(be32_to_cpu(vol->primary_voldesc->freeBlocks), vol->block_size_shift);
  return FSW_SUCCESS;
}

/**
 * Get full information on a dnode from disk. This function is called by the core
 * whenever it needs to access fields in the dnode structure that may not
 * be filled immediately upon creation of the dnode.
 */

static fsw_status_t fsw_hfs_dnode_fill(struct fsw_hfs_volume *vol, struct fsw_hfs_dnode *dno)
{
//  Print(L"dnode_fill\n");
  return FSW_SUCCESS;
}

/**
 * Free the dnode data structure. Called by the core when deallocating a dnode
 * structure to release the memory used by the file system type specific part
 * of the dnode structure.
 */

static void fsw_hfs_dnode_free(struct fsw_hfs_volume *vol, struct fsw_hfs_dnode *dno)
{
}

static fsw_u32 mac_to_posix(fsw_u32 mac_time)
{
  /* Mac time is 1904 year based */
  return mac_time ?  mac_time - 2082844800 : 0;
}

/**
 * Get in-depth information on a dnode. The core makes sure that fsw_hfs_dnode_fill
 * has been called on the dnode before this function is called. Note that some
 * data is not directly stored into the structure, but passed to a host-specific
 * callback that converts it to the host-specific format.
 */

static fsw_status_t fsw_hfs_dnode_stat(struct fsw_hfs_volume *vol,
                                       struct fsw_hfs_dnode  *dno,
                                       struct fsw_dnode_stat_str *sb)
{
  sb->used_bytes = dno->used_bytes;
//  DBG("dnode_stat used_bytes =%ld\n", sb->used_bytes);
  sb->store_time_posix(sb, FSW_DNODE_STAT_CTIME, mac_to_posix(dno->ctime));
  sb->store_time_posix(sb, FSW_DNODE_STAT_MTIME, mac_to_posix(dno->mtime));
  sb->store_time_posix(sb, FSW_DNODE_STAT_ATIME, 0);
  sb->store_attr_posix(sb, 0700);

  return FSW_SUCCESS;
}

static int
fsw_hfs_find_block(HFSPlusExtentRecord *exts,
                   fsw_u32             *lbno,
                   fsw_u32             *pbno)
{
  int i;
  fsw_u32 cur_lbno = *lbno;
//  DBG("search for lbno=%d\n", *lbno);
  
  for (i = 0; i < 8; i++) {
    fsw_u32 start = be32_to_cpu ((*exts)[i].startBlock);
    fsw_u32 count = be32_to_cpu ((*exts)[i].blockCount);
    
    if (cur_lbno < count) {
      *pbno = start + cur_lbno;
//      DBG("found block %d in segment %d\n", *pbno, i);
      return 1;
    }
    cur_lbno -= count;
  }
  
  *lbno = cur_lbno;
  return 0;
}

//
// Find record offset, numbering starts from the end
//
static fsw_u32
fsw_hfs_btree_recoffset (struct fsw_hfs_btree *btree,
                         BTNodeDescriptor     *node,
                         fsw_u32              index)
{ //+
  fsw_u8 *cnode = (fsw_u8 *)node;
  fsw_u16 *recptr;
  fsw_u32 offset = (fsw_u32)be16_to_cpu(node->numRecords);
  if (index >= offset) {
    DBG(" index is too large %d > %d for this node\n", index, offset);
    return 0; //impossible for good run
  }
  recptr = (fsw_u16 *)(cnode + btree->node_size - index * 2 - 2);
  offset = (fsw_u32)be16_to_cpu(*recptr);
  return offset;
}

//
// Pointer to the key record by index inside the node
//
static BTreeKey *
fsw_hfs_btree_rec (struct fsw_hfs_btree   *btree,
                   BTNodeDescriptor       *node,
                   fsw_u32                index)
{ //+
  fsw_u8 *cnode = (fsw_u8 *)node;
  fsw_u32 offset;
  offset = fsw_hfs_btree_recoffset(btree, node, index);
  if (offset < sizeof(BTNodeDescriptor) || offset > btree->node_size) {
    DBG(" wrong offset %d\n", offset);
    return NULL;
  }
  return (BTreeKey *)(cnode + offset);
}

//
//  returns pointer to data of the index record
//
static fsw_u32
fsw_hfs_btree_next_node (BTreeKey *currkey)
{ //+
  fsw_u32 *pointer;
  
  pointer = (fsw_u32 *)((char *) currkey + be16_to_cpu (currkey->length16) + 2);
  return be32_to_cpu_ua(pointer);
}

static fsw_status_t
fsw_hfs_btree_search (struct fsw_hfs_btree *btree,
                      BTreeKey             *key,
                      int (*compare_keys) (BTreeKey *key1, BTreeKey *key2),
                      BTNodeDescriptor    **result,
                      fsw_u32              *key_offset)
{
  fsw_status_t status;
  fsw_u8 *buffer = NULL;
  BTNodeDescriptor *node;
  fsw_u32 currnode;
  fsw_u32 recnum;
#ifdef VBOXHFS_BTREE_BINSEARCH
  fsw_u32 lower, upper;
#endif

  currnode = btree->root_node; //always from root?
  status = fsw_alloc(btree->node_size, &buffer);
  if (status != FSW_SUCCESS) {
    fsw_free(buffer);
    return status;
  }
  node = (BTNodeDescriptor *) buffer;

  for (;;) { //node cycle
    fsw_s32 cmp = 0;
//    int match;
    fsw_u32 count;
    BTreeKey *currkey;

//    match = 0;
    /* Read a node */
    if ((fsw_u32)fsw_hfs_read_file(btree->file,
                                   MultU64x32(currnode, btree->node_size),
                                   btree->node_size, buffer) !=
        btree->node_size) {
          status = FSW_VOLUME_CORRUPTED;
          DBG("differ node size while read file\n");
          break;
        }
//check record0 pointing to end of descriptor
    if (be16_to_cpu (*(fsw_u16 *) (buffer + btree->node_size - 2)) !=
        sizeof (BTNodeDescriptor)) {
      status = FSW_VOLUME_CORRUPTED;
      DBG("differ BTNodeDescriptor\n");
      break;
    }
    count = be16_to_cpu (node->numRecords);

#ifndef VBOXHFS_BTREE_BINSEARCH
    //preliminary test for last record. May be not needed look all for this node
    currkey = fsw_hfs_btree_rec (btree, node, count - 1);
    cmp = compare_keys (currkey, key);
    if ((cmp == 0) && (node->kind == kBTLeafNode)) {
      /* Found!  */
      *result = node;
      *key_offset = recnum;
      hardlink = 0;
      return FSW_SUCCESS;
    } else if (cmp < 0) { //all records has key less than searched
      if (node->kind == kBTIndexNode) {
        currnode = fsw_hfs_btree_next_node (currkey);
      } else if (node->fLink) {
        currnode = be32_to_cpu (node->fLink);
      } else {
        status = FSW_NOT_FOUND;
        break;
      }
      continue;
    } //else search for all records
    
    for (recnum = 0; recnum < count; recnum++) {
      currkey = fsw_hfs_btree_rec (btree, node, recnum);
      cmp = compare_keys (currkey, key);  //fsw_hfs_cmpi_catkey
 //     DBG(": currnode %d rec=%d count %d cmp=%d kind=%d\n",
 //         currnode, recnum, count, cmp, node->kind);

      /* Leaf node. */
      if (node->kind == kBTLeafNode) {
        if (cmp == 0) {
          /* Found!  */
          *result = node;
          *key_offset = recnum;
          hardlink = 0;
          return FSW_SUCCESS;
        }
      } else if (node->kind == kBTIndexNode) {
        if (cmp > 0)
          break;
        currnode = fsw_hfs_btree_next_node (currkey); //largest that <= search
      }
    }
    if (node->kind == kBTLeafNode) {
      status = FSW_NOT_FOUND;
      break;
    }

    if (cmp <= 0 && node->fLink) {
      currnode = be32_to_cpu (node->fLink);
    }
#else
    /* Perform binary search */
    lower = 0;
    upper = count - 1;
    currkey = NULL;

    if (count == 0) {
      status = FSW_NOT_FOUND;
      goto done;
    }

    while (lower <= upper) {
      recnum = (lower + upper) / 2;

      currkey = fsw_hfs_btree_rec (btree, node, recnum);

      cmp = compare_keys (currkey, key);  //fsw_hfs_cmpi_catkey
 //     DBG(": currnode %d lower/recnum/upper %d/%d/%d (%d) cmp=%d kind=%d\n",
 //         currnode, lower, recnum, upper, count, cmp, node->kind);
      if (cmp > 0) {
        upper = recnum - 1;
      } else if (cmp < 0) {
        lower = recnum + 1;
      } else if (cmp == 0) {
        if (node->kind == kBTLeafNode) {
          // Found!
          *result = node;
          *key_offset = recnum;
          return FSW_SUCCESS;

        } else if (node->kind == kBTIndexNode) {
          currnode = fsw_hfs_btree_next_node (currkey);
          break;
        }
      }
    }

    if (cmp > 0) {
      currkey = fsw_hfs_btree_rec (btree, node, upper);
    }

    if (node->kind == kBTIndexNode && currkey != NULL) {
      currnode = fsw_hfs_btree_next_node (currkey);
 //     DBG(": candidate for the next currnode is %d\n", currnode);
    }
    else {
      status = FSW_NOT_FOUND;
      break;
    }
#endif
  }

#ifdef VBOXHFS_BTREE_BINSEARCH
done:
#endif
  if (buffer != NULL && status != FSW_SUCCESS)
    fsw_free(buffer);
  
  return status;
}

static void
fill_fileinfo (
               struct fsw_hfs_volume* vol,
               HFSPlusCatalogKey* key,
               file_info_t* finfo
               )
{
  fsw_u8* base;
  fsw_u16 rec_type;
  fsw_u16 flags;

  /* for plain HFS "-(keySize & 1)" would be needed */
  base = (fsw_u8 *) key + be16_to_cpu (key->keyLength) + 2;
  rec_type = be16_to_cpu (*(fsw_u16 *) base);

    /** @todo: read additional info */
  switch (rec_type) {
  case kHFSPlusFolderRecord:
    {
      HFSPlusCatalogFolder *info = (HFSPlusCatalogFolder *) base;

      finfo->id = be32_to_cpu_ua ((fsw_u32*)(void*)&info->folderID);
      finfo->type = FSW_DNODE_TYPE_DIR;
      /* @todo: return number of elements, maybe use smth else */
      finfo->size = be32_to_cpu (info->valence); //this is wrong because of deleted entries
      finfo->used = be32_to_cpu (info->valence);
      finfo->ctime = be32_to_cpu (info->createDate);
      finfo->mtime = be32_to_cpu (info->contentModDate);
//      DBG("number of folder entries=%d\n", finfo->size);
//      DBG("  fileMode=%x\n", finfo->fileMode);
//      DBG("  flags=%x\n", be16_to_cpu(info->flags));
//      DBG("  folderID=%d\n\n", finfo->id);
      break;
    }
  case kHFSPlusFileThreadRecord:
    {  //never happen
      HFSPlusCatalogThread *info = (HFSPlusCatalogThread *) base;
      finfo->id = be32_to_cpu_ua((fsw_u32*)(void*)&info->parentID);
      finfo->type = FSW_DNODE_TYPE_SPECIAL;
//      DBG("  CatalogThread\n");
//      DBG("  folderID=%d\n\n", finfo->id);
      
    }
  case kHFSPlusFileRecord:
    {
      HFSPlusCatalogFile *info = (HFSPlusCatalogFile *) base;
      file_info_t  tmp_finfo;
      fsw_status_t status;

      finfo->id = be32_to_cpu (info->fileID);
      flags = be16_to_cpu(info->flags);
      finfo->creator = be32_to_cpu (info->userInfo.fdCreator);
      finfo->crtype = be32_to_cpu (info->userInfo.fdType);
      finfo->type = FSW_DNODE_TYPE_FILE;
      finfo->ilink = 0;
      finfo->isfilelink = 0;
      finfo->isdirlink = 0;
      DBG("file creator=%x crtype=%x\n", finfo->creator, finfo->crtype);
      /* Is the file any kind of link? */
      if ((finfo->creator == kSymLinkCreator && finfo->crtype == kSymLinkFileType) ||
          (finfo->creator == kHFSPlusCreator && finfo->crtype == kHardLinkFileType)) {
     //   if (finfo->crtype == kSymLinkFileType) {
          finfo->type = FSW_DNODE_TYPE_SYMLINK;
     //   }
        finfo->isfilelink = 1;
      } else if ((flags & kHFSHasLinkChainMask) &&
                 (finfo->crtype == kHFSAliasType) &&
                 (finfo->creator == kHFSAliasCreator)) {
        DBG("found dirlink, fileID=%d\n", finfo->id);
        finfo->isdirlink = 0; //do nothing for now
      }
      if ((finfo->isfilelink || finfo->isdirlink)/* && !(flags & HFS_LOOKUP_HARDLINK)*/) {
        finfo->ilink = be32_to_cpu_ua ((fsw_u32*)(void*)&info->bsdInfo.special.iNodeNum);
        if (finfo->type != FSW_DNODE_TYPE_SYMLINK) {
          status = fsw_hfs_dir_lookup_id(vol, finfo->ilink, &tmp_finfo);
          if (!status) {
 //           DBG("hardlink resolved to fileID=%d\n", finfo->ilink);
            finfo->id = finfo->ilink;
            finfo->ilink = 0;
            finfo->size = tmp_finfo.size;
            finfo->used = tmp_finfo.used;
            finfo->ctime = be32_to_cpu (info->createDate);
            finfo->mtime = be32_to_cpu (info->contentModDate);
            fsw_memcpy (&finfo->extents, &tmp_finfo.extents,
                        sizeof(HFSPlusExtentRecord));
            break;
          }
        }
      }
/*      DBG("file type=");
      if (finfo->isfilelink && finfo->crtype == kHardLinkFileType) {
        DBG("hardlink to fileID=%d\n", finfo->ilink);
      } else if (finfo->isdirlink) {
        DBG("dirlink\n");
      } else {
        switch (finfo->type) {
        case FSW_DNODE_TYPE_SYMLINK:
          DBG("symlink\n");
          break;
        case FSW_DNODE_TYPE_FILE:
            DBG("file\n");
          break;

        default:
          DBG("xxx\n");
          break;
        }
      } */
      finfo->fileMode = be16_to_cpu(info->bsdInfo.fileMode);
//      DBG("  fileMode=%x\n", finfo->fileMode);
//      DBG("  flags=%x\n", flags);
//      DBG("  fileID=%d\n", finfo->id);
      finfo->size = be64_to_cpu_ua((fsw_u64*)(void*)&info->dataFork.logicalSize);
      finfo->used =
        LShiftU64 ((fsw_u64)be32_to_cpu_ua ((fsw_u32*)(void*)&info->dataFork.totalBlocks),
                   vol->block_size_shift);
//      DBG("file size=%ld, used=%ld\n\n", finfo->size, finfo->used);
//      if (finfo->size == 0) {
        //TODO - how to find the real file?
 //       DBG("file info:\n");
 //       DBG("\tdataFork.logicalSize=%d\n", info->dataFork.logicalSize);
 //       DBG("\tdataFork.totalBlocks=%d\n", info->dataFork.totalBlocks);
//        DBG("\tresourceFork.logicalSize=%d\n", info->resourceFork.logicalSize);
//        DBG("\tresourceFork.totalBlocks=%d\n", info->resourceFork.totalBlocks);

//      }
      finfo->ctime = be32_to_cpu (info->createDate);
      finfo->mtime = be32_to_cpu (info->contentModDate);
      fsw_memcpy (&finfo->extents, &info->dataFork.extents,
                  sizeof(HFSPlusExtentRecord));
      break;
    }
  default:
    finfo->type = FSW_DNODE_TYPE_UNKNOWN;
    break;
  }
}

typedef struct {
    fsw_u32                 cur_pos; /* current position */
    fsw_u32                 parent;
    struct fsw_hfs_volume   *vol;
    struct fsw_shandle      *shandle; /* this one track iterator's state */
    file_info_t             file_info;
} visitor_parameter_t;

///
//  at input: record = fsw_hfs_btree_rec (btree, node, i) - having
//      param contains file name and folder ID            - wishing
//      for directory list name=0
//
static int
fsw_hfs_btree_visit_node(BTreeKey *record, void *param)
{
  visitor_parameter_t *vp       = (visitor_parameter_t*)param;
  fsw_u8              *base     = (fsw_u8*)record->rawData + be16_to_cpu(record->length16) + 2; //data next to key
  fsw_u16             rec_type  =  be16_to_cpu(*(fsw_u16*)base);
  HFSPlusCatalogKey   *cat_key  = (HFSPlusCatalogKey*)record;
  fsw_u16   name_len;
  fsw_u16   *name_ptr;
  fsw_u32   i;
  struct fsw_string *file_name;
  
  i = be32_to_cpu(cat_key->parentID);
  if (vp->parent != 0 && i != vp->parent) { //in some cases we wish no check parentID
//    DBG("cat ID=%x while parentID=%x\n", i, vp->parent);
//    vp->shandle->pos++;
    return -1;
  }
  
  /* not smth we care about */
  if (vp->shandle->pos != vp->cur_pos++)
    return 0;
  
  fill_fileinfo (vp->vol, cat_key, &vp->file_info);

  switch (rec_type) {
    case kHFSPlusFolderThreadRecord:
    case kHFSPlusFileThreadRecord:
    {
//      DBG("skip thread\n");
      vp->shandle->pos++;
      return 0;
    }
    default:
      break;
  }
  
  name_len = be16_to_cpu(cat_key->nodeName.length);
  
  file_name =  vp->file_info.name;
  file_name->len = name_len;
  fsw_memdup(&file_name->data, &cat_key->nodeName.unicode[0], 2*name_len);
  file_name->size = 2*name_len;
  file_name->type = FSW_STRING_TYPE_UTF16;
  name_ptr = (fsw_u16*)file_name->data;
  for (i = 0; i < name_len; i++) {
    name_ptr[i] = be16_to_cpu(name_ptr[i]);
  }
  vp->shandle->pos++;
/*  DBG("   visited file=", name_ptr);
  for (i=0; i<name_len; i++) {
    DBG("%c", name_ptr[i]?name_ptr[i]:L'@');
  }
  DBG("\n"); */
  return 1;
}

static fsw_status_t
fsw_hfs_btree_iterate_node (struct fsw_hfs_btree *btree,
                            BTNodeDescriptor     *first_node,
                            fsw_u32              first_rec,
                            int                  (*callback) (BTreeKey *record, void* param),
                            void                 *param)
{
  fsw_status_t status;
  /* We modify node, so make a copy */
  BTNodeDescriptor *node = first_node;
  fsw_u8 *buffer = NULL;

  if (!btree || !btree->node_size) {
    DBG("no node_size\n");
    return FSW_NOT_FOUND;
  }
  
  status = fsw_alloc (btree->node_size, &buffer);
  if (status) {
    if (buffer != NULL)
      fsw_free(buffer);
    return status;
  }
  
  for (;;) {
    fsw_u32 i;
    fsw_u32 count = be16_to_cpu (node->numRecords);
    fsw_u32 next_node;
    
//    DBG("first_rec=%d, count=%d, kind=%d\n", first_rec, count, node->kind);
    status = FSW_NOT_FOUND;
    // Iterate over all records in this node.
    i = first_rec;
    while (i < count) {
      int rv = callback (fsw_hfs_btree_rec (btree, node, i), param); //fsw_hfs_btree_visit_node
//      DBG("test record %d, status=%d\n", i, rv);
      if (rv == 1) {
        status = FSW_SUCCESS;
        goto done;
      } else if (rv == -1) {
//        DBG("tested record %d, status=%d\n", i, rv);
        status = FSW_NOT_FOUND; //it means the node has other owner
        goto done; //no need to test more
      }
      /* if callback returned 0 - continue */
//      status = FSW_SUCCESS;
      i++;
    }
    
    next_node = be32_to_cpu(node->fLink);
//    DBG("next_node=%d\n", next_node);
    
    if (!next_node) {
      status = FSW_NOT_FOUND;
//      DBG("FSW_NOT_FOUND at next_node\n");
      break;
    }

    if ((fsw_u32) fsw_hfs_read_file(btree->file, next_node * btree->node_size,
                                    btree->node_size, buffer) != btree->node_size) {
      status = FSW_VOLUME_CORRUPTED;
//      DBG("differ node size btree->node_size=%d\n", btree->node_size);
      break;
    }
    node = (BTNodeDescriptor *) buffer;
    first_rec = 0;
  }
done:
  if (buffer)
    fsw_free(buffer);
  
  return status;
}

static int
fsw_hfs_cmp_extkey(BTreeKey* key1, BTreeKey* key2)
{
    HFSPlusExtentKey* ekey1 = (HFSPlusExtentKey*)key1;
    HFSPlusExtentKey* ekey2 = (HFSPlusExtentKey*)key2;
    int result;

    /* First key is read from the FS data, second is in-memory in CPU endianess */
    result = be32_to_cpu(ekey1->fileID) - ekey2->fileID;

    if (result)
        return result;

    result = ekey1->forkType - ekey2->forkType;

    if (result)
        return result;

    result = be32_to_cpu(ekey1->startBlock) - ekey2->startBlock;
    return result;
}

static int
fsw_hfs_cmp_catkey (BTreeKey *key1, BTreeKey *key2)
{
  HFSPlusCatalogKey *ckey1 = (HFSPlusCatalogKey*)key1;
  HFSPlusCatalogKey *ckey2 = (HFSPlusCatalogKey*)key2;

  int      apos, bpos, lc;
  fsw_u16  ac, bc;
  fsw_u32  parentId1;
  int      key1Len;
  fsw_u16 *p1;
  fsw_u16 *p2;

  parentId1 = be32_to_cpu(ckey1->parentID);

  if (parentId1 > ckey2->parentID)
      return 1;
  if (parentId1 < ckey2->parentID)
      return -1;

  p1 = &ckey1->nodeName.unicode[0];
  p2 = &ckey2->nodeName.unicode[0];
  key1Len = be16_to_cpu (ckey1->nodeName.length);
  apos = bpos = 0;

  while(1) {
    /* get next valid character from ckey1 */
    for (lc = 0; lc == 0 && apos < key1Len; apos++) {
      ac = be16_to_cpu(p1[apos]);
      lc = ac;
    }
    ac = (fsw_u16)lc;

    /* get next valid character from ckey2 */
    for (lc = 0; lc == 0 && bpos < ckey2->nodeName.length; bpos++) {
      bc = p2[bpos];
      lc = bc;
    }
    bc = (fsw_u16)lc;

    if (ac != bc || (ac == 0  && bc == 0))
      return ac - bc;
  }
}

static int
fsw_hfs_cmpi_catkey (BTreeKey *key1, BTreeKey *key2)
{
  HFSPlusCatalogKey *ckey1 = (HFSPlusCatalogKey*)key1;
  HFSPlusCatalogKey *ckey2 = (HFSPlusCatalogKey*)key2;

  int      apos, bpos, lc = 0;
  fsw_u16  ac, bc;
  fsw_u32  parentId1;
  int      key1Len;
  int      key2Len;
  fsw_u16 *p1;
  fsw_u16 *p2;

  parentId1 = be32_to_cpu_ua((fsw_u32 *)(((UINT8 *)ckey1) + OFFSET_OF(HFSPlusCatalogKey, parentID))); // MSC warns about a possibly unaligned result of '&' (C4366)
//  if (hardlink) {
//    DBG("parents: %d <-> %d\n", parentId1, ckey2->parentID);
//  }
  if (parentId1 > ckey2->parentID) {
    return 1;
  } else if (parentId1 < ckey2->parentID) {
    return -1;
  } else {

    key1Len = be16_to_cpu (ckey1->nodeName.length);
    key2Len = ckey2->nodeName.length; //it is constructed so CPU endiness
//    if (hardlink) {
//      DBG("keylen: %d <-> %d\n", key1Len, key2Len);
//    }

    if (key1Len == 0 || key2Len == 0) {
      return key1Len - key2Len;
    }
    p1 = &ckey1->nodeName.unicode[0];
    p2 = &ckey2->nodeName.unicode[0];

/*    if (hardlink) {
      DBG("name:");
      for (apos = 0; apos < key1Len; apos++) {
        ac = be16_to_cpu(p1[apos]);
        DBG("%c", ac?ac:L'@');
      }
      DBG(":");
      for (apos = 0; apos < key2Len; apos++) {
        ac = p2[apos];
        DBG("%c", ac?ac:L'@');
      }
      DBG(":\n");
    }
*/
    apos = bpos = 0;

    while(1) {
      /* get next valid character from ckey1 */
      for (lc = 0; lc == 0 && apos < key1Len; apos++) {
//      for (lc = 0; apos < key1Len; apos++) {
        ac = be16_to_cpu(p1[apos]);
        lc = ac ? fsw_to_lower(ac) : 0xFFFF;
      }
      ac = (fsw_u16)lc;

      /* get next valid character from ckey2 */
      for (lc = 0; lc == 0 && bpos < key2Len; bpos++) {
//      for (lc = 0; bpos < key2Len; bpos++) {
        bc = p2[bpos];
        lc = bc ? fsw_to_lower(bc) : 0xFFFF;;
      }
      bc = (fsw_u16)lc;
//      if (hardlink) {
//        DBG("compare: %c:%c\n", ac?ac:L'@', bc?bc:L'@');
//      }

      if (ac != bc) {
        break;
      }
      if (bpos == key1Len) {
        return (key1Len - key2Len);
      }
    }
    if (ac == bc)
      return 0;
    else if (ac < bc)
      return -1;
    else
      return 1;
  }
}

//
// helper function to compare fileID with node
// key1 is catalog key, param contains FileID to find in "parent" field
//
static int
fsw_hfs_cmp_id (BTreeKey *key1, void *param)
{
  visitor_parameter_t *vp   = (visitor_parameter_t*)param;
  HFSPlusCatalogFile *crec1 = (HFSPlusCatalogFile*)((char*)key1 + be16_to_cpu (key1->length16));
  fsw_u32       fileID    = be32_to_cpu(crec1->fileID);
  fsw_u32       rec_type  = be16_to_cpu(crec1->recordType);
  if ((rec_type != kHFSPlusFileRecord) &&
      (rec_type != kHFSPlusFolderRecord)){
    DBG("bad record type? =%d fileID=%d\n", rec_type, fileID);
    return 2;
  }
  if (fileID != vp->parent) {
    return 0;
  }
  return 1;
}

/**
 * Retrieve file data mapping information. This function is called by the core when
 * fsw_shandle_read needs to know where on the disk the required piece of the file's
 * data can be found. The core makes sure that fsw_hfs_dnode_fill has been called
 * on the dnode before. Our task here is to get the physical disk block number for
 * the requested logical block number.
 */

static fsw_status_t fsw_hfs_get_extent(struct fsw_hfs_volume * vol,
                                       struct fsw_hfs_dnode  * dno,
                                       struct fsw_extent     * extent)
{
  fsw_status_t         status;
  fsw_u32              lbno;
  HFSPlusExtentRecord  *exts;
  BTNodeDescriptor     *node = NULL;
  struct HFSPlusExtentKey* key;
  struct HFSPlusExtentKey  overflowkey;
  fsw_u32                  ptr;
  fsw_u32                  phys_bno;
  
  extent->type = FSW_EXTENT_TYPE_PHYSBLOCK;
  extent->log_count = 1;
  lbno = extent->log_start;
  
  /* we only care about data forks atm, do we? */
  exts = &dno->extents;
  
  while (1) {
    if (fsw_hfs_find_block(exts, &lbno, &phys_bno)) {
      extent->phys_start = phys_bno + vol->emb_block_off;
      status = FSW_SUCCESS;
      break;
    }
    
    /* Find appropriate overflow record */
    overflowkey.forkType = 0;  //data fork
    overflowkey.fileID = dno->g.dnode_id;
    overflowkey.startBlock = extent->log_start - lbno;
    
    if (node != NULL) {
      fsw_free(node);
      node = NULL;
    }
    
    status = fsw_hfs_btree_search (&vol->extents_tree,
                                   (BTreeKey*)&overflowkey,
                                   fsw_hfs_cmp_extkey,
                                   &node, &ptr);
    if (status) {
      break;
    }
    
    key = (struct HFSPlusExtentKey *) fsw_hfs_btree_rec (&vol->extents_tree, node, ptr);
    exts = (HFSPlusExtentRecord*) (key + 1);
  }
  
  if (node != NULL)
    fsw_free(node);
  
  return status;
}

static fsw_status_t
create_hfs_dnode(struct fsw_hfs_dnode  *dno,
                 file_info_t           *file_info,
                 struct fsw_hfs_dnode **child_dno_out)
{
  fsw_status_t          status;
  struct fsw_hfs_dnode *baby = NULL;
  
  if (!file_info || !child_dno_out) {
    return FSW_NOT_FOUND;
  }
  status = fsw_dnode_create(dno, file_info->id, file_info->type,
                            file_info->name, &baby);
  if (status || !baby)
    return status;
  
  baby->g.size = file_info->size;
  baby->used_bytes = file_info->used;
  baby->ctime = file_info->ctime;
  baby->mtime = file_info->mtime;
  
  /* Fill-in extents info */
  if (file_info->type == FSW_DNODE_TYPE_FILE) {
    fsw_memcpy(baby->extents, &file_info->extents, sizeof(file_info->extents));
  }
  
  // Fill-in link file info 
  if (file_info->isfilelink) {
    baby->creator = file_info->creator;
    baby->crtype = file_info->crtype;
    baby->ilink = file_info->ilink;
    fsw_memcpy(baby->extents, &file_info->extents, sizeof file_info->extents);
  }
  
  *child_dno_out = baby;
  return FSW_SUCCESS;
}

/**
 * Lookup a directory's child dnode by name. This function is called on a directory
 * to retrieve the directory entry with the given name. A dnode is constructed for
 * this entry and returned. The core makes sure that fsw_hfs_dnode_fill has been called
 * and the dnode is actually a directory.
 */

static fsw_status_t fsw_hfs_dir_lookup(struct fsw_hfs_volume * vol,
                                       struct fsw_hfs_dnode  * dno,
                                       struct fsw_string     * lookup_name,
                                       struct fsw_hfs_dnode ** child_dno_out)
{
  fsw_status_t            status;
  HFSPlusCatalogKey       catkey;
  fsw_u32                 ptr;
//  fsw_u16                    rec_type;
  BTNodeDescriptor *      node = NULL;
  struct fsw_string       rec_name;
  int                     free_data = 0; //, i;
  HFSPlusCatalogKey*      file_key;
  file_info_t             file_info;
  
  fsw_memzero(&file_info, sizeof(file_info_t));
  file_info.name = &rec_name;

  catkey.parentID = dno->g.dnode_id;
  catkey.nodeName.length = (fsw_u16)lookup_name->len;
  
  // no need to allocate anything 
  if (lookup_name->type == FSW_STRING_TYPE_UTF16) {
    fsw_memcpy(catkey.nodeName.unicode, lookup_name->data, lookup_name->size);
//    rec_name = *lookup_name;
    fsw_memcpy(&rec_name, lookup_name, sizeof(struct fsw_string));
  } else {
    status = fsw_strdup_coerce(&rec_name, FSW_STRING_TYPE_UTF16, lookup_name);
    // nothing allocated so far
    if (status)
      goto done;
    free_data = 1;
    fsw_memcpy(catkey.nodeName.unicode, rec_name.data, rec_name.size);
  }
    
  catkey.keyLength = (fsw_u16)(6 + rec_name.size);
  status = fsw_hfs_btree_search (&vol->catalog_tree,
                                 (BTreeKey*)&catkey,
                                 vol->case_sensitive ?
                                 fsw_hfs_cmp_catkey : fsw_hfs_cmpi_catkey,
                                 &node, &ptr);
  if (status) {
//    DBG("fsw_hfs_btree_search dir lookup  status %a\n", fsw_errors[status]);
    goto done;
  }
  
  file_key = (HFSPlusCatalogKey *)fsw_hfs_btree_rec (&vol->catalog_tree, node, ptr);
  
  fill_fileinfo (vol, file_key, &file_info);
  status = create_hfs_dnode(dno, &file_info,  child_dno_out); //&tmp_dno_out); //
//  if (status) {
//    DBG("create_hfs_dnode  status %a\n", fsw_errors[status]);
//    goto done;
//  }
done:
  
  if (node != NULL)
    fsw_free(node);
  
  if (free_data)
    fsw_strfree(&rec_name);
  
  return status;
}

/**
 * Lookup a directory's child dnode by ID. This function is called on a directory
 * to retrieve the directory entry with the fileID. A dnode is constructed for
 * this entry and returned.
 */

static fsw_status_t fsw_hfs_dir_lookup_id(struct fsw_hfs_volume *vol,
                                          fsw_u32               lookup_id,
                                          file_info_t           *file_info)
{
  fsw_status_t            status;
  fsw_u8                  *buffer = NULL;
  fsw_u32                 ptr = 0;
  visitor_parameter_t     param;
  BTNodeDescriptor *      node = NULL; //root
  fsw_u32                 currnode = vol->catalog_tree.root_node;
  fsw_u32                 nodeSize = vol->catalog_tree.node_size;
//  DBG("...lookup for fileID=%d\n", lookup_id);
  status = fsw_alloc(nodeSize, &buffer);
  if (status != FSW_SUCCESS) {
    fsw_free(buffer);
    goto done;
  }
  node = (BTNodeDescriptor *) buffer;
  /* Read a node */
  if ((fsw_u32)fsw_hfs_read_file(vol->catalog_tree.file,
                                 MultU64x32(currnode, nodeSize),
                                 nodeSize, buffer) != nodeSize) {
    status = FSW_VOLUME_CORRUPTED;
//    DBG("differ node size while read file\n");
    return status;
  }
  
  fsw_memzero(&param, sizeof(visitor_parameter_t));
  param.parent = lookup_id;

  status = fsw_hfs_btree_iterate_node (&vol->catalog_tree,
                                       node,
                                       ptr,
                                       fsw_hfs_cmp_id,
                                       &param);

  if (status) {    
    goto done;
  }
  fsw_memcpy(file_info, &param.file_info, sizeof(file_info_t));
//  status = create_hfs_dnode(dno, &param.file_info,  child_dno_out); //&tmp_dno_out); //
//  if (status) {
//    DBG("create_hfs_dnode  status %a\n", fsw_errors[status]);
    //    goto done;
//  }
done:

  if (node != NULL)
    fsw_free(node);
//  DBG("fsw_hfs_dir_lookup_id return status %a\n", fsw_errors[status]);
  return status;
}

/**
 * Get the next directory entry when reading a directory. This function is called during
 * directory iteration to retrieve the next directory entry. A dnode is constructed for
 * the entry and returned. The core makes sure that fsw_hfs_dnode_fill has been called
 * and the dnode is actually a directory. The shandle provided by the caller is used to
 * record the position in the directory between calls.
 */

static fsw_status_t fsw_hfs_dir_read(struct fsw_hfs_volume *vol,
                                     struct fsw_hfs_dnode  *dno,
                                     struct fsw_shandle    *shand,
                                     struct fsw_hfs_dnode  **child_dno_out)
{
  BTNodeDescriptor          *node = NULL;
  fsw_u32                   ptr;
  HFSPlusCatalogKey         catkey;
  fsw_status_t              status;
  
  visitor_parameter_t       param;
  struct fsw_string         rec_name;
  
  catkey.parentID = dno->g.dnode_id;
  catkey.nodeName.length = 0;
  
  fsw_memzero(&param, sizeof(param));
  
  rec_name.type = FSW_STRING_TYPE_EMPTY;
  param.file_info.name = &rec_name;
  param.file_info.id = 0;
  // we are searching a node with name=0 and parentID?
  status = fsw_hfs_btree_search (&vol->catalog_tree,
                                 (BTreeKey*)&catkey,
                                 vol->case_sensitive ?
                                 fsw_hfs_cmp_catkey : fsw_hfs_cmpi_catkey,
                                 &node, &ptr);
  if (status) {
//    DBG("fsw_hfs_btree_search dir read  status %a\n", fsw_errors[status]);
    goto done;
  }
  
  /* Iterator updates shand state */
  param.vol = vol;
  param.shandle = shand;
  param.parent = dno->g.dnode_id;
  if (dno->ilink != 0) {
    param.parent = 0;
  }
  param.cur_pos = 0;
  status = fsw_hfs_btree_iterate_node (&vol->catalog_tree,
                                       node,
                                       ptr,
                                       fsw_hfs_btree_visit_node,
                                       &param);
  if (!status) {  
    status = create_hfs_dnode(dno, &param.file_info, child_dno_out);
//    if (status) {
//never      DBG("create_hfs_dnode  status %a\n", fsw_errors[status]);
//    }
  } else {
//    DBG("fsw_hfs_btree_iterate_node  status %a\n", fsw_errors[status]);
  }


done:
  if (node)
    fsw_free(node);
  fsw_strfree(&rec_name);
  
  return status;
}

static const char metaprefix[] = "/\0\0\0\0HFS+ Private Data/iNode0123456789";
/**
 * Get the target path of a  link. This function is called when a 
 * link needs to be resolved. The core makes sure that the fsw_hfs_dnode_fill has been
 * called on the dnode.
 *
 */
static fsw_status_t fsw_hfs_readlink(struct fsw_hfs_volume *vol,
                                     struct fsw_hfs_dnode *dno,
                                     struct fsw_string *link_target)
{
  /*
   * XXX: Hardlinks for directories -- it is alias to folder .
   * Hex dump visual inspection of Apple hfsplus{32,64}.efi
   * revealed no signs of directory hardlinks support.
   finder links, not properly documented
   kHFSAliasType    = 0x66647270,  // 'fdrp' - finder alias for folder
   kHFSAliasCreator = 0x4D414353   // 'MACS'
   kHFSAliasFile    = 0x616C6973   // 'alis' - finder alias for file - not implemented yet
   */
  fsw_u32 sz = 0;
//  int     i;
//  fsw_u16    ch;
  fsw_status_t status = FSW_UNSUPPORTED;

  if(dno->creator == kHFSPlusCreator && dno->crtype == kHardLinkFileType) {
#define MPRFSIZE (sizeof (metaprefix))
#define MPRFINUM (MPRFSIZE - 1 - 10)
    struct fsw_string *tmp_target;
    status = fsw_alloc(sizeof(struct fsw_string), &tmp_target);
    if (status) {
      DBG("can't alloc tmp_target\n");
    }

    tmp_target->type = FSW_STRING_TYPE_ISO88591;
    tmp_target->size = MPRFSIZE;
    DBG(" hfs readlink: size=%d, iLink=%d\n", tmp_target->size, dno->ilink);
    fsw_memdup ((void**)&tmp_target->data, (void*)&metaprefix[0], tmp_target->size);
    sz = (fsw_u32)AsciiSPrint(((char *)tmp_target->data) + MPRFINUM, 10, "%d", dno->ilink);
    tmp_target->len = MPRFINUM + sz;
//      DBG(" iNode name len=%d\n", tmp_target->len);
    status = fsw_strdup_coerce(link_target, vol->g.host_string_type, tmp_target);
    hardlink = 1;
/*
    for (i = 0; i < tmp_target->len; i++) {
      ch = ((fsw_u16 *) link_target->data)[i];
      DBG("%c", ch?ch:L'@');
    }
    DBG("\n");
*/
    fsw_strfree(tmp_target);
    fsw_free(tmp_target);
    return status;
#undef MPRFINUM
#undef MPRFSIZE
  } else if (dno->creator == kSymLinkCreator && dno->crtype == kSymLinkFileType) {
    hardlink = 1;
    return fsw_dnode_readlink_data(dno, link_target);
  } else if (dno->creator == kHFSAliasCreator && dno->crtype == kHFSAliasType) {
    CHAR16 inodename[48];
    sz = (fsw_u32)UnicodeSPrint(inodename, 48, L".HFS+ Private Directory Data/dir_%d", dno->ilink);
    link_target->type = FSW_STRING_TYPE_UTF16;
    link_target->size = 96;
    link_target->len = sz;
    fsw_memdup (&link_target->data, &inodename[0], link_target->size);
    hardlink = 1;
    return FSW_SUCCESS;
  }

  /* Unknown link type */

  return FSW_UNSUPPORTED;
}

// EOF
