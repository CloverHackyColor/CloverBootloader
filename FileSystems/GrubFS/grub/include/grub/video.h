/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2006,2007,2008,2009  Free Software Foundation, Inc.
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

#ifndef GRUB_VIDEO_HEADER
#define GRUB_VIDEO_HEADER	1

#include <grub/err.h>
#include <grub/types.h>
#include <grub/list.h>

/* Video color in hardware dependent format.  Users should not assume any
   specific coding format.  */
typedef grub_uint32_t grub_video_color_t;

/* Video color in hardware independent format.  */
typedef struct grub_video_rgba_color
{
  grub_uint8_t red;
  grub_uint8_t green;
  grub_uint8_t blue;
  grub_uint8_t alpha;
} grub_video_rgba_color_t;

/* This structure is driver specific and should not be accessed directly by
   outside code.  */
struct grub_video_render_target;

/* Forward declarations for used data structures.  */
struct grub_video_bitmap;

/* Defines used to describe video mode or rendering target.  */
/* If following is set render target contains currenly displayed image
   after swapping buffers (otherwise it contains previously displayed image).
 */
typedef enum grub_video_mode_type
  {
    GRUB_VIDEO_MODE_TYPE_RGB              = 0x00000001,
    GRUB_VIDEO_MODE_TYPE_INDEX_COLOR      = 0x00000002,
    GRUB_VIDEO_MODE_TYPE_1BIT_BITMAP      = 0x00000004,
    GRUB_VIDEO_MODE_TYPE_YUV              = 0x00000008,

    /* Defines used to mask flags.  */
    GRUB_VIDEO_MODE_TYPE_COLOR_MASK       = 0x0000000F,

    GRUB_VIDEO_MODE_TYPE_DOUBLE_BUFFERED  = 0x00000010,
    GRUB_VIDEO_MODE_TYPE_ALPHA            = 0x00000020,
    GRUB_VIDEO_MODE_TYPE_PURE_TEXT        = 0x00000040,
    GRUB_VIDEO_MODE_TYPE_UPDATING_SWAP    = 0x00000080,
    GRUB_VIDEO_MODE_TYPE_OPERATIONAL_MASK = 0x000000F0,

    /* Defines used to specify requested bit depth.  */
    GRUB_VIDEO_MODE_TYPE_DEPTH_MASK       = 0x0000FF00,
#define GRUB_VIDEO_MODE_TYPE_DEPTH_POS	  8

    GRUB_VIDEO_MODE_TYPE_UNKNOWN          = 0x00010000,
    GRUB_VIDEO_MODE_TYPE_HERCULES         = 0x00020000,
    GRUB_VIDEO_MODE_TYPE_PLANAR           = 0x00040000,
    GRUB_VIDEO_MODE_TYPE_NONCHAIN4        = 0x00080000,
    GRUB_VIDEO_MODE_TYPE_CGA              = 0x00100000,
    GRUB_VIDEO_MODE_TYPE_INFO_MASK        = 0x00FF0000,
  } grub_video_mode_type_t;

/* The basic render target representing the whole display.  This always
   renders to the back buffer when double-buffering is in use.  */
#define GRUB_VIDEO_RENDER_TARGET_DISPLAY \
  ((struct grub_video_render_target *) 0)

