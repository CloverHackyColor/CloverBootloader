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
#include <Protocol/OcQuirksProtocol.h>
#include "Injectors.h"

#ifndef DEBUG_ALL
#define DEBUG_PRO 1
#else
#define DEBUG_PRO DEBUG_ALL
#endif

#if DEBUG_PRO == 0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_PRO, __VA_ARGS__)
#endif

EFI_GUID gDevicePropertiesGuid = {
  0x91BD12FE, 0xF6C3, 0x44FB, {0xA5, 0xB7, 0x51, 0x22, 0xAB, 0x30, 0x3A, 0xE0}
};
/*
EFI_GUID gAppleFramebufferInfoProtocolGuid = {
	0xe316e100, 0x0751, 0x4c49, {0x90, 0x56, 0x48, 0x6c, 0x7e, 0x47, 0x29, 0x03}
}; */
// gEfiKeyboardInfoProtocolGuid
// {0xE82A0A1E, 0x0E4D, 0x45AC, {0xA6, 0xDC, 0x2A, 0xE0, 0x58, 0x00, 0xD3, 0x11}}

// C5C5DA95-7D5C-45E6-B2F1-3FD52BB10077 - EfiOSInfo
// 03622D6D-362A-4E47-9710-C238B23755C1 - GraphConfig

extern EFI_GUID gAppleFramebufferInfoProtocolGuid;
extern BOOLEAN  gProvideConsoleGopEnable;

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

// GET_PROPERTY_VALUE
/** Locates a device property in the database and returns its value into Value.

 @param[in]      This        A pointer to the protocol instance.
 @param[in]      DevicePath  The device path of the device to get the property of.
 @param[in]      Name        The Name of the requested property.
 @param[out]     Value       The Buffer allocated by the caller to return the
                              value of the property into.
 @param[in, out] Size        On input the size of the allocated Value Buffer.
                              On output the size required to fill the Buffer.

 @return                       The status of the operation is returned.
 @retval EFI_BUFFER_TOO_SMALL  The memory required to return the value exceeds
                               the size of the allocated Buffer.
                               The required size to complete the operation has
                                been returned into Size.
 @retval EFI_NOT_FOUND         The given device path does not have a property
                                with the specified Name.
 @retval EFI_SUCCESS           The operation completed successfully.
 **/

typedef
EFI_STATUS
(EFIAPI *APPLE_GETVAR_PROTOCOL_GET_PROPERTY_VALUE) (
    IN     APPLE_GETVAR_PROTOCOL        *This,
    IN     EFI_DEVICE_PATH_PROTOCOL     *DevicePath,
    IN     CHAR16                       *Name,
    OUT    VOID                         *Value, OPTIONAL
    IN OUT UINTN                        *Size
);

// SET_PROPERTY
/** Sets the specified property of the given device path to the provided Value.

 @param[in]  This        A pointer to the protocol instance.
 @param[in]  DevicePath  The device path of the device to set the property of.
 @param[in]  Name        The Name of the desired property.
 @param[in]  Value       The Buffer holding the value to set the property to.
 @param[out] Size        The size of the Value Buffer.

 @return                       The status of the operation is returned.
 @retval EFI_OUT_OF_RESOURCES  The memory necessary to complete the operation
 could not be allocated.
 @retval EFI_SUCCESS           The operation completed successfully.
 **/

typedef
EFI_STATUS
(EFIAPI *APPLE_GETVAR_PROTOCOL_SET_PROPERTY) (
    IN APPLE_GETVAR_PROTOCOL        *This,
    IN EFI_DEVICE_PATH_PROTOCOL     *DevicePath,
    IN CHAR16                       *Name,
    IN VOID                         *Value,
    IN UINTN                        Size
);

// REMOVE_PROPERTY
/** Removes the specified property from the given device path.

 @param[in] This        A pointer to the protocol instance.
 @param[in] DevicePath  The device path of the device to set the property of.
 @param[in] Name        The Name of the desired property.

 @return                The status of the operation is returned.
 @retval EFI_NOT_FOUND  The given device path does not have a property with
                        the specified Name.
 @retval EFI_SUCCESS    The operation completed successfully.
 **/
typedef
EFI_STATUS
(EFIAPI *APPLE_GETVAR_PROTOCOL_REMOVE_PROPERTY) (
    IN APPLE_GETVAR_PROTOCOL        *This,
    IN EFI_DEVICE_PATH_PROTOCOL     *DevicePath,
    IN CHAR16                       *Name
);


typedef
EFI_STATUS
(EFIAPI *APPLE_GETVAR_PROTOCOL_GET_DEVICE_PROPS) (
    IN     APPLE_GETVAR_PROTOCOL    *This,
    IN     CHAR8                    *Buffer,
    IN OUT UINT32                   *BufferSize);

