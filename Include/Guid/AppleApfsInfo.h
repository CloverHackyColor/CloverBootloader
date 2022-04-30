/** @file
  Copyright (C) 2017, CupertinoNet.  All rights reserved.<BR>

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
**/

#ifndef APPLE_APFS_INFO_H_
#define APPLE_APFS_INFO_H_

#define APFS_GPT_PARTITION_UUID  \
  { 0x7C3457EF, 0x0000, 0x11AA,         \
    { 0xAA, 0x11, 0x00, 0x30, 0x65, 0x43, 0xEC, 0xAC } }

#define APFS_PARTITION_TYPE_GUID  \
  { 0x41504653, 0x0000, 0x11AA,         \
    { 0xAA, 0x11, 0x00, 0x30, 0x65, 0x43, 0xEC, 0xAC } }


extern EFI_GUID gAppleApfsPartitionTypeGuid;

#define APPLE_APFS_CONTAINER_INFO_GUID  \
  { 0x3533CF0D, 0x685F, 0x5EBF,         \
    { 0x8D, 0xC6, 0x73, 0x93, 0x48, 0x5B, 0xAF, 0xA2 } }

typedef struct {
  UINT32 Always1;
  GUID   Uuid;
} APPLE_APFS_CONTAINER_INFO;

extern EFI_GUID gAppleApfsContainerInfoGuid;

#define APPLE_APFS_VOLUME_INFO_GUID  \
  { 0x900C7693, 0x8C14, 0x58BA,      \
    { 0xB4, 0x4E, 0x97, 0x45, 0x15, 0xD2, 0x7C, 0x78 } }

#define APFS_VOLUME_ENUM_SHIFT   6
#define OID_NX_SUPERBLOCK           1
#define OID_INVALID                 0ULL
#define OID_RESERVED_COUNT          1024
#define MAX_CKSUM_SIZE              8
#define APFS_MAX_HIST               8
#define APFS_VOLNAME_LEN            256

#define OBJECT_TYPE_MASK            0x0000ffff
#define OBJECT_TYPE_FLAGS_MASK      0xffff0000
#define OBJ_STORAGETYPE_MASK        0xc0000000
#define OBJECT_TYPE_FLAGS_DEFINED_MASK 0xf8000000

#define APFS_VOL_ROLE_NONE              0x0000
#define APPLE_APFS_VOLUME_ROLE_SYSTEM    BIT0  // 0x01 Main partition. Joint with DATA.
#define APPLE_APFS_VOLUME_ROLE_USER      BIT1  // 0x02 The volume contains usersʼ home directories.
#define APPLE_APFS_VOLUME_ROLE_RECOVERY  BIT2  // 0x04
#define APPLE_APFS_VOLUME_ROLE_VM        BIT3  // 0x08
#define APPLE_APFS_VOLUME_ROLE_PREBOOT   BIT4  // 0x10
#define APPLE_APFS_VOLUME_ROLE_INSTALLER BIT5  // 0x20
#define APPLE_APFS_VOLUME_ROLE_DATA      BIT6  // 0x40 The volume contains mutable data.

#define APFS_VOL_ROLE_DATA        (1 << APFS_VOLUME_ENUM_SHIFT) //0x40
#define APFS_VOL_ROLE_BASEBAND    (2 << APFS_VOLUME_ENUM_SHIFT) // The volume is used by the radio firmware.
#define APFS_VOL_ROLE_UPDATE      (3 << APFS_VOLUME_ENUM_SHIFT) //0xC0 This role is used only on devices running iOS.???
#define APFS_VOL_ROLE_XART        (4 << APFS_VOLUME_ENUM_SHIFT) //The volume is used to manage OS access to secure user data.
#define APFS_VOL_ROLE_HARDWARE    (5 << APFS_VOLUME_ENUM_SHIFT) //The volume is used for firmware data. iOS
#define APFS_VOL_ROLE_BACKUP      (6 << APFS_VOLUME_ENUM_SHIFT) //The volume is used by Time Machine to store backups.
#define APFS_VOL_ROLE_RESERVED_7  (7 << APFS_VOLUME_ENUM_SHIFT)
#define APFS_VOL_ROLE_RESERVED_8  (8 << APFS_VOLUME_ENUM_SHIFT)
#define APFS_VOL_ROLE_ENTERPRISE  (9 << APFS_VOLUME_ENUM_SHIFT)
#define APFS_VOL_ROLE_RESERVED_10 (10 << APFS_VOLUME_ENUM_SHIFT)
#define APFS_VOL_ROLE_PRELOGIN    (11 << APFS_VOLUME_ENUM_SHIFT) //This volume is used to store system data used before login.

#define APFS_FEATURE_DEFRAG_PRERELEASE        0x00000001LL
#define APFS_FEATURE_HARDLINK_MAP_RECORDS     0x00000002LL
#define APFS_FEATURE_DEFRAG                   0x00000004LL
#define APFS_FEATURE_STRICTATIME              0x00000008LL
#define APFS_FEATURE_VOLGRP_SYSTEM_INO_SPACE  0x00000010LL


#define APFS_SUPPORTED_FEATURES_MASK (APFS_FEATURE_DEFRAG \
| APFS_FEATURE_DEFRAG_PRERELEASE \
| APFS_FEATURE_HARDLINK_MAP_RECORDS \
| APFS_FEATURE_STRICTATIME \
| APFS_FEATURE_VOLGRP_SYSTEM_INO_SPACE)