/* Defined blitting formats.  */
enum grub_video_blit_format
  {
    /* Generic RGBA, use fields & masks.  */
    GRUB_VIDEO_BLIT_FORMAT_RGBA,

    /* Optimized RGBA's.  */
    GRUB_VIDEO_BLIT_FORMAT_RGBA_8888,
    GRUB_VIDEO_BLIT_FORMAT_BGRA_8888,

    /* Generic RGB, use fields & masks.  */
    GRUB_VIDEO_BLIT_FORMAT_RGB,

    /* Optimized RGB's.  */
    GRUB_VIDEO_BLIT_FORMAT_RGB_888,
    GRUB_VIDEO_BLIT_FORMAT_BGR_888,
    GRUB_VIDEO_BLIT_FORMAT_RGB_565,
    GRUB_VIDEO_BLIT_FORMAT_BGR_565,

    /* When needed, decode color or just use value as is.  */
    GRUB_VIDEO_BLIT_FORMAT_INDEXCOLOR,
    /* Like index but only 16-colors and F0 is a special value for transparency.
       Could be extended to 4 bits of alpha and 4 bits of color if necessary.
       Used internally for text rendering.
     */
    GRUB_VIDEO_BLIT_FORMAT_INDEXCOLOR_ALPHA,

    /* Two color bitmap; bits packed: rows are not padded to byte boundary.  */
    GRUB_VIDEO_BLIT_FORMAT_1BIT_PACKED
  };

/* Define blitting operators.  */
enum grub_video_blit_operators
  {
    /* Replace target bitmap data with source.  */
    GRUB_VIDEO_BLIT_REPLACE,
    /* Blend target and source based on source's alpha value.  */
    GRUB_VIDEO_BLIT_BLEND
  };

struct grub_video_mode_info
{
  /* Width of the screen.  */
  unsigned int width;

  /* Height of the screen.  */
  unsigned int height;

  /* Mode type bitmask.  Contains information like is it Index color or
     RGB mode.  */
  grub_video_mode_type_t mode_type;

  /* Bits per pixel.  */
  unsigned int bpp;

  /* Bytes per pixel.  */
  unsigned int bytes_per_pixel;

  /* Pitch of one scanline.  How many bytes there are for scanline.  */
  unsigned int pitch;

  /* In index color mode, number of colors.  In RGB mode this is 256.  */
  unsigned int number_of_colors;

  unsigned int mode_number;
#define GRUB_VIDEO_MODE_NUMBER_INVALID 0xffffffff

  /* Optimization hint how binary data is coded.  */
  enum grub_video_blit_format blit_format;

  /* How many bits are reserved for red color.  */
  unsigned int red_mask_size;

  /* What is location of red color bits.  In Index Color mode, this is 0.  */
  unsigned int red_field_pos;

  /* How many bits are reserved for green color.  */
  unsigned int green_mask_size;

  /* What is location of green color bits.  In Index Color mode, this is 0.  */
  unsigned int green_field_pos;

  /* How many bits are reserved for blue color.  */
  unsigned int blue_mask_size;

  /* What is location of blue color bits.  In Index Color mode, this is 0.  */
  unsigned int blue_field_pos;

  /* How many bits are reserved in color.  */
  unsigned int reserved_mask_size;

  /* What is location of reserved color bits.  In Index Color mode,
     this is 0.  */
  unsigned int reserved_field_pos;

  /* For 1-bit bitmaps, the background color.  Used for bits = 0.  */
  grub_uint8_t bg_red;
  grub_uint8_t bg_green;
  grub_uint8_t bg_blue;
  grub_uint8_t bg_alpha;

  /* For 1-bit bitmaps, the foreground color.  Used for bits = 1.  */
  grub_uint8_t fg_red;
  grub_uint8_t fg_green;
  grub_uint8_t fg_blue;
  grub_uint8_t fg_alpha;
};

/* A 2D rectangle type.  */
struct grub_video_rect
{
  unsigned x;
  unsigned y;
  unsigned width;
  unsigned height;
};
typedef struct grub_video_rect grub_video_rect_t;

struct grub_video_signed_rect
{
  signed x;
  signed y;
  unsigned width;
  unsigned height;
};
typedef struct grub_video_signed_rect grub_video_signed_rect_t;

struct grub_video_palette_data
{
  grub_uint8_t r; /* Red color value (0-255).  */
  grub_uint8_t g; /* Green color value (0-255).  */
  grub_uint8_t b; /* Blue color value (0-255).  */
  grub_uint8_t a; /* Reserved bits value (0-255).  */
};

