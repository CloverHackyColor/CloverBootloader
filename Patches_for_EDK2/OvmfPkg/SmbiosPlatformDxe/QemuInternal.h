/** @file
  This header provides common includes, and communicates internal types,
  function prototypes and macros between "Qemu.c" and "QemuTypeXX.c", that
  relate to the installation and patching of SMBIOS tables on the QEMU
  platform.

  Copyright (C) 2013, Red Hat, Inc.

  This program and the accompanying materials are licensed and made available
  under the terms and conditions of the BSD License which accompanies this
  distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS, WITHOUT
  WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/

#ifndef _QEMU_INTERNAL_H_
#define _QEMU_INTERNAL_H_

#include <IndustryStandard/SmBios.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/QemuFwCfgLib.h>
#include <Protocol/Smbios.h>


//
// Type identifiers of all tables mandated by the SMBIOS-2.7.1 specification
// fall strictly under this limit.
//
#define TABLE_TYPE_LIMIT (EFI_SMBIOS_TYPE_SYSTEM_BOOT_INFORMATION + 1)


//
// Track a patch in dynamic memory, originating from a QEMU SMBIOS firmware
// configuration entry with ET_FIELD type.
//
typedef struct {
  UINT8  *Base;
  UINT16 Size;
} PATCH;


#define FIELD_OFFSET_MINIMUM  ((INT32) sizeof(SMBIOS_STRUCTURE))
#define PATCH_SUBSCRIPT_LIMIT (255 - FIELD_OFFSET_MINIMUM)

//
// The following structure tracks the installation of each SMBIOS table with a
// type below TABLE_TYPE_LIMIT, and captures QEMU SMBIOS firmware configuration
// entries with ET_FIELD type that target the default table for the same type.
//
typedef struct {
  BOOLEAN Installed; // at least one instance of the type has been installed

  PATCH Patch[PATCH_SUBSCRIPT_LIMIT]; // Patches indexed by the field offset
                                      // that they target in this specific
                                      // table type. Patching the SMBIOS table
                                      // header is not allowed, hence we can
                                      // shift down field offsets. An unused
                                      // element has zeroed-out fields.
} TABLE_CONTEXT;


//
// Track the installation of, and stored patches for, all table types below
// TABLE_TYPE_LIMIT.
//
typedef struct {
  TABLE_CONTEXT Table[TABLE_TYPE_LIMIT];
} BUILD_CONTEXT;


//
// Convenience / safety macro for defining C structure types for default SMBIOS
// tables.
//
// Rules of use:
// - Use only within #pragma pack(1).
// - This macro depends on the macro
//   "OVMF_TYPE ## TableType ## _STRINGS" specifying the text strings
//   (unformatted area) for TableType. Each "QemuTypeXX.c" file needs to
//   provide said macro before using the one below.
//
#define OVMF_SMBIOS(TableType)                                                \
          typedef struct {                                                    \
            SMBIOS_TABLE_TYPE##TableType Base;                                \
            UINT8             Strings[sizeof OVMF_TYPE##TableType##_STRINGS]; \
          } OVMF_TYPE##TableType


//
// Convenience / safety macro for patching a field in the formatted area of
// an SMBIOS table.
//
#define PATCH_FORMATTED(Context, TableType, OvmfTablePtr, FieldName)      \
          PatchSmbiosFormatted (                                          \
            Context,                                                      \
            TableType,                                                    \
            (UINT16) OFFSET_OF (SMBIOS_TABLE_TYPE##TableType, FieldName), \
            (UINT16) sizeof (OvmfTablePtr)->Base.FieldName,               \
            (UINT8 *) (OvmfTablePtr)                                      \
            )


/**
  Apply a saved patch to a field located in the formatted are of a not yet
  installed SMBIOS table.

  The patch is looked up based on (Context, TableType, FieldOffset).

  @param[in]  Context      The BUILD_CONTEXT object storing saved patches.
  @param[in]  TableType    Selects the table type for which the patch has been
                           saved. It is assumed that the caller has validated
                           TableType against TABLE_TYPE_LIMIT (upper
                           exclusive).
  @param[in]  FieldOffset  Selects the SMBIOS field for which the patch has
                           been saved. It is assumed that the caller has
                           validated FieldOffset against FIELD_OFFSET_MINIMUM
                           (lower inclusive) and 255 (upper exclusive).
  @param[in]  FieldSize    The caller supplies the size of the field to patch
                           in FieldSize. The patch saved for
                           TableType:FieldOffset, if any, is only applied if
                           its size equals FieldSize.
  @param[out] TableBase    Base of the SMBIOS table of type TableType in which
                           the field starting at FieldOffset needs to be
                           patched.

  @retval EFI_NOT_FOUND          No patch found for TableType:FieldOffset in
                                 Context. This return value is considered
                                 informative (ie. non-fatal).
  @retval EFI_INVALID_PARAMETER  Patch found for TableType:FieldOffset, but its
                                 size doesn't match FieldSize. This result is
                                 considered a fatal error of the patch origin.
  @retval EFI_SUCCESS            The SMBIOS table at TableBase has been patched
                                 starting at FieldOffset for a length of
                                 FieldSize.
**/
EFI_STATUS
EFIAPI
PatchSmbiosFormatted (
  IN  BUILD_CONTEXT *Context,
  IN  UINT8         TableType,
  IN  UINT16        FieldOffset,
  IN  UINT16        FieldSize,
  OUT UINT8         *TableBase
  );


