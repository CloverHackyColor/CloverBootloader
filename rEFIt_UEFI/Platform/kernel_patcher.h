/*
 * Copyright (c) 2009-2010 Frank peng. All rights reserved.
 *
 */

#ifndef __LIBSAIO_KERNEL_PATCHER_H
#define __LIBSAIO_KERNEL_PATCHER_H

#include "boot.h"

//#define DBG_RT( ...)    if ((KernelAndKextPatches != NULL) && KernelAndKextPatches->KPDebug) { printf(__VA_ARGS__); }
#define DBG_RT(...)    if ( KernelAndKextPatches.KPDebug ) { printf(__VA_ARGS__); }


#define CPUFAMILY_INTEL_6_13		  0xaa33392b
#define CPUFAMILY_INTEL_YONAH		  0x73d67300
#define CPUFAMILY_INTEL_MEROM		  0x426f69ef
#define CPUFAMILY_INTEL_PENRYN		0x78ea4fbc
#define CPUFAMILY_INTEL_NEHALEM		0x6b5a4cd2
#define CPUFAMILY_INTEL_WESTMERE	0x573b5eec

#define CPUIDFAMILY_DEFAULT 6

#define MACH_GET_MAGIC(hdr)        (((struct mach_header_64*)(hdr))->magic)
#define MACH_GET_NCMDS(hdr)        (((struct mach_header_64*)(hdr))->ncmds)
#define MACH_GET_CPU(hdr)          (((struct mach_header_64*)(hdr))->cputype)
#define MACH_GET_FLAGS(hdr)        (((struct mach_header_64*)(hdr))->flags)
#define SC_GET_CMD(hdr)            (((struct segment_command_64*)(hdr))->cmd)


const char   kPrelinkTextSegment[] =                "__PRELINK_TEXT";
const char   kPrelinkTextSection[] =                "__text";
const char   kPrelinkLinkStateSegment[] =           "__PRELINK_STATE";
const char   kPrelinkKernelLinkStateSection[] =     "__kernel";
const char   kPrelinkKextsLinkStateSection[] =      "__kexts";
const char   kPrelinkInfoSegment[] =                "__PRELINK_INFO";
const char   kPrelinkInfoSection[] =                "__info";
const char   kLinkEditSegment[] =                   "__LINKEDIT";
const char   kTextSegment[] =                       "__TEXT";
const char   kDataSegment[] =                       "__DATA";
const char   kDataConstSegment[] =                  "__DATA_CONST";
const char   kKldSegment[] =                        "__KLD";
const char   kHibSegment[] =                        "__HIB";
const char   kTextExecSegment[] =                   "__TEXT_EXEC";
const char   kConstSection[] =                      "__const";
const char   kBssSection[] =                        "__bss";
const char   kCommonSection[] =                     "__common";
const char   kDataSection[] =                       "__data";

#define ID_SEG_STEXT                           0x010e
#define ID_SEG_TEXT                            0x010f
#define ID_SEG_TEXT2                           0x0124

#define ID_SEG_TEXT_CONST                      0x030e
#define ID_SEG_DATA_DATA                       0x080e
#define ID_SEG_DATA_DATA2                      0x081e
#define ID_SEC_BSS                             0x0a0e
#define ID_SEC_CONST                           0x070f
#define ID_SEG_DATA_COMMON                     0x090f
#define ID_SEG_DATA                            0x0f0f
#define ID_SEG_DATA_CONST                      0x110f
#define ID_SEG_HIB                             0x170f
#define ID_SEG_KLD                             0x180f
#define ID_SEG_KLD2                            0x1a0f
#define ID_SEG_KLD3                            0x210f


const char  ctor_used[] =                           ".constructors_used";
const char  kPrelinkBundlePathKey[] =               "_PrelinkBundlePath";
const char  kPrelinkExecutableRelativePathKey[] =   "_PrelinkExecutableRelativePath";
const char  kPrelinkExecutableLoadKey[] =           "_PrelinkExecutableLoadAddr";
const char  kPrelinkExecutableSourceKey[] =         "_PrelinkExecutableSourceAddr";
const char  kPrelinkExecutableSizeKey[] =           "_PrelinkExecutableSize";
const char  kPrelinkInfoDictionaryKey[] =           "_PrelinkInfoDictionary";
const char  kPrelinkInterfaceUUIDKey[] =            "_PrelinkInterfaceUUID";
const char  kPrelinkKmodInfoKey[] =                 "_PrelinkKmodInfo";
const char  kPrelinkLinkStateKey[] =                "_PrelinkLinkState";
const char  kPrelinkLinkStateSizeKey[] =            "_PrelinkLinkStateSize";

