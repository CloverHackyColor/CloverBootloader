/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2009  Free Software Foundation, Inc.
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
#ifndef GRUB_AUTH_HEADER
#define GRUB_AUTH_HEADER	1

#include <grub/err.h>
#include <grub/crypto.h>

#define GRUB_AUTH_MAX_PASSLEN 1024

typedef grub_err_t (*grub_auth_callback_t) (const char *, const char *, void *);

grub_err_t grub_auth_register_authentication (const char *user,
					      grub_auth_callback_t callback,
					      void *arg);
grub_err_t grub_auth_unregister_authentication (const char *user);

grub_err_t grub_auth_authenticate (const char *user);
grub_err_t grub_auth_deauthenticate (const char *user);
grub_err_t grub_auth_check_authentication (const char *userlist);

#endif /* ! GRUB_AUTH_HEADER */
