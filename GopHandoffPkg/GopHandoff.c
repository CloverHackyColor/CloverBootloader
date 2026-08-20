#include <Uefi.h>

#include <IndustryStandard/Acpi.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Protocol/EdidActive.h>
#include <Protocol/EdidDiscovered.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/PciIo.h>

#include "Common/GopFramebufferProtocol.h"


#include "../rEFIt_UEFI/Platform/BootLog.h"

#define DEBUG_GH 0
#if DEBUG_GH==0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_GH, __VA_ARGS__)
#endif

#undef MDEPKG_NDEBUG

#define GOPHANDOFF_TARGET_PCI_VENDOR    0x8086U  /* Intel */
#define GOPHANDOFF_TARGET_PCI_SEGMENT   0U
#define GOPHANDOFF_TARGET_PCI_BUS       0U
#define GOPHANDOFF_TARGET_PCI_DEVICE    2U
#define GOPHANDOFF_TARGET_PCI_FUNCTION  0U


STATIC EFI_EVENT  mReadyToBootEvent = NULL;
STATIC EFI_HANDLE mImageHandle = NULL;

STATIC EFI_GUID mGopFramebufferVariableGuid = {
  0xb5a6f0a9,
  0x3e6f,
  0x4d90,
  { 0x8a, 0x1a, 0x2f, 0x9d, 0x1d, 0x2e, 0x8c, 0x71 }
};

STATIC CHAR16 mHandoffVariableName[] = L"GopFramebufferHandoff";
STATIC CHAR16 mPciInfoVariableName[] = L"GopFramebufferPciInfo";
STATIC CHAR16 mDisplayInfoVariableName[] = L"GopFramebufferDisplayInfo";
STATIC CHAR16 mModeRequestVariableName[] = L"GopFramebufferModeRequest";

STATIC
EFI_STATUS
FindPciIoForGopHandle (
                       IN  EFI_HANDLE           GopHandle,
                       OUT EFI_PCI_IO_PROTOCOL **PciIo
                       );

