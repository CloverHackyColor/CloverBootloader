#include <Uefi.h>

#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Protocol/EdidActive.h>
#include <Protocol/EdidDiscovered.h>
#include <Protocol/GraphicsOutput.h>

#include "Common/GopFramebufferProtocol.h"

STATIC EFI_EVENT  mReadyToBootEvent = NULL;
STATIC EFI_HANDLE mImageHandle = NULL;

STATIC EFI_GUID mGopFramebufferVariableGuid = {
    0xb5a6f0a9,
    0x3e6f,
    0x4d90,
    { 0x8a, 0x1a, 0x2f, 0x9d, 0x1d, 0x2e, 0x8c, 0x71 }
};

STATIC CHAR16 mHandoffVariableName[] = L"GopFramebufferHandoff";
STATIC CHAR16 mDisplayInfoVariableName[] = L"GopFramebufferDisplayInfo";
STATIC CHAR16 mModeRequestVariableName[] = L"GopFramebufferModeRequest";

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
    EFI_STATUS DisplayInfoStatus;

    HandoffStatus = DeleteVariable(mHandoffVariableName);
    DisplayInfoStatus = DeleteVariable(mDisplayInfoVariableName);

    if (EFI_ERROR(HandoffStatus)) {
        return HandoffStatus;
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
    if (!EFI_ERROR(Status) && IsUsableGop(Gop)) {
        *SelectedHandle = gST->ConsoleOutHandle;
        *SelectedGop = Gop;
        *IsConsoleOut = TRUE;
        return EFI_SUCCESS;
    }

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
            Gop = Candidate;
            GopHandle = Handles[Index];
            ++UsableCount;
        }
    }

    FreePool(Handles);

    if (UsableCount == 0U) {
        return EFI_NOT_FOUND;
    }

    if (UsableCount != 1U) {
        DEBUG((
            DEBUG_ERROR,
            "GopHandoff: %Lu usable GOP instances and no unique ConsoleOut GOP\n",
            (UINT64)UsableCount
            ));
        return EFI_ABORTED;
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
    Size = sizeof(*Request);
    Status = gRT->GetVariable(
                    mModeRequestVariableName,
                    &mGopFramebufferVariableGuid,
                    NULL,
                    &Size,
                    Request
                    );
    if (Status == EFI_NOT_FOUND) {
        return EFI_SUCCESS;
    }
    if (EFI_ERROR(Status)) {
        return Status;
    }
    if (Size != sizeof(*Request)) {
        return EFI_COMPROMISED_DATA;
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
    IN UINTN  RequiredSize
    )
{
    EFI_STATUS Status;
    UINT64     MaximumVariableStorageSize;
    UINT64     RemainingVariableStorageSize;
    UINT64     MaximumVariableSize;

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

    if (((UINT64)RequiredSize > MaximumVariableSize) ||
        ((UINT64)RequiredSize > RemainingVariableStorageSize)) {
        return EFI_OUT_OF_RESOURCES;
    }

    return EFI_SUCCESS;
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

    Status = CheckVariableCapacity(
                 EFI_VARIABLE_BOOTSERVICE_ACCESS |
                 EFI_VARIABLE_RUNTIME_ACCESS,
                 TotalSize
                 );
    if (EFI_ERROR(Status)) {
        FreePool(ModeStorage);
        return Status;
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
    Handoff->flags = GOPFB_FLAG_READY_TO_BOOT |
                     GOPFB_FLAG_MODE_REAPPLIED;
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
EFI_STATUS
CaptureCurrentGop (
    VOID
    )
{
    EFI_STATUS                    Status;
    EFI_HANDLE                    GopHandle;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop;
    BOOLEAN                       IsConsoleOut;
    BOOLEAN                       RequestApplied;
    GopFramebufferHandoff         Handoff;
    VOID                         *DisplayInfo;
    UINTN                         DisplayInfoSize;
    UINT32                        Attributes;

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

    Status = BuildHandoff(Gop, IsConsoleOut, RequestApplied, &Handoff);
    if (EFI_ERROR(Status)) {
        return Status;
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
        return Status;
    }

    Attributes = EFI_VARIABLE_BOOTSERVICE_ACCESS |
                 EFI_VARIABLE_RUNTIME_ACCESS;
    Status = gRT->SetVariable(
                    mDisplayInfoVariableName,
                    &mGopFramebufferVariableGuid,
                    Attributes,
                    DisplayInfoSize,
                    DisplayInfo
                    );
    FreePool(DisplayInfo);
    if (EFI_ERROR(Status)) {
        DEBUG((
            DEBUG_ERROR,
            "GopHandoff: display-info SetVariable failed: %r\n",
            Status
            ));
        return Status;
    }

    Status = gRT->SetVariable(
                    mHandoffVariableName,
                    &mGopFramebufferVariableGuid,
                    Attributes,
                    sizeof(Handoff),
                    &Handoff
                    );
    if (EFI_ERROR(Status)) {
        DeleteVariable(mDisplayInfoVariableName);
        DEBUG((DEBUG_ERROR, "GopHandoff: handoff SetVariable failed: %r\n", Status));
        return Status;
    }

    DEBUG((
        DEBUG_INFO,
        "GopHandoff: FB=%Lx size=%Lu mode=%ux%u stride=%u format=%u modeset=%a\n",
        Handoff.framebufferBase,
        Handoff.framebufferSize,
        Handoff.width,
        Handoff.height,
        Handoff.pixelsPerScanLine,
        Handoff.pixelFormat,
        RequestApplied ? "requested" : "current-reapplied"
        ));

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

    Status = DeleteOutputVariables();
    if (EFI_ERROR(Status)) {
        DEBUG((
            DEBUG_ERROR,
            "GopHandoff: stale-output cleanup failed at ReadyToBoot: %r\n",
            Status
            ));
        return;
    }

    Status = CaptureCurrentGop();
    if (EFI_ERROR(Status)) {
        DeleteOutputVariables();
        DEBUG((DEBUG_ERROR, "GopHandoff: ReadyToBoot capture failed: %r\n", Status));
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

    DEBUG((
        DEBUG_INFO,
        "GopHandoff: armed for GOP mode programming, EDID and final framebuffer capture\n"
        ));
    return EFI_SUCCESS;
}
