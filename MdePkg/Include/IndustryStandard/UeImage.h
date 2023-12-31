/** @file
  Definitions of the UEFI Executable (UE) file format.

  Copyright (c) 2021 - 2023, Marvin Häuser. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef UE_IMAGE_H_
#define UE_IMAGE_H_

#include <Library/BaseLib.h>

//
// UE segment definitions.
//

///
/// Definition of the UE segment permission configurations.
///
enum {
  UeSegmentPermX  = 0,
  UeSegmentPermRX = 1,
  UeSegmentPermRW = 2,
  //
  // Read-only is the last value, as this makes it easier to implement it as
  // the else/default case.
  //
  UeSegmentPermR  = 3,
  UeSegmentPermMax
};

///
/// The minimum alignment requirement, in bytes, of each UE segment in the UE
/// address space.
///
#define UE_SEGMENT_MIN_ALIGNMENT  0x00001000U

///
/// The maximum alignment requirement, in bytes, of each UE segment in the UE
/// address space.
///
#define UE_SEGMENT_MAX_ALIGNMENT  0x08000000U

///
/// Information about the UE segment in the UE address space.
///
/// [Bits 19:0]  The size, in 4-KiB units, of the UE segment in the UE address
///              space.
/// [Bits 21:20] The UE segment permissions.
/// [Bits 31:22] Reserved for future use. Must be zero.
///
typedef UINT32 UE_SEGMENT_IMAGE_INFO;

///
/// Definition of a UE segment header.
///
typedef struct {
  ///
  /// Information about the UE segment in the UE address space.
  ///
  UE_SEGMENT_IMAGE_INFO  ImageInfo;
  ///
  /// The size, in bytes, of the UE segment in the UE file.
  ///
  UINT32                 FileSize;
} UE_SEGMENT;

STATIC_ASSERT (
  sizeof (UE_SEGMENT) == 8 && ALIGNOF (UE_SEGMENT) == 4,
  "The UE segment definition does not meet the specification."
  );

///
/// Definition of a UE XIP segment header.
///
typedef struct {
  ///
  /// Information about the UE segment in the UE address space.
  ///
  UE_SEGMENT_IMAGE_INFO  ImageInfo;
} UE_SEGMENT_XIP;

STATIC_ASSERT (
  sizeof (UE_SEGMENT_XIP) == 4 && ALIGNOF (UE_SEGMENT_XIP) == 4,
  "The UE XIP segment definition does not meet the specification."
  );

/**
  Retrieve the UE segment memory permissions.

  @param[in] ImageInfo  The UE segment image information.
**/
#define UE_SEGMENT_PERMISSIONS(ImageInfo)  \
  ((UINT8)(((ImageInfo) >> 20U) & 0x03U))

/**
  Retrieve the size, in bytes, of the UE segment in the UE address space.

  @param[in] ImageInfo  The UE segment image information.
**/
#define UE_SEGMENT_SIZE(ImageInfo) ((ImageInfo) << 12U)

STATIC_ASSERT (
  IS_ALIGNED (UE_SEGMENT_SIZE (0xFFFFFFFF), UE_SEGMENT_MIN_ALIGNMENT),
  "The UE segment size definition does not meet the specification."
  );

//
// UE load table definitions.
//

///
/// The alignment, in bytes, of each UE load table in the UE file.
///
#define UE_LOAD_TABLE_ALIGNMENT  8U

///
/// Definition of the UE load table identifiers.
///
enum {
  //
  // An array of UE fixup roots. Blocks are ordered ascending by their
  // base address.
  //
  UeLoadTableIdReloc = 0x00,
  //
  // An instance of the UE debug table..
  //
  UeLoadTableIdDebug = 0x01
};

///
/// Definition of a UE load table header.
///
typedef struct {
  ///
  /// Information about the UE load table.
  ///
  /// [Bits 28:0]  The size, in 8-byte units, of the UE load table in the UE
  ///              file.
  /// [Bits 31:29] The identifier of the UE load table.
  ///
  UINT32 FileInfo;
} UE_LOAD_TABLE;

STATIC_ASSERT (
  sizeof (UE_LOAD_TABLE) == 4 && ALIGNOF (UE_LOAD_TABLE) == 4,
  "The UE load table definition does not meet the specification."
  );

/**
  Retrieves the UE load table identifier.

  @param[in] FileInfo  The UE load table file information.
**/
#define UE_LOAD_TABLE_ID(FileInfo) ((UINT8)((FileInfo) >> 29U))

