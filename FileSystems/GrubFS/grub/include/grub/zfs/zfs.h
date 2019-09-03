/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 1999,2000,2001,2002,2003,2004,2009  Free Software Foundation, Inc.
 *
 *  GRUB is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
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
 /*
  * Copyright (c) 2007, 2010, Oracle and/or its affiliates. All rights reserved.
  */

#ifndef	GRUB_ZFS_HEADER
#define	GRUB_ZFS_HEADER 1

#include <grub/err.h>
#include <grub/disk.h>
#include <grub/crypto.h>

typedef enum grub_zfs_endian
  {
    GRUB_ZFS_UNKNOWN_ENDIAN = -2,
    GRUB_ZFS_LITTLE_ENDIAN = -1,
    GRUB_ZFS_BIG_ENDIAN = 0
  } grub_zfs_endian_t;

/*
 * On-disk version number.
 */
#define	SPA_VERSION_INITIAL		1ULL
#define	SPA_VERSION_BEFORE_FEATURES	33ULL
#define	SPA_VERSION_FEATURES		5000ULL
#define	SPA_VERSION_IS_SUPPORTED(v) \
	(((v) >= SPA_VERSION_INITIAL && (v) <= SPA_VERSION_BEFORE_FEATURES) || \
	((v) == SPA_VERSION_FEATURES))
/*
 * The following are configuration names used in the nvlist describing a pool's
 * configuration.
 */
#define	ZPOOL_CONFIG_VERSION		"version"
#define	ZPOOL_CONFIG_POOL_NAME		"name"
#define	ZPOOL_CONFIG_POOL_STATE		"state"
#define	ZPOOL_CONFIG_POOL_TXG		"txg"
#define	ZPOOL_CONFIG_POOL_GUID		"pool_guid"
#define	ZPOOL_CONFIG_CREATE_TXG		"create_txg"
#define	ZPOOL_CONFIG_TOP_GUID		"top_guid"
#define	ZPOOL_CONFIG_VDEV_TREE		"vdev_tree"
#define	ZPOOL_CONFIG_TYPE		"type"
#define	ZPOOL_CONFIG_CHILDREN		"children"
#define	ZPOOL_CONFIG_ID			"id"
#define	ZPOOL_CONFIG_GUID		"guid"
#define	ZPOOL_CONFIG_PATH		"path"
#define	ZPOOL_CONFIG_DEVID		"devid"
#define	ZPOOL_CONFIG_METASLAB_ARRAY	"metaslab_array"
#define	ZPOOL_CONFIG_METASLAB_SHIFT	"metaslab_shift"
#define	ZPOOL_CONFIG_ASHIFT		"ashift"
#define	ZPOOL_CONFIG_ASIZE		"asize"
#define	ZPOOL_CONFIG_DTL		"DTL"
#define	ZPOOL_CONFIG_STATS		"stats"
#define	ZPOOL_CONFIG_WHOLE_DISK		"whole_disk"
#define	ZPOOL_CONFIG_ERRCOUNT		"error_count"
#define	ZPOOL_CONFIG_NOT_PRESENT	"not_present"
#define	ZPOOL_CONFIG_SPARES		"spares"
#define	ZPOOL_CONFIG_IS_SPARE		"is_spare"
#define	ZPOOL_CONFIG_NPARITY		"nparity"
#define	ZPOOL_CONFIG_PHYS_PATH		"phys_path"
#define	ZPOOL_CONFIG_L2CACHE		"l2cache"
#define	ZPOOL_CONFIG_HOLE_ARRAY		"hole_array"
#define	ZPOOL_CONFIG_VDEV_CHILDREN	"vdev_children"
#define	ZPOOL_CONFIG_IS_HOLE		"is_hole"
#define	ZPOOL_CONFIG_DDT_HISTOGRAM	"ddt_histogram"
#define	ZPOOL_CONFIG_DDT_OBJ_STATS	"ddt_object_stats"
#define	ZPOOL_CONFIG_DDT_STATS		"ddt_stats"
#define	ZPOOL_CONFIG_FEATURES_FOR_READ	"features_for_read"
/*
 * The persistent vdev state is stored as separate values rather than a single
 * 'vdev_state' entry.  This is because a device can be in multiple states, such
 * as offline and degraded.
 */
#define	ZPOOL_CONFIG_OFFLINE		"offline"
#define	ZPOOL_CONFIG_FAULTED		"faulted"
#define	ZPOOL_CONFIG_DEGRADED		"degraded"
#define	ZPOOL_CONFIG_REMOVED		"removed"

#define	VDEV_TYPE_ROOT			"root"
#define	VDEV_TYPE_MIRROR		"mirror"
#define	VDEV_TYPE_REPLACING		"replacing"
#define	VDEV_TYPE_RAIDZ			"raidz"
#define	VDEV_TYPE_DISK			"disk"
#define	VDEV_TYPE_FILE			"file"
#define	VDEV_TYPE_MISSING		"missing"
#define	VDEV_TYPE_HOLE			"hole"
#define	VDEV_TYPE_SPARE			"spare"
#define	VDEV_TYPE_L2CACHE		"l2cache"

/*
 * pool state.  The following states are written to disk as part of the normal
 * SPA lifecycle: ACTIVE, EXPORTED, DESTROYED, SPARE, L2CACHE.  The remaining
 * states are software abstractions used at various levels to communicate pool
 * state.
 */
typedef enum pool_state {
	POOL_STATE_ACTIVE = 0,		/* In active use		*/
	POOL_STATE_EXPORTED,		/* Explicitly exported		*/
	POOL_STATE_DESTROYED,		/* Explicitly destroyed		*/
	POOL_STATE_SPARE,		/* Reserved for hot spare use	*/
	POOL_STATE_L2CACHE,		/* Level 2 ARC device		*/
	POOL_STATE_UNINITIALIZED,	/* Internal spa_t state		*/
	POOL_STATE_UNAVAIL,		/* Internal libzfs state	*/
	POOL_STATE_POTENTIALLY_ACTIVE	/* Internal libzfs state	*/
} pool_state_t;

struct grub_zfs_data;

grub_err_t grub_zfs_fetch_nvlist (grub_device_t dev, char **nvlist);
grub_err_t grub_zfs_getmdnobj (grub_device_t dev, const char *fsfilename,
			       grub_uint64_t *mdnobj);

char *grub_zfs_nvlist_lookup_string (const char *nvlist, const char *name);
char *grub_zfs_nvlist_lookup_nvlist (const char *nvlist, const char *name);
int grub_zfs_nvlist_lookup_uint64 (const char *nvlist, const char *name,
				   grub_uint64_t *out);
char *grub_zfs_nvlist_lookup_nvlist_array (const char *nvlist,
					   const char *name,
					   grub_size_t array_index);
int grub_zfs_nvlist_lookup_nvlist_array_get_nelm (const char *nvlist,
						  const char *name);
grub_err_t
grub_zfs_add_key (grub_uint8_t *key_in,
		  grub_size_t keylen,
		  int passphrase);

extern grub_err_t (*grub_zfs_decrypt) (grub_crypto_cipher_handle_t cipher,
				       grub_uint64_t algo,
				       void *nonce,
				       char *buf, grub_size_t size,
				       const grub_uint32_t *expected_mac,
				       grub_zfs_endian_t endian);

struct grub_zfs_key;

extern grub_crypto_cipher_handle_t (*grub_zfs_load_key) (const struct grub_zfs_key *key,
							 grub_size_t keysize,
							 grub_uint64_t salt,
							 grub_uint64_t algo);



#endif	/* ! GRUB_ZFS_HEADER */
