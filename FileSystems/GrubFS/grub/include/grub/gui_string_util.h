/* gui_string_util.h - String utilities for the graphical menu interface. */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2008,2009  Free Software Foundation, Inc.
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

#ifndef GRUB_GUI_STRING_UTIL_HEADER
#define GRUB_GUI_STRING_UTIL_HEADER 1

#include <grub/types.h>
#include <grub/gui.h>

char *grub_new_substring (const char *buf,
                          grub_size_t start, grub_size_t end);

char *grub_resolve_relative_path (const char *base, const char *path);

char *grub_get_dirname (const char *file_path);

#endif /* GRUB_GUI_STRING_UTIL_HEADER */
