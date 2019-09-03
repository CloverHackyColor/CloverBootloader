/*
 * gptsync/gptsync.c
 * Platform-independent code for syncing GPT and MBR
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

#ifdef _USE_LINUX_MBR
#include "syslinux_mbr.h"
#else /* _USE_CLOVER_MBR */
#include "clover_boot0ss_mbr.h"
#endif /* _USE_MBR */

//
// MBR functions
//

static UINTN check_mbr(VOID)
{
    UINTN i = 0;
    UINTN k = 0;
    
    // check each entry
    for (i = 0; i < mbr_part_count; i++) {
        // check for overlap
        for (k = 0; k < mbr_part_count; k++) {
            if (k != i && !(mbr_parts[i].start_lba > mbr_parts[k].end_lba || mbr_parts[k].start_lba > mbr_parts[i].end_lba)) {
                Print(L"Status: MBR partition table is invalid, partitions overlap.\n");
                return 1;
            }
        }
        
        // check for extended partitions
        if ((mbr_parts[i].mbr_type == 0x05) || (mbr_parts[i].mbr_type == 0x0f) || (mbr_parts[i].mbr_type == 0x85)) {
            Print(L"Status: Extended partition found in MBR table, will not touch this disk.\n",
                  gpt_parts[i].gpt_parttype->name);
            return 1;
        }
    }
    
    return 0;
}

static UINTN write_mbr(VOID)
{
    UINTN               status = 0;
    UINTN               i = 0;
    UINTN			  k = 0;
    UINT8               active = 0;
    UINT64              lba = 0;
    MBR_PARTITION_INFO  *table = NULL;
    BOOLEAN             have_bootcode = FALSE;
    
    Print(L"\nWriting new MBR...\n");
    
    // read MBR data
    status = read_sector(0, sector);
    if (status != 0)
        return status;
    
    // write partition table
    *((UINT16 *)(sector + 510)) = 0xaa55;
    
    table = (MBR_PARTITION_INFO *)(sector + 446);
    active = 0x80;
    for (i = 0; i < 4; i++) {
        for (k = 0; k < new_mbr_part_count; k++) {
            if (new_mbr_parts[k].index == i)
                break;
        }
        if (k >= new_mbr_part_count) {
            // unused entry
            table[i].flags        = 0;
            table[i].start_chs[0] = 0;
            table[i].start_chs[1] = 0;
            table[i].start_chs[2] = 0;
            table[i].type         = 0;
            table[i].end_chs[0]   = 0;
            table[i].end_chs[1]   = 0;
            table[i].end_chs[2]   = 0;
            table[i].start_lba    = 0;
            table[i].size         = 0;
        } else {
            if (new_mbr_parts[k].active) {
                table[i].flags        = active;
                active = 0x00;
            } else
                table[i].flags        = 0x00;
            table[i].start_chs[0] = 0xfe;
            table[i].start_chs[1] = 0xff;
            table[i].start_chs[2] = 0xff;
            table[i].type         = (UINT8)new_mbr_parts[k].mbr_type;
            table[i].end_chs[0]   = 0xfe;
            table[i].end_chs[1]   = 0xff;
            table[i].end_chs[2]   = 0xff;
            
            lba = new_mbr_parts[k].start_lba;
            if (lba > 0xffffffffULL) {
                Print(L"Warning: Partition %d starts beyond 2 TiB limit\n", i+1);
                lba = 0xffffffffULL;
            }
            table[i].start_lba    = (UINT32)lba;
            
            lba = new_mbr_parts[k].end_lba + 1 - new_mbr_parts[k].start_lba;
            if (lba > 0xffffffffULL) {
                Print(L"Warning: Partition %d extends beyond 2 TiB limit\n", i+1);
                lba = 0xffffffffULL;
            }
            table[i].size         = (UINT32)lba;
        }
    }
    
    // add boot code if necessary
    have_bootcode = FALSE;
    for (i = 0; i < MBR_BOOTCODE_SIZE; i++) {
        if (sector[i] != 0) {
            have_bootcode = TRUE;
            break;
        }
    }

    if (!have_bootcode) {
        // no boot code found in the MBR, add the syslinux MBR code
        // or add the Clover boot0ss MBR code
        SetMem(sector, MBR_BOOTCODE_SIZE, 0);

#ifdef __SYSLINUX_MBR_H__
        CopyMem(sector, syslinux_mbr, SYSLINUX_MBR_SIZE);
#else /* __CLOVER_BOOT0SS_MBR_H__ */
        CopyMem(sector, clover_boot0ss_mbr, CLOVER_BOOT0SS_MBR_SIZE);
#endif /* _MBR_COPY_DATA_H_ */
    }
    
    // write MBR data
    status = write_sector(0, sector);
    if (status != 0)
        return status;
    
    Print(L"MBR updated successfully!\n");
    
    return 0;
}

