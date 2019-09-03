/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2005,2006,2007,2008  Free Software Foundation, Inc.
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

#ifndef GRUB_VIDEO_FB_HEADER
#define GRUB_VIDEO_FB_HEADER	1

#include <grub/symbol.h>
#include <grub/types.h>
#include <grub/err.h>
#include <grub/video.h>

/* FB module internal prototype (should not be used from elsewhere).  */

struct grub_video_fbblit_info;

struct grub_video_fbrender_target;

#define GRUB_VIDEO_FBSTD_NUMCOLORS 16
#define GRUB_VIDEO_FBSTD_EXT_NUMCOLORS 256

extern struct grub_video_palette_data EXPORT_VAR(grub_video_fbstd_colors)[GRUB_VIDEO_FBSTD_EXT_NUMCOLORS];

grub_err_t
EXPORT_FUNC(grub_video_fb_init) (void);

grub_err_t
EXPORT_FUNC(grub_video_fb_fini) (void);

grub_err_t
EXPORT_FUNC(grub_video_fb_get_info) (struct grub_video_mode_info *mode_info);

grub_err_t
EXPORT_FUNC(grub_video_fb_get_palette) (unsigned int start, unsigned int count,
					struct grub_video_palette_data *palette_data);
grub_err_t
EXPORT_FUNC(grub_video_fb_set_palette) (unsigned int start, unsigned int count,
					struct grub_video_palette_data *palette_data);
grub_err_t
EXPORT_FUNC(grub_video_fb_set_viewport) (unsigned int x, unsigned int y,
					 unsigned int width, unsigned int height);
grub_err_t
EXPORT_FUNC(grub_video_fb_get_viewport) (unsigned int *x, unsigned int *y,
					 unsigned int *width,
					 unsigned int *height);

grub_err_t
EXPORT_FUNC(grub_video_fb_set_region) (unsigned int x, unsigned int y,
                                       unsigned int width, unsigned int height);
grub_err_t
EXPORT_FUNC(grub_video_fb_get_region) (unsigned int *x, unsigned int *y,
                                       unsigned int *width,
                                       unsigned int *height);

grub_err_t
EXPORT_FUNC(grub_video_fb_set_area_status)
    (grub_video_area_status_t area_status);

grub_err_t
EXPORT_FUNC(grub_video_fb_get_area_status)
    (grub_video_area_status_t *area_status);

grub_video_color_t
EXPORT_FUNC(grub_video_fb_map_color) (grub_uint32_t color_name);

grub_video_color_t
EXPORT_FUNC(grub_video_fb_map_rgb) (grub_uint8_t red, grub_uint8_t green,
				    grub_uint8_t blue);

grub_video_color_t
EXPORT_FUNC(grub_video_fb_map_rgba) (grub_uint8_t red, grub_uint8_t green,
				     grub_uint8_t blue, grub_uint8_t alpha);

grub_err_t
EXPORT_FUNC(grub_video_fb_unmap_color) (grub_video_color_t color,
					grub_uint8_t *red, grub_uint8_t *green,
					grub_uint8_t *blue, grub_uint8_t *alpha);

void
grub_video_fb_unmap_color_int (struct grub_video_fbblit_info * source,
			       grub_video_color_t color,
			       grub_uint8_t *red, grub_uint8_t *green,
			       grub_uint8_t *blue, grub_uint8_t *alpha);

grub_err_t
EXPORT_FUNC(grub_video_fb_fill_rect) (grub_video_color_t color, int x, int y,
				      unsigned int width, unsigned int height);

grub_err_t
EXPORT_FUNC(grub_video_fb_blit_bitmap) (struct grub_video_bitmap *bitmap,
			   enum grub_video_blit_operators oper, int x, int y,
			   int offset_x, int offset_y,
			   unsigned int width, unsigned int height);

grub_err_t
EXPORT_FUNC(grub_video_fb_blit_render_target) (struct grub_video_fbrender_target *source,
				  enum grub_video_blit_operators oper,
				  int x, int y, int offset_x, int offset_y,
				  unsigned int width, unsigned int height);

grub_err_t
EXPORT_FUNC(grub_video_fb_scroll) (grub_video_color_t color, int dx, int dy);

grub_err_t
EXPORT_FUNC(grub_video_fb_create_render_target) (struct grub_video_fbrender_target **result,
				    unsigned int width, unsigned int height,
				    unsigned int mode_type __attribute__ ((unused)));

grub_err_t
EXPORT_FUNC(grub_video_fb_create_render_target_from_pointer) (struct grub_video_fbrender_target **result,
						 const struct grub_video_mode_info *mode_info,
						 void *ptr);

grub_err_t
EXPORT_FUNC(grub_video_fb_delete_render_target) (struct grub_video_fbrender_target *target);

grub_err_t
EXPORT_FUNC(grub_video_fb_get_active_render_target) (struct grub_video_fbrender_target **target);

grub_err_t
EXPORT_FUNC(grub_video_fb_set_active_render_target) (struct grub_video_fbrender_target *target);

typedef grub_err_t (*grub_video_fb_set_page_t) (int page);

grub_err_t
EXPORT_FUNC (grub_video_fb_setup) (unsigned int mode_type, unsigned int mode_mask,
		     struct grub_video_mode_info *mode_info,
		     volatile void *page0_ptr,
		     grub_video_fb_set_page_t set_page_in,
		     volatile void *page1_ptr);
grub_err_t
EXPORT_FUNC (grub_video_fb_swap_buffers) (void);
grub_err_t
EXPORT_FUNC (grub_video_fb_get_info_and_fini) (struct grub_video_mode_info *mode_info,
					       void **framebuf);

#endif /* ! GRUB_VIDEO_FB_HEADER */
