/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2010,2011,2012,2013  Free Software Foundation, Inc.
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

#ifndef GRUB_EMU_HOSTFILE_H
#define GRUB_EMU_HOSTFILE_H 1

#include <config.h>
#include <stdarg.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>

typedef struct dirent *grub_util_fd_dirent_t;
typedef DIR *grub_util_fd_dir_t;

static inline grub_util_fd_dir_t
grub_util_fd_opendir (const char *name)
{
  return opendir (name);
}

static inline void
grub_util_fd_closedir (grub_util_fd_dir_t dirp)
{
  closedir (dirp);
}

static inline grub_util_fd_dirent_t
grub_util_fd_readdir (grub_util_fd_dir_t dirp)
{
  return readdir (dirp);
}

static inline int
grub_util_rmdir (const char *pathname)
{
  return rmdir (pathname);
}

static inline int
grub_util_unlink (const char *pathname)
{
  return unlink (pathname);
}

static inline int
grub_util_rename (const char *from, const char *to)
{
  return rename (from, to);
}

#define grub_util_mkdir(a) mkdir ((a), 0755)

struct grub_util_fd
{
  enum { GRUB_UTIL_FD_FILE, GRUB_UTIL_FD_DISK } type;
  grub_uint64_t off;
  union
  {
    int fd;
    struct {
      struct IOExtTD *ioreq;
      struct MsgPort *mp;
      unsigned int is_floppy:1;
      unsigned int is_64:1;
    };
  };
};
typedef struct grub_util_fd *grub_util_fd_t;

enum grub_util_fd_open_flags_t
  {
    GRUB_UTIL_FD_O_RDONLY = O_RDONLY,
    GRUB_UTIL_FD_O_WRONLY = O_WRONLY,
    GRUB_UTIL_FD_O_RDWR = O_RDWR,
    GRUB_UTIL_FD_O_CREATTRUNC = O_CREAT | O_TRUNC,
    GRUB_UTIL_FD_O_SYNC = (0
#ifdef O_SYNC
			   | O_SYNC
#endif
#ifdef O_FSYNC
			   | O_FSYNC
#endif
			   )
  };

#define GRUB_UTIL_FD_INVALID NULL
#define GRUB_UTIL_FD_IS_VALID(x) ((x) != GRUB_UTIL_FD_INVALID)
#define GRUB_UTIL_FD_STAT_IS_FUNCTIONAL 0

#define DEFAULT_DIRECTORY	"SYS:" GRUB_BOOT_DIR_NAME "/" GRUB_DIR_NAME
#define DEFAULT_DEVICE_MAP	DEFAULT_DIRECTORY "/device.map"

#endif
