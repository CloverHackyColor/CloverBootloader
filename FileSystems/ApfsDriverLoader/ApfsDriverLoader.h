/** @file

APFS Driver Loader - loads apfs.efi from EfiBootRecord block

Copyright (c) 2017-2018, savvas

All rights reserved.

This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef APFS_DRIVER_LOADER_H_
#define APFS_DRIVER_LOADER_H_

#define APFS_DRIVER_INFO_PRIVATE_DATA_SIGNATURE  SIGNATURE_32 ('A', 'F', 'J', 'S')

//
// Container Superblock definitions
//
#define APFS_CSB_SIGNATURE  SIGNATURE_32 ('N', 'X', 'S', 'B')
#define APFS_CSB_MAX_FILE_SYSTEMS  100
#define APFS_CSB_EPH_INFO_COUNT  4
#define APFS_CSB_EPH_MIN_BLOCK_COUNT  8
#define APFS_CSB_MAX_FILE_SYSTEM_EPH_STRUCTS  4
#define APFS_CSB_TX_MIN_CHECKPOINT_COUNT  4
#define APFS_CSB_EPH_INFO_VERSION_1  1
#define APFS_CSB_NUM_COUNTERS  32

//
// Volume Superblock definitions
//
#define APFS_VSB_SIGNATURE  SIGNATURE_32 ('A', 'P', 'S', 'B')

//
// EfiBootRecord block definitions
//
#define APFS_EFIBOOTRECORD_SIGNATURE  SIGNATURE_32 ('J', 'S', 'D', 'R')
#define APFS_EFIBOOTRECORD_VERSION 1

typedef struct PhysicalRange_ {
    INT64     StartPhysicalAddr;
    UINT64    BlockCount;
} PhysicalRange;

typedef struct UNKNOWNFIELD_
{
  UINT32      Unknown1;
  EFI_HANDLE  Handle;
  EFI_HANDLE  AgentHandle;
  UINT8       Unknown2[88];
  UINT64      Unknown3;
} UNKNOWNFIELD;

//
// Private ApfsJumpStart structure
//
typedef struct _APFS_DRIVER_INFO_PRIVATE_DATA
{
  UINT32                            Magic;
  EFI_HANDLE                        ControllerHandle;
  EFI_HANDLE                        DriverBindingHandle;
  APFS_EFIBOOTRECORD_LOCATION_INFO  EfiBootRecordLocationInfo;
  UINT8                             Unknown1[24];
  EFI_EVENT                         NotifyEvent;
  VOID                              *ApfsDriverPtr;
  UINT32                            ApfsDriverSize;
  UINT32                            ContainerBlockSize;
  UINT64                            ContainerTotalBlocks;
  UINT8                             Unknown2[4];
  UINT32                            Unknown3;
  EFI_BLOCK_IO_PROTOCOL             *BlockIoInterface;
  UNKNOWNFIELD                      *Unknown4;
  UINT64                            UnknownAddress;
} APFS_DRIVER_INFO_PRIVATE_DATA;

#define APFS_EFIBOOTRECORD_INFO_PRIVATE_DATA_FROM_THIS(a) \
          CR(a, APFS_DRIVER_INFO_PRIVATE_DATA, EfiBootRecordLocationInfo, APFS_DRIVER_INFO_PRIVATE_DATA_SIGNATURE)

#pragma pack(push, 1)
typedef struct APFS_BLOCK_HEADER_
{
  //
  // Fletcher checksum, 64-bit. All metadata blocks
  //
  UINT64             Checksum;
  //
  // The object's identifier
  // Probably plays a role in the Btree structure NXSB=01 00
  // APSB=02 04, 06 04 and 08 04
  // nid
  //
  UINT64             ObjectOid;
  //
  // The identifier of the most recent transaction that this object
  // was modified in.
  //
  UINT64             ObjectXid;
  //
  // The objectʼs type and flags.
  // #define OBJ_VIRTUAL  0x00000000
  // #define OBJ_EPHEMERAL  0x80000000
  // #define OBJ_PHYSICAL  0x40000000
  // #define OBJ_NOHEADER  0x20000000
  // #define OBJ_ENCRYPTED  0x10000000
  // #define OBJ_NONPERSISTENT  0x08000000
  //
  UINT32             ObjectType;
  //
  // The objectʼs subtype
  // Subtypes indicate the type of data stored in a data structure such as a
  // B-tree. For example, a node in a B-tree that contains volume records has
  // a type of OBJECT_TYPE_BTREE_NODE and a subtype of OBJECT_TYPE_FS.
  //
  UINT32             ObjectSubType;
} APFS_BLOCK_HEADER;
#pragma pack(pop)

/**
  NXSB Container Superblock
  The container superblock is the entry point to the filesystem.
  Because of the structure with containers and flexible volumes,
  allocation needs to handled on a container level.
  The container superblock contains information on the blocksize,
  the number of blocks and pointers to the spacemanager for this task.
  Additionally the block IDs of all volumes are stored in the superblock.
  To map block IDs to block offsets a pointer to a block map b-tree is stored.
  This b-tree contains entries for each volume with its ID and offset.
**/
#pragma pack(push, 1)
typedef struct APFS_CSB_
{
  APFS_BLOCK_HEADER  BlockHeader;
  //
  // Magic: NXSB
  //
  UINT32             Magic;
  //
  // Size of each allocation unit: 4096 bytes
  // (by default)
  //
  UINT32             BlockSize;
  //
  // Number of blocks in the container
  //
  UINT64             TotalBlocks;
  UINT64             Features;
  UINT64             ReadOnlyCompatibleFeatures;
  UINT64             IncompatibleFeatures;
  EFI_GUID           Uuid;
  UINT64             NextOid;
  UINT64             NextXid;
  UINT32             XpDescBlocks;
  UINT32             XpDataBlocks;
  INT64              XpDescBase;
  INT64              XpDataBase;
  UINT32             XpDescNext;
  UINT32             XpDataNext;
  UINT32             XpDescIndex;
  UINT32             XpDescLen;
  UINT32             XpDataIndex;
  UINT32             XpDataLen;
  UINT64             SpacemanOid;
  UINT64             ObjectMapOid;
  UINT64             ReaperOid;
  UINT32             TestType;
  UINT32             MaxFileSystems;
  UINT64             FileSystemOid[APFS_CSB_MAX_FILE_SYSTEMS];
  UINT64             Counters[APFS_CSB_NUM_COUNTERS];
  PhysicalRange      BlockedOutPhysicalRange;
  UINT64             EvictMappingTreeOid;
  UINT64             Flags;
  //
  // Pointer to JSDR block (EfiBootRecordBlock)
  //
  UINT64             EfiBootRecordBlock;
  EFI_GUID           FusionUuid;
  PhysicalRange      KeyLocker;
  UINT64             EphermalInfo[APFS_CSB_EPH_INFO_COUNT];
  UINT64             TestOid;
  UINT64             FusionMtIod;
  UINT64             FusionWbcOid;
  PhysicalRange      FusionWbc;
} APFS_CSB;
#pragma pack(pop)