/**
  Retrieves the size, in bytes, of the UE load table in the UE file.

  @param[in] FileInfo  The UE load table file information.
**/
#define UE_LOAD_TABLE_SIZE(FileInfo) ((FileInfo) << 3U)

STATIC_ASSERT (
  IS_ALIGNED (UE_LOAD_TABLE_SIZE (0xFFFFFFFF), UE_LOAD_TABLE_ALIGNMENT),
  "The UE load table size definition does not meet the specification."
  );

//
// UE relocation table definitions.
//

///
/// Definitions of the generic UE relocation identifiers.
///
enum {
  UeReloc32       = 0x00,
  UeReloc64       = 0x01,
  UeReloc32NoMeta = 0x02,
  UeRelocGenericMax
};

#if 0
///
/// Definition of the ARM UE relocation identifiers.
///
enum {
  UeRelocArmMovtMovw = 0x02
};
#endif

///
/// The alignment requirement for a UE fixup root.
///
#define UE_FIXUP_ROOT_ALIGNMENT  4U

STATIC_ASSERT (
  UE_FIXUP_ROOT_ALIGNMENT <= UE_LOAD_TABLE_ALIGNMENT,
  "The UE fixup root definition does not meet the specification."
  );

///
/// Definition of a UE fixup root.
///
typedef struct {
  ///
  /// The offset of the first head fixup, in bytes, from the end of the previous
  /// UE relocation fixup (chained or not). The first UE fixup root is
  /// relative to 0.
  ///
  UINT32  FirstOffset;
  ///
  /// The head fixups of the UE fixup root.
  ///
  /// [Bits 3:0]   The type of the UE relocation fixup.
  /// [Bits 15:4]  The offset of the next UE head fixup from the end of the last
  ///              UE relocation fixup in the chain (if chained). If 0x0FFF, the
  ///              current fixup root is terminated.
  ///
  UINT16  Heads[];
} UE_FIXUP_ROOT;

STATIC_ASSERT (
  sizeof (UE_FIXUP_ROOT) == 4 && ALIGNOF (UE_FIXUP_ROOT) == UE_FIXUP_ROOT_ALIGNMENT,
  "The UE fixup root definition does not meet the specification."
  );

STATIC_ASSERT (
  OFFSET_OF (UE_FIXUP_ROOT, Heads) == sizeof (UE_FIXUP_ROOT),
  "The UE fixup root definition does not meet the specification."
  );

STATIC_ASSERT (
  sizeof (UE_FIXUP_ROOT) <= UE_LOAD_TABLE_ALIGNMENT,
  "The UE fixup root definition is misaligned."
  );

#define MIN_SIZE_OF_UE_FIXUP_ROOT  (sizeof (UE_FIXUP_ROOT) + sizeof (UINT16))

///
/// The maximum offset, in bytes, of the next UE head fixup.
///
#define UE_HEAD_FIXUP_MAX_OFFSET  0x0FFEU

///
/// UE head fixup offset that terminates a fixup root.
///
#define UE_HEAD_FIXUP_OFFSET_END  0x0FFFU

/**
  Retrieves the target offset of the UE relocation fixup.

  @param[in] FixupInfo  The UE relocation fixup information.
**/
#define UE_RELOC_FIXUP_OFFSET(FixupInfo) ((UINT16)((FixupInfo) >> 4U))

/**
  Retrieves the type of the UE relocation fixup.

  @param[in] FixupInfo  The UE relocation fixup information.
**/
#define UE_RELOC_FIXUP_TYPE(FixupInfo) ((FixupInfo) & 0x000FU)

/**
  Retrieves the offset of the next UE chained relocation fixup.

  @param[in] FixupInfo  The UE relocation fixup information.
**/
#define UE_CHAINED_RELOC_FIXUP_NEXT_OFFSET(FixupInfo)  \
  ((UINT16)((UINT16)(FixupInfo) >> 4U) & 0x0FFFU)

///
/// The maximum offset, in bytes, of the next UE chained relocation fixup.
///
#define UE_CHAINED_RELOC_FIXUP_MAX_OFFSET  0x0FFEU

///
/// UE chained relocation fixup offset that terminates a chain.
///
#define UE_CHAINED_RELOC_FIXUP_OFFSET_END  0x0FFFU

/**
  Retrieves the type of the next UE chained relocation fixup.

  @param[in] FixupInfo  The UE relocation fixup information.
**/
#define UE_CHAINED_RELOC_FIXUP_NEXT_TYPE(FixupInfo)  \
  ((UINT8)((UINT16)(FixupInfo) & 0x0FU))

