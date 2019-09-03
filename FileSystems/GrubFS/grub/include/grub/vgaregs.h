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

#ifndef GRUB_VGAREGS_HEADER
#define GRUB_VGAREGS_HEADER	1

#ifdef ASM_FILE
#define GRUB_VGA_IO_SR_INDEX 0x3c4
#define GRUB_VGA_IO_SR_DATA 0x3c5
#else

enum
  {
    GRUB_VGA_IO_CR_BW_INDEX = 0x3b4,
    GRUB_VGA_IO_CR_BW_DATA = 0x3b5,
    GRUB_VGA_IO_ARX = 0x3c0,
    GRUB_VGA_IO_ARX_READ = 0x3c1,
    GRUB_VGA_IO_MISC_WRITE = 0x3c2,
    GRUB_VGA_IO_SR_INDEX = 0x3c4,
    GRUB_VGA_IO_SR_DATA = 0x3c5,
    GRUB_VGA_IO_PIXEL_MASK = 0x3c6,
    GRUB_VGA_IO_PALLETTE_READ_INDEX = 0x3c7,
    GRUB_VGA_IO_PALLETTE_WRITE_INDEX = 0x3c8,
    GRUB_VGA_IO_PALLETTE_DATA = 0x3c9,
    GRUB_VGA_IO_GR_INDEX = 0x3ce,
    GRUB_VGA_IO_GR_DATA = 0x3cf,
    GRUB_VGA_IO_CR_INDEX = 0x3d4,
    GRUB_VGA_IO_CR_DATA = 0x3d5,
    GRUB_VGA_IO_INPUT_STATUS1_REGISTER = 0x3da
  };

#define GRUB_VGA_IO_INPUT_STATUS1_VERTR_BIT 0x08

enum
  {
    GRUB_VGA_CR_HTOTAL = 0x00,
    GRUB_VGA_CR_HORIZ_END = 0x01,
    GRUB_VGA_CR_HBLANK_START = 0x02,    
    GRUB_VGA_CR_HBLANK_END = 0x03,    
    GRUB_VGA_CR_HORIZ_SYNC_PULSE_START = 0x04,
    GRUB_VGA_CR_HORIZ_SYNC_PULSE_END = 0x05,
    GRUB_VGA_CR_VERT_TOTAL = 0x06,
    GRUB_VGA_CR_OVERFLOW = 0x07,
    GRUB_VGA_CR_BYTE_PANNING = 0x08,
    GRUB_VGA_CR_CELL_HEIGHT = 0x09,
    GRUB_VGA_CR_CURSOR_START = 0x0a,
    GRUB_VGA_CR_CURSOR_END = 0x0b,
    GRUB_VGA_CR_START_ADDR_HIGH_REGISTER = 0x0c,
    GRUB_VGA_CR_START_ADDR_LOW_REGISTER = 0x0d,
    GRUB_VGA_CR_CURSOR_ADDR_HIGH = 0x0e,
    GRUB_VGA_CR_CURSOR_ADDR_LOW = 0x0f,
    GRUB_VGA_CR_VSYNC_START = 0x10,
    GRUB_VGA_CR_VSYNC_END = 0x11,
    GRUB_VGA_CR_VDISPLAY_END = 0x12,
    GRUB_VGA_CR_PITCH = 0x13,
    GRUB_VGA_CR_UNDERLINE_LOCATION = 0x14,
    GRUB_VGA_CR_VERTICAL_BLANK_START = 0x15,
    GRUB_VGA_CR_VERTICAL_BLANK_END = 0x16,
    GRUB_VGA_CR_MODE = 0x17,
    GRUB_VGA_CR_LINE_COMPARE = 0x18,
  };

enum
  {
    GRUB_VGA_CR_BYTE_PANNING_NORMAL = 0
  };

enum
  {
    GRUB_VGA_CR_UNDERLINE_LOCATION_DWORD_MODE = 0x40
  };

