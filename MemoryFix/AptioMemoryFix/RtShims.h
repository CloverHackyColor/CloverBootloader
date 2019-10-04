/**

  Runtime Services Wrappers.

  by Download-Fritz & vit9696

**/

#ifndef APTIOFIX_RT_SHIMS_H
#define APTIOFIX_RT_SHIMS_H

extern VOID *gRtShims;

typedef struct {
  UINTN           *gFunc;
  UINTN           *Func;
  BOOLEAN         Fixed;
} RT_SHIM_PTRS;

typedef struct {
  EFI_PHYSICAL_ADDRESS  PhysicalStart;
  EFI_MEMORY_TYPE       Type;
} RT_RELOC_PROTECT_INFO;

typedef struct {
  UINTN                 NumEntries;
  RT_RELOC_PROTECT_INFO RelocInfo[APTIFIX_MAX_RT_RELOC_NUM];
} RT_RELOC_PROTECT_DATA;

VOID
InstallRtShims (
  EFI_GET_VARIABLE GetVariableOverride
  );

VOID
VirtualizeRtShims (
  UINTN                  MemoryMapSize,
  UINTN                  DescriptorSize,
  EFI_MEMORY_DESCRIPTOR  *MemoryMap
  );

EFI_STATUS
EFIAPI
OrgGetVariable (
  IN     CHAR16    *VariableName,
  IN     EFI_GUID  *VendorGuid,
  OUT    UINT32    *Attributes OPTIONAL,
  IN OUT UINTN     *DataSize,
  OUT    VOID      *Data
  );

VOID
ProtectRtMemoryFromRelocation (
  IN     UINTN                  MemoryMapSize,
  IN     UINTN                  DescriptorSize,
  IN     UINT32                 DescriptorVersion,
  IN OUT EFI_MEMORY_DESCRIPTOR  *MemoryMap,
  IN     EFI_PHYSICAL_ADDRESS   SysTableArea
  );

VOID
RestoreProtectedRtMemoryTypes (
  IN     UINTN                  MemoryMapSize,
  IN     UINTN                  DescriptorSize,
  IN OUT EFI_MEMORY_DESCRIPTOR  *MemoryMap
  );

VOID
SetWriteUnprotectorMode (
  IN     BOOLEAN                Enable
  );

BOOLEAN
EFIAPI
SetBootVariableRedirect (
  IN     BOOLEAN                Enable
  );

#endif // APTIOFIX_RT_SHIMS_H
