//
//  AppleProtocols.c
//  Clover
//
//  Created by Slice on 14.10.16.
//

#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>

//#include <EfiImageFormat.h>
#include <Protocol/UgaDraw.h>
#include <Protocol/AppleSMC.h>
#include <Protocol/AppleImageCodecProtocol.h>
#include <Protocol/AppleKeyState.h>
#include <Protocol/OSInfo.h>
#include <Protocol/AppleGraphConfig.h>
//#include <Protocol/AppleEvent.h>
#include <Protocol/FirmwareVolume.h>
#include <Protocol/KeyboardInfo.h> 
#include <Protocol/AppleKeyMapDatabase.h> 

#include "Common.h"

//#define APPLE_SMC_PROTOCOL_GUID         {0x17407e5a, 0xaf6c, 0x4ee8, {0x98, 0xa8, 0x00, 0x21, 0x04, 0x53, 0xcd, 0xd9}}
//#define APPLE_IMAGE_CODEC_PROTOCOL_GUID	{0x0dfce9f6, 0xc4e3, 0x45ee, {0xa0, 0x6a, 0xa8, 0x61, 0x3b, 0x98, 0xa5, 0x07}}

/****************************************************/
/**/
/** Installs our AppleSMC overrides. */

/** Original DataHub protocol. */
APPLE_SMC_IO_PROTOCOL gOrgAppleSMC;
APPLE_SMC_IO_PROTOCOL *gAppleSMC;

EFI_STATUS EFIAPI
OvrReadData (IN  APPLE_SMC_IO_PROTOCOL  *This,
             IN  SMC_KEY                Key,
             IN  SMC_DATA_SIZE          Size,
             OUT SMC_DATA               *Value
             )
{
	EFI_STATUS				Status;
  CHAR8 Str[512];
  CHAR8 StrSmall[10];
  CHAR8 *Ptr;
  INTN i;
  
  Status = gOrgAppleSMC.SmcReadValue(This, Key, Size, Value);
  PRINT("->AppleSMC.SmcReadValue SMC=%x (%c%c%c%c) len=%d\n", Key, (Key >> 24) & 0xFF,
        (Key >> 16) & 0xFF, (Key >> 8) & 0xFF,  Key & 0xFF, Size);
  AsciiSPrint(Str, 512, "--> data=:");
  Ptr = &Str[10];
  *Ptr++ = ' ';
  for (i=0; i<MIN(Size, 500); i++) {
    AsciiSPrint(StrSmall, 10, "%02x ", Value[i]);
    *Ptr++ = StrSmall[0];
    *Ptr++ = StrSmall[1];
    *Ptr++ = ' ';
  }
  *Ptr++ = '\0';
  PRINT("%a\n", Str);
  return Status;
}

EFI_STATUS EFIAPI
OvrWriteValue (IN  APPLE_SMC_IO_PROTOCOL  *This,
               IN  SMC_KEY                Key,
               IN  SMC_DATA_SIZE          Size,
               OUT SMC_DATA               *Value
               )
{
	EFI_STATUS				Status;
  CHAR8 Str[512];
  CHAR8 StrSmall[10];
  CHAR8 *Ptr;
  INTN i;
  
  Status = gOrgAppleSMC.SmcWriteValue(This, Key, Size, Value);
  PRINT("->AppleSMC.SmcWriteValue SMC=%x (%c%c%c%c) len=%d\n", Key, (Key >> 24) & 0xFF,
        (Key >> 16) & 0xFF, (Key >> 8) & 0xFF,  Key & 0xFF, Size);
  AsciiSPrint(Str, 512, "--> data=:");
  Ptr = &Str[10];
  *Ptr++ = ' ';
  for (i=0; i<MIN(Size, 500); i++) {
    AsciiSPrint(StrSmall, 10, "%02x ", Value[i]);
    *Ptr++ = StrSmall[0];
    *Ptr++ = StrSmall[1];
    *Ptr++ = ' ';
  }
  *Ptr++ = '\0';
  PRINT("%a\n", Str);
  return Status;
}
      
