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

#ifndef GRUB_EMU_EXEC_H
#define GRUB_EMU_EXEC_H 1

#include <config.h>
#include <stdarg.h>

#include <sys/types.h>
pid_t
grub_util_exec_pipe (const char *const *argv, int *fd);
pid_t
grub_util_exec_pipe_stderr (const char *const *argv, int *fd);

int
grub_util_exec_redirect_all (const char *const *argv, const char *stdin_file,
			     const char *stdout_file, const char *stderr_file);
int
grub_util_exec (const char *const *argv);
int
grub_util_exec_redirect (const char *const *argv, const char *stdin_file,
			 const char *stdout_file);
int
grub_util_exec_redirect_null (const char *const *argv);

#endif
