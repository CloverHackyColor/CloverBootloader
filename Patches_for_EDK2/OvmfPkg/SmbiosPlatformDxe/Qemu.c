/** @file
  This file fetches and installs SMBIOS tables on the QEMU hypervisor.

  Copyright (C) 2013, Red Hat, Inc.

  This program and the accompanying materials are licensed and made available
  under the terms and conditions of the BSD License which accompanies this
  distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS, WITHOUT
  WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/

#include <Library/MemoryAllocationLib.h>

#include "Qemu.h"
#include "QemuInternal.h"


//
// An SMBIOS entry exported by QEMU over fw_cfg can have one of the following
// types.
//
typedef enum
{
  ET_FIELD, // defines one field in some SMBIOS table
  ET_TABLE  // defines an SMBIOS table instance in entirety
} FW_CFG_SMBIOS_ENTRY_TYPE;

//
// Header type introducing each entry in the QemuFwCfgItemX86SmbiosTables
// fw_cfg blob.
//
#pragma pack(1)
typedef struct {
  UINT16 Size; // including payload and this header
  UINT8  Type; // value from FW_CFG_SMBIOS_ENTRY_TYPE
} FW_CFG_SMBIOS_ENTRY_HDR;
#pragma pack()

//
// Fields included at the beginning of the the payload in QEMU SMBIOS entries
// with ET_FIELD type.
//
#pragma pack(1)
typedef struct {
  UINT8  TableType; // SMBIOS table type to patch
  UINT16 Offset;    // offset of a field in the formatted area
} FW_CFG_SMBIOS_FIELD;
#pragma pack()


/**
  Initialize a context object tracking SMBIOS table installation and patches
  for fields.

  @param[out] Context  A BUILD_CONTEXT object allocated dynamically and
                       initialized.

  @retval EFI_OUT_OF_RESOURCES  Memory allocation failed.
  @retval EFI_SUCCESS           Allocation and initialization successful.
**/
STATIC
EFI_STATUS
EFIAPI
InitSmbiosContext (
  OUT BUILD_CONTEXT **Context
  )
{
  *Context = AllocateZeroPool (sizeof **Context);
  if (*Context == NULL) {
    DEBUG ((DEBUG_ERROR, "%a: out of memory\n", __FUNCTION__));
    return EFI_OUT_OF_RESOURCES;
  }
  return EFI_SUCCESS;
}


/**
  Release a context object tracking SMBIOS table installation and patches for
  fields.

  @param[in,out] Context  The BUILD_CONTEXT object to tear down.
**/
STATIC
VOID
EFIAPI
UninitSmbiosContext (
  IN OUT BUILD_CONTEXT *Context
  )
{
  INT32 Type;
  INT32 Idx;

  //
  // free all patches
  //
  for (Type = 0; Type < TABLE_TYPE_LIMIT; ++Type) {
    for (Idx = 0; Idx < PATCH_SUBSCRIPT_LIMIT; ++Idx) {
      PATCH *Patch;

      Patch = &Context->Table[Type].Patch[Idx];
      if (Patch->Base != NULL) {
        FreePool (Patch->Base);
      }
    }
  }
  FreePool (Context);
}


