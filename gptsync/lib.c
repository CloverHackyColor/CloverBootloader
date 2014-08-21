/*
 * gptsync/lib.c
 * Platform-independent code common to gptsync and showpart
 *
 * Copyright (c) 2006-2007 Christoph Pfisterer
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

#include "gptsync.h"

// variables

UINT8 empty_guid[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

PARTITION_INFO  mbr_parts[4];
UINTN           mbr_part_count = 0;
PARTITION_INFO  gpt_parts[128];
UINTN           gpt_part_count = 0;

PARTITION_INFO  new_mbr_parts[4];
UINTN           new_mbr_part_count = 0;

UINT8           sector[512];

MBR_PARTTYPE    mbr_types[] = {
    { 0x01, STR("FAT12 (CHS)") },
    { 0x04, STR("FAT16 <32M (CHS)") },
    { 0x05, STR("Extended (CHS)") },
    { 0x06, STR("FAT16 (CHS)") },
    { 0x07, STR("NTFS/HPFS") },
    { 0x0b, STR("FAT32 (CHS)") },
    { 0x0c, STR("FAT32 (LBA)") },
    { 0x0e, STR("FAT16 (LBA)") },
    { 0x0f, STR("Extended (LBA)") },
    { 0x11, STR("Hidden FAT12 (CHS)") },
    { 0x14, STR("Hidden FAT16 <32M (CHS)") },
    { 0x16, STR("Hidden FAT16 (CHS)") },
    { 0x17, STR("Hidden NTFS/HPFS") },
    { 0x1b, STR("Hidden FAT32 (CHS)") },
    { 0x1c, STR("Hidden FAT32 (LBA)") },
    { 0x1e, STR("Hidden FAT16 (LBA)") },
    { 0x82, STR("Linux swap / Solaris") },
    { 0x83, STR("Linux") },
    { 0x85, STR("Linux Extended") },
    { 0x86, STR("NT FAT volume set") },
    { 0x87, STR("NTFS volume set") },
    { 0x8e, STR("Linux LVM") },
    { 0xa5, STR("FreeBSD") },
    { 0xa6, STR("OpenBSD") },
    { 0xa7, STR("NeXTSTEP") },
    { 0xa8, STR("Mac OS X UFS") },
    { 0xa9, STR("NetBSD") },
    { 0xab, STR("Mac OS X Boot") },
    { 0xac, STR("Apple RAID") },
    { 0xaf, STR("Mac OS X HFS+") },
    { 0xbe, STR("Solaris Boot") },
    { 0xbf, STR("Solaris") },
    { 0xeb, STR("BeOS") },
    { 0xee, STR("EFI Protective") },
    { 0xef, STR("EFI System (FAT)") },
    { 0xfd, STR("Linux RAID") },
    { 0, NULL },
};

#ifdef _MSC_VER
#pragma warning(disable : 4295)
#endif

GPT_PARTTYPE    gpt_types[] = {
    // Defined by EFI/UEFI specification
    { "\x28\x73\x2A\xC1\x1F\xF8\xD2\x11\xBA\x4B\x00\xA0\xC9\x3E\xC9\x3B", 0xef, STR("EFI System (FAT)"), GPT_KIND_SYSTEM },
    { "\x41\xEE\x4D\x02\xE7\x33\xD3\x11\x9D\x69\x00\x08\xC7\x81\xF3\x9F", 0x00, STR("MBR partition scheme"), GPT_KIND_FATAL },
    // Generally well-known
    { "\x16\xE3\xC9\xE3\x5C\x0B\xB8\x4D\x81\x7D\xF9\x2D\xF0\x02\x15\xAE", 0x00, STR("MS Reserved"), GPT_KIND_SYSTEM },
    { "\xA2\xA0\xD0\xEB\xE5\xB9\x33\x44\x87\xC0\x68\xB6\xB7\x26\x99\xC7", 0x00, STR("Basic Data"), GPT_KIND_BASIC_DATA },
    // From Wikipedia
    { "\xAA\xC8\x08\x58\x8F\x7E\xE0\x42\x85\xD2\xE1\xE9\x04\x34\xCF\xB3", 0x00, STR("MS LDM Metadata"), GPT_KIND_FATAL },
    { "\xA0\x60\x9B\xAF\x31\x14\x62\x4F\xBC\x68\x33\x11\x71\x4A\x69\xAD", 0x00, STR("MS LDM Data"), GPT_KIND_FATAL },
    { "\x1E\x4C\x89\x75\xEB\x3A\xD3\x11\xB7\xC1\x7B\x03\xA0\x00\x00\x00", 0x00, STR("HP/UX Data"), GPT_KIND_DATA },
    { "\x28\xE7\xA1\xE2\xE3\x32\xD6\x11\xA6\x82\x7B\x03\xA0\x00\x00\x00", 0x00, STR("HP/UX Service"), GPT_KIND_SYSTEM },
    // From Linux repository, fs/partitions/efi.h
    { "\x0F\x88\x9D\xA1\xFC\x05\x3B\x4D\xA0\x06\x74\x3F\x0F\x84\x91\x1E", 0xfd, STR("Linux RAID"), GPT_KIND_DATA },
    { "\x6D\xFD\x57\x06\xAB\xA4\xC4\x43\x84\xE5\x09\x33\xC8\x4B\x4F\x4F", 0x82, STR("Linux Swap"), GPT_KIND_SYSTEM },
    { "\x79\xD3\xD6\xE6\x07\xF5\xC2\x44\xA2\x3C\x23\x8F\x2A\x3D\xF9\x28", 0x8e, STR("Linux LVM"), GPT_KIND_DATA },
    // From Wikipedia
    { "\x39\x33\xA6\x8D\x07\x00\xC0\x60\xC4\x36\x08\x3A\xC8\x23\x09\x08", 0x00, STR("Linux Reserved"), GPT_KIND_SYSTEM },
    // From grub2 repository, grub/include/grub/gpt_partition.h
    { "\x48\x61\x68\x21\x49\x64\x6F\x6E\x74\x4E\x65\x65\x64\x45\x46\x49", 0x00, STR("GRUB2 BIOS Boot"), GPT_KIND_SYSTEM },
    // From FreeBSD repository, sys/sys/gpt.h
    { "\xB4\x7C\x6E\x51\xCF\x6E\xD6\x11\x8F\xF8\x00\x02\x2D\x09\x71\x2B", 0xa5, STR("FreeBSD Data"), GPT_KIND_DATA },
    { "\xB5\x7C\x6E\x51\xCF\x6E\xD6\x11\x8F\xF8\x00\x02\x2D\x09\x71\x2B", 0x00, STR("FreeBSD Swap"), GPT_KIND_SYSTEM },
    { "\xB6\x7C\x6E\x51\xCF\x6E\xD6\x11\x8F\xF8\x00\x02\x2D\x09\x71\x2B", 0xa5, STR("FreeBSD UFS"), GPT_KIND_DATA },
    { "\xB8\x7C\x6E\x51\xCF\x6E\xD6\x11\x8F\xF8\x00\x02\x2D\x09\x71\x2B", 0x00, STR("FreeBSD Vinum"), GPT_KIND_DATA },
    { "\xBA\x7C\x6E\x51\xCF\x6E\xD6\x11\x8F\xF8\x00\x02\x2D\x09\x71\x2B", 0xa5, STR("FreeBSD ZFS"), GPT_KIND_DATA },
    { "\x9D\x6B\xBD\x83\x41\x7F\xDC\x11\xBE\x0B\x00\x15\x60\xB8\x4F\x0F", 0xa5, STR("FreeBSD Boot"), GPT_KIND_DATA },
    // From NetBSD repository, sys/sys/disklabel_gpt.h
    { "\x32\x8D\xF4\x49\x0E\xB1\xDC\x11\xB9\x9B\x00\x19\xD1\x87\x96\x48", 0x00, STR("NetBSD Swap"), GPT_KIND_SYSTEM },
    { "\x5A\x8D\xF4\x49\x0E\xB1\xDC\x11\xB9\x9B\x00\x19\xD1\x87\x96\x48", 0xa9, STR("NetBSD FFS"), GPT_KIND_DATA },
    { "\x82\x8D\xF4\x49\x0E\xB1\xDC\x11\xB9\x9B\x00\x19\xD1\x87\x96\x48", 0xa9, STR("NetBSD LFS"), GPT_KIND_DATA },
    { "\xAA\x8D\xF4\x49\x0E\xB1\xDC\x11\xB9\x9B\x00\x19\xD1\x87\x96\x48", 0xa9, STR("NetBSD RAID"), GPT_KIND_DATA },
    { "\xC4\x19\xB5\x2D\x0E\xB1\xDC\x11\xB9\x9B\x00\x19\xD1\x87\x96\x48", 0xa9, STR("NetBSD CCD"), GPT_KIND_DATA },
    { "\xEC\x19\xB5\x2D\x0E\xB1\xDC\x11\xB9\x9B\x00\x19\xD1\x87\x96\x48", 0xa9, STR("NetBSD CGD"), GPT_KIND_DATA },
    // From http://developer.apple.com/mac/library/technotes/tn2006/tn2166.html
    { "\x00\x53\x46\x48\x00\x00\xAA\x11\xAA\x11\x00\x30\x65\x43\xEC\xAC", 0xaf, STR("Mac OS X HFS+"), GPT_KIND_DATA },
    { "\x00\x53\x46\x55\x00\x00\xAA\x11\xAA\x11\x00\x30\x65\x43\xEC\xAC", 0xa8, STR("Mac OS X UFS"), GPT_KIND_DATA },
    { "\x74\x6F\x6F\x42\x00\x00\xAA\x11\xAA\x11\x00\x30\x65\x43\xEC\xAC", 0xab, STR("Mac OS X Boot"), GPT_KIND_DATA },
    { "\x44\x49\x41\x52\x00\x00\xAA\x11\xAA\x11\x00\x30\x65\x43\xEC\xAC", 0xac, STR("Apple RAID"), GPT_KIND_DATA },
    { "\x44\x49\x41\x52\x4F\x5F\xAA\x11\xAA\x11\x00\x30\x65\x43\xEC\xAC", 0xac, STR("Apple RAID (Offline)"), GPT_KIND_DATA },
    { "\x65\x62\x61\x4C\x00\x6C\xAA\x11\xAA\x11\x00\x30\x65\x43\xEC\xAC", 0x00, STR("Apple Label"), GPT_KIND_SYSTEM },
    // From Wikipedia
    { "\x6F\x63\x65\x52\x65\x76\xAA\x11\xAA\x11\x00\x30\x65\x43\xEC\xAC", 0x00, STR("Apple TV Recovery"), GPT_KIND_BASIC_DATA },
    // From OpenSolaris repository, usr/src/uts/common/sys/efi_partition.h
    { "\x7f\x23\x96\x6A\xD2\x1D\xB2\x11\x99\xa6\x08\x00\x20\x73\x66\x31", 0x00, STR("Solaris Reserved"), GPT_KIND_SYSTEM },
    { "\x45\xCB\x82\x6A\xD2\x1D\xB2\x11\x99\xa6\x08\x00\x20\x73\x66\x31", 0xbf, STR("Solaris Boot"), GPT_KIND_DATA },
    { "\x4D\xCF\x85\x6A\xD2\x1D\xB2\x11\x99\xa6\x08\x00\x20\x73\x66\x31", 0xbf, STR("Solaris Root"), GPT_KIND_DATA },
    { "\x6F\xC4\x87\x6A\xD2\x1D\xB2\x11\x99\xa6\x08\x00\x20\x73\x66\x31", 0x00, STR("Solaris Swap"), GPT_KIND_SYSTEM },
    { "\xC3\x8C\x89\x6A\xD2\x1D\xB2\x11\x99\xa6\x08\x00\x20\x73\x66\x31", 0xbf, STR("Solaris Usr / Apple ZFS"), GPT_KIND_DATA },
    { "\x2B\x64\x8B\x6A\xD2\x1D\xB2\x11\x99\xa6\x08\x00\x20\x73\x66\x31", 0x00, STR("Solaris Backup"), GPT_KIND_SYSTEM },
    { "\xC7\x2A\x8D\x6A\xD2\x1D\xB2\x11\x99\xa6\x08\x00\x20\x73\x66\x31", 0x00, STR("Solaris Reserved (Stand)"), GPT_KIND_SYSTEM },
    { "\xE9\xF2\x8E\x6A\xD2\x1D\xB2\x11\x99\xa6\x08\x00\x20\x73\x66\x31", 0xbf, STR("Solaris Var"), GPT_KIND_DATA },
    { "\x39\xBA\x90\x6A\xD2\x1D\xB2\x11\x99\xa6\x08\x00\x20\x73\x66\x31", 0xbf, STR("Solaris Home"), GPT_KIND_DATA },
    { "\xA5\x83\x92\x6A\xD2\x1D\xB2\x11\x99\xa6\x08\x00\x20\x73\x66\x31", 0x00, STR("Solaris Alternate Sector"), GPT_KIND_SYSTEM },
    { "\x3B\x5A\x94\x6A\xD2\x1D\xB2\x11\x99\xa6\x08\x00\x20\x73\x66\x31", 0x00, STR("Solaris Reserved (Cache)"), GPT_KIND_SYSTEM },
    { "\xD1\x30\x96\x6A\xD2\x1D\xB2\x11\x99\xa6\x08\x00\x20\x73\x66\x31", 0x00, STR("Solaris Reserved"), GPT_KIND_SYSTEM },
    { "\x67\x07\x98\x6A\xD2\x1D\xB2\x11\x99\xa6\x08\x00\x20\x73\x66\x31", 0x00, STR("Solaris Reserved"), GPT_KIND_SYSTEM },
    // List sentinel
    { { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }, 0, NULL, 0 },
};

#ifdef _MSC_VER
#pragma warning(default : 4295)
#endif

GPT_PARTTYPE gpt_dummy_type = { { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }, 0, STR("Unknown"), GPT_KIND_FATAL };

//
// MBR functions
//

CHARN * mbr_parttype_name(UINT8 type)
{
    int i = 0;
    
    for (i = 0; mbr_types[i].name; i++) {
        if (mbr_types[i].type == type) {
            return mbr_types[i].name;
        }
    }

    return STR("Unknown");
}

UINTN read_mbr(VOID)
{
    UINTN               status = 0;
    UINTN               i = 0;
    BOOLEAN             used = FALSE;
    MBR_PARTITION_INFO  *table = NULL;
    
    Print(L"\nCurrent MBR partition table:\n");
    
    // read MBR data
    status = read_sector(0, sector);
    if (status != 0) {
        return status;
    }

    // check for validity
    if (*((UINT16 *)(sector + 510)) != 0xaa55) {
        Print(L" No MBR partition table present!\n");
        return 1;
    }

    table = (MBR_PARTITION_INFO *)(sector + 446);

    for (i = 0; i < 4; i++) {
        if (table[i].flags != 0x00 && table[i].flags != 0x80) {
            Print(L" MBR partition table is invalid!\n");
            return 1;
        }
    }
    
    // check if used
    used = FALSE;

    for (i = 0; i < 4; i++) {
        if ((table[i].start_lba > 0) && (table[i].size > 0)) {
            used = TRUE;
            break;
        }
    }

    if (!used) {
        Print(L" No partitions defined\n");
        return 0;
    }
    
    // dump current state & fill internal structures
    Print(L" # A    Start LBA      End LBA  Type\n");
    for (i = 0; i < 4; i++) {
        if ((table[i].start_lba == 0) || (table[i].size == 0)) {
            continue;
        }

        mbr_parts[mbr_part_count].index     = i;
        mbr_parts[mbr_part_count].start_lba = (UINT64)table[i].start_lba;
        mbr_parts[mbr_part_count].end_lba   = (UINT64)table[i].start_lba + (UINT64)table[i].size - 1;
        mbr_parts[mbr_part_count].mbr_type  = table[i].type;
        mbr_parts[mbr_part_count].active    = (table[i].flags == 0x80) ? TRUE : FALSE;
        
        Print(L" %d %s %12lld %12lld  %02x  %s\n",
              mbr_parts[mbr_part_count].index + 1,
              mbr_parts[mbr_part_count].active ? STR("*") : STR(" "),
              mbr_parts[mbr_part_count].start_lba,
              mbr_parts[mbr_part_count].end_lba,
              mbr_parts[mbr_part_count].mbr_type,
              mbr_parttype_name((UINT8)mbr_parts[mbr_part_count].mbr_type));
        
        mbr_part_count++;
    }
    
    return 0;
}

//
// GPT functions
//

GPT_PARTTYPE * gpt_parttype(UINT8 *type_guid)
{
    int i = 0;
    
    for (i = 0; gpt_types[i].name; i++) {
        if (guids_are_equal(gpt_types[i].guid, type_guid)) {
            return &(gpt_types[i]);
        }
    }

    return &gpt_dummy_type;
}

UINTN read_gpt(VOID)
{
    UINTN       status = 0;
    GPT_HEADER  *header = NULL;
    GPT_ENTRY   *entry = NULL;
    UINT64      entry_lba = 0;
    UINTN       entry_count = 0;
    UINTN       entry_size = 0;
    UINTN       i = 0;
    
    Print(L"\nCurrent GPT partition table:\n");
    
    // read GPT header
    status = read_sector(1, sector);

    if (status != 0) {
        return status;
    }
    
    // check signature
    header = (GPT_HEADER *)sector;

    if (header->signature != 0x5452415020494645ULL) {
        Print(L" No GPT partition table present!\n");
        return 0;
    }

    if (header->spec_revision != 0x00010000UL) {
        Print(L" Warning: Unknown GPT spec revision 0x%08x\n", header->spec_revision);
    }

    if (((512 % header->entry_size) > 0) || (header->entry_size > 512)) {
        Print(L" Error: Invalid GPT entry size (misaligned or more than 512 bytes)\n");
        return 0;
    }
    
    // read entries
    entry_lba   = header->entry_lba;
    entry_size  = header->entry_size;
    entry_count = header->entry_count;
    
    for (i = 0; i < entry_count; i++) {
        if (((i * entry_size) % 512) == 0) {
            status = read_sector(entry_lba, sector);
            if (status != 0) {
                return status;
            }
            entry_lba++;
        }

        entry = (GPT_ENTRY *)(sector + ((i * entry_size) % 512));
        
        if (guids_are_equal(entry->type_guid, empty_guid)) {
            continue;
        }

        if (gpt_part_count == 0) {
            Print(L" #      Start LBA      End LBA  Type\n");
        }
        
        gpt_parts[gpt_part_count].index     = i;
        gpt_parts[gpt_part_count].start_lba = entry->start_lba;
        gpt_parts[gpt_part_count].end_lba   = entry->end_lba;
        gpt_parts[gpt_part_count].mbr_type  = 0;
        copy_guid(gpt_parts[gpt_part_count].gpt_type, entry->type_guid);
        gpt_parts[gpt_part_count].gpt_parttype = gpt_parttype(gpt_parts[gpt_part_count].gpt_type);
        gpt_parts[gpt_part_count].active    = FALSE;

        Print(L" %d   %12lld %12lld  %s\n",
              gpt_parts[gpt_part_count].index + 1,
              gpt_parts[gpt_part_count].start_lba,
              gpt_parts[gpt_part_count].end_lba,
              gpt_parts[gpt_part_count].gpt_parttype->name);
        
        gpt_part_count++;
    }

    if (gpt_part_count == 0) {
        Print(L" No partitions defined\n");
        return 0;
    }
    
    return 0;
}

//
// detect file system type
//

UINTN detect_mbrtype_fs(UINT64 partlba, UINTN *parttype, CHARN **fsname)
{
    UINTN status = 0;
    UINTN signature = 0;
    UINTN score = 0;
    UINTN sectsize = 0;
    UINTN clustersize = 0;
    UINTN reserved = 0;
    UINTN fatcount = 0;
    UINTN dirsize = 0;
    UINTN sectcount = 0;
    UINTN fatsize = 0;
    UINTN clustercount = 0;
    
    *fsname = STR("Unknown");
    *parttype = 0;
    
    // READ sector 0 / offset 0K
    status = read_sector(partlba, sector);

    if (status != 0) {
        return status;
    }

    // detect XFS
    signature = *((UINT32 *)(sector));

    if (signature == 0x42534658) {
        *parttype = 0x83;
        *fsname = STR("XFS");
        return 0;
    }
    
    // detect FAT and NTFS
    sectsize = *((UINT16 *)(sector + 11));
    clustersize = sector[13];

    if ((sectsize >= 512) && ((sectsize & (sectsize - 1)) == 0) &&
        (clustersize > 0) && ((clustersize & (clustersize - 1)) == 0)) {
        // preconditions for both FAT and NTFS are now met
        
        if (CompareMem(sector + 3, "NTFS    ", 8) == 0) {
            *parttype = 0x07;
            *fsname = STR("NTFS");
            return 0;
        }
        
        score = 0;

        // boot jump
        if (((sector[0] == 0xEB) && (sector[2] == 0x90)) || 
            (sector[0] == 0xE9)) {
            score++;
        }

        // boot signature
        if ((sector[510] == 0x55) && (sector[511] == 0xAA)) {
            score++;
        }

        // reserved sectors
        reserved = *((UINT16 *)(sector + 14));

        if ((reserved == 1) || (reserved == 32)) {
            score++;
        }

        // number of FATs
        fatcount = sector[16];

        if (fatcount == 2) {
            score++;
        }

        // number of root dir entries
        dirsize = *((UINT16 *)(sector + 17));

        // sector count (16-bit and 32-bit versions)
        sectcount = *((UINT16 *)(sector + 19));

        if (sectcount == 0) {
            sectcount = *((UINT32 *)(sector + 32));
        }

        // media byte
        if ((sector[21] == 0xF0) || (sector[21] >= 0xF8)) {
            score++;
        }

        // FAT size in sectors
        fatsize = *((UINT16 *)(sector + 22));

        if (fatsize == 0) {
            fatsize = *((UINT32 *)(sector + 36));
        }

        // determine FAT type
        dirsize = (((dirsize * 32) + (sectsize - 1)) / sectsize);
        clustercount = (sectcount - (reserved + (fatcount * fatsize) + dirsize));
        clustercount /= clustersize;
        
        if (score >= 3) {
            if (clustercount < 4085) {
                *parttype = 0x01;
                *fsname = STR("FAT12");
            } else if (clustercount < 65525) {
                *parttype = 0x0e;
                *fsname = STR("FAT16");
            } else {
                *parttype = 0x0c;
                *fsname = STR("FAT32");
            }

            // TODO: check if 0e and 0c are okay to use, maybe we should use 06 and 0b instead...

            return 0;
        }
    }
    
    // READ sector 2 / offset 1K
    status = read_sector(partlba + 2, sector);

    if (status != 0) {
        return status;
    }
   
    // detect HFS+
    signature = *((UINT16 *)(sector));

    if (signature == 0x4442) {
        *parttype = 0xaf;
        if (*((UINT16 *)(sector + 0x7c)) == 0x2B48)  {
            *fsname = STR("HFS Extended (HFS+)");
        } else {
            *fsname = STR("HFS Standard");
        }

        return 0;
    } else if (signature == 0x2B48) {
        *parttype = 0xaf;
        *fsname = STR("HFS Extended (HFS+)");
        return 0;
    }
    
    // detect ext2/ext3/ext4
    signature = *((UINT16 *)(sector + 56));

    if (signature == 0xEF53) {
        *parttype = 0x83;
        if ((*((UINT16 *)(sector + 96)) & 0x02C0) ||
            (*((UINT16 *)(sector + 100)) & 0x0078)) {
            *fsname = STR("ext4");
        } else if (*((UINT16 *)(sector + 92)) & 0x0004) {
            *fsname = STR("ext3");
        } else {
            *fsname = STR("ext2");
        }
 
        return 0;
    }
    
    // READ sector 128 / offset 64K
    status = read_sector(partlba + 128, sector);

    if (status != 0) {
        return status;
    }

    // detect btrfs
    if (CompareMem(sector + 64, "_BHRfS_M", 8) == 0) {
        *parttype = 0x83;
        *fsname = STR("btrfs");
        return 0;
    }
    
    // detect ReiserFS
    if ((CompareMem(sector + 52, "ReIsErFs", 8)  == 0) ||
        (CompareMem(sector + 52, "ReIsEr2Fs", 9) == 0) ||
        (CompareMem(sector + 52, "ReIsEr3Fs", 9) == 0)) {
        *parttype = 0x83;
        *fsname = STR("ReiserFS");
        return 0;
    }
    
    // detect Reiser4
    if (CompareMem(sector, "ReIsEr4", 7) == 0) {
        *parttype = 0x83;
        *fsname = STR("Reiser4");
        return 0;
    }
    
    // READ sector 64 / offset 32K
    status = read_sector(partlba + 64, sector);
    if (status != 0) {
        return status;
    }

    // detect JFS
    if (CompareMem(sector, "JFS1", 4) == 0) {
        *parttype = 0x83;
        *fsname = STR("JFS");
        return 0;
    }
    
    // READ sector 16 / offset 8K
    status = read_sector(partlba + 16, sector);

    if (status != 0) {
        return status;
    }

    // detect ReiserFS
    if ((CompareMem(sector + 52, "ReIsErFs",  8) == 0) ||
        (CompareMem(sector + 52, "ReIsEr2Fs", 9) == 0) ||
        (CompareMem(sector + 52, "ReIsEr3Fs", 9) == 0)) {
        *parttype = 0x83;
        *fsname = STR("ReiserFS");
        return 0;
    }
    
    return 0;
}