///
/// The shift exponent for UE chained relocation fixup values.
///
#define UE_CHAINED_RELOC_FIXUP_VALUE_SHIFT  16U

/**
  Retrieves the value of the current UE chained relocation fixup.

  @param[in] FixupInfo  The UE relocation fixup information.
**/
#define UE_CHAINED_RELOC_FIXUP_VALUE(FixupInfo)  \
  RShiftU64 (FixupInfo, UE_CHAINED_RELOC_FIXUP_VALUE_SHIFT)

///
/// Definition of the common header of UE chained relocation fixups.
///
/// [Bits 3:0]  The relocation type of the next chained relocation fixup. Only
///             valid when [Bits 15:4] are not 0x0FFF.
/// [Bits 15:4] The offset to the next chained relocation fixup from the end
///             of the current one. If 0x0FFF, the current chain is terminated.
///             Consult the fixup root for further relocation fixups.
///
typedef UINT16  UE_RELOC_FIXUP_HDR;

///
/// Definition of the generic 64-bit UE chained relocation fixup.
///
/// [Bits 15:0]  The common header of UE chained relocation fixups.
/// [Bits 47:16] The address value to relocate.
/// [Bits 63:48] Must be zero.
///
typedef UINT64  UE_RELOC_FIXUP_64;

///
/// The shift exponent for UE chained 32-bit relocation fixup values.
///
#define UE_CHAINED_RELOC_FIXUP_VALUE_32_SHIFT  12U

/**
  Retrieves the value of the current UE chained 32-bit relocation fixup.

  @param[in] FixupInfo  The UE relocation fixup information.
**/
#define UE_CHAINED_RELOC_FIXUP_VALUE_32(FixupInfo)  \
  (UINT32)((UINT32)(FixupInfo) >> UE_CHAINED_RELOC_FIXUP_VALUE_32_SHIFT)

///
/// Definition of the generic 32-bit UE chained relocation fixup.
///
/// [Bits 11:0]  The offset to the next chained relocation fixup from the end
///              of the current one. If 0x0FFF, the current chain is terminated.
///              Consult the fixup root for further relocation fixups.
/// [Bits 31:12] The address value to relocate.
///
typedef UINT32  UE_RELOC_FIXUP_32;

#if 0
///
/// Definition of the ARM Thumb MOVT/MOVW UE chained relocation fixup.
///
/// [Bits 15:0]  The common header of UE chained relocation fixups.
/// [Bits 31:16] The 16-bit immediate value to relocate.
///
typedef UINT32  UE_RELOC_FIXUP_ARM_MOVT_MOVW;
#endif

//
// UE debug table definitions.
//
// NOTE: The UE symbols base address offset is required for conversion of
//       PE Images that have their first section start after the end of the
//       Image headers. As PDBs cannot easily be rebased, store the offset.
//

///
/// Definition of a UE segment name. Must be \0-terminated.
///
typedef UINT8 UE_SEGMENT_NAME[8];

STATIC_ASSERT (
  sizeof (UE_SEGMENT_NAME) == 8 && ALIGNOF (UE_SEGMENT_NAME) == 1,
  "The UE segment name definition does not meet the specification."
  );

///
/// Definition of the UE debug table header.
///
typedef struct {
  ///
  /// Information about the image regarding the symbols file.
  ///
  /// [Bits 1:0] The offset, in image alignment units, to be subtracted from the
  ///            UE base address in order to retrieve the UE symbols base
  ///            address.
  /// [Bits 7:2] Reserved for future use. Must be zero.
  ///
  UINT8            ImageInfo;
  ///
  /// The length, in bytes, of the UE symbols path (excluding the terminator).
  ///
  UINT8            SymbolsPathLength;
  ///
  /// The UE symbols path. Must be \0-terminated.
  ///
  UINT8            SymbolsPath[];
  ///
  /// The UE segment name table. The order matches the UE segment table.
  ///
//UE_SEGMENT_NAME  SegmentNames[];
} UE_DEBUG_TABLE;

///
/// The minimum size, in bytes, of the UE debug table.
///
#define MIN_SIZE_OF_UE_DEBUG_TABLE  \
  (OFFSET_OF (UE_DEBUG_TABLE, SymbolsPath) + 1U)