struct _APPLE_GETVAR_PROTOCOL {
  UINT64    Sign;
  APPLE_GETVAR_PROTOCOL_GET_PROPERTY_VALUE  GetPropertyValue; //8
  APPLE_GETVAR_PROTOCOL_SET_PROPERTY        SetProperty;      //0x10
  APPLE_GETVAR_PROTOCOL_REMOVE_PROPERTY     RemoveProperty;   //0x18
  APPLE_GETVAR_PROTOCOL_GET_DEVICE_PROPS    GetPropertyBuffer;//0x20
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
	GetDeviceProps
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
  DBG("GetScreenInfo called\n");
//	printf("GetScreenInfo called with args: %lx %lx %lx %lx %lx %lx\n",
//        baseAddress, frameBufferSize, bpr, w, h, colorDepth);
	*frameBufferSize = (UINT64)mGraphicsOutput->Mode->FrameBufferSize;
	*baseAddress = (UINT64)mGraphicsOutput->Mode->FrameBufferBase;
	*w = (UINT32)mGraphicsOutput->Mode->Info->HorizontalResolution;
	*h = (UINT32)mGraphicsOutput->Mode->Info->VerticalResolution;
	*colorDepth = 32;
	*bpr = (UINT32)(mGraphicsOutput->Mode->Info->PixelsPerScanLine*32) >> 3;
//	printf("  Screen info: FBsize=%lx FBaddr=%lx w=%d h=%d\n",
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

extern EFI_GUID gAppleOSLoadedNamedEventGuid;
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
  BootOSName = (__typeof__(BootOSName))AllocateCopyPool(AsciiStrLen(OSName) + 1, (VOID*)OSName);
  DBG("OSInfo:OSName called\n");
  EfiNamedEventSignal (&gAppleOSLoadedNamedEventGuid);
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
  DBG("OSInfo:OSVendor called\n");
  if (!OSVendor) {
    return;
  }
  Result = AsciiStrCmp(OSVendor, OS_INFO_VENDOR_NAME);
  
  if (Result == 0) {
    //   EfiLibNamedEventSignal (&gAppleOSLoadedNamedEventGuid);
  }
}

EFI_OS_INFO_PROTOCOL mEfiOSInfo = {
  EFI_OS_INFO_PROTOCOL_REVISION,
  OSInfoOSNameImpl,
  OSInfoOSVendorImpl,
  NULL
};

//Usage
/*RestoreConfig(This, 2, 400, R, G, B)
->LocateProtocol(gAppleGraphConfigProtocolGuid, 0, 78000000438/1F7DAEC8) = Success
 call sub_30150
  ->GetVariable(boot-gamma, gEfiAppleBootGuid, 0/50, 0, 0) = Not Found
 mRestored = TRUE;
->GraphConfig.RestoreConfig(2, 400, 1FFCE8B0, 1FFCE0B0, 1FFCD8B0) status=Success
*/

EFI_STATUS
EFIAPI
RestoreConfig (APPLE_GRAPH_CONFIG_PROTOCOL* This,
               UINT32 Param1, UINT32 Param2, VOID* Param3, VOID* Param4, VOID* Param5
               )
{
  DBG("RestoreConfig called Param1=%x\n", Param1);
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
  DBG("KeyboardDeviceInfo called\n");
  return EFI_SUCCESS;
}

EFI_KEYBOARD_INFO_PROTOCOL mKeyboardInfo = {
  UsbKbGetKeyboardDeviceInfo
};

#define OCQUIRKS_PROTOCOL_REVISION  23

EFI_STATUS
EFIAPI
GetQuirksConfig (IN  OCQUIRKS_PROTOCOL  *This,
                 OUT OC_ABC_SETTINGS    *Buffer,
                 OUT BOOLEAN            *GopEnable
                 )
{
  DBG("GetQuirksConfig called\n");
  CopyMem(Buffer, &gQuirks, sizeof(OC_ABC_SETTINGS));
  *GopEnable = gProvideConsoleGopEnable;
  return EFI_SUCCESS;
}

OCQUIRKS_PROTOCOL mQuirksConfig = {
  OCQUIRKS_PROTOCOL_REVISION,
  0,  //reserved
  GetQuirksConfig
};

EFI_STATUS
SetPrivateVarProto(VOID)
{
  EFI_STATUS  Status;
  //This must be independent install
  // optional protocols
  /*Status = */gBS->InstallMultipleProtocolInterfaces (&gImageHandle,
                                                       &gAppleFramebufferInfoProtocolGuid,
                                                       &mScreenInfo, 
                                                       &gEfiOSInfoProtocolGuid,
                                                       &mEfiOSInfo,
                                                       &gAppleGraphConfigProtocolGuid,
                                                       &mGraphConfig,
                                                       &gEfiKeyboardInfoProtocolGuid,
                                                       &mKeyboardInfo,
                                                       &gOcQuirksProtocolGuid,
                                                       &mQuirksConfig,
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
