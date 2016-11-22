/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 1999,2000,2001,2002,2004,2007  Free Software Foundation, Inc.
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

#ifndef GRUB_BSDLABEL_PARTITION_HEADER
#define GRUB_BSDLABEL_PARTITION_HEADER	1

/* Constants for BSD disk label.  */
#define GRUB_PC_PARTITION_BSD_LABEL_SECTOR	1
#define GRUB_PC_PARTITION_BSD_LABEL_MAGIC	0x82564557

/* BSD partition types.  */
enum
  {
    GRUB_PC_PARTITION_BSD_TYPE_UNUSED  =  0,
    GRUB_PC_PARTITION_BSD_TYPE_SWAP    =  1,
    GRUB_PC_PARTITION_BSD_TYPE_V6      =  2,
    GRUB_PC_PARTITION_BSD_TYPE_V7      =  3,
    GRUB_PC_PARTITION_BSD_TYPE_SYSV    =  4,
    GRUB_PC_PARTITION_BSD_TYPE_V71K    =  5,
    GRUB_PC_PARTITION_BSD_TYPE_V8      =  6,
    GRUB_PC_PARTITION_BSD_TYPE_BSDFFS  =  7,
    GRUB_PC_PARTITION_BSD_TYPE_MSDOS   =  8,
    GRUB_PC_PARTITION_BSD_TYPE_BSDLFS  =  9,
    GRUB_PC_PARTITION_BSD_TYPE_OTHER   = 10,
    GRUB_PC_PARTITION_BSD_TYPE_HPFS    = 11,
    GRUB_PC_PARTITION_BSD_TYPE_ISO9660 = 12,
    GRUB_PC_PARTITION_BSD_TYPE_BOOT    = 13
  };

/* FreeBSD-specific types.  */
enum
  {
    GRUB_PC_PARTITION_FREEBSD_TYPE_VINUM = 14,
    GRUB_PC_PARTITION_FREEBSD_TYPE_RAID  = 15,
    GRUB_PC_PARTITION_FREEBSD_TYPE_JFS2  = 21
  };

/* NetBSD-specific types.  */
enum
  {
    GRUB_PC_PARTITION_NETBSD_TYPE_ADOS     = 14,
    GRUB_PC_PARTITION_NETBSD_TYPE_HFS      = 15,
    GRUB_PC_PARTITION_NETBSD_TYPE_FILECORE = 16,
    GRUB_PC_PARTITION_NETBSD_TYPE_EXT2FS   = 17,
    GRUB_PC_PARTITION_NETBSD_TYPE_NTFS     = 18,
    GRUB_PC_PARTITION_NETBSD_TYPE_RAID     = 19,
    GRUB_PC_PARTITION_NETBSD_TYPE_CCD      = 20,
    GRUB_PC_PARTITION_NETBSD_TYPE_JFS2     = 21,
    GRUB_PC_PARTITION_NETBSD_TYPE_APPLEUFS = 22
  };

/* OpenBSD-specific types.  */
enum
  {
    GRUB_PC_PARTITION_OPENBSD_TYPE_ADOS     = 14,
    GRUB_PC_PARTITION_OPENBSD_TYPE_HFS      = 15,
    GRUB_PC_PARTITION_OPENBSD_TYPE_FILECORE = 16,
    GRUB_PC_PARTITION_OPENBSD_TYPE_EXT2FS   = 17,
    GRUB_PC_PARTITION_OPENBSD_TYPE_NTFS     = 18,
    GRUB_PC_PARTITION_OPENBSD_TYPE_RAID     = 19
  };

#define GRUB_PC_PARTITION_BSD_LABEL_WHOLE_DISK_PARTITION 2

/* The BSD partition entry.  */
struct grub_partition_bsd_entry
{
  grub_uint32_t size;
  grub_uint32_t offset;
  grub_uint32_t fragment_size;
  grub_uint8_t fs_type;
  grub_uint8_t fs_fragments;
  grub_uint16_t fs_cylinders;
} GRUB_PACKED;

/* The BSD disk label. Only define members useful for GRUB.  */
struct grub_partition_bsd_disk_label
{
  grub_uint32_t magic;
  grub_uint16_t type;
  grub_uint8_t unused1[18];
  grub_uint8_t packname[16];
  grub_uint8_t unused2[92];
  grub_uint32_t magic2;
  grub_uint16_t checksum;
  grub_uint16_t num_partitions;
  grub_uint32_t boot_size;
  grub_uint32_t superblock_size;
} GRUB_PACKED;

#endif /* ! GRUB_PC_PARTITION_HEADER */
