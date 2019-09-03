/*
 * gptsync/gptsync.h
 * Common header for gptsync and showpart
 *
 * Copyright (c) 2006 Christoph Pfisterer
 * All rights reserved.
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

#ifndef _GPTSYNC_H
#define _GPTSYNC_H 1

//
// config
//

//
// platform-dependent types
//

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DevicePathLib.h>
#include <Library/MemoryAllocationLib.h>

#include <Protocol/BlockIo.h>
#include <Protocol/BlockIo2.h>

#define copy_guid(destguid, srcguid) (CopyMem(destguid, srcguid, 16))
#define guids_are_equal(guid1, guid2) (CompareMem(guid1, guid2, 16) == 0)

typedef CHAR16 CHARN;
#define STR(x) L##x

//
// platform-independent types
//

typedef struct _MBR_PARTITION_INFO {
    UINT8   flags;
    UINT8   start_chs[3];
    UINT8   type;
    UINT8   end_chs[3];
    UINT32  start_lba;
    UINT32  size;
} MBR_PARTITION_INFO;

typedef struct _MBR_PARTTYPE {
    UINT8   type;
    CHARN   *name;
} MBR_PARTTYPE;

typedef struct _GPT_HEADER {
    UINT64  signature;
    UINT32  spec_revision;
    UINT32  header_size;
    UINT32  header_crc32;
    UINT32  reserved;
    UINT64  header_lba;
    UINT64  alternate_header_lba;
    UINT64  first_usable_lba;
    UINT64  last_usable_lba;
    UINT8   disk_guid[16];
    UINT64  entry_lba;
    UINT32  entry_count;
    UINT32  entry_size;
    UINT32  entry_crc32;
} GPT_HEADER;

typedef struct _GPT_ENTRY {
    UINT8   type_guid[16];
    UINT8   partition_guid[16];
    UINT64  start_lba;
    UINT64  end_lba;
    UINT64  attributes;
    CHAR16  name[36];
} GPT_ENTRY;

#define GPT_KIND_SYSTEM     (0)
#define GPT_KIND_DATA       (1)
#define GPT_KIND_BASIC_DATA (2)
#define GPT_KIND_FATAL      (3)

typedef struct {
    UINT8   guid[16];
    UINT8   mbr_type;
    CHARN   *name;
    UINTN   kind;
} GPT_PARTTYPE;

typedef struct {
    UINTN   index;
    UINT64  start_lba;
    UINT64  end_lba;
    UINTN   mbr_type;
    UINT8   gpt_type[16];
    GPT_PARTTYPE *gpt_parttype;
    BOOLEAN active;
} PARTITION_INFO;

//
// functions provided by the OS-specific module
//

UINTN read_sector(UINT64 lba, UINT8 *buffer);
UINTN write_sector(UINT64 lba, UINT8 *buffer);
UINTN input_boolean(CHARN *prompt, BOOLEAN *bool_out);

//
// vars and functions provided by the common lib module
//

extern UINT8           empty_guid[16];

extern PARTITION_INFO  mbr_parts[4];
extern UINTN           mbr_part_count;
extern PARTITION_INFO  gpt_parts[128];
extern UINTN           gpt_part_count;

extern PARTITION_INFO  new_mbr_parts[4];
extern UINTN           new_mbr_part_count;

extern UINT8           sector[512];

extern MBR_PARTTYPE    mbr_types[];
extern GPT_PARTTYPE    gpt_types[];
extern GPT_PARTTYPE    gpt_dummy_type;

CHARN * mbr_parttype_name(UINT8 type);
UINTN read_mbr(VOID);

GPT_PARTTYPE * gpt_parttype(UINT8 *type_guid);
UINTN read_gpt(VOID);

UINTN detect_mbrtype_fs(UINT64 partlba, UINTN *parttype, CHARN **fsname);

//
// actual platform-independent programs
//

UINTN gptsync(VOID);
UINTN showpart(VOID);

/* EOF */
#endif