struct grub_video_edid_info
{
  grub_uint8_t header[8];
  grub_uint16_t manufacturer_id;
  grub_uint16_t product_id;
  grub_uint32_t serial_number;
  grub_uint8_t week_of_manufacture;
  grub_uint8_t year_of_manufacture;
  grub_uint8_t version;
  grub_uint8_t revision;

  grub_uint8_t video_input_definition;
  grub_uint8_t max_horizontal_image_size;
  grub_uint8_t max_vertical_image_size;
  grub_uint8_t display_gamma;
  grub_uint8_t feature_support;
#define GRUB_VIDEO_EDID_FEATURE_PREFERRED_TIMING_MODE	(1 << 1)

  grub_uint8_t red_green_lo;
  grub_uint8_t blue_white_lo;
  grub_uint8_t red_x_hi;
  grub_uint8_t red_y_hi;
  grub_uint8_t green_x_hi;
  grub_uint8_t green_y_hi;
  grub_uint8_t blue_x_hi;
  grub_uint8_t blue_y_hi;
  grub_uint8_t white_x_hi;
  grub_uint8_t white_y_hi;

  grub_uint8_t established_timings_1;
  grub_uint8_t established_timings_2;
  grub_uint8_t manufacturer_reserved_timings;

  grub_uint16_t standard_timings[8];

  struct {
    grub_uint16_t pixel_clock;
    /* Only valid if the pixel clock is non-null.  */
    grub_uint8_t horizontal_active_lo;
    grub_uint8_t horizontal_blanking_lo;
    grub_uint8_t horizontal_hi;
    grub_uint8_t vertical_active_lo;
    grub_uint8_t vertical_blanking_lo;
    grub_uint8_t vertical_hi;
    grub_uint8_t horizontal_sync_offset_lo;
    grub_uint8_t horizontal_sync_pulse_width_lo;
    grub_uint8_t vertical_sync_lo;
    grub_uint8_t sync_hi;
    grub_uint8_t horizontal_image_size_lo;
    grub_uint8_t vertical_image_size_lo;
    grub_uint8_t image_size_hi;
    grub_uint8_t horizontal_border;
    grub_uint8_t vertical_border;
    grub_uint8_t flags;
  } detailed_timings[4];

  grub_uint8_t extension_flag;
  grub_uint8_t checksum;
} GRUB_PACKED;

typedef enum grub_video_driver_id
  {
    GRUB_VIDEO_DRIVER_NONE,
    GRUB_VIDEO_DRIVER_VBE,
    GRUB_VIDEO_DRIVER_EFI_UGA,
    GRUB_VIDEO_DRIVER_EFI_GOP,
    GRUB_VIDEO_DRIVER_SM712,
    GRUB_VIDEO_DRIVER_VGA,
    GRUB_VIDEO_DRIVER_CIRRUS,
    GRUB_VIDEO_DRIVER_BOCHS,
    GRUB_VIDEO_DRIVER_SDL,
    GRUB_VIDEO_DRIVER_SIS315PRO,
    GRUB_VIDEO_DRIVER_RADEON_FULOONG2E,
    GRUB_VIDEO_DRIVER_COREBOOT,
    GRUB_VIDEO_DRIVER_IEEE1275,
    GRUB_VIDEO_ADAPTER_CAPTURE,
    GRUB_VIDEO_DRIVER_XEN,
    GRUB_VIDEO_DRIVER_RADEON_YEELOONG3A
  } grub_video_driver_id_t;

typedef enum grub_video_adapter_prio
  {
    GRUB_VIDEO_ADAPTER_PRIO_FALLBACK = 60,
    GRUB_VIDEO_ADAPTER_PRIO_FIRMWARE_DIRTY = 70,
    GRUB_VIDEO_ADAPTER_PRIO_FIRMWARE = 80,
    GRUB_VIDEO_ADAPTER_PRIO_NATIVE = 100
  } grub_video_adapter_prio_t;

