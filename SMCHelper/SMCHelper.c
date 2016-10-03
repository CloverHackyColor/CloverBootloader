/*
 *  SMCHelper.c
 *  
 *  Created by Slice on 03.10.2016.
 *
 */

#include <Library/BaseLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/MemLogLib.h>

#include <Protocol/AppleSMC.h>

// DBG_TO: 0=no debug, 1=serial, 2=console 3=log
// serial requires
// [PcdsFixedAtBuild]
//  gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x07
//  gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0xFFFFFFFF
// in package DSC file

#define DBG_SMC 3

#if DBG_SMC == 3
#define DBG(...) MemLog(FALSE, 0, __VA_ARGS__)
#elif DBG_SMC == 2
#define DBG(...) AsciiPrint(__VA_ARGS__)
#elif DBG_SMC == 1
#define DBG(...) DebugPrint(1, __VA_ARGS__)
#else
#define DBG(...)
#endif


#define APPLE_SMC_SIGNATURE SIGNATURE_64('A','P','P','L','E','S','M','C')

EFI_HANDLE              mHandle = NULL;

typedef struct _SMC_STACK SMC_STACK;

struct _SMC_STACK {
  SMC_STACK *Next;
  UINT32 Id;
  UINT8 *Data;
};

SMC_STACK *SmcStack = NULL;

CHAR8 *StringId(UINT32 DataId)
{
  CHAR8 *Str;
  Str = AllocatePool(5);
  Str[4] = '\0';
  Str[3] = DataId & 0xFF;
  Str[2] = (DataId >> 8) & 0xFF;
  Str[1] = (DataId >> 16) & 0xFF;
  Str[0] = (DataId >> 24) & 0xFF;
  return Str;
}

EFI_STATUS EFIAPI
ReadData (IN APPLE_SMC_PROTOCOL* This, IN UINT32 DataId, IN UINT32 DataLength, IN VOID* DataBuffer)
{
  CHAR8 *Str = StringId(DataId);
  DBG("asked for SMC=%x (%a) len=%d\n", DataId, Str, DataLength);
  FreePool(Str);
  return EFI_NOT_FOUND;
}
 
EFI_STATUS EFIAPI
WriteData (IN APPLE_SMC_PROTOCOL* This, IN UINT32 DataId, IN UINT32 DataLength, IN VOID* DataBuffer)
{
  return EFI_NOT_FOUND;
}

APPLE_SMC_PROTOCOL SMCHelperProtocol = {
  APPLE_SMC_SIGNATURE,
  ReadData,
  WriteData,
};

/****************************************************************
 * Entry point
 ***************************************************************/

/**
 * SMCHelper entry point. Installs AppleSMCProtocol.
 */
EFI_STATUS
EFIAPI
SMCHelperEntrypoint (
                    IN EFI_HANDLE           ImageHandle,
                    IN EFI_SYSTEM_TABLE		*SystemTable
                    )
{
  EFI_STATUS					Status; // = EFI_SUCCESS;
  EFI_BOOT_SERVICES*			gBS; 
  
  gBS				= SystemTable->BootServices;

  
  Status = gBS->InstallMultipleProtocolInterfaces (
                &mHandle,
                &gAppleSMCProtocolGuid,
                &SMCHelperProtocol,
                NULL
                );
  
  return Status;
}