#define kPropCFBundleIdentifier ("CFBundleIdentifier")
#define kPropCFBundleExecutable ("CFBundleExecutable")
#define kPropOSBundleRequired   ("OSBundleRequired")
#define kPropOSBundleLibraries  ("OSBundleLibraries")
#define kPropIOKitPersonalities ("IOKitPersonalities")
#define kPropIONameMatch        ("IONameMatch")

typedef struct _BooterKextFileInfo {
    UINT32  infoDictPhysAddr;
    UINT32  infoDictLength;
    UINT32  executablePhysAddr;
    UINT32  executableLength;
    UINT32  bundlePathPhysAddr;
    UINT32  bundlePathLength;
} _BooterKextFileInfo;

class _DeviceTreeBuffer
{
  public:
    uint32_t paddr = 0;
    uint32_t length = 0;

    _DeviceTreeBuffer() { }
    _DeviceTreeBuffer(const _DeviceTreeBuffer& other) = default; // default is fine if there is only native type and objects that have copy ctor
    _DeviceTreeBuffer& operator = ( const _DeviceTreeBuffer & ) = default; // default is fine if there is only native type and objects that have copy ctor
    ~_DeviceTreeBuffer() {}
};

typedef struct VTABLE {
  UINT32 NameOffset;
  UINT32 Seg;
  UINT64 ProcAddr;
} VTABLE;

typedef struct SEGMENT {
  CHAR8  Name[16];    //0
  UINT64 SegAddress;  //0x10
  UINT64 vmsize;      //0x18 0x16FB60
  UINT32 fileoff;     //0x20 0xDDA000 //Slice - it is not UINT64. MachoLib is wrong
  UINT32 fileoff64;   //0x24
  UINT32 filesize;    //0x28 0x16FB60
  UINT32 filesize64;  //0x2c
  UINT32 maxprot;     //0x30 01-Cat 07-Moj
  UINT32 initprot;    //0x34 01
  UINT32 NumSects;    //0x38 00
  UINT32 Flags;       //0x3C 00
  UINT32 Cmd;         //0x40 02  //LC_SYMTAB link-edit stab symbol table info
  UINT32 Cmdsize;     //0x44 18
  UINT32 AddrVtable;  //0x48
  UINT32 SizeVtable;  //0x4C
  UINT32 AddrNames;   //0x50
  UINT32 SizeNamesTable;
} SEGMENT;

//extern EFI_PHYSICAL_ADDRESS KernelRelocBase;
//extern BootArgs1    *bootArgs1;
//extern BootArgs2    *bootArgs2;
//extern CHAR8        *dtRoot;
//extern UINT32       *dtLength;
//extern UINT8        *KernelData;
//extern UINT32       KernelSlide;
//extern BOOLEAN      isKernelcache;
//extern BOOLEAN      is64BitKernel;
//extern BOOLEAN      gSNBEAICPUFixRequire; // SandyBridge-E AppleIntelCpuPowerManagement patch require or not
//extern BOOLEAN      gBDWEIOPCIFixRequire; // Broadwell-E IOPCIFamily fix require or not


//extern UINT32       DisplayVendor[];
//void findCPUfamily();

//extern BOOLEAN                         SSSE3;


//UINT64 kernelsize;

//void Patcher_SSE3_5(void* kernelData);
//void Patcher_SSE3_6(void* kernelData);
//void Patcher_SSE3_7();

//#include "../gui/menu_items/menu_items.h" // for LOADER_ENTRY
//class LOADER_ENTRY;
//void KernelAndKextsPatcherStart(IN LOADER_ENTRY *Entry);

//void register_kernel_symbol(CONST CHAR8* name);
//UINT64 symbol_handler(CHAR8* symbolName, UINT64 addr);
//INTN locate_symbols(void* kernelData);


/////////////////////
//
// kext_patcher.c
//

//
// Called from SetFSInjection(), before boot.efi is started,
// to allow patchers to prepare FSInject to force load needed kexts.
//
//void KextPatcherRegisterKexts(FSINJECTION_PROTOCOL *FSInject, FSI_STRING_LIST *ForceLoadKexts, LOADER_ENTRY *Entry);

//
// Entry for all kext patches.
// Will iterate through kext in prelinked kernel (kernelcache)
// or DevTree (drivers boot) and do patches.
//
//void KextPatcherStart(LOADER_ENTRY *Entry);

UINTN FindRelative32(const UINT8 *Source, UINTN Start, UINTN SourceSize, UINTN taskLocation);


#endif /* !__LIBSAIO_KERNEL_PATCHER_H */
