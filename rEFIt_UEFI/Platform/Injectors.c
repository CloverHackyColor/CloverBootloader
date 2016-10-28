/*
 * Copyright (C) 2010 Oracle Corporation
 *
 * This file is part of VirtualBox Open Source Edition (OSE), as
 * available from http://www.virtualbox.org. This file is free software;
 * you can redistribute it and/or modify it under the terms of the GNU
 * General Public License (GPL) as published by the Free Software
 * Foundation, in version 2 as it comes in the "COPYING" file of the
 * VirtualBox OSE distribution. VirtualBox OSE is distributed in the
 * hope that it will be useful, but WITHOUT ANY WARRANTY of any kind.
 */
/*
 Slice 2011, 
 some more adoptations for Apple's OS
 */

/*******************************************************************************
 *   Header Files                                                               *
 *******************************************************************************/
#include "Platform.h"
#include <Protocol/OSInfo.h>
#include <Protocol/AppleGraphConfig.h>
#include <Protocol/KeyboardInfo.h>

EFI_GUID gDevicePropertiesGuid = {
  0x91BD12FE, 0xF6C3, 0x44FB, {0xA5, 0xB7, 0x51, 0x22, 0xAB, 0x30, 0x3A, 0xE0}
};
/*
EFI_GUID gAppleScreenInfoProtocolGuid = {
	0xe316e100, 0x0751, 0x4c49, {0x90, 0x56, 0x48, 0x6c, 0x7e, 0x47, 0x29, 0x03}
}; */
// gEfiKeyboardInfoProtocolGuid
// {0xE82A0A1E, 0x0E4D, 0x45AC, {0xA6, 0xDC, 0x2A, 0xE0, 0x58, 0x00, 0xD3, 0x11}}

// C5C5DA95-7D5C-45E6-B2F1-3FD52BB10077 - EfiOSInfo
// 03622D6D-362A-4E47-9710-C238B23755C1 - GraphConfig

extern EFI_GUID gAppleScreenInfoProtocolGuid;

UINT32 mPropSize = 0;
UINT8* mProperties = NULL;
CHAR8* gDeviceProperties = NULL;

UINT32 cPropSize = 0;
UINT8* cProperties = NULL;
CHAR8* cDeviceProperties = NULL;
CHAR8* BootOSName = NULL;

UINT16 KeyboardVendor = 0x05ac; //Apple inc.
UINT16 KeyboardProduct = 0x021d; //iMac aluminium

typedef struct _APPLE_GETVAR_PROTOCOL APPLE_GETVAR_PROTOCOL;

typedef
EFI_STATUS
(EFIAPI *APPLE_GETVAR_PROTOCOL_GET_DEVICE_PROPS) (
                                                  IN     APPLE_GETVAR_PROTOCOL   *This,
                                                  IN     CHAR8                   *Buffer,
                                                  IN OUT UINT32                  *BufferSize);

struct _APPLE_GETVAR_PROTOCOL {
  UINT64    Sign;
  EFI_STATUS(EFIAPI *Unknown1)(IN VOID *);              //GetPropertyValue
  EFI_STATUS(EFIAPI *Unknown2)(IN VOID *);              //SetProperty
  EFI_STATUS(EFIAPI *Unknown3)(IN VOID *);              //RemoveProperty
  APPLE_GETVAR_PROTOCOL_GET_DEVICE_PROPS  GetDevProps;  //GetPropertyBuffer
  APPLE_GETVAR_PROTOCOL_GET_DEVICE_PROPS  GetDevProps1;
};


#define DEVICE_PROPERTIES_SIGNATURE SIGNATURE_64('A','P','P','L','E','D','E','V')

EFI_STATUS EFIAPI
GetDeviceProps(IN     APPLE_GETVAR_PROTOCOL   *This,
               IN     CHAR8                   *Buffer,
               IN OUT UINT32                  *BufferSize)
{ 

  if(!gSettings.StringInjector && (mProperties != NULL) && (mPropSize > 1)) {
    if (*BufferSize < mPropSize) {
      *BufferSize = mPropSize;
      return EFI_BUFFER_TOO_SMALL;
    }
    *BufferSize = mPropSize;
    CopyMem(Buffer, mProperties,  mPropSize);
    return EFI_SUCCESS;      
  } else if ((cProperties != NULL) && (cPropSize > 1)) {
    if (*BufferSize < cPropSize) {
      *BufferSize = cPropSize;
      return EFI_BUFFER_TOO_SMALL;
    }
    *BufferSize = cPropSize;
    CopyMem(Buffer, cProperties,  cPropSize);
    return EFI_SUCCESS;      
  }
  *BufferSize = 0;    
	return EFI_SUCCESS;
}

APPLE_GETVAR_PROTOCOL mDeviceProperties=
{
	DEVICE_PROPERTIES_SIGNATURE,
	NULL,
	NULL,
	NULL,
	GetDeviceProps,   
  NULL,
};

