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

#include <Platform.h>
#include "entry_scan.h"
#include "../Platform/Self.h"

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

// Each time a new graphics protocol is opened,
// lock it and save the information here
typedef struct LOCKED_GRAPHICS LOCKED_GRAPHICS;
struct LOCKED_GRAPHICS
{
  LOCKED_GRAPHICS                      *Next;
  EFI_HANDLE                            Owner;
  EFI_HANDLE                            Agent;
  EFI_HANDLE                            Controller;
  UINT16                                GOPCount;
  UINT16                                UGACount;
  EFI_GRAPHICS_OUTPUT_PROTOCOL         *GOPInterface;
  EFI_GRAPHICS_OUTPUT_PROTOCOL          GOP;
  EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE     GOPMode;
  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION  GOPInfo;
  EFI_UGA_DRAW_PROTOCOL                *UGAInterface;
  EFI_UGA_DRAW_PROTOCOL                 UGA;
};

// The old boot services saved to restore later
static EFI_BOOT_SERVICES  OldBootServices;
// The locked graphics collection
static LOCKED_GRAPHICS   *LockedGraphics;
// The screen lock
static BOOLEAN            ScreenIsLocked;

static EFI_STATUS EFIAPI LockedGOPBlt(IN EFI_GRAPHICS_OUTPUT_PROTOCOL *This, IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL *BltBuffer, OPTIONAL IN EFI_GRAPHICS_OUTPUT_BLT_OPERATION BltOperation, IN UINTN SourceX, IN UINTN SourceY, IN UINTN DestinationX, IN UINTN DestinationY, IN UINTN Width, IN UINTN Height, IN UINTN Delta OPTIONAL)
{
  return EFI_SUCCESS;
}
static EFI_STATUS EFIAPI LockedGOPSetMode(IN EFI_GRAPHICS_OUTPUT_PROTOCOL *This, IN UINT32 ModeNumber)
{
  if ((This == NULL) || (ModeNumber != 0)) {
    return EFI_INVALID_PARAMETER;
  }
  return EFI_SUCCESS;
}
static EFI_STATUS EFIAPI LockedGOPQueryMode(IN EFI_GRAPHICS_OUTPUT_PROTOCOL *This, IN UINT32 ModeNumber, OUT UINTN *SizeOfInfo, OUT EFI_GRAPHICS_OUTPUT_MODE_INFORMATION **Info)
{
  if ((This == NULL) ||
      (This->Mode == NULL) ||
      (This->Mode->Info == NULL) ||
      (SizeOfInfo == NULL) ||
      (Info == NULL) ||
      (ModeNumber != 0)) {
     return EFI_INVALID_PARAMETER;
  }
  *Info = This->Mode->Info;
  *SizeOfInfo = This->Mode->SizeOfInfo;
  return EFI_SUCCESS;
}