enum
  {
    GRUB_VGA_IO_MISC_COLOR = 0x01,
    GRUB_VGA_IO_MISC_ENABLE_VRAM_ACCESS = 0x02,
    GRUB_VGA_IO_MISC_28MHZ = 0x04,
    GRUB_VGA_IO_MISC_EXTERNAL_CLOCK_0 = 0x08,
    GRUB_VGA_IO_MISC_UPPER_64K = 0x20,
    GRUB_VGA_IO_MISC_NEGATIVE_HORIZ_POLARITY = 0x40,
    GRUB_VGA_IO_MISC_NEGATIVE_VERT_POLARITY = 0x80,
  };

enum
  {
    GRUB_VGA_ARX_MODE = 0x10,
    GRUB_VGA_ARX_OVERSCAN = 0x11,
    GRUB_VGA_ARX_COLOR_PLANE_ENABLE = 0x12,
    GRUB_VGA_ARX_HORIZONTAL_PANNING = 0x13,
    GRUB_VGA_ARX_COLOR_SELECT = 0x14
  };

enum
  {
    GRUB_VGA_ARX_MODE_TEXT = 0x00,
    GRUB_VGA_ARX_MODE_GRAPHICS = 0x01,
    GRUB_VGA_ARX_MODE_ENABLE_256COLOR = 0x40
  };

#define GRUB_VGA_CR_WIDTH_DIVISOR 8

#define GRUB_VGA_CR_OVERFLOW_VERT_DISPLAY_ENABLE_END1_SHIFT 7
#define GRUB_VGA_CR_OVERFLOW_VERT_DISPLAY_ENABLE_END1_MASK 0x02
#define GRUB_VGA_CR_OVERFLOW_VERT_DISPLAY_ENABLE_END2_SHIFT 3
#define GRUB_VGA_CR_OVERFLOW_VERT_DISPLAY_ENABLE_END2_MASK 0x40

#define GRUB_VGA_CR_OVERFLOW_VERT_TOTAL1_SHIFT 8
#define GRUB_VGA_CR_OVERFLOW_VERT_TOTAL1_MASK 0x01
#define GRUB_VGA_CR_OVERFLOW_VERT_TOTAL2_SHIFT 4
#define GRUB_VGA_CR_OVERFLOW_VERT_TOTAL2_MASK 0x20

#define GRUB_VGA_CR_OVERFLOW_VSYNC_START1_SHIFT 6
#define GRUB_VGA_CR_OVERFLOW_VSYNC_START1_MASK 0x04
#define GRUB_VGA_CR_OVERFLOW_VSYNC_START2_SHIFT 2
#define GRUB_VGA_CR_OVERFLOW_VSYNC_START2_MASK 0x80

#define GRUB_VGA_CR_OVERFLOW_HEIGHT1_SHIFT 7
#define GRUB_VGA_CR_OVERFLOW_HEIGHT1_MASK 0x02
#define GRUB_VGA_CR_OVERFLOW_HEIGHT2_SHIFT 3
#define GRUB_VGA_CR_OVERFLOW_HEIGHT2_MASK 0xc0
#define GRUB_VGA_CR_OVERFLOW_LINE_COMPARE_SHIFT 4
#define GRUB_VGA_CR_OVERFLOW_LINE_COMPARE_MASK 0x10

#define GRUB_VGA_CR_CELL_HEIGHT_LINE_COMPARE_MASK 0x40
#define GRUB_VGA_CR_CELL_HEIGHT_LINE_COMPARE_SHIFT 3
#define GRUB_VGA_CR_CELL_HEIGHT_VERTICAL_BLANK_MASK 0x20
#define GRUB_VGA_CR_CELL_HEIGHT_VERTICAL_BLANK_SHIFT 4
#define GRUB_VGA_CR_CELL_HEIGHT_DOUBLE_SCAN 0x80
enum
  {
    GRUB_VGA_CR_CURSOR_START_DISABLE = (1 << 5)
  };

#define GRUB_VGA_CR_PITCH_DIVISOR 8

enum
  {
    GRUB_VGA_CR_MODE_NO_CGA = 0x01,
    GRUB_VGA_CR_MODE_NO_HERCULES = 0x02,
    GRUB_VGA_CR_MODE_ADDRESS_WRAP = 0x20,
    GRUB_VGA_CR_MODE_BYTE_MODE = 0x40,
    GRUB_VGA_CR_MODE_TIMING_ENABLE = 0x80
  };