EFI_STATUS
EFIAPI
OvrGetKeyCount (IN  APPLE_SMC_IO_PROTOCOL  *This,
                    OUT SMC_DATA           *Count
                    )
{
  EFI_STATUS				Status;
  Status = gOrgAppleSMC.SmcGetKeyCount(This, Count);
  PRINT("->AppleSMC.SmcGetKeyCount(%p), =>%d\n", Count, Count?(Count[3] + Count[2]*16):0);
  return Status;
}
               
EFI_STATUS
EFIAPI
OvrAddKey (IN  APPLE_SMC_IO_PROTOCOL  *This,
           IN   SMC_KEY                Key,
           IN   SMC_DATA_SIZE          Size,
           IN   SMC_KEY_TYPE           Type,
           IN   SMC_KEY_ATTRIBUTES     Attributes
           )
{
  EFI_STATUS				Status;
  Status = gOrgAppleSMC.SmcAddKey(This, Key, Size, Type, Attributes);
  PRINT("->AppleSMC.SmcAddKey SMC=%x (%c%c%c%c) len=%d type=%a, attr=%x\n", Key, (Key >> 24) & 0xFF,
        (Key >> 16) & 0xFF, (Key >> 8) & 0xFF,  Key & 0xFF, Size, Type, Attributes);
  return Status;
}

EFI_STATUS
EFIAPI
OvrKeyFromIndex (IN  APPLE_SMC_IO_PROTOCOL  *This,
                 IN  SMC_INDEX              Index,
                 OUT SMC_KEY                *Key
                )
{
  EFI_STATUS				Status;
  Status = gOrgAppleSMC.SmcGetKeyFromIndex(This, Index, Key);
  PRINT("->AppleSMC.SmcGetKeyFromIndex(%d), =>0x%x\n", Index, Key?*Key:0);
  return Status;
}

EFI_STATUS
EFIAPI
OvrGetKeyInfo (IN  APPLE_SMC_IO_PROTOCOL  *This,
           IN   SMC_KEY                Key,
           OUT   SMC_DATA_SIZE          *Size,
           OUT   SMC_KEY_TYPE           *Type,
           OUT   SMC_KEY_ATTRIBUTES     *Attributes
           )
{
  EFI_STATUS				Status;
  Status = gOrgAppleSMC.SmcGetKeyInfo(This, Key, Size, Type, Attributes);
  PRINT("->AppleSMC.SmcGetKeyInfo SMC=%x (%c%c%c%c) len=%d type=%a, attr=%x\n", Key, (Key >> 24) & 0xFF,
        (Key >> 16) & 0xFF, (Key >> 8) & 0xFF,  Key & 0xFF, Size?*Size:0, Type?*Type:0, Attributes?*Attributes:0);
  return Status;
}