typedef UINT64 uint64_t;
typedef  INT64  int64_t;
typedef UINT32 uint32_t;
typedef UINT16 uint16_t;
typedef UINT8  uint8_t;
typedef uint64_t oid_t;
typedef uint64_t xid_t;
typedef int64_t     paddr_t;

typedef unsigned char   uuid_t[16];

struct obj_phys {
    uint8_t     o_cksum[MAX_CKSUM_SIZE];
    oid_t       o_oid;
    xid_t       o_xid;
    uint32_t    o_type;
    uint32_t    o_subtype;
};
typedef struct obj_phys obj_phys_t;

struct prange {
    paddr_t     pr_start_paddr;
    uint64_t    pr_block_count;
};
typedef struct prange prange_t;

typedef uint32_t cp_key_class_t;
typedef uint32_t cp_key_os_version_t;
/*
 * This type stores an OS version and build number as follows:
• Two bytes for the major version number as an unsigned integer
• Two bytes for the minor version letter as an ASCII character
• Four bytes for the build number as an unsigned integer
For example, to store the build number 18A391:
1. Store the number 18(0x12) in the highest two bytes, yielding 0x12000000.
2. Store the character A(0x41) in the next two bytes, yielding  0x12410000.
3. Store the number 391(0x0187) in the lowest four bytes, yielding 0x12410187.
 */
typedef uint16_t cp_key_revision_t;
typedef uint32_t crypto_flags_t;

struct wrapped_meta_crypto_state {
    uint16_t         major_version;
    uint16_t          minor_version;
    crypto_flags_t    cpflags;
    cp_key_class_t    persistent_class;
    cp_key_os_version_t key_os_version;
    cp_key_revision_t   key_revision;
    uint16_t            unused;
} __attribute__((aligned(2), packed));
typedef struct wrapped_meta_crypto_state wrapped_meta_crypto_state_t;

#define APFS_MODIFIED_NAMELEN 32
struct apfs_modified_by {
    uint8_t     id[APFS_MODIFIED_NAMELEN];
    uint64_t    timestamp;
    xid_t       last_xid;
 };
typedef struct apfs_modified_by apfs_modified_by_t;

struct apfs_superblock {
    obj_phys_t      apfs_o;
    uint32_t        apfs_magic;
    uint32_t        apfs_fs_index;
    uint64_t        apfs_features;
    uint64_t        apfs_readonly_compatible_features;
    uint64_t        apfs_incompatible_features;
    uint64_t        apfs_unmount_time;
    uint64_t        apfs_fs_reserve_block_count;
    uint64_t        apfs_fs_quota_block_count;
    uint64_t        apfs_fs_alloc_count;
    wrapped_meta_crypto_state_t              apfs_meta_crypto;
    uint32_t        apfs_root_tree_type;
    uint32_t        apfs_extentref_tree_type;
    uint32_t        apfs_snap_meta_tree_type;
    oid_t           apfs_omap_oid;
    oid_t           apfs_root_tree_oid;
    oid_t           apfs_extentref_tree_oid;
    oid_t           apfs_snap_meta_tree_oid;
    xid_t           apfs_revert_to_xid;
    oid_t           apfs_revert_to_sblock_oid;
    uint64_t        apfs_next_obj_id;
    uint64_t        apfs_num_files;
    uint64_t        apfs_num_directories;
    uint64_t        apfs_num_symlinks;
    uint64_t        apfs_num_other_fsobjects;
    uint64_t        apfs_num_snapshots;
    uint64_t        apfs_total_blocks_alloced;
    uint64_t        apfs_total_blocks_freed;
    uuid_t          apfs_vol_uuid;
    uint64_t        apfs_last_mod_time;
    uint64_t        apfs_fs_flags;
    apfs_modified_by_t      apfs_formatted_by;
    apfs_modified_by_t      apfs_modified_by[APFS_MAX_HIST];
    uint8_t         apfs_volname[APFS_VOLNAME_LEN];
    uint32_t        apfs_next_doc_id;
    uint16_t        apfs_role;
    uint16_t        reserved;
    xid_t           apfs_root_to_xid;
    oid_t           apfs_er_state_oid;
    uint64_t        apfs_cloneinfo_id_epoch;
    uint64_t        apfs_cloneinfo_xid;
    oid_t           apfs_snap_meta_ext_oid;
    uuid_t          apfs_volume_group_id;
    oid_t           apfs_integrity_meta_oid;
    oid_t           apfs_fext_tree_oid;
    uint32_t        apfs_fext_tree_type;
    uint32_t        reserved_type;
    oid_t           reserved_oid;
};

#define APFS_MAGIC      'BSPA'
#define APFS_MAX_HIST   8
#define APFS_VOLNAME_LEN  256


typedef UINT32 APPLE_APFS_VOLUME_ROLE;

typedef struct {
  UINT32                 Always1;
  GUID                   Uuid;
  APPLE_APFS_VOLUME_ROLE Role;
} APPLE_APFS_VOLUME_INFO;

extern EFI_GUID gAppleApfsVolumeInfoGuid;

#endif // APPLE_APFS_H_