/**
  Save a patch targeting an SMBIOS field in dynamically allocated memory.

  @param[in,out] Context      The initialized BUILD_CONTEXT object to save the
                              patch in.
  @param[in]     TableType    The patch to be saved targets this table type.
                              Patches for table types equal to or greater than
                              TABLE_TYPE_LIMIT are ignored.
  @param[in]     FieldOffset  The patch to be saved targets the field that
                              begins at offset FieldOffset in SMBIOS table type
                              TableType. FieldOffset is enforced not to point
                              into the SMBIOS table header. A FieldOffset value
                              equal to or greater than 255 is rejected, since
                              the formatted area of an SMBIOS table never
                              exceeds 255 bytes. FieldOffset is not validated
                              against actual field offsets here, it is only
                              saved for later lookup.
  @param[in]     PatchData    Byte array constituting the patch body.
  @param[in]     PatchSize    Number of bytes in PatchData.

  @retval EFI_SUCCESS            Patch has been either ignored due to not
                                 meeting the criterion on TableType, or it has
                                 been saved successfully.
  @retval EFI_INVALID_PARAMETER  FieldOffset is invalid.
  @retval EFI_OUT_OF_RESOURCES   Couldn't allocate memory for the patch.
**/
STATIC
EFI_STATUS
EFIAPI
SaveSmbiosPatch (
  IN OUT BUILD_CONTEXT *Context,
  IN     UINT8         TableType,
  IN     UINT16        FieldOffset,
  IN     UINT8         *PatchData,
  IN     UINT16        PatchSize
)
{
  UINT8 *NewBase;
  PATCH *Patch;

  if (TableType >= TABLE_TYPE_LIMIT) {
    DEBUG ((DEBUG_VERBOSE,
      "%a: ignoring patch for unsupported table type %d\n",
      __FUNCTION__, TableType));
    return EFI_SUCCESS;
  }

  if (FieldOffset < FIELD_OFFSET_MINIMUM
      || FieldOffset - FIELD_OFFSET_MINIMUM >= PATCH_SUBSCRIPT_LIMIT) {
    DEBUG ((DEBUG_ERROR,
      "%a: invalid patch for table type %d field offset %d\n",
      __FUNCTION__, TableType, FieldOffset));
    return EFI_INVALID_PARAMETER;
  }

  NewBase = AllocateCopyPool (PatchSize, PatchData);
  if (PatchSize > 0 && NewBase == NULL) {
    DEBUG ((DEBUG_ERROR, "%a: table type %d field offset %d: out of memory\n",
      __FUNCTION__, TableType, FieldOffset));
    return EFI_OUT_OF_RESOURCES;
  }

  Patch = &Context->Table[TableType].Patch[FieldOffset - FIELD_OFFSET_MINIMUM];
  //
  // replace previous patch if it exists
  //
  if (Patch->Base != NULL) {
    DEBUG ((DEBUG_VERBOSE,
      "%a: replacing prior patch for table type %d field offset %d\n",
      __FUNCTION__, TableType, FieldOffset));
    FreePool (Patch->Base);
  }

  Patch->Base = NewBase;
  Patch->Size = PatchSize;
  return EFI_SUCCESS;
}


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
  )
{
  PATCH *Patch;

  ASSERT (TableType < TABLE_TYPE_LIMIT);
  ASSERT (FieldOffset >= FIELD_OFFSET_MINIMUM);
  ASSERT (FieldOffset - FIELD_OFFSET_MINIMUM < PATCH_SUBSCRIPT_LIMIT);

  Patch = &Context->Table[TableType].Patch[FieldOffset - FIELD_OFFSET_MINIMUM];
  if (Patch->Base == NULL) {
    return EFI_NOT_FOUND;
  }

  if (Patch->Size != FieldSize) {
    DEBUG ((DEBUG_ERROR, "%a: table type %d, field offset %d: "
      "patch size %d doesn't match field size %d\n",
      __FUNCTION__, TableType, FieldOffset, Patch->Size, FieldSize));
    return EFI_INVALID_PARAMETER;
  }

  CopyMem (TableBase + FieldOffset, Patch->Base, FieldSize);
  return EFI_SUCCESS;
}


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
  )
{
  PATCH      *Patch;
  UINTN      StringNumber;
  EFI_STATUS Status;

  ASSERT (TableType < TABLE_TYPE_LIMIT);
  ASSERT (FieldOffset >= FIELD_OFFSET_MINIMUM);
  ASSERT (FieldOffset - FIELD_OFFSET_MINIMUM < PATCH_SUBSCRIPT_LIMIT);

  Patch = &Context->Table[TableType].Patch[FieldOffset - FIELD_OFFSET_MINIMUM];
  if (Patch->Base == NULL) {
    return EFI_NOT_FOUND;
  }

  if (Patch->Size == 0 || Patch->Base[Patch->Size - 1] != '\0') {
    DEBUG ((DEBUG_ERROR, "%a: table type %d, field offset %d: "
      "missing terminator, or trailing garbage\n",
      __FUNCTION__, TableType, FieldOffset));
    return EFI_INVALID_PARAMETER;
  }

  StringNumber = TableBase[FieldOffset];
  ASSERT (StringNumber != 0);

  Status = Smbios->UpdateString (Smbios, &SmbiosHandle, &StringNumber,
                     (CHAR8 *)Patch->Base);
  if (EFI_ERROR (Status)) {
    ASSERT (Status != EFI_NOT_FOUND);
    DEBUG ((DEBUG_ERROR, "%a: table type %d, field offset %d, "
      "string number %d: Smbios->UpdateString(): %r\n", __FUNCTION__,
      TableType, FieldOffset, TableBase[FieldOffset], Status));
    return Status;
  }
  return EFI_SUCCESS;
}