EFI_STATUS
EFIAPI
OvrReset (IN  APPLE_SMC_IO_PROTOCOL  *This,
          IN UINT32                 Mode
          )
{
  EFI_STATUS				Status;
  Status = gOrgAppleSMC.SmcReset(This, Mode);
  PRINT("->AppleSMC.SmcReset(%d)\n", Mode);
  return Status;
}
/*
EFI_STATUS
EFIAPI
OvrUnknown1 (IN  APPLE_SMC_IO_PROTOCOL  *This
             )
{
  EFI_STATUS				Status;
  Status = gOrgAppleSMC.SmcUnknown1(This);
  PRINT("->AppleSMC.SmcUnknown1()\n");
  return Status;
}

EFI_STATUS
EFIAPI
OvrUnknown2 (IN  APPLE_SMC_IO_PROTOCOL  *This,
             IN UINTN                 Unkn1,
             IN UINTN                 Unkn2
             )
{
  EFI_STATUS				Status;
  Status = gOrgAppleSMC.SmcUnknown2(This, Unkn1, Unkn2);
  PRINT("->AppleSMC.SmcUnknown2(0x%x, 0x%x)\n", Unkn1, Unkn2);
  return Status;
}

EFI_STATUS
EFIAPI
OvrUnknown3 (IN  APPLE_SMC_IO_PROTOCOL  *This,
             IN UINTN                 Unkn1,
             IN UINTN                 Unkn2
             )
{
  EFI_STATUS				Status;
  Status = gOrgAppleSMC.SmcUnknown3(This, Unkn1, Unkn2);
  PRINT("->AppleSMC.SmcUnknown3(0x%x, 0x%x)\n", Unkn1, Unkn2);
  return Status;
}

EFI_STATUS
EFIAPI
OvrUnknown4 (IN  APPLE_SMC_IO_PROTOCOL  *This,
             IN UINTN                 Mode
             )
{
  EFI_STATUS				Status;
  Status = gOrgAppleSMC.SmcUnknown4(This, Mode);
  PRINT("->AppleSMC.SmcUnknown4(0x%x)\n", Mode);
  return Status;
}

EFI_STATUS
EFIAPI
OvrUnknown5 (IN  APPLE_SMC_IO_PROTOCOL  *This,
             IN UINTN                 Mode
             )
{
  EFI_STATUS				Status;
  Status = gOrgAppleSMC.SmcUnknown5(This, Mode);
  PRINT("->AppleSMC.SmcUnknown5(0x%x)\n", Mode);
  return Status;
}
*/

EFI_STATUS EFIAPI
OvrAppleSMC(VOID)
{
	EFI_STATUS				Status;
	
//	PRINT("Overriding AppleSMC ...\n");
	
	// Locate AppleSMC protocol
	Status = gBS->LocateProtocol(&gAppleSMCProtocolGuid, NULL, (VOID **) &gAppleSMC);
	if (EFI_ERROR(Status)) {
		PRINT("Error Overriding AppleSMC: %r\n", Status);
		return Status;
	}
	
	// Store originals
	CopyMem(&gOrgAppleSMC, gAppleSMC, sizeof(APPLE_SMC_IO_PROTOCOL));
	
	// Override with our implementation
	gAppleSMC->SmcReadValue = OvrReadData;
	gAppleSMC->SmcWriteValue = OvrWriteValue;
	gAppleSMC->SmcGetKeyCount = OvrGetKeyCount;
	gAppleSMC->SmcAddKey = OvrAddKey;
	gAppleSMC->SmcGetKeyFromIndex = OvrKeyFromIndex;
	gAppleSMC->SmcGetKeyInfo = OvrGetKeyInfo;
	gAppleSMC->SmcReset = OvrReset;
/*	gAppleSMC->SmcUnknown1 = OvrUnknown1;
	gAppleSMC->SmcUnknown2 = OvrUnknown2;
	gAppleSMC->SmcUnknown3 = OvrUnknown3;
	gAppleSMC->SmcUnknown4 = OvrUnknown4;
	gAppleSMC->SmcUnknown5 = OvrUnknown5; */
	
	PRINT("AppleSMC overriden!\n");
	return EFI_SUCCESS;
}

/****************************************************/
/**/
/** Installs our AppleImageCodecProtocol overrides. */

APPLE_IMAGE_CODEC_PROTOCOL gOrgAppleImageCodec;
APPLE_IMAGE_CODEC_PROTOCOL *gAppleImageCodec;

EFI_STATUS
EFIAPI
OvrRecognizeImageData (//IN APPLE_IMAGE_CODEC_PROTOCOL* This,
                    VOID         *ImageBuffer,
                    UINTN         ImageSize,
                    OUT VOID    **OutBuffer
                    )
{
  EFI_STATUS				Status;
  
  Status = gOrgAppleImageCodec.RecognizeImageData(ImageBuffer, ImageSize, OutBuffer);
  if (EFI_ERROR(Status)) {
    PRINT("->AppleImageCodec.RecognizeImageData(%p, 0x%x, %p), sign=%4x, status=%r\n", ImageBuffer, ImageSize, OutBuffer,
          ImageBuffer?(*(UINT32*)ImageBuffer):0, Status);
  }
  return Status;
}

