/**
 
 UEFI driver for enabling loading of OSX without memory relocation.
 
 by dmazar
 
 **/

#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/CpuLib.h>

#include <Guid/GlobalVariable.h>

#include <Protocol/LoadedImage.h>

#include "BootFixes3.h"
#include "DecodedKernelCheck.h"
#include "BootArgs.h"
#include "AsmFuncs.h"
#include "VMem.h"
#include "Lib.h"
#include "Hibernate.h"
#include "NVRAMDebug.h"

#include "RTShims.h"

// DBG_TO: 0=no debug, 1=serial, 2=console
// serial requires
// [PcdsFixedAtBuild]
//  gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x07
//  gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0xFFFFFFFF
// in package DSC file
#define DBG_TO 0

#if DBG_TO == 2
#define DBG(...) AsciiPrint(__VA_ARGS__);
#elif DBG_TO == 1
#define DBG(...) DebugPrint(1, __VA_ARGS__);
#else
#define DBG(...)
#endif

#include "../Version.h"
CONST CHAR8* CloverRevision = REVISION_STR;
STATIC UINTN Counter = 0;


// TRUE if we are doing hibernate wake
BOOLEAN gHibernateWake = FALSE;


// placeholders for storing original Boot and RT Services functions
EFI_ALLOCATE_PAGES       gStoredAllocatePages = NULL;
EFI_GET_MEMORY_MAP       gStoredGetMemoryMap = NULL;
EFI_EXIT_BOOT_SERVICES     gStoredExitBootServices = NULL;
EFI_IMAGE_START       gStartImage = NULL;
EFI_HANDLE_PROTOCOL      gHandleProtocol = NULL;
EFI_SET_VIRTUAL_ADDRESS_MAP gStoredSetVirtualAddressMap = NULL;
UINT32                      OrgRTCRC32 = 0;


// monitoring AlocatePages
EFI_PHYSICAL_ADDRESS gMinAllocatedAddr = 0;
EFI_PHYSICAL_ADDRESS gMaxAllocatedAddr = 0;

// relocation base address
EFI_PHYSICAL_ADDRESS gRelocBase = 0;
// relocation block size in pages
UINTN gRelocSizePages = 0;

// location of memory allocated by boot.efi for hibernate image
EFI_PHYSICAL_ADDRESS gHibernateImageAddress = 0;

// last memory map obtained by boot.efi
UINTN          gLastMemoryMapSize = 0;
EFI_MEMORY_DESCRIPTOR  *gLastMemoryMap = NULL;
UINTN          gLastDescriptorSize = 0;
UINT32          gLastDescriptorVersion = 0;

/** Helper function that calls GetMemoryMap() and returns new MapKey.
 * Uses gStoredGetMemoryMap, so can be called only after gStoredGetMemoryMap is set.
 */
EFI_STATUS
GetMemoryMapKey(OUT UINTN *MapKey)
{
  EFI_STATUS          Status;
  UINTN            MemoryMapSize;
  EFI_MEMORY_DESCRIPTOR    *MemoryMap;
  UINTN            DescriptorSize;
  UINT32            DescriptorVersion;
  
  Status = GetMemoryMapAlloc(gStoredGetMemoryMap, &MemoryMapSize, &MemoryMap, MapKey, &DescriptorSize, &DescriptorVersion);
  return Status;
}

