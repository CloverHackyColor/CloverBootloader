/**
 * \file lsroot.c
 * Example program for the POSIX user space environment.
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

#include "fsw_posix.h"


extern struct fsw_fstype_table FSW_FSTYPE_TABLE_NAME(ext2);
extern struct fsw_fstype_table FSW_FSTYPE_TABLE_NAME(reiserfs);
extern struct fsw_fstype_table FSW_FSTYPE_TABLE_NAME(iso9660);
extern struct fsw_fstype_table FSW_FSTYPE_TABLE_NAME(hfs);

int main(int argc, char **argv)
{
    struct fsw_posix_volume *vol;
    struct fsw_posix_dir *dir;
    struct dirent *dent;

    if (argc != 2) {
        printf("Usage: lsroot <file/device>\n");
        return 1;
    }

    //vol = fsw_posix_mount(argv[1], &FSW_FSTYPE_TABLE_NAME(ext2));
    //vol = fsw_posix_mount(argv[1], &FSW_FSTYPE_TABLE_NAME(reiserfs));
    vol = fsw_posix_mount(argv[1], &FSW_FSTYPE_TABLE_NAME(FSTYPE));
    if (vol == NULL) {
        printf("Mounting failed.\n");
        return 1;
    }
    //dir = fsw_posix_opendir(vol, "/drivers/net/");
    dir = fsw_posix_opendir(vol, "/");
    if (dir == NULL) {
        printf("opendir call failed.\n");
        return 1;
    }
    while ((dent = fsw_posix_readdir(dir)) != NULL) {
        printf("- %s\n", dent->d_name);
    }
    fsw_posix_closedir(dir);
    fsw_posix_unmount(vol);

    return 0;
}

// EOF
