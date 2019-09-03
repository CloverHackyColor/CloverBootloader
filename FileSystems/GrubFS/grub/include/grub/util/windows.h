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

#ifndef GRUB_WINDOWS_UTIL_HEADER
#define GRUB_WINDOWS_UTIL_HEADER	1

#include <windows.h>

LPTSTR
grub_util_get_windows_path (const char *unix_path);

char *
grub_util_tchar_to_utf8 (LPCTSTR in);

TCHAR *
grub_get_mount_point (const TCHAR *path);

#endif
