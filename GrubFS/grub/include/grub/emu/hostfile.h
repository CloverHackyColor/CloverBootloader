/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2013  Free Software Foundation, Inc.
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

#ifndef GRUB_HOSTFILE_EMU_HEADER
#define GRUB_HOSTFILE_EMU_HEADER	1

#include <grub/disk.h>
#include <grub/partition.h>
#include <sys/types.h>
#include <grub/osdep/hostfile.h>

int
grub_util_is_directory (const char *path);
int
grub_util_is_special_file (const char *path);
int
grub_util_is_regular (const char *path);

char *
grub_util_path_concat (size_t n, ...);
char *
grub_util_path_concat_ext (size_t n, ...);

int
grub_util_fd_seek (grub_util_fd_t fd, grub_uint64_t off);
ssize_t
EXPORT_FUNC(grub_util_fd_read) (grub_util_fd_t fd, char *buf, size_t len);
ssize_t
EXPORT_FUNC(grub_util_fd_write) (grub_util_fd_t fd, const char *buf, size_t len);

grub_util_fd_t
EXPORT_FUNC(grub_util_fd_open) (const char *os_dev, int flags);
const char *
EXPORT_FUNC(grub_util_fd_strerror) (void);
void
grub_util_fd_sync (grub_util_fd_t fd);
void
grub_util_disable_fd_syncs (void);
void
EXPORT_FUNC(grub_util_fd_close) (grub_util_fd_t fd);

grub_uint64_t
grub_util_get_fd_size (grub_util_fd_t fd, const char *name, unsigned *log_secsize);
char *
grub_util_make_temporary_file (void);
char *
grub_util_make_temporary_dir (void);
void
grub_util_unlink_recursive (const char *name);
grub_uint32_t
grub_util_get_mtime (const char *name);

#endif /* ! GRUB_BIOSDISK_MACHINE_UTIL_HEADER */