//
// Convenience / safety macro for patching a string in the unformatted area of
// an SMBIOS table.
//
#define PATCH_UNFORMATTED(Smbios, SmbiosHandle, Context, TableType,       \
          OvmfTablePtr, FieldName)                                        \
                                                                          \
          PatchSmbiosUnformatted (                                        \
            Smbios,                                                       \
            SmbiosHandle,                                                 \
            Context,                                                      \
            TableType,                                                    \
            (UINT16) OFFSET_OF (SMBIOS_TABLE_TYPE##TableType, FieldName), \
            (UINT8 *) (OvmfTablePtr)                                      \
            )


/**
  Apply a saved patch to a text string located in the unformatted area of an
  already installed SMBIOS table.

  The patch is looked up based on (Context, TableType, FieldOffset).

  @param[in]  Smbios        The EFI_SMBIOS_PROTOCOL instance used previously
                            for installing the SMBIOS table.
  @param[in]  SmbiosHandle  The EFI_SMBIOS_HANDLE previously returned by
                            Smbios->Add().
  @param[in]  Context       The BUILD_CONTEXT object storing saved patches.
  @param[in]  TableType     Selects the table type for which the patch has been
                            saved. It is assumed that the caller has validated
                            TableType against TABLE_TYPE_LIMIT (upper
                            exclusive).
  @param[in]  FieldOffset   Selects the SMBIOS field for which the patch has
                            been saved. It is assumed that the caller has
                            validated FieldOffset against FIELD_OFFSET_MINIMUM
                            (lower inclusive) and 255 (upper exclusive).
                            It is also assumed that TableBase[FieldOffset]
                            accesses a field of type SMBIOS_TABLE_STRING, ie. a
                            field in the formatted area that identifies an
                            existent text string in the unformatted area. Text
                            string identifiers are one-based.
  @param[out] TableBase     Base of the SMBIOS table of type TableType in which
                            the SMBIOS_TABLE_STRING field at FieldOffset
                            identifies the existent text string to update.

  @retval EFI_NOT_FOUND          No patch found for TableType:FieldOffset in
                                 Context. This return value is considered
                                 informative (ie. non-fatal).
  @retval EFI_INVALID_PARAMETER  Patch found for TableType:FieldOffset, but it
                                 doesn't end with a NUL character. This result
                                 is considered a fatal error of the patch
                                 origin.
  @retval EFI_SUCCESS            The text string identified by
                                 TableBase[FieldOffset] has been replaced in
                                 the installed SMBIOS table under SmbiosHandle.
  @return                        Error codes returned by
                                 Smbios->UpdateString(). EFI_NOT_FOUND shall
                                 not be returned.
**/
EFI_STATUS
EFIAPI
PatchSmbiosUnformatted (
  IN  EFI_SMBIOS_PROTOCOL *Smbios,
  IN  EFI_SMBIOS_HANDLE   SmbiosHandle,
  IN  BUILD_CONTEXT       *Context,
  IN  UINT8               TableType,
  IN  UINT16              FieldOffset,
  IN  UINT8               *TableBase
  );


/**
  Install default (fallback) table for SMBIOS Type 0.

  In case QEMU has provided no Type 0 SMBIOS table in whole, prepare one here,
  patch it with any referring saved patches, and install it.

  @param[in]     Smbios          The EFI_SMBIOS_PROTOCOL instance used for
                                 installing SMBIOS tables.
  @param[in]     ProducerHandle  Passed on to Smbios->Add(), ProducerHandle
                                 tracks the origin of installed SMBIOS tables.
  @param[in,out] Context         The BUILD_CONTEXT object tracking installed
                                 tables and saved patches.

  @retval EFI_SUCCESS  A Type 0 table has already been installed from the
                       SMBIOS firmware configuration blob.
  @retval EFI_SUCCESS  No Type 0 table was installed previously, and installing
                       the default here has succeeded.
  @return              Error codes from the PATCH_FORMATTED() and
                       PATCH_UNFORMATTED() macros, except EFI_NOT_FOUND, which
                       is only an informative result of theirs.
**/
EFI_STATUS
EFIAPI
InstallSmbiosType0 (
  IN     EFI_SMBIOS_PROTOCOL *Smbios,
  IN     EFI_HANDLE          ProducerHandle,
  IN OUT BUILD_CONTEXT       *Context
  );


/**
  Install default (fallback) table for SMBIOS Type 1.

  In case QEMU has provided no Type 1 SMBIOS table in whole, prepare one here,
  patch it with any referring saved patches, and install it.

  @param[in]     Smbios          The EFI_SMBIOS_PROTOCOL instance used for
                                 installing SMBIOS tables.
  @param[in]     ProducerHandle  Passed on to Smbios->Add(), ProducerHandle
                                 tracks the origin of installed SMBIOS tables.
  @param[in,out] Context         The BUILD_CONTEXT object tracking installed
                                 tables and saved patches.

  @retval EFI_SUCCESS  A Type 1 table has already been installed from the
                       SMBIOS firmware configuration blob.
  @retval EFI_SUCCESS  No Type 1 table was installed previously, and installing
                       the default here has succeeded.
  @return              Error codes from the PATCH_FORMATTED() and
                       PATCH_UNFORMATTED() macros, except EFI_NOT_FOUND, which
                       is only an informative result of theirs.
**/
EFI_STATUS
EFIAPI
InstallSmbiosType1 (
  IN     EFI_SMBIOS_PROTOCOL *Smbios,
  IN     EFI_HANDLE          ProducerHandle,
  IN OUT BUILD_CONTEXT       *Context
  );

#endif