typedef enum grub_video_area_status
  {
    GRUB_VIDEO_AREA_DISABLED,
    GRUB_VIDEO_AREA_ENABLED
  } grub_video_area_status_t;

struct grub_video_adapter
{
  /* The next video adapter.  */
  struct grub_video_adapter *next;
  struct grub_video_adapter **prev;

  /* The video adapter name.  */
  const char *name;
  grub_video_driver_id_t id;

  grub_video_adapter_prio_t prio;

  /* Initialize the video adapter.  */
  grub_err_t (*init) (void);

  /* Clean up the video adapter.  */
  grub_err_t (*fini) (void);

  grub_err_t (*setup) (unsigned int width,  unsigned int height,
                       grub_video_mode_type_t mode_type,
		       grub_video_mode_type_t mode_mask);

  grub_err_t (*get_info) (struct grub_video_mode_info *mode_info);

  grub_err_t (*get_info_and_fini) (struct grub_video_mode_info *mode_info,
				   void **framebuffer);

  grub_err_t (*set_palette) (unsigned int start, unsigned int count,
                             struct grub_video_palette_data *palette_data);

  grub_err_t (*get_palette) (unsigned int start, unsigned int count,
                             struct grub_video_palette_data *palette_data);

  grub_err_t (*set_viewport) (unsigned int x, unsigned int y,
                              unsigned int width, unsigned int height);

  grub_err_t (*get_viewport) (unsigned int *x, unsigned int *y,
                              unsigned int *width, unsigned int *height);

  grub_err_t (*set_region) (unsigned int x, unsigned int y,
                            unsigned int width, unsigned int height);

  grub_err_t (*get_region) (unsigned int *x, unsigned int *y,
                            unsigned int *width, unsigned int *height);

  grub_err_t (*set_area_status) (grub_video_area_status_t area_status);

  grub_err_t (*get_area_status) (grub_video_area_status_t *area_status);

  grub_video_color_t (*map_color) (grub_uint32_t color_name);

  grub_video_color_t (*map_rgb) (grub_uint8_t red, grub_uint8_t green,
                                 grub_uint8_t blue);

  grub_video_color_t (*map_rgba) (grub_uint8_t red, grub_uint8_t green,
                                  grub_uint8_t blue, grub_uint8_t alpha);

  grub_err_t (*unmap_color) (grub_video_color_t color,
                             grub_uint8_t *red, grub_uint8_t *green,
                             grub_uint8_t *blue, grub_uint8_t *alpha);

  grub_err_t (*fill_rect) (grub_video_color_t color, int x, int y,
                           unsigned int width, unsigned int height);

  grub_err_t (*blit_bitmap) (struct grub_video_bitmap *bitmap,
                             enum grub_video_blit_operators oper,
                             int x, int y, int offset_x, int offset_y,
                             unsigned int width, unsigned int height);

  grub_err_t (*blit_render_target) (struct grub_video_render_target *source,
                                    enum grub_video_blit_operators oper,
                                    int x, int y, int offset_x, int offset_y,
                                    unsigned int width, unsigned int height);

  grub_err_t (*scroll) (grub_video_color_t color, int dx, int dy);

  grub_err_t (*swap_buffers) (void);

  grub_err_t (*create_render_target) (struct grub_video_render_target **result,
                                      unsigned int width, unsigned int height,
                                      unsigned int mode_type);

  grub_err_t (*delete_render_target) (struct grub_video_render_target *target);

  grub_err_t (*set_active_render_target) (struct grub_video_render_target *target);

  grub_err_t (*get_active_render_target) (struct grub_video_render_target **target);

  int (*iterate) (int (*hook) (const struct grub_video_mode_info *info, void *hook_arg), void *hook_arg);

  grub_err_t (*get_edid) (struct grub_video_edid_info *edid_info);

  void (*print_adapter_specific_info) (void);
};
typedef struct grub_video_adapter *grub_video_adapter_t;

extern grub_video_adapter_t EXPORT_VAR(grub_video_adapter_list);

