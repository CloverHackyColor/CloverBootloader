/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2002,2003,2004,2005,2006,2007,2008,2009  Free Software Foundation, Inc.
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

#ifndef GRUB_CRYPTODISK_HEADER
#define GRUB_CRYPTODISK_HEADER	1

#include <grub/disk.h>
#include <grub/crypto.h>
#include <grub/list.h>
#ifdef GRUB_UTIL
#include <grub/emu/hostdisk.h>
#endif

typedef enum
  {
    GRUB_CRYPTODISK_MODE_ECB,
    GRUB_CRYPTODISK_MODE_CBC,
    GRUB_CRYPTODISK_MODE_PCBC,
    GRUB_CRYPTODISK_MODE_XTS,
    GRUB_CRYPTODISK_MODE_LRW
  } grub_cryptodisk_mode_t;

typedef enum
  {
    GRUB_CRYPTODISK_MODE_IV_NULL,
    GRUB_CRYPTODISK_MODE_IV_PLAIN,
    GRUB_CRYPTODISK_MODE_IV_PLAIN64,
    GRUB_CRYPTODISK_MODE_IV_ESSIV,
    GRUB_CRYPTODISK_MODE_IV_BENBI,
    GRUB_CRYPTODISK_MODE_IV_BYTECOUNT64,
    GRUB_CRYPTODISK_MODE_IV_BYTECOUNT64_HASH
  } grub_cryptodisk_mode_iv_t;

#define GRUB_CRYPTODISK_MAX_UUID_LENGTH 71

#define GRUB_CRYPTODISK_GF_LOG_SIZE 7
#define GRUB_CRYPTODISK_GF_SIZE (1U << GRUB_CRYPTODISK_GF_LOG_SIZE)
#define GRUB_CRYPTODISK_GF_LOG_BYTES (GRUB_CRYPTODISK_GF_LOG_SIZE - 3)
#define GRUB_CRYPTODISK_GF_BYTES (1U << GRUB_CRYPTODISK_GF_LOG_BYTES)
#define GRUB_CRYPTODISK_MAX_KEYLEN 128

struct grub_cryptodisk;

typedef gcry_err_code_t
(*grub_cryptodisk_rekey_func_t) (struct grub_cryptodisk *dev,
				 grub_uint64_t zoneno);

struct grub_cryptodisk
{
  struct grub_cryptodisk *next;
  struct grub_cryptodisk **prev;

  char *source;
  grub_disk_addr_t offset;
  grub_disk_addr_t total_length;
  grub_disk_t source_disk;
  int ref;
  grub_crypto_cipher_handle_t cipher;
  grub_crypto_cipher_handle_t secondary_cipher;
  grub_crypto_cipher_handle_t essiv_cipher;
  const gcry_md_spec_t *essiv_hash, *hash, *iv_hash;
  grub_cryptodisk_mode_t mode;
  grub_cryptodisk_mode_iv_t mode_iv;
  int benbi_log;
  unsigned long id, source_id;
  enum grub_disk_dev_id source_dev_id;
  char uuid[GRUB_CRYPTODISK_MAX_UUID_LENGTH + 1];
  grub_uint8_t lrw_key[GRUB_CRYPTODISK_GF_BYTES];
  grub_uint8_t *lrw_precalc;
  grub_uint8_t iv_prefix[64];
  grub_size_t iv_prefix_len;
  grub_uint8_t key[GRUB_CRYPTODISK_MAX_KEYLEN];
  grub_size_t keysize;
#ifdef GRUB_UTIL
  char *cheat;
  grub_util_fd_t cheat_fd;
#endif
  const char *modname;
  int log_sector_size;
  grub_cryptodisk_rekey_func_t rekey;
  int rekey_shift;
  grub_uint8_t rekey_key[64];
  grub_uint64_t last_rekey;
  int rekey_derived_size;
};
typedef struct grub_cryptodisk *grub_cryptodisk_t;

struct grub_cryptodisk_dev
{
  struct grub_cryptodisk_dev *next;
  struct grub_cryptodisk_dev **prev;

  grub_cryptodisk_t (*scan) (grub_disk_t disk, const char *check_uuid,
			     int boot_only);
  grub_err_t (*recover_key) (grub_disk_t disk, grub_cryptodisk_t dev);
};
typedef struct grub_cryptodisk_dev *grub_cryptodisk_dev_t;

extern grub_cryptodisk_dev_t EXPORT_VAR (grub_cryptodisk_list);

#ifndef GRUB_LST_GENERATOR
static inline void
grub_cryptodisk_dev_register (grub_cryptodisk_dev_t cr)
{
  grub_list_push (GRUB_AS_LIST_P (&grub_cryptodisk_list), GRUB_AS_LIST (cr));
}
#endif

static inline void
grub_cryptodisk_dev_unregister (grub_cryptodisk_dev_t cr)
{
  grub_list_remove (GRUB_AS_LIST (cr));
}

#define FOR_CRYPTODISK_DEVS(var) FOR_LIST_ELEMENTS((var), (grub_cryptodisk_list))

gcry_err_code_t
grub_cryptodisk_setkey (grub_cryptodisk_t dev,
			grub_uint8_t *key, grub_size_t keysize);
gcry_err_code_t
grub_cryptodisk_decrypt (struct grub_cryptodisk *dev,
			 grub_uint8_t * data, grub_size_t len,
			 grub_disk_addr_t sector);
grub_err_t
grub_cryptodisk_insert (grub_cryptodisk_t newdev, const char *name,
			grub_disk_t source);
#ifdef GRUB_UTIL
grub_err_t
grub_cryptodisk_cheat_insert (grub_cryptodisk_t newdev, const char *name,
			      grub_disk_t source, const char *cheat);
void
grub_util_cryptodisk_get_abstraction (grub_disk_t disk,
				      void (*cb) (const char *val, void *data),
				      void *data);

char *
grub_util_get_geli_uuid (const char *dev);
#endif

grub_cryptodisk_t grub_cryptodisk_get_by_uuid (const char *uuid);
grub_cryptodisk_t grub_cryptodisk_get_by_source_disk (grub_disk_t disk);

#endif