/**
  Process an SMBIOS firmware configuration entry with ET_FIELD type, exported
  by QEMU under QemuFwCfgItemX86SmbiosTables.

  Such entries describe patches to be saved with SaveSmbiosPatch().

  @param[in,out] Context      The BUILD_CONTEXT object tracking saved patches.
  @param[in]     Payload      Points to the buffer to parse as
                              FW_CFG_SMBIOS_FIELD.
  @param[in]     PayloadSize  Number of bytes in Payload.

  @retval EFI_INVALID_PARAMETER  PayloadSize is less than the size of
                                 FW_CFG_SMBIOS_FIELD -- fields describing
                                 the patch are incomplete.
  @retval EFI_SUCCESS            Payload has been parsed and patch has been
                                 saved successfully.
  @return                        Error codes returned by SaveSmbiosPatch().
**/
STATIC
EFI_STATUS
EFIAPI
VisitSmbiosField (
  IN OUT BUILD_CONTEXT *Context,
  IN     UINT8         *Payload,
  IN     UINT16        PayloadSize
  )
{
  FW_CFG_SMBIOS_FIELD *Field;

  if (PayloadSize < (INT32) sizeof *Field) {
    DEBUG ((DEBUG_ERROR, "%a: required minimum size %d, available %d\n",
      __FUNCTION__, (INT32) sizeof *Field, PayloadSize));
    return EFI_INVALID_PARAMETER;
  }

  Field = (FW_CFG_SMBIOS_FIELD *) Payload;
  return SaveSmbiosPatch (Context, Field->TableType, Field->Offset,
           Payload + sizeof *Field, (UINT16) (PayloadSize - sizeof *Field));
}


/**
  Process an SMBIOS firmware configuration entry with ET_TABLE type, exported
  by QEMU under QemuFwCfgItemX86SmbiosTables.

  Such entries describe entire SMBIOS table instances to install verbatim. This
  module never overrides tables installed in this manner with default tables.

  @param[in]     Smbios          The EFI_SMBIOS_PROTOCOL instance used for
                                 installing SMBIOS tables.
  @param[in]     ProducerHandle  Passed on to Smbios->Add(), ProducerHandle
                                 tracks the origin of installed SMBIOS tables.
  @param[in,out] Context         The BUILD_CONTEXT object tracking installed
                                 tables.
  @param[in]     Payload         Points to the buffer to install as an SMBIOS
                                 table.
  @param[in]     PayloadSize     Number of bytes in Payload.

  @retval EFI_INVALID_PARAMETER  The buffer at Payload, interpreted as an
                                 SMBIOS table, failed basic sanity checks.
  @retval EFI_SUCCESS            Payload has been installed successfully as an
                                 SMBIOS table.
  @return                        Error codes returned by Smbios->Add().
**/
STATIC
EFI_STATUS
EFIAPI
VisitSmbiosTable (
  IN     EFI_SMBIOS_PROTOCOL *Smbios,
  IN     EFI_HANDLE          ProducerHandle,
  IN OUT BUILD_CONTEXT       *Context,
  IN     UINT8               *Payload,
  IN     UINT16              PayloadSize
  )
{
  SMBIOS_STRUCTURE  *SmbiosHeader;
  UINT16            MinimumSize;
  EFI_SMBIOS_HANDLE SmbiosHandle;
  EFI_STATUS        Status;

  //
  // Basic sanity checks only in order to help debugging and to catch blatantly
  // invalid data passed with "-smbios file=binary_file" on the QEMU command
  // line. Beyond these we don't enforce correct, type-specific SMBIOS table
  // formatting.
  //
  if (PayloadSize < (INT32) sizeof *SmbiosHeader) {
    DEBUG ((DEBUG_ERROR, "%a: required minimum size %d, available %d\n",
      __FUNCTION__, (INT32) sizeof *SmbiosHeader, PayloadSize));
    return EFI_INVALID_PARAMETER;
  }

  SmbiosHeader = (SMBIOS_STRUCTURE *) Payload;

  if (SmbiosHeader->Length < (INT32) sizeof *SmbiosHeader) {
    DEBUG ((DEBUG_ERROR, "%a: required minimum size %d, stated %d\n",
      __FUNCTION__, (INT32) sizeof *SmbiosHeader, SmbiosHeader->Length));
    return EFI_INVALID_PARAMETER;
  }

  MinimumSize = (UINT16) (SmbiosHeader->Length + 2);

  if (PayloadSize < MinimumSize) {
    DEBUG ((DEBUG_ERROR,
      "%a: minimum for formatted area plus terminator is %d, available %d\n",
      __FUNCTION__, MinimumSize, PayloadSize));
    return EFI_INVALID_PARAMETER;
  }

  if (Payload[PayloadSize - 2] != '\0' ||
      Payload[PayloadSize - 1] != '\0') {
    DEBUG ((DEBUG_ERROR, "%a: missing terminator, or trailing garbage\n",
      __FUNCTION__));
    return EFI_INVALID_PARAMETER;
  }

  //
  // request unique handle
  //
  SmbiosHandle = SMBIOS_HANDLE_PI_RESERVED;
  Status = Smbios->Add (Smbios, ProducerHandle, &SmbiosHandle,
                     (EFI_SMBIOS_TABLE_HEADER *) SmbiosHeader);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a: Smbios->Add(): %r\n", __FUNCTION__, Status));
    return Status;
  }

  //
  // track known tables
  //
  if (SmbiosHeader->Type < TABLE_TYPE_LIMIT) {
    Context->Table[SmbiosHeader->Type].Installed = TRUE;
  }
  return EFI_SUCCESS;
}


