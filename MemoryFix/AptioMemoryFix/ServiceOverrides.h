/**

  Temporary BS and RT overrides for boot.efi support.
  Unlike RtShims they do not affect the kernel.

  by dmazar

**/

#ifndef APTIOFIX_SERVICE_OVERRIDES_H
#define APTIOFIX_SERVICE_OVERRIDES_H

//
// Last descriptor size obtained from GetMemoryMap
//
extern UINTN                       gMemoryMapDescriptorSize;

//
// Amount of nested boot.efi detected
//
extern UINTN                       gMacOSBootNestedCount;

VOID
InstallBsOverrides (
  VOID
  );

VOID
InstallRtOverrides (
  VOID
  );

VOID
UninstallRtOverrides (
  VOID
  );

VOID
DisableDynamicPoolAllocations (
  VOID
  );

VOID
EnableDynamicPoolAllocations (
  VOID
  );

EFI_STATUS
EFIAPI
MOStartImage (
  IN     EFI_HANDLE  ImageHandle,
     OUT UINTN       *ExitDataSize,
     OUT CHAR16      **ExitData  OPTIONAL
  );

EFI_STATUS
EFIAPI
MOAllocatePages (
  IN     EFI_ALLOCATE_TYPE     Type,
  IN     EFI_MEMORY_TYPE       MemoryType,
  IN     UINTN                 NumberOfPages,
  IN OUT EFI_PHYSICAL_ADDRESS  *Memory
  );

EFI_STATUS
EFIAPI
MOAllocatePool (
  IN     EFI_MEMORY_TYPE  Type,
  IN     UINTN            Size,
     OUT VOID             **Buffer
  );

EFI_STATUS
EFIAPI
MOFreePool(
  IN VOID  *Buffer
  );

EFI_STATUS
EFIAPI
MOGetMemoryMap (
  IN OUT UINTN                  *MemoryMapSize,
  IN OUT EFI_MEMORY_DESCRIPTOR  *MemoryMap,
     OUT UINTN                  *MapKey,
     OUT UINTN                  *DescriptorSize,
     OUT UINT32                 *DescriptorVersion
  );

EFI_STATUS
EFIAPI
OrgGetMemoryMap (
  IN OUT UINTN                  *MemoryMapSize,
  IN OUT EFI_MEMORY_DESCRIPTOR  *MemoryMap,
     OUT UINTN                  *MapKey,
     OUT UINTN                  *DescriptorSize,
     OUT UINT32                 *DescriptorVersion
  );

EFI_STATUS
EFIAPI
MOExitBootServices (
  IN EFI_HANDLE  ImageHandle,
  IN UINTN       MapKey
  );

EFI_STATUS
ForceExitBootServices (
  IN EFI_EXIT_BOOT_SERVICES  ExitBs,
  IN EFI_HANDLE              ImageHandle,
  IN UINTN                   MapKey
  );

EFI_STATUS
EFIAPI
MOSetVirtualAddressMap (
  IN UINTN                  MemoryMapSize,
  IN UINTN                  DescriptorSize,
  IN UINT32                 DescriptorVersion,
  IN EFI_MEMORY_DESCRIPTOR  *VirtualMap
  );

#endif // APTIOFIX_SERVICE_OVERRIDES_H