//
// GPT functions
//

static UINTN check_gpt(VOID)
{
    UINTN       i, k;
    BOOLEAN     found_data_parts;
    
    if (gpt_part_count == 0) {
        Print(L"Status: No GPT partition table, no need to sync.\n");
        return 1;
    }
    
    // check each entry
    found_data_parts = FALSE;
    for (i = 0; i < gpt_part_count; i++) {
        // check sanity
        if (gpt_parts[i].end_lba < gpt_parts[i].start_lba) {
            Print(L"Status: GPT partition table is invalid.\n");
            return 1;
        }
        // check for overlap
        for (k = 0; k < gpt_part_count; k++) {
            if (k != i && !(gpt_parts[i].start_lba > gpt_parts[k].end_lba || gpt_parts[k].start_lba > gpt_parts[i].end_lba)) {
                Print(L"Status: GPT partition table is invalid, partitions overlap.\n");
                return 1;
            }
        }
        
        // check for partitions kind
        if (gpt_parts[i].gpt_parttype->kind == GPT_KIND_FATAL) {
            Print(L"Status: GPT partition of type '%s' found, will not touch this disk.\n",
                  gpt_parts[i].gpt_parttype->name);
            return 1;
        }
        if ((gpt_parts[i].gpt_parttype->kind == GPT_KIND_DATA) ||
            (gpt_parts[i].gpt_parttype->kind == GPT_KIND_BASIC_DATA))
            found_data_parts = TRUE;
    }
    
    if (!found_data_parts) {
        Print(L"Status: GPT partition table has no data partitions, no need to sync.\n");
        return 1;
    }
    
    return 0;
}

//
// compare GPT and MBR tables
//

#define ACTION_NONE        (0)
#define ACTION_NOP         (1)
#define ACTION_REWRITE     (2)