enum
  {
    GRUB_VGA_SR_RESET = 0,
    GRUB_VGA_SR_CLOCKING_MODE = 1,
    GRUB_VGA_SR_MAP_MASK_REGISTER = 2,
    GRUB_VGA_SR_CHAR_MAP_SELECT = 3,
    GRUB_VGA_SR_MEMORY_MODE = 4,
  };

enum
  {
    GRUB_VGA_SR_RESET_ASYNC = 1,
    GRUB_VGA_SR_RESET_SYNC = 2
  };

enum
  {
    GRUB_VGA_SR_CLOCKING_MODE_8_DOT_CLOCK = 1
  };

enum
  {
    GRUB_VGA_SR_MEMORY_MODE_NORMAL = 0,
    GRUB_VGA_SR_MEMORY_MODE_EXTERNAL_VIDEO_MEMORY = 2,
    GRUB_VGA_SR_MEMORY_MODE_SEQUENTIAL_ADDRESSING = 4,
    GRUB_VGA_SR_MEMORY_MODE_CHAIN4 = 8,
  };

enum
  {
    GRUB_VGA_GR_SET_RESET_PLANE = 0,
    GRUB_VGA_GR_SET_RESET_PLANE_ENABLE = 1,
    GRUB_VGA_GR_COLOR_COMPARE = 2,
    GRUB_VGA_GR_DATA_ROTATE = 3,
    GRUB_VGA_GR_READ_MAP_REGISTER = 4,
    GRUB_VGA_GR_MODE = 5,
    GRUB_VGA_GR_GR6 = 6,
    GRUB_VGA_GR_COLOR_COMPARE_DISABLE = 7,
    GRUB_VGA_GR_BITMASK = 8,
    GRUB_VGA_GR_MAX
  };

#define GRUB_VGA_ALL_PLANES 0xf
#define GRUB_VGA_NO_PLANES 0x0

enum
  {
    GRUB_VGA_GR_DATA_ROTATE_NOP = 0
  };

enum
  {
    GRUB_VGA_TEXT_TEXT_PLANE = 0,
    GRUB_VGA_TEXT_ATTR_PLANE = 1,
    GRUB_VGA_TEXT_FONT_PLANE = 2
  };

enum
  {
    GRUB_VGA_GR_GR6_GRAPHICS_MODE = 1,
    GRUB_VGA_GR_GR6_MMAP_A0 = (1 << 2),
    GRUB_VGA_GR_GR6_MMAP_CGA = (3 << 2)
  };

enum
  {
    GRUB_VGA_GR_MODE_READ_MODE1 = 0x08,
    GRUB_VGA_GR_MODE_ODD_EVEN = 0x10,
    GRUB_VGA_GR_MODE_ODD_EVEN_SHIFT = 0x20,
    GRUB_VGA_GR_MODE_256_COLOR = 0x40
  };

struct grub_video_hw_config
{
  unsigned vertical_total;
  unsigned vertical_blank_start;
  unsigned vertical_blank_end;
  unsigned vertical_sync_start;
  unsigned vertical_sync_end;
  unsigned line_compare;
  unsigned vdisplay_end;
  unsigned pitch;
  unsigned horizontal_total;
  unsigned horizontal_blank_start;
  unsigned horizontal_blank_end;
  unsigned horizontal_sync_pulse_start;
  unsigned horizontal_sync_pulse_end;
  unsigned horizontal_end;
};

