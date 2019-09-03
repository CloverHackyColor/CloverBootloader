/* $Id: fsw_iso9660.h 29125 2010-05-06 09:43:05Z vboxsync $ */
/** @file
 * fsw_iso9660.h - ISO9660 file system driver header.
 */

/*-
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

#ifndef _FSW_ISO9660_H_
#define _FSW_ISO9660_H_

#define VOLSTRUCTNAME fsw_iso9660_volume
#define DNODESTRUCTNAME fsw_iso9660_dnode
#include "fsw_core.h"


//! Block size for ISO9660 volumes.
#define ISO9660_BLOCKSIZE          2048
#define ISO9660_BLOCKSIZE_BITS       11
//! Block number where the ISO9660 superblock resides.
#define ISO9660_SUPERBLOCK_BLOCKNO   16
//Slice - we already have shifted blockIO by 16
//but we should use ParentBlockIo
//#define ISO9660_SUPERBLOCK_BLOCKNO   0

#pragma pack(1)

typedef struct {
    fsw_u16     lsb;
    fsw_u16     msb;
} iso9660_u16;

typedef struct {
    fsw_u32     lsb;
    fsw_u32     msb;
} iso9660_u32;

#define ISOINT(lsbmsbvalue) ((lsbmsbvalue).lsb)

struct iso9660_dirrec {
    fsw_u8      dirrec_length;
    fsw_u8      ear_length;
    iso9660_u32 extent_location;
    iso9660_u32 data_length;
    fsw_u8      recording_datetime[7];
    fsw_u8      file_flags;
    fsw_u8      file_unit_size;
    fsw_u8      interleave_gap_size;
    iso9660_u16 volume_sequence_number;
    fsw_u8      file_identifier_length;
    char        file_identifier[1];
};
//#if sizeof(struct fsw_iso9660_dirrec) != 34
//#fail Structure fsw_iso9660_dirrec has wrong size
//#endif

struct iso9660_volume_descriptor {
    fsw_u8      volume_descriptor_type;
    char        standard_identifier[5];
    fsw_u8      volume_descriptor_version;
};

struct iso9660_primary_volume_descriptor {
    fsw_u8      volume_descriptor_type;
    char        standard_identifier[5];
    fsw_u8      volume_descriptor_version;
    fsw_u8      unused1;
    char        system_identifier[32];
    char        volume_identifier[32];
    fsw_u8      unused2[8];
    iso9660_u32 volume_space_size;
    fsw_u8      unused3[4];
    fsw_u8      escape[3];
    fsw_u8      unused4[25];
    iso9660_u16 volume_set_size;
    iso9660_u16 volume_sequence_number;
    iso9660_u16 logical_block_size;
    iso9660_u32 path_table_size;
    fsw_u32     location_type_l_path_table;
    fsw_u32     location_optional_type_l_path_table;
    fsw_u32     location_type_m_path_table;
    fsw_u32     location_optional_type_m_path_table;
    struct iso9660_dirrec root_directory;
    char        volume_set_identifier[128];
    char        publisher_identifier[128];
    char        data_preparer_identifier[128];
    char        application_identifier[128];
    char        copyright_file_identifier[37];
    char        abstract_file_identifier[37];
    char        bibliographic_file_identifier[37];
    char        volume_creation_datetime[17];
    char        volume_modification_datetime[17];
    char        volume_expiration_datetime[17];
    char        volume_effective_datetime[17];
    fsw_u8      file_structure_version;
    fsw_u8      reserved1;
    fsw_u8      application_use[512];
    fsw_u8      reserved2[653];
};
//#if sizeof(struct fsw_iso9660_volume_descriptor) != 2048
//#fail Structure fsw_iso9660_volume_descriptor has wrong size
//#endif

#pragma pack()

struct iso9660_dirrec_buffer {
    fsw_u32     ino;
    struct fsw_string name;
    struct iso9660_dirrec dirrec;
    char        dirrec_buffer[222];
};


/**
 * ISO9660: Volume structure with ISO9660-specific data.
 */

struct fsw_iso9660_volume {
    struct fsw_volume g;            //!< Generic volume structure
    /*Note: don't move g!*/
    int fJoliet;
    /*Joliet specific fields*/
    int fRockRidge;
    /*Rock Ridge specific fields*/
    int rr_susp_skip;

    struct iso9660_primary_volume_descriptor *primary_voldesc;  //!< Full Primary Volume Descriptor
};

/**
 * ISO9660: Dnode structure with ISO9660-specific data.
 */

struct fsw_iso9660_dnode {
    struct fsw_dnode g;             //!< Generic dnode structure

    struct iso9660_dirrec dirrec;   //!< Fixed part of the directory record (i.e. w/o name)
};


struct fsw_rock_ridge_susp_entry
{
    fsw_u8  sig[2];
    fsw_u8  len;
    fsw_u8  ver;
};

struct fsw_rock_ridge_susp_sp
{
    struct fsw_rock_ridge_susp_entry e;
    fsw_u8  magic[2];
    fsw_u8  skip;
};

struct fsw_rock_ridge_susp_nm
{
    struct fsw_rock_ridge_susp_entry e;
    fsw_u8  flags;
    fsw_u8  name[1];
};

#define RR_NM_CONT (1<<0)
#define RR_NM_CURR (1<<1)
#define RR_NM_PARE (1<<2)

union fsw_rock_ridge_susp_ce
{
    struct X{
        struct fsw_rock_ridge_susp_entry e;
        iso9660_u32 block_loc;
        iso9660_u32 offset;
        iso9660_u32 len;
    } X;
    fsw_u8 raw[28];
};

#endif
