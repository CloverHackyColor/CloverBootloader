/* progress.c - show loading progress */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2013  Free Software Foundation, Inc.
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

#include <grub/types.h>
#include <grub/time.h>
#include <grub/term.h>
#include <grub/dl.h>
#include <grub/misc.h>
#include <grub/normal.h>

GRUB_MOD_LICENSE ("GPLv3+");

#define UPDATE_INTERVAL 800

static void
grub_file_progress_hook_real (grub_disk_addr_t sector __attribute__ ((unused)),
                              unsigned offset __attribute__ ((unused)),
                              unsigned length, void *data)
{
  static int call_depth = 0;
  grub_uint64_t now;
  static grub_uint64_t last_progress_update_time;
  grub_file_t file = data;
  file->progress_offset += length;

  if (call_depth)
    return;

  call_depth = 1;
  now = grub_get_time_ms ();

  if (((now - last_progress_update_time > UPDATE_INTERVAL) &&
       (file->progress_offset - file->offset > 0)) ||
      (file->progress_offset == file->size))
    {
      static char buffer[80];
      struct grub_term_output *term;
      const char *partial_file_name;

      unsigned long long percent;
      grub_uint64_t current_speed;

      if (now - file->last_progress_time < 10)
	current_speed = 0;
      else
	current_speed = grub_divmod64 ((file->progress_offset
					- file->last_progress_offset)
				       * 100ULL * 1000ULL,
				       now - file->last_progress_time, 0);

      if (file->size == 0)
	percent = 100;
      else
	percent = grub_divmod64 (100 * file->progress_offset,
				 file->size, 0);

      partial_file_name = grub_strrchr (file->name, '/');
      if (partial_file_name)
	partial_file_name++;
      else
	partial_file_name = "";

      file->estimated_speed = (file->estimated_speed + current_speed) >> 1;

      grub_snprintf (buffer, sizeof (buffer), "      [ %.20s  %s  %llu%%  ",
                     partial_file_name,
                     grub_get_human_size (file->progress_offset,
                                          GRUB_HUMAN_SIZE_NORMAL),
                     (unsigned long long) percent);

      char *ptr = buffer + grub_strlen (buffer);
      grub_snprintf (ptr, sizeof (buffer) - (ptr - buffer), "%s ]",
                     grub_get_human_size (file->estimated_speed,
                                          GRUB_HUMAN_SIZE_SPEED));

      grub_size_t len = grub_strlen (buffer);
      FOR_ACTIVE_TERM_OUTPUTS (term)
        {
          if (term->progress_update_counter++ > term->progress_update_divisor
	      || (file->progress_offset == file->size
		  && term->progress_update_divisor
		  != (unsigned) GRUB_PROGRESS_NO_UPDATE))
            {
              struct grub_term_coordinate old_pos = grub_term_getxy (term);
              struct grub_term_coordinate new_pos = old_pos;
              new_pos.x = grub_term_width (term) - len - 1;

              grub_term_gotoxy (term, new_pos);
	      grub_puts_terminal (buffer, term);
              grub_term_gotoxy (term, old_pos);

              term->progress_update_counter = 0;

	      if (term->refresh)
		term->refresh (term);
            }
        }

      file->last_progress_offset = file->progress_offset;
      file->last_progress_time = now;
      last_progress_update_time = now;
    }
  call_depth = 0;
}

GRUB_MOD_INIT(progress)
{
  grub_file_progress_hook = grub_file_progress_hook_real;
}

GRUB_MOD_FINI(progress)
{
  grub_file_progress_hook = 0;
}