EFI_STATUS
EFIAPI
OvrGetImageDims (//IN APPLE_IMAGE_CODEC_PROTOCOL* This,
              VOID          *ImageBuffer,
              UINTN         ImageSize,
              UINT32         *ImageWidth,
              UINT32         *ImageHeight
              )
{
  EFI_STATUS				Status;
  
  Status = gOrgAppleImageCodec.GetImageDims(ImageBuffer, ImageSize, ImageWidth, ImageHeight);
  if (EFI_ERROR(Status)) {
    PRINT("->AppleImageCodec.GetImageDims(%p, 0x%x, %p, %p), status=%r\n",
          ImageBuffer, ImageSize, ImageWidth, ImageHeight, Status);
//    PRINT("--> ImageWidth=%d, ImageHeight=%d\n", ImageWidth?*ImageWidth:0, ImageHeight?*ImageHeight:0);
  }
  return Status;
}

EFI_STATUS
EFIAPI
OvrDecodeImageData (//IN APPLE_IMAGE_CODEC_PROTOCOL* This,
                 VOID          *ImageBuffer,
                 UINTN         ImageSize,
                 EFI_UGA_PIXEL **RawImageData,
                 UINT32         *RawImageDataSize
                 )
{
  EFI_STATUS				Status;
  
  Status = gOrgAppleImageCodec.DecodeImageData(ImageBuffer, ImageSize, RawImageData, RawImageDataSize);
  if (EFI_ERROR(Status)) {
    PRINT("->AppleImageCodec.DecodeImageData(%p, 0x%x, %p, %p), status=%r\n",
          ImageBuffer, ImageSize, RawImageData, RawImageDataSize, Status);
 // if (!EFI_ERROR(Status)) {
 //   PRINT("--> RawImageDataSize=%d\n", RawImageDataSize?*RawImageDataSize:0);
  }
  return Status;  
}

EFI_STATUS
EFIAPI
OvrAICUnknown1 (VOID* ImageBuffer, UINTN Param1, UINTN Param2, UINTN Param3)

{
  EFI_STATUS				Status;
  
  Status = gOrgAppleImageCodec.Unknown1(ImageBuffer, Param1, Param2, Param3);
  PRINT("->AppleImageCodec.Unknown1(%p, 0x%x, 0x%x, 0x%x), status=%r\n", ImageBuffer, Param1, Param2, Param3);
  return Status;
}

EFI_STATUS
EFIAPI
OvrAICUnknown2 (VOID* ImageBuffer, UINTN Param1, UINTN Param2, UINTN Param3)

{
  EFI_STATUS				Status;
  
  Status = gOrgAppleImageCodec.Unknown2(ImageBuffer, Param1, Param2, Param3);
  PRINT("->AppleImageCodec.Unknown2(%p, 0x%x, 0x%x, 0x%x), status=%r\n", ImageBuffer, Param1, Param2, Param3);
  return Status;
}


EFI_STATUS EFIAPI
OvrAppleImageCodec(VOID)
{
	EFI_STATUS				Status;
	
//	PRINT("Overriding AppleImageCodec ...\n");
	
	// Locate AppleSMC protocol
	Status = gBS->LocateProtocol(&gAppleImageCodecProtocolGuid, NULL, (VOID **) &gAppleImageCodec);
	if (EFI_ERROR(Status)) {
		PRINT("Error Overriding AppleImageCodec: %r\n", Status);
		return Status;
	}
	
	// Store originals
	CopyMem(&gOrgAppleImageCodec, gAppleImageCodec, sizeof(APPLE_IMAGE_CODEC_PROTOCOL));
	
	// Override with our implementation
	gAppleImageCodec->RecognizeImageData = OvrRecognizeImageData;
	gAppleImageCodec->GetImageDims = OvrGetImageDims;
	gAppleImageCodec->DecodeImageData = OvrDecodeImageData;
  gAppleImageCodec->Unknown1 = OvrAICUnknown1;
  gAppleImageCodec->Unknown2 = OvrAICUnknown2;
	
	PRINT("AppleImageCodec overriden!\n");
	return EFI_SUCCESS;
}