#ifndef GRUB_LST_GENERATOR
/* Register video driver.  */
static inline void
grub_video_register (grub_video_adapter_t adapter)
{
  grub_video_adapter_t *p;
  for (p = &grub_video_adapter_list; *p && (*p)->prio > adapter->prio; 
       p = &((*p)->next));
  adapter->next = *p;
  *p = adapter;

  adapter->prev = p;
  if (adapter->next)
    adapter->next->prev = &adapter->next;
}
#endif

/* Unregister video driver.  */
static inline void
grub_video_unregister (grub_video_adapter_t adapter)
{
  grub_list_remove (GRUB_AS_LIST (adapter));
}

#define FOR_VIDEO_ADAPTERS(var) FOR_LIST_ELEMENTS((var), (grub_video_adapter_list))

grub_err_t EXPORT_FUNC (grub_video_restore) (void);

grub_err_t EXPORT_FUNC (grub_video_get_info) (struct grub_video_mode_info *mode_info);

/* Framebuffer address may change as a part of normal operation
   (e.g. double buffering). That's why you need to stop video subsystem to be
   sure that framebuffer address doesn't change. To ensure this abstraction
   grub_video_get_info_and_fini is the only function supplying framebuffer
   address. */
grub_err_t EXPORT_FUNC (grub_video_get_info_and_fini) (struct grub_video_mode_info *mode_info,
					 void **framebuffer);

enum grub_video_blit_format EXPORT_FUNC(grub_video_get_blit_format) (struct grub_video_mode_info *mode_info);

grub_err_t grub_video_set_palette (unsigned int start, unsigned int count,
                                   struct grub_video_palette_data *palette_data);

grub_err_t EXPORT_FUNC (grub_video_get_palette) (unsigned int start,
						 unsigned int count,
						 struct grub_video_palette_data *palette_data);

grub_err_t EXPORT_FUNC (grub_video_set_viewport) (unsigned int x,
						  unsigned int y,
						  unsigned int width,
						  unsigned int height);

grub_err_t EXPORT_FUNC (grub_video_get_viewport) (unsigned int *x,
						  unsigned int *y,
						  unsigned int *width,
						  unsigned int *height);

grub_err_t EXPORT_FUNC (grub_video_set_region) (unsigned int x,
                                                unsigned int y,
                                                unsigned int width,
                                                unsigned int height);

grub_err_t EXPORT_FUNC (grub_video_get_region) (unsigned int *x,
                                                unsigned int *y,
                                                unsigned int *width,
                                                unsigned int *height);

grub_err_t EXPORT_FUNC (grub_video_set_area_status)
    (grub_video_area_status_t area_status);

grub_err_t EXPORT_FUNC (grub_video_get_area_status)
    (grub_video_area_status_t *area_status);

grub_video_color_t EXPORT_FUNC (grub_video_map_color) (grub_uint32_t color_name);

grub_video_color_t EXPORT_FUNC (grub_video_map_rgb) (grub_uint8_t red,
						     grub_uint8_t green,
						     grub_uint8_t blue);

grub_video_color_t EXPORT_FUNC (grub_video_map_rgba) (grub_uint8_t red,
						      grub_uint8_t green,
						      grub_uint8_t blue,
						      grub_uint8_t alpha);

grub_err_t EXPORT_FUNC (grub_video_unmap_color) (grub_video_color_t color,
						 grub_uint8_t *red,
						 grub_uint8_t *green,
						 grub_uint8_t *blue,
						 grub_uint8_t *alpha);

grub_err_t EXPORT_FUNC (grub_video_fill_rect) (grub_video_color_t color, 
					       int x, int y,
					       unsigned int width,
					       unsigned int height);

grub_err_t EXPORT_FUNC (grub_video_blit_bitmap) (struct grub_video_bitmap *bitmap,
						 enum grub_video_blit_operators oper,
						 int x, int y,
						 int offset_x, int offset_y,
						 unsigned int width,
						 unsigned int height);

