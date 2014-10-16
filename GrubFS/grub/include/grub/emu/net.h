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

#ifndef GRUB_EMUNET_HEADER
#define GRUB_EMUNET_HEADER	1

#include <grub/types.h>
#include <grub/symbol.h>

grub_ssize_t
EXPORT_FUNC(grub_emunet_send) (const void *packet, grub_size_t sz);

grub_ssize_t
EXPORT_FUNC(grub_emunet_receive) (void *packet, grub_size_t sz);

int
EXPORT_FUNC(grub_emunet_create) (grub_size_t *mtu);

void
EXPORT_FUNC(grub_emunet_close) (void);

#endif