STATIC
EFI_STATUS
DeleteVariable (
                IN CHAR16 *VariableName
                )
{
  EFI_STATUS Status;
  
  if (VariableName == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  
  Status = gRT->SetVariable(
                            VariableName,
                            &mGopFramebufferVariableGuid,
                            0U,
                            0U,
                            NULL
                            );
  if (Status == EFI_NOT_FOUND) {
    return EFI_SUCCESS;
  }
  
  return Status;
}

STATIC
EFI_STATUS
DeleteOutputVariables (
                       VOID
                       )
{
  EFI_STATUS HandoffStatus;
  EFI_STATUS PciInfoStatus;
  EFI_STATUS DisplayInfoStatus;
  
  HandoffStatus = DeleteVariable(mHandoffVariableName);
  PciInfoStatus = DeleteVariable(mPciInfoVariableName);
  DisplayInfoStatus = DeleteVariable(mDisplayInfoVariableName);
  
  if (EFI_ERROR(HandoffStatus)) {
    DBG("HandoffStatus: %llx\n", HandoffStatus);
    return HandoffStatus;
  }
  if (EFI_ERROR(PciInfoStatus)) {
    DBG("PciInfoStatus: %llx\n", PciInfoStatus);
    return PciInfoStatus;
  }
  
  return DisplayInfoStatus;
}

STATIC
INTN
HighestSetBit (
               IN UINT32 Value
               )
{
  INTN Bit;
  
  for (Bit = 31; Bit >= 0; --Bit) {
    if ((Value & (1U << (UINT32)Bit)) != 0U) {
      return Bit;
    }
  }
  
  return -1;
}

STATIC
EFI_STATUS
DerivePixelLayout (
                   IN  CONST EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *ModeInfo,
                   OUT GopFramebufferModeDescriptor               *Mode
                   )
{
  UINT32 AllMasks;
  INTN   HighestBit;
  
  if ((ModeInfo == NULL) || (Mode == NULL)) {
    return EFI_INVALID_PARAMETER;
  }
  
  Mode->pixelFormat = (UINT32)ModeInfo->PixelFormat;
  
  switch (ModeInfo->PixelFormat) {
    case PixelRedGreenBlueReserved8BitPerColor:
      Mode->bytesPerPixel = 4U;
      Mode->redMask = 0x000000FFU;
      Mode->greenMask = 0x0000FF00U;
      Mode->blueMask = 0x00FF0000U;
      Mode->reservedMask = 0xFF000000U;
      return EFI_SUCCESS;
      
    case PixelBlueGreenRedReserved8BitPerColor:
      Mode->bytesPerPixel = 4U;
      Mode->redMask = 0x00FF0000U;
      Mode->greenMask = 0x0000FF00U;
      Mode->blueMask = 0x000000FFU;
      Mode->reservedMask = 0xFF000000U;
      return EFI_SUCCESS;
      
    case PixelBitMask:
      Mode->redMask = ModeInfo->PixelInformation.RedMask;
      Mode->greenMask = ModeInfo->PixelInformation.GreenMask;
      Mode->blueMask = ModeInfo->PixelInformation.BlueMask;
      Mode->reservedMask = ModeInfo->PixelInformation.ReservedMask;
      
      AllMasks = Mode->redMask |
      Mode->greenMask |
      Mode->blueMask |
      Mode->reservedMask;
      HighestBit = HighestSetBit(AllMasks);
      if (HighestBit < 0) {
        return EFI_UNSUPPORTED;
      }
      
      Mode->bytesPerPixel = ((UINT32)HighestBit + 1U + 7U) / 8U;
      return EFI_SUCCESS;
      
    case PixelBltOnly:
    case PixelFormatMax:
    default:
      return EFI_UNSUPPORTED;
  }
}

STATIC
EFI_STATUS
BuildModeDescriptor (
                     IN  UINT32                                     ModeNumber,
                     IN  CONST EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *ModeInfo,
                     OUT GopFramebufferModeDescriptor               *Mode
                     )
{
  EFI_STATUS Status;
  
  if ((ModeInfo == NULL) || (Mode == NULL)) {
    return EFI_INVALID_PARAMETER;
  }
  
  ZeroMem(Mode, sizeof(*Mode));
  Mode->modeNumber = ModeNumber;
  Mode->width = ModeInfo->HorizontalResolution;
  Mode->height = ModeInfo->VerticalResolution;
  Mode->pixelsPerScanLine = ModeInfo->PixelsPerScanLine;
  
  Status = DerivePixelLayout(ModeInfo, Mode);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  
  if (gopfb_validate_mode_descriptor(Mode) != GOPFB_STATUS_OK) {
    return EFI_UNSUPPORTED;
  }
  
  return EFI_SUCCESS;
}

STATIC
BOOLEAN
IsTargetPciIo (
               IN EFI_PCI_IO_PROTOCOL *PciIo
               )
{
  EFI_STATUS Status;
  UINTN      Segment;
  UINTN      Bus;
  UINTN      Device;
  UINTN      Function;
  UINT16     VendorId;
  UINT16     DeviceId;
  UINT32     RevisionClass;
  UINT32     ClassCode;
  
  if (PciIo == NULL) {
    return FALSE;
  }
  
  Segment  = 0U;
  Bus      = 0U;
  Device   = 0U;
  Function = 0U;
  
  Status = PciIo->GetLocation(
                              PciIo,
                              &Segment,
                              &Bus,
                              &Device,
                              &Function
                              );
  if (EFI_ERROR(Status)) {
    return FALSE;
  }
  
  VendorId      = 0xFFFFU;
  DeviceId      = 0xFFFFU;
  RevisionClass = 0U;
  
  if (EFI_ERROR(PciIo->Pci.Read(
                                PciIo,
                                EfiPciIoWidthUint16,
                                0x00U,
                                1U,
                                &VendorId
                                )) ||
      EFI_ERROR(PciIo->Pci.Read(
                                PciIo,
                                EfiPciIoWidthUint16,
                                0x02U,
                                1U,
                                &DeviceId
                                )) ||
      EFI_ERROR(PciIo->Pci.Read(
                                PciIo,
                                EfiPciIoWidthUint32,
                                0x08U,
                                1U,
                                &RevisionClass
                                ))) {
                                  return FALSE;
                                }
  
  ClassCode = (RevisionClass >> 8U) & 0x00FFFFFFU;
  
  DEBUG((
         DEBUG_INFO,
         "GopHandoff: candidate GOP PCI %04x:%02x:%02x.%x vendor=%04x device=%04x class=%06x\n",
         (UINT32)Segment,
         (UINT32)Bus,
         (UINT32)Device,
         (UINT32)Function,
         VendorId,
         DeviceId,
         ClassCode
         ));
  
  /* Только Intel */
  if (VendorId != GOPHANDOFF_TARGET_PCI_VENDOR) {
    return FALSE;
  }
  
  /* Только display controller, base class 0x03 */
  if ((ClassCode & 0xFF0000U) != 0x030000U) {
    return FALSE;
  }
  
  /*
   * Для desktop Intel iGPU обычно:
   * 0000:00:02.0
   */
  if ((Segment  == GOPHANDOFF_TARGET_PCI_SEGMENT) &&
      (Bus      == GOPHANDOFF_TARGET_PCI_BUS) &&
      (Device   == GOPHANDOFF_TARGET_PCI_DEVICE) &&
      (Function == GOPHANDOFF_TARGET_PCI_FUNCTION)) {
    return TRUE;
  }
  
  /*
   * Опционально можно разрешить выбор по Device ID,
   * если вдруг платформа показывает iGPU не как 00:02.0.
   *
   * Для i5-13400 / UHD 730 часто встречается 0xA788,
   * но лучше проверить на вашей машине через lspci.
   */
  // if (DeviceId == 0xA788U) {
  //   return TRUE;
  // }
  
  return FALSE;
}

STATIC
BOOLEAN
IsTargetGopHandle (
                   IN EFI_HANDLE GopHandle
                   )
{
  EFI_STATUS           Status;
  EFI_PCI_IO_PROTOCOL *PciIo;
  
  if (GopHandle == NULL) {
    return FALSE;
  }
  
  PciIo = NULL;
  
  Status = FindPciIoForGopHandle(GopHandle, &PciIo);
  if (EFI_ERROR(Status) || (PciIo == NULL)) {
    return FALSE;
  }
  
  return IsTargetPciIo(PciIo);
}

STATIC
BOOLEAN
IsUsableGop (
             IN CONST EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop
             )
{
  GopFramebufferModeDescriptor Mode;
  
  if ((Gop == NULL) ||
      (Gop->Mode == NULL) ||
      (Gop->Mode->Info == NULL) ||
      (Gop->Mode->Mode >= Gop->Mode->MaxMode) ||
      (Gop->Mode->FrameBufferBase == 0U) ||
      (Gop->Mode->FrameBufferSize == 0U)) {
    return FALSE;
  }
  
  return !EFI_ERROR(BuildModeDescriptor(
                                        Gop->Mode->Mode,
                                        Gop->Mode->Info,
                                        &Mode));
}

STATIC
EFI_STATUS
SelectGop (
           OUT EFI_HANDLE                    *SelectedHandle,
           OUT EFI_GRAPHICS_OUTPUT_PROTOCOL **SelectedGop,
           OUT BOOLEAN                       *IsConsoleOut
           )
{
  EFI_STATUS                    Status;
  EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop;
  EFI_HANDLE                   *Handles;
  EFI_HANDLE                    GopHandle;
  UINTN                         HandleCount;
  UINTN                         Index;
  UINTN                         UsableCount;
  
  if ((SelectedHandle == NULL) ||
      (SelectedGop == NULL) ||
      (IsConsoleOut == NULL)) {
    return EFI_INVALID_PARAMETER;
  }
  
  *SelectedHandle = NULL;
  *SelectedGop = NULL;
  *IsConsoleOut = FALSE;
  
  Gop = NULL;
  Status = gBS->HandleProtocol(
                               gST->ConsoleOutHandle,
                               &gEfiGraphicsOutputProtocolGuid,
                               (VOID **)&Gop
                               );
  if (!EFI_ERROR(Status) &&
      IsUsableGop(Gop) &&
      IsTargetGopHandle(gST->ConsoleOutHandle)) {
    *SelectedHandle = gST->ConsoleOutHandle;
    *SelectedGop = Gop;
    *IsConsoleOut = TRUE;
    return EFI_SUCCESS;
  }
  
  DEBUG((
         DEBUG_WARN,
         "GopHandoff: ConsoleOut GOP is not the target iGPU or is unusable\n"
         ));
  
  Handles = NULL;
  HandleCount = 0U;
  Status = gBS->LocateHandleBuffer(
                                   ByProtocol,
                                   &gEfiGraphicsOutputProtocolGuid,
                                   NULL,
                                   &HandleCount,
                                   &Handles
                                   );
  if (EFI_ERROR(Status)) {
    return Status;
  }
  
  Gop = NULL;
  GopHandle = NULL;
  UsableCount = 0U;
  for (Index = 0U; Index < HandleCount; ++Index) {
    EFI_GRAPHICS_OUTPUT_PROTOCOL *Candidate;
    
    Candidate = NULL;
    Status = gBS->HandleProtocol(
                                 Handles[Index],
                                 &gEfiGraphicsOutputProtocolGuid,
                                 (VOID **)&Candidate
                                 );
    if (!EFI_ERROR(Status) && IsUsableGop(Candidate)) {
      if (!IsTargetGopHandle(Handles[Index])) {
        DEBUG((
               DEBUG_INFO,
               "GopHandoff: skipping non-target GOP handle %p\n",
               Handles[Index]
               ));
        continue;
      }
      
      if (Gop == NULL) {
        Gop = Candidate;
        GopHandle = Handles[Index];
      }
      
      ++UsableCount;
    }
  }
  
  FreePool(Handles);
  
  if (UsableCount == 0U) {
    DEBUG((
           DEBUG_ERROR,
           "GopHandoff: no usable target iGPU GOP found\n"
           ));
    return EFI_NOT_FOUND;
  }
  
  if (UsableCount > 1U) {
    DEBUG((
           DEBUG_WARN,
           "GopHandoff: found %Lu usable target GOP instances; using first\n",
           (UINT64)UsableCount
           ));
  }
  
  *SelectedHandle = GopHandle;
  *SelectedGop = Gop;
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
ReadModeRequest (
                 OUT GopFramebufferModeRequest *Request,
                 OUT BOOLEAN                   *Present
                 )
{
  EFI_STATUS                      Status;
  UINTN                           Size;
  GopFramebufferModeRequestStatus ValidationStatus;
  
  if ((Request == NULL) || (Present == NULL)) {
    return EFI_INVALID_PARAMETER;
  }
  
  ZeroMem(Request, sizeof(*Request));
  *Present = FALSE;
  Size = 0U;
  Status = gRT->GetVariable(
                            mModeRequestVariableName,
                            &mGopFramebufferVariableGuid,
                            NULL,
                            &Size,
                            NULL
                            );
  if (Status == EFI_NOT_FOUND) {
    return EFI_SUCCESS;
  }
  if (Status != EFI_BUFFER_TOO_SMALL) {
    return EFI_ERROR(Status) ? Status : EFI_COMPROMISED_DATA;
  }
  if (Size != sizeof(*Request)) {
    DEBUG((
           DEBUG_ERROR,
           "GopHandoff: mode request has unexpected size %Lu (expected %Lu)\n",
           (UINT64)Size,
           (UINT64)sizeof(*Request)
           ));
    return EFI_COMPROMISED_DATA;
  }
  
  Status = gRT->GetVariable(
                            mModeRequestVariableName,
                            &mGopFramebufferVariableGuid,
                            NULL,
                            &Size,
                            Request
                            );
  if (EFI_ERROR(Status) || (Size != sizeof(*Request))) {
    return EFI_ERROR(Status) ? Status : EFI_COMPROMISED_DATA;
  }
  
  ValidationStatus = gopfb_validate_mode_request(Request);
  if (ValidationStatus != GOPFB_MODE_REQUEST_STATUS_OK) {
    DEBUG((
           DEBUG_ERROR,
           "GopHandoff: invalid mode request: %a\n",
           gopfb_mode_request_status_string(ValidationStatus)
           ));
    return EFI_COMPROMISED_DATA;
  }
  
  *Present = TRUE;
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
QueryModeDescriptor (
                     IN  EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop,
                     IN  UINT32                        ModeNumber,
                     OUT GopFramebufferModeDescriptor *Mode
                     )
{
  EFI_STATUS                            Status;
  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *ModeInfo;
  UINTN                                 ModeInfoSize;
  
  if ((Gop == NULL) || (Mode == NULL)) {
    return EFI_INVALID_PARAMETER;
  }
  
  ModeInfo = NULL;
  ModeInfoSize = 0U;
  Status = Gop->QueryMode(Gop, ModeNumber, &ModeInfoSize, &ModeInfo);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  if ((ModeInfo == NULL) ||
      (ModeInfoSize < sizeof(EFI_GRAPHICS_OUTPUT_MODE_INFORMATION))) {
    if (ModeInfo != NULL) {
      FreePool(ModeInfo);
    }
    return EFI_COMPROMISED_DATA;
  }
  
  Status = BuildModeDescriptor(ModeNumber, ModeInfo, Mode);
  FreePool(ModeInfo);
  return Status;
}

STATIC
BOOLEAN
ModeMatchesRequest (
                    IN CONST GopFramebufferModeDescriptor *Mode,
                    IN CONST GopFramebufferModeRequest    *Request
                    )
{
  if ((Mode == NULL) || (Request == NULL)) {
    return FALSE;
  }
  
  if (((Request->flags & GOPFB_MODE_REQUEST_BY_NUMBER) != 0U) &&
      (Mode->modeNumber != Request->modeNumber)) {
    return FALSE;
  }
  
  if (((Request->flags & GOPFB_MODE_REQUEST_BY_DIMENSIONS) != 0U) &&
      ((Mode->width != Request->width) ||
       (Mode->height != Request->height))) {
    return FALSE;
  }
  
  return TRUE;
}

STATIC
EFI_STATUS
ResolveRequestedMode (
                      IN  EFI_GRAPHICS_OUTPUT_PROTOCOL     *Gop,
                      IN  CONST GopFramebufferModeRequest *Request,
                      OUT UINT32                           *TargetMode
                      )
{
  EFI_STATUS Status;
  UINT32     ModeNumber;
  UINT32     MatchCount;
  UINT32     MatchedMode;
  
  if ((Gop == NULL) ||
      (Gop->Mode == NULL) ||
      (Request == NULL) ||
      (TargetMode == NULL)) {
    return EFI_INVALID_PARAMETER;
  }
  
  MatchCount = 0U;
  MatchedMode = 0U;
  for (ModeNumber = 0U; ModeNumber < Gop->Mode->MaxMode; ++ModeNumber) {
    GopFramebufferModeDescriptor Mode;
    
    Status = QueryModeDescriptor(Gop, ModeNumber, &Mode);
    if (EFI_ERROR(Status)) {
      continue;
    }
    
    if (ModeMatchesRequest(&Mode, Request)) {
      MatchedMode = ModeNumber;
      ++MatchCount;
    }
  }
  
  if (MatchCount == 0U) {
    return EFI_NOT_FOUND;
  }
  if (MatchCount != 1U) {
    DEBUG((
           DEBUG_ERROR,
           "GopHandoff: mode request matched %u GOP modes; use mode number to disambiguate\n",
           MatchCount
           ));
    return EFI_ABORTED;
  }
  
  *TargetMode = MatchedMode;
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
ApplyGopMode (
              IN  EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop,
              OUT BOOLEAN                      *RequestApplied
              )
{
  EFI_STATUS                    Status;
  GopFramebufferModeRequest     Request;
  BOOLEAN                       RequestPresent;
  UINT32                        TargetMode;
  GopFramebufferModeDescriptor  Mode;
  
  if ((Gop == NULL) ||
      (Gop->Mode == NULL) ||
      (RequestApplied == NULL)) {
    return EFI_INVALID_PARAMETER;
  }
  
  *RequestApplied = FALSE;
  TargetMode = Gop->Mode->Mode;
  RequestPresent = FALSE;
  Status = ReadModeRequest(&Request, &RequestPresent);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  
  if (RequestPresent) {
    Status = ResolveRequestedMode(Gop, &Request, &TargetMode);
    if (EFI_ERROR(Status)) {
      return Status;
    }
    *RequestApplied = TRUE;
  }
  
  Status = QueryModeDescriptor(Gop, TargetMode, &Mode);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  
  Status = Gop->SetMode(Gop, TargetMode);
  if (EFI_ERROR(Status)) {
    DEBUG((
           DEBUG_ERROR,
           "GopHandoff: GOP SetMode(%u) failed: %r\n",
           TargetMode,
           Status
           ));
    return Status;
  }
  
  if (!IsUsableGop(Gop) || (Gop->Mode->Mode != TargetMode)) {
    return EFI_DEVICE_ERROR;
  }
  
  DEBUG((
         DEBUG_INFO,
         "GopHandoff: firmware GOP programmed mode %u (%ux%u), request=%a\n",
         TargetMode,
         Mode.width,
         Mode.height,
         RequestPresent ? "yes" : "no"
         ));
  return EFI_SUCCESS;
}

STATIC
VOID
SelectEdid (
            IN  EFI_HANDLE       GopHandle,
            OUT CONST UINT8    **Edid,
            OUT UINT32          *EdidSize,
            OUT UINT32          *DisplayFlags
            )
{
  EFI_STATUS                    Status;
  EFI_EDID_ACTIVE_PROTOCOL     *Active;
  EFI_EDID_DISCOVERED_PROTOCOL *Discovered;
  GopFramebufferEdidStatus      ValidationStatus;
  
  if ((Edid == NULL) || (EdidSize == NULL) || (DisplayFlags == NULL)) {
    return;
  }
  
  *Edid = NULL;
  *EdidSize = 0U;
  *DisplayFlags = 0U;
  if (GopHandle == NULL) {
    return;
  }
  
  Active = NULL;
  Status = gBS->HandleProtocol(
                               GopHandle,
                               &gEfiEdidActiveProtocolGuid,
                               (VOID **)&Active
                               );
  if (!EFI_ERROR(Status) &&
      (Active != NULL) &&
      (Active->Edid != NULL) &&
      (Active->SizeOfEdid != 0U)) {
    ValidationStatus = gopfb_validate_edid(
                                           Active->Edid,
                                           (GopFbSize)Active->SizeOfEdid);
    if (ValidationStatus == GOPFB_EDID_STATUS_OK) {
      *Edid = Active->Edid;
      *EdidSize = Active->SizeOfEdid;
      *DisplayFlags = GOPFB_DISPLAY_FLAG_EDID_ACTIVE;
      return;
    }
    DEBUG((
           DEBUG_ERROR,
           "GopHandoff: active EDID rejected: %a\n",
           gopfb_edid_status_string(ValidationStatus)
           ));
  }
  
  Discovered = NULL;
  Status = gBS->HandleProtocol(
                               GopHandle,
                               &gEfiEdidDiscoveredProtocolGuid,
                               (VOID **)&Discovered
                               );
  if (!EFI_ERROR(Status) &&
      (Discovered != NULL) &&
      (Discovered->Edid != NULL) &&
      (Discovered->SizeOfEdid != 0U)) {
    ValidationStatus = gopfb_validate_edid(
                                           Discovered->Edid,
                                           (GopFbSize)Discovered->SizeOfEdid);
    if (ValidationStatus == GOPFB_EDID_STATUS_OK) {
      *Edid = Discovered->Edid;
      *EdidSize = Discovered->SizeOfEdid;
      *DisplayFlags = GOPFB_DISPLAY_FLAG_EDID_DISCOVERED;
      return;
    }
    DEBUG((
           DEBUG_ERROR,
           "GopHandoff: discovered EDID rejected: %a\n",
           gopfb_edid_status_string(ValidationStatus)
           ));
  }
}

STATIC
EFI_STATUS
CheckVariableCapacity (
                       IN UINT32 Attributes,
                       IN UINTN  LargestVariableSize,
                       IN UINTN  TotalPayloadSize
                       )
{
  EFI_STATUS Status;
  UINT64     MaximumVariableStorageSize;
  UINT64     RemainingVariableStorageSize;
  UINT64     MaximumVariableSize;
  
  if ((LargestVariableSize == 0U) || (TotalPayloadSize == 0U)) {
    return EFI_INVALID_PARAMETER;
  }
  
  MaximumVariableStorageSize = 0U;
  RemainingVariableStorageSize = 0U;
  MaximumVariableSize = 0U;
  Status = gRT->QueryVariableInfo(
                                  Attributes,
                                  &MaximumVariableStorageSize,
                                  &RemainingVariableStorageSize,
                                  &MaximumVariableSize
                                  );
  if (Status == EFI_UNSUPPORTED) {
    return EFI_SUCCESS;
  }
  if (EFI_ERROR(Status)) {
    return Status;
  }
  
  if (((UINT64)LargestVariableSize > MaximumVariableSize) ||
      ((UINT64)TotalPayloadSize > RemainingVariableStorageSize)) {
    return EFI_OUT_OF_RESOURCES;
  }
  
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
VerifyVariablePayload (
                       IN CHAR16     *VariableName,
                       IN UINT32      ExpectedAttributes,
                       IN CONST VOID *ExpectedData,
                       IN UINTN       ExpectedSize
                       )
{
  EFI_STATUS Status;
  UINT32     Attributes;
  UINTN      Size;
  VOID      *Buffer;
  
  if ((VariableName == NULL) ||
      (ExpectedData == NULL) ||
      (ExpectedSize == 0U)) {
    return EFI_INVALID_PARAMETER;
  }
  
  Buffer = AllocatePool(ExpectedSize);
  if (Buffer == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  
  Attributes = 0U;
  Size = ExpectedSize;
  Status = gRT->GetVariable(
                            VariableName,
                            &mGopFramebufferVariableGuid,
                            &Attributes,
                            &Size,
                            Buffer
                            );
  if (!EFI_ERROR(Status) &&
      ((Size != ExpectedSize) ||
       (Attributes != ExpectedAttributes) ||
       (CompareMem(Buffer, ExpectedData, ExpectedSize) != 0))) {
    Status = EFI_COMPROMISED_DATA;
  }
  
  FreePool(Buffer);
  return Status;
}

STATIC
EFI_STATUS
FindPciIoForGopHandle (
                       IN  EFI_HANDLE           GopHandle,
                       OUT EFI_PCI_IO_PROTOCOL **PciIo
                       )
{
  EFI_STATUS                Status;
  EFI_DEVICE_PATH_PROTOCOL *GopPath;
  EFI_HANDLE               *Handles;
  UINTN                     HandleCount;
  UINTN                     GopPathSize;
  UINTN                     GopPrefixSize;
  UINTN                     BestPrefixSize;
  UINTN                     Index;
  EFI_PCI_IO_PROTOCOL      *BestPciIo;
  BOOLEAN                   Ambiguous;
  
  if ((GopHandle == NULL) || (PciIo == NULL)) {
    return EFI_INVALID_PARAMETER;
  }
  
  *PciIo = NULL;
  Status = gBS->HandleProtocol(
                               GopHandle,
                               &gEfiPciIoProtocolGuid,
                               (VOID **)PciIo
                               );
  if (!EFI_ERROR(Status) && (*PciIo != NULL)) {
    return EFI_SUCCESS;
  }
  
  GopPath = DevicePathFromHandle(GopHandle);
  if (GopPath == NULL) {
    return EFI_NOT_FOUND;
  }
  GopPathSize = GetDevicePathSize(GopPath);
  if (GopPathSize <= END_DEVICE_PATH_LENGTH) {
    return EFI_NOT_FOUND;
  }
  GopPrefixSize = GopPathSize - END_DEVICE_PATH_LENGTH;
  
  Handles = NULL;
  HandleCount = 0U;
  Status = gBS->LocateHandleBuffer(
                                   ByProtocol,
                                   &gEfiPciIoProtocolGuid,
                                   NULL,
                                   &HandleCount,
                                   &Handles
                                   );
  if (EFI_ERROR(Status)) {
    return Status;
  }
  
  BestPciIo = NULL;
  BestPrefixSize = 0U;
  Ambiguous = FALSE;
  for (Index = 0U; Index < HandleCount; ++Index) {
    EFI_DEVICE_PATH_PROTOCOL *PciPath;
    EFI_PCI_IO_PROTOCOL      *Candidate;
    UINTN                     PciPathSize;
    UINTN                     PrefixSize;
    
    PciPath = DevicePathFromHandle(Handles[Index]);
    if (PciPath == NULL) {
      continue;
    }
    PciPathSize = GetDevicePathSize(PciPath);
    if (PciPathSize <= END_DEVICE_PATH_LENGTH) {
      continue;
    }
    PrefixSize = PciPathSize - END_DEVICE_PATH_LENGTH;
    if ((PrefixSize > GopPrefixSize) ||
        (CompareMem(PciPath, GopPath, PrefixSize) != 0)) {
      continue;
    }
    
    Candidate = NULL;
    Status = gBS->HandleProtocol(
                                 Handles[Index],
                                 &gEfiPciIoProtocolGuid,
                                 (VOID **)&Candidate
                                 );
    if (EFI_ERROR(Status) || (Candidate == NULL)) {
      continue;
    }
    
    if (PrefixSize > BestPrefixSize) {
      BestPrefixSize = PrefixSize;
      BestPciIo = Candidate;
      Ambiguous = FALSE;
    } else if ((PrefixSize == BestPrefixSize) &&
               (BestPciIo != NULL) &&
               (Candidate != BestPciIo)) {
      Ambiguous = TRUE;
    }
  }
  
  if (Handles != NULL) {
    FreePool(Handles);
  }
  if ((BestPciIo == NULL) || Ambiguous) {
    return Ambiguous ? EFI_ABORTED : EFI_NOT_FOUND;
  }
  
  *PciIo = BestPciIo;
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
BuildPciInfo (
              IN  EFI_HANDLE                      GopHandle,
              IN  CONST GopFramebufferHandoff    *Handoff,
              OUT GopFramebufferPciInfo          *PciInfo
              )
{
  EFI_STATUS                  Status;
  EFI_PCI_IO_PROTOCOL        *PciIo;
  UINTN                       Segment;
  UINTN                       Bus;
  UINTN                       Device;
  UINTN                       Function;
  UINT16                      VendorId;
  UINT16                      DeviceId;
  UINT32                      RevisionClass;
  UINT32                      BarIndex;
  GopFbUint64                 RequiredBytes;
  GopFramebufferPciInfoStatus ValidationStatus;
  
  if ((GopHandle == NULL) || (Handoff == NULL) || (PciInfo == NULL)) {
    return EFI_INVALID_PARAMETER;
  }
  
  gopfb_initialize_pci_info(PciInfo);
  PciIo = NULL;
  Status = FindPciIoForGopHandle(GopHandle, &PciIo);
  if (EFI_ERROR(Status) || (PciIo == NULL)) {
    return EFI_NOT_FOUND;
  }
  
  Segment = 0U;
  Bus = 0U;
  Device = 0U;
  Function = 0U;
  Status = PciIo->GetLocation(PciIo, &Segment, &Bus, &Device, &Function);
  if (EFI_ERROR(Status) ||
      (Segment > 0xFFFFU) ||
      (Bus > 0xFFU) ||
      (Device > 0x1FU) ||
      (Function > 0x07U)) {
    return EFI_DEVICE_ERROR;
  }
  PciInfo->segment = (UINT32)Segment;
  PciInfo->bus = (UINT32)Bus;
  PciInfo->device = (UINT32)Device;
  PciInfo->function = (UINT32)Function;
  PciInfo->flags |= GOPFB_PCI_FLAG_LOCATION_VALID;
  
  VendorId = 0U;
  DeviceId = 0U;
  if (!EFI_ERROR(PciIo->Pci.Read(
                                 PciIo,
                                 EfiPciIoWidthUint16,
                                 0x00U,
                                 1U,
                                 &VendorId)) &&
      !EFI_ERROR(PciIo->Pci.Read(
                                 PciIo,
                                 EfiPciIoWidthUint16,
                                 0x02U,
                                 1U,
                                 &DeviceId)) &&
      (VendorId != 0U) &&
      (VendorId != 0xFFFFU) &&
      (DeviceId != 0xFFFFU)) {
    PciInfo->vendorId = (UINT32)VendorId;
    PciInfo->deviceId = (UINT32)DeviceId;
    PciInfo->flags |= GOPFB_PCI_FLAG_IDENTITY_VALID;
  }
  
  RevisionClass = 0U;
  if (!EFI_ERROR(PciIo->Pci.Read(
                                 PciIo,
                                 EfiPciIoWidthUint32,
                                 0x08U,
                                 1U,
                                 &RevisionClass))) {
                                   PciInfo->classCode = (RevisionClass >> 8U) & 0x00FFFFFFU;
                                   if (PciInfo->classCode != 0U) {
                                     PciInfo->flags |= GOPFB_PCI_FLAG_CLASS_VALID;
                                   }
                                 }
  
  RequiredBytes = 0U;
  if (gopfb_validate(Handoff, &RequiredBytes) != GOPFB_STATUS_OK) {
    return EFI_COMPROMISED_DATA;
  }
  
  for (BarIndex = 0U; BarIndex <= 5U; ++BarIndex) {
    VOID                              *Resources;
    EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *Descriptor;
    UINT64                             Base;
    UINT64                             Length;
    UINT64                             Offset;
    
    Resources = NULL;
    Status = PciIo->GetBarAttributes(
                                     PciIo,
                                     (UINT8)BarIndex,
                                     NULL,
                                     &Resources
                                     );
    if (EFI_ERROR(Status) || (Resources == NULL)) {
      continue;
    }
    
    Descriptor = (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *)Resources;
    if ((Descriptor->Desc == ACPI_ADDRESS_SPACE_DESCRIPTOR) &&
        (Descriptor->ResType == ACPI_ADDRESS_SPACE_TYPE_MEM)) {
      Base = Descriptor->AddrRangeMin;
      Length = Descriptor->AddrLen;
      if ((Length != 0U) &&
          (Handoff->framebufferBase >= Base)) {
        Offset = Handoff->framebufferBase - Base;
        if ((Offset < Length) &&
            (RequiredBytes <= (Length - Offset))) {
          PciInfo->barIndex = BarIndex;
          PciInfo->barBase = Base;
          PciInfo->barSize = Length;
          PciInfo->framebufferOffset = Offset;
          PciInfo->flags |= GOPFB_PCI_FLAG_BAR_VALID;
        }
      }
    }
    FreePool(Resources);
    
    if ((PciInfo->flags & GOPFB_PCI_FLAG_BAR_VALID) != 0U) {
      break;
    }
  }
  
  gopfb_finalize_pci_info(PciInfo);
  ValidationStatus = gopfb_validate_pci_info(PciInfo);
  if (ValidationStatus != GOPFB_PCI_STATUS_OK) {
    DEBUG((
           DEBUG_ERROR,
           "GopHandoff: generated PCI info rejected: %a\n",
           gopfb_pci_status_string(ValidationStatus)
           ));
    return EFI_COMPROMISED_DATA;
  }
  
  return EFI_SUCCESS;
}

STATIC
VOID
PublishOptionalPciInfo (
                        IN EFI_HANDLE                   GopHandle,
                        IN CONST GopFramebufferHandoff *Handoff,
                        IN UINT32                       Attributes
                        )
{
  EFI_STATUS             Status;
  GopFramebufferPciInfo PciInfo;
  
  Status = BuildPciInfo(GopHandle, Handoff, &PciInfo);
  if (EFI_ERROR(Status)) {
    DEBUG((
           DEBUG_WARN,
           "GopHandoff: optional PCI identity unavailable: %r; primary framebuffer handoff remains valid\n",
           Status
           ));
    return;
  }
  
  Status = CheckVariableCapacity(
                                 Attributes,
                                 sizeof(PciInfo),
                                 sizeof(PciInfo)
                                 );
  if (EFI_ERROR(Status)) {
    DEBUG((
           DEBUG_WARN,
           "GopHandoff: optional PCI info variable capacity unavailable: %r; primary framebuffer handoff remains valid\n",
           Status
           ));
    return;
  }
  
  Status = gRT->SetVariable(
                            mPciInfoVariableName,
                            &mGopFramebufferVariableGuid,
                            Attributes,
                            sizeof(PciInfo),
                            &PciInfo
                            );
  if (!EFI_ERROR(Status)) {
    Status = VerifyVariablePayload(
                                   mPciInfoVariableName,
                                   Attributes,
                                   &PciInfo,
                                   sizeof(PciInfo)
                                   );
  }
  if (EFI_ERROR(Status)) {
    EFI_STATUS DeleteStatus;
    
    DeleteStatus = DeleteVariable(mPciInfoVariableName);
    DBG(           "GopHandoff: optional PCI info publication failed: %llx; cleanup=%llx; primary framebuffer handoff remains valid\n", Status,DeleteStatus);
    DEBUG((
           DEBUG_WARN,
           "GopHandoff: optional PCI info publication failed: %r; cleanup=%r; primary framebuffer handoff remains valid\n",
           Status,
           DeleteStatus
           ));
    return;
  }
  
  DEBUG((
         DEBUG_INFO,
         "GopHandoff: PCI %04x:%02x:%02x.%x vendor=%04x device=%04x class=%06x BAR=%a base=%Lx size=%Lx offset=%Lx\n",
         PciInfo.segment,
         PciInfo.bus,
         PciInfo.device,
         PciInfo.function,
         PciInfo.vendorId,
         PciInfo.deviceId,
         PciInfo.classCode,
         (PciInfo.flags & GOPFB_PCI_FLAG_BAR_VALID) != 0U ? "matched" : "not-found",
         PciInfo.barBase,
         PciInfo.barSize,
         PciInfo.framebufferOffset
         ));
}

STATIC
EFI_STATUS
BuildDisplayInfo (
                  IN  EFI_HANDLE                    GopHandle,
                  IN  EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop,
                  OUT VOID                         **DisplayInfo,
                  OUT UINTN                         *DisplayInfoSize
                  )
{
  EFI_STATUS                            Status;
  GopFramebufferModeDescriptor         *ModeStorage;
  UINTN                                 ModeStorageSize;
  UINT32                                ModeNumber;
  UINT32                                ModeCount;
  UINT32                                CurrentModeArrayIndex;
  CONST UINT8                          *Edid;
  UINT32                                EdidSize;
  UINT32                                DisplayFlags;
  UINTN                                 ModeBytes;
  UINTN                                 TotalSize;
  GopFramebufferDisplayInfoHeader      *Header;
  GopFramebufferModeDescriptor         *OutputModes;
  GopFramebufferDisplayInfoStatus       ValidationStatus;
  CONST GopFramebufferModeDescriptor   *ValidatedModes;
  CONST GopFbUint8                     *ValidatedEdid;
  
  if ((Gop == NULL) ||
      (Gop->Mode == NULL) ||
      (DisplayInfo == NULL) ||
      (DisplayInfoSize == NULL)) {
    return EFI_INVALID_PARAMETER;
  }
  
  *DisplayInfo = NULL;
  *DisplayInfoSize = 0U;
  
  if ((Gop->Mode->MaxMode == 0U) ||
      (Gop->Mode->MaxMode > GOPFB_MODE_CATALOG_MAX_COUNT)) {
    DEBUG((
           DEBUG_ERROR,
           "GopHandoff: GOP MaxMode %u exceeds supported catalog limit %u\n",
           Gop->Mode->MaxMode,
           GOPFB_MODE_CATALOG_MAX_COUNT
           ));
    return EFI_BAD_BUFFER_SIZE;
  }
  
  if (!gopfb_size_multiply(
                           (GopFbSize)Gop->Mode->MaxMode,
                           sizeof(GopFramebufferModeDescriptor),
                           &ModeStorageSize)) {
                             return EFI_BAD_BUFFER_SIZE;
                           }
  
  ModeStorage = AllocateZeroPool(ModeStorageSize);
  if ((ModeStorage == NULL) && (ModeStorageSize != 0U)) {
    return EFI_OUT_OF_RESOURCES;
  }
  
  ModeCount = 0U;
  CurrentModeArrayIndex = GOPFB_INVALID_MODE_INDEX;
  for (ModeNumber = 0U; ModeNumber < Gop->Mode->MaxMode; ++ModeNumber) {
    GopFramebufferModeDescriptor Mode;
    
    Status = QueryModeDescriptor(Gop, ModeNumber, &Mode);
    if (EFI_ERROR(Status)) {
      DEBUG((
             DEBUG_WARN,
             "GopHandoff: skipping unusable GOP mode %u: %r\n",
             ModeNumber,
             Status
             ));
      continue;
    }
    
    ModeStorage[ModeCount] = Mode;
    if (ModeNumber == Gop->Mode->Mode) {
      CurrentModeArrayIndex = ModeCount;
    }
    ++ModeCount;
  }
  
  if ((ModeCount == 0U) ||
      (CurrentModeArrayIndex == GOPFB_INVALID_MODE_INDEX)) {
    FreePool(ModeStorage);
    return EFI_NOT_FOUND;
  }
  
  Edid = NULL;
  EdidSize = 0U;
  DisplayFlags = 0U;
  SelectEdid(GopHandle, &Edid, &EdidSize, &DisplayFlags);
  
  if (!gopfb_size_multiply(
                           (GopFbSize)ModeCount,
                           sizeof(GopFramebufferModeDescriptor),
                           &ModeBytes) ||
      !gopfb_size_add(
                      sizeof(GopFramebufferDisplayInfoHeader),
                      ModeBytes,
                      &TotalSize) ||
      !gopfb_size_add(TotalSize, (GopFbSize)EdidSize, &TotalSize) ||
      (TotalSize > MAX_UINT32)) {
    FreePool(ModeStorage);
    return EFI_BAD_BUFFER_SIZE;
  }
  
  Header = AllocateZeroPool(TotalSize);
  if (Header == NULL) {
    FreePool(ModeStorage);
    return EFI_OUT_OF_RESOURCES;
  }
  
  gopfb_initialize_display_info(Header, (UINT32)TotalSize);
  Header->flags = DisplayFlags;
  Header->modeCount = ModeCount;
  Header->currentModeArrayIndex = CurrentModeArrayIndex;
  Header->edidOffset = (UINT32)(sizeof(*Header) + ModeBytes);
  Header->edidSize = EdidSize;
  
  OutputModes = (GopFramebufferModeDescriptor *)(VOID *)(Header + 1);
  CopyMem(OutputModes, ModeStorage, ModeBytes);
  if (EdidSize != 0U) {
    CopyMem((UINT8 *)Header + Header->edidOffset, Edid, EdidSize);
  }
  FreePool(ModeStorage);
  
  gopfb_finalize_display_info(Header, TotalSize);
  ValidatedModes = NULL;
  ValidatedEdid = NULL;
  ValidationStatus = gopfb_validate_display_info(
                                                 Header,
                                                 TotalSize,
                                                 &ValidatedModes,
                                                 &ValidatedEdid);
  if (ValidationStatus != GOPFB_DISPLAY_STATUS_OK) {
    DEBUG((
           DEBUG_ERROR,
           "GopHandoff: generated display info rejected: %a\n",
           gopfb_display_status_string(ValidationStatus)
           ));
    FreePool(Header);
    return EFI_COMPROMISED_DATA;
  }
  
  *DisplayInfo = Header;
  *DisplayInfoSize = TotalSize;
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
BuildHandoff (
              IN  EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop,
              IN  BOOLEAN                       IsConsoleOut,
              IN  BOOLEAN                       RequestApplied,
              IN  BOOLEAN                       ReadyToBootCapture,
              OUT GopFramebufferHandoff        *Handoff
              )
{
  EFI_STATUS                     Status;
  GopFramebufferModeDescriptor  Mode;
  GopFramebufferStatus          ValidationStatus;
  UINT64                        RequiredBytes;
  
  if ((Gop == NULL) ||
      (Gop->Mode == NULL) ||
      (Gop->Mode->Info == NULL) ||
      (Handoff == NULL)) {
    return EFI_INVALID_PARAMETER;
  }
  
  Status = BuildModeDescriptor(Gop->Mode->Mode, Gop->Mode->Info, &Mode);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  
  gopfb_initialize(Handoff);
  Handoff->flags = GOPFB_FLAG_MODE_REAPPLIED |
  (ReadyToBootCapture
   ? GOPFB_FLAG_READY_TO_BOOT
   : GOPFB_FLAG_EARLY_CAPTURE);
  if (IsConsoleOut) {
    Handoff->flags |= GOPFB_FLAG_CONSOLE_OUT;
  }
  if (RequestApplied) {
    Handoff->flags |= GOPFB_FLAG_MODE_REQUEST_APPLIED;
  }
  
  Handoff->framebufferBase = (UINT64)Gop->Mode->FrameBufferBase;
  Handoff->framebufferSize = (UINT64)Gop->Mode->FrameBufferSize;
  Handoff->width = Mode.width;
  Handoff->height = Mode.height;
  Handoff->pixelsPerScanLine = Mode.pixelsPerScanLine;
  Handoff->pixelFormat = Mode.pixelFormat;
  Handoff->bytesPerPixel = Mode.bytesPerPixel;
  Handoff->redMask = Mode.redMask;
  Handoff->greenMask = Mode.greenMask;
  Handoff->blueMask = Mode.blueMask;
  Handoff->reservedMask = Mode.reservedMask;
  
  gopfb_finalize(Handoff);
  RequiredBytes = 0U;
  ValidationStatus = gopfb_validate(Handoff, &RequiredBytes);
  if (ValidationStatus != GOPFB_STATUS_OK) {
    DEBUG((
           DEBUG_ERROR,
           "GopHandoff: generated handoff rejected (%a), required=%Lu, available=%Lu\n",
           gopfb_status_string(ValidationStatus),
           RequiredBytes,
           Handoff->framebufferSize
           ));
    return EFI_COMPROMISED_DATA;
  }
  
  return EFI_SUCCESS;
}

STATIC
VOID
PublishOptionalDisplayInfo (
                            IN EFI_HANDLE                    GopHandle,
                            IN EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop,
                            IN UINT32                        Attributes
                            )
{
  EFI_STATUS Status;
  VOID      *DisplayInfo;
  UINTN      DisplayInfoSize;
  
  if ((GopHandle == NULL) || (Gop == NULL)) {
    DEBUG((
           DEBUG_ERROR,
           "GopHandoff: optional display-info skipped because the GOP context is invalid\n"
           ));
    return;
  }
  
  DisplayInfo = NULL;
  DisplayInfoSize = 0U;
  Status = BuildDisplayInfo(
                            GopHandle,
                            Gop,
                            &DisplayInfo,
                            &DisplayInfoSize
                            );
  if (EFI_ERROR(Status)) {
    DEBUG((
           DEBUG_ERROR,
           "GopHandoff: optional display-info capture unavailable: %r; primary framebuffer handoff remains valid\n",
           Status
           ));
    return;
  }
  
  Status = CheckVariableCapacity(
                                 Attributes,
                                 DisplayInfoSize,
                                 DisplayInfoSize
                                 );
  if (EFI_ERROR(Status)) {
    DEBUG((
           DEBUG_ERROR,
           "GopHandoff: optional display-info variable capacity unavailable: %r; primary framebuffer handoff remains valid\n",
           Status
           ));
    FreePool(DisplayInfo);
    return;
  }
  
  Status = gRT->SetVariable(
                            mDisplayInfoVariableName,
                            &mGopFramebufferVariableGuid,
                            Attributes,
                            DisplayInfoSize,
                            DisplayInfo
                            );
  if (!EFI_ERROR(Status)) {
    Status = VerifyVariablePayload(
                                   mDisplayInfoVariableName,
                                   Attributes,
                                   DisplayInfo,
                                   DisplayInfoSize
                                   );
  }
  FreePool(DisplayInfo);
  
  if (EFI_ERROR(Status)) {
    EFI_STATUS DeleteStatus;
    
    DeleteStatus = DeleteVariable(mDisplayInfoVariableName);
    DBG("GopHandoff: optional display-info publication failed: %llx; cleanup=%llx; primary framebuffer handoff remains valid\n",
        Status,
        DeleteStatus);
    DEBUG((
           DEBUG_ERROR,
           "GopHandoff: optional display-info publication failed: %r; cleanup=%r; primary framebuffer handoff remains valid\n",
           Status,
           DeleteStatus
           ));
    return;
  }
  
  DEBUG((
         DEBUG_INFO,
         "GopHandoff: optional display-info published and verified (%Lu bytes)\n",
         (UINT64)DisplayInfoSize
         ));
}

STATIC
BOOLEAN
ReadExistingPrimaryHandoff (
                            OUT GopFramebufferHandoff *Handoff,
                            OUT UINT32                *Attributes
                            )
{
  EFI_STATUS            Status;
  GopFramebufferStatus  ValidationStatus;
  UINTN                 Size;
  
  if ((Handoff == NULL) || (Attributes == NULL)) {
    return FALSE;
  }
  
  ZeroMem(Handoff, sizeof(*Handoff));
  *Attributes = 0U;
  Size = sizeof(*Handoff);
  Status = gRT->GetVariable(
                            mHandoffVariableName,
                            &mGopFramebufferVariableGuid,
                            Attributes,
                            &Size,
                            Handoff
                            );
  if (EFI_ERROR(Status) || (Size != sizeof(*Handoff))) {
    return FALSE;
  }
  
  ValidationStatus = gopfb_validate(Handoff, NULL);
  if (ValidationStatus != GOPFB_STATUS_OK) {
    return FALSE;
  }
  
  return TRUE;
}

STATIC
EFI_STATUS
RestorePrimaryHandoff (
                       IN CONST GopFramebufferHandoff *Handoff,
                       IN UINT32                       Attributes
                       )
{
  EFI_STATUS Status;
  
  if (Handoff == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  
  Status = gRT->SetVariable(
                            mHandoffVariableName,
                            &mGopFramebufferVariableGuid,
                            Attributes,
                            sizeof(*Handoff),
                            (VOID *)Handoff
                            );
  if (EFI_ERROR(Status)) {
    return Status;
  }
  
  return VerifyVariablePayload(
                               mHandoffVariableName,
                               Attributes,
                               Handoff,
                               sizeof(*Handoff)
                               );
}

STATIC
EFI_STATUS
CaptureCurrentGop (
                   IN BOOLEAN ReadyToBootCapture
                   )
{
  EFI_STATUS                    Status;
  EFI_HANDLE                    GopHandle;
  EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop;
  BOOLEAN                       IsConsoleOut;
  BOOLEAN                       RequestApplied;
  GopFramebufferHandoff         Handoff;
  GopFramebufferHandoff         PreviousHandoff;
  UINT32                        Attributes;
  UINT32                        PreviousAttributes;
  BOOLEAN                       PreviousHandoffValid;
  
  GopHandle = NULL;
  Gop = NULL;
  IsConsoleOut = FALSE;
  Status = SelectGop(&GopHandle, &Gop, &IsConsoleOut);
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "GopHandoff: GOP selection failed: %r\n", Status));
    return Status;
  }
  
  RequestApplied = FALSE;
  Status = ApplyGopMode(Gop, &RequestApplied);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  
  Status = BuildHandoff(
                        Gop,
                        IsConsoleOut,
                        RequestApplied,
                        ReadyToBootCapture,
                        &Handoff
                        );
  if (EFI_ERROR(Status)) {
    return Status;
  }
  
  Attributes = EFI_VARIABLE_NON_VOLATILE |
               EFI_VARIABLE_BOOTSERVICE_ACCESS |
               EFI_VARIABLE_RUNTIME_ACCESS;
  Status = CheckVariableCapacity(
                                 Attributes,
                                 sizeof(Handoff),
                                 sizeof(Handoff)
                                 );
  if (EFI_ERROR(Status)) {
    DEBUG((
           DEBUG_ERROR,
           "GopHandoff: insufficient UEFI variable capacity for primary framebuffer handoff: %r\n",
           Status
           ));
    return Status;
  }
  
  PreviousAttributes = 0U;
  PreviousHandoffValid = ReadExistingPrimaryHandoff(
                                                    &PreviousHandoff,
                                                    &PreviousAttributes
                                                    );
  
  Status = gRT->SetVariable(
                            mHandoffVariableName,
                            &mGopFramebufferVariableGuid,
                            Attributes,
                            sizeof(Handoff),
                            &Handoff
                            );
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "GopHandoff: primary handoff SetVariable failed: %r\n", Status));
    return Status;
  }
  
  Status = VerifyVariablePayload(
                                 mHandoffVariableName,
                                 Attributes,
                                 &Handoff,
                                 sizeof(Handoff)
                                 );
  if (EFI_ERROR(Status)) {
    EFI_STATUS RecoveryStatus;
    
    if (PreviousHandoffValid) {
      RecoveryStatus = RestorePrimaryHandoff(
                                             &PreviousHandoff,
                                             PreviousAttributes
                                             );
      DBG("GopHandoff: primary handoff read-back verification failed: %llx; previous verified handoff restore=%llx\n",
          Status,
          RecoveryStatus);
      DEBUG((
             DEBUG_ERROR,
             "GopHandoff: primary handoff read-back verification failed: %r; previous verified handoff restore=%r\n",
             Status,
             RecoveryStatus
             ));
    } else {
      RecoveryStatus = DeleteVariable(mHandoffVariableName);
      DEBUG((
             DEBUG_ERROR,
             "GopHandoff: primary handoff read-back verification failed: %r; no previous verified handoff, cleanup=%r\n",
             Status,
             RecoveryStatus
             ));
    }
    return Status;
  }
  
  DEBUG((
         DEBUG_INFO,
         "GopHandoff: primary FB=%Lx size=%Lu mode=%ux%u stride=%u format=%u modeset=%a phase=%a\n",
         Handoff.framebufferBase,
         Handoff.framebufferSize,
         Handoff.width,
         Handoff.height,
         Handoff.pixelsPerScanLine,
         Handoff.pixelFormat,
         RequestApplied ? "requested" : "current-reapplied",
         ReadyToBootCapture ? "ReadyToBoot" : "EntryPoint"
         ));
  
  PublishOptionalPciInfo(GopHandle, &Handoff, Attributes);
  PublishOptionalDisplayInfo(GopHandle, Gop, Attributes);
  return EFI_SUCCESS;
}

STATIC
VOID
EFIAPI
CaptureAtReadyToBoot (
                      IN EFI_EVENT Event,
                      IN VOID     *Context
                      )
{
  EFI_STATUS Status;
  
  if ((Event == NULL) ||
      (Event != mReadyToBootEvent) ||
      (Context != mImageHandle)) {
    DEBUG((DEBUG_ERROR, "GopHandoff: invalid ReadyToBoot callback context\n"));
    return;
  }
  
  Status = CaptureCurrentGop(TRUE);
  if (EFI_ERROR(Status)) {
    DEBUG((
           DEBUG_ERROR,
           "GopHandoff: ReadyToBoot capture failed: %r; preserving the last verified early handoff when available\n",
           Status
           ));
  }
}

EFI_STATUS
EFIAPI
GopHandoffUnload (
                  IN EFI_HANDLE ImageHandle
                  )
{
  EFI_STATUS Status;
  
  if ((ImageHandle == NULL) || (ImageHandle != mImageHandle)) {
    return EFI_INVALID_PARAMETER;
  }
  
  if (mReadyToBootEvent != NULL) {
    Status = gBS->CloseEvent(mReadyToBootEvent);
    if (EFI_ERROR(Status)) {
      return Status;
    }
    mReadyToBootEvent = NULL;
  }
  
  mImageHandle = NULL;
  return DeleteOutputVariables();
}

EFI_STATUS
EFIAPI
GopHandoffEntryPoint (
                      IN EFI_HANDLE        ImageHandle,
                      IN EFI_SYSTEM_TABLE *SystemTable
                      )
{
  EFI_STATUS Status;
  
  if ((ImageHandle == NULL) ||
      (SystemTable == NULL) ||
      (SystemTable != gST) ||
      (mImageHandle != NULL)) {
    return EFI_INVALID_PARAMETER;
  }
  
  mImageHandle = ImageHandle;
  Status = DeleteOutputVariables();
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "GopHandoff: initial cleanup failed: %r\n", Status));
    mImageHandle = NULL;
    return Status;
  }
  
  Status = EfiCreateEventReadyToBootEx(
                                       TPL_CALLBACK,
                                       CaptureAtReadyToBoot,
                                       mImageHandle,
                                       &mReadyToBootEvent
                                       );
  if (EFI_ERROR(Status)) {
    mReadyToBootEvent = NULL;
    mImageHandle = NULL;
    DEBUG((
           DEBUG_ERROR,
           "GopHandoff: ReadyToBoot event creation failed: %r\n",
           Status
           ));
    return Status;
  }
  
  Status = CaptureCurrentGop(FALSE);
  if (EFI_ERROR(Status)) {
    DEBUG((
           DEBUG_ERROR,
           "GopHandoff: early EntryPoint capture failed: %r; ReadyToBoot retry remains armed\n",
           Status
           ));
  } else {
    DEBUG((
           DEBUG_INFO,
           "GopHandoff: early EntryPoint handoff published before the OS booter can snapshot /options\n"
           ));
  }
  
  DEBUG((
         DEBUG_INFO,
         "GopHandoff: armed for ReadyToBoot recapture, GOP mode programming and final framebuffer refresh\n"
         ));
  return EFI_SUCCESS;
}

