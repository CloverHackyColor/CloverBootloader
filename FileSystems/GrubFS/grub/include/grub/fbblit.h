/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2006,2007,2008  Free Software Foundation, Inc.
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

#ifndef GRUB_FBBLIT_HEADER
#define GRUB_FBBLIT_HEADER	1

/* NOTE: This header is private header for fb driver and should not be used
   in other parts of the code.  */

struct grub_video_fbblit_info;

/* NOTE: This function assumes that given coordinates are within bounds of
   handled data.  */
void
grub_video_fb_dispatch_blit (struct grub_video_fbblit_info *target,
			     struct grub_video_fbblit_info *source,
			     enum grub_video_blit_operators oper,
			     int x, int y,
			     unsigned int width, unsigned int height,
			     int offset_x, int offset_y);
#endif /* ! GRUB_FBBLIT_HEADER */
