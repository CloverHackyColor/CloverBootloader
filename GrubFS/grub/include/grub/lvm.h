/* lvm.h - On disk structures for LVM. */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2006,2007  Free Software Foundation, Inc.
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

#ifndef GRUB_LVM_H
#define GRUB_LVM_H	1

#include <grub/types.h>
#include <grub/diskfilter.h>

/* Length of ID string, excluding terminating zero. */
#define GRUB_LVM_ID_STRLEN 38

#define GRUB_LVM_LABEL_SIZE GRUB_DISK_SECTOR_SIZE
#define GRUB_LVM_LABEL_SCAN_SECTORS 4L

#define GRUB_LVM_LABEL_ID "LABELONE"
#define GRUB_LVM_LVM2_LABEL "LVM2 001"

#define GRUB_LVM_ID_LEN 32

/* On disk - 32 bytes */
struct grub_lvm_label_header {
  grub_int8_t id[8];		/* LABELONE */
  grub_uint64_t sector_xl;	/* Sector number of this label */
  grub_uint32_t crc_xl;		/* From next field to end of sector */
  grub_uint32_t offset_xl;	/* Offset from start of struct to contents */
  grub_int8_t type[8];		/* LVM2 001 */
} GRUB_PACKED;

/* On disk */
struct grub_lvm_disk_locn {
  grub_uint64_t offset;		/* Offset in bytes to start sector */
  grub_uint64_t size;		/* Bytes */
} GRUB_PACKED;

/* Fields with the suffix _xl should be xlate'd wherever they appear */
/* On disk */
struct grub_lvm_pv_header {
  grub_int8_t pv_uuid[GRUB_LVM_ID_LEN];

  /* This size can be overridden if PV belongs to a VG */
  grub_uint64_t device_size_xl;	/* Bytes */

  /* NULL-terminated list of data areas followed by */
  /* NULL-terminated list of metadata area headers */
  struct grub_lvm_disk_locn disk_areas_xl[0];	/* Two lists */
} GRUB_PACKED;

#define GRUB_LVM_FMTT_MAGIC "\040\114\126\115\062\040\170\133\065\101\045\162\060\116\052\076"
#define GRUB_LVM_FMTT_VERSION 1
#define GRUB_LVM_MDA_HEADER_SIZE 512

/* On disk */
struct grub_lvm_raw_locn {
  grub_uint64_t offset;		/* Offset in bytes to start sector */
  grub_uint64_t size;		/* Bytes */
  grub_uint32_t checksum;
  grub_uint32_t filler;
} GRUB_PACKED;

/* On disk */
/* Structure size limited to one sector */
struct grub_lvm_mda_header {
  grub_uint32_t checksum_xl;	/* Checksum of rest of mda_header */
  grub_int8_t magic[16];	/* To aid scans for metadata */
  grub_uint32_t version;
  grub_uint64_t start;		/* Absolute start byte of mda_header */
  grub_uint64_t size;		/* Size of metadata area */

  struct grub_lvm_raw_locn raw_locns[0];	/* NULL-terminated list */
} GRUB_PACKED;


#endif /* ! GRUB_LVM_H */