grub_err_t EXPORT_FUNC (grub_video_blit_render_target) (struct grub_video_render_target *source,
							enum grub_video_blit_operators oper,
							int x, int y,
							int offset_x,
							int offset_y,
							unsigned int width,
							unsigned int height);

grub_err_t grub_video_scroll (grub_video_color_t color, int dx, int dy);

grub_err_t EXPORT_FUNC (grub_video_swap_buffers) (void);

grub_err_t EXPORT_FUNC (grub_video_create_render_target) (struct grub_video_render_target **result,
							  unsigned int width,
							  unsigned int height,
							  unsigned int mode_type);

grub_err_t EXPORT_FUNC (grub_video_delete_render_target) (struct grub_video_render_target *target);

grub_err_t EXPORT_FUNC (grub_video_set_active_render_target) (struct grub_video_render_target *target);

grub_err_t grub_video_get_active_render_target (struct grub_video_render_target **target);

grub_err_t EXPORT_FUNC (grub_video_edid_checksum) (struct grub_video_edid_info *edid_info);
grub_err_t EXPORT_FUNC (grub_video_edid_preferred_mode) (struct grub_video_edid_info *edid_info,
					   unsigned int *width,
					   unsigned int *height);

grub_err_t EXPORT_FUNC (grub_video_set_mode) (const char *modestring,
					      unsigned int modemask,
					      unsigned int modevalue);

static inline int
grub_video_check_mode_flag (grub_video_mode_type_t flags,
			    grub_video_mode_type_t mask,
			    grub_video_mode_type_t flag, int def)
{
  return (flag & mask) ? !! (flags & flag) : def;
}

grub_video_driver_id_t EXPORT_FUNC (grub_video_get_driver_id) (void);

static __inline grub_video_rgba_color_t
grub_video_rgba_color_rgb (grub_uint8_t r, grub_uint8_t g, grub_uint8_t b)
{
  grub_video_rgba_color_t c;
  c.red = r;
  c.green = g;
  c.blue = b;
  c.alpha = 255;
  return c;
}

static __inline grub_video_color_t
grub_video_map_rgba_color (grub_video_rgba_color_t c)
{
  return grub_video_map_rgba (c.red, c.green, c.blue, c.alpha);
}

#ifndef GRUB_MACHINE_EMU
extern void grub_font_init (void);
extern void grub_font_fini (void);
extern void grub_gfxterm_init (void);
extern void grub_gfxterm_fini (void);
extern void grub_video_sm712_init (void);
extern void grub_video_sm712_fini (void);
extern void grub_video_sis315pro_init (void);
extern void grub_video_radeon_fuloong2e_init (void);
extern void grub_video_sis315pro_fini (void);
extern void grub_video_radeon_fuloong2e_fini (void);
extern void grub_video_radeon_yeeloong3a_init (void);
extern void grub_video_radeon_yeeloong3a_fini (void);
#endif

void
grub_video_set_adapter (grub_video_adapter_t adapter);
grub_video_adapter_t
grub_video_get_adapter (void);
grub_err_t
grub_video_capture_start (const struct grub_video_mode_info *mode_info,
			  struct grub_video_palette_data *palette,
			  unsigned int palette_size);
void
grub_video_capture_end (void);

void *
grub_video_capture_get_framebuffer (void);

extern grub_video_adapter_t EXPORT_VAR (grub_video_adapter_active);
extern void (*grub_video_capture_refresh_cb) (void);

#define GRUB_VIDEO_MI_RGB555(x)						\
  x.mode_type = GRUB_VIDEO_MODE_TYPE_RGB,				\
    x.bpp = 15,								\
    x.bytes_per_pixel = 2,						\
    x.number_of_colors = 256,						\
    x.red_mask_size = 5,						\
    x.red_field_pos = 10,						\
    x.green_mask_size = 5,						\
    x.green_field_pos = 5,						\
    x.blue_mask_size = 5,						\
    x.blue_field_pos = 0

