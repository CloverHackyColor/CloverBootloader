/* bitmap_scale.h - Bitmap scaling functions. */
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

#ifndef GRUB_BITMAP_SCALE_HEADER
#define GRUB_BITMAP_SCALE_HEADER 1

#include <grub/err.h>
#include <grub/types.h>
#include <grub/bitmap_scale.h>

enum grub_video_bitmap_scale_method
{
  /* Choose the fastest interpolation algorithm.  */
  GRUB_VIDEO_BITMAP_SCALE_METHOD_FASTEST,
  /* Choose the highest quality interpolation algorithm.  */
  GRUB_VIDEO_BITMAP_SCALE_METHOD_BEST,

  /* Specific algorithms:  */
  /* Nearest neighbor interpolation.  */
  GRUB_VIDEO_BITMAP_SCALE_METHOD_NEAREST,
  /* Bilinear interpolation.  */
  GRUB_VIDEO_BITMAP_SCALE_METHOD_BILINEAR
};

typedef enum grub_video_bitmap_selection_method
{
  GRUB_VIDEO_BITMAP_SELECTION_METHOD_STRETCH,
  GRUB_VIDEO_BITMAP_SELECTION_METHOD_CROP,
  GRUB_VIDEO_BITMAP_SELECTION_METHOD_PADDING,
  GRUB_VIDEO_BITMAP_SELECTION_METHOD_FITWIDTH,
  GRUB_VIDEO_BITMAP_SELECTION_METHOD_FITHEIGHT
} grub_video_bitmap_selection_method_t;

typedef enum grub_video_bitmap_v_align
{
  GRUB_VIDEO_BITMAP_V_ALIGN_TOP,
  GRUB_VIDEO_BITMAP_V_ALIGN_CENTER,
  GRUB_VIDEO_BITMAP_V_ALIGN_BOTTOM
} grub_video_bitmap_v_align_t;

typedef enum grub_video_bitmap_h_align
{
  GRUB_VIDEO_BITMAP_H_ALIGN_LEFT,
  GRUB_VIDEO_BITMAP_H_ALIGN_CENTER,
  GRUB_VIDEO_BITMAP_H_ALIGN_RIGHT
} grub_video_bitmap_h_align_t;

grub_err_t
EXPORT_FUNC (grub_video_bitmap_create_scaled) (struct grub_video_bitmap **dst,
					       int dst_width, int dst_height,
					       struct grub_video_bitmap *src,
					       enum 
					       grub_video_bitmap_scale_method
					       scale_method);

grub_err_t
EXPORT_FUNC (grub_video_bitmap_scale_proportional)
                                     (struct grub_video_bitmap **dst,
                                      int dst_width, int dst_height,
                                      struct grub_video_bitmap *src,
                                      enum grub_video_bitmap_scale_method
                                      scale_method,
                                      grub_video_bitmap_selection_method_t
                                      selection_method,
                                      grub_video_bitmap_v_align_t v_align,
                                      grub_video_bitmap_h_align_t h_align);


#endif /* ! GRUB_BITMAP_SCALE_HEADER */