static EFI_STATUS AddLockedGraphicsGOP(IN EFI_HANDLE Handle, IN EFI_HANDLE AgentHandle, IN EFI_HANDLE ControllerHandle, IN EFI_GRAPHICS_OUTPUT_PROTOCOL *GOPInterface)
{
  BOOLEAN          Modify = TRUE;
  LOCKED_GRAPHICS *Ptr = LockedGraphics;
  if ((Handle == NULL) ||
      (AgentHandle == NULL) ||
      (GOPInterface == NULL)) {
    return EFI_INVALID_PARAMETER;
  }
  // Check if this is already been locked
  while (Ptr != NULL) {
    if ((Ptr->Owner == Handle) &&
        (Ptr->Agent == AgentHandle) &&
        (Ptr->Controller == ControllerHandle)) {
      if (Ptr->GOPCount == 0) {
        break;
      }
      ++(Ptr->GOPCount);
      return EFI_SUCCESS;
    }
    if (Ptr->GOPInterface == GOPInterface) {
      Modify = FALSE;
    }
    Ptr = Ptr->Next;
  }
  // Create a new locked graphics if needed
  if (Ptr == NULL) {
    Ptr = (LOCKED_GRAPHICS *)AllocateZeroPool(sizeof(LOCKED_GRAPHICS));
    if (Ptr == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
    // Add to list
    Ptr->Next = LockedGraphics;
    LockedGraphics = Ptr;
  }
  // Store these elements for later
  Ptr->Owner = Handle;
  Ptr->Agent = AgentHandle;
  Ptr->Controller = ControllerHandle;
  CopyMem(&(Ptr->GOP), GOPInterface, sizeof(EFI_GRAPHICS_OUTPUT_PROTOCOL));
  CopyMem(&(Ptr->GOPMode), GOPInterface->Mode, sizeof(EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE));
  CopyMem(&(Ptr->GOPInfo), GOPInterface->Mode->Info, GOPInterface->Mode->SizeOfInfo);
  Ptr->GOPInterface = GOPInterface;
  Ptr->GOPCount = 1;
  // Alter the interface
  if (Modify) {
    GOPInterface->Blt = LockedGOPBlt;
    GOPInterface->SetMode = LockedGOPSetMode;
    GOPInterface->QueryMode = LockedGOPQueryMode;
    if (GOPInterface->Mode != NULL) {
      UINTN BufferSize = (1024 * 768 * 4); // (GOPInterface->Mode->Info->HorizontalResolution * GOPInterface->Mode->Info->VerticalResolution * 4);
      GOPInterface->Mode->MaxMode = 1;
      GOPInterface->Mode->Mode = 0;
      GOPInterface->Mode->SizeOfInfo = sizeof(EFI_GRAPHICS_OUTPUT_MODE_INFORMATION);
		DBG("Custom boot framebuffer 0x%llX 0x%llX\n", (uintptr_t)GOPInterface->Mode->FrameBufferBase, GOPInterface->Mode->FrameBufferSize);
      GOPInterface->Mode->FrameBufferBase = (EFI_PHYSICAL_ADDRESS)(UINTN)(GOPInterface->Mode->FrameBufferBase + GOPInterface->Mode->FrameBufferSize);
      GOPInterface->Mode->FrameBufferSize = BufferSize;
		DBG("Custom boot GOP: 0x%llX @0x%llX 0x%llX", (uintptr_t)GOPInterface, (uintptr_t)GOPInterface->Mode->FrameBufferBase, BufferSize);
      if (GOPInterface->Mode->Info != NULL) {
        // /*
        GOPInterface->Mode->Info->Version = 0;
        GOPInterface->Mode->Info->HorizontalResolution = 1024;
        GOPInterface->Mode->Info->PixelsPerScanLine = 1024;
        GOPInterface->Mode->Info->VerticalResolution = 768;
        GOPInterface->Mode->Info->PixelFormat = PixelBlueGreenRedReserved8BitPerColor;
        // */
        DBG(" %d(%d)x%d", GOPInterface->Mode->Info->HorizontalResolution, GOPInterface->Mode->Info->PixelsPerScanLine, GOPInterface->Mode->Info->VerticalResolution);
      }
      DBG("\n");
    }
  }
  return EFI_SUCCESS;
}
static EFI_STATUS RestoreLockedGraphicsGOP(IN LOCKED_GRAPHICS *Graphics)
{
  if (Graphics == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  if (Graphics->GOPInterface == NULL) {
    Graphics->GOPCount = 0;
    return EFI_SUCCESS;
  }
  // Restore GOP
  CopyMem(Graphics->GOPInterface, &(Graphics->GOP), sizeof(EFI_GRAPHICS_OUTPUT_PROTOCOL));
  if (Graphics->GOPInterface->Mode != NULL) {
    // Restore mode
    CopyMem(Graphics->GOPInterface->Mode, &(Graphics->GOPMode), sizeof(EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE));
    if (Graphics->GOPInterface->Mode->Info != NULL) {
      // Restore info
      CopyMem(Graphics->GOPInterface->Mode->Info, &(Graphics->GOPInfo), sizeof(EFI_GRAPHICS_OUTPUT_MODE_INFORMATION));
    }
  }
  Graphics->GOPInterface = NULL;
  Graphics->GOPCount = 0;
  return EFI_SUCCESS;
}
static EFI_STATUS RemoveLockedGraphics(IN LOCKED_GRAPHICS *Graphics)
{
  if (Graphics == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  // Only remove if not in use
  if ((Graphics->GOPInterface != NULL) ||
      (Graphics->UGAInterface != NULL)) {
    return EFI_SUCCESS;
  }
  // Check if first one
  if (Graphics == LockedGraphics) {
    LockedGraphics = Graphics->Next;
    FreePool(Graphics);
    return EFI_SUCCESS;
  } else if (LockedGraphics != NULL) {
    // Check the next ones
    LOCKED_GRAPHICS *Ptr = LockedGraphics;
    while (Ptr->Next != NULL) {
      if (Ptr->Next == Graphics) {
        Ptr->Next = Graphics->Next;
        FreePool(Graphics);
        return EFI_SUCCESS;
      }
      Ptr = Ptr->Next;
    }
  }
  // Not found
  return EFI_NOT_FOUND;
}
static EFI_STATUS RemoveLockedGraphicsGOP(IN EFI_HANDLE Handle, IN EFI_HANDLE AgentHandle, IN EFI_HANDLE ControllerHandle)
{
  LOCKED_GRAPHICS *Ptr = LockedGraphics;
  while (Ptr != NULL) {
    if ((Ptr->Owner == Handle) ||
        (Ptr->Agent == AgentHandle) ||
        (Ptr->Controller == ControllerHandle)) {
      if (Ptr->GOPCount <= 1) {
        EFI_STATUS Status = RestoreLockedGraphicsGOP(Ptr);
        if (EFI_ERROR(Status)) {
          return Status;
        }
        return RemoveLockedGraphics(Ptr);
      }
      --(Ptr->GOPCount);
      return EFI_SUCCESS;
    }
    Ptr = Ptr->Next;
  }
  return EFI_NOT_FOUND;
}

static  EFI_STATUS EFIAPI LockedUGASetMode(IN EFI_UGA_DRAW_PROTOCOL *This, IN UINT32 HorizontalResolution, IN UINT32 VerticalResolution, IN UINT32 ColorDepth, IN UINT32 RefreshRate)
{
   return EFI_UNSUPPORTED;
}
static EFI_STATUS EFIAPI LockedUGABlt(IN EFI_UGA_DRAW_PROTOCOL *This, IN EFI_UGA_PIXEL *BltBuffer, OPTIONAL IN EFI_UGA_BLT_OPERATION BltOperation, IN UINTN SourceX, IN UINTN SourceY, IN UINTN DestinationX, IN UINTN DestinationY, IN UINTN Width, IN UINTN Height, IN UINTN Delta OPTIONAL)
{
   return EFI_SUCCESS;
}

static EFI_STATUS RestoreLockedGraphicsUGA(IN LOCKED_GRAPHICS *Graphics)
{
   if (Graphics == NULL) {
      return EFI_INVALID_PARAMETER;
   }
   if (Graphics->UGAInterface == NULL) {
      Graphics->UGACount = 0;
      return EFI_SUCCESS;
   }
   // Restore UGA
   CopyMem(Graphics->UGAInterface, &(Graphics->UGA), sizeof(EFI_UGA_DRAW_PROTOCOL));
   Graphics->UGAInterface = NULL;
   Graphics->UGACount = 0;
   return EFI_SUCCESS;
}
static EFI_STATUS RemoveLockedGraphicsUGA(IN EFI_HANDLE Handle, IN EFI_HANDLE AgentHandle, IN EFI_HANDLE ControllerHandle)
{
  LOCKED_GRAPHICS *Ptr = LockedGraphics;
  while (Ptr != NULL) {
    if ((Ptr->Owner == Handle) ||
        (Ptr->Agent == AgentHandle) ||
        (Ptr->Controller == ControllerHandle)) {
      if (Ptr->UGACount <= 1) {
        EFI_STATUS Status = RestoreLockedGraphicsUGA(Ptr);
        if (EFI_ERROR(Status)) {
          return Status;
        }
        return RemoveLockedGraphics(Ptr);
      }
      --(Ptr->UGACount);
      return EFI_SUCCESS;
    }
    Ptr = Ptr->Next;
  }
  return EFI_NOT_FOUND;
}

static EFI_STATUS AddLockedGraphicsUGA(IN EFI_HANDLE Handle, IN EFI_HANDLE AgentHandle, IN EFI_HANDLE ControllerHandle, IN EFI_UGA_DRAW_PROTOCOL *UGAInterface)
{
  BOOLEAN          Modify = TRUE;
  LOCKED_GRAPHICS *Ptr = LockedGraphics;
  if ((Handle == NULL) ||
      (AgentHandle == NULL) ||
      (UGAInterface == NULL)) {
    return EFI_INVALID_PARAMETER;
  }
  // Check if this is already been locked
  while (Ptr != NULL) {
    if ((Ptr->Owner == Handle) &&
        (Ptr->Agent == AgentHandle) &&
        (Ptr->Controller == ControllerHandle)) {
      if (Ptr->UGACount == 0) {
        break;
      }
      ++(Ptr->UGACount);
      return EFI_SUCCESS;
    }
    if (Ptr->UGAInterface == UGAInterface) {
       Modify = FALSE;
    }
    Ptr = Ptr->Next;
  }
  // Create a new locked graphics if needed
  if (Ptr == NULL) {
    Ptr = (LOCKED_GRAPHICS *)AllocateZeroPool(sizeof(LOCKED_GRAPHICS));
    if (Ptr == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
    // Add to list
    Ptr->Next = LockedGraphics;
    LockedGraphics = Ptr;
  }
  // Store these elements for later
  Ptr->Owner = Handle;
  Ptr->Agent = AgentHandle;
  Ptr->Controller = ControllerHandle;
  CopyMem(&(Ptr->UGA), UGAInterface, sizeof(EFI_UGA_DRAW_PROTOCOL));
  Ptr->UGAInterface = UGAInterface;
  Ptr->UGACount = 1;
  // Alter the interface
  if (Modify) {
    UGAInterface->Blt = LockedUGABlt;
    UGAInterface->SetMode = LockedUGASetMode;
  }
  return EFI_SUCCESS;
}

static EFI_STATUS EFIAPI LockedOpenProtocol(IN EFI_HANDLE Handle, IN EFI_GUID *Protocol, OUT void **Interface OPTIONAL, IN EFI_HANDLE AgentHandle, IN EFI_HANDLE ControllerHandle, IN UINT32 Attributes)
{
  if ((Attributes & EFI_OPEN_PROTOCOL_GET_PROTOCOL) != 0) {
    if ((Protocol == NULL) || (Interface == NULL)) {
      return EFI_INVALID_PARAMETER;
    }
    if (CompareMem(Protocol, &gEfiGraphicsOutputProtocolGuid, sizeof(EFI_GUID)) == 0) {
      EFI_GRAPHICS_OUTPUT_PROTOCOL *GOPInterface = NULL;
      // Open the actual protocol
      EFI_STATUS Status = OldBootServices.OpenProtocol(Handle, Protocol, (void **)&GOPInterface, AgentHandle, ControllerHandle, Attributes);
      if (EFI_ERROR(Status)) {
        return Status;
      }
      if (GOPInterface == NULL) {
        return EFI_NOT_FOUND;
      }
      // Save this protocol for later and alter it
      if (EFI_ERROR(Status = AddLockedGraphicsGOP(Handle, AgentHandle, ControllerHandle, GOPInterface))) {
        return Status;
      }
      // Return the altered protocol
      *Interface = GOPInterface;
      return EFI_SUCCESS;
    } else if (CompareMem(Protocol, &gEfiUgaDrawProtocolGuid, sizeof(EFI_GUID)) == 0) {
      EFI_UGA_DRAW_PROTOCOL *UGAInterface = NULL;
      // Open the actual protocol
      EFI_STATUS Status = OldBootServices.OpenProtocol(Handle, Protocol, (void **)&UGAInterface, AgentHandle, ControllerHandle, Attributes);
      if (EFI_ERROR(Status)) {
        return Status;
      }
      if (UGAInterface == NULL) {
        return EFI_NOT_FOUND;
      }
      // Save this protocol for later and alter it
      if (EFI_ERROR(Status = AddLockedGraphicsUGA(Handle, AgentHandle, ControllerHandle, UGAInterface))) {
        return Status;
      }
      // Return the altered protocol
      *Interface = UGAInterface;
      return EFI_SUCCESS;
    }
  }
  return OldBootServices.OpenProtocol(Handle, Protocol, Interface, AgentHandle, ControllerHandle, Attributes);
}

static EFI_STATUS EFIAPI LockedCloseProtocol(IN EFI_HANDLE Handle, IN EFI_GUID *Protocol, IN EFI_HANDLE AgentHandle, IN EFI_HANDLE ControllerHandle)
{
  if (Handle == self.getSelfImageHandle()) {
    if (Protocol == NULL) {
      return EFI_INVALID_PARAMETER;
    }
    if (CompareMem(Protocol, &gEfiGraphicsOutputProtocolGuid, sizeof(EFI_GUID)) == 0) {
      // Remove a reference to locked graphics
      EFI_STATUS Status = RemoveLockedGraphicsGOP(Handle, AgentHandle, ControllerHandle);
      if (EFI_ERROR(Status)) {
        return Status;
      }
    } else if (CompareMem(Protocol, &gEfiUgaDrawProtocolGuid, sizeof(EFI_GUID)) == 0) {
      // Remove a reference to locked graphics
      EFI_STATUS Status = RemoveLockedGraphicsUGA(Handle, AgentHandle, ControllerHandle);
      if (EFI_ERROR(Status)) {
        return Status;
      }
    }
  }
  return OldBootServices.CloseProtocol(Handle, Protocol, AgentHandle, ControllerHandle);
}

// Lock the graphics GOP
static EFI_STATUS LockGraphicsGOP(void)
{
  EFI_HANDLE *Buffer;
  void       *Interface;
  UINTN       i, Size = 0;
  // Get needed buffer size
  EFI_STATUS  Status = gBS->LocateHandle(ByProtocol, &gEfiGraphicsOutputProtocolGuid, NULL, &Size, NULL);
  if ((Status != EFI_BUFFER_TOO_SMALL) || (Size == 0)) {
    return Status;
  }
  // Allocate buffer
  Buffer = (EFI_HANDLE *)AllocatePool(Size);
  if (Buffer == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  // Locate handles for GOP
  Status = gBS->LocateHandle(ByProtocol, &gEfiGraphicsOutputProtocolGuid, NULL, &Size, Buffer);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  // Open GOP protocols, they will be modified by our modified boot services
  Size /= sizeof(EFI_HANDLE *);
  for (i = 0; i < Size; ++i) {
     gBS->OpenProtocol(Buffer[i], &gEfiGraphicsOutputProtocolGuid, &Interface, self.getSelfImageHandle(), NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
  }
  return EFI_SUCCESS;
}

// Lock the graphics UGA
static EFI_STATUS LockGraphicsUGA(void)
{
   EFI_HANDLE *Buffer;
   void       *Interface;
   UINTN       i, Size = 0;
   // Get needed buffer size
   EFI_STATUS  Status = gBS->LocateHandle(ByProtocol, &gEfiUgaDrawProtocolGuid, NULL, &Size, NULL);
   if ((Status != EFI_BUFFER_TOO_SMALL) || (Size == 0)) {
      return Status;
   }
   // Allocate buffer
   Buffer = (EFI_HANDLE *)AllocatePool(Size);
   if (Buffer == NULL) {
      return EFI_OUT_OF_RESOURCES;
   }
   // Locate handles for UGA
   Status = gBS->LocateHandle(ByProtocol, &gEfiUgaDrawProtocolGuid, NULL, &Size, Buffer);
   if (EFI_ERROR(Status)) {
      return Status;
   }
   // Open UGA protocols, they will be modified by our modified boot services
   Size /= sizeof(EFI_HANDLE *);
   for (i = 0; i < Size; ++i) {
      gBS->OpenProtocol(Buffer[i], &gEfiUgaDrawProtocolGuid, &Interface, self.getSelfImageHandle(), NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
   }
   return EFI_SUCCESS;
}

// Lock the graphics
EFI_STATUS LockBootScreen(void)
{
  // Make sure we're not locked
  if (ScreenIsLocked) {
     DBG("Custom boot screen is already locked\n");
    return EFI_ACCESS_DENIED;
  }
  DBG("Custom boot lock\n");
  // Lock open and close protocol
  CopyMem(&OldBootServices, gBS, sizeof(EFI_BOOT_SERVICES));
  gBS->OpenProtocol = LockedOpenProtocol;
  gBS->CloseProtocol = LockedCloseProtocol;
  gBS->Hdr.CRC32 = 0;
  gBS->CalculateCrc32(gBS, gBS->Hdr.HeaderSize, &(gBS->Hdr.CRC32));
  // Find graphics and modify them
  LockGraphicsGOP();
  LockGraphicsUGA();
  // Lock screen
  ScreenIsLocked = TRUE;
  return EFI_SUCCESS;
}

// Unlock the graphics
EFI_STATUS UnlockBootScreen(void)
{
  // Make sure we're locked
  if (!ScreenIsLocked) {
    DBG("Custom boot screen is not locked\n");
    return EFI_NOT_READY;
  }
  DBG("Custom boot unlock\n");
  // Remove all locked graphics
  while (LockedGraphics != NULL) {
     LOCKED_GRAPHICS *Ptr = LockedGraphics;
     LockedGraphics = Ptr->Next;
     RestoreLockedGraphicsGOP(Ptr);
     RestoreLockedGraphicsUGA(Ptr);
     RemoveLockedGraphics(Ptr);
  }
  // Restore locate handle, open and close protocol
  CopyMem(gBS, &OldBootServices, sizeof(EFI_BOOT_SERVICES));
  // Unlock
  ScreenIsLocked = FALSE;
  return EFI_SUCCESS;
}