/****************************************************/
/**/
/** Installs our AppleKeyState overrides. */

APPLE_KEY_STATE_PROTOCOL gOrgAppleKeyState;
APPLE_KEY_STATE_PROTOCOL *gAppleKeyState;

EFI_STATUS
EFIAPI
OvrReadKeyState (IN APPLE_KEY_STATE_PROTOCOL *This,
                 OUT UINT16 *ModifyFlags,
                 OUT UINTN  *PressedKeyStatesCount,
                 OUT APPLE_KEY_CODE *PressedKeyStates)
{
  EFI_STATUS				Status;

  Status = gOrgAppleKeyState.ReadKeyState(This, ModifyFlags, PressedKeyStatesCount, PressedKeyStates);
  if (PressedKeyStatesCount && *PressedKeyStatesCount && PressedKeyStates) {
    PRINT("->ReadKeyState(), count=%d, flags=0x%x states={%x,%x}, status=%r\n",
          *PressedKeyStatesCount,
          ModifyFlags?*ModifyFlags:0,
          PressedKeyStates[0],
          PressedKeyStates[1],
          Status);
  }
  return Status;
}

EFI_STATUS
EFIAPI
OvrSearchKeyStroke (APPLE_KEY_STATE_PROTOCOL* This,
                 IN UINT16 ModifyFlags,
                 IN UINTN PressedKeyStatesCount,
                 IN OUT APPLE_KEY_CODE *PressedKeyStates,
                 IN BOOLEAN ExactMatch)
{
  EFI_STATUS				Status;
  
  Status = gOrgAppleKeyState.SearchKeyStroke(This, ModifyFlags, PressedKeyStatesCount,
                                             PressedKeyStates, ExactMatch);
  if (PressedKeyStates) {
    PRINT("->SearchKeyStroke(), count=%d, flags=0x%x, %a match, states={%x,%x}, status=%r\n",
          PressedKeyStatesCount,
          ModifyFlags, ExactMatch?"exact":"~",
          PressedKeyStates[0], PressedKeyStates[1], Status);
  }

  return Status;
}




EFI_STATUS EFIAPI
OvrAppleKeyState(VOID)
{
  EFI_STATUS				Status;

//  PRINT("Overriding AppleKeyState ...\n");

  // Locate AppleKeyState protocol
  Status = gBS->LocateProtocol(&gAppleKeyStateProtocolGuid, NULL, (VOID **) &gAppleKeyState);
  if (EFI_ERROR(Status)) {
    PRINT("Error Overriding AppleKeyState: %r\n", Status);
    return Status;
  }

  // Store originals
  CopyMem(&gOrgAppleKeyState, gAppleKeyState, sizeof(APPLE_KEY_STATE_PROTOCOL));

  // Override with our implementation
  gAppleKeyState->ReadKeyState = OvrReadKeyState;
  gAppleKeyState->SearchKeyStroke = OvrSearchKeyStroke;

  PRINT("AppleKeyState overriden!\n");
  return EFI_SUCCESS;

}

//**************************************************
EFI_OS_INFO_PROTOCOL gOrgOSInfo;
EFI_OS_INFO_PROTOCOL *gOSInfo;

// OSInfoOSNameImpl
VOID
EFIAPI
OvrOSName (
           IN CHAR8 *Name)
{
//  EFI_STATUS				Status;
  gOrgOSInfo.OSName(Name);
  PRINT("->OSInfo.OSName=%a\n", Name);
}

// OSInfoOSVendorImpl
VOID
EFIAPI
OvrOSVendor (
             IN CHAR8 *Name)
{
//  EFI_STATUS				Status;
  gOrgOSInfo.OSVendor(Name);
  PRINT("->OSInfo.OSVendor=%a\n", Name);
}