#define GRUB_VIDEO_MI_RGB565(x)						\
  x.mode_type = GRUB_VIDEO_MODE_TYPE_RGB,				\
    x.bpp = 16,								\
    x.bytes_per_pixel = 2,						\
    x.number_of_colors = 256,						\
    x.red_mask_size = 5,						\
    x.red_field_pos = 11,						\
    x.green_mask_size = 6,						\
    x.green_field_pos = 5,						\
    x.blue_mask_size = 5,						\
    x.blue_field_pos = 0

#define GRUB_VIDEO_MI_RGB888(x) \
  x.mode_type = GRUB_VIDEO_MODE_TYPE_RGB,			\
    x.bpp = 24,							\
    x.bytes_per_pixel = 3,					\
    x.number_of_colors = 256,					\
    x.red_mask_size = 8,					\
    x.red_field_pos = 16,					\
    x.green_mask_size = 8,					\
    x.green_field_pos = 8,					\
    x.blue_mask_size = 8,					\
    x.blue_field_pos = 0

#define GRUB_VIDEO_MI_RGBA8888(x) \
  x.mode_type = GRUB_VIDEO_MODE_TYPE_RGB,	\
    x.bpp = 32,					\
    x.bytes_per_pixel = 4,			\
    x.number_of_colors = 256,			\
    x.reserved_mask_size = 8,			\
    x.reserved_field_pos = 24,			\
    x.red_mask_size = 8,			\
    x.red_field_pos = 16,			\
    x.green_mask_size = 8,			\
    x.green_field_pos = 8,			\
    x.blue_mask_size = 8,			\
    x.blue_field_pos = 0


#define GRUB_VIDEO_MI_BGR555(x)						\
  x.mode_type = GRUB_VIDEO_MODE_TYPE_RGB,				\
    x.bpp = 15,								\
    x.bytes_per_pixel = 2,						\
    x.number_of_colors = 256,						\
    x.red_mask_size = 5,						\
    x.red_field_pos = 0,						\
    x.green_mask_size = 5,						\
    x.green_field_pos = 5,						\
    x.blue_mask_size = 5,						\
    x.blue_field_pos = 10

#define GRUB_VIDEO_MI_BGR565(x)						\
  x.mode_type = GRUB_VIDEO_MODE_TYPE_RGB,				\
    x.bpp = 16,								\
    x.bytes_per_pixel = 2,						\
    x.number_of_colors = 256,						\
    x.red_mask_size = 5,						\
    x.red_field_pos = 0,						\
    x.green_mask_size = 6,						\
    x.green_field_pos = 5,						\
    x.blue_mask_size = 5,						\
    x.blue_field_pos = 11

#define GRUB_VIDEO_MI_BGR888(x) \
  x.mode_type = GRUB_VIDEO_MODE_TYPE_RGB,			\
    x.bpp = 24,							\
    x.bytes_per_pixel = 3,					\
    x.number_of_colors = 256,					\
    x.red_mask_size = 8,					\
    x.red_field_pos = 0,					\
    x.green_mask_size = 8,					\
    x.green_field_pos = 8,					\
    x.blue_mask_size = 8,					\
    x.blue_field_pos = 16

#define GRUB_VIDEO_MI_BGRA8888(x) \
  x.mode_type = GRUB_VIDEO_MODE_TYPE_RGB,	\
    x.bpp = 32,					\
    x.bytes_per_pixel = 4,			\
    x.number_of_colors = 256,			\
    x.reserved_mask_size = 8,			\
    x.reserved_field_pos = 24,			\
    x.red_mask_size = 8,			\
    x.red_field_pos = 0,			\
    x.green_mask_size = 8,			\
    x.green_field_pos = 8,			\
    x.blue_mask_size = 8,			\
    x.blue_field_pos = 16

#endif /* ! GRUB_VIDEO_HEADER */
