/*
* refit/scan/bootscreen.c
*
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

#include "entry_scan.h"

#ifndef DEBUG_ALL
#define DEBUG_LOCK_BOOT_SCREEN 1
#else
#define DEBUG_LOCK_BOOT_SCREEN DEBUG_ALL
#endif

#if DEBUG_LOCK_BOOT_SCREEN == 0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_LOCK_BOOT_SCREEN, __VA_ARGS__)
#endif

static BOOLEAN                      ScreenIsLocked;
static EFI_UGA_DRAW_PROTOCOL        OldUgaDraw;
static EFI_GRAPHICS_OUTPUT_PROTOCOL OldGraphicsOutput;

static EFI_GRAPHICS_OUTPUT_MODE_INFORMATION GraphicsOutputInfo = {
   // Version == 0
   0,
   // Horizontal resolution in pixels
   0,
   // Vertical resolution in pixels
   0,
   // Pixel format
   PixelBlueGreenRedReserved8BitPerColor,
   // Pixel mask
   { 0, 0, 0, 0 },
   // Pixels per scan line
   0,
};
static EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE GraphicsOutputMode = {
   // Max mode
   1,
// Current mode
0,
// Information
&GraphicsOutputInfo,
// Size of information in bytes
sizeof(EFI_GRAPHICS_OUTPUT_MODE_INFORMATION),
// Framebuffer
(EFI_PHYSICAL_ADDRESS)(UINTN)NULL,
// Framebuffer size in bytes
0
};

static EFI_STATUS EFIAPI LockedGraphicsOutputBlt(IN EFI_GRAPHICS_OUTPUT_PROTOCOL *This, IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL *BltBuffer, OPTIONAL IN EFI_GRAPHICS_OUTPUT_BLT_OPERATION BltOperation, IN UINTN SourceX, IN UINTN SourceY, IN UINTN DestinationX, IN UINTN DestinationY, IN UINTN Width, IN UINTN Height, IN UINTN Delta OPTIONAL)
{
   return EFI_SUCCESS;
}
static EFI_STATUS EFIAPI LockedGraphicsOutputSetMode(IN EFI_GRAPHICS_OUTPUT_PROTOCOL *This, IN UINT32 ModeNumber)
{
   if ((This == NULL) || (ModeNumber != 0)) {
      return EFI_INVALID_PARAMETER;
   }
   return EFI_SUCCESS;
}
static EFI_STATUS EFIAPI LockedGraphicsOutputQueryMode(IN EFI_GRAPHICS_OUTPUT_PROTOCOL *This, IN UINT32 ModeNumber, OUT UINTN *SizeOfInfo, OUT EFI_GRAPHICS_OUTPUT_MODE_INFORMATION **Info)
{
   if ((This == NULL) ||
      (SizeOfInfo == NULL) ||
      (Info == NULL) ||
      (ModeNumber != 0)) {
      return EFI_INVALID_PARAMETER;
   }
   *Info = &GraphicsOutputInfo;
   *SizeOfInfo = sizeof(EFI_GRAPHICS_OUTPUT_MODE_INFORMATION);
   return EFI_SUCCESS;
}

static UINT32 UgaDrawHorizontalResolution;
static UINT32 UgaDrawVerticalResolution;
static UINT32 UgaDrawColorDepth;
static UINT32 UgaDrawRefreshRate;

static EFI_STATUS EFIAPI LockedUgaDrawBlt(IN EFI_UGA_DRAW_PROTOCOL *This, IN EFI_UGA_PIXEL *BltBuffer, OPTIONAL IN EFI_UGA_BLT_OPERATION BltOperation, IN UINTN SourceX, IN UINTN SourceY, IN UINTN DestinationX, IN UINTN DestinationY, IN UINTN Width, IN UINTN Height, IN UINTN Delta OPTIONAL)
{
   return EFI_SUCCESS;
}
static EFI_STATUS EFIAPI LockedUgaDrawSetMode(IN EFI_UGA_DRAW_PROTOCOL *This, IN UINT32 HorizontalResolution, IN UINT32 VerticalResolution, IN UINT32 ColorDepth, IN UINT32 RefreshRate)
{
   if (This == NULL){
      return EFI_INVALID_PARAMETER;
   }
   if ((HorizontalResolution == UgaDrawHorizontalResolution) &&
      (VerticalResolution == UgaDrawVerticalResolution) &&
      (ColorDepth == UgaDrawColorDepth) &&
      (RefreshRate == UgaDrawRefreshRate)) {
      return EFI_SUCCESS;
   }
   return EFI_UNSUPPORTED;
}
static EFI_STATUS EFIAPI LockedUgaDrawGetMode(IN EFI_UGA_DRAW_PROTOCOL *This, OUT UINT32 *HorizontalResolution, OUT UINT32 *VerticalResolution, OUT UINT32 *ColorDepth, OUT UINT32 *RefreshRate)
{
   if ((This == NULL) ||
      (HorizontalResolution == NULL) ||
      (VerticalResolution == NULL) ||
      (ColorDepth == NULL) ||
      (RefreshRate == NULL)) {
      return EFI_INVALID_PARAMETER;
   }
   *HorizontalResolution = UgaDrawHorizontalResolution;
   *VerticalResolution = UgaDrawVerticalResolution;
   *ColorDepth = UgaDrawColorDepth;
   *RefreshRate = UgaDrawRefreshRate;
   return EFI_SUCCESS;
}

static EFI_BOOT_SERVICES OldBootServices;

static EFI_STATUS LockedLocateHandle(IN EFI_LOCATE_SEARCH_TYPE SearchType, IN EFI_GUID *Protocol OPTIONAL, IN VOID *SearchKey OPTIONAL, IN OUT UINTN *BufferSize, OUT EFI_HANDLE *Buffer)
{
  if (SearchType == ByProtocol) {
    if ((Protocol == NULL) || (BufferSize == NULL)) {
      return EFI_INVALID_PARAMETER;
    }
    if (CompareMem(Protocol, &gEfiGraphicsOutputProtocolGuid, sizeof(EFI_GUID)) == 0) {
      if (*BufferSize < sizeof(SelfImageHandle)) {
        *BufferSize = sizeof(SelfImageHandle);
        return EFI_BUFFER_TOO_SMALL;
      }
      if (Buffer == NULL) {
        return EFI_INVALID_PARAMETER;
      }
      *Buffer = SelfImageHandle;
      return EFI_SUCCESS;
    }
  }
  return OldBootServices.LocateHandle(SearchType, Protocol, SearchKey, BufferSize, Buffer);
}

static EFI_STATUS EFIAPI LockedOpenProtocol(IN EFI_HANDLE Handle, IN EFI_GUID *Protocol, OUT VOID **Interface OPTIONAL, IN EFI_HANDLE AgentHandle, IN EFI_HANDLE ControllerHandle, IN UINT32 Attributes)
{
  if ((Attributes & EFI_OPEN_PROTOCOL_GET_PROTOCOL) != 0) {
    if ((Protocol == NULL) || (Interface == NULL)) {
      return EFI_INVALID_PARAMETER;
    }
    if (CompareMem(Protocol, &gEfiGraphicsOutputProtocolGuid, sizeof(EFI_GUID)) == 0) {
      if (Handle != SelfImageHandle) {
         return EFI_UNSUPPORTED;
      }
      *Interface = egGetGOP();
      return EFI_SUCCESS;
    } else if (CompareMem(Protocol, &gEfiDevicePathProtocolGuid, sizeof(EFI_GUID)) == 0) {
      if (Handle == SelfImageHandle) {
        *Interface = SelfDevicePath;
        return EFI_SUCCESS;
      }
    }
  }
  return OldBootServices.OpenProtocol(Handle, Protocol, Interface, AgentHandle, ControllerHandle, Attributes);
}

static EFI_STATUS EFIAPI LockedCloseProtocol(IN EFI_HANDLE Handle, IN EFI_GUID *Protocol, IN EFI_HANDLE AgentHandle, IN EFI_HANDLE ControllerHandle)
{
  if (Handle == SelfImageHandle) {
    if (Protocol == NULL) {
      return EFI_INVALID_PARAMETER;
    }
    if ((CompareMem(Protocol, &gEfiGraphicsOutputProtocolGuid, sizeof(EFI_GUID)) == 0) ||
        (CompareMem(Protocol, &gEfiDevicePathProtocolGuid, sizeof(EFI_GUID)) == 0)) {
      return EFI_SUCCESS;
    }
  }
  return OldBootServices.CloseProtocol(Handle, Protocol, AgentHandle, ControllerHandle);
}

EFI_STATUS LockBootScreen(VOID)
{
  INTN Width, Height;
  EFI_GRAPHICS_OUTPUT_PROTOCOL *GraphicsOutput;
  EFI_UGA_DRAW_PROTOCOL *UgaDraw;
  // Make sure we're not locked
  if (ScreenIsLocked) {
     DBG("Custom boot screen is already locked\n");
    return EFI_ACCESS_DENIED;
  }
  DBG("Custom boot lock");
  // Get screen size
  egGetScreenSize(&Width, &Height);
  // Lock GOP
  GraphicsOutput = egGetGOP();
  if (GraphicsOutput != NULL) {
     DBG(" GOP");
    // Remember old gop
    CopyMem(&OldGraphicsOutput, GraphicsOutput, sizeof(EFI_GRAPHICS_OUTPUT_PROTOCOL));
    // Setup locked gop
    GraphicsOutput->Blt = LockedGraphicsOutputBlt;
    GraphicsOutput->SetMode = LockedGraphicsOutputSetMode;
    GraphicsOutput->QueryMode = LockedGraphicsOutputQueryMode;
    // Setup mode
    GraphicsOutputInfo.HorizontalResolution = (UINT32)Width;
    GraphicsOutputInfo.PixelsPerScanLine = (UINT32)Width;
    GraphicsOutputInfo.VerticalResolution = (UINT32)Height;
    GraphicsOutputMode.FrameBufferSize = (UINT32)(Width * Height * 4);
    GraphicsOutputMode.FrameBufferBase = (EFI_PHYSICAL_ADDRESS)(UINTN)AllocatePool(GraphicsOutputMode.FrameBufferSize);
    // Set mode
    GraphicsOutput->Mode = &GraphicsOutputMode;
  }
  // Lock UGA
  UgaDraw = egGetUGA();
  if (UgaDraw != NULL) {
    DBG(" UGA");
    // Remember old uga
    CopyMem(&OldUgaDraw, UgaDraw, sizeof(EFI_UGA_DRAW_PROTOCOL));
    UgaDraw->GetMode(UgaDraw, &UgaDrawHorizontalResolution, &UgaDrawVerticalResolution, &UgaDrawColorDepth, &UgaDrawRefreshRate);
    // Setup locked uga
    UgaDraw->Blt = LockedUgaDrawBlt;
    UgaDraw->SetMode = LockedUgaDrawSetMode;
    UgaDraw->GetMode = LockedUgaDrawGetMode;
  }
  DBG(" BS Screen\n");
  // Lock locate handle, open and close protocol
  CopyMem(&OldBootServices, gBS, sizeof(EFI_BOOT_SERVICES));
  gBS->LocateHandle = LockedLocateHandle;
  gBS->OpenProtocol = LockedOpenProtocol;
  gBS->CloseProtocol = LockedCloseProtocol;
  gBS->Hdr.CRC32 = 0;
  gBS->CalculateCrc32(gBS, gBS->Hdr.HeaderSize, &(gBS->Hdr.CRC32));
  // Lock screen
  ScreenIsLocked = TRUE;
  return EFI_SUCCESS;
}

EFI_STATUS UnlockBootScreen(VOID)
{
  EFI_GRAPHICS_OUTPUT_PROTOCOL *GraphicsOutput;
  EFI_UGA_DRAW_PROTOCOL *UgaDraw;
  // Make sure we're locked
  if (!ScreenIsLocked) {
    DBG("Custom boot screen is not locked\n");
    return EFI_NOT_READY;
  }
  DBG("Custom boot unlock");
  // Restore GOP
  GraphicsOutput = egGetGOP();
  if (GraphicsOutput != NULL) {
    DBG(" GOP");
    // Free locked framebuffer
    if (GraphicsOutputMode.FrameBufferBase != (EFI_PHYSICAL_ADDRESS)(UINTN)NULL) {
      FreePool((VOID *)(UINTN)GraphicsOutputMode.FrameBufferBase);
      GraphicsOutputMode.FrameBufferBase = (EFI_PHYSICAL_ADDRESS)(UINTN)NULL;
    }
    GraphicsOutputMode.FrameBufferSize = 0;
    CopyMem(GraphicsOutput, &OldGraphicsOutput, sizeof(EFI_GRAPHICS_OUTPUT_PROTOCOL));
  }
  // Restore UGA
  UgaDraw = egGetUGA();
  if (UgaDraw != NULL) {
    DBG(" UGA");
    CopyMem(UgaDraw, &OldUgaDraw, sizeof(EFI_UGA_DRAW_PROTOCOL));
  }
  DBG(" BS Screen\n");
  // Restore locate handle, open and close protocol
  CopyMem(gBS, &OldBootServices, sizeof(EFI_BOOT_SERVICES));
  // Unlock
  ScreenIsLocked = FALSE;
  return EFI_SUCCESS;
}
