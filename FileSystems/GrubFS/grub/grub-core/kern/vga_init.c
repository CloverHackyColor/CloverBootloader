/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2010,2011  Free Software Foundation, Inc.
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

#ifndef __mips__
#include <grub/pci.h>
#include <grub/mm.h>
#endif
#include <grub/machine/kernel.h>
#include <grub/misc.h>
#include <grub/vga.h>

static struct {grub_uint8_t r, g, b, a; } colors[] =
  {
    // {R, G, B, A}
    {0x00, 0x00, 0x00, 0xFF}, // 0 = black
    {0x00, 0x00, 0xA8, 0xFF}, // 1 = blue
    {0x00, 0xA8, 0x00, 0xFF}, // 2 = green
    {0x00, 0xA8, 0xA8, 0xFF}, // 3 = cyan
    {0xA8, 0x00, 0x00, 0xFF}, // 4 = red
    {0xA8, 0x00, 0xA8, 0xFF}, // 5 = magenta
    {0xA8, 0x54, 0x00, 0xFF}, // 6 = brown
    {0xA8, 0xA8, 0xA8, 0xFF}, // 7 = light gray

    {0x54, 0x54, 0x54, 0xFF}, // 8 = dark gray
    {0x54, 0x54, 0xFE, 0xFF}, // 9 = bright blue
    {0x54, 0xFE, 0x54, 0xFF}, // 10 = bright green
    {0x54, 0xFE, 0xFE, 0xFF}, // 11 = bright cyan
    {0xFE, 0x54, 0x54, 0xFF}, // 12 = bright red
    {0xFE, 0x54, 0xFE, 0xFF}, // 13 = bright magenta
    {0xFE, 0xFE, 0x54, 0xFF}, // 14 = yellow
    {0xFE, 0xFE, 0xFE, 0xFF}  // 15 = white
  };

#include <ascii.h>

#ifdef __mips__
#define VGA_ADDR 0xb00a0000
#else
#define VGA_ADDR 0xa0000
#endif

static void
load_font (void)
{
  unsigned i;

  grub_vga_gr_write (0 << 2, GRUB_VGA_GR_GR6);

  grub_vga_sr_write (GRUB_VGA_SR_MEMORY_MODE_NORMAL, GRUB_VGA_SR_MEMORY_MODE);
  grub_vga_sr_write (1 << GRUB_VGA_TEXT_FONT_PLANE,
		     GRUB_VGA_SR_MAP_MASK_REGISTER);

  grub_vga_gr_write (0, GRUB_VGA_GR_DATA_ROTATE);
  grub_vga_gr_write (0, GRUB_VGA_GR_MODE);
  grub_vga_gr_write (0xff, GRUB_VGA_GR_BITMASK);

  for (i = 0; i < 128; i++)
    grub_memcpy ((void *) (VGA_ADDR + 32 * i), ascii_bitmaps + 16 * i, 16);
}

static void
load_palette (void)
{
  unsigned i;
  for (i = 0; i < 16; i++)
    grub_vga_write_arx (i, i);

  for (i = 0; i < ARRAY_SIZE (colors); i++)
    grub_vga_palette_write (i, colors[i].r, colors[i].g, colors[i].b);
}

void
grub_qemu_init_cirrus (void)
{
  grub_outb (GRUB_VGA_IO_MISC_COLOR,
	     GRUB_MACHINE_PCI_IO_BASE + GRUB_VGA_IO_MISC_WRITE);

  load_font ();

  grub_vga_gr_write (GRUB_VGA_GR_GR6_MMAP_CGA, GRUB_VGA_GR_GR6);
  grub_vga_gr_write (GRUB_VGA_GR_MODE_ODD_EVEN, GRUB_VGA_GR_MODE);

  grub_vga_sr_write (GRUB_VGA_SR_MEMORY_MODE_NORMAL, GRUB_VGA_SR_MEMORY_MODE);

  grub_vga_sr_write ((1 << GRUB_VGA_TEXT_TEXT_PLANE)
		     | (1 << GRUB_VGA_TEXT_ATTR_PLANE),
		     GRUB_VGA_SR_MAP_MASK_REGISTER);

  grub_vga_cr_write (15, GRUB_VGA_CR_CELL_HEIGHT);
  grub_vga_cr_write (79, GRUB_VGA_CR_HORIZ_END);
  grub_vga_cr_write (40, GRUB_VGA_CR_PITCH);

  int vert = 25 * 16;
  grub_vga_cr_write (vert & 0xff, GRUB_VGA_CR_VDISPLAY_END);
  grub_vga_cr_write (((vert >> GRUB_VGA_CR_OVERFLOW_HEIGHT1_SHIFT)
		      & GRUB_VGA_CR_OVERFLOW_HEIGHT1_MASK)
		     | ((vert >> GRUB_VGA_CR_OVERFLOW_HEIGHT2_SHIFT)
			& GRUB_VGA_CR_OVERFLOW_HEIGHT2_MASK),
		     GRUB_VGA_CR_OVERFLOW);

  load_palette ();

  grub_vga_write_arx (GRUB_VGA_ARX_MODE_TEXT, GRUB_VGA_ARX_MODE);
  grub_vga_write_arx (0, GRUB_VGA_ARX_COLOR_SELECT);

  grub_vga_sr_write (GRUB_VGA_SR_CLOCKING_MODE_8_DOT_CLOCK,
		     GRUB_VGA_SR_CLOCKING_MODE);

  grub_vga_cr_write (14, GRUB_VGA_CR_CURSOR_START);
  grub_vga_cr_write (15, GRUB_VGA_CR_CURSOR_END);

  grub_outb (0x20, GRUB_MACHINE_PCI_IO_BASE + GRUB_VGA_IO_ARX);
}
