//
//  OSInfo.c
//  Clover
//
//  Created by Slice on 17.10.16.
//


#include <Library/BaseLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/MemLogLib.h>

#include <Protocol/OSInfo.h>

EFI_HANDLE              mHandle = NULL;

#define EFI_OS_INFO_PROTOCOL_REVISION  0x01

// OS_INFO_VENDOR_NAME
#define OS_INFO_VENDOR_NAME  "Apple Inc."

// OSInfoOSNameImpl
VOID
EFIAPI
OSInfoOSNameImpl (
                  OUT CHAR8 *OSName
                  )
{
  
}

// OSInfoOSVendorImpl
VOID
EFIAPI
OSInfoOSVendorImpl (
                    IN CHAR8 *OSVendor
                    )
{
  INTN Result;
  if (!OSVendor) {
    return;
  }
  Result = AsciiStrCmp (OSVendor, OS_INFO_VENDOR_NAME);
  
  if (Result == 0) {
 //   EfiLibNamedEventSignal (&gAppleOsLoadedNamedEventGuid);
  }
  
}

VOID
EFIAPI
EmptyImpl (
                  OUT CHAR8 *OSName
                  )
{

}


// mEfiOSInfo
EFI_OS_INFO_PROTOCOL mEfiOSInfo = {
  EFI_OS_INFO_PROTOCOL_REVISION,
  OSInfoOSNameImpl,
  OSInfoOSVendorImpl,
  EmptyImpl
};


/****************************************************************
 * Entry point
 ***************************************************************/

/**
 * SMCHelper entry point. Installs AppleOSInfoProtocol.
 */
EFI_STATUS
EFIAPI
OSInfoEntrypoint (
                     IN EFI_HANDLE           ImageHandle,
                     IN EFI_SYSTEM_TABLE		*SystemTable
                     )
{
  EFI_STATUS					Status; // = EFI_SUCCESS;
  EFI_BOOT_SERVICES*			gBS;
  
  gBS				= SystemTable->BootServices;
  
  Status = gBS->InstallMultipleProtocolInterfaces (
                                                   &mHandle,
                                                   &gEfiOSInfoProtocolGuid,
                                                   &mEfiOSInfo,
                                                   NULL
                                                   );
  
  return Status;
}
