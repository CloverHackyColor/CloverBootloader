/**

  Methods for setting callback jump from kernel entry point, callback, fixes to kernel boot image.
  
  by dmazar

**/

/* DevTree may contain /chosen/memory-map
 * with properties with values = UINT32 address, UINT32 length:
 * "BootCLUT" = 8bit boot time colour lookup table
 * "Pict-FailedBoot" = picture shown if booting fails
 * "RAMDisk" = ramdisk
 * "Driver-<hex addr of DriverInfo>" = Kext, UINT32 address points to BooterKextFileInfo
 * "DriversPackage-..." = MKext, UINT32 address points to mkext_header (libkern/libkern/mkext.h), UINT32 length
 *
*/
#define BOOTER_KEXT_PREFIX   "Driver-"
#define BOOTER_MKEXT_PREFIX  "DriversPackage-"
#define BOOTER_RAMDISK_PREFIX  "RAMDisk"

/** Struct at the beginning of every loaded kext.
  * Pointers to every loaded kext (to this struct) are
  * properties Driver-<hex addr of DriverInfo> in DevTree /chosen/memory-map
  */
typedef struct _BooterKextFileInfo {
	UINT32	infoDictPhysAddr;
	UINT32	infoDictLength;
	UINT32	executablePhysAddr;
	UINT32	executableLength;
	UINT32  bundlePathPhysAddr;
	UINT32	bundlePathLength;
} BooterKextFileInfo;

struct DTMemMapEntry {
  UINT32	Address;
  UINT32	Length;
};
typedef struct DTMemMapEntry DTMemMapEntry;

typedef struct {
  EFI_PHYSICAL_ADDRESS  PhysicalStart;
  EFI_MEMORY_TYPE       Type;
} RT_RELOC_PROTECT_INFO;

typedef struct {
  UINTN                 NumEntries;
  RT_RELOC_PROTECT_INFO RelocInfo[50]; // You probably want to adapt this.
} RT_RELOC_PROTECT_DATA;



extern EFI_PHYSICAL_ADDRESS gRelocBase;
extern EFI_PHYSICAL_ADDRESS	gSysTableRtArea;
extern BOOLEAN gHibernateWake;
extern UINTN					gLastMemoryMapSize;
extern EFI_MEMORY_DESCRIPTOR	*gLastMemoryMap;
extern UINTN					gLastDescriptorSize;
extern UINT32					gLastDescriptorVersion;

EFI_STATUS PrepareJumpFromKernel(VOID);
EFI_STATUS KernelEntryPatchJump(UINT32 KernelEntry);
EFI_STATUS KernelEntryFromMachOPatchJump(VOID *MachOImage, UINTN SlideAddr);
//EFI_STATUS KernelEntryPatchJumpFill(VOID);
EFI_STATUS KernelEntryPatchHalt(UINT32 KernelEntry);
EFI_STATUS KernelEntryPatchZero(UINT32 KernelEntry);
EFI_STATUS
ExecSetVirtualAddressesToMemMap(
                                IN UINTN			MemoryMapSize,
                                IN UINTN			DescriptorSize,
                                IN UINT32			DescriptorVersion,
                                IN EFI_MEMORY_DESCRIPTOR	*MemoryMap
                                );
VOID
CopyEfiSysTableToSeparateRtDataArea(IN OUT UINT32	*EfiSystemTable);

VOID
ProtectRtDataFromRelocation(
                            IN UINTN		MemoryMapSize,
                            IN UINTN		DescriptorSize,
                            IN UINT32		DescriptorVersion,
                            IN EFI_MEMORY_DESCRIPTOR	*MemoryMap
                            );

VOID
VirtualizeRTShimPointers (UINTN MemoryMapSize, UINTN DescriptorSize, EFI_MEMORY_DESCRIPTOR  *MemoryMap);

VOID
DefragmentRuntimeServices(
						  IN UINTN			MemoryMapSize,
						  IN UINTN			DescriptorSize,
						  IN UINT32			DescriptorVersion,
						  IN EFI_MEMORY_DESCRIPTOR	*MemoryMap,
						  IN OUT UINT32		*EfiSystemTable,
						  IN BOOLEAN		SkipOurSysTableRtArea
						  );
/** Fixes stuff for booting with relocation block. Called when boot.efi jumps to kernel. */
UINTN FixBootingWithRelocBlock(UINTN bootArgs, BOOLEAN ModeX64);

/** Fixes stuff for booting without relocation block. Called when boot.efi jumps to kernel. */
UINTN FixBootingWithoutRelocBlock(UINTN bootArgs, BOOLEAN ModeX64);

/** Fixes stuff for hibernate wake booting without relocation block. Called when boot.efi jumps to kernel. */
UINTN FixHibernateWakeWithoutRelocBlock(UINTN imageHeaderPage, BOOLEAN ModeX64);

