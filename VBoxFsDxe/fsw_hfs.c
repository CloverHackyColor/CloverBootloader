/* $Id: fsw_hfs.c 33540 2010-10-28 09:27:05Z vboxsync $ */
/** @file
 * fsw_hfs.c - HFS file system driver code, see
 *
 *   http://developer.apple.com/technotes/tn/tn1150.html
 *
 * Current limitations:
 *  - Doesn't support permissions
 *  - Complete Unicode case-insensitiveness disabled (large tables)
 *  - No links
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
// improved by nms42: VolumeName, links

#include "fsw_hfs.h"

#ifdef HOST_POSIX
#define DPRINT(x) printf(x)
#define DPRINT2(x,y) printf(x,y)
#define BP(msg)    do { printf("ERROR: %s", msg); asm("int3"); } while (0)
#else
#define CONCAT(x,y) x##y
#define DPRINT(x) Print(CONCAT(L,x))
#define DPRINT2(x,y) Print(CONCAT(L,x), y)
#define BP(msg) DPRINT(msg)
#endif

// functions
#if 0
void dump_str(fsw_u16* p, fsw_u32 len, int swap)
{
    int i;

    for (i=0; i<len; i++)
    {
        fprintf(stderr, "%c", swap ? be16_to_cpu(p[i]) : p[i]);
    }
    fprintf(stderr, "\n");
}
#endif

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
    fsw_hfs_readlink,   // return FSW_UNSUPPORTED;
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
    status = fsw_block_get(dno->g.vol, phys_bno, 0, (void **)&buffer);
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

    while (len > 0)
    {
        fsw_u32 off = (fsw_u32)(pos & block_size_mask);
        fsw_s32 next_len = len;

        log_bno = (fsw_u32)RShiftU64(pos, block_size_bits);

        if (   next_len >= 0
            && (fsw_u32)next_len >  block_size)
            next_len = block_size;
        status = fsw_hfs_read_block(dno, log_bno, off, next_len, buf);
        if (status)
            return -1;
        buf  += next_len;
        pos  += next_len;
        len  -= next_len;
        read += next_len;
    }

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

//    BP("BUG\n");
    return 0;
}

/**
 * Mount an HFS+ volume. Reads the superblock and constructs the
 * root directory dnode.
 */
//algo from Chameleon
/*
void
HFSGetDescription(CICell ih, char *str, long strMaxLen)
{
  
  UInt16 nodeSize;
  UInt32 firstLeafNode;
  long long dirIndex;
  char *name;
  long flags, time;
  
  if (HFSInitPartition(ih) == -1)  { return; }
  
  // Fill some crucial data structures by side effect. 
  dirIndex = 0;
  HFSGetDirEntry(ih, "/", &dirIndex, &name, &flags, &time, 0, 0);
  
  // Now we can loook up the volume name node. 
  nodeSize = be16_to_cpu(gBTHeaders[kBTreeCatalog]->nodeSize);
  firstLeafNode = SWAP_BE32(gBTHeaders[kBTreeCatalog]->firstLeafNode);
  
  dirIndex = (long long) firstLeafNode * nodeSize;
  
  GetCatalogEntry(&dirIndex, &name, &flags, &time, 0, 0);
  
  strncpy(str, name, strMaxLen);
  str[strMaxLen] = '\0';
}
*/