/**
  Retrieves the UE symbol address subtrahend in SegmentAlignment-units.

  @param[in] ImageInfo  The UE debug table image information.
**/
#define UE_DEBUG_TABLE_IMAGE_INFO_SYM_SUBTRAHEND_FACTOR(ImageInfo)  \
  ((UINT8)((ImageInfo) & 0x03U))

/**
  Retrieves the UE segment name table of a UE debug table.

  @param[in] DebugTable  The UE debug table.
**/
#define UE_DEBUG_TABLE_SEGMENT_NAMES(DebugTable)                     \
  (CONST UE_SEGMENT_NAME *) (                                        \
    (DebugTable)->SymbolsPath + (DebugTable)->SymbolsPathLength + 1  \
    )

STATIC_ASSERT (
  sizeof (UE_DEBUG_TABLE) == 2 && ALIGNOF (UE_DEBUG_TABLE) == 1,
  "The UE debug table definition does not meet the specification."
  );

STATIC_ASSERT (
  ALIGNOF (UE_DEBUG_TABLE) <= UE_LOAD_TABLE_ALIGNMENT,
  "The UE debug table definition is misaligned."
  );

STATIC_ASSERT (
  OFFSET_OF (UE_DEBUG_TABLE, SymbolsPath) == sizeof (UE_DEBUG_TABLE),
  "The UE fixup root definition does not meet the specification."
  );

//
// UE header definitions.
//

///
/// The file magic number of a UE header.
///
#define UE_HEADER_MAGIC  SIGNATURE_16 ('U', 'E')

///
/// Definition of the UE machine identifiers.
///
enum {
  UeMachineI386          = 0,
  UeMachineX64           = 1,
  UeMachineArmThumbMixed = 2,
  UeMachineArm64         = 3,
  UeMachineRiscV32       = 4,
  UeMachineRiscV64       = 5,
  UeMachineRiscV128      = 6
};

///
/// Definition of the UE subsystem identifiers.
///
enum {
  UeSubsystemEfiApplication        = 0,
  UeSubsystemEfiBootServicesDriver = 1,
  UeSubsystemEfiRuntimeDriver      = 2
};

///
/// Definition of a UE file header.
///
typedef struct {
  ///
  /// The file magic number to identify the UE file format. Must be 'UE'.
  ///
  UINT16        Magic;
  ///
  /// Information about the image kind and supported architectures.
  ///
  /// [Bits 2:0] Indicates the subsystem.
  /// [Bits 7:3] Indicates the supported architectures.
  ///
  UINT8         Type;
  ///
  /// Information about the UE load tables and segments.
  ///
  /// [Bits 2:0] The number of UE load tables.
  /// [Bits 7:3] The index of the last segment in the UE segment table.
  ///
  UINT8         TableCounts;
  ///
  /// Indicates the offset of the UE entry point in the UE address space.
  ///
  UINT32        EntryPointAddress;
  ///
  /// Information about the UE image.
  ///
  /// [Bits 51:0]  The base UEFI page of the UE image, i.e., the base address in
  ///              4 KiB units.
  /// [Bits 55:52] Reserved for future use. Must be zero.
  /// [Bit 56]     Indicates whether the UE image is XIP
  /// [Bit 57]     Indicates whether the UE image is designated for a fixed
  ///              address.
  /// [Bit 58]     Indicates whether the UE relocation table has been stripped.
  /// [Bit 59]     Indicates whether UE chained fixups are used.
  /// [Bits 63:60] The shift exponent, offset by -12, for the UE segment
  ///              alignment in bytes.
  ///
  UINT64        ImageInfo;
  ///
  /// The UE segment table. It contains all data of the UE address space.
  ///
  /// All UE segments are contiguous in the UE address space.
  /// The offset of the first UE segment in the UE address space is 0.
  ///
  /// All UE segments' data are contiguous in the UE file.
  /// The offset of the first UE segment in the UE file is the end of the UE
  /// file header.
  ///
  UE_SEGMENT    Segments[];
  ///
  /// The UE load tables. They contain data useful for UE loading.
  ///
  /// All UE load tables are contiguous in the UE file.
  /// The offset of the first UE load table in the UE file is the end of the last
  /// UE segment in the UE file.
  ///
  /// All UE load tables are ordered ascending by their identifier.
  ///
//UE_LOAD_TABLE LoadTables[];
} UE_HEADER;

///
/// The minimum size, in bytes, of a valid UE header.
///
#define MIN_SIZE_OF_UE_HEADER  \
  (OFFSET_OF (UE_HEADER, Segments) + sizeof (UE_SEGMENT))