/**
  Traverse the SMBIOS firmware configuration blob exported by QEMU under
  QemuFwCfgItemX86SmbiosTables, processing each entry in turn.

  Entries with ET_FIELD type are parsed as patches for the SMBIOS tables this
  module installs as fallbacks, while entries of type ET_TABLE are parsed and
  installed as verbatim SMBIOS tables.

  Unknown entry types are silently skipped. Any error encountered during
  traversal (for example, a recognized but malformed entry) aborts the
  iteration, leaving the function with a possibly incomplete set of installed
  tables.

  @param[in]     Smbios          The EFI_SMBIOS_PROTOCOL instance used for
                                 installing SMBIOS tables.
  @param[in]     ProducerHandle  Passed on to Smbios->Add(), ProducerHandle
                                 tracks the origin of installed SMBIOS tables.
  @param[in,out] Context         The BUILD_CONTEXT object tracking installed
                                 tables and saved patches.

  @retval EFI_SUCCESS            The firmware configuration interface is
                                 unavailable (no patches saved, no tables
                                 installed).
  @retval EFI_SUCCESS            Traversal complete. Tables provided by QEMU
                                 have been installed. Patches have been saved
                                 for any default tables that will be necessary.
  @retval EFI_INVALID_PARAMETER  Encountered a corrupt entry in the SMBIOS
                                 firmware configuration blob.
  @retval EFI_OUT_OF_RESOURCES   Memory allocation failed.
  @return                        Error codes returned by VisitSmbiosField() and
                                 VisitSmbiosTable().
**/
STATIC
EFI_STATUS
EFIAPI
ScanQemuSmbios (
  IN     EFI_SMBIOS_PROTOCOL *Smbios,
  IN     EFI_HANDLE          ProducerHandle,
  IN OUT BUILD_CONTEXT       *Context
  )
{
  EFI_STATUS Status;
  UINT16     NumEntries;
  UINT16     CurEntry;

  Status = EFI_SUCCESS;

  if (!QemuFwCfgIsAvailable ()) {
    return Status;
  }

  QemuFwCfgSelectItem (QemuFwCfgItemX86SmbiosTables);

  NumEntries = QemuFwCfgRead16 ();
  for (CurEntry = 0; CurEntry < NumEntries && !EFI_ERROR (Status);
       ++CurEntry) {
    FW_CFG_SMBIOS_ENTRY_HDR Header;
    UINT16                  PayloadSize;
    UINT8                   *Payload;

    QemuFwCfgReadBytes (sizeof Header, &Header);

    if (Header.Size < (INT32) sizeof Header) {
      DEBUG ((DEBUG_ERROR, "%a: invalid header size %d in entry %d\n",
        __FUNCTION__, Header.Size, CurEntry));
      return EFI_INVALID_PARAMETER;
    }

    PayloadSize = (UINT16) (Header.Size - sizeof Header);
    Payload = AllocatePool (PayloadSize);

    if (PayloadSize > 0 && Payload == NULL) {
      DEBUG ((DEBUG_ERROR, "%a: failed to allocate %d bytes for entry %d\n",
        __FUNCTION__, PayloadSize, CurEntry));
      return EFI_OUT_OF_RESOURCES;
    }

    QemuFwCfgReadBytes (PayloadSize, Payload);

    //
    // dump the payload
    //
    DEBUG_CODE (
      UINT16 Idx;

      DEBUG ((DEBUG_VERBOSE,
        "%a: entry %d, type %d, payload size %d, payload hex dump follows:",
        __FUNCTION__, CurEntry, Header.Type, PayloadSize));
      for (Idx = 0; Idx < PayloadSize; ++Idx) {
        switch (Idx % 16) {
          case 0:
            DEBUG ((DEBUG_VERBOSE, "\n%04X:", Idx));
            break;
          case 8:
            DEBUG ((DEBUG_VERBOSE, " "));
            break;
          default:
            ;
        }
        DEBUG ((DEBUG_VERBOSE, " %02X", Payload[Idx]));
      }
      DEBUG ((DEBUG_VERBOSE, "\n"));
    );

    switch (Header.Type) {
    case ET_FIELD:
      Status = VisitSmbiosField (Context, Payload, PayloadSize);
      break;
    case ET_TABLE:
      Status = VisitSmbiosTable (Smbios, ProducerHandle, Context, Payload,
                 PayloadSize);
      break;
    default:
      ;
    }

    FreePool (Payload);
  }

  return Status;
}


