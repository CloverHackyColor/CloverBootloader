/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2010 Free Software Foundation, Inc.
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

#ifndef GRUB_TEST_HEADER
#define GRUB_TEST_HEADER

#include <grub/dl.h>
#include <grub/list.h>
#include <grub/misc.h>
#include <grub/types.h>
#include <grub/symbol.h>

#include <grub/video.h>
#include <grub/video_fb.h>

#ifdef __cplusplus
extern "C" {
#endif

struct grub_test
{
  /* The next test.  */
  struct grub_test *next;
  struct grub_test **prev;

  /* The test name.  */
  char *name;

  /* The test main function.  */
  void (*main) (void);
};
typedef struct grub_test *grub_test_t;

extern grub_test_t grub_test_list;

void grub_test_register   (const char *name, void (*test) (void));
void grub_test_unregister (const char *name);

/* Execute a test and print results.  */
int grub_test_run (grub_test_t test);

/* Test `cond' for nonzero; log failure otherwise.  */
void grub_test_nonzero (int cond, const char *file,
			const char *func, grub_uint32_t line,
			const char *fmt, ...)
  __attribute__ ((format (GNU_PRINTF, 5, 6)));

/* Macro to fill in location details and an optional error message.  */
void grub_test_assert_helper (int cond, const char *file,
                            const char *func, grub_uint32_t line,
                            const char *condstr, const char *fmt, ...)
  __attribute__ ((format (GNU_PRINTF, 6, 7)));

#define grub_test_assert(cond, ...)				\
  grub_test_assert_helper(cond, GRUB_FILE, __FUNCTION__, __LINE__,     \
                         #cond, ## __VA_ARGS__);

void grub_unit_test_init (void);
void grub_unit_test_fini (void);

/* Macro to define a unit test.  */
#define GRUB_UNIT_TEST(name, funp)		\
  void grub_unit_test_init (void)		\
  {						\
    grub_test_register (name, funp);		\
  }						\
						\
  void grub_unit_test_fini (void)		\
  {						\
    grub_test_unregister (name);		\
  }

/* Macro to define a functional test.  */
#define GRUB_FUNCTIONAL_TEST(name, funp)	\
  GRUB_MOD_INIT(name)				\
  {						\
    grub_test_register (#name, funp);		\
  }						\
						\
  GRUB_MOD_FINI(name)				\
  {						\
    grub_test_unregister (#name);		\
  }

void
grub_video_checksum (const char *basename_in);
void
grub_video_checksum_end (void);
void
grub_terminal_input_fake_sequence (int *seq_in, int nseq_in);
void
grub_terminal_input_fake_sequence_end (void);
const char *
grub_video_checksum_get_modename (void);


#define GRUB_TEST_VIDEO_SMALL_N_MODES 7
#define GRUB_TEST_VIDEO_ALL_N_MODES 31

extern struct grub_video_mode_info grub_test_video_modes[GRUB_TEST_VIDEO_ALL_N_MODES];

int
grub_test_use_gfxterm (void);
void grub_test_use_gfxterm_end (void);

#ifdef __cplusplus
}
#endif

#endif /* ! GRUB_TEST_HEADER */