//
// APSB volume header structure
//
#pragma pack(push, 1)
typedef struct APFS_APSB_
{
  APFS_BLOCK_HEADER  BlockHeader;
  //
  // Volume Superblock magic
  // Magic: APSB
  //
  UINT32             Magic;
  //
  // Volume#. First volume start with 0, (0x00)
  //
  UINT32             VolumeNumber;
  UINT8              Reserved_1[20];
  //
  // Case setting of the volume.
  // 1 = Not case sensitive
  // 8 = Case sensitive (0x01, Not C.S)
  //
  UINT32             CaseSetting;
  UINT8              Reserved_2[12];
  //
  // Size of volume in Blocks. Last volume has no
  // size set and has available the rest of the blocks
  //
  UINT64             VolumeSize;
  UINT64             Reserved_3;
  //
  // Blocks in use in this volumes
  //
  UINT64             BlocksInUseCount;
  UINT8              Reserved_4[32];
  //
  // Block# to initial block of catalog B-Tree Object
  // Map (BTOM)
  //
  UINT64             BlockNumberToInitialBTOM;
  //
  // Node Id of root-node
  //
  UINT64             RootNodeId;
  //
  // Block# to Extents B-Tree,block#
  //
  UINT64             BlockNumberToEBTBlockNumber;
  //
  // Block# to list of Snapshots
  //
  UINT64             BlockNumberToListOfSnapshots;
  UINT8              Reserved_5[16];
  //
  // Next CNID
  //
  UINT64             NextCnid;
  //
  // Number of files on the volume
  //
  UINT64             NumberOfFiles;
  //
  // Number of folders on the volume
  //
  UINT64             NumberOfFolder;
  UINT8              Reserved_6[40];
  //
  // Volume UUID
  //
  EFI_GUID           VolumeUuid;
  //
  // Time Volume last written/modified
  //
  UINT64             ModificationTimestamp;
  UINT64             Reserved_7;
  //
  // Creator/APFS-version
  // Ex. (hfs_convert (apfs- 687.0.0.1.7))
  //
  UINT8              CreatorVersionInfo[32];
  //
  // Time Volume created
  //
  UINT64             CreationTimestamp;
  //
  // ???
  //
} APFS_APSB;
#pragma pack(pop)

//
// JSDR block structure
//
#pragma pack(push, 1)
typedef struct APFS_EFI_BOOT_RECORD_
{
  APFS_BLOCK_HEADER  BlockHeader;
  //
  // A number that can be used to verify that youʼre reading an instance of
  // APFS_EFI_BOOT_RECORD
  //
  UINT32             Magic;
  //
  // The version of this data structure
  //
  UINT32             Version;
  //
  // The length in bytes, of the embedded EFI driver
  //
  UINT32             EfiFileLen;
  //
  // Num of extents in the array
  //
  UINT32             NumOfExtents;
  //
  // Reserved
  // Populate this field with 0 when you create a new instance,
  // and preserve its value when you modify an existing instance.
  //
  UINT64             Reserved[16];
  //
  // Apfs driver physical range location
  //
  PhysicalRange      RecordExtents[];
} APFS_EFI_BOOT_RECORD;
#pragma pack(pop)

#endif // APFS_DRIVER_LOADER_H_
