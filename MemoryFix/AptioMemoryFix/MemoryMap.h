/**

  MemoryMap helper functions.

  by dmazar

**/

#ifndef APTIOFIX_MEMORY_MAP_H
#define APTIOFIX_MEMORY_MAP_H

/** MemMap reversed scan */
#define PREV_MEMORY_DESCRIPTOR(MemoryDescriptor, Size) \
  ((EFI_MEMORY_DESCRIPTOR *)((UINT8 *)(MemoryDescriptor) - (Size)))

/** Shrinks mem map by joining non-runtime records. */
VOID
ShrinkMemMap (
  IN OUT UINTN                  *MemoryMapSize,
  IN OUT EFI_MEMORY_DESCRIPTOR  *MemoryMap,
  IN     UINTN                  DescriptorSize
  );

/** Protects AMI CSM region from being overwritten by the kernel. */
VOID
ProtectCsmRegion (
  UINTN                  MemoryMapSize,
  EFI_MEMORY_DESCRIPTOR  *MemoryMap,
  UINTN                  DescriptorSize
  );

/** Prints mem map. */
VOID
PrintMemMap (
  IN CONST CHAR16           *Name,
  IN UINTN                  MemoryMapSize,
  IN UINTN                  DescriptorSize,
  IN EFI_MEMORY_DESCRIPTOR  *MemoryMap,
  IN VOID                   *Shims,
  IN EFI_PHYSICAL_ADDRESS   SysTable
  );

/** Helper function that calls GetMemoryMap(), allocates space for mem map and returns it. */
EFI_STATUS
GetMemoryMapAlloc (
  IN OUT UINTN                  *AllocatedTopPages,
     OUT UINTN                  *MemoryMapSize,
     OUT EFI_MEMORY_DESCRIPTOR  **MemoryMap,
     OUT UINTN                  *MapKey,
     OUT UINTN                  *DescriptorSize,
     OUT UINT32                 *DescriptorVersion
  );

/** Alloctes pages from the top of mem, up to address specified in Memory. Returns allocated address in Memory. */
EFI_STATUS
AllocatePagesFromTop (
  IN     EFI_MEMORY_TYPE       MemoryType,
  IN     UINTN                 Pages,
  IN OUT EFI_PHYSICAL_ADDRESS  *Memory,
  IN     BOOLEAN               CheckRange
  );

#endif // APTIOFIX_MEMORY_MAP_H
