/**
 * \file lslr.c
 * Test program for the POSIX user space environment.
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


//extern struct fsw_fstype_table FSW_FSTYPE_TABLE_NAME(ext2);
//extern struct fsw_fstype_table FSW_FSTYPE_TABLE_NAME(reiserfs);
extern struct fsw_fstype_table FSW_FSTYPE_TABLE_NAME(FSTYPE);

static struct fsw_fstype_table *fstypes[] = {
    //&FSW_FSTYPE_TABLE_NAME(ext2),
    //&FSW_FSTYPE_TABLE_NAME(reiserfs),
    &FSW_FSTYPE_TABLE_NAME(FSTYPE       ),
    NULL
};

static int listdir(struct fsw_posix_volume *vol, char *path, int level)
{
    struct fsw_posix_dir *dir;
    struct dirent *dent;
    int i;
    char subpath[4096];

    dir = fsw_posix_opendir(vol, path);
    if (dir == NULL) {
        printf("opendir(%s) call failed.\n", path);
        return 1;
    }
    while ((dent = fsw_posix_readdir(dir)) != NULL) {
        for (i = 0; i < level*2; i++)
            fputc(' ', stdout);
        printf("%d  %s\n", dent->d_type, dent->d_name);

        if (dent->d_type == DT_DIR) {
            snprintf(subpath, 4095, "%s%s/", path, dent->d_name);
            listdir(vol, subpath, level + 1);
        }
    }
    fsw_posix_closedir(dir);

    return 0;
}

static int catfile(struct fsw_posix_volume *vol, char *path)
{
    struct fsw_posix_file *file;
    int r;
    char buf[256];

    file = fsw_posix_open(vol, path, 0, 0);
    if (file == NULL) {
        printf("open(%s) call failed.\n", path);
        return 1;
    }
    while ((r=fsw_posix_read(file, buf, sizeof(buf))) > 0)
    {
        int i;
        for (i=0; i<r; i++)
        {
           printf("%c", buf[i]);
        }
    }
    fsw_posix_close(file);

    return 0;
}

int main(int argc, char **argv)
{
    struct fsw_posix_volume *vol;
    int i;

    if (argc != 2) {
        printf("Usage: lslr <file/device>\n");
        return 1;
    }

    for (i = 0; fstypes[i]; i++) {
        vol = fsw_posix_mount(argv[1], fstypes[i]);
        if (vol != NULL) {
            printf("Mounted as '%s'.\n", fstypes[i]->name.data);
            break;
        }
    }
    if (vol == NULL) {
        printf("Mounting failed.\n");
        return 1;
    }

    //listdir(vol, "/System/Library/Extensions/udf.kext/", 0);
    //listdir(vol, "/System/Library/Extensions/AppleACPIPlatform.kext/", 0);
    //listdir(vol, "/System/Library/Extensions/", 0);
    catfile(vol, "/System/Library/Extensions/AppleHPET.kext/Contents/Info.plist");
    //listdir(vol, "/", 0);

    fsw_posix_unmount(vol);

    return 0;
}

// EOF