STATIC_ASSERT (
  sizeof (UE_HEADER) == 16 && ALIGNOF (UE_HEADER) == 8,
  "The UE header definition does not meet the specification."
  );

STATIC_ASSERT (
  ALIGNOF (UE_SEGMENT) <= ALIGNOF (UE_LOAD_TABLE),
  "The UE header definition is misaligned."
  );

STATIC_ASSERT (
  OFFSET_OF (UE_HEADER, Segments) == sizeof (UE_HEADER),
  "The UE header definition does not meet the specification."
  );

/**
  Retrieves the UE base address.

  @param[in] ImageInfo  The UE header image information.
**/
#define UE_HEADER_BASE_ADDRESS(ImageInfo) LShiftU64 (ImageInfo, 12)

/**
  Retrieves the UE segment alignment, in bytes, as a power of two.

  @param[in] ImageInfo  The UE header image information.
**/
#define UE_HEADER_SEGMENT_ALIGNMENT(ImageInfo)  \
  (1U << ((UINT8)RShiftU64 (ImageInfo, 60) + 12U))

///
/// UE header image information bit that indicates whether the image is XIP.
///
#define UE_HEADER_IMAGE_INFO_XIP            0x0100000000000000ULL

///
/// UE header image information bit that indicates whether the image is
/// designated to be loaded to a fixed address.
///
#define UE_HEADER_IMAGE_INFO_FIXED_ADDRESS  0x0200000000000000ULL

///
/// UE header image information bit that indicates whether the relocation fixups
/// have been stripped.
///
#define UE_HEADER_IMAGE_INFO_RELOCATION_FIXUPS_STRIPPED  0x0400000000000000ULL

///
/// UE header image information bit that indicates whether UE relocation fixup
/// chains are utilized.
///
#define UE_HEADER_IMAGE_INFO_CHAINED_FIXUPS  0x0800000000000000ULL

/**
  Retrieves the UE subsystem.

  @param[in] Type  The UE header type information.
**/
#define UE_HEADER_SUBSYSTEM(Type) ((Type) & 0x07U)

/**
  Retrieves the UE supported architectures.

  @param[in] Type  The UE header type information.
**/
#define UE_HEADER_ARCH(Type) ((Type) >> 3U)

///
/// The maximum number of UE load tables.
///
#define UE_HEADER_NUM_LOAD_TABLES_MAX  7U

/**
  Retrieves the number of UE load tables.

  @param[in] TableCounts  The UE header segment and load table information.
**/
#define UE_HEADER_NUM_LOAD_TABLES(TableCounts) ((TableCounts) & 0x07U)

STATIC_ASSERT (
  UE_HEADER_NUM_LOAD_TABLES (0xFFU) == UE_HEADER_NUM_LOAD_TABLES_MAX,
  "The number of load tables violates the specification."
  );

///
/// The maximum number of UE segments.
///
#define UE_HEADER_NUM_SEGMENTS_MAX  32U

/**
  Retrieves the index of the last UE segment, i.e., their amount minus 1.

  @param[in] TableCounts  The UE header segment and load table information.
**/
#define UE_HEADER_LAST_SEGMENT_INDEX(TableCounts) ((TableCounts) >> 3U)

STATIC_ASSERT (
  UE_HEADER_LAST_SEGMENT_INDEX (0xFFU) + 1U == UE_HEADER_NUM_SEGMENTS_MAX,
  "The number of load tables violates the specification."
  );

/**
  Retrieves the 8 byte aligned UE file size.

  If the file size is larger than this value, the appended data may be the UE
  certificate table.

  @param[in] FileInfo  The UE header file information.
**/
#define UE_HEADER_FILE_SIZE(FileInfo) ((FileInfo) << 3U)

STATIC_ASSERT (
  IS_ALIGNED (UE_HEADER_FILE_SIZE (0xFFFFFFFF), UE_LOAD_TABLE_ALIGNMENT),
  "The UE file size definition does not meet the specification."
  );

///
/// The maximum size, in bytes, of a valid UE header.
///
#define MAX_SIZE_OF_UE_HEADER  \
  MIN_SIZE_OF_UE_HEADER + \
  UE_HEADER_NUM_SEGMENTS_MAX * sizeof (UE_SEGMENT) + \
  UE_HEADER_NUM_LOAD_TABLES_MAX * sizeof (UE_LOAD_TABLE)

#endif // UE_IMAGE_H_