static fsw_status_t fsw_hfs_volume_mount(struct fsw_hfs_volume *vol)
{
    fsw_status_t           status, rv;
    void                  *buffer = NULL;
    HFSPlusVolumeHeader   *voldesc;
    fsw_u32                blockno;
    struct fsw_string      s;
  HFSMasterDirectoryBlock* mdb;
  UINTN i;

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
  //            DPRINT("found HFS+\n");
                vol->hfs_kind = FSW_HFS_PLUS;
            }
        } else if (signature == kHFSSigWord) {// 'BD'
//            HFSMasterDirectoryBlock* mdb = (HFSMasterDirectoryBlock*)buffer;
//VolumeName = mdb->drVN 28bytes
            if (be16_to_cpu(mdb->drEmbedSigWord) == kHFSPlusSigWord) {
//                DPRINT("found HFS+ inside HFS, untested\n");
                vol->hfs_kind = FSW_HFS_PLUS_EMB;
                vol->emb_block_off = be32_to_cpu(mdb->drEmbedExtent.startBlock);
                fsw_block_release (vol, blockno, buffer);
                blockno += vol->emb_block_off;
                /* retry */
                continue;
            } else {
//                DPRINT("found plain HFS, unsupported\n");
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

        fsw_block_release(vol, blockno, buffer);
        buffer = NULL;
        voldesc = NULL;
        fsw_set_blocksize(vol, block_size, block_size);

        /* get volume name */
      for (i = kHFSMaxVolumeNameChars; i > 0; i--)
        if (mdb->drVN[i-1] != ' ')
          break;
      
        s.type = FSW_STRING_TYPE_ISO88591;
        s.size = s.len = 0;
        s.data = NULL; //&mdb->drVN; //"HFS+ volume";
      
       //fsw_status_t fsw_strdup_coerce(struct fsw_string *dest, int type, struct fsw_string *src)
        status = fsw_strdup_coerce(&vol->g.label, vol->g.host_string_type, &s);
        CHECK(status);

        /* Setup catalog dnode */
        status = fsw_dnode_create_root(vol, kHFSCatalogFileID, &vol->catalog_tree.file);
        CHECK(status);
      //Slice - why copy structure?
        fsw_memcpy (vol->catalog_tree.file->extents,
                    vol->primary_voldesc->catalogFile.extents,
                    sizeof vol->catalog_tree.file->extents); 

        vol->catalog_tree.file->g.size =
                be64_to_cpu(vol->primary_voldesc->catalogFile.logicalSize);

        /* Setup extents overflow file */
        /*status = */fsw_dnode_create_root(vol, kHFSExtentsFileID, &vol->extents_tree.file);
       fsw_memcpy (vol->extents_tree.file->extents,
                    vol->primary_voldesc->extentsFile.extents,
                    sizeof vol->extents_tree.file->extents); 
        vol->extents_tree.file->g.size =
                be64_to_cpu(vol->primary_voldesc->extentsFile.logicalSize);

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
			if (btnd->kind == kBTLeafNode &&
				be32_to_cpu (ck->parentID) == kHFSRootParentID) {
				struct fsw_string vn;

				vn.type = FSW_STRING_TYPE_UTF16_BE;
				vn.len = be16_to_cpu (ck->nodeName.length);
				vn.size = vn.len * sizeof (fsw_u16);
				vn.data = ck->nodeName.unicode;
				fsw_strfree (&vol->g.label);
				status = fsw_strdup_coerce(&vol->g.label, vol->g.host_string_type, &vn);
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
            break;
        }

        vol->extents_tree.root_node = be32_to_cpu (tree_header.rootNode);
        vol->extents_tree.node_size = be16_to_cpu (tree_header.nodeSize);

        rv = FSW_SUCCESS;
    } while (0);

#undef CHECK

    if (buffer != NULL) {
        fsw_block_release(vol, blockno, buffer);
    }

    return rv;
}
//Here is a method to obtain Volume label from Apple
//how to implement it?
/*
UInt16 nodeSize;
UInt32 firstLeafNode;
long long dirIndex;
char *name;
long flags, time;
 char              *nodeBuf, *testKey, *entry;


if (HFSInitPartition(ih) == -1)  { return; }

// Fill some crucial data structures by side effect. 
dirIndex = 0;
HFSGetDirEntry(ih, "/", &dirIndex, &name, &flags, &time, 0, 0);

// Now we can loook up the volume name node.
nodeSize = SWAP_BE16(gBTHeaders[kBTreeCatalog]->nodeSize);
firstLeafNode = SWAP_BE32(gBTHeaders[kBTreeCatalog]->firstLeafNode);

dirIndex = (long long) firstLeafNode * nodeSize;
 index   = (long) (*dirIndex % nodeSize); == 0
 curNode = (long) (*dirIndex / nodeSize); == firstLeafNode

//GetCatalogEntry(&dirIndex, &name, &flags, &time, 0, 0);
 // Read the BTree node and get the record for index.
 ReadExtent(extent, extentSize, kHFSCatalogFileID,
 (long long) curNode * nodeSize, nodeSize, nodeBuf, 1);
 GetBTreeRecord(index, nodeBuf, nodeSize, &testKey, &entry);
 
 utf_encodestr(((HFSPlusCatalogKey *)testKey)->nodeName.unicode,
 SWAP_BE16(((HFSPlusCatalogKey *)testKey)->nodeName.length),
 (u_int8_t *)gTempStr, 256, OSBigEndian);
 
 *name = gTempStr; 

strncpy(str, name, strMaxLen);
str[strMaxLen] = '\0';
*/

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
  
  for (i = 0; i < 8; i++) {
    fsw_u32 start = be32_to_cpu ((*exts)[i].startBlock);
    fsw_u32 count = be32_to_cpu ((*exts)[i].blockCount);
    
    if (cur_lbno < count) {
      *pbno = start + cur_lbno;
      return 1;
    }
    cur_lbno -= count;
  }
  
  *lbno = cur_lbno;
  return 0;
}