static UINTN analyze(VOID)
{
    UINTN   action;
    UINTN   i, k, iter, count_active, detected_parttype;
    CHARN   *fsname;
    UINT64  min_start_lba;
    UINTN   status;
    BOOLEAN have_esp;
    
    new_mbr_part_count = 0;
    
    // determine correct MBR types for GPT partitions
    if (gpt_part_count == 0) {
        Print(L"Status: No GPT partitions defined, nothing to sync.\n");
        return 0;
    }
    have_esp = FALSE;
    for (i = 0; i < gpt_part_count; i++) {
        gpt_parts[i].mbr_type = gpt_parts[i].gpt_parttype->mbr_type;
        if (gpt_parts[i].gpt_parttype->kind == GPT_KIND_BASIC_DATA) {
            // Basic Data: need to look at data in the partition
            status = detect_mbrtype_fs(gpt_parts[i].start_lba, &detected_parttype, &fsname);
            if (detected_parttype)
                gpt_parts[i].mbr_type = detected_parttype;
            else
                gpt_parts[i].mbr_type = 0x0b;  // fallback: FAT32
        } else if (gpt_parts[i].mbr_type == 0xef) {
            // EFI System Partition: GNU parted can put this on any partition,
            // need to detect file systems
            status = detect_mbrtype_fs(gpt_parts[i].start_lba, &detected_parttype, &fsname);
            if (!have_esp && (detected_parttype == 0x01 || detected_parttype == 0x0e || detected_parttype == 0x0c))
                ;  // seems to be a legitimate ESP, don't change
            else if (detected_parttype)
                gpt_parts[i].mbr_type = detected_parttype;
            else if (have_esp)    // make sure there's no more than one ESP per disk
                gpt_parts[i].mbr_type = 0x83;  // fallback: Linux
        }
        // NOTE: mbr_type may still be 0 if content detection fails for exotic GPT types or file systems
        
        if (gpt_parts[i].mbr_type == 0xef)
            have_esp = TRUE;
    }
    
    // check for common scenarios
    action = ACTION_NONE;
    if (mbr_part_count == 0) {
        // current MBR is empty
        action = ACTION_REWRITE;
    } else if ((mbr_part_count == 1) && (mbr_parts[0].mbr_type == 0xee)) {
        // MBR has just the EFI Protective partition (i.e. untouched)
        action = ACTION_REWRITE;
    }
    if ((action == ACTION_NONE) && (mbr_part_count > 0)) {
        if (mbr_parts[0].mbr_type == 0xee &&
            gpt_parts[0].mbr_type == 0xef &&
            mbr_parts[0].start_lba == 1 &&
            mbr_parts[0].end_lba == gpt_parts[0].end_lba) {
            // The Apple Way, "EFI Protective" covering the tables and the ESP
            action = ACTION_NOP;
            if (((mbr_part_count != gpt_part_count) && (gpt_part_count <= 4)) ||
                ((mbr_part_count != 4)              && (gpt_part_count > 4))) {
                // number of partitions has changed
                action = ACTION_REWRITE;
            } else {
                // check partition ranges and types
                for (i = 1; i < mbr_part_count; i++) {
                    if ((mbr_parts[i].start_lba != gpt_parts[i].start_lba) ||
                        (mbr_parts[i].end_lba   != gpt_parts[i].end_lba) ||
                        ((gpt_parts[i].mbr_type) && (mbr_parts[i].mbr_type != gpt_parts[i].mbr_type)))
                        // position or type has changed
                        action = ACTION_REWRITE;
                }
            }
            // check number of active partitions
            count_active = 0;
            for (i = 0; i < mbr_part_count; i++)
                if (mbr_parts[i].active)
                    count_active++;
            if (count_active!= 1)
                action = ACTION_REWRITE;
        }
    }
    if ((action == ACTION_NONE) && (mbr_part_count > 0) && (mbr_parts[0].mbr_type == 0xef)) {
        // The XOM Way, all partitions mirrored 1:1
        action = ACTION_REWRITE;
        // check partition ranges and types
        for (i = 0; i < mbr_part_count; i++) {
            if ((mbr_parts[i].start_lba != gpt_parts[i].start_lba) ||
                (mbr_parts[i].end_lba   != gpt_parts[i].end_lba) ||
                ((gpt_parts[i].mbr_type) && (mbr_parts[i].mbr_type != gpt_parts[i].mbr_type)))
                // position or type has changed -> better don't touch
                action = ACTION_NONE;
        }
    }
    
    if (action == ACTION_NOP) {
        Print(L"Status: Tables are synchronized, no need to sync.\n");
        return 0;
    } else if (action == ACTION_REWRITE) {
        Print(L"Status: MBR table must be updated.\n");
    } else {
        Print(L"Status: Analysis inconclusive, will not touch this disk.\n");
        return 1;
    }
    
    // generate the new table
    
    // first entry: EFI Protective
    new_mbr_parts[0].index     = 0;
    new_mbr_parts[0].start_lba = 1;
    new_mbr_parts[0].mbr_type  = 0xee;
    new_mbr_part_count = 1;
    
    if (gpt_parts[0].mbr_type == 0xef) {
        new_mbr_parts[0].end_lba = gpt_parts[0].end_lba;
        i = 1;
    } else {
        min_start_lba = gpt_parts[0].start_lba;
        for (k = 0; k < gpt_part_count; k++) {
            if (min_start_lba > gpt_parts[k].start_lba) {
                min_start_lba = gpt_parts[k].start_lba;
            }
        }

        new_mbr_parts[0].end_lba = min_start_lba - 1;
        i = 0;
    }
    
    // add other GPT partitions until the table is full
    // TODO: in the future, prioritize partitions by kind
    for (; i < gpt_part_count && new_mbr_part_count < 4; i++) {
        new_mbr_parts[new_mbr_part_count].index = new_mbr_part_count;
        new_mbr_parts[new_mbr_part_count].start_lba = gpt_parts[i].start_lba;
        new_mbr_parts[new_mbr_part_count].end_lba = gpt_parts[i].end_lba;
        new_mbr_parts[new_mbr_part_count].mbr_type = gpt_parts[i].mbr_type;
        new_mbr_parts[new_mbr_part_count].active = FALSE;
        
        // find matching partition in the old MBR table
        for (k = 0; k < mbr_part_count; k++) {
            if (mbr_parts[k].start_lba == gpt_parts[i].start_lba) {
                // keep type if not detected
                if (new_mbr_parts[new_mbr_part_count].mbr_type == 0) {
                    new_mbr_parts[new_mbr_part_count].mbr_type = mbr_parts[k].mbr_type;
                }

                // keep active flag
                new_mbr_parts[new_mbr_part_count].active = mbr_parts[k].active;
                break;
            }
        }
        
        if (new_mbr_parts[new_mbr_part_count].mbr_type == 0) {
            // final fallback: set to a (hopefully) unused type
            new_mbr_parts[new_mbr_part_count].mbr_type = 0xc0;
        }

        new_mbr_part_count++;
    }
    
    // make sure there's exactly one active partition
    for (iter = 0; iter < 3; iter++) {
        // check
        count_active = 0;
        for (i = 0; i < new_mbr_part_count; i++) {
            if (new_mbr_parts[i].active) {
                count_active++;
            }
        }

        if (count_active == 1) {
            break;
        }

        // set active on the first matching partition
        if (count_active == 0) {
            for (i = 0; i < new_mbr_part_count; i++) {
                if (((iter == 0) &&
                    ((new_mbr_parts[i].mbr_type == 0xa8) ||    // Mac OS X UFS
                     (new_mbr_parts[i].mbr_type == 0xab) ||    // Mac OS X Boot
                     (new_mbr_parts[i].mbr_type == 0xac) ||    // Apple RAID
                     (new_mbr_parts[i].mbr_type == 0xaf) ||    // HFS+
                     (new_mbr_parts[i].mbr_type == 0x07) ||    // NTFS
                     (new_mbr_parts[i].mbr_type == 0x0b) ||    // FAT32
                     (new_mbr_parts[i].mbr_type == 0x0c))) ||  // FAT32 (LBA)
                    ((iter >= 1) && (new_mbr_parts[i].mbr_type == 0x83)) ||   // Linux
                    ((iter >= 2) && (i > 0))) {
                    new_mbr_parts[i].active = TRUE;
                    break;
                }
            }
        } else if ((count_active > 1) && (iter == 0)) {
            // too many active partitions, try deactivating the ESP / EFI Protective entry

            if (((new_mbr_parts[0].mbr_type == 0xee) || 
                 (new_mbr_parts[0].mbr_type == 0xef)) &&
                  new_mbr_parts[0].active) {
                new_mbr_parts[0].active = FALSE;
            }
        } else if ((count_active > 1) && (iter > 0)) {
            // too many active partitions, deactivate all but the first one
            count_active = 0;
            for (i = 0; i < new_mbr_part_count; i++) {
                if (new_mbr_parts[i].active) {
                    if (count_active > 0) {
                        new_mbr_parts[i].active = FALSE;
                    }
                    count_active++;
                }
            }
        }
    }
    
    // dump table
    Print(L"\nProposed new MBR partition table:\n");
    Print(L" # A    Start LBA      End LBA  Type\n");
    for (i = 0; i < new_mbr_part_count; i++) {
        Print(L" %d %s %12lld %12lld  %02x  %s\n",
              new_mbr_parts[i].index + 1,
              new_mbr_parts[i].active ? STR("*") : STR(" "),
              new_mbr_parts[i].start_lba,
              new_mbr_parts[i].end_lba,
              new_mbr_parts[i].mbr_type,
              mbr_parttype_name((UINT8)new_mbr_parts[i].mbr_type));
    }
    
    return 0;
}

//
// sync algorithm entry point
//

UINTN gptsync(VOID)
{
    UINTN   status = 0;
    UINTN   status_gpt, status_mbr;
    BOOLEAN proceed = FALSE;
    
    // get full information from disk
    status_gpt = read_gpt();
    status_mbr = read_mbr();
    if ((status_gpt != 0) || (status_mbr != 0))
        return ((status_gpt) || (status_mbr));
    
    // cross-check current situation
    Print(L"\n");
    status = check_gpt();   // check GPT for consistency
    if (status != 0)
        return status;
    status = check_mbr();   // check MBR for consistency
    if (status != 0)
        return status;
    status = analyze();     // analyze the situation & compose new MBR table
    if (status != 0)
        return status;
    if (new_mbr_part_count == 0)
        return status;
    
    // offer user the choice what to do
    status = input_boolean(STR("\nMay I update the MBR as printed above? [y/N] "), &proceed);
    if ((status != 0) || (proceed != TRUE))
        return status;
    
    // adjust the MBR and write it back
    status = write_mbr();
    if (status != 0)
        return status;
    
    return status;
}
