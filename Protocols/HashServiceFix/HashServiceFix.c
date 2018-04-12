/**
 * Hash service fix for AMI EFIs with broken SHA implementations.
 *
 * If an existing SHA1 implementation is found and produces wrong hashes,
 * is is first unregistered and then replaced with a working one. This
 * replacement protocol only implements SHA1.
 * 
 * Author: Joel Höner <athre0z@zyantific.com>
 */

#include <Protocol/ServiceBinding.h>
#include <Protocol/Hash.h>

#include "sha1.h"

/* ===================================================================== */
/* [Hash Protocol]                                                            */
/* ===================================================================== */

typedef struct _HS_PRIVATE_DATA
{
  SHA1_CTX Sha1Ctx;
  EFI_HASH_PROTOCOL Proto;
} HS_PRIVATE_DATA;

#define HS_PRIVATE_FROM_PROTO(x) \
  (HS_PRIVATE_DATA*)   \
  ((UINTN)x - OFFSET_OF(HS_PRIVATE_DATA, Proto))

EFI_STATUS
EFIAPI
HSGetHashSize(
  IN  CONST EFI_HASH_PROTOCOL *This,
  IN  CONST EFI_GUID          *HashAlgorithm,
  OUT UINTN                   *HashSize
  )
{
  if (!HashAlgorithm || !HashSize) {
    return EFI_INVALID_PARAMETER;
  }

  if (CompareGuid(&gEfiHashAlgorithmSha1Guid, HashAlgorithm)) {
    *HashSize = sizeof(EFI_SHA1_HASH);
    return EFI_SUCCESS;
  }

  return EFI_UNSUPPORTED;
}

EFI_STATUS
EFIAPI
HSHash(
  IN CONST EFI_HASH_PROTOCOL  *This,
  IN CONST EFI_GUID           *HashAlgorithm,
  IN BOOLEAN                  Extend,
  IN CONST UINT8              *Message,
  IN UINT64                   MessageSize,
  IN OUT EFI_HASH_OUTPUT      *Hash
  )
{
  HS_PRIVATE_DATA *PrivateData;
  SHA1_CTX        CtxCopy;

  if (!This || !HashAlgorithm || !Message || !Hash || !MessageSize) {
    return EFI_INVALID_PARAMETER;
  }
  if (!CompareGuid(&gEfiHashAlgorithmSha1Guid, HashAlgorithm)) {
    return EFI_UNSUPPORTED;
  }

  PrivateData = HS_PRIVATE_FROM_PROTO(This);
  if (!Extend) {
    SHA1Init(&PrivateData->Sha1Ctx);
  }

  SHA1Update(&PrivateData->Sha1Ctx, Message, MessageSize);

  // SHA1Final clears the context, so we need to create a copy 
  // in order to be able to support later updates.
  CopyMem(&CtxCopy, &PrivateData->Sha1Ctx, sizeof CtxCopy);
  SHA1Final(*Hash->Sha1Hash, &CtxCopy);

  return EFI_SUCCESS;  
}

EFI_HASH_PROTOCOL HashProto = {
  &HSGetHashSize,
  &HSHash
};

/* ========================================================================== */
/* [Hash Binding Protocol]                                                    */
/* ========================================================================== */

EFI_STATUS
EFIAPI
HSCreateChild(
  IN     EFI_SERVICE_BINDING_PROTOCOL  *This,
  IN OUT EFI_HANDLE                    *ChildHandle
  )
{
  HS_PRIVATE_DATA *PrivateData;
  EFI_STATUS      Status;

  PrivateData = AllocateZeroPool(sizeof *PrivateData);
  if (!PrivateData) {
    return EFI_OUT_OF_RESOURCES;
  }

  CopyMem(&PrivateData->Proto, &HashProto, sizeof HashProto);

  Status = gBS->InstallProtocolInterface(
    ChildHandle,
    &gEfiHashProtocolGuid,
    EFI_NATIVE_INTERFACE,
    &PrivateData->Proto
  );

  if (EFI_ERROR(Status)) {
    FreePool(PrivateData);
  }

  return Status;
}

EFI_STATUS
EFIAPI
HSDestroyChild(
  IN EFI_SERVICE_BINDING_PROTOCOL *This,
  IN EFI_HANDLE                   ChildHandle
  )
{
  EFI_HASH_PROTOCOL *HashProto;
  EFI_STATUS        Status;

  if (!This || !ChildHandle) {
    return EFI_INVALID_PARAMETER;
  }

  Status = gBS->HandleProtocol(
    ChildHandle,
    &gEfiHashProtocolGuid,
    (VOID**) &HashProto
  );

  if (EFI_ERROR(Status)) {
    return Status;
  }

  Status = gBS->UninstallProtocolInterface(
    ChildHandle,
    &gEfiHashProtocolGuid,
    HashProto
  );

  if (EFI_ERROR(Status)) {
    return Status;
  }

  FreePool(HS_PRIVATE_FROM_PROTO(HashProto));
  return EFI_SUCCESS;
}

