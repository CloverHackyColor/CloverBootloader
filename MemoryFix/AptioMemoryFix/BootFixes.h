/**

  Methods for setting callback jump from kernel entry point, callback, fixes to kernel boot image.

  by dmazar

**/

#ifndef APTIOFIX_BOOT_FIXES_H
#define APTIOFIX_BOOT_FIXES_H

#include <Protocol/LoadedImage.h>

//
// Original and relocated new area for EFI System Table.
// XNU requires gST pointers to be passed relative to boot.efi.
// We have to allocate a new system table and let boot.efi relocate it.
//
extern EFI_PHYSICAL_ADDRESS   gSysTableRtArea;
extern EFI_PHYSICAL_ADDRESS   gRelocatedSysTableRtArea;

//
// TRUE if we are doing hibernate wake
//
extern BOOLEAN gHibernateWake;

//
// TRUE if booting with -aptiodump
//
extern BOOLEAN gDumpMemArgPresent;

//
// TRUE if booting with a manually specified slide=X
//
extern BOOLEAN gSlideArgPresent;

//
// TRUE if booting on memory map unstable firmware, such as APTIO
//
extern BOOLEAN gHasBrokenS4MemoryMap;

//
// TRUE if booting on memory allocation unstable firmware, such as INSYDE
//
extern BOOLEAN gHasBrokenS4Allocator;

VOID
ApplyFirmwareQuirks (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  );

VOID
ReadBooterArguments (
  CHAR16  *Options,
  UINTN   OptionsSize
  );

EFI_STATUS
PrepareJumpFromKernel (
  VOID
  );

EFI_STATUS
KernelEntryPatchJump (
  UINT32  KernelEntry
  );

EFI_STATUS
KernelEntryFromMachOPatchJump (
  VOID   *MachOImage,
  UINTN  SlideAddr
  );

EFI_STATUS
ExecSetVirtualAddressesToMemMap (
  IN UINTN                  MemoryMapSize,
  IN UINTN                  DescriptorSize,
  IN UINT32                 DescriptorVersion,
  IN EFI_MEMORY_DESCRIPTOR  *MemoryMap
  );

VOID
CopyEfiSysTableToRtArea (
  IN OUT UINT32  *EfiSystemTable
  );

EFI_LOADED_IMAGE_PROTOCOL *
GetAppleBootLoadedImage (
  EFI_HANDLE  ImageHandle
  );

#endif // APTIOFIX_BOOT_FIXES_H
