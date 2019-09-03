/* xz_stream.h - Definitions for handling the .xz file format */
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
/*
 * This file is based on code from XZ embedded project
 * http://tukaani.org/xz/embedded.html
 */

#ifndef XZ_STREAM_H
#define XZ_STREAM_H

/*
 * See the .xz file format specification at
 * http://tukaani.org/xz/xz-file-format.txt
 * to understand the container format.
 */

#define STREAM_HEADER_SIZE 12

#define HEADER_MAGIC "\3757zXZ\0"
#define HEADER_MAGIC_SIZE 6

#define FOOTER_MAGIC "YZ"
#define FOOTER_MAGIC_SIZE 2

/*
 * Variable-length integer can hold a 63-bit unsigned integer, or a special
 * value to indicate that the value is unknown.
 */
typedef uint64_t vli_type;

#define VLI_MAX ((vli_type)-1 / 2)
#define VLI_UNKNOWN ((vli_type)-1)

/* Maximum encoded size of a VLI */
#define VLI_BYTES_MAX (sizeof(vli_type) * 8 / 7)

#endif
