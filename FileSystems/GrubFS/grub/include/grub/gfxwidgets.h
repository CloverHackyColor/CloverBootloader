/* gfxwidgets.h - Widgets for the graphical menu (gfxmenu).  */
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

#ifndef GRUB_GFXWIDGETS_HEADER
#define GRUB_GFXWIDGETS_HEADER 1

#include <grub/video.h>

typedef struct grub_gfxmenu_box *grub_gfxmenu_box_t;

struct grub_gfxmenu_box
{
  /* The size of the content.  */
  int content_width;
  int content_height;

  struct grub_video_bitmap **raw_pixmaps;
  struct grub_video_bitmap **scaled_pixmaps;

  void (*draw) (grub_gfxmenu_box_t self, int x, int y);
  void (*set_content_size) (grub_gfxmenu_box_t self,
                            int width, int height);
  int (*get_border_width) (grub_gfxmenu_box_t self);
  int (*get_left_pad) (grub_gfxmenu_box_t self);
  int (*get_top_pad) (grub_gfxmenu_box_t self);
  int (*get_right_pad) (grub_gfxmenu_box_t self);
  int (*get_bottom_pad) (grub_gfxmenu_box_t self);
  void (*destroy) (grub_gfxmenu_box_t self);
};

grub_gfxmenu_box_t grub_gfxmenu_create_box (const char *pixmaps_prefix,
                                            const char *pixmaps_suffix);

#endif /* ! GRUB_GFXWIDGETS_HEADER */