VOID
EFIAPI
OvrOSGetVtd (
           OUT UINTN  *BootVTdEnabled)
{
  //  EFI_STATUS				Status;
  gOrgOSInfo.GetBootVTdEnabled(BootVTdEnabled);
  PRINT("->OSInfo.GetVtd=0x%x\n", *BootVTdEnabled);
}

VOID
EFIAPI
OvrOSSetVtd (
            IN UINTN  *BootVTdEnabled)
{
  //  EFI_STATUS        Status;
  gOrgOSInfo.SetBootVTdEnabled(BootVTdEnabled);
  PRINT("->OSInfo.SetVtd=0x%x\n", *BootVTdEnabled);
}


EFI_STATUS EFIAPI
OvrOSInfo(VOID)
{
  EFI_STATUS				Status;
  
//  PRINT("Overriding OSInfo ...\n");
  
  // Locate EfiOSInfo protocol
  Status = gBS->LocateProtocol(&gEfiOSInfoProtocolGuid, NULL, (VOID **) &gOSInfo);
  if (EFI_ERROR(Status)) {
    PRINT("Error Overriding OSInfo: %r\n", Status);
    return Status;
  }
  
  // Store originals
  CopyMem(&gOrgOSInfo, gOSInfo, sizeof(EFI_OS_INFO_PROTOCOL));
  
  // Override with our implementation
  gOSInfo->OSVendor = OvrOSVendor;
  gOSInfo->OSName = OvrOSName;
//  gOSInfo->OSEmpty = OvrOSEmpty;
  gOSInfo->SetBootVTdEnabled = OvrOSSetVtd;
  gOSInfo->GetBootVTdEnabled = OvrOSGetVtd;
  
  PRINT("EfiOSInfo overriden!\n");
  return EFI_SUCCESS;
  
}

//**************************************************
APPLE_GRAPH_CONFIG_PROTOCOL gOrgGraphConfig;
APPLE_GRAPH_CONFIG_PROTOCOL *gGraphConfig;

EFI_STATUS
EFIAPI
OvrRestoreConfig (APPLE_GRAPH_CONFIG_PROTOCOL* This,
               UINT32 Param1, UINT32 Param2, VOID* Param3, VOID* Param4, VOID* Param5
               )
{
  EFI_STATUS				Status;
  Status = gOrgGraphConfig.RestoreConfig(This, Param1, Param2, Param3, Param4, Param5);
  PRINT("->GraphConfig.RestoreConfig(%x, %x, %p, %p, %p) status=%r\n",
        Param1, Param2, Param3, Param4, Param5, Status);
  return EFI_SUCCESS;
}



EFI_STATUS EFIAPI
OvrGraphConfig(VOID)
{
  EFI_STATUS				Status;
  
//  PRINT("Overriding GraphConfig ...\n");
  
  // Locate AppleGraphConfig protocol
  Status = gBS->LocateProtocol(&gAppleGraphConfigProtocolGuid, NULL, (VOID **) &gGraphConfig);
  if (EFI_ERROR(Status)) {
    PRINT("Error Overriding GraphConfig: %r\n", Status);
    return Status;
  }
  
  // Store originals
  CopyMem(&gOrgGraphConfig, gGraphConfig, sizeof(APPLE_GRAPH_CONFIG_PROTOCOL));
  
  // Override with our implementation
  gGraphConfig->RestoreConfig = OvrRestoreConfig;
  
  PRINT("AppleGraphConfig overriden!\n");
  return EFI_SUCCESS;
  
}

//**********************************************
EFI_FIRMWARE_VOLUME_PROTOCOL gOrgFV;
EFI_FIRMWARE_VOLUME_PROTOCOL *gFirmwareVolume;

EFI_STATUS
EFIAPI
OvrFvReadFile (
            IN EFI_FIRMWARE_VOLUME_PROTOCOL   *This,
            IN EFI_GUID                       *NameGuid,
            IN OUT VOID                       **Buffer,
            IN OUT UINTN                      *BufferSize,
            OUT EFI_FV_FILETYPE               *FoundType,
            OUT EFI_FV_FILE_ATTRIBUTES        *FileAttributes,
            OUT UINT32                        *AuthenticationStatus
            )
{
  EFI_STATUS				Status;
  Status = gOrgFV.ReadFile(This, NameGuid, Buffer, BufferSize, FoundType, FileAttributes, AuthenticationStatus);
  PRINT("->FirmwareVolume.ReadFile(%g, size=%d) status=%r\n",
        NameGuid, BufferSize?*BufferSize:0, Status);
  return Status;
}

