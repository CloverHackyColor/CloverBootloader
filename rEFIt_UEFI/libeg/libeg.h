/*
 * libeg/libeg.h
 * EFI graphics library header for users
 *
 * Copyright (c) 2006 Christoph Pfisterer
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *  * Neither the name of Christoph Pfisterer nor the names of the
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __LIBEG_LIBEG_H__
#define __LIBEG_LIBEG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <PiDxe.h>
#include <Base.h>
#include <Uefi.h>
#include <FrameworkDxe.h>
// Protocol Includes
#include <Protocol/AbsolutePointer.h>
#include <Protocol/AcpiTable.h>
#include <Protocol/BlockIo.h>
#include <Protocol/BlockIo2.h>
#include <Protocol/Cpu.h>
#include <Protocol/DataHub.h>
#include <Protocol/DebugPort.h>
#include <Protocol/Decompress.h>
#include <Protocol/DevicePath.h>
#include <Protocol/DevicePathFromText.h>
#include <Protocol/DevicePathToText.h>
#include <Protocol/DiskIo.h>
#include <Protocol/EdidActive.h>
#include <Protocol/EdidDiscovered.h>
#include <Protocol/FirmwareVolume2.h>
#include <Protocol/FrameworkHii.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/HiiDatabase.h>
#include <Protocol/HiiImage.h>
#include <Protocol/LegacyBios.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/PciIo.h>
#include <Protocol/ScsiIo.h>
#include <Protocol/ScsiPassThru.h>
#include <Protocol/ScsiPassThruExt.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/SimpleNetwork.h>
#include <Protocol/SimplePointer.h>
#include <Protocol/SimpleTextIn.h>
#include <Protocol/SimpleTextOut.h>
#include <Protocol/Smbios.h>
#include <Protocol/SmbusHc.h>
#include <Protocol/UgaDraw.h>
#include <Protocol/UgaIo.h>
#include <Protocol/UnicodeCollation.h>
#include <Protocol/UsbIo.h>

// Guid Includes
#include <Guid/Acpi.h>
#include <Guid/ConsoleInDevice.h>
#include <Guid/ConsoleOutDevice.h>
#include <Guid/DataHubRecords.h>
#include <Guid/DxeServices.h>
#include <Guid/EventGroup.h>
#include <Guid/FileInfo.h>
#include <Guid/FileSystemInfo.h>
#include <Guid/FileSystemVolumeLabelInfo.h>
#include <Guid/GlobalVariable.h>
#include <Guid/HobList.h>
#include <Guid/MemoryTypeInformation.h>
#include <Guid/MemoryAllocationHob.h>
#include <Guid/SmBios.h>
#include <Guid/StandardErrorDevice.h>

// Library Includes
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/DeviceTreeLib.h>
#include <Library/DxeServicesLib.h>
#include <Library/DxeServicesTableLib.h>
//#include <Library/EblCmdLib.h>
//#include <Library/EblNetworkLib.h>
//#include "EfiFileLib.h"
#include <Library/HiiLib.h>
#include <Library/HobLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/PrintLib.h>
#include <Library/TimerLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiScsiLib.h>

// IndustryStandard Includes
#include <IndustryStandard/Pci.h>
#include <IndustryStandard/SmBus.h>
#include <IndustryStandard/Acpi.h>
//#include <IndustryStandard/Acpi20.h>
//#include <IndustryStandard/Acpi30.h>
//#include <IndustryStandard/Acpi40.h>
#include <IndustryStandard/HighPrecisionEventTimerTable.h>
#include <IndustryStandard/Scsi.h>
#include <IndustryStandard/Atapi.h>
#include <IndustryStandard/AppleSmBios.h>

#ifdef __cplusplus
} // extern "C"
#endif

#define USE_XTHEME 0

/* types */

typedef enum {
  imNone,
  imScale,
  imCrop,
  imTile

} SCALING;

typedef enum {
  FONT_ALFA,
  FONT_GRAY,
  FONT_LOAD
} FONT_TYPE;

/* This should be compatible with EFI_UGA_PIXEL */
typedef struct {
    UINT8 b, g, r, a;
} EG_PIXEL;
/*
typedef struct {
  UINT8 Blue;
  UINT8 Green;
  UINT8 Red;
  UINT8 Reserved;
} EFI_GRAPHICS_OUTPUT_BLT_PIXEL;

typedef union {
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL Pixel;
  UINT32                        Raw;
} EFI_GRAPHICS_OUTPUT_BLT_PIXEL_UNION;
*/


typedef struct {
    INTN      Width;
    INTN      Height;
    EG_PIXEL    *PixelData;
    BOOLEAN     HasAlpha;   //moved here to avoid alignment issue
} EG_IMAGE;

#ifdef __cplusplus
class EG_RECT {
public:
  INTN     XPos;
  INTN     YPos;
  INTN     Width;
  INTN     Height;
	
  EG_RECT() : XPos(0), YPos(0), Width(0), Height(0) {};
  EG_RECT(const EG_RECT& other) { XPos = other.XPos; YPos = other.YPos; Width = other.Width; Height = other.Height; }
  const EG_RECT& operator = (const EG_RECT& other) { XPos = other.XPos; YPos = other.YPos; Width = other.Width; Height = other.Height; return *this; }
  bool operator == (const EG_RECT& other) { return XPos == other.XPos  &&  YPos == other.YPos  &&  Width == other.Width  &&  Height == other.Height; }
  bool operator != (const EG_RECT& other) { return !(*this == other); }
};
#else
typedef struct EG_RECT {
  INTN     XPos;
  INTN     YPos;
  INTN     Width;
  INTN     Height;
} EG_RECT;
#endif

