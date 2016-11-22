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

#ifndef GRUB_GFXTERM_HEADER
#define GRUB_GFXTERM_HEADER	1

#include <grub/err.h>
#include <grub/types.h>
#include <grub/term.h>
#include <grub/video.h>
#include <grub/font.h>

grub_err_t
EXPORT_FUNC (grub_gfxterm_set_window) (struct grub_video_render_target *target,
				       int x, int y, int width, int height,
				       int double_repaint,
				       grub_font_t font, int border_width);

void EXPORT_FUNC (grub_gfxterm_schedule_repaint) (void);

extern void (*EXPORT_VAR (grub_gfxterm_decorator_hook)) (void);

struct grub_gfxterm_background
{
  struct grub_video_bitmap *bitmap;
  int blend_text_bg;
  grub_video_rgba_color_t default_bg_color;
};

extern struct grub_gfxterm_background EXPORT_VAR (grub_gfxterm_background);

void EXPORT_FUNC (grub_gfxterm_video_update_color) (void);
void
EXPORT_FUNC (grub_gfxterm_get_dimensions) (unsigned *width, unsigned *height);

#endif /* ! GRUB_GFXTERM_HEADER */