EFI_SERVICE_BINDING_PROTOCOL HashBindingProto = {
  &HSCreateChild,
  &HSDestroyChild
};

BOOLEAN
EFIAPI
HSTestExistingShaImpl(BOOLEAN *ImplExists)
{
  EFI_SERVICE_BINDING_PROTOCOL  *BindingProto;
  EFI_HASH_PROTOCOL             *HashProto;
  EFI_HASH_OUTPUT               HashOut;
  EFI_SHA1_HASH                 Sha1Hash;
  EFI_STATUS                    Status;
  EFI_HANDLE                    Child;
  UINTN                         HashIter;

  //
  // Create binding.
  //
  Status = gBS->LocateProtocol(
    &gEfiHashServiceBindingProtocolGuid,
    NULL,
    (VOID **)&BindingProto
  );

  if (EFI_ERROR(Status)) {
    *ImplExists = FALSE;
    return FALSE;
  }
  
  Child = NULL;
  *ImplExists = TRUE;

  Status = BindingProto->CreateChild(BindingProto, &Child);
  if (EFI_ERROR(Status)) {
    return FALSE;
  }

  //
  // Obtain protocol.
  //
  HashProto = NULL;
  Status = gBS->LocateProtocol(
    &gEfiHashProtocolGuid, 
    NULL, 
    (VOID **)&HashProto
  );

  if (EFI_ERROR(Status)) {
    BindingProto->DestroyChild(BindingProto, Child);
    return FALSE;
  }
  
  //
  // Do two iterations of test-hashing.
  //
  HashOut.Sha1Hash = &Sha1Hash;
  for (HashIter = 0; HashIter < 2; ++HashIter) {
    Status = HashProto->Hash(
      HashProto, 
      &gEfiHashAlgorithmSha1Guid, 
      (BOOLEAN)HashIter, 
      (CONST UINT8 *)"ABCDEFGHIJKLMNOP", 
      16, 
      &HashOut
    );

    if (EFI_ERROR(Status)) {
      BindingProto->DestroyChild(BindingProto, Child);
      return FALSE;
    }
  }

  BindingProto->DestroyChild(BindingProto, Child);

  //
  // Verify result.
  //
  if (CompareMem(
    Sha1Hash,
    "\x14\x80\x6E\x23\xB4\xCE\xB6\x5D\xDF\x01"
    "\xE5\xEA\x7F\xBC\xDD\x03\xAA\xFA\xF5\xCD",
    sizeof Sha1Hash
    )) {
    return FALSE;
  }
  
  return TRUE;
}

/* ========================================================================== */
/* [Entry Point]                                                              */
/* ========================================================================== */

EFI_STATUS
EFIAPI
HSEntryPoint(
  IN EFI_HANDLE       ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
{
  EFI_SERVICE_BINDING_PROTOCOL  *BrokenProto;
  EFI_STATUS                    Status;
  EFI_HANDLE                    BrokenImpl;
  BOOLEAN                       CurImplWorks;
  BOOLEAN                       CurImplExists;
  UINTN                         NumHandles;

  // Does the currently registered (if any) SHA implementation work?
  CurImplWorks = HSTestExistingShaImpl(&CurImplExists);

  if (CurImplWorks) {
    // Great! Nothing to do here.
    return EFI_SUCCESS;
  }

  if (CurImplExists) {
    // There is an implementation registered, but it doesn't work.
    // Uninstall it.
    NumHandles = sizeof NumHandles;
    while (!EFI_ERROR(gBS->LocateHandle(
        ByProtocol,
        &gEfiHashServiceBindingProtocolGuid,
        NULL,
        &NumHandles,
        &BrokenImpl
      ))) {

      Status = gBS->HandleProtocol(
        BrokenImpl,
        &gEfiHashServiceBindingProtocolGuid,
        (VOID **)&BrokenProto
      );

      if (EFI_ERROR(Status)) {
        return Status;
      }

      Status = gBS->UninstallProtocolInterface(
        BrokenImpl,
        &gEfiHashServiceBindingProtocolGuid,
        BrokenProto
      );

      if (EFI_ERROR(Status)) {
        return Status;
      }

      NumHandles = sizeof NumHandles;
    }
  }

  return gBS->InstallProtocolInterface(
    &ImageHandle,
    &gEfiHashServiceBindingProtocolGuid,
    EFI_NATIVE_INTERFACE,
    &HashBindingProto
  );
}