EFI_STATUS
EFIAPI
OvrFvReadSection (
               IN EFI_FIRMWARE_VOLUME_PROTOCOL   *This,
               IN EFI_GUID                       *NameGuid,
               IN EFI_SECTION_TYPE               SectionType,
               IN UINTN                          SectionInstance,
               IN OUT VOID                       **Buffer,
               IN OUT UINTN                      *BufferSize,
               OUT UINT32                        *AuthenticationStatus
               )
{
  EFI_STATUS				Status;
  Status = gOrgFV.ReadSection(This, NameGuid, SectionType, SectionInstance, Buffer, BufferSize, AuthenticationStatus);
  PRINT("->FirmwareVolume.ReadSection(%g, type=%x, #%d, %d) status=%r\n",
        NameGuid, SectionType, SectionInstance, BufferSize?*BufferSize:0, Status);
  return Status;
}


EFI_STATUS EFIAPI
OvrFirmwareVolume(VOID)
{
  EFI_STATUS				Status;
  
  PRINT("Overriding FirmwareVolume ...\n");
  
  // Locate FirmwareVolume protocol
  Status = gBS->LocateProtocol(&gEfiFirmwareVolumeProtocolGuid, NULL, (VOID **) &gFirmwareVolume);
  if (EFI_ERROR(Status)) {
    PRINT("Error Overriding FirmwareVolume: %r\n", Status);
    return Status;
  }
  
  // Store originals
  CopyMem(&gOrgFV, gFirmwareVolume, sizeof(EFI_FIRMWARE_VOLUME_PROTOCOL));
  
  // Override with our implementation
  gFirmwareVolume->ReadSection = OvrFvReadSection;
  gFirmwareVolume->ReadFile = OvrFvReadFile;
  
  PRINT("FirmwareVolume overriden!\n");
  return EFI_SUCCESS;
  
}

/****************************************************/
/**/
/** Installs our AppleKeyboardInfo overrides. */

EFI_KEYBOARD_INFO_PROTOCOL gOrgAppleKeyboardInfo;
EFI_KEYBOARD_INFO_PROTOCOL *gAppleKeyboardInfo;

EFI_STATUS
EFIAPI
GetKeyboardDeviceInfo (
                            OUT UINT16  *IdVendor,
                            OUT UINT16  *IdProduct,
                            OUT UINT8   *CountryCode
                            )
{
/*  *IdVendor    = mIdVendor;
  *IdProduct   = mIdProduct;
  *CountryCode = mCountryCode; */
  EFI_STATUS				Status;
  Status = gOrgAppleKeyboardInfo.GetInfo(IdVendor, IdProduct, CountryCode);
  PRINT("->KeyboardInfo => Vendor=0x%4x, Product=0x%4x, CountryCode=%x\n",
        IdVendor?*IdVendor:0, IdProduct?*IdProduct:0, CountryCode?*CountryCode:0);
  
  return Status;
}

EFI_STATUS EFIAPI
OvrEfiKeyboardInfo(VOID)
{
  EFI_STATUS				Status;
  
  PRINT("Overriding EfiKeyboardInfo ...\n");
  
  // Locate EfiKeyboardInfo protocol
  Status = gBS->LocateProtocol(&gEfiKeyboardInfoProtocolGuid, NULL, (VOID **)&gAppleKeyboardInfo);
  if (EFI_ERROR(Status)) {
    PRINT("Error Overriding EfiKeyboardInfo: %r\n", Status);
    return Status;
  }
  
  // Store originals
  CopyMem(&gOrgAppleKeyboardInfo, gAppleKeyboardInfo, sizeof(EFI_KEYBOARD_INFO_PROTOCOL));
  
  // Override with our implementation
  gAppleKeyboardInfo->GetInfo = GetKeyboardDeviceInfo;
  
  PRINT("EfiKeyboardInfo overriden!\n");
  return EFI_SUCCESS;
  
}

