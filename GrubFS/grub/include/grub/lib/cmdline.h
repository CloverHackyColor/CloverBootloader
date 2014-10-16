/* cmdline.h - linux command line handling */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2010  Free Software Foundation, Inc.
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

#ifndef GRUB_CMDLINE_HEADER
#define GRUB_CMDLINE_HEADER	1

#include <grub/types.h>

#define LINUX_IMAGE "BOOT_IMAGE="

unsigned int grub_loader_cmdline_size (int argc, char *argv[]);
int grub_create_loader_cmdline (int argc, char *argv[], char *buf,
				grub_size_t size);

#endif /* ! GRUB_CMDLINE_HEADER */
