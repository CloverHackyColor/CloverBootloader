//
//  GraphicsConfig.c
//  Clover
//
//  Created by Slice on 18.10.16.
//

#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>

//#include <Protocol/UgaDraw.h>
#include <Protocol/AppleGraphConfig.h>

EFI_HANDLE              mHandle = NULL;
/*
VOID
EFIAPI
EmptyImpl (
           IN UINTN Unknown
           )
{
  
}
*/
EFI_STATUS
EFIAPI
RestoreConfig (APPLE_GRAPH_CONFIG_PROTOCOL* This,
               UINT32 Param1, UINT32 Param2, VOID* Param3, VOID* Param4, VOID* Param5
               )
{
  return EFI_SUCCESS;
}


APPLE_GRAPH_CONFIG_PROTOCOL mGraphConfig = {
  1,
  RestoreConfig,
};


/****************************************************************
 * Entry point
 ***************************************************************/

/**
 * GraphicsConfig entry point. Installs AppleGraphConfigProtocol.
 */
EFI_STATUS
EFIAPI
GraphicsConfigEntrypoint (
                  IN EFI_HANDLE           ImageHandle,
                  IN EFI_SYSTEM_TABLE		*SystemTable
                  )
{
  EFI_STATUS					Status; // = EFI_SUCCESS;
  EFI_BOOT_SERVICES*			gBS;  
  gBS				= SystemTable->BootServices;
  
  Status = gBS->InstallMultipleProtocolInterfaces (
                                                   &mHandle,
                                                   &gAppleGraphConfigProtocolGuid,
                                                   &mGraphConfig,
                                                   NULL
                                                   );
  
  return Status;
}