/* Find record offset, numbering starts from the end */
static fsw_u32
fsw_hfs_btree_recoffset (struct fsw_hfs_btree *btree,
                         BTNodeDescriptor     *node,
                         fsw_u32              index)
{
  fsw_u8 *cnode = (fsw_u8 *)node;
  fsw_u16 *recptr;
  recptr = (fsw_u16 *)(cnode+btree->node_size - index * 2 - 2);
  return be16_to_cpu(*recptr);
}

/* Pointer to the key inside node */
static BTreeKey *
fsw_hfs_btree_rec (struct fsw_hfs_btree   *btree,
                   BTNodeDescriptor       *node,
                   fsw_u32                index)
{
  fsw_u8 *cnode = (fsw_u8 *)node;
  fsw_u32 offset;
  offset = fsw_hfs_btree_recoffset(btree, node, index);
  return (BTreeKey *)(cnode + offset);
}

#ifdef HFS_BINARY_SEARCH
static fsw_u32
fsw_hfs_btree_next_node (
  BTreeKey *currkey
)
{
        fsw_u32 *pointer;

        pointer = (fsw_u32 *) ((char *) currkey + be16_to_cpu (currkey->length16) + 2);
        return be32_to_cpu (*pointer);
}
#endif

static fsw_status_t
fsw_hfs_btree_search (struct fsw_hfs_btree *btree,
                      BTreeKey             *key,
                      int (*compare_keys) (BTreeKey *key1, BTreeKey *key2),
                      BTNodeDescriptor    **result,
                      fsw_u32              *key_offset)
{
  BTNodeDescriptor *node;
  fsw_u32 currnode;
  fsw_u32 rec;
  fsw_status_t status;
  fsw_u8 *buffer = NULL;
  int match;
  fsw_u32 count;
  int cmp = 0;
  fsw_u32 *pointer;
  
  currnode = btree->root_node;
  status = fsw_alloc(btree->node_size, &buffer);
  if (status != FSW_SUCCESS) {
    fsw_free(buffer);
    return status;
  }
  node = (BTNodeDescriptor*)buffer;
  
  status = FSW_NOT_FOUND;
  while (1) {
    match = 0;
    /* Read a node.  */
    if (fsw_hfs_read_file (btree->file,
                           (fsw_u64)currnode * btree->node_size,
                           btree->node_size, buffer) <= 0) {
      status = FSW_VOLUME_CORRUPTED;
      break;
    }
    
    if (be16_to_cpu(*(fsw_u16*)(buffer + btree->node_size - 2)) != sizeof(BTNodeDescriptor)) {
      BP("corrupted node\n");
    }
    count = be16_to_cpu (node->numRecords);
    
#ifndef HFS_BINARY_SEARCH
    for (rec = 0; rec < count; rec++) {
      BTreeKey *currkey;
      
      currkey = fsw_hfs_btree_rec (btree, node, rec);
      cmp = compare_keys (currkey, key);
      //fprintf(stderr, "rec=%d cmp=%d kind=%d \n", rec, cmp, node->kind);
      
      /* Leaf node. */
      if (node->kind == kBTLeafNode) {
        if (cmp == 0) {
          /* Found!  */
          *result = node;
          *key_offset = rec;
          
          status = FSW_SUCCESS;
          //                goto done;
          break;
        }
      } else if (node->kind == kBTIndexNode) {                
        if (cmp > 0)
          break;        
        pointer = (fsw_u32 *) ((CHAR8 *)currkey
                               + be16_to_cpu (currkey->length16)
                               + 2);
        currnode = be32_to_cpu (*pointer);
        match = 1;
      }
    }
    if (status == FSW_SUCCESS) {
      break;
    }
    if (node->kind == kBTLeafNode && (cmp < 0) && node->fLink) {
      currnode = be32_to_cpu(node->fLink);
      //          goto readnode; //continue
    } else if (!match) {
      status = FSW_NOT_FOUND;
      break;
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
     
      cmp = 0 - compare_keys (currkey, key);
#if 0
      FSW_MSG_DEBUGV ((FSW_MSGSTR (__FUNCTION__ ": currnode %d lower/recnum/upper %d/%d/%d (%d) cmp=%d kind=%d\n"),
            currnode, lower, recnum, upper, count, cmp, node->kind));
#endif
      if (cmp < 0)
        upper = recnum - 1;
      if (cmp > 0)
        lower = recnum + 1;
     if (cmp == 0) {
     // Found!  
     *result = node;
        *key_offset = recnum;
     
        return FSW_SUCCESS;
     }
     }

     if (cmp < 0)
     currkey = fsw_hfs_btree_rec (btree, node, upper);
     
    if (node->kind == kBTIndexNode && currkey != NULL) {
      currnode = fsw_hfs_btree_next_node (currkey);
#if 0
      FSW_MSG_DEBUGV ((FSW_MSGSTR (__FUNCTION__ ": candidate for the next currnode is %d\n"), currnode));
#endif
    }
    else {
     status = FSW_NOT_FOUND;
     break;
    }
#endif
  }
  //  done:
  if (buffer != NULL && status != FSW_SUCCESS)
    fsw_free(buffer);
  
  return status;
}