//************************
// APPLE_KEY_MAP_DATABASE_PROTOCOL

APPLE_KEY_MAP_DATABASE_PROTOCOL gOrgAppleKeyMapDb;
APPLE_KEY_MAP_DATABASE_PROTOCOL *gAppleKeyMapDb;

EFI_STATUS
EFIAPI
OvrCreateKeyStrokesBuffer (IN  APPLE_KEY_MAP_DATABASE_PROTOCOL  *This,
                           IN  UINTN                            KeyBufferSize,
                           OUT UINTN                            *Index)
{
  EFI_STATUS               Status;
  Status = gOrgAppleKeyMapDb.CreateKeyStrokesBuffer(This, KeyBufferSize, Index);
  PRINT("->CreateKeyStrokesBuffer => KeyBufferSize=%d, Index=%d, Status=%r\n",
        KeyBufferSize, Index?*Index:0, Status);
  
  return Status;
}

EFI_STATUS
EFIAPI
OvrRemoveKeyStrokesBuffer (
                              IN APPLE_KEY_MAP_DATABASE_PROTOCOL  *This,
                              IN UINTN                            Index
                              )
{
  EFI_STATUS               Status;
  Status = gOrgAppleKeyMapDb.RemoveKeyStrokesBuffer(This, Index);
  PRINT("->RemoveKeyStrokesBuffer => Index=%d, Status=%r\n",
        Index, Status);
  
  return Status;
}
  
 EFI_STATUS
EFIAPI
OvrSetKeyStrokeBufferKeys (
                              IN APPLE_KEY_MAP_DATABASE_PROTOCOL  *This,
                              IN UINTN                            Index,
                              IN APPLE_MODIFIER_MAP               Modifiers,
                              IN UINTN                            NumberOfKeys,
                              IN APPLE_KEY_CODE                        *Keys
                              )
{
  EFI_STATUS               Status;
  Status = gOrgAppleKeyMapDb.SetKeyStrokeBufferKeys(This, Index, Modifiers, NumberOfKeys, Keys);
#if SET_KEY_STROKE
  PRINT("->SetKeyStrokeBufferKeys => Index=%d, Modifiers=%x, NoKeys=%d, Keys={%x, %x}, Status=%r\n",
        Index, Modifiers, NumberOfKeys, Keys?*Keys:0, (Keys && NumberOfKeys>1)?Keys[1]:0, Status);
#endif
  return Status;
}



EFI_STATUS EFIAPI
OvrAppleKeyMapDb(VOID)
{
  EFI_STATUS				Status;
  
  PRINT("Overriding AppleKeyMapDb ...\n");
  
  // Locate AppleKeyMapDb protocol
  Status = gBS->LocateProtocol(&gAppleKeyMapDatabaseProtocolGuid, NULL, (VOID **)&gAppleKeyMapDb);
  if (EFI_ERROR(Status)) {
    PRINT("Error Overriding AppleKeyMapDb: %r\n", Status);
    return Status;
  }
  
  // Store originals
  CopyMem(&gOrgAppleKeyMapDb, gAppleKeyMapDb, sizeof(APPLE_KEY_MAP_DATABASE_PROTOCOL));
  
  // Override with our implementation
  gAppleKeyMapDb->CreateKeyStrokesBuffer = OvrCreateKeyStrokesBuffer;
  gAppleKeyMapDb->RemoveKeyStrokesBuffer = OvrRemoveKeyStrokesBuffer;
  gAppleKeyMapDb->SetKeyStrokeBufferKeys = OvrSetKeyStrokeBufferKeys;
  
  
  
  PRINT("AppleKeyMapDb overriden!\n");
  return EFI_SUCCESS;
  
}

