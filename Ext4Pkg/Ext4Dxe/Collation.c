/** @file
  Unicode collation routines

  Copyright (c) 2021 - 2023 Pedro Falcato All rights reserved.
  Copyright (c) 2005 - 2017, Intel Corporation. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Uefi.h>

#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>

#include <Protocol/UnicodeCollation.h>

STATIC EFI_UNICODE_COLLATION_PROTOCOL  *gUnicodeCollationInterface = NULL;

/*
 * Note: This code is heavily based on FatPkg's Unicode collation, since they seem to know what
 * they're doing.
 * PS: Maybe all this code could be put in a library? It looks heavily shareable.
**/

/**
   Check if unicode collation is initialized

   @retval TRUE if Ext4InitialiseUnicodeCollation() was already called successfully
   @retval FALSE if Ext4InitialiseUnicodeCollation() was not yet called successfully
**/
STATIC
BOOLEAN
Ext4IsCollationInitialized (
  VOID
  )
{
  return gUnicodeCollationInterface != NULL;
}

/**
  Worker function to initialize Unicode Collation support.

  It tries to locate Unicode Collation (2) protocol and matches it with current
  platform language code.

  @param[in]  DriverHandle         The handle used to open Unicode Collation (2) protocol.
  @param[in]  ProtocolGuid         The pointer to Unicode Collation (2) protocol GUID.
  @param[in]  VariableName         The name of the RFC 4646 or ISO 639-2 language variable.
  @param[in]  DefaultLanguage      The default language in case the RFC 4646 or ISO 639-2 language is absent.

  @retval EFI_SUCCESS          The Unicode Collation (2) protocol has been successfully located.
  @retval Others               The Unicode Collation (2) protocol has not been located.

**/
STATIC
EFI_STATUS
Ext4InitialiseUnicodeCollationInternal (
  IN EFI_HANDLE    DriverHandle,
  IN EFI_GUID      *ProtocolGuid,
  IN CONST CHAR16  *VariableName,
  IN CONST CHAR8   *DefaultLanguage
  )
{
  UINTN                           NumHandles;
  EFI_HANDLE                      *Handles;
  EFI_UNICODE_COLLATION_PROTOCOL  *Uci;
  BOOLEAN                         Iso639Language;
  CHAR8                           *Language;
  EFI_STATUS                      RetStatus;
  EFI_STATUS                      Status;
  UINTN                           Idx;
  CHAR8                           *BestLanguage;

  Iso639Language = (BOOLEAN)(ProtocolGuid == &gEfiUnicodeCollationProtocolGuid);
  RetStatus      = EFI_UNSUPPORTED;
  GetEfiGlobalVariable2 (VariableName, (VOID **)&Language, NULL);

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  ProtocolGuid,
                  NULL,
                  &NumHandles,
                  &Handles
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  for (Idx = 0; Idx < NumHandles; Idx++) {
    Status = gBS->OpenProtocol (
                    Handles[Idx],
                    ProtocolGuid,
                    (VOID **)&Uci,
                    DriverHandle,
                    NULL,
                    EFI_OPEN_PROTOCOL_GET_PROTOCOL
                    );

    if (EFI_ERROR (Status)) {
      continue;
    }

    BestLanguage = GetBestLanguage (
                     Uci->SupportedLanguages,
                     Iso639Language,
                     (Language == NULL) ? "" : Language,
                     DefaultLanguage,
                     NULL
                     );
    if (BestLanguage != NULL) {
      FreePool (BestLanguage);
      gUnicodeCollationInterface = Uci;
      RetStatus                  = EFI_SUCCESS;
      break;
    }
  }

  if (Language != NULL) {
    FreePool (Language);
  }

  FreePool (Handles);
  return RetStatus;
}

/**
   Initialises Unicode collation, which is needed for case-insensitive string comparisons
   within the driver (a good example of an application of this is filename comparison).

   @param[in]      DriverHandle    Handle to the driver image.

   @retval EFI_SUCCESS   Unicode collation was successfully initialised.
   @retval !EFI_SUCCESS  Failure.
**/
EFI_STATUS
Ext4InitialiseUnicodeCollation (
  EFI_HANDLE  DriverHandle
  )
{
  EFI_STATUS  Status;

  Status = EFI_UNSUPPORTED;

  // If already done, just return success.
  if (Ext4IsCollationInitialized ()) {
    return EFI_SUCCESS;
  }

  //
  // First try to use RFC 4646 Unicode Collation 2 Protocol.
  //
  Status = Ext4InitialiseUnicodeCollationInternal (
             DriverHandle,
             &gEfiUnicodeCollation2ProtocolGuid,
             L"PlatformLang",
             (CONST CHAR8 *)PcdGetPtr (PcdUefiVariableDefaultPlatformLang)
             );
  //
  // If the attempt to use Unicode Collation 2 Protocol fails, then we fall back
  // on the ISO 639-2 Unicode Collation Protocol.
  //
  if (EFI_ERROR (Status)) {
    Status = Ext4InitialiseUnicodeCollationInternal (
               DriverHandle,
               &gEfiUnicodeCollationProtocolGuid,
               L"Lang",
               (CONST CHAR8 *)PcdGetPtr (PcdUefiVariableDefaultLang)
               );
  }

  return Status;
}

/**
   Does a case-insensitive string comparison. Refer to EFI_UNICODE_COLLATION_PROTOCOL's StriColl
   for more details.

   @param[in]      Str1   Pointer to a null terminated string.
   @param[in]      Str2   Pointer to a null terminated string.

   @retval 0   Str1 is equivalent to Str2.
   @retval >0  Str1 is lexically greater than Str2.
   @retval <0  Str1 is lexically less than Str2.
**/
INTN
Ext4StrCmpInsensitive (
  IN CHAR16  *Str1,
  IN CHAR16  *Str2
  )
{
  ASSERT (gUnicodeCollationInterface != NULL);
  return gUnicodeCollationInterface->StriColl (gUnicodeCollationInterface, Str1, Str2);
}