#define TEXT_YMARGIN (2)
#define TEXT_XMARGIN (8)

#define EG_EIPIXELMODE_GRAY         (0)
#define EG_EIPIXELMODE_GRAY_ALPHA   (1)
#define EG_EIPIXELMODE_COLOR        (2)
#define EG_EIPIXELMODE_COLOR_ALPHA  (3)
#define EG_EIPIXELMODE_ALPHA        (4)
#define EG_MAX_EIPIXELMODE          EG_EIPIXELMODE_ALPHA

#define EG_EICOMPMODE_NONE          (0)
#define EG_EICOMPMODE_RLE           (1)
#define EG_EICOMPMODE_EFICOMPRESS   (2)


typedef struct {
  EG_IMAGE    *Image;
  CONST CHAR16      *Path;
  CONST CHAR16      *Format;
  UINTN       PixelSize;
} BUILTIN_ICON;


/* functions */

VOID    egInitScreen(IN BOOLEAN SetMaxResolution);
VOID    egDumpGOPVideoModes(VOID);
//EFI_STATUS egSetScreenResolution(IN CHAR16 *WidthHeight); 
//EFI_STATUS egSetMaxResolution(VOID);
EFI_STATUS egSetMode(INT32 Next);

VOID    egGetScreenSize(OUT INTN *ScreenWidth, OUT INTN *ScreenHeight);
CONST CHAR16* egScreenDescription(VOID);
BOOLEAN egHasGraphicsMode(VOID);
BOOLEAN egIsGraphicsModeEnabled(VOID);
VOID    egSetGraphicsModeEnabled(IN BOOLEAN Enable);
// NOTE: Even when egHasGraphicsMode() returns FALSE, you should
//  call egSetGraphicsModeEnabled(FALSE) to ensure the system
//  is running in text mode. egHasGraphicsMode() only determines
//  if libeg can draw to the screen in graphics mode.

EG_IMAGE * egCreateImage(IN INTN Width, IN INTN Height, IN BOOLEAN HasAlpha);
EG_IMAGE * egCreateFilledImage(IN INTN Width, IN INTN Height, IN BOOLEAN HasAlpha, IN EG_PIXEL *Color);
EG_IMAGE * egCopyImage(IN EG_IMAGE *Image);
EG_IMAGE * egCopyScaledImage(IN EG_IMAGE *Image, IN INTN Ratio);
VOID       egFreeImage(IN EG_IMAGE *Image);
VOID      ScaleImage(OUT EG_IMAGE *NewImage, IN EG_IMAGE *OldImage);

EG_IMAGE * egLoadImage(IN EFI_FILE_HANDLE BaseDir, IN CONST CHAR16 *FileName, IN BOOLEAN WantAlpha);
EG_IMAGE * egLoadIcon(IN EFI_FILE_HANDLE BaseDir, IN CONST CHAR16 *FileName, IN UINTN IconSize);

EG_IMAGE * egEnsureImageSize(IN EG_IMAGE *Image, IN INTN Width, IN INTN Height, IN EG_PIXEL *Color);

EFI_STATUS egLoadFile(IN EFI_FILE_HANDLE BaseDir, IN CONST CHAR16 *FileName,
                      OUT UINT8 **FileData, OUT UINTN *FileDataLength);
EFI_STATUS egSaveFile(IN EFI_FILE_HANDLE BaseDir OPTIONAL, IN CONST CHAR16 *FileName,
                      IN CONST VOID *FileData, IN UINTN FileDataLength);
EFI_STATUS egMkDir(IN EFI_FILE_HANDLE BaseDir OPTIONAL, IN CONST CHAR16 *DirName);
EFI_STATUS egFindESP(OUT EFI_FILE_HANDLE *RootDir);

VOID egFillImage(IN OUT EG_IMAGE *CompImage, IN EG_PIXEL *Color);
VOID egFillImageArea(IN OUT EG_IMAGE *CompImage,
                     IN INTN AreaPosX, IN INTN AreaPosY,
                     IN INTN AreaWidth, IN INTN AreaHeight,
                     IN EG_PIXEL *Color);
VOID egComposeImage(IN OUT EG_IMAGE *CompImage, IN EG_IMAGE *TopImage, IN INTN PosX, IN INTN PosY);
VOID PrepareFont(VOID);
VOID egMeasureText(IN CONST CHAR16 *Text, OUT INTN *Width, OUT INTN *Height);
INTN egRenderText(IN CONST CHAR16 *Text, IN OUT EG_IMAGE *CompImage, IN INTN PosX, IN INTN PosY, IN INTN Cursor, INTN textType);

VOID egClearScreen(IN EG_PIXEL *Color);
//VOID egDrawImage(IN EG_IMAGE *Image, IN INTN ScreenPosX, IN INTN ScreenPosY);
// will be replaced by XImage.Draw(ScreenPosX, ScreenPosY, 1.f); assuming Area* = 0
VOID egDrawImageArea(IN EG_IMAGE *Image,
                     IN INTN AreaPosX, IN INTN AreaPosY,
                     IN INTN AreaWidth, IN INTN AreaHeight,
                     IN INTN ScreenPosX, IN INTN ScreenPosY);
VOID egTakeImage(IN EG_IMAGE *Image, INTN ScreenPosX, INTN ScreenPosY,
                 IN INTN AreaWidth, IN INTN AreaHeight);

EFI_STATUS egScreenShot(VOID);

INTN drawSVGtext(EG_IMAGE* TextBufferXY, INTN posX, INTN posY, INTN textType, CONST CHAR16* text, UINTN Cursor);
VOID testSVG(VOID);

#endif /* __LIBEG_LIBEG_H__ */

/* EOF */
