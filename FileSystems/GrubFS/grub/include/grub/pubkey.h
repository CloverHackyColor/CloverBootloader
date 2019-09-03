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

#ifndef GRUB_PUBKEY_HEADER
#define GRUB_PUBKEY_HEADER 1

#include <grub/crypto.h>

struct grub_public_key *
grub_load_public_key (grub_file_t f);

grub_err_t
grub_verify_signature (grub_file_t f, grub_file_t sig,
		       struct grub_public_key *pk);


struct grub_public_subkey *
grub_crypto_pk_locate_subkey (grub_uint64_t keyid, struct grub_public_key *pkey);

struct grub_public_subkey *
grub_crypto_pk_locate_subkey_in_trustdb (grub_uint64_t keyid);

#endif
