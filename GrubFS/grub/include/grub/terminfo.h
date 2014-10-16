/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2002,2003,2005,2007  Free Software Foundation, Inc.
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

#ifndef GRUB_TERMINFO_HEADER
#define GRUB_TERMINFO_HEADER	1

#include <grub/err.h>
#include <grub/types.h>
#include <grub/term.h>

char *EXPORT_FUNC(grub_terminfo_get_current) (struct grub_term_output *term);
grub_err_t EXPORT_FUNC(grub_terminfo_set_current) (struct grub_term_output *term,
												const char *);

#define GRUB_TERMINFO_READKEY_MAX_LEN 6
struct grub_terminfo_input_state
{
  int input_buf[GRUB_TERMINFO_READKEY_MAX_LEN];
  int npending;
#if defined(__powerpc__) && defined(GRUB_MACHINE_IEEE1275)
  int last_key;
  grub_uint64_t last_key_time;
#endif
  int (*readkey) (struct grub_term_input *term);
};

struct grub_terminfo_output_state
{
  struct grub_term_output *next;

  char *name;

  char *gotoxy;
  char *cls;
  char *reverse_video_on;
  char *reverse_video_off;
  char *cursor_on;
  char *cursor_off;
  char *setcolor;

  struct grub_term_coordinate size;
  struct grub_term_coordinate pos;

  void (*put) (struct grub_term_output *term, const int c);
};

grub_err_t EXPORT_FUNC(grub_terminfo_output_init) (struct grub_term_output *term);
void EXPORT_FUNC(grub_terminfo_gotoxy) (grub_term_output_t term,
					struct grub_term_coordinate pos);
void EXPORT_FUNC(grub_terminfo_cls) (grub_term_output_t term);
struct grub_term_coordinate EXPORT_FUNC (grub_terminfo_getxy) (struct grub_term_output *term);
void EXPORT_FUNC (grub_terminfo_setcursor) (struct grub_term_output *term,
					    const int on);
void EXPORT_FUNC (grub_terminfo_setcolorstate) (struct grub_term_output *term,
				  const grub_term_color_state state);


grub_err_t EXPORT_FUNC (grub_terminfo_input_init) (struct grub_term_input *term);
int EXPORT_FUNC (grub_terminfo_getkey) (struct grub_term_input *term);
void EXPORT_FUNC (grub_terminfo_putchar) (struct grub_term_output *term,
					  const struct grub_unicode_glyph *c);
struct grub_term_coordinate EXPORT_FUNC (grub_terminfo_getwh) (struct grub_term_output *term);


grub_err_t EXPORT_FUNC (grub_terminfo_output_register) (struct grub_term_output *term,
							const char *type);
grub_err_t EXPORT_FUNC (grub_terminfo_output_unregister) (struct grub_term_output *term);

void grub_terminfo_init (void);
void grub_terminfo_fini (void);

#endif /* ! GRUB_TERMINFO_HEADER */