/**
  Install some of the default SMBIOS tables for table types that QEMU hasn't
  provided under QemuFwCfgItemX86SmbiosTables, but are required by the
  SMBIOS-2.7.1 specification.

  @param[in]     Smbios          The EFI_SMBIOS_PROTOCOL instance used for
                                 installing SMBIOS tables.
  @param[in]     ProducerHandle  Passed on to Smbios->Add(), ProducerHandle
                                 tracks the origin of installed SMBIOS tables.
  @param[in,out] Context         The BUILD_CONTEXT object tracking installed
                                 tables and saved patches.

  @return  Status codes returned by the InstallSmbiosTypeXX() functions,
           including the final EFI_SUCCESS if all such calls succeed.
**/
STATIC
EFI_STATUS
EFIAPI
InstallDefaultTables (
  IN     EFI_SMBIOS_PROTOCOL *Smbios,
  IN     EFI_HANDLE          ProducerHandle,
  IN OUT BUILD_CONTEXT       *Context
  )
{
  EFI_STATUS Status;

  Status = InstallSmbiosType0 (Smbios, ProducerHandle, Context);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = InstallSmbiosType1 (Smbios, ProducerHandle, Context);
  return Status;
}


/**
  Fetch and install SMBIOS tables on the QEMU hypervisor.

  First, tables provided by QEMU in entirety are installed verbatim.

  Then the function prepares some of the remaining tables required by the
  SMBIOS-2.7.1 specification. For each such table,
  - if QEMU provides any fields for the table, they take effect verbatim,
  - remaining fields are set by this function.

  @param[in] Smbios       The EFI_SMBIOS_PROTOCOL instance used for installing
                          the SMBIOS tables.
  @param[in] ImageHandle  The image handle of the calling module, passed as
                          ProducerHandle to the Smbios->Add() call.

  @retval EFI_SUCCESS            All tables have been installed.
  @retval EFI_UNSUPPORTED        The pair (Smbios->MajorVersion,
                                 Smbios->MinorVersion) precedes (2, 3)
                                 lexicographically.
  @return                        Error codes returned by Smbios->Add() or
                                 internal functions. Some tables may not have
                                 been installed or fully patched.
**/
EFI_STATUS
EFIAPI
InstallQemuSmbiosTables (
  IN EFI_SMBIOS_PROTOCOL *Smbios,
  IN EFI_HANDLE          ImageHandle
  )
{
  EFI_STATUS    Status;
  BUILD_CONTEXT *Context;

  if (Smbios->MajorVersion < 2 || Smbios->MinorVersion < 3) {
    DEBUG ((DEBUG_ERROR, "%a: unsupported Smbios version %d.%d\n",
      __FUNCTION__, Smbios->MajorVersion, Smbios->MinorVersion));
    return EFI_UNSUPPORTED;
  }

  Status = InitSmbiosContext (&Context);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // <IndustryStandard/SmBios.h> and <Protocol/Smbios.h> must agree.
  //
  ASSERT (sizeof(SMBIOS_STRUCTURE) == sizeof(EFI_SMBIOS_TABLE_HEADER));

  Status = ScanQemuSmbios (Smbios, ImageHandle, Context);
  if (EFI_ERROR (Status)) {
    goto Cleanup;
  }

  Status = InstallDefaultTables (Smbios, ImageHandle, Context);

Cleanup:
  UninitSmbiosContext (Context);
  return Status;
}
