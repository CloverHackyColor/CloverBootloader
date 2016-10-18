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


#include <Protocol/UgaDraw.h>
#include <Protocol/AppleSMC.h>
#include <Protocol/AppleImageCodecProtocol.h>
#include <Protocol/AppleKeyState.h>
#include <Protocol/OSInfo.h>
#include <Protocol/AppleGraphConfig.h>
//#include <Protocol/AppleEvent.h>
#include <Protocol/FirmwareVolume.h>

#include "Common.h"

//#define APPLE_SMC_PROTOCOL_GUID         {0x17407e5a, 0xaf6c, 0x4ee8, {0x98, 0xa8, 0x00, 0x21, 0x04, 0x53, 0xcd, 0xd9}}
//#define APPLE_IMAGE_CODEC_PROTOCOL_GUID	{0x0dfce9f6, 0xc4e3, 0x45ee, 0xa0, 0x6a, 0xa8, 0x61, 0x3b, 0x98, 0xa5, 0x07}

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
  CHAR8 StrSmall[3];
  INTN i;
  
  Status = gOrgAppleSMC.SmcReadValue(This, Key, Size, Value);
  PRINT("->AppleSMC.SmcReadValue SMC=%x (%c%c%c%c) len=%d\n", Key, (Key >> 24) & 0xFF,
        (Key >> 16) & 0xFF, (Key >> 8) & 0xFF,  Key & 0xFF, Size);
  AsciiSPrint(Str, 512, "--> data=:");
  for (i=0; i<Size; i++) {
    AsciiSPrint(StrSmall, 3, "%02x ", Value[i]);
    Str[11+i*3] = StrSmall[0];
    Str[12+i*3] = StrSmall[1];
    Str[13+i*3] = ' ';
  }
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
  CHAR8 StrSmall[3];
  INTN i;
  
  Status = gOrgAppleSMC.SmcWriteValue(This, Key, Size, Value);
  PRINT("->AppleSMC.SmcWriteValue SMC=%x (%c%c%c%c) len=%d\n", Key, (Key >> 24) & 0xFF,
        (Key >> 16) & 0xFF, (Key >> 8) & 0xFF,  Key & 0xFF, Size);
  AsciiSPrint(Str, 512, "--> data=:");
  for (i=0; i<Size; i++) {
    AsciiSPrint(StrSmall, 3, "%02x ", Value[i]);
    Str[11+i*3] = StrSmall[0];
    Str[12+i*3] = StrSmall[1];
    Str[13+i*3] = ' ';
  }
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


EFI_STATUS EFIAPI
OvrAppleSMC(VOID)
{
	EFI_STATUS				Status;
	
	PRINT("Overriding AppleSMC ...\n");
	
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
	gAppleSMC->SmcUnknown1 = OvrUnknown1;
	gAppleSMC->SmcUnknown2 = OvrUnknown2;
	gAppleSMC->SmcUnknown3 = OvrUnknown3;
	gAppleSMC->SmcUnknown4 = OvrUnknown4;
	gAppleSMC->SmcUnknown5 = OvrUnknown5;
	
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
OvrRecognizeImageData (IN APPLE_IMAGE_CODEC_PROTOCOL* This, 
                    VOID         *ImageBuffer,
                    UINTN         ImageSize
                    )
{
  EFI_STATUS				Status;
  
  Status = gOrgAppleImageCodec.RecognizeImageData(This, ImageBuffer, ImageSize);
  PRINT("->RecognizeImageData(%p, 0x%x), sign=%4x, status=%r\n", ImageBuffer, ImageSize,
        ImageBuffer?(*(UINT32*)ImageBuffer):0, Status);
  return Status;
}

EFI_STATUS
EFIAPI
OvrGetImageDims (IN APPLE_IMAGE_CODEC_PROTOCOL* This,
              VOID          *ImageBuffer,
              UINTN         ImageSize,
              UINT32         *ImageWidth,
              UINT32         *ImageHeight
              )
{
  EFI_STATUS				Status;
  
  Status = gOrgAppleImageCodec.GetImageDims(This, ImageBuffer, ImageSize, ImageWidth, ImageHeight);
  PRINT("->GetImageDims(%p, 0x%x, %p, %p), status=%r\n", ImageBuffer, ImageSize, ImageWidth, ImageHeight, Status);
  if (!EFI_ERROR(Status)) {
    PRINT("--> ImageWidth=%d, ImageHeight=%d\n", ImageWidth?*ImageWidth:0, ImageHeight?*ImageHeight:0);
  }
  return Status;
}

EFI_STATUS
EFIAPI
OvrDecodeImageData (IN APPLE_IMAGE_CODEC_PROTOCOL* This,
                 VOID          *ImageBuffer,
                 UINTN         ImageSize,
                 EFI_UGA_PIXEL **RawImageData,
                 UINTN         *RawImageDataSize
                 )
{
  EFI_STATUS				Status;
  
  Status = gOrgAppleImageCodec.DecodeImageData(This, ImageBuffer, ImageSize, RawImageData, RawImageDataSize);
  PRINT("->DecodeImageData(%p, 0x%x, %p, %p), status=%r\n", ImageBuffer, ImageSize, RawImageData, RawImageDataSize, Status);
  if (!EFI_ERROR(Status)) {
    PRINT("--> RawImageDataSize=%d\n", RawImageDataSize?*RawImageDataSize:0);
  }
  return Status;  
}

EFI_STATUS
EFIAPI
OvrUnknown (IN APPLE_IMAGE_CODEC_PROTOCOL* This,
                       VOID         *ImageBuffer,
                       UINTN         ImageSize
                       )
{
  EFI_STATUS				Status;
  
  Status = gOrgAppleImageCodec.Unknown(This, ImageBuffer, ImageSize);
  PRINT("->UnknownCall(%p, 0x%x), status=%r\n", ImageBuffer, ImageSize, Status);
  return Status;
}


EFI_STATUS EFIAPI
OvrAppleImageCodec(VOID)
{
	EFI_STATUS				Status;
	
	PRINT("Overriding AppleImageCodec ...\n");
	
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
  gAppleImageCodec->Unknown = OvrUnknown;
	
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
                 OUT CHAR16 *PressedKeyStates)
{
  EFI_STATUS				Status;

  Status = gOrgAppleKeyState.ReadKeyState(This, ModifyFlags, PressedKeyStatesCount, PressedKeyStates);
  PRINT("->ReadKeyState(), count=%d, flags=0x%x states=%s, status=%r\n",
        PressedKeyStatesCount?*PressedKeyStatesCount:0,
        ModifyFlags?*ModifyFlags:0,
        PressedKeyStates, Status);
  return Status;
}



EFI_STATUS EFIAPI
OvrAppleKeyState(VOID)
{
  EFI_STATUS				Status;

  PRINT("Overriding AppleKeyState ...\n");

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
OvrOSEmpty (
           IN CHAR8 *Name)
{
  //  EFI_STATUS				Status;
  gOrgOSInfo.OSName(Name);
  PRINT("->OSInfo.OSEmpty=%a\n", Name);
}


EFI_STATUS EFIAPI
OvrOSInfo(VOID)
{
  EFI_STATUS				Status;
  
  PRINT("Overriding OSInfo ...\n");
  
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
  gOSInfo->OSEmpty = OvrOSEmpty;
  
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
  
  PRINT("Overriding GraphConfig ...\n");
  
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

