/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2008  Free Software Foundation, Inc.
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

#ifndef GRUB_LEGACY_PARSE_HEADER
#define GRUB_LEGACY_PARSE_HEADER 1

#include <grub/types.h>

char *grub_legacy_parse (const char *buf, char **entryname, char **suffix);
char *grub_legacy_escape (const char *in, grub_size_t len);

/* Entered has to be GRUB_AUTH_MAX_PASSLEN long, zero-padded.  */
int
grub_legacy_check_md5_password (int argc, char **args,
				char *entered);

#endif
