/* ntfs.h - header for the NTFS filesystem */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2007,2009  Free Software Foundation, Inc.
 *
 *  GRUB is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  GRUB is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GRUB.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GRUB_NTFS_H
#define GRUB_NTFS_H	1

enum
  {
    GRUB_NTFS_FILE_MFT     =  0,
    GRUB_NTFS_FILE_MFTMIRR =  1,
    GRUB_NTFS_FILE_LOGFILE =  2,
    GRUB_NTFS_FILE_VOLUME  =  3,
    GRUB_NTFS_FILE_ATTRDEF =  4,
    GRUB_NTFS_FILE_ROOT    =  5,
    GRUB_NTFS_FILE_BITMAP  =  6,
    GRUB_NTFS_FILE_BOOT    =  7,
    GRUB_NTFS_FILE_BADCLUS =  8,
    GRUB_NTFS_FILE_QUOTA   =  9,
    GRUB_NTFS_FILE_UPCASE  = 10,
  };

enum
  {
    GRUB_NTFS_AT_STANDARD_INFORMATION = 0x10,
    GRUB_NTFS_AT_ATTRIBUTE_LIST       = 0x20,
    GRUB_NTFS_AT_FILENAME             = 0x30,
    GRUB_NTFS_AT_OBJECT_ID            = 0x40,
    GRUB_NTFS_AT_SECURITY_DESCRIPTOR  = 0x50,
    GRUB_NTFS_AT_VOLUME_NAME          = 0x60,
    GRUB_NTFS_AT_VOLUME_INFORMATION   = 0x70,
    GRUB_NTFS_AT_DATA                 = 0x80,
    GRUB_NTFS_AT_INDEX_ROOT           = 0x90,
    GRUB_NTFS_AT_INDEX_ALLOCATION     = 0xA0,
    GRUB_NTFS_AT_BITMAP               = 0xB0,
    GRUB_NTFS_AT_SYMLINK              = 0xC0,
    GRUB_NTFS_AT_EA_INFORMATION	      = 0xD0,
    GRUB_NTFS_AT_EA                   = 0xE0,
  };

enum
  {
    GRUB_NTFS_ATTR_READ_ONLY   = 0x1,
    GRUB_NTFS_ATTR_HIDDEN      = 0x2,
    GRUB_NTFS_ATTR_SYSTEM      = 0x4,
    GRUB_NTFS_ATTR_ARCHIVE     = 0x20,
    GRUB_NTFS_ATTR_DEVICE      = 0x40,
    GRUB_NTFS_ATTR_NORMAL      = 0x80,
    GRUB_NTFS_ATTR_TEMPORARY   = 0x100,
    GRUB_NTFS_ATTR_SPARSE      = 0x200,
    GRUB_NTFS_ATTR_REPARSE     = 0x400,
    GRUB_NTFS_ATTR_COMPRESSED  = 0x800,
    GRUB_NTFS_ATTR_OFFLINE     = 0x1000,
    GRUB_NTFS_ATTR_NOT_INDEXED = 0x2000,
    GRUB_NTFS_ATTR_ENCRYPTED   = 0x4000,
    GRUB_NTFS_ATTR_DIRECTORY   = 0x10000000,
    GRUB_NTFS_ATTR_INDEX_VIEW  = 0x20000000
  };

enum
  {
    GRUB_NTFS_FLAG_COMPRESSED		= 1,
    GRUB_NTFS_FLAG_ENCRYPTED		= 0x4000,
    GRUB_NTFS_FLAG_SPARSE		= 0x8000
  };

#define GRUB_NTFS_BLK_SHR		GRUB_DISK_SECTOR_BITS

#define GRUB_NTFS_MAX_MFT		(4096 >> GRUB_NTFS_BLK_SHR)
#define GRUB_NTFS_MAX_IDX		(16384 >> GRUB_NTFS_BLK_SHR)

#define GRUB_NTFS_COM_LEN		4096
#define GRUB_NTFS_COM_LOG_LEN	12
#define GRUB_NTFS_COM_SEC		(GRUB_NTFS_COM_LEN >> GRUB_NTFS_BLK_SHR)
#define GRUB_NTFS_LOG_COM_SEC		(GRUB_NTFS_COM_LOG_LEN - GRUB_NTFS_BLK_SHR)

enum
  {
    GRUB_NTFS_AF_ALST		= 1,
    GRUB_NTFS_AF_MMFT		= 2,
    GRUB_NTFS_AF_GPOS		= 4,
  };

enum
  {
    GRUB_NTFS_RF_BLNK		= 1
  };

struct grub_ntfs_bpb
{
  grub_uint8_t jmp_boot[3];
  grub_uint8_t oem_name[8];
  grub_uint16_t bytes_per_sector;
  grub_uint8_t sectors_per_cluster;
  grub_uint8_t reserved_1[7];
  grub_uint8_t media;
  grub_uint16_t reserved_2;
  grub_uint16_t sectors_per_track;
  grub_uint16_t num_heads;
  grub_uint32_t num_hidden_sectors;
  grub_uint32_t reserved_3;
  grub_uint8_t bios_drive;
  grub_uint8_t reserved_4[3];
  grub_uint64_t num_total_sectors;
  grub_uint64_t mft_lcn;
  grub_uint64_t mft_mirr_lcn;
  grub_int8_t clusters_per_mft;
  grub_int8_t reserved_5[3];
  grub_int8_t clusters_per_index;
  grub_int8_t reserved_6[3];
  grub_uint64_t num_serial;
  grub_uint32_t checksum;
} GRUB_PACKED;

struct grub_ntfs_attr
{
  int flags;
  grub_uint8_t *emft_buf, *edat_buf;
  grub_uint8_t *attr_cur, *attr_nxt, *attr_end;
  grub_uint32_t save_pos;
  grub_uint8_t *sbuf;
  struct grub_ntfs_file *mft;
};

struct grub_ntfs_file
{
  struct grub_ntfs_data *data;
  grub_uint8_t *buf;
  grub_uint64_t size;
  grub_uint64_t mtime;
  grub_uint64_t ino;
  int inode_read;
  struct grub_ntfs_attr attr;
};

struct grub_ntfs_data
{
  struct grub_ntfs_file cmft;
  struct grub_ntfs_file mmft;
  grub_disk_t disk;
  grub_uint64_t mft_size;
  grub_uint64_t idx_size;
  int log_spc;
  grub_uint64_t mft_start;
  grub_uint64_t uuid;
};

struct grub_ntfs_comp_table_element
{
  grub_uint32_t next_vcn;
  grub_uint32_t next_lcn;
};

struct grub_ntfs_comp
{
  grub_disk_t disk;
  int comp_head, comp_tail;
  struct grub_ntfs_comp_table_element comp_table[16];
  grub_uint32_t cbuf_ofs, cbuf_vcn;
  int log_spc;
  grub_uint8_t *cbuf;
};

struct grub_ntfs_rlst
{
  int flags;
  grub_disk_addr_t target_vcn, curr_vcn, next_vcn, curr_lcn;
  grub_uint8_t *cur_run;
  struct grub_ntfs_attr *attr;
  struct grub_ntfs_comp comp;
  void *file;
};

grub_err_t grub_ntfscomp_func (grub_uint8_t *dest,
					    grub_disk_addr_t ofs,
					    grub_size_t len,
					    struct grub_ntfs_rlst * ctx);

grub_err_t grub_ntfs_read_run_list (struct grub_ntfs_rlst *ctx);

#endif /* ! GRUB_NTFS_H */