typedef struct {
  fsw_u32             id;
  fsw_u32             type;
  fsw_u32             creator;
  fsw_u32             crtype;
  fsw_u32             ilink;
  struct fsw_string   *name;
  fsw_u64             size;
  fsw_u64             used;
  fsw_u32             ctime;
  fsw_u32             mtime;
  HFSPlusExtentRecord extents;
} file_info_t;

static void
fill_fileinfo (
  struct fsw_hfs_volume* vol,
  HFSPlusCatalogKey* key,
  file_info_t* finfo
)
{
  fsw_u8* base;
  fsw_u16 rec_type;

  /* for plain HFS "-(keySize & 1)" would be needed */
  base = (fsw_u8 *) key + be16_to_cpu (key->keyLength) + 2;
  rec_type = be16_to_cpu (*(fsw_u16 *) base);

    /** @todo: read additional info */
  switch (rec_type) {
  case kHFSPlusFolderRecord:
    {
      HFSPlusCatalogFolder *info = (HFSPlusCatalogFolder *) base;

      finfo->id = be32_to_cpu (info->folderID);
      finfo->type = FSW_DNODE_TYPE_DIR;
      /* @todo: return number of elements, maybe use smth else */
      finfo->size = be32_to_cpu (info->valence);
      finfo->used = be32_to_cpu (info->valence);
      finfo->ctime = be32_to_cpu (info->createDate);
      finfo->mtime = be32_to_cpu (info->contentModDate);
      break;
    }
  case kHFSPlusFileRecord:
    {
      HFSPlusCatalogFile *info = (HFSPlusCatalogFile *) base;

      finfo->id = be32_to_cpu (info->fileID);

      finfo->creator = be32_to_cpu (info->userInfo.fdCreator);
      finfo->crtype = be32_to_cpu (info->userInfo.fdType);

      /* Is the file any kind of link? */
      if ((finfo->creator == kSymLinkCreator && finfo->crtype == kSymLinkFileType) ||
          (finfo->creator == kHFSPlusCreator && finfo->crtype == kHardLinkFileType)) {
        finfo->type = FSW_DNODE_TYPE_SYMLINK;
        finfo->ilink = be32_to_cpu (info->bsdInfo.special.iNodeNum);
      } else {
        finfo->type = FSW_DNODE_TYPE_FILE;
      }

      finfo->size = be64_to_cpu (info->dataFork.logicalSize);
      finfo->used =
        LShiftU64 (be32_to_cpu (info->dataFork.totalBlocks),
                   vol->block_size_shift);
      finfo->ctime = be32_to_cpu (info->createDate);
      finfo->mtime = be32_to_cpu (info->contentModDate);
      fsw_memcpy (&finfo->extents, &info->dataFork.extents,
                  sizeof finfo->extents);
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

static int
fsw_hfs_btree_visit_node(BTreeKey *record, void *param)
{
  visitor_parameter_t *vp = (visitor_parameter_t*)param;
  fsw_u8 *base = (fsw_u8*)record->rawData + be16_to_cpu(record->length16) + 2;
  fsw_u16 rec_type =  be16_to_cpu(*(fsw_u16*)base);
  struct HFSPlusCatalogKey *cat_key = (HFSPlusCatalogKey*)record;
  fsw_u16   name_len;
  fsw_u16   *name_ptr;
  fsw_u32   i;
  struct fsw_string *file_name;
  
  if (be32_to_cpu(cat_key->parentID) != vp->parent)
    return -1;
  
  /* not smth we care about */
  if (vp->shandle->pos != vp->cur_pos++)
    return 0;
  
  fill_fileinfo (vp->vol, cat_key, &vp->file_info);

  switch (rec_type) {
    case kHFSPlusFolderThreadRecord:
    case kHFSPlusFileThreadRecord:
    {
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
  fsw_u32 i;
  fsw_u32 count;
  fsw_u32 next_node;
  INTN rv;
  
  status = fsw_alloc(btree->node_size, &buffer);
  if (status)
    return status;
  
  while (1) {
    count =  be16_to_cpu(node->numRecords);
    
    /* Iterate over all records in this node.  */
    for (i = first_rec; i < count; i++) {
      rv = callback(fsw_hfs_btree_rec (btree, node, i), param);
      switch (rv) {
        case 1:
          status = FSW_SUCCESS;
          goto done;
        case -1:
          status = FSW_NOT_FOUND;
          goto done;
      }
      /* if callback returned 0 - continue */
    }
    
    next_node = be32_to_cpu(node->fLink);
    
    if (!next_node) {
      status = FSW_NOT_FOUND;
      break;
    }
    
    if (fsw_hfs_read_file (btree->file,
                           next_node * btree->node_size,
                           btree->node_size, buffer) <= 0) {
      status = FSW_VOLUME_CORRUPTED;
      return 1;
    }
    
    node = (BTNodeDescriptor*)buffer;
    first_rec = 0;
  }
done:
  if (buffer)
    fsw_free(buffer);
  
  return status;
}

#if 0
void deb(fsw_u16* p, int len, int swap)
{
    int i;
    for (i=0; i<len; i++)
    {
      printf("%c", swap ?  be16_to_cpu(p[i]) : p[i]);
    }
    printf("\n");
}
#endif

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

  key1Len = be16_to_cpu (ckey1->nodeName.length);

  if (key1Len == 0 && ckey2->nodeName.length == 0)
      return 0;

  p1 = &ckey1->nodeName.unicode[0];
  p2 = &ckey2->nodeName.unicode[0];

  apos = bpos = 0;

  while(1) {
    /* get next valid character from ckey1 */
    for (lc = 0; lc == 0 && apos < key1Len; apos++) {
      ac = be16_to_cpu(p1[apos]);
      lc = ac ? fsw_to_lower(ac) : 0;
    }
    ac = (fsw_u16)lc;

    /* get next valid character from ckey2 */
    for (lc = 0; lc == 0 && bpos < ckey2->nodeName.length; bpos++) {
      bc = p2[bpos];
      lc = bc ? fsw_to_lower(bc) : 0;
    }
    bc = (fsw_u16)lc;

    if (ac != bc || (ac == 0  && bc == 0))
      return ac - bc;
  }
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
    if (status)
      break;
    
    key = (struct HFSPlusExtentKey *)
    fsw_hfs_btree_rec (&vol->extents_tree, node, ptr);
    exts = (HFSPlusExtentRecord*) (key + 1);
  }
  
  if (node != NULL)
    fsw_free(node);
  
  return status;
}
/*
static const fsw_u16* g_blacklist[] =
{
    //L"AppleIntelCPUPowerManagement.kext",
    NULL
};
*/

//#define HFS_FILE_INJECTION

#ifdef HFS_FILE_INJECTION
static struct
{
  const fsw_u16* path;
  const fsw_u16* name;
} g_injectList[] =
{
  {
    L"/System/Library/Extensions",
    L"ApplePS2Controller.kext"
  },
  {
    NULL,
    NULL
  }
};
#endif

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
  if (file_info->type == FSW_DNODE_TYPE_SYMLINK) {
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
  fsw_status_t               status;
  struct HFSPlusCatalogKey   catkey;
  fsw_u32                    ptr;
//  fsw_u16                    rec_type;
  BTNodeDescriptor *         node = NULL;
  struct fsw_string          rec_name;
  int                        free_data = 0; //, i;
  HFSPlusCatalogKey*         file_key;
  file_info_t                file_info;
//  fsw_u8*                    base;
  
  
  fsw_memzero(&file_info, sizeof file_info);
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
  
  /* Dirty hack: blacklisting of certain files on FS driver level */
  /* 
 for (i = 0; g_blacklist[i]; i++) {
   if (fsw_memeq(g_blacklist[i], catkey.nodeName.unicode, catkey.nodeName.length*2)) {
     DPRINT2("Blacklisted %s\n", g_blacklist[i]);
     status = FSW_NOT_FOUND;
     goto done;
   }
 }
   */
#ifdef HFS_FILE_INJECTION
  if (fsw_hfs_inject(vol,
                     dno,
                     catkey.nodeName.unicode,
                     catkey.nodeName.length,
                     &file_info))
  {
    status = FSW_SUCCESS;
    goto create;
  }
#endif
  
  catkey.keyLength = (fsw_u16)(6 + rec_name.size);
//  catkey.keyLength = (fsw_u16)(6 + lookup_name->size);
  status = fsw_hfs_btree_search (&vol->catalog_tree,
                                 (BTreeKey*)&catkey,
                                 vol->case_sensitive ?
                                 fsw_hfs_cmp_catkey : fsw_hfs_cmpi_catkey,
                                 &node, &ptr);
  if (status)
    goto done;
  
  file_key = (HFSPlusCatalogKey *)fsw_hfs_btree_rec (&vol->catalog_tree, node, ptr);
  
  fill_fileinfo (vol, file_key, &file_info);

#ifdef HFS_FILE_INJECTION
create:
#endif
  status = create_hfs_dnode(dno, &file_info, child_dno_out);
  if (status)
    goto done;
  
done:
  
  if (node != NULL)
    fsw_free(node);
  
  if (free_data)
    fsw_strfree(&rec_name);
  
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
  fsw_status_t               status;
  struct HFSPlusCatalogKey   catkey;
  fsw_u32                    ptr;
  BTNodeDescriptor           *node = NULL;
  
  visitor_parameter_t        param;
  struct fsw_string          rec_name;
  
  catkey.parentID = dno->g.dnode_id;
  catkey.nodeName.length = 0;
  
//  fsw_memzero(&param, sizeof(param));
  
  rec_name.type = FSW_STRING_TYPE_EMPTY;
  param.file_info.name = &rec_name;
  param.file_info.id = 0;
  
  status = fsw_hfs_btree_search (&vol->catalog_tree,
                                 (BTreeKey*)&catkey,
                                 vol->case_sensitive ?
                                 fsw_hfs_cmp_catkey : fsw_hfs_cmpi_catkey,
                                 &node, &ptr);
  if (status)
    goto done;
  
  /* Iterator updates shand state */
  param.vol = vol;
  param.shandle = shand;
  param.parent = dno->g.dnode_id;
  param.cur_pos = 0;
  status = fsw_hfs_btree_iterate_node (&vol->catalog_tree,
                                       node,
                                       ptr,
                                       fsw_hfs_btree_visit_node,
                                       &param);
  if (!status) {  
    status = create_hfs_dnode(dno, &param.file_info, child_dno_out);
  }
  
done:
  if (node)
    fsw_free(node);
  fsw_strfree(&rec_name);
  
  return status;
}

/**
 * Get the target path of a symbolic link. This function is called when a symbolic
 * link needs to be resolved. The core makes sure that the fsw_hfs_dnode_fill has been
 * called on the dnode and that it really is a symlink.
 *
 */
static fsw_status_t fsw_hfs_readlink(struct fsw_hfs_volume *vol,
                                     struct fsw_hfs_dnode *dno,
                                     struct fsw_string *link_target)
{
  /*
   * XXX: Hardlinks for directories -- not yet.
   * Hex dump visual inspection of Apple hfsplus{32,64}.efi
   * revealed no signs of directory hardlinks support. Manana ;-)
   */

  if(dno->creator == kHFSPlusCreator && dno->crtype == kHardLinkFileType) {
#define MPRFSIZE (sizeof (metaprefix))
#define MPRFINUM (MPRFSIZE - 1 - 10)
    static fsw_u8 metaprefix[] = "/\0\0\0\0HFS+ Private Data/iNode0123456789";
    fsw_u32 sz = 0;

    link_target->type = FSW_STRING_TYPE_ISO88591;
    link_target->size = MPRFSIZE;
    fsw_memdup (&link_target->data, metaprefix, link_target->size);
    //AsciiSPrint
//    sz = (UINT32)AsciiSPrint(inumstart, 10 + 1, "%d", dno->ilink);
    sz = (UINT32)AsciiSPrint(((char *) link_target->data) + MPRFINUM, 10, "%d", dno->ilink);

    link_target->len = MPRFINUM + sz;
    return FSW_SUCCESS;
#undef MPRFINUM
#undef MPRFSIZE
  } else if (dno->creator == kSymLinkCreator && dno->crtype == kSymLinkFileType) {
    return fsw_dnode_readlink_data(dno, link_target);
  }
  /* Unknown link type */

  return FSW_UNSUPPORTED;
}

// EOF