static inline void
grub_vga_set_geometry (struct grub_video_hw_config *config,
		       void (*cr_write) (grub_uint8_t val, grub_uint8_t addr))
{
  unsigned vertical_total = config->vertical_total - 2;
  unsigned vertical_blank_start = config->vertical_blank_start - 1;
  unsigned vdisplay_end = config->vdisplay_end - 1;
  grub_uint8_t overflow, cell_height_reg;

  /* Disable CR0-7 write protection.  */
  cr_write (0, GRUB_VGA_CR_VSYNC_END);

  overflow = ((vertical_total >> GRUB_VGA_CR_OVERFLOW_VERT_TOTAL1_SHIFT)
	      & GRUB_VGA_CR_OVERFLOW_VERT_TOTAL1_MASK)
    | ((vertical_total >> GRUB_VGA_CR_OVERFLOW_VERT_TOTAL2_SHIFT)
       & GRUB_VGA_CR_OVERFLOW_VERT_TOTAL2_MASK)
    | ((config->vertical_sync_start >> GRUB_VGA_CR_OVERFLOW_VSYNC_START2_SHIFT)
       & GRUB_VGA_CR_OVERFLOW_VSYNC_START2_MASK)
    | ((config->vertical_sync_start >> GRUB_VGA_CR_OVERFLOW_VSYNC_START1_SHIFT)
       & GRUB_VGA_CR_OVERFLOW_VSYNC_START1_MASK)
    | ((vdisplay_end >> GRUB_VGA_CR_OVERFLOW_VERT_DISPLAY_ENABLE_END1_SHIFT)
       & GRUB_VGA_CR_OVERFLOW_VERT_DISPLAY_ENABLE_END1_MASK)
    | ((vdisplay_end >> GRUB_VGA_CR_OVERFLOW_VERT_DISPLAY_ENABLE_END2_SHIFT)
       & GRUB_VGA_CR_OVERFLOW_VERT_DISPLAY_ENABLE_END2_MASK)
    | ((config->vertical_sync_start >> GRUB_VGA_CR_OVERFLOW_VSYNC_START1_SHIFT)
       & GRUB_VGA_CR_OVERFLOW_VSYNC_START1_MASK)
    | ((config->line_compare >> GRUB_VGA_CR_OVERFLOW_LINE_COMPARE_SHIFT)
       & GRUB_VGA_CR_OVERFLOW_LINE_COMPARE_MASK);

  cell_height_reg = ((vertical_blank_start
		      >> GRUB_VGA_CR_CELL_HEIGHT_VERTICAL_BLANK_SHIFT)
		     & GRUB_VGA_CR_CELL_HEIGHT_VERTICAL_BLANK_MASK)
    | ((config->line_compare >> GRUB_VGA_CR_CELL_HEIGHT_LINE_COMPARE_SHIFT)
       & GRUB_VGA_CR_CELL_HEIGHT_LINE_COMPARE_MASK);

  cr_write (config->horizontal_total - 1, GRUB_VGA_CR_HTOTAL);
  cr_write (config->horizontal_end - 1, GRUB_VGA_CR_HORIZ_END);
  cr_write (config->horizontal_blank_start - 1, GRUB_VGA_CR_HBLANK_START);
  cr_write (config->horizontal_blank_end, GRUB_VGA_CR_HBLANK_END);
  cr_write (config->horizontal_sync_pulse_start,
	    GRUB_VGA_CR_HORIZ_SYNC_PULSE_START);
  cr_write (config->horizontal_sync_pulse_end,
	    GRUB_VGA_CR_HORIZ_SYNC_PULSE_END);
  cr_write (vertical_total & 0xff, GRUB_VGA_CR_VERT_TOTAL);
  cr_write (overflow, GRUB_VGA_CR_OVERFLOW);
  cr_write (cell_height_reg, GRUB_VGA_CR_CELL_HEIGHT);
  cr_write (config->vertical_sync_start & 0xff, GRUB_VGA_CR_VSYNC_START);
  cr_write (config->vertical_sync_end & 0x0f, GRUB_VGA_CR_VSYNC_END);
  cr_write (vdisplay_end & 0xff, GRUB_VGA_CR_VDISPLAY_END);
  cr_write (config->pitch & 0xff, GRUB_VGA_CR_PITCH);
  cr_write (vertical_blank_start & 0xff, GRUB_VGA_CR_VERTICAL_BLANK_START);
  cr_write (config->vertical_blank_end & 0xff, GRUB_VGA_CR_VERTICAL_BLANK_END);
  cr_write (config->line_compare & 0xff, GRUB_VGA_CR_LINE_COMPARE);
}

#endif

#endif
