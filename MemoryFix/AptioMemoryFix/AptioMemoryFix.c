/**

 UEFI driver for enabling loading of macOS without memory relocation.

 by dmazar

 **/

#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Guid/GlobalVariable.h>

#include <Protocol/AptioMemoryFix.h>

#include "Config.h"
#include "BootArgs.h"
#include "BootFixes.h"
#include "CustomSlide.h"
#include "RtShims.h"
#include "ServiceOverrides.h"
#include "VMem.h"

//
// One could discover AptioMemoryFix with this protocol
//
STATIC APTIOMEMORYFIX_PROTOCOL mAptioMemoryFixProtocol = {
  APTIOMEMORYFIX_PROTOCOL_REVISION,
  SetBootVariableRedirect
};

/**
 * Entry point. Installs our StartImage override.
 * All other stuff will be installed from there when boot.efi is started.
 */
EFI_STATUS
EFIAPI
AptioMemoryFixEntrypoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS  Status;
  VOID        *Interface;
  EFI_HANDLE  Handle = NULL;

  Status = gBS->LocateProtocol (
    &gAptioMemoryFixProtocolGuid,
    NULL,
    &Interface
    );

  if (!EFI_ERROR(Status)) {
    //
    // In case for whatever reason one tried to reload the driver.
    //
    return EFI_ALREADY_STARTED;
  }

  Status = gBS->InstallProtocolInterface (
    &Handle,
    &gAptioMemoryFixProtocolGuid,
    EFI_NATIVE_INTERFACE,
    &mAptioMemoryFixProtocol
    );

  if (EFI_ERROR(Status)) {
    Print(L"AMF: protocol install failure - %r\n", Status);
    return Status;
  }

  //
  // Detect and apply the necessary firmware workarounds
  //
  ApplyFirmwareQuirks (ImageHandle, SystemTable);

  //
  // Init VMem memory pool - will be used after ExitBootServices
  //
  Status = VmAllocateMemoryPool ();
  if (EFI_ERROR(Status)) {
    return Status;
  }

  //
  // Install permanent shims and overrides
  //
  InstallRtShims (GetVariableCustomSlide);
  InstallBsOverrides ();
  InstallRtOverrides ();

  return EFI_SUCCESS;
}
