/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2009  Free Software Foundation, Inc.
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

#ifndef GRUB_MACHOLOAD_HEADER
#define GRUB_MACHOLOAD_HEADER	1

#include <grub/err.h>
#include <grub/elf.h>
#include <grub/file.h>
#include <grub/symbol.h>
#include <grub/types.h>

struct grub_macho_file
{
  grub_file_t file;
  grub_ssize_t offset32;
  grub_ssize_t end32;
  int ncmds32;
  grub_size_t cmdsize32;
  grub_uint8_t *cmds32;
  grub_uint8_t *uncompressed32;
  int compressed32;
  grub_size_t compressed_size32;
  grub_size_t uncompressed_size32;
  grub_ssize_t offset64;
  grub_ssize_t end64;
  int ncmds64;
  grub_size_t cmdsize64;
  grub_uint8_t *cmds64;
  grub_uint8_t *uncompressed64;
  int compressed64;
  grub_size_t compressed_size64;
  grub_size_t uncompressed_size64;
};
typedef struct grub_macho_file *grub_macho_t;

grub_macho_t grub_macho_open (const char *, int is_64bit);
grub_macho_t grub_macho_file (grub_file_t file, const char *filename,
			      int is_64bit);
grub_err_t grub_macho_close (grub_macho_t);

grub_err_t grub_macho_size32 (grub_macho_t macho, grub_uint32_t *segments_start,
			      grub_uint32_t *segments_end, int flags,
			      const char *filename);
grub_uint32_t grub_macho_get_entry_point32 (grub_macho_t macho,
					    const char *filename);

grub_err_t grub_macho_size64 (grub_macho_t macho, grub_uint64_t *segments_start,
			      grub_uint64_t *segments_end, int flags,
			      const char *filename);
grub_uint64_t grub_macho_get_entry_point64 (grub_macho_t macho,
					    const char *filename);

/* Ignore BSS segments when loading. */
#define GRUB_MACHO_NOBSS 0x1
grub_err_t grub_macho_load32 (grub_macho_t macho, const char *filename,
			      char *offset, int flags, int *darwin_version);
grub_err_t grub_macho_load64 (grub_macho_t macho, const char *filename,
			      char *offset, int flags, int *darwin_version);

/* Like filesize and file_read but take only 32-bit part
   for current architecture. */
grub_size_t grub_macho_filesize32 (grub_macho_t macho);
grub_err_t grub_macho_readfile32 (grub_macho_t macho, const char *filename,
				  void *dest);
grub_size_t grub_macho_filesize64 (grub_macho_t macho);
grub_err_t grub_macho_readfile64 (grub_macho_t macho, const char *filename,
				  void *dest);

void grub_macho_parse32 (grub_macho_t macho, const char *filename);
void grub_macho_parse64 (grub_macho_t macho, const char *filename);

#endif /* ! GRUB_MACHOLOAD_HEADER */
