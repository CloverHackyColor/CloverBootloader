/*
 * gptsync/showpart.c
 * Platform-independent code for analyzing hard disk partitioning
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

#include "gptsync.h"

//
// memory string search
//

static INTN FindMem(VOID *Buffer, UINTN BufferLength, VOID *SearchString, UINTN SearchStringLength)
{
    UINT8 *BufferPtr;
    UINTN Offset;
    
    BufferPtr = Buffer;
    BufferLength -= SearchStringLength;
    for (Offset = 0; Offset < BufferLength; Offset++, BufferPtr++) {
        if (CompareMem(BufferPtr, SearchString, SearchStringLength) == 0)
            return (INTN)Offset;
    }
    
    return -1;
}

//
// detect boot code
//

static UINTN detect_bootcode(UINT64 partlba, CHARN **bootcodename)
{
    UINTN   status;
    BOOLEAN bootable;
    
    // read MBR data
    status = read_sector(partlba, sector);
    if (status != 0)
        return status;
    
    // check bootable signature
    if (*((UINT16 *)(sector + 510)) == 0xaa55 && sector[0] != 0)
        bootable = TRUE;
    else
        bootable = FALSE;
    *bootcodename = NULL;
    
    // detect specific boot codes
    if (CompareMem(sector + 2, "LILO", 4) == 0 ||
        CompareMem(sector + 6, "LILO", 4) == 0) {
        *bootcodename = STR("LILO");
        
    } else if (CompareMem(sector + 3, "SYSLINUX", 8) == 0) {
        *bootcodename = STR("SYSLINUX");
        
    } else if (FindMem(sector, 512, "ISOLINUX", 8) >= 0) {
        *bootcodename = STR("ISOLINUX");
        
    } else if (FindMem(sector, 512, "Geom\0Hard Disk\0Read\0 Error", 26) >= 0) {
        *bootcodename = STR("GRUB");
        
    } else if ((*((UINT32 *)(sector + 502)) == 0 &&
                *((UINT32 *)(sector + 506)) == 50000 &&
                *((UINT16 *)(sector + 510)) == 0xaa55) ||
               FindMem(sector, 512, "Starting the BTX loader", 23) >= 0) {
        *bootcodename = STR("FreeBSD");
        
    } else if (FindMem(sector, 512, "!Loading", 8) >= 0 ||
               FindMem(sector, 512, "/cdboot\0/CDBOOT\0", 16) >= 0) {
        *bootcodename = STR("OpenBSD");
        
    } else if (FindMem(sector, 512, "Not a bootxx image", 18) >= 0) {
        *bootcodename = STR("NetBSD");
        
    } else if (FindMem(sector, 512, "NTLDR", 5) >= 0) {
        *bootcodename = STR("Windows NTLDR");
        
    } else if (FindMem(sector, 512, "BOOTMGR", 7) >= 0) {
        *bootcodename = STR("Windows BOOTMGR (Vista)");
        
    } else if (FindMem(sector, 512, "CPUBOOT SYS", 11) >= 0 ||
               FindMem(sector, 512, "KERNEL  SYS", 11) >= 0) {
        *bootcodename = STR("FreeDOS");
        
    } else if (FindMem(sector, 512, "OS2LDR", 6) >= 0 ||
               FindMem(sector, 512, "OS2BOOT", 7) >= 0) {
        *bootcodename = STR("eComStation");
        
    } else if (FindMem(sector, 512, "Be Boot Loader", 14) >= 0) {
        *bootcodename = STR("BeOS");
        
    } else if (FindMem(sector, 512, "yT Boot Loader", 14) >= 0) {
        *bootcodename = STR("ZETA");
        
    } else if (FindMem(sector, 512, "\x04" "beos\x06" "system\x05" "zbeos", 18) >= 0) {
        *bootcodename = STR("Haiku");
        
    } else if (FindMem(sector, 512, "\x0A\x0Dboot0ss: ", 11) >= 0) {
        *bootcodename = STR ("Mac OS X (boot0ss)");
    } else if (FindMem(sector, 512, "\x0A\x0Dboot0af: ", 11) >= 0) {
        *bootcodename = STR ("Mac OS X (boot0af)");
    } else if (FindMem(sector, 512, "\x0A\x0Dboot0: ", 9) >= 0) {
        *bootcodename = STR ("Mac OS X (boot0)");
   }
    
    if (FindMem(sector, 512, "Non-system disk", 15) >= 0)   // dummy FAT boot sector
        *bootcodename = STR("None (Non-system disk message)");
    
    // TODO: Add a note if a specific code was detected, but the sector is not bootable?
    
    if (*bootcodename == NULL) {
        if (bootable)
            *bootcodename = STR("Unknown, but bootable");
        else
            *bootcodename = STR("None");
    }
    
    return 0;
}

//
// check one partition
//

static UINTN analyze_part(UINT64 partlba)
{
    UINTN   status;
    UINTN   i;
    CHARN   *bootcodename;
    UINTN   parttype;
    CHARN   *fsname;
    
    if (partlba == 0)
        Print(L"\nMBR contents:\n");
    else
        Print(L"\nPartition at LBA %lld:\n", partlba);
    
    // detect boot code
    status = detect_bootcode(partlba, &bootcodename);
    if (status)
        return status;
    Print(L" Boot Code: %s\n", bootcodename);
    
    if (partlba == 0)
        return 0;   // short-circuit MBR analysis
    
    // detect file system
    status = detect_mbrtype_fs(partlba, &parttype, &fsname);
    if (status)
        return status;
    Print(L" File System: %s\n", fsname);
    
    // cross-reference with partition table
    for (i = 0; i < gpt_part_count; i++) {
        if (gpt_parts[i].start_lba == partlba) {
            Print(L" Listed in GPT as partition %d, type %s\n", i+1,
                  gpt_parts[i].gpt_parttype->name);
        }
    }
    for (i = 0; i < mbr_part_count; i++) {
        if (mbr_parts[i].start_lba == partlba) {
            Print(L" Listed in MBR as partition %d, type %02x  %s%s\n", i+1,
                  mbr_parts[i].mbr_type,
                  mbr_parttype_name((UINT8)mbr_parts[i].mbr_type),
                  mbr_parts[i].active ? STR(", active") : STR(""));
        }
    }
    
    return 0;
}

//
// check all partitions
//

static UINTN analyze_parts(VOID)
{
    UINTN   i, k;
    UINTN   status;
    BOOLEAN is_dupe;
    
    // check MBR (bootcode only)
    status = analyze_part(0);
    if (status)
        return status;
    
    // check partitions listed in GPT
    for (i = 0; i < gpt_part_count; i++) {
        status = analyze_part(gpt_parts[i].start_lba);
        if (status)
            return status;
    }
    
    // check partitions listed in MBR, but not in GPT
    for (i = 0; i < mbr_part_count; i++) {
        if (mbr_parts[i].start_lba == 1 && mbr_parts[i].mbr_type == 0xee)
            continue;   // skip EFI Protective entry
        
        is_dupe = FALSE;
        for (k = 0; k < gpt_part_count; k++)
            if (gpt_parts[k].start_lba == mbr_parts[i].start_lba)
                is_dupe = TRUE;
        
        if (!is_dupe) {
            status = analyze_part(mbr_parts[i].start_lba);
            if (status)
                return status;
        }
    }
    
    return 0;
}

//
// display algorithm entry point
//

UINTN showpart(VOID)
{
    UINTN   status = 0;
    UINTN   status_gpt, status_mbr;
    
    // get full information from disk
    status_gpt = read_gpt();
    status_mbr = read_mbr();
    if (status_gpt != 0 || status_mbr != 0)
        return (status_gpt || status_mbr);
    
    // analyze all partitions
    status = analyze_parts();
    if (status != 0)
        return status;
    
    return status;
}
