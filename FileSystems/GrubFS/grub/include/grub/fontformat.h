/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2010 Free Software Foundation, Inc.
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

#ifndef GRUB_FONT_FORMAT_HEADER
#define GRUB_FONT_FORMAT_HEADER	1

/* FONT_FORMAT_PFF2_MAGIC use only 4 relevants bytes and the \0.  */
#define FONT_FORMAT_PFF2_MAGIC "PFF2"
#define FONT_FORMAT_SECTION_NAMES_FILE "FILE"
#define FONT_FORMAT_SECTION_NAMES_FONT_NAME "NAME"
#define FONT_FORMAT_SECTION_NAMES_POINT_SIZE "PTSZ"
#define FONT_FORMAT_SECTION_NAMES_WEIGHT "WEIG"
#define FONT_FORMAT_SECTION_NAMES_MAX_CHAR_WIDTH "MAXW"
#define FONT_FORMAT_SECTION_NAMES_MAX_CHAR_HEIGHT "MAXH"
#define FONT_FORMAT_SECTION_NAMES_ASCENT "ASCE"
#define FONT_FORMAT_SECTION_NAMES_DESCENT "DESC"
#define FONT_FORMAT_SECTION_NAMES_CHAR_INDEX "CHIX"
#define FONT_FORMAT_SECTION_NAMES_DATA "DATA"
#define FONT_FORMAT_SECTION_NAMES_FAMILY "FAMI"
#define FONT_FORMAT_SECTION_NAMES_SLAN "SLAN"

#endif /* ! GRUB_FONT_FORMAT_HEADER */

