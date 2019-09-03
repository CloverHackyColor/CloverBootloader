/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2006,2007  Free Software Foundation, Inc.
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

/* NOTE: This header is private header for vbe driver and should not be used
   in other parts of the code.  */

#ifndef GRUB_VBEUTIL_MACHINE_HEADER
#define GRUB_VBEUTIL_MACHINE_HEADER	1

#include <grub/types.h>
#include <grub/video.h>

struct grub_video_fbblit_info
{
  struct grub_video_mode_info *mode_info;
  grub_uint8_t *data;
};

/* Don't use for 1-bit bitmaps, addressing needs to be done at the bit level
   and it doesn't make sense, in general, to ask for a pointer
   to a particular pixel's data.  */
static inline void *
grub_video_fb_get_video_ptr (struct grub_video_fbblit_info *source,
              unsigned int x, unsigned int y)
{
  return source->data + y * source->mode_info->pitch + x * source->mode_info->bytes_per_pixel;
}

/* Advance pointer by VAL bytes. If there is no unaligned access available,
   VAL has to be divisible by size of pointed type.
 */
#ifdef GRUB_HAVE_UNALIGNED_ACCESS
#define GRUB_VIDEO_FB_ADVANCE_POINTER(ptr, val) ((ptr) = (typeof (ptr)) ((char *) ptr + val))
#else
#define GRUB_VIDEO_FB_ADVANCE_POINTER(ptr, val) ((ptr) += (val) / sizeof (*(ptr)))
#endif

grub_video_color_t get_pixel (struct grub_video_fbblit_info *source,
                              unsigned int x, unsigned int y);

void set_pixel (struct grub_video_fbblit_info *source,
                unsigned int x, unsigned int y, grub_video_color_t color);

#endif /* ! GRUB_VBEUTIL_MACHINE_HEADER */