typedef	EFI_STATUS (EFIAPI *EFI_SCREEN_INFO_FUNCTION)(
                                                      VOID* This, 
                                                      UINT64* baseAddress,
                                                      UINT64* frameBufferSize,
                                                      UINT32* byterPerRow,
                                                      UINT32* Width,
                                                      UINT32* Height,
                                                      UINT32* colorDepth
                                                      );

typedef struct {	
	EFI_SCREEN_INFO_FUNCTION GetScreenInfo;	
} EFI_INTERFACE_SCREEN_INFO;

EFI_STATUS EFIAPI GetScreenInfo(VOID* This, UINT64* baseAddress, UINT64* frameBufferSize,
                         UINT32* bpr, UINT32* w, UINT32* h, UINT32* colorDepth)
{
  /*
   * This implementation has no sense
   * as this protocol will be called only if GOP is absent. Somehow as a replacement
   * There will be a sense to find bootargs->frame_address
   * and fill other values
   * but we always have GOP ;)
   */
	EFI_GRAPHICS_OUTPUT_PROTOCOL	*mGraphicsOutput=NULL;
	EFI_STATUS						Status;
	
	Status = gBS->HandleProtocol (gST->ConsoleOutHandle,
                                &gEfiGraphicsOutputProtocolGuid,
                                (VOID **) &mGraphicsOutput);
	if(EFI_ERROR(Status))
		return EFI_UNSUPPORTED;
  //this print never occured so this procedure is redundant
//	Print(L"GetScreenInfo called with args: %lx %lx %lx %lx %lx %lx\n",
//        baseAddress, frameBufferSize, bpr, w, h, colorDepth);
	*frameBufferSize = (UINT64)mGraphicsOutput->Mode->FrameBufferSize;
	*baseAddress = (UINT64)mGraphicsOutput->Mode->FrameBufferBase;
	*w = (UINT32)mGraphicsOutput->Mode->Info->HorizontalResolution;
	*h = (UINT32)mGraphicsOutput->Mode->Info->VerticalResolution;
	*colorDepth = 32;
	*bpr = (UINT32)(mGraphicsOutput->Mode->Info->PixelsPerScanLine*32) >> 3;
//	Print(L"  Screen info: FBsize=%lx FBaddr=%lx w=%d h=%d\n",
//      *frameBufferSize, *baseAddress, *w, *h);
//  PauseForKey(L"--- press any key ---\n");
	return EFI_SUCCESS;
}

EFI_INTERFACE_SCREEN_INFO mScreenInfo=
{
	GetScreenInfo
};

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
  // for future developers
  // this variable can be used at OnExitBoootServices,
  // as it will be set by boot.efi
  BootOSName = AllocateCopyPool(AsciiStrLen(OSName) + 1, (VOID*)OSName);
}

// OSInfoOSVendorImpl
VOID
EFIAPI
OSInfoOSVendorImpl (
                    IN CHAR8 *OSVendor
                    )
{
  // never used as never called
  INTN Result;
  if (!OSVendor) {
    return;
  }
  Result = AsciiStrCmp (OSVendor, OS_INFO_VENDOR_NAME);
  
  if (Result == 0) {
    //   EfiLibNamedEventSignal (&gAppleOsLoadedNamedEventGuid);
  }
}

EFI_OS_INFO_PROTOCOL mEfiOSInfo = {
  EFI_OS_INFO_PROTOCOL_REVISION,
  OSInfoOSNameImpl,
  OSInfoOSVendorImpl,
  NULL
};

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

EFI_STATUS
EFIAPI
UsbKbGetKeyboardDeviceInfo (
                            OUT UINT16  *IdVendor,
                            OUT UINT16  *IdProduct,
                            OUT UINT8   *CountryCode
                            )
{
  if (IdVendor) {
    *IdVendor    = KeyboardVendor;
  }
  if (IdProduct) {
    *IdProduct   = KeyboardProduct;
  }
  if (CountryCode) {
    *CountryCode = 0;
  }

  return EFI_SUCCESS;
}

EFI_KEYBOARD_INFO_PROTOCOL mKeyboardInfo = {
  UsbKbGetKeyboardDeviceInfo
};

EFI_STATUS
SetPrivateVarProto(VOID)
{
  EFI_STATUS  Status;
  //This must be independent install
  // optional protocols
  /*Status = */gBS->InstallMultipleProtocolInterfaces (&gImageHandle,
                                                       &gAppleScreenInfoProtocolGuid,
                                                       &mScreenInfo, 
                                                       &gEfiOSInfoProtocolGuid,
                                                       &mEfiOSInfo,
                                                       &gAppleGraphConfigProtocolGuid,
                                                       &mGraphConfig,
                                                       &gEfiKeyboardInfoProtocolGuid,
                                                       &mKeyboardInfo,
                                                       NULL
                                                       );
	//obligatory protocol
  Status = gBS->InstallProtocolInterface (&gImageHandle,
                                          &gDevicePropertiesGuid,
                                          EFI_NATIVE_INTERFACE,
                                          &mDeviceProperties
                                          );
	
  return Status;
}