/** Helper function that calculates number of RT and MMIO pages from mem map. */
EFI_STATUS
GetNumberOfRTPages(OUT UINTN *NumPages)
{
  EFI_STATUS          Status;
  UINTN            MemoryMapSize;
  EFI_MEMORY_DESCRIPTOR    *MemoryMap;
  UINTN            MapKey;
  UINTN            DescriptorSize;
  UINT32            DescriptorVersion;
  UINTN            NumEntries;
  UINTN            Index;
  EFI_MEMORY_DESCRIPTOR    *Desc;
  
  Status = GetMemoryMapAlloc(gBS->GetMemoryMap, &MemoryMapSize, &MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  
  //
  // Apply some fixes
  //
  FixMemMap(MemoryMapSize, MemoryMap, DescriptorSize, DescriptorVersion);
  
  //
  // Sum RT and MMIO areas - all that have runtime attribute
  //
  
  *NumPages = 0;
  Desc = MemoryMap;
  NumEntries = MemoryMapSize / DescriptorSize;
  
  for (Index = 0; Index < NumEntries; Index++) {
    if ((Desc->Attribute & EFI_MEMORY_RUNTIME) != 0) {
      *NumPages += Desc->NumberOfPages;
    }
    Desc = NEXT_MEMORY_DESCRIPTOR(Desc, DescriptorSize);
  }
  
  return Status;
}


/** gBS->HandleProtocol override:
 * Boot.efi requires EfiGraphicsOutputProtocol on ConOutHandle, but it is not present
 * there on Aptio 2.0. EfiGraphicsOutputProtocol exists on some other handle.
 * If this is the case, we'll intercept that call and return EfiGraphicsOutputProtocol
 * from that other handle.
 */
EFI_STATUS EFIAPI
MOHandleProtocol(
                 IN EFI_HANDLE    Handle,
                 IN EFI_GUID      *Protocol,
                 OUT VOID      **Interface
                 )
{
  EFI_STATUS      res;
  EFI_GRAPHICS_OUTPUT_PROTOCOL  *GraphicsOutput;
  
  // special handling if gEfiGraphicsOutputProtocolGuid is requested by boot.efi
  if (CompareGuid(Protocol, &gEfiGraphicsOutputProtocolGuid)) {
    res = gHandleProtocol(Handle, Protocol, Interface);
    if (res != EFI_SUCCESS) {
      // let's find it on some other handle
      res = gBS->LocateProtocol(&gEfiGraphicsOutputProtocolGuid, NULL, (VOID**)&GraphicsOutput);
      if (res == EFI_SUCCESS) {
        // return it
        *Interface = GraphicsOutput;
        //      DBG("->HandleProtocol(%p, %s, %p) = %r (returning from other handle)\n", Handle, GuidStr(Protocol), *Interface, res);
        DBGnvr("->HandleProtocol(%p, %s, %p) = %r (from other handle)\n", Handle, GuidStr(Protocol), *Interface, res);
        return res;
      }
    }
    DBGnvr("->HandleProtocol(%p, %s, %p) = %r\n", Handle, GuidStr(Protocol), *Interface, res);
  } else {
    res = gHandleProtocol(Handle, Protocol, Interface);
  }
  //  DBG("->HandleProtocol(%p, %s, %p) = %r\n", Handle, GuidStr(Protocol), *Interface, res);
  return res;
}

/** gBS->AllocatePages override:
 * Returns pages from free memory block to boot.efi for kernel boot image.
 */
EFI_STATUS
EFIAPI
MOAllocatePages (
                 IN EFI_ALLOCATE_TYPE    Type,
                 IN EFI_MEMORY_TYPE      MemoryType,
                 IN UINTN          NumberOfPages,
                 IN OUT EFI_PHYSICAL_ADDRESS  *Memory
                 )
{
  EFI_STATUS          Status;
  EFI_PHYSICAL_ADDRESS    UpperAddr;
  //  EFI_PHYSICAL_ADDRESS    MemoryIn;
  //  BOOLEAN            FromRelocBlock = FALSE;
  
  
  //  MemoryIn = *Memory;
  
  if (Type == AllocateAddress && MemoryType == EfiLoaderData) {
    // called from boot.efi
    
    UpperAddr = *Memory + EFI_PAGES_TO_SIZE(NumberOfPages);
    
    // store min and max mem - can be used later to determine start and end of kernel boot image
    if (gMinAllocatedAddr == 0 || *Memory < gMinAllocatedAddr) gMinAllocatedAddr = *Memory;
    if (UpperAddr > gMaxAllocatedAddr) gMaxAllocatedAddr = UpperAddr;
    
    Status = gStoredAllocatePages(Type, MemoryType, NumberOfPages, Memory);
    //    FromRelocBlock = FALSE;
    
  } else if (gHibernateWake && Type == AllocateAnyPages && MemoryType == EfiLoaderData) {
    // called from boot.efi during hibernate wake
    // first such allocation is for hibernate image
    Status = gStoredAllocatePages(Type, MemoryType, NumberOfPages, Memory);
    if (gHibernateImageAddress == 0 && Status == EFI_SUCCESS) {
      gHibernateImageAddress = *Memory;
    }
    
  } else {
    // default page allocation
    Status = gStoredAllocatePages(Type, MemoryType, NumberOfPages, Memory);
    
  }
  
  //DBG("AllocatePages(%s, %s, %x, %lx/%lx) = %r %c\n",
  //  EfiAllocateTypeDesc[Type], EfiMemoryTypeDesc[MemoryType], NumberOfPages, MemoryIn, *Memory, Status, FromRelocBlock ? L'+' : L' ');
  return Status;
}


/** gBS->GetMemoryMap override:
 * Returns shrinked memory map. I think kernel can handle up to 128 entries.
 */
EFI_STATUS
EFIAPI
MOGetMemoryMap (
                IN OUT UINTN          *MemoryMapSize,
                IN OUT EFI_MEMORY_DESCRIPTOR  *MemoryMap,
                OUT UINTN            *MapKey,
                OUT UINTN            *DescriptorSize,
                OUT UINT32            *DescriptorVersion
                )
{
  EFI_STATUS            Status;
  
  Status = gStoredGetMemoryMap(MemoryMapSize, MemoryMap, MapKey, DescriptorSize, DescriptorVersion);
  //PrintMemMap(*MemoryMapSize, MemoryMap, *DescriptorSize, *DescriptorVersion);
  DBGnvr("GetMemoryMap: %p = %r\n", MemoryMap, Status);
  if (Status == EFI_SUCCESS) {
    FixMemMap(*MemoryMapSize, MemoryMap, *DescriptorSize, *DescriptorVersion);
    ShrinkMemMap(MemoryMapSize, MemoryMap, *DescriptorSize, *DescriptorVersion);
    //PrintMemMap(*MemoryMapSize, MemoryMap, *DescriptorSize, *DescriptorVersion);
    
    // remember last/final memmap
    gLastMemoryMapSize = *MemoryMapSize;
    gLastMemoryMap = MemoryMap;
    gLastDescriptorSize = *DescriptorSize;
    gLastDescriptorVersion = *DescriptorVersion;
  }
  return Status;
}

/** gBS->ExitBootServices override:
 * Patches kernel entry point with jump to our KernelEntryPatchJumpBack().
 */
EFI_STATUS
EFIAPI
MOExitBootServices (
                    IN EFI_HANDLE        ImageHandle,
                    IN UINTN          MapKey
                    )
{
  EFI_STATUS          Status;
  UINTN             NewMapKey;
  UINTN            SlideAddr = 0;
  VOID            *MachOImage = NULL;
  IOHibernateImageHeader      *ImageHeader = NULL;
  
  // we need hibernate image address for wake
  if (gHibernateWake && gHibernateImageAddress == 0) {
    Print(L"OsxAptioFix3 error: Doing hibernate wake, but did not find hibernate image address.");
    Print(L"... waiting 5 secs ...\n");
    gBS->Stall(5000000);
    return EFI_INVALID_PARAMETER;
  }
  
  // for  tests: we can just return EFI_SUCCESS and continue using Print for debug.
  //  Status = EFI_SUCCESS;
  //Print(L"ExitBootServices()\n");
  Status = gStoredExitBootServices(ImageHandle, MapKey);
  DBGnvr("ExitBootServices:  = %r\n", Status);
  if (EFI_ERROR(Status)) {
    // just report error as var in nvram to be visible from OSX with "nvram -p"
    gRT->SetVariable(L"OsxAptioFixDrv-ErrorExitingBootServices",
                     &gEfiAppleBootGuid,
                     EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                     3,
                     "Yes"
                     );
    
    Status = GetMemoryMapKey(&NewMapKey);
    DBGnvr("ExitBootServices: GetMemoryMapKey = %r\n", Status);
    if (Status == EFI_SUCCESS) {
      // we have latest mem map and NewMapKey
      // we'll try again ExitBootServices with NewMapKey
      Status = gStoredExitBootServices(ImageHandle, NewMapKey);
      DBGnvr("ExitBootServices: 2nd try = %r\n", Status);
      if (EFI_ERROR(Status)) {
        // Error!
        Print(L"OsxAptioFix3Drv: Error ExitBootServices() 2nd try = Status: %r\n", Status);
      }
    } else {
      Print(L"OsxAptioFix3Drv: Error ExitBootServices(), GetMemoryMapKey() = Status: %r\n", Status);
      Status = EFI_INVALID_PARAMETER;
    }
    
  }
  
  if (EFI_ERROR(Status)) {
    Print(L"... waiting 10 secs ...\n");
    gBS->Stall(10*1000000);
    return Status;
  }
  
  if (!gHibernateWake) {
    // normal boot
    DBG("ExitBootServices: gMinAllocatedAddr: %lx, gMaxAllocatedAddr: %lx\n", gMinAllocatedAddr, gMaxAllocatedAddr);
    
    SlideAddr = gMinAllocatedAddr - 0x100000;
    MachOImage = (VOID*)(UINTN)(SlideAddr + 0x200000);
    KernelEntryFromMachOPatchJump(MachOImage, SlideAddr);
    
  } else {
    // hibernate wake
    
    // at this stage HIB section is not yet copied from sleep image to it's
    // proper memory destination. so we'll patch entry point in sleep image.
    ImageHeader = (IOHibernateImageHeader *)(UINTN)gHibernateImageAddress;
    KernelEntryPatchJump( ((UINT32)(UINTN)&(ImageHeader->fileExtentMap[0])) + ImageHeader->fileExtentMapSize + ImageHeader->restore1CodeOffset );
  }
  
  return Status;
}


/** gRT->SetVirtualAddressMap override:
 * Fixes virtualizing of RT services.
 */
EFI_STATUS EFIAPI
OvrSetVirtualAddressMap(
                        IN UINTN      MemoryMapSize,
                        IN UINTN      DescriptorSize,
                        IN UINT32      DescriptorVersion,
                        IN EFI_MEMORY_DESCRIPTOR  *VirtualMap
                        )
{
  EFI_STATUS      Status;
  UINT32              EfiSystemTable;
  
  DBG("->SetVirtualAddressMap(%d, %d, 0x%x, %p) START ...\n", MemoryMapSize, DescriptorSize, DescriptorVersion, VirtualMap);
  DBGnvr("->SetVirtualAddressMap(%d, %d, 0x%x, %p) START ...\n", MemoryMapSize, DescriptorSize, DescriptorVersion, VirtualMap);
  
  // restore origs
  gRT->Hdr.CRC32 = OrgRTCRC32;
  gRT->SetVirtualAddressMap = gStoredSetVirtualAddressMap;
  
  // Protect RT data areas from relocation by marking then MemMapIO
  ProtectRtDataFromRelocation(MemoryMapSize, DescriptorSize, DescriptorVersion, VirtualMap);
  
  // Remember physical sys table addr
  EfiSystemTable = (UINT32)(UINTN)gST;
  
  // virtualize RT services with all needed fixes
  Status = ExecSetVirtualAddressesToMemMap(MemoryMapSize, DescriptorSize, DescriptorVersion, VirtualMap);

  CopyEfiSysTableToSeparateRtDataArea(&EfiSystemTable);
  
  // we will defragment RT data and code that is left unprotected.
  // this will also mark those as AcpiNVS and by this protect it
  // from boot.efi relocation and zeroing
  DefragmentRuntimeServices(MemoryMapSize,
                            DescriptorSize,
                            DescriptorVersion,
                            VirtualMap,
                            NULL,
                            gHibernateWake ? FALSE : TRUE
                            );

  // For AptioFix V2 we can correct the pointers earlier
  VirtualizeRTShimPointers(MemoryMapSize, DescriptorSize, VirtualMap);
  
  return Status;
}


/** Callback called when boot.efi jumps to kernel. */
UINTN
EFIAPI
KernelEntryPatchJumpBack(UINTN bootArgs, BOOLEAN ModeX64)
{
  
  DBGnvr("\nBACK FROM KERNEL: BootArgs = %x, KernelEntry: %x, Kernel called in %s bit mode\n", bootArgs, AsmKernelEntry, (ModeX64 ? L"64" : L"32"));
  
  if (!gHibernateWake) {
    bootArgs = FixBootingWithoutRelocBlock(bootArgs, ModeX64);
  } else {
    bootArgs = FixHibernateWakeWithoutRelocBlock(bootArgs, ModeX64);
  }
  
  DBGnvr("BACK TO KERNEL: BootArgs = %x, KImgStartReloc = %x, KImgStart = %x, KImgSize = %x\n",
         bootArgs, AsmKernelImageStartReloc, AsmKernelImageStart, AsmKernelImageSize);
  
  return bootArgs;
}


/** SWITCH_STACK_ENTRY_POINT implementation:
 * Allocates kernel image reloc block, installs UEFI overrides and starts given image.
 * If image returns, then deinstalls overrides and releases kernel image reloc block.
 *
 * If started with ImgContext->JumpBuffer, then it will return with LongJump().
 */
EFI_STATUS
RunImageWithOverrides(
                      IN EFI_HANDLE ImageHandle,
                      IN EFI_LOADED_IMAGE_PROTOCOL  *Image,
                      OUT UINTN *ExitDataSize,
                      OUT CHAR16 **ExitData  OPTIONAL
                      )
{
  EFI_STATUS          Status;

  // save current 64bit state - will be restored later in callback from kernel jump
  // and relocate MyAsmCopyAndJumpToKernel32 code to higher mem (for copying kernel back to
  // proper place and jumping back to it)
  Status = PrepareJumpFromKernel();
  if (EFI_ERROR(Status)) {
    return Status;
  }
  
  // init VMem memory pool - will be used after ExitBootServices
  Status = VmAllocateMemoryPool();
  if (EFI_ERROR(Status)) {
    return Status;
  }

  Status = gBS->AllocatePool (
                  EfiRuntimeServicesCode,
                  ((UINTN)&gRTShimsDataEnd - (UINTN)&gRTShimsDataStart),
                  &RTShims
                  );

  if (!EFI_ERROR(Status)) {
    gGetVariable         = (UINTN)gRT->GetVariable;
    gGetNextVariableName = (UINTN)gRT->GetNextVariableName;
    gSetVariable         = (UINTN)gRT->SetVariable;

    CopyMem (
      RTShims,
      (VOID *)&gRTShimsDataStart,
      ((UINTN)&gRTShimsDataEnd - (UINTN)&gRTShimsDataStart)
      );

    gRT->GetVariable         = (EFI_GET_VARIABLE)((UINTN)RTShims           + ((UINTN)&RTShimGetVariable         - (UINTN)&gRTShimsDataStart));
    gRT->GetNextVariableName = (EFI_GET_NEXT_VARIABLE_NAME)((UINTN)RTShims + ((UINTN)&RTShimGetNextVariableName - (UINTN)&gRTShimsDataStart));
    gRT->SetVariable         = (EFI_SET_VARIABLE)((UINTN)RTShims           + ((UINTN)&RTShimSetVariable         - (UINTN)&gRTShimsDataStart));

    gRT->Hdr.CRC32 = 0;
    gBS->CalculateCrc32 (gRT, gRT->Hdr.HeaderSize, &gRT->Hdr.CRC32);
  } else {
    RTShims = NULL;
  }
  
  // clear monitoring vars
  gMinAllocatedAddr = 0;
  gMaxAllocatedAddr = 0;
  
  // save original BS functions
  gStoredAllocatePages = gBS->AllocatePages;
  gStoredGetMemoryMap = gBS->GetMemoryMap;
  gStoredExitBootServices = gBS->ExitBootServices;
  gHandleProtocol = gBS->HandleProtocol;
  
  // install our overrides
  gBS->AllocatePages = MOAllocatePages;
  gBS->GetMemoryMap = MOGetMemoryMap;
  gBS->ExitBootServices = MOExitBootServices;
  gBS->HandleProtocol = MOHandleProtocol;
  
  gBS->Hdr.CRC32 = 0;
  gBS->CalculateCrc32(gBS, gBS->Hdr.HeaderSize, &gBS->Hdr.CRC32);
  
  OrgRTCRC32 = gRT->Hdr.CRC32;
  gStoredSetVirtualAddressMap = gRT->SetVirtualAddressMap;
  gRT->SetVirtualAddressMap = OvrSetVirtualAddressMap;
  gRT->Hdr.CRC32 = 0;
  gBS->CalculateCrc32(gRT, gRT->Hdr.HeaderSize, &gRT->Hdr.CRC32);
  
  // force boot.efi to use our copy od system table
  DBG("StartImage: orig sys table: %p\n", Image->SystemTable);
  Image->SystemTable = (EFI_SYSTEM_TABLE *)(UINTN)gSysTableRtArea;
  DBG("StartImage: new sys table: %p\n", Image->SystemTable);
  
  // run image
  Status = gStartImage(ImageHandle, ExitDataSize, ExitData);
  
  // if we get here then boot.efi did not start kernel
  // and we'll try to do some cleanup ...
  
  // return back originals
  gBS->AllocatePages = gStoredAllocatePages;
  gBS->GetMemoryMap = gStoredGetMemoryMap;
  gBS->ExitBootServices = gStoredExitBootServices;
  gBS->HandleProtocol = gHandleProtocol;
  
  gBS->Hdr.CRC32 = 0;
  gBS->CalculateCrc32(gBS, gBS->Hdr.HeaderSize, &gBS->Hdr.CRC32);
  
  gRT->SetVirtualAddressMap = gStoredSetVirtualAddressMap;
  gBS->CalculateCrc32(gRT, gRT->Hdr.HeaderSize, &gRT->Hdr.CRC32);
  
  return Status;
}

/** gBS->StartImage override:
 * Called to start an efi image.
 *
 * If this is boot.efi, then run it with our overrides.
 */
EFI_STATUS
EFIAPI
MOStartImage (
              IN EFI_HANDLE      ImageHandle,
              OUT UINTN          *ExitDataSize,
              OUT CHAR16        **ExitData  OPTIONAL
              )
{
  EFI_STATUS                  Status;
  EFI_LOADED_IMAGE_PROTOCOL   *Image;
  CHAR16                      *FilePathText = NULL;
  UINTN                       Size          = 0;
  VOID                        *Value        = NULL;
  UINTN                       Size2         = 0;
  CHAR16                      *StartFlag    = NULL;

  
  DBG("StartImage(%lx)\n", ImageHandle);
  
  // find out image name from EfiLoadedImageProtocol
  Status = gBS->OpenProtocol(ImageHandle, &gEfiLoadedImageProtocolGuid, (VOID **) &Image, gImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
  if (Status != EFI_SUCCESS) {
    DBG("ERROR: MOStartImage: OpenProtocol(gEfiLoadedImageProtocolGuid) = %r\n", Status);
    return EFI_INVALID_PARAMETER;
  }
  FilePathText = FileDevicePathToText(Image->FilePath);
  if (FilePathText != NULL) {
    DBG("FilePath: %s\n", FilePathText);
  }
  DBG("ImageBase: %p - %lx (%lx)\n", Image->ImageBase, (UINT64)Image->ImageBase + Image->ImageSize, Image->ImageSize);
  Status = gBS->CloseProtocol(ImageHandle, &gEfiLoadedImageProtocolGuid, gImageHandle, NULL);
  if (EFI_ERROR(Status)) {
    DBG("CloseProtocol error: %r\n", Status);
  }
  
  if (StrStriBasic(FilePathText,L"boot.efi") /*|| StrStriBasic(FilePathText,L"booter")*/) {
    Status = GetVariable2 (L"aptiofixflag", &gEfiAppleBootGuid, &Value, &Size2);
    if (!EFI_ERROR(Status)) {
      Status = gRT->SetVariable(L"recovery-boot-mode", &gEfiAppleBootGuid,
                                EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                                Size2, Value);
      if (EFI_ERROR(Status)) {
        DBG(" Something goes wrong while setting recovery-boot-mode\n");
      }
      Status = gRT->SetVariable (L"aptiofixflag", &gEfiAppleBootGuid, 0, 0, NULL);
      FreePool(Value);
    }
    
    Size2 =0;
    //Check recovery-boot-mode present for nested boot.efi
    Status = GetVariable2 (L"recovery-boot-mode", &gEfiAppleBootGuid, &Value, &Size2);
    if (!EFI_ERROR(Status)) {
      //If it presents, then wait for \com.apple.recovery.boot\boot.efi boot
      DBG(" recovery-boot-mode present\n");
      StartFlag = StrStriBasic(FilePathText,L"\\com.apple.recovery.boot\\boot.efi");
      if (Counter > 0x00){
        Status = gRT->SetVariable(L"aptiofixflag", &gEfiAppleBootGuid,
                                  EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                                  Size2, Value);
        if (EFI_ERROR(Status)) {
          DBG("Something goes wrong! \n");
        }
          gRT->ResetSystem(EfiResetWarm, EFI_SUCCESS, 0, NULL);
      }
    } else {
      StartFlag = StrStriBasic(FilePathText,L"boot.efi");
      /*if (!StartFlag) {
        StartFlag = StrStriBasic(FilePathText,L"booter") ;
      }*/
    }
    FreePool(Value);
  }
  

  
  // check if this is boot.efi
  // if (StrStriBasic(FilePathText, L"boot.efi")) {
  if (StartFlag) {
    Counter++;
    //the presence of the variable means HibernateWake
    //if the wake is canceled then the variable must be deleted
    Status = gRT->GetVariable(L"boot-switch-vars", &gEfiAppleBootGuid, NULL, &Size, NULL);
    gHibernateWake = (Status == EFI_BUFFER_TOO_SMALL);
    
    Print(L"OsxAptioFix3Drv: Starting overrides for %s\nUsing reloc block: no, hibernate wake: %s \n",
          FilePathText, gHibernateWake ? L"yes" : L"no");
    //gBS->Stall(2000000);
    
    // run with our overrides
    Status = RunImageWithOverrides(ImageHandle, Image, ExitDataSize, ExitData);
    
  } else {
    // call original function to do the job
    Status = gStartImage(ImageHandle, ExitDataSize, ExitData);
  }
  
  if (FilePathText != NULL) {
    gBS->FreePool(FilePathText);
  }
  return Status;
}


/** Entry point. Installs our StartImage override.
 * All other stuff will be installed from there when boot.efi is started.
 */
EFI_STATUS
EFIAPI
OsxAptioFixDrvEntrypoint (
                          IN EFI_HANDLE        ImageHandle,
                          IN EFI_SYSTEM_TABLE      *SystemTable
                          )
{
  // install StartImage override
  // all other overrides will be started when boot.efi is started
  gStartImage = gBS->StartImage;
  gBS->StartImage = MOStartImage;
  gBS->Hdr.CRC32 = 0;
  gBS->CalculateCrc32(gBS, gBS->Hdr.HeaderSize, &gBS->Hdr.CRC32);
  
  return EFI_SUCCESS;
}

