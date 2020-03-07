/*
 * Original idea of patching kernel by Evan Lojewsky, 2009
 *
 * Copyright (c) 2011-2012 Frank Peng. All rights reserved.
 *
 * Correction and improvements by Clover team
 */

#include "Platform.h"
#include "LoaderUefi.h"
//#include "device_tree.h"

#include "kernel_patcher.h"
#include "sse3_patcher.h"
#include "sse3_5_patcher.h"

#define KERNEL_DEBUG 0

#if KERNEL_DEBUG
#define DBG(...)    AsciiPrint(__VA_ARGS__);
#else
#define DBG(...)
#endif

// runtime debug
#define DBG_RT(entry, ...)    if ((entry != NULL) && (entry->KernelAndKextPatches != NULL) && entry->KernelAndKextPatches->KPDebug) { AsciiPrint(__VA_ARGS__); }


EFI_PHYSICAL_ADDRESS    KernelRelocBase = 0;
BootArgs1   *bootArgs1 = NULL;
BootArgs2   *bootArgs2 = NULL;
CHAR8       *dtRoot = NULL;
UINT32      *dtLength;
UINT8       *KernelData = NULL;
UINT32      KernelSlide = 0;
BOOLEAN     isKernelcache = FALSE;
BOOLEAN     is64BitKernel = FALSE;
BOOLEAN     SSSE3;

BOOLEAN     PatcherInited = FALSE;
BOOLEAN     gSNBEAICPUFixRequire = FALSE; // SandyBridge-E AppleIntelCpuPowerManagement patch require or not
BOOLEAN     gBDWEIOPCIFixRequire = FALSE; // Broadwell-E IOPCIFamily fix require or not

// notes:
// - 64bit segCmd64->vmaddr is 0xffffff80xxxxxxxx and we are taking
//   only lower 32bit part into PrelinkTextAddr
// - PrelinkTextAddr is segCmd64->vmaddr + KernelRelocBase
UINT32     PrelinkTextLoadCmdAddr = 0;
UINT32     PrelinkTextAddr = 0;
UINT32     PrelinkTextSize = 0;

// notes:
// - 64bit sect->addr is 0xffffff80xxxxxxxx and we are taking
//   only lower 32bit part into PrelinkInfoAddr
// - PrelinkInfoAddr is sect->addr + KernelRelocBase
UINT32     PrelinkInfoLoadCmdAddr = 0;
UINT32     PrelinkInfoAddr = 0;
UINT32     PrelinkInfoSize = 0;


VOID SetKernelRelocBase()
{
//  EFI_STATUS      Status;
  UINTN           DataSize = sizeof(KernelRelocBase);

  KernelRelocBase = 0;
  // OsxAptioFixDrv will set this
  /*Status = */gRT->GetVariable(L"OsxAptioFixDrv-RelocBase", &gEfiAppleBootGuid, NULL, &DataSize, &KernelRelocBase);
  DeleteNvramVariable(L"OsxAptioFixDrv-RelocBase", &gEfiAppleBootGuid); // clean up the temporary variable
  // KernelRelocBase is now either read or 0
  return;
}

//TimeWalker - extended and corrected for systems up to Yosemite
VOID KernelPatcher_64(VOID* kernelData, LOADER_ENTRY *Entry)
{
  UINT8       *bytes = (UINT8*)kernelData;
  UINT32      patchLocation=0, patchLocation1=0;
  UINT32      i;
  UINT32      switchaddr=0;
  UINT32      mask_family=0, mask_model=0;
  UINT32      cpuid_family_addr=0, cpuid_model_addr=0;
  UINT64      os_version;

  DBG_RT(Entry, "\nLooking for _cpuid_set_info _panic ...\n");

  // Determine location of _cpuid_set_info _panic call for reference
  // basically looking for info_p->cpuid_model = bitfield32(reg[eax],  7,  4);
  for (i=0; i<0x1000000; i++) {
    if (bytes[i+ 0] == 0xC7 && bytes[i+ 1] == 0x05 && bytes[i+ 5] == 0x00 &&
        bytes[i+ 6] == 0x07 && bytes[i+ 7] == 0x00 && bytes[i+ 8] == 0x00 && bytes[i+ 9] == 0x00 &&
        bytes[i-5] == 0xE8) { // matching 0xE8 for _panic call start
      patchLocation = i-5;
      break;
    }
  }

  if (!patchLocation) {
    DBG_RT(Entry, "_cpuid_set_info Unsupported CPU _panic not found \n");
    return;
  }

  os_version = AsciiOSVersionToUint64(Entry->OSVersion);

  // make sure only kernels for OSX 10.6.0 to 10.7.3 are being patched by this approach
  if (os_version >= AsciiOSVersionToUint64("10.6") && os_version <= AsciiOSVersionToUint64("10.7.3")) {

    DBG_RT(Entry, "will patch kernel for macOS 10.6.0 to 10.7.3\n");

    // remove tsc_init: unknown CPU family panic for kernels prior to 10.6.2 which still had Atom support
    if (os_version < AsciiOSVersionToUint64("10.6.2")) {
      for (i=0; i<0x1000000; i++) {
        // find _tsc_init panic address by byte sequence 488d3df4632a00
        if (bytes[i] == 0x48 && bytes[i+1] == 0x8D && bytes[i+2] == 0x3D && bytes[i+3] == 0xF4 &&
            bytes[i+4] == 0x63 && bytes[i+5] == 0x2A && bytes[i+6] == 0x00) {
          patchLocation1 = i+9;
          DBG_RT(Entry, "Found _tsc_init _panic address at 0x%08x\n",patchLocation1);
          break;
        }
      }

      // NOP _panic call
      if (patchLocation1) {
        bytes[patchLocation1 + 0] = 0x90;
        bytes[patchLocation1 + 1] = 0x90;
        bytes[patchLocation1 + 2] = 0x90;
        bytes[patchLocation1 + 3] = 0x90;
        bytes[patchLocation1 + 4] = 0x90;
      }
    }
    else { // assume patching logic for OSX 10.6.2 to 10.7.3

      /*
       Here is our case from CPUID switch statement, it sets CPUFAMILY_UNKNOWN
       C7051C2C5F0000000000   mov     dword [ds:0xffffff80008a22c0], 0x0 (example from 10.7)
       */
      switchaddr = patchLocation - 19;
      DBG_RT(Entry, "switch statement patch location is 0x%08x\n", (switchaddr+6));

      if (bytes[switchaddr + 0] == 0xC7 && bytes[switchaddr + 1] == 0x05 &&
          bytes[switchaddr + 5] == 0x00 && bytes[switchaddr + 6] == 0x00 &&
          bytes[switchaddr + 7] == 0x00 && bytes[switchaddr + 8] == 0x00) {

        // Determine cpuid_family address from above mov operation
        cpuid_family_addr =
        bytes[switchaddr + 2] <<  0 |
        bytes[switchaddr + 3] <<  8 |
        bytes[switchaddr + 4] << 16 |
        bytes[switchaddr + 5] << 24;
        cpuid_family_addr = cpuid_family_addr + (switchaddr + 10);

        if (cpuid_family_addr) {

          // Determine cpuid_model address
          // for 10.6.2 kernels it's offset by 299 bytes from cpuid_family address
          if (os_version ==  AsciiOSVersionToUint64("10.6.2")) {
            cpuid_model_addr = cpuid_family_addr - 0X12B;
          }
          // for 10.6.3 to 10.6.7 it's offset by 303 bytes
          else if (os_version <= AsciiOSVersionToUint64("10.6.7")) {
            cpuid_model_addr = cpuid_family_addr - 0X12F;
          }
          // for 10.6.8 to 10.7.3 kernels - by 339 bytes
          else {
            cpuid_model_addr = cpuid_family_addr - 0X153;
          }

          DBG_RT(Entry, "cpuid_family address: 0x%08x\n", cpuid_family_addr);
          DBG_RT(Entry, "cpuid_model address: 0x%08x\n",  cpuid_model_addr);

          switchaddr += 6; // offset 6 bytes in mov operation to write a dword instead of zero

          // calculate mask for patching, cpuid_family mask not needed as we offset on a valid mask
          mask_model   = cpuid_model_addr - (switchaddr+14);
          DBG_RT(Entry, "model mask 0x%08x\n", mask_model);

          DBG_RT(Entry, "overriding cpuid_family and cpuid_model as CPUID_INTEL_PENRYN\n");
          bytes[switchaddr+0] = (CPUFAMILY_INTEL_PENRYN & 0x000000FF) >>  0;
          bytes[switchaddr+1] = (CPUFAMILY_INTEL_PENRYN & 0x0000FF00) >>  8;
          bytes[switchaddr+2] = (CPUFAMILY_INTEL_PENRYN & 0x00FF0000) >> 16;
          bytes[switchaddr+3] = (CPUFAMILY_INTEL_PENRYN & 0xFF000000) >> 24;

          // mov  dword [ds:0xffffff80008a216d], 0x2000117
          bytes[switchaddr+4] = 0xC7;
          bytes[switchaddr+5] = 0x05;
          bytes[switchaddr+6] = (UINT8)((mask_model & 0x000000FF) >> 0);
          bytes[switchaddr+7] = (UINT8)((mask_model & 0x0000FF00) >> 8);
          bytes[switchaddr+8] = (UINT8)((mask_model & 0x00FF0000) >> 16);
          bytes[switchaddr+9] = (UINT8)((mask_model & 0xFF000000) >> 24);
          bytes[switchaddr+10] = 0x17; // cpuid_model (Penryn)
          bytes[switchaddr+11] = 0x01; // cpuid_extmodel
          bytes[switchaddr+12] = 0x00; // cpuid_extfamily
          bytes[switchaddr+13] = 0x02; // cpuid_stepping

          // fill remainder with 4 NOPs
          for (i=14; i<18; i++) {
            bytes[switchaddr+i] = 0x90;
          }
        }
      }
      else {
        DBG_RT(Entry, "Unable to determine cpuid_family address, patching aborted\n");
        return;
      }
    }

    // patch ssse3
    if (!SSSE3 && (AsciiStrnCmp(Entry->OSVersion,"10.6",4)==0)) {
      Patcher_SSE3_6((VOID*)bytes);
    }
    if (!SSSE3 && (AsciiStrnCmp(Entry->OSVersion,"10.7",4)==0)) {
      Patcher_SSE3_7((VOID*)bytes);
    }
  }

  // all 10.7.4+ kernels share common CPUID switch statement logic,
  // it needs to be exploited in diff manner due to the lack of space
  else if (os_version >= AsciiOSVersionToUint64("10.7.4")) {

    DBG_RT(Entry, "will patch kernel for macOS 10.7.4+\n");

    /*
     Here is our switchaddress location ... it should be case 20 from CPUID switch statement
     833D78945F0000  cmp        dword [ds:0xffffff80008a21d0], 0x0;
     7417            je         0xffffff80002a8d71
     */
    switchaddr = patchLocation-45;
    DBG_RT(Entry, "switch statement patch location is 0x%08x\n", switchaddr);

    if(bytes[switchaddr + 0] == 0x83 && bytes[switchaddr + 1] == 0x3D &&
       bytes[switchaddr + 5] == 0x00 && bytes[switchaddr + 6] == 0x00 &&
       bytes[switchaddr + 7] == 0x74) {

      // Determine cpuid_family address
      // 891D4F945F00    mov        dword [ds:0xffffff80008a21a0], ebx
      cpuid_family_addr =
      bytes[switchaddr - 4] <<  0 |
      bytes[switchaddr - 3] <<  8 |
      bytes[switchaddr - 2] << 16 |
      bytes[switchaddr - 1] << 24;
      cpuid_family_addr = cpuid_family_addr + switchaddr;

      if (cpuid_family_addr) {

        // Determine cpuid_model address
        // for 10.6.8+ kernels it's 339 bytes apart from cpuid_family address
        cpuid_model_addr = cpuid_family_addr - 0X153;

        DBG_RT(Entry, "cpuid_family address: 0x%08x\n", cpuid_family_addr);
        DBG_RT(Entry, "cpuid_model address: 0x%08x\n",  cpuid_model_addr);

        // Calculate masks for patching
        mask_family  = cpuid_family_addr - (switchaddr +15);
        mask_model   = cpuid_model_addr -  (switchaddr +25);
        DBG_RT(Entry, "\nfamily mask: 0x%08x \nmodel mask: 0x%08x\n", mask_family, mask_model);

        // retain original
        // test ebx, ebx
        bytes[switchaddr+0] = bytes[patchLocation-13];
        bytes[switchaddr+1] = bytes[patchLocation-12];
        // retain original, but move jump offset by 20 bytes forward
        // jne for above test
        bytes[switchaddr+2] = bytes[patchLocation-11];
        bytes[switchaddr+3] = bytes[patchLocation-10]+0x20;
        // mov ebx, 0x78ea4fbc
        bytes[switchaddr+4] = 0xBB;
        bytes[switchaddr+5] = (CPUFAMILY_INTEL_PENRYN & 0x000000FF) >>  0;
        bytes[switchaddr+6] = (CPUFAMILY_INTEL_PENRYN & 0x0000FF00) >>  8;
        bytes[switchaddr+7] = (CPUFAMILY_INTEL_PENRYN & 0x00FF0000) >> 16;
        bytes[switchaddr+8] = (CPUFAMILY_INTEL_PENRYN & 0xFF000000) >> 24;

        // mov dword, ebx
        bytes[switchaddr+9]  = 0x89;
        bytes[switchaddr+10] = 0x1D;
        // cpuid_cpufamily address 0xffffff80008a21a0
        bytes[switchaddr+11] = (UINT8)((mask_family & 0x000000FF) >> 0);
        bytes[switchaddr+12] = (UINT8)((mask_family & 0x0000FF00) >> 8);
        bytes[switchaddr+13] = (UINT8)((mask_family & 0x00FF0000) >> 16);
        bytes[switchaddr+14] = (UINT8)((mask_family & 0xFF000000) >> 24);

        // mov dword
        bytes[switchaddr+15] = 0xC7;
        bytes[switchaddr+16] = 0x05;
        // cpuid_model address 0xffffff80008b204d
        bytes[switchaddr+17] = (UINT8)((mask_model & 0x000000FF) >> 0);
        bytes[switchaddr+18] = (UINT8)((mask_model & 0x0000FF00) >> 8);
        bytes[switchaddr+19] = (UINT8)((mask_model & 0x00FF0000) >> 16);
        bytes[switchaddr+20] = (UINT8)((mask_model & 0xFF000000) >> 24);

        bytes[switchaddr+21] = 0x17; // cpuid_model
        bytes[switchaddr+22] = 0x01; // cpuid_extmodel
        bytes[switchaddr+23] = 0x00; // cpuid_extfamily
        bytes[switchaddr+24] = 0x02; // cpuid_stepping

        // fill remainder with 25 NOPs
        for (i=25; i<25+25; i++) {
          bytes[switchaddr+i] = 0x90;
        }
      }
    }
    else {
      DBG_RT(Entry, "Unable to determine cpuid_family address, patching aborted\n");
      return;
    }
  }
}

VOID KernelPatcher_32(VOID* kernelData, CHAR8 *OSVersion)
{
  UINT8* bytes = (UINT8*)kernelData;
  UINT32 patchLocation=0, patchLocation1=0;
  UINT32 i;
  UINT32 jumpaddr;

  DBG("Found _cpuid_set_info _panic Start\n");
  // _cpuid_set_info _panic address
  for (i=0; i<0x1000000; i++) {
    if (bytes[i] == 0xC7 && bytes[i+1] == 0x05 && bytes[i+6] == 0x07 && bytes[i+7] == 0x00 &&
        bytes[i+8] == 0x00 && bytes[i+9] == 0x00 && bytes[i+10] == 0xC7 && bytes[i+11] == 0x05 &&
        bytes[i-5] == 0xE8) {
      patchLocation = i-5;
      DBG("Found _cpuid_set_info _panic address at 0x%08x\n",patchLocation);
      break;
    }
  }

  if (!patchLocation) {
    DBG("Can't find _cpuid_set_info _panic address, patch kernel abort.\n",i);
    return;
  }

  // this for 10.6.0 and 10.6.1 kernel and remove tsc.c unknow cpufamily panic
  //  c70424540e5900
  // find _tsc_init panic address
  for (i=0; i<0x1000000; i++) {
    // _cpuid_set_info _panic address
    if (bytes[i] == 0xC7 && bytes[i+1] == 0x04 && bytes[i+2] == 0x24 &&
        bytes[i+3] == 0x54 && bytes[i+4] == 0x0E && bytes[i+5] == 0x59 &&
        bytes[i+6] == 0x00) {
      patchLocation1 = i+7;
      DBG("Found _tsc_init _panic address at 0x%08x\n",patchLocation1);
      break;
    }
  }

  // found _tsc_init panic addres and patch it
  if (patchLocation1) {
    bytes[patchLocation1 + 0] = 0x90;
    bytes[patchLocation1 + 1] = 0x90;
    bytes[patchLocation1 + 2] = 0x90;
    bytes[patchLocation1 + 3] = 0x90;
    bytes[patchLocation1 + 4] = 0x90;
  }
  // end tsc.c panic

  //first move panic code total 5 bytes, if patch cpuid fail still can boot with kernel
  bytes[patchLocation + 0] = 0x90;
  bytes[patchLocation + 1] = 0x90;
  bytes[patchLocation + 2] = 0x90;
  bytes[patchLocation + 3] = 0x90;
  bytes[patchLocation + 4] = 0x90;

  jumpaddr = patchLocation;

  for (i=0;i<500;i++) {
    if (bytes[jumpaddr-i-3] == 0x85 && bytes[jumpaddr-i-2] == 0xC0 &&
        bytes[jumpaddr-i-1] == 0x75 ) {
      jumpaddr -= i;
      bytes[jumpaddr-1] = 0x77;
      if(bytes[patchLocation - 17] == 0xC7)
        bytes[jumpaddr] -=10;

      break;
    }
  }

  if (jumpaddr == patchLocation) {
    DBG("Can't Found jumpaddr address.\n");
    return;  //can't find jump location
  }
  // patch info_p->cpufamily to CPUFAMILY_INTEL_MEROM

  if (bytes[patchLocation - 17] == 0xC7) {
    bytes[patchLocation - 11] = (CPUFAMILY_INTEL_MEROM & 0x000000FF) >>  0;
    bytes[patchLocation - 10] = (CPUFAMILY_INTEL_MEROM & 0x0000FF00) >>  8;
    bytes[patchLocation -  9] = (CPUFAMILY_INTEL_MEROM & 0x00FF0000) >> 16;
    bytes[patchLocation -  8] = (CPUFAMILY_INTEL_MEROM & 0xFF000000) >> 24;
  }

  //patch info->cpuid_cpufamily
  bytes[patchLocation -  7] = 0xC7;
  bytes[patchLocation -  6] = 0x05;
  bytes[patchLocation -  5] = bytes[jumpaddr + 3];
  bytes[patchLocation -  4] = bytes[jumpaddr + 4];
  bytes[patchLocation -  3] = bytes[jumpaddr + 5];
  bytes[patchLocation -  2] = bytes[jumpaddr + 6];

  bytes[patchLocation -  1] = CPUIDFAMILY_DEFAULT; //cpuid_family  need alway set 0x06
  bytes[patchLocation +  0] = CPUID_MODEL_MEROM;   //cpuid_model set CPUID_MODEL_MEROM
  bytes[patchLocation +  1] = 0x01;                //cpuid_extmodel alway set 0x01
  bytes[patchLocation +  2] = 0x00;                //cpuid_extfamily alway set 0x00
  bytes[patchLocation +  3] = 0x90;
  bytes[patchLocation +  4] = 0x90;

  if (OSVersion) {
    if (AsciiStrnCmp(OSVersion,"10.7",4)==0) return;

    if (!SSSE3 && (AsciiStrnCmp(OSVersion,"10.6",4)==0)) {
      Patcher_SSE3_6((VOID*)bytes);
    }
    if (!SSSE3 && (AsciiStrnCmp(OSVersion,"10.5",4)==0)) {
      Patcher_SSE3_5((VOID*)bytes);
    }
  }
}

//Slice - FakeCPUID substitution, (c)2014

//procedure location
STATIC UINT8 StrCpuid1_tigLeo[]  = {0xb9, 0x01, 0x00, 0x00, 0x00, 0x89, 0xc8, 0x0f, 0xa2};
STATIC UINT8 StrCpuid1_snowLeo[] = {0xb8, 0x01, 0x00, 0x00, 0x00, 0x31, 0xdb, 0x89, 0xd9, 0x89, 0xda, 0x0f, 0xa2};
STATIC UINT8 StrMsr8b[]          = {0xb9, 0x8b, 0x00, 0x00, 0x00, 0x0f, 0x32};

// Tiger/Leopard/Snow Leopard
/*
 This patch searches
  and eax, 0xf0   ||    and eax, 0x0f0000
  shr eax, 0x04   ||    shr eax, 0x10
 and replaces to
  mov eax, FakeModel  | mov eax, FakeExt
 */
STATIC UINT8 TigLeoSLSearchModel[]  = {0x25, 0xf0, 0x00, 0x00, 0x00, 0xc1, 0xe8, 0x04};
STATIC UINT8 TigLeoSLSearchExt[]    = {0x25, 0x00, 0x00, 0x0f, 0x00, 0xc1, 0xe8, 0x10};
STATIC UINT8 TigLeoSLReplaceModel[] = {0xb8, 0x07, 0x00, 0x00, 0x00, 0x90, 0x90, 0x90};

// Lion
/*
 This patch searches
  mov ecx, eax
  shr ecx, 0x04   ||  shr ecx, 0x10
 and replaces to
  mov ecx, FakeModel  || mov ecx, FakeExt
 */
STATIC UINT8 LionSearchModel[]  = {0x89, 0xc1, 0xc1, 0xe9, 0x04};
STATIC UINT8 LionSearchExt[]    = {0x89, 0xc1, 0xc1, 0xe9, 0x10};
STATIC UINT8 LionReplaceModel[] = {0xb9, 0x07, 0x00, 0x00, 0x00};

// Mountain Lion/Mavericks
/*
 This patch searches
  mov bl, al     ||   shr eax, 0x10
  shr bl, 0x04   ||   and al,0x0f
 and replaces to
  mov ebx, FakeModel || mov eax, FakeExt
 */
STATIC UINT8 MLMavSearchModel[]   = {0x88, 0xc3, 0xc0, 0xeb, 0x04};
STATIC UINT8 MLMavSearchExt[]     = {0xc1, 0xe8, 0x10, 0x24, 0x0f};
STATIC UINT8 MLMavReplaceModel[]  = {0xbb, 0x0a, 0x00, 0x00, 0x00};
STATIC UINT8 MLMavReplaceExt[]    = {0xb8, 0x02, 0x00, 0x00, 0x00};

// Yosemite/El Capitan/Sierra
/*
 This patch searches
  mov cl, al     ||   mov ecx, eax
  shr cl, 0x04   ||   shr ecx, 0x10
 and replaces to
  mov ecx, FakeModel || mov ecx, FakeExt
 */
STATIC UINT8 YosECSieSearchModel[]   = {0x88, 0xc1, 0xc0, 0xe9, 0x04};
STATIC UINT8 YosECSieSearchExt[]     = {0x89, 0xc1, 0xc1, 0xe9, 0x10};
// Need to use LionReplaceModel

// High Sierra/Mojave
/*
 This patch searches
  mov ecx, ecx   ||   mov ecx, eax
  shr cl, 0x04   ||   shr ecx, 0x10
 and replaces to
  mov ecx, FakeModel || mov ecx, FakeExt
 */
STATIC UINT8 HSieMojSearchModel[]   = {0x89, 0xc1, 0xc0, 0xe9, 0x04};
// Need to use YosECSieSearchExt, LionReplaceModel

// Catalina
/*
 This patch searches
  mov eax, r12   ||   mov eax, r12
  shr al, 0x4    ||   shr eax, 0x10
 and replaces to
  mov eax, FakeModel || mov eax, FakeExt
  nop                || nop
*/
STATIC UINT8 CataSearchModel[]      = {0x44, 0x89, 0xE0, 0xC0, 0xE8, 0x04};
STATIC UINT8 CataSearchExt[]        = {0x44, 0x89, 0xE0, 0xC1, 0xE8, 0x10};
STATIC UINT8 CataReplaceMovEax[]    = {0xB8, 0x00, 0x00, 0x00, 0x00, 0x90}; // mov eax, val || nop

BOOLEAN PatchCPUID(UINT8* bytes, UINT8* Location, INT32 LenLoc,
                   UINT8* Search4, UINT8* Search10, UINT8* ReplaceModel,
                   UINT8* ReplaceExt, INT32 Len, LOADER_ENTRY *Entry)
{
  INT32 patchLocation=0, patchLocation1=0;
  INT32 Adr = 0, Num;
  BOOLEAN Patched = FALSE;
  UINT8 FakeModel = (Entry->KernelAndKextPatches->FakeCPUID >> 4) & 0x0f;
  UINT8 FakeExt = (Entry->KernelAndKextPatches->FakeCPUID >> 0x10) & 0x0f;
  for (Num = 0; Num < 2; Num++) {
    Adr = FindBin(&bytes[Adr], 0x800000 - Adr, Location, LenLoc);
    if (Adr < 0) {
      break;
    }
    DBG_RT(Entry, "found location at %x\n", Adr);
    patchLocation = FindBin(&bytes[Adr], 0x100, Search4, Len);
    if (patchLocation > 0 && patchLocation < 70) {
      //found
      DBG_RT(Entry, "found Model location at %x\n", Adr + patchLocation);
      CopyMem(&bytes[Adr + patchLocation], ReplaceModel, Len);
      bytes[Adr + patchLocation + 1] = FakeModel;
      patchLocation1 = FindBin(&bytes[Adr], 0x100, Search10, Len);
      if (patchLocation1 > 0 && patchLocation1 < 100) {
        DBG_RT(Entry, "found ExtModel location at %x\n", Adr + patchLocation1);
        CopyMem(&bytes[Adr + patchLocation1], ReplaceExt, Len);
        bytes[Adr + patchLocation1 + 1] = FakeExt;
      }
      Patched = TRUE;
    }
  }
  return Patched;
}

VOID KernelCPUIDPatch(UINT8* kernelData, LOADER_ENTRY *Entry)
{
// Tiger/Leopard patterns
  DBG_RT(Entry, "CPUID: try Tiger/Leopard patch...\n");
  if (PatchCPUID(kernelData, &StrCpuid1_tigLeo[0], sizeof(StrCpuid1_tigLeo), &TigLeoSLSearchModel[0],
                 &TigLeoSLSearchExt[0], &TigLeoSLReplaceModel[0], &TigLeoSLReplaceModel[0],
                 sizeof(TigLeoSLSearchModel), Entry)) {
    DBG_RT(Entry, "...done!\n");
    return;
  }
// Snow Leopard patterns
  DBG_RT(Entry, "CPUID: try Snow Leopard patch...\n");
  if (PatchCPUID(kernelData, &StrCpuid1_snowLeo[0], sizeof(StrCpuid1_snowLeo), &TigLeoSLSearchModel[0],
                 &TigLeoSLSearchExt[0], &TigLeoSLReplaceModel[0], &TigLeoSLReplaceModel[0],
                 sizeof(TigLeoSLSearchModel), Entry)) {
    DBG_RT(Entry, "...done!\n");
    return;
  }
// Lion patterns
  DBG_RT(Entry, "CPUID: try Lion patch...\n");
  if (PatchCPUID(kernelData, &StrMsr8b[0], sizeof(StrMsr8b), &LionSearchModel[0],
                 &LionSearchExt[0], &LionReplaceModel[0], &LionReplaceModel[0],
                 sizeof(LionSearchModel), Entry)) {
    DBG_RT(Entry, "...done!\n");
    return;
  }
// Mountain Lion/Mavericks patterns
  DBG_RT(Entry, "CPUID: try Mountain Lion/Mavericks patch...\n");
  if (PatchCPUID(kernelData, &StrMsr8b[0], sizeof(StrMsr8b), &MLMavSearchModel[0],
                 &MLMavSearchExt[0], &MLMavReplaceModel[0], &MLMavReplaceExt[0],
                 sizeof(MLMavSearchModel), Entry)) {
    DBG_RT(Entry, "...done!\n");
    return;
  }
// Yosemite/El Capitan/Sierra patterns
  DBG_RT(Entry, "CPUID: try Yosemite/El Capitan/Sierra patch...\n");
  if (PatchCPUID(kernelData, &StrMsr8b[0], sizeof(StrMsr8b), &YosECSieSearchModel[0],
                 &YosECSieSearchExt[0], &LionReplaceModel[0], &LionReplaceModel[0],
                 sizeof(YosECSieSearchModel), Entry)) {
    DBG_RT(Entry, "...done!\n");
    return;
  }
// High Sierra/Mojave patterns
// Sherlocks: 10.13/10.14
  DBG_RT(Entry, "CPUID: try High Sierra/Mojave patch...\n");
  if (PatchCPUID(kernelData, &StrMsr8b[0], sizeof(StrMsr8b), &HSieMojSearchModel[0],
                 &YosECSieSearchExt[0], &LionReplaceModel[0], &LionReplaceModel[0],
                 sizeof(HSieMojSearchModel), Entry)) {
    DBG_RT(Entry, "...done!\n");
    return;
  }
// Catalina patterns
// PMheart: 10.15.DP1
  DBG_RT(Entry, "CPUID: try Catalina patch...\n");
  if (PatchCPUID(kernelData, &StrMsr8b[0], sizeof(StrMsr8b), &CataSearchModel[0],
                 &CataSearchExt[0], &CataReplaceMovEax[0], &CataReplaceMovEax[0],
                 sizeof(CataSearchModel), Entry)) {
    DBG_RT(Entry, "...done!\n");
    return;
  }
}

// new way by RehabMan 2017-08-13
#define CompareWithMask(x,m,c) (((x) & (m)) == (c))

BOOLEAN KernelPatchPm(VOID *kernelData, LOADER_ENTRY *Entry)
{
  UINT64* Ptr = (UINT64*)kernelData;
  UINT64* End = Ptr + 0x1000000/sizeof(UINT64);
  if (Ptr == NULL) {
    return FALSE;
  }
  // Credits to RehabMan for the kernel patch information
  DBG("Patching kernel power management...\n");
  for (; Ptr < End; Ptr += 2) {
    // check for xcpm_scope_msr common 0xe2 prologue
    //    e2000000 xxxx0000 00000000 00000000 xx040000 00000000
    if (CompareWithMask(Ptr[0], 0xFFFF0000FFFFFFFF, 0x00000000000000e2) && 0 == Ptr[1] &&
        CompareWithMask(Ptr[2], 0xFFFFFFFFFFFFFF00, 0x0000000000000400)) {
      // check for last xcpm_scope_msr entry; terminates search
      // example data:
      //   e2000000 10000000 00000000 00000000 00040000 00000000 0800007e 00000000 00000000 00000000 00000000 00000000
      // or
      //   e2000000 90330000 00000000 00000000 0f040000 00000000 0800007e 00000000 00000000 00000000 00000000 00000000
      if (0x000000007e000008 == Ptr[3] && 0 == Ptr[4] && 0 == Ptr[5]) {
        // zero out 0xE2 MSR and CPU mask
        Ptr[0] = 0;
        DBG("Kernel power management: LAST entry found and patched\n");
        return TRUE;
      }
      // check for other xcpm_scope_msr entry
      // example data:
      //   e2000000 02000000 00000000 00000000 00040000 00000000 0700001e 00000000 00000000 00000000 00000000 00000000
      //   e2000000 0c000000 00000000 00000000 00040000 00000000 0500001e 00000000 00000000 00000000 00000000 00000000
      // or
      //   e2000000 4c000000 00000000 00000000 0f040000 00000000 0500001e 00000000 00000000 00000000 00000000 00000000
      else if (CompareWithMask(Ptr[3], 0xFFFFFFFFFFFFFF00, 0x000000001e000000) && 0 == Ptr[4] && 0 == Ptr[5]) {
        // zero out 0xE2 MSR and CPU mask
        Ptr[0] = 0;
        DBG("Kernel power management: entry found and patched\n");
        // last entry not found yet; continue searching for other entries
      }
    }
  }
  DBG("Kernel power management: LAST patch region not found!\n");
  return FALSE;
}

STATIC UINT8 PanicNoKextDumpFind[6]    = {0x00, 0x25, 0x2E, 0x2A, 0x73, 0x00};
//STATIC UINT8 PanicNoKextDumpReplace[6] = {0x00, 0x00, 0x2E, 0x2A, 0x73, 0x00};

BOOLEAN KernelPanicNoKextDump(VOID *kernelData)
{
  UINT8      *bytes = (UINT8*)kernelData;
  INT32      patchLocation;
  patchLocation = FindBin(bytes, 0xF00000, PanicNoKextDumpFind, 6);
  if (patchLocation > 0) {
    bytes[patchLocation + 1] = 0;
    return TRUE;
  }
  return FALSE;
}

BOOLEAN KernelLapicPatch_64(VOID *kernelData)
{
  // Credits to donovan6000 and Sherlocks for providing the lapic kernel patch source used to build this function

  UINT8       *bytes = (UINT8*)kernelData;
  UINT32      patchLocation1 = 0, patchLocation2 = 0;
  UINT32      i, y;

  DBG("Looking for Lapic panic call (64-bit) Start\n");

  for (i = 0; i < 0x1000000; i++) {
    if (bytes[i+0] == 0x65 && bytes[i+1] == 0x8B && bytes[i+2] == 0x04 && bytes[i+3] == 0x25 &&
        bytes[i+4] == 0x3C && bytes[i+5] == 0x00 && bytes[i+6] == 0x00 && bytes[i+7] == 0x00 &&
        bytes[i+45] == 0x65 && bytes[i+46] == 0x8B && bytes[i+47] == 0x04 && bytes[i+48] == 0x25 &&
        bytes[i+49] == 0x3C && bytes[i+50] == 0x00 && bytes[i+51] == 0x00 && bytes[i+52] == 0x00) {
      patchLocation1 = i+40;
      DBG("Found Lapic panic (10.6) at 0x%08x\n", patchLocation1);
      break;
    } else if (bytes[i+0]  == 0x65 && bytes[i+1]  == 0x8B && bytes[i+2]  == 0x04 && bytes[i+3]  == 0x25 &&
               bytes[i+4]  == 0x14 && bytes[i+5]  == 0x00 && bytes[i+6]  == 0x00 && bytes[i+7]  == 0x00 &&
               bytes[i+35] == 0x65 && bytes[i+36] == 0x8B && bytes[i+37] == 0x04 && bytes[i+38] == 0x25 &&
               bytes[i+39] == 0x14 && bytes[i+40] == 0x00 && bytes[i+41] == 0x00 && bytes[i+42] == 0x00) {
      patchLocation1 = i+30;
      DBG("Found Lapic panic (10.7 - 10.8) at 0x%08x\n", patchLocation1);
      break;
    } else if (bytes[i+0] == 0x65 && bytes[i+1] == 0x8B && bytes[i+2] == 0x04 && bytes[i+3] == 0x25 &&
               bytes[i+4] == 0x1C && bytes[i+5] == 0x00 && bytes[i+6] == 0x00 && bytes[i+7] == 0x00 &&
               bytes[i+36] == 0x65 && bytes[i+37] == 0x8B && bytes[i+38] == 0x04 && bytes[i+39] == 0x25 &&
               bytes[i+40] == 0x1C && bytes[i+41] == 0x00 && bytes[i+42] == 0x00 && bytes[i+43] == 0x00) {
      patchLocation1 = i+31;
      DBG("Found Lapic panic (10.9) at 0x%08x\n", patchLocation1);
      break;
    // 00 29 C7 78 XX 31 DB 8D 47 FA 83
    } else if (bytes[i+0] == 0x00 && bytes[i+1] == 0x29 && bytes[i+2] == 0xC7 && bytes[i+3] == 0x78 &&
               //(bytes[i+4] == 0x3F || bytes[i+4] == 0x4F) && // 3F:10.10-10.12/4F:10.13+
               bytes[i+5] == 0x31 && bytes[i+6] == 0xDB && bytes[i+7] == 0x8D && bytes[i+8] == 0x47 &&
               bytes[i+9] == 0xFA && bytes[i+10] == 0x83) {
      for (y = i; y < 0x1000000; y++) {
        // Lapic panic patch, by vit9696
        // mov eax, gs:XX
        // cmp eax, cs:_master_cpu
        // 65 8B 04 25 XX 00 00 00 3B 05 XX XX XX 00
        if (bytes[y+0] == 0x65 && bytes[y+1] == 0x8B && bytes[y+2] == 0x04 && bytes[y+3] == 0x25 &&
            //(bytes[y+4] == 0x1C || bytes[y+4] == 0x18) && // 1C:10.10-10.15.3/18:10.15.4+
            bytes[y+5] == 0x00 && bytes[y+6] == 0x00 && bytes[y+7] == 0x00 &&
            bytes[y+8] == 0x3B && bytes[y+9] == 0x05 && bytes[y+13] == 0x00) {
          patchLocation1 = y;
          DBG("Found Lapic panic (10.10 - recent macOS) at 0x%08x\n", patchLocation1);
          break;
        }
      }
      break;
    }
  }

  if (!patchLocation1) {
    DBG("Can't find Lapic panic, kernel patch aborted.\n");
    return FALSE;
  }

  // Already patched?  May be running a non-vanilla kernel already?
  if (bytes[patchLocation1 + 0] == 0x90 && bytes[patchLocation1 + 1] == 0x90 &&
      bytes[patchLocation1 + 2] == 0x90 && bytes[patchLocation1 + 3] == 0x90 &&
      bytes[patchLocation1 + 4] == 0x90) {
    DBG("Lapic panic already patched, kernel file (10.6 - 10.9) manually patched?\n");
    return FALSE;
  } else if (bytes[patchLocation1 + 0] == 0x31 && bytes[patchLocation1 + 1] == 0xC0 &&
             bytes[patchLocation1 + 2] == 0x90 && bytes[patchLocation1 + 3] == 0x90) {
    DBG("Lapic panic already patched, kernel file (10.10 - recent macOS) manually patched?\n");
    return FALSE;
  } else {
    if (bytes[patchLocation1 + 8] == 0x3B && bytes[patchLocation1 + 9] == 0x05 &&
        bytes[patchLocation1 + 13] == 0x00) {
      // 65 8B 04 25 XX 00 00 00 3B 05 XX XX XX 00
      // 31 C0 90 90 90 90 90 90 90 90 90 90 90 90
      DBG("Patched Lapic panic (10.10 - recent macOS)\n");
      bytes[patchLocation1 + 0] = 0x31;
      bytes[patchLocation1 + 1] = 0xC0;
      for (i = 2; i < 14; i++) {
        bytes[patchLocation1 + i] = 0x90;
      }

      for (i = 0; i < 0x1000000; i++) {
        // 00 29 C7 78 XX 31 DB 8D 47 FA 83
        if (bytes[i+0] == 0x00 && bytes[i+1] == 0x29 && bytes[i+2] == 0xC7 && bytes[i+3] == 0x78 &&
            //(bytes[i+4] == 0x3F || bytes[i+4] == 0x4F) && // 3F:10.10-10.12/4F:10.13+
            bytes[i+5] == 0x31 && bytes[i+6] == 0xDB && bytes[i+7] == 0x8D && bytes[i+8] == 0x47 &&
            bytes[i+9] == 0xFA && bytes[i+10] == 0x83) {
          for (y = i; y < 0x1000000; y++) {
            // Lapic panic master patch, by vit9696
            // cmp cs:_debug_boot_arg, 0
            // E8 XX XX FF FF 83 XX XX XX XX 00 00
            if (bytes[y+0] == 0xE8 && bytes[y+3] == 0xFF && bytes[y+4] == 0xFF &&
                bytes[y+5] == 0x83 && bytes[y+10] == 0x00 && bytes[y+11] == 0x00) {
              patchLocation2 = y;
              DBG("Found Lapic panic master (10.10 - recent macOS) at 0x%08x\n", patchLocation2);
              break;
            }
          }
          break;
        }
      }
        
      if (!patchLocation2) {
        DBG("Can't find Lapic panic master (10.10 - recent macOS), kernel patch aborted.\n");
        return FALSE;
      }
        
      // Already patched? May be running a non-vanilla kernel already?
      if (bytes[patchLocation2 + 5] == 0x31 && bytes[patchLocation2 + 6] == 0xC0) {
        DBG("Lapic panic master already patched, kernel file (10.10 - recent macOS) manually patched?\n");
        return FALSE;
      } else {
        DBG("Patched Lapic panic master (10.10 - recent macOS)\n");
        // E8 XX XX FF FF 83 XX XX XX XX 00 00
        // E8 XX XX FF FF 31 C0 90 90 90 90 90
        bytes[patchLocation2 + 5] = 0x31;
        bytes[patchLocation2 + 6] = 0xC0;
        for (i = 7; i < 12; i++) {
          bytes[patchLocation2 + i] = 0x90;
        }
      }
    } else {
      DBG("Patched Lapic panic (10.6 - 10.9)\n");
      for (i = 0; i < 5; i++) {
        bytes[patchLocation1 + i] = 0x90;
      }
    }
  }
  return TRUE;
}

BOOLEAN KernelLapicPatch_32(VOID *kernelData)
{
  // Credits to donovan6000 and Sherlocks for providing the lapic kernel patch source used to build this function

  UINT8       *bytes = (UINT8*)kernelData;
  UINT32      patchLocation = 0;
  UINT32      i;

  DBG("Looking for Lapic panic call (32-bit) Start\n");

  for (i = 0; i < 0x1000000; i++) {
    if (bytes[i+0]  == 0x65 && bytes[i+1]  == 0xA1 && bytes[i+2]  == 0x0C && bytes[i+3]  == 0x00 &&
        bytes[i+4]  == 0x00 && bytes[i+5]  == 0x00 &&
        bytes[i+30] == 0x65 && bytes[i+31] == 0xA1 && bytes[i+32] == 0x0C && bytes[i+33] == 0x00 &&
        bytes[i+34] == 0x00 && bytes[i+35] == 0x00) {
      patchLocation = i+25;
      DBG("Found Lapic panic at 0x%08x\n", patchLocation);
      break;
    }
  }

  if (!patchLocation) {
    DBG("Can't find Lapic panic, kernel patch aborted.\n");
    return FALSE;
  }

  // Already patched?  May be running a non-vanilla kernel already?

  if (bytes[patchLocation + 0] == 0x90 && bytes[patchLocation + 1] == 0x90 &&
      bytes[patchLocation + 2] == 0x90 && bytes[patchLocation + 3] == 0x90 &&
      bytes[patchLocation + 4] == 0x90) {
    DBG("Lapic panic already patched, kernel file manually patched?\n");
    return FALSE;
  } else {
    DBG("Patched Lapic panic (32-bit)\n");
    for (i = 0; i < 5; i++) {
      bytes[patchLocation + i] = 0x90;
    }
  }
  return TRUE;
}

//
// syscl - EnableExtCpuXCPM(): enable extra(unsupport) Cpu XCPM function
// PowerManagement that will be enabled on:
// SandyBridge-E, Ivy Bridge, Ivy Bridge-E, Haswell Celeron/Pentium, Haswell-E, Broadwell-E, ...
// credit Pike R.Alpha, stinga11, syscl
//
BOOLEAN (*EnableExtCpuXCPM)(VOID *kernelData, LOADER_ENTRY *Entry, BOOLEAN use_xcpm_idle);

//
// syscl - applyKernPatch a wrapper for SearchAndReplace() to make the CpuPM patch tidy and clean
//
static inline VOID applyKernPatch(UINT8 *kern, UINT8 *find, UINTN size, UINT8 *repl, const CHAR8 *comment)
{
    DBG("Searching %a...\n", comment);
    if (SearchAndReplace(kern, KERNEL_MAX_SIZE, find, size, repl, 0)) {
        DBG("Found %a\nApplied %a patch\n", comment, comment);
    } else {
        DBG("%a no found, patched already?\n", comment);
    }
}

// PMHeart
// Global XCPM patches compatibility
// Currently 10.8.5 - 10.15
//
static inline BOOLEAN IsXCPMOSVersionCompat(UINT64 os_version)
{
  return (os_version >= AsciiOSVersionToUint64("10.8.5")) && (os_version < AsciiOSVersionToUint64("10.16")); 
}

//
// Enable Unsupported CPU PowerManagement
//
// syscl - SandyBridgeEPM(): enable PowerManagement on SandyBridge-E
//
BOOLEAN SandyBridgeEPM(VOID *kernelData, LOADER_ENTRY *Entry, BOOLEAN use_xcpm_idle)
{
    // note: a dummy function that made patches consistency
    return TRUE;
}

//
// syscl - Enable Haswell-E XCPM
// Hex data provided and polished (c) PMheart, idea (c) Pike R.Alpha
//
BOOLEAN HaswellEXCPM(VOID *kernelData, LOADER_ENTRY *Entry, BOOLEAN use_xcpm_idle)
{
  DBG("HaswellEXCPM() ===>\n");
  UINT8       *kern = (UINT8*)kernelData;
  CONST CHAR8       *comment;
  UINT32      i;
  UINT32      patchLocation;
  UINT64      os_version = AsciiOSVersionToUint64(Entry->OSVersion);

  // check OS version suit for patches
  if (!IsXCPMOSVersionCompat(os_version)) {
    DBG("HaswellEXCPM(): Unsupported macOS.\n");
    DBG("HaswellEXCPM() <===FALSE\n");
    return FALSE;
  }

  // _cpuid_set_info
  comment = "_cpuid_set_info";
  if (os_version <= AsciiOSVersionToUint64("10.8.5")) {
    // 10.8.5
    STATIC UINT8 find[] = { 0x83, 0xF8, 0x3C, 0x74, 0x2D };
    STATIC UINT8 repl[] = { 0x83, 0xF8, 0x3F, 0x74, 0x2D };
    applyKernPatch(kern, find, sizeof(find), repl, comment);
  } else if (os_version < AsciiOSVersionToUint64("10.10")) {
    // 10.9.x
    STATIC UINT8 find[] = { 0x83, 0xF8, 0x3C, 0x75, 0x07 };
    STATIC UINT8 repl[] = { 0x83, 0xF8, 0x3F, 0x75, 0x07 };
    applyKernPatch(kern, find, sizeof(find), repl, comment);
  } else if (os_version <= AsciiOSVersionToUint64("10.10.1")) {
    // 10.10 - 10.10.1
    STATIC UINT8 find[] = { 0x74, 0x11, 0x83, 0xF8, 0x3C };
    STATIC UINT8 repl[] = { 0x74, 0x11, 0x83, 0xF8, 0x3F };
    applyKernPatch(kern, find, sizeof(find), repl, comment);
  } // 10.10.2+: native support reached, no need to patch

  // _xcpm_bootstrap
  comment = "_xcpm_bootstrap";
  if (os_version <= AsciiOSVersionToUint64("10.8.5")) {
    // 10.8.5
    STATIC UINT8 find[] = { 0x83, 0xFB, 0x3C, 0x75, 0x54 };
    STATIC UINT8 repl[] = { 0x83, 0xFB, 0x3F, 0x75, 0x54 };
    applyKernPatch(kern, find, sizeof(find), repl, comment);
  } else if (os_version < AsciiOSVersionToUint64("10.10")) {
    // 10.9.x
    STATIC UINT8 find[] = { 0x83, 0xFB, 0x3C, 0x75, 0x68 };
    STATIC UINT8 repl[] = { 0x83, 0xFB, 0x3F, 0x75, 0x68 };
    applyKernPatch(kern, find, sizeof(find), repl, comment);
  } else if (os_version <= AsciiOSVersionToUint64("10.10.2")) {
    // 10.10 - 10.10.2
    STATIC UINT8 find[] = { 0x83, 0xFB, 0x3C, 0x75, 0x63 };
    STATIC UINT8 repl[] = { 0x83, 0xFB, 0x3F, 0x75, 0x63 };
    applyKernPatch(kern, find, sizeof(find), repl, comment);
  } else if (os_version <= AsciiOSVersionToUint64("10.10.5")) {
    // 10.10.3 - 10.10.5
    STATIC UINT8 find[] = { 0x83, 0xC3, 0xC6, 0x83, 0xFB, 0x0D };
    STATIC UINT8 repl[] = { 0x83, 0xC3, 0xC3, 0x83, 0xFB, 0x0D };
    applyKernPatch(kern, find, sizeof(find), repl, comment);
  } else if (os_version <= AsciiOSVersionToUint64("10.11")) {
    // 10.11 DB/PB - 10.11.0
    STATIC UINT8 find[] = { 0x83, 0xC3, 0xC6, 0x83, 0xFB, 0x0D };
    STATIC UINT8 repl[] = { 0x83, 0xC3, 0xC3, 0x83, 0xFB, 0x0D };
    applyKernPatch(kern, find, sizeof(find), repl, comment);
  } else if (os_version <= AsciiOSVersionToUint64("10.11.6")) {
    // 10.11.1 - 10.11.6
    STATIC UINT8 find[] = { 0x83, 0xC3, 0xBB, 0x83, 0xFB, 0x09 };
    STATIC UINT8 repl[] = { 0x83, 0xC3, 0xB8, 0x83, 0xFB, 0x09 };
    applyKernPatch(kern, find, sizeof(find), repl, comment);
  } else if (os_version <= AsciiOSVersionToUint64("10.12.5")) {
    // 10.12 - 10.12.5
    STATIC UINT8 find[] = { 0x83, 0xC3, 0xC4, 0x83, 0xFB, 0x22 };
    STATIC UINT8 repl[] = { 0x83, 0xC3, 0xC1, 0x83, 0xFB, 0x22 };
    applyKernPatch(kern, find, sizeof(find), repl, comment);
  } else if (os_version < AsciiOSVersionToUint64("10.13")) {
    // 10.12.6
    STATIC UINT8 find[] = { 0x8D, 0x43, 0xC4, 0x83, 0xF8, 0x22 };
    STATIC UINT8 repl[] = { 0x8D, 0x43, 0xC1, 0x83, 0xF8, 0x22 };
    applyKernPatch(kern, find, sizeof(find), repl, comment);
    // PMheart: attempt to add 10.14 compatibility
  } else if (os_version < AsciiOSVersionToUint64("10.15")) {
    // 10.13/10.14
    STATIC UINT8 find[] = { 0x89, 0xD8, 0x04, 0xC4, 0x3C, 0x22 };
    STATIC UINT8 repl[] = { 0x89, 0xD8, 0x04, 0xC1, 0x3C, 0x22 };
    applyKernPatch(kern, find, sizeof(find), repl, comment);
    // PMheart: attempt to add 10.15 compatibility
  } else if (os_version < AsciiOSVersionToUint64("10.16")) {
    STATIC UINT8 find[] = { 0x8D, 0x43, 0xC4, 0x3C, 0x22 };
    STATIC UINT8 repl[] = { 0x8D, 0x43, 0xC1, 0x3C, 0x22 };
    applyKernPatch(kern, find, sizeof(find), repl, comment);
  }

  DBG("Searching _xcpm_pkg_scope_msr ...\n");
  comment = "_xcpm_pkg_scope_msrs";
  if (os_version <= AsciiOSVersionToUint64("10.8.5")) {
    // 10.8.5
    STATIC UINT8 find[] = {
      0x48, 0x8D, 0x3D, 0x02, 0x71, 0x55, 0x00, 0xBE,
      0x07, 0x00, 0x00, 0x00, 0xEB, 0x1F, 0x48, 0x8D,
      0x3D, 0xF4, 0x70, 0x55, 0x00, 0xBE, 0x07, 0x00,
      0x00, 0x00, 0x31, 0xD2, 0xE8, 0x28, 0x02, 0x00, 0x00
    };
    STATIC UINT8 repl[] = {
      0x48, 0x8D, 0x3D, 0x02, 0x71, 0x55, 0x00, 0xBE,
      0x07, 0x00, 0x00, 0x00, 0x90, 0x90, 0x48, 0x8D,
      0x3D, 0xF4, 0x70, 0x55, 0x00, 0xBE, 0x07, 0x00,
      0x00, 0x00, 0x31, 0xD2, 0x90, 0x90, 0x90, 0x90, 0x90
    };
    applyKernPatch(kern, find, sizeof(find), repl, comment);
  } else if (os_version < AsciiOSVersionToUint64("10.10")) {
    // 10.9.x
    STATIC UINT8 find[] = { 0xBE, 0x07, 0x00, 0x00, 0x00, 0x74, 0x13, 0x31, 0xD2, 0xE8, 0x5F, 0x02, 0x00, 0x00 };
    STATIC UINT8 repl[] = { 0xBE, 0x07, 0x00, 0x00, 0x00, 0x90, 0x90, 0x31, 0xD2, 0x90, 0x90, 0x90, 0x90, 0x90 };
    applyKernPatch(kern, find, sizeof(find), repl, comment);
  } else {
    // 10.10+
    patchLocation = 0; // clean out the value just in case
    for (i = 0; i < 0x1000000; i++) {
      if (kern[i+0] == 0xBE && kern[i+1] == 0x07 && kern[i+2] == 0x00 && kern[i+3] == 0x00 &&
          kern[i+4] == 0x00 && kern[i+5] == 0x31 && kern[i+6] == 0xD2 && kern[i+7] == 0xE8) {
        patchLocation = i+7;
        DBG("Found _xcpm_pkg_scope_msr\n");
        break;
      }
    }

    if (patchLocation) {
      for (i = 0; i < 5; i++) {
        kern[patchLocation+i] = 0x90;
      }
      DBG("Applied _xcpm_pkg_scope_msr patch\n");
    } else {
      DBG("_xcpm_pkg_scope_msr not found, patch aborted\n");
      DBG("HaswellEXCPM() <===FALSE\n");
      return FALSE;
    }
  }

  DBG("HaswellEXCPM() <===\n");
  return TRUE;
}

//
// Enable Broadwell-E/EP PowerManagement on 10.12+ by syscl
//
BOOLEAN BroadwellEPM(VOID *kernelData, LOADER_ENTRY *Entry, BOOLEAN use_xcpm_idle)
{
  DBG("BroadwellEPM() ===>\n");
  UINT8       *kern = (UINT8*)kernelData;
  UINT32      i;
  UINT32      patchLocation;
  UINT64      os_version = AsciiOSVersionToUint64(Entry->OSVersion);

  // check OS version suit for patches
  if (!IsXCPMOSVersionCompat(os_version)) {
    DBG("BroadwellEPM(): Unsupported macOS.\n");
    DBG("BroadwellEPM() <===FALSE\n");
    return FALSE;
  }

  Entry->KernelAndKextPatches->FakeCPUID = (UINT32)(os_version < AsciiOSVersionToUint64("10.10.3") ? 0x0306C0 : 0x040674);
  KernelCPUIDPatch(kern, Entry);

  DBG("Searching _xcpm_pkg_scope_msr ...\n");
  if (os_version >= AsciiOSVersionToUint64("10.12")) {
    // 10.12+
    patchLocation = 0; // clean out the value just in case
    for (i = 0; i < 0x1000000; i++) {
      if (kern[i+0] == 0xBE && kern[i+1] == 0x07 && kern[i+2] == 0x00 && kern[i+3] == 0x00 &&
          kern[i+4] == 0x00 && kern[i+5] == 0x31 && kern[i+6] == 0xD2 && kern[i+7] == 0xE8) {
        patchLocation = i+7;
        DBG("Found _xcpm_pkg_scope_msr\n");
        break;
      }
    }

    if (patchLocation) {
      for (i = 0; i < 5; i++) {
        kern[patchLocation+i] = 0x90;
      }
      DBG("Applied _xcpm_pkg_scope_msr patch\n");
    } else {
      DBG("_xcpm_pkg_scope_msr not found, patch aborted\n");
      DBG("BroadwellEPM() <===FALSE\n");
      return FALSE;
    }
  }

  DBG("BroadwellEPM() <===\n");
  return TRUE;
}
//
// syscl - this patch provides XCPM support for Haswell low-end(HSWLowEnd) and platforms later than Haswell
// implemented by syscl
// credit also Pike R.Alpha, stinga11, Sherlocks, vit9696
//
BOOLEAN HaswellLowEndXCPM(VOID *kernelData, LOADER_ENTRY *Entry, BOOLEAN use_xcpm_idle)
{
  DBG("HaswellLowEndXCPM() ===>\n");
  UINT8       *kern = (UINT8*)kernelData;
  UINT64      os_version = AsciiOSVersionToUint64(Entry->OSVersion);
  CONST CHAR8       *comment;

  // check OS version suit for patches
  if (!IsXCPMOSVersionCompat(os_version)) {
    DBG("HaswellLowEndXCPM(): Unsupported macOS.\n");
    DBG("HaswellLowEndXCPM() <===FALSE\n");
    return FALSE;
  }

  Entry->KernelAndKextPatches->FakeCPUID = (UINT32)(0x0306A0);    // correct FakeCPUID
  KernelCPUIDPatch(kern, Entry);

  // 10.8.5 - 10.11.x no need the following kernel patches on Haswell Celeron/Pentium
  if (os_version >= AsciiOSVersionToUint64("10.8.5") && os_version < AsciiOSVersionToUint64("10.12") &&
      (!use_xcpm_idle)) {
    DBG("HaswellLowEndXCPM() <===\n");
    return TRUE;
  }

  // _xcpm_idle
  if (use_xcpm_idle) {
    DBG("HWPEnable - ON.\n");
    comment = "_xcpm_idle";
    STATIC UINT8 find[] = { 0xB9, 0xE2, 0x00, 0x00, 0x00, 0x0F, 0x30 };
    STATIC UINT8 repl[] = { 0xB9, 0xE2, 0x00, 0x00, 0x00, 0x90, 0x90 };
    applyKernPatch(kern, find, sizeof(find), repl, comment);
  }

  comment = "_xcpm_bootstrap";
  if (os_version <= AsciiOSVersionToUint64("10.12.5")) {
    // 10.12 - 10.12.5
    STATIC UINT8 find[] = { 0x83, 0xC3, 0xC4, 0x83, 0xFB, 0x22 };
    STATIC UINT8 repl[] = { 0x83, 0xC3, 0xC6, 0x83, 0xFB, 0x22 };
    applyKernPatch(kern, find, sizeof(find), repl, comment);
  } else if (os_version < AsciiOSVersionToUint64("10.13")) {
    // 10.12.6
    STATIC UINT8 find[] = { 0x8D, 0x43, 0xC4, 0x83, 0xF8, 0x22 };
    STATIC UINT8 repl[] = { 0x8D, 0x43, 0xC6, 0x83, 0xF8, 0x22 };
    applyKernPatch(kern, find, sizeof(find), repl, comment);
  } else if (os_version < AsciiOSVersionToUint64("10.15")) {
    // 10.13/10.14
    STATIC UINT8 find[] = { 0x89, 0xD8, 0x04, 0xC4, 0x3C, 0x22 };
    STATIC UINT8 repl[] = { 0x89, 0xD8, 0x04, 0xC6, 0x3C, 0x22 };
    applyKernPatch(kern, find, sizeof(find), repl, comment);
    // PMheart: attempt to add 10.15 compatibility
  } else if (os_version < AsciiOSVersionToUint64("10.16")) {
    STATIC UINT8 find[] = { 0x8D, 0x43, 0xC4, 0x3C, 0x22 };
    STATIC UINT8 repl[] = { 0x8D, 0x43, 0xC6, 0x3C, 0x22 };
    applyKernPatch(kern, find, sizeof(find), repl, comment);
  }

  comment = "_cpuid_set_info_rdmsr";
  // PMheart: bytes seem stable as of 10.12
  if (os_version >= AsciiOSVersionToUint64("10.12")) {
    // 10.12+
    STATIC UINT8 find[] = { 0xB9, 0xA0, 0x01, 0x00, 0x00, 0x0F, 0x32 };
    STATIC UINT8 repl[] = { 0xB9, 0xA0, 0x01, 0x00, 0x00, 0x31, 0xC0 };
    applyKernPatch(kern, find, sizeof(find), repl, comment);
  }

  DBG("HaswellLowEndXCPM() <===\n");
  return TRUE;
}

//
// this patch provides XCPM support for Ivy Bridge. by PMheart
//
BOOLEAN KernelIvyBridgeXCPM(VOID *kernelData, LOADER_ENTRY *Entry, BOOLEAN use_xcpm_idle)
{
  UINT8       *kern = (UINT8*)kernelData;
  CONST CHAR8       *comment;
  UINT32      i;
  UINT32      patchLocation;
  UINT64      os_version = AsciiOSVersionToUint64(Entry->OSVersion);

  // check whether Ivy Bridge
  if (gCPUStructure.Model != CPU_MODEL_IVY_BRIDGE) {
    DBG("KernelIvyBridgeXCPM(): Unsupported platform.\nRequires Ivy Bridge, aborted\n");
    DBG("KernelIvyBridgeXCPM() <===FALSE\n");
    return FALSE;
  }

  // check OS version suit for patches
  // PMheart: attempt to add 10.14 compatibility
  if (!IsXCPMOSVersionCompat(os_version)) {
    DBG("KernelIvyBridgeXCPM():Unsupported macOS.\n");
    DBG("KernelIvyBridgeXCPM() <===FALSE\n");
    return FALSE;
  } else if (os_version >= AsciiOSVersionToUint64("10.8.5") && os_version < AsciiOSVersionToUint64("10.12")) {
    // 10.8.5 - 10.11.x no need the following kernel patches on Ivy Bridge - we just use -xcpm boot-args
    DBG("KernelIvyBridgeXCPM() <===\n");
    return TRUE;
  }

  DBG("Searching _xcpm_pkg_scope_msr ...\n");
  if (os_version >= AsciiOSVersionToUint64("10.12")) {
    // 10.12+
    patchLocation = 0; // clean out the value just in case
    for (i = 0; i < 0x1000000; i++) {
      if (kern[i+0] == 0xBE && kern[i+1] == 0x07 && kern[i+2] == 0x00 && kern[i+3] == 0x00 &&
          kern[i+4] == 0x00 && kern[i+5] == 0x31 && kern[i+6] == 0xD2 && kern[i+7] == 0xE8) {
        patchLocation = i+7;
        DBG("Found _xcpm_pkg_scope_msr\n");
        break;
      }
    }

    if (patchLocation) {
      for (i = 0; i < 5; i++) {
        kern[patchLocation+i] = 0x90;
      }
      DBG("Applied _xcpm_pkg_scope_msr patch\n");
    } else {
      DBG("_xcpm_pkg_scope_msr not found, patch aborted\n");
      DBG("KernelIvyBridgeXCPM() <===FALSE\n");
      return FALSE;
    }
  }

  comment = "_xcpm_bootstrap";
  if (os_version <= AsciiOSVersionToUint64("10.12.5")) {
    // 10.12 - 10.12.5
    STATIC UINT8 find[] = { 0x83, 0xC3, 0xC4, 0x83, 0xFB, 0x22 };
    STATIC UINT8 repl[] = { 0x83, 0xC3, 0xC6, 0x83, 0xFB, 0x22 };
    applyKernPatch(kern, find, sizeof(find), repl, comment);
  } else if (os_version < AsciiOSVersionToUint64("10.13")) {
    // 10.12.6
    STATIC UINT8 find[] = { 0x8D, 0x43, 0xC4, 0x83, 0xF8, 0x22 };
    STATIC UINT8 repl[] = { 0x8D, 0x43, 0xC6, 0x83, 0xF8, 0x22 };
    applyKernPatch(kern, find, sizeof(find), repl, comment);
    // PMheart: attempt to add 10.14 compatibility
  } else if (os_version < AsciiOSVersionToUint64("10.15")) {
    // 10.13/10.14
    STATIC UINT8 find[] = { 0x89, 0xD8, 0x04, 0xC4, 0x3C, 0x22 };
    STATIC UINT8 repl[] = { 0x89, 0xD8, 0x04, 0xC6, 0x3C, 0x22 };
    applyKernPatch(kern, find, sizeof(find), repl, comment);
    // PMheart: attempt to add 10.15 compatibility
  } else if (os_version < AsciiOSVersionToUint64("10.16")) {
    STATIC UINT8 find[] = { 0x8D, 0x43, 0xC4, 0x3C, 0x22 };
    STATIC UINT8 repl[] = { 0x8D, 0x43, 0xC6, 0x3C, 0x22 };
    applyKernPatch(kern, find, sizeof(find), repl, comment);
  }

  DBG("KernelIvyBridgeXCPM() <===\n");
  return TRUE;
}

//
// this patch provides XCPM support for Ivy Bridge-E. by PMheart
// attempt to enable XCPM for Ivy-E, still need to test further
//
BOOLEAN KernelIvyE5XCPM(VOID *kernelData, LOADER_ENTRY *Entry, BOOLEAN use_xcpm_idle)
{
  UINT8       *kern = (UINT8*)kernelData;
  CONST CHAR8       *comment;
  UINT32      i;
  UINT32      patchLocation;
  UINT64      os_version = AsciiOSVersionToUint64(Entry->OSVersion);
  
  // check whether Ivy Bridge-E5
  if (gCPUStructure.Model != CPU_MODEL_IVY_BRIDGE_E5) {
    DBG("KernelIvyE5XCPM(): Unsupported platform.\nRequires Ivy Bridge-E, aborted\n");
    DBG("KernelIvyE5XCPM() <===FALSE\n");
    return FALSE;
  }
  
  // check OS version suit for patches
  // PMheart: attempt to add 10.15 compatibility
  if (!IsXCPMOSVersionCompat(os_version)) {
    DBG("KernelIvyE5XCPM(): Unsupported macOS.\n");
    DBG("KernelIvyE5XCPM() <===FALSE\n");
    return FALSE;
  }
  
  // _cpuid_set_info
  // TO-DO: should we use FakeCPUID instead?
  comment = "_cpuid_set_info";
  if (os_version <= AsciiOSVersionToUint64("10.8.5")) {
    // 10.8.5
    STATIC UINT8 find[] = { 0x83, 0xF8, 0x3C, 0x74, 0x2D };
    STATIC UINT8 repl[] = { 0x83, 0xF8, 0x3E, 0x74, 0x2D };
    applyKernPatch(kern, find, sizeof(find), repl, comment);
  } else if (os_version == AsciiOSVersionToUint64("10.9") || os_version == AsciiOSVersionToUint64("10.9.1")) {
    // 10.9.0 - 10.9.1
    STATIC UINT8 find[] = { 0x83, 0xF8, 0x3C, 0x75, 0x07 };
    STATIC UINT8 repl[] = { 0x83, 0xF8, 0x3E, 0x75, 0x07 };
    applyKernPatch(kern, find, sizeof(find), repl, comment);
  } // 10.9.2+: native support reached, no need to patch
  
  // _xcpm_pkg_scope_msrs
  DBG("Searching _xcpm_pkg_scope_msrs ...\n");
  comment = "_xcpm_pkg_scope_msrs";
  if (os_version <= AsciiOSVersionToUint64("10.8.5")) {
    // 10.8.5
    STATIC UINT8 find[] = {
      0x48, 0x8D, 0x3D, 0x02, 0x71, 0x55, 0x00, 0xBE,
      0x07, 0x00, 0x00, 0x00, 0xEB, 0x1F, 0x48, 0x8D,
      0x3D, 0xF4, 0x70, 0x55, 0x00, 0xBE, 0x07, 0x00,
      0x00, 0x00, 0x31, 0xD2, 0xE8, 0x28, 0x02, 0x00, 0x00
    };
    STATIC UINT8 repl[] = {
      0x48, 0x8D, 0x3D, 0x02, 0x71, 0x55, 0x00, 0xBE,
      0x07, 0x00, 0x00, 0x00, 0x90, 0x90, 0x48, 0x8D,
      0x3D, 0xF4, 0x70, 0x55, 0x00, 0xBE, 0x07, 0x00,
      0x00, 0x00, 0x31, 0xD2, 0x90, 0x90, 0x90, 0x90, 0x90
    };
    applyKernPatch(kern, find, sizeof(find), repl, comment);
  } else if (os_version < AsciiOSVersionToUint64("10.10")) {
    // 10.9.x
    STATIC UINT8 find[] = { 0xBE, 0x07, 0x00, 0x00, 0x00, 0x74, 0x13, 0x31, 0xD2, 0xE8, 0x5F, 0x02, 0x00, 0x00 };
    STATIC UINT8 repl[] = { 0xBE, 0x07, 0x00, 0x00, 0x00, 0x90, 0x90, 0x31, 0xD2, 0x90, 0x90, 0x90, 0x90, 0x90 };
    applyKernPatch(kern, find, sizeof(find), repl, comment);
  } else {
    // 10.10+
    patchLocation = 0; // clean out the value just in case
    for (i = 0; i < 0x1000000; i++) {
      if (kern[i+0] == 0xBE && kern[i+1] == 0x07 && kern[i+2] == 0x00 && kern[i+3] == 0x00 &&
          kern[i+4] == 0x00 && kern[i+5] == 0x31 && kern[i+6] == 0xD2 && kern[i+7] == 0xE8) {
        patchLocation = i+7;
        DBG("Found _xcpm_pkg_scope_msr\n");
        break;
      }
    }
    
    if (patchLocation) {
      for (i = 0; i < 5; i++) {
        kern[patchLocation+i] = 0x90;
      }
      DBG("Applied _xcpm_pkg_scope_msr patch\n");
    } else {
      DBG("_xcpm_pkg_scope_msr not found, patch aborted\n");
      DBG("KernelIvyE5XCPM() <===FALSE\n");
      return FALSE;
    }
  }
  
  // _xcpm_bootstrap
  comment = "_xcpm_bootstrap";
  if (os_version <= AsciiOSVersionToUint64("10.8.5")) {
    // 10.8.5
    STATIC UINT8 find[] = { 0x83, 0xFB, 0x3C, 0x75, 0x54 };
    STATIC UINT8 repl[] = { 0x83, 0xFB, 0x3E, 0x75, 0x54 };
    applyKernPatch(kern, find, sizeof(find), repl, comment);
  } else if (os_version < AsciiOSVersionToUint64("10.10")) {
    // 10.9.x
    STATIC UINT8 find[] = { 0x83, 0xFB, 0x3C, 0x75, 0x68 };
    STATIC UINT8 repl[] = { 0x83, 0xFB, 0x3E, 0x75, 0x68 };
    applyKernPatch(kern, find, sizeof(find), repl, comment);
  } else if (os_version <= AsciiOSVersionToUint64("10.10.2")) {
    // 10.10 - 10.10.2
    STATIC UINT8 find[] = { 0x83, 0xFB, 0x3C, 0x75, 0x63 };
    STATIC UINT8 repl[] = { 0x83, 0xFB, 0x3E, 0x75, 0x63 };
    applyKernPatch(kern, find, sizeof(find), repl, comment);
  } else if (os_version <= AsciiOSVersionToUint64("10.10.5")) {
    // 10.10.3 - 10.10.5
    STATIC UINT8 find[] = { 0x83, 0xC3, 0xC6, 0x83, 0xFB, 0x0D };
    STATIC UINT8 repl[] = { 0x83, 0xC3, 0xC4, 0x83, 0xFB, 0x0D };
    applyKernPatch(kern, find, sizeof(find), repl, comment);
  } else if (os_version <= AsciiOSVersionToUint64("10.11")) {
    // 10.11 DB/PB - 10.11.0
    STATIC UINT8 find[] = { 0x83, 0xC3, 0xC6, 0x83, 0xFB, 0x0D };
    STATIC UINT8 repl[] = { 0x83, 0xC3, 0xC4, 0x83, 0xFB, 0x0D };
    applyKernPatch(kern, find, sizeof(find), repl, comment);
  } else if (os_version <= AsciiOSVersionToUint64("10.11.6")) {
    // 10.11.1 - 10.11.6
    STATIC UINT8 find[] = { 0x83, 0xC3, 0xBB, 0x83, 0xFB, 0x09 };
    STATIC UINT8 repl[] = { 0x83, 0xC3, 0xB9, 0x83, 0xFB, 0x09 };
    applyKernPatch(kern, find, sizeof(find), repl, comment);
  } else if (os_version <= AsciiOSVersionToUint64("10.12.5")) {
    // 10.12 - 10.12.5
    STATIC UINT8 find[] = { 0x83, 0xC3, 0xC4, 0x83, 0xFB, 0x22 };
    STATIC UINT8 repl[] = { 0x83, 0xC3, 0xC2, 0x83, 0xFB, 0x22 };
    applyKernPatch(kern, find, sizeof(find), repl, comment);
  } else if (os_version < AsciiOSVersionToUint64("10.13")) {
    // 10.12.6
    STATIC UINT8 find[] = { 0x8D, 0x43, 0xC4, 0x83, 0xF8, 0x22 };
    STATIC UINT8 repl[] = { 0x8D, 0x43, 0xC2, 0x83, 0xF8, 0x22 };
    applyKernPatch(kern, find, sizeof(find), repl, comment);
  // PMheart: attempt to add 10.14 compatibility
  } else if (os_version < AsciiOSVersionToUint64("10.15")) {
    // 10.13/10.14
    STATIC UINT8 find[] = { 0x89, 0xD8, 0x04, 0xC4, 0x3C, 0x22 };
    STATIC UINT8 repl[] = { 0x89, 0xD8, 0x04, 0xC1, 0x3C, 0x22 };
    applyKernPatch(kern, find, sizeof(find), repl, comment);
  // PMheart: attempt to add 10.15 compatibility
  } else if (os_version < AsciiOSVersionToUint64("10.16")) {
    STATIC UINT8 find[] = { 0x8D, 0x43, 0xC4, 0x3C, 0x22 };
    STATIC UINT8 repl[] = { 0x8D, 0x43, 0xC1, 0x3C, 0x22 };
    applyKernPatch(kern, find, sizeof(find), repl, comment);
  }
  
  DBG("KernelIvyE5XCPM() <===\n");
  return TRUE;
}

VOID Patcher_SSE3_6(VOID* kernelData)
{
  UINT8* bytes = (UINT8*)kernelData;
  UINT32 patchLocation1 = 0;
  UINT32 patchLocation2 = 0;
  UINT32 patchlast = 0;
  UINT32 i;
  //UINT32 Length = sizeof(kernelData);

  DBG("Start find SSE3 address\n");
  i=0;
  //for (i=0;i<Length;i++)
  while(TRUE) {
    if (bytes[i] == 0x66 && bytes[i+1] == 0x0F && bytes[i+2] == 0x6F &&
        bytes[i+3] == 0x44 && bytes[i+4] == 0x0E && bytes[i+5] == 0xF1 &&
        bytes[i-1664-32] == 0x55
        ) {
      patchLocation1 = i-1664-32;
      DBG("Found SSE3 data address at 0x%08x\n",patchLocation1);
    }

    // hasSSE2+..... title
    if (bytes[i] == 0xE3 && bytes[i+1] == 0x07 && bytes[i+2] == 0x00 &&
        bytes[i+3] == 0x00 && bytes[i+4] == 0x80 && bytes[i+5] == 0x07 &&
        bytes[i+6] == 0xFF && bytes[i+7] == 0xFF && bytes[i+8] == 0x24 &&
        bytes[i+9] == 0x01) {
      patchLocation2 = i;
      DBG("Found SSE3 Title address at 0x%08x\n",patchLocation2);
      break;
    }
    i++;
  }

  if (!patchLocation1 || !patchLocation2) {
    DBG("Can't found SSE3 data addres or Title address at 0x%08x 0x%08x\n", patchLocation1, patchLocation2);
    return;
  }

  DBG("Found SSE3 last data addres Start\n");
  i = patchLocation1 + 1500;
  //for (i=(patchLocation1+1500); i<(patchLocation1+3000); i++)
  while(TRUE) {
    if (bytes[i] == 0x90 && bytes[i+1] == 0x90 && bytes[i+2] == 0x55 ) {
      patchlast = (i+1) - patchLocation1;
      DBG("Found SSE3 last data addres at 0x%08x\n", patchlast);
      break;
    }
    i++;
  }

  if (!patchlast) {
    DBG("Can't found SSE3 data last addres at 0x%08x\n", patchlast);
    return;
  }
  // patch sse3_64 data

  for (i=0; i<patchlast; i++) {
    if (i<sizeof(sse3_patcher)) {
      bytes[patchLocation1 + i] = sse3_patcher[i];
    } else {
      bytes[patchLocation1 + i] = 0x90;
    }
  }

  // patch kHasSSE3 title
  bytes[patchLocation2 + 0] = 0xFC;
  bytes[patchLocation2 + 1] = 0x05;
  bytes[patchLocation2 + 8] = 0x2C;
  bytes[patchLocation2 + 9] = 0x00;

}

VOID Patcher_SSE3_5(VOID* kernelData)
{
  UINT8* bytes = (UINT8*)kernelData;
  UINT32 patchLocation1 = 0;
  UINT32 patchLocation2 = 0;
  UINT32 patchlast=0;
  UINT32 Length = sizeof(kernelData);
  UINT32 i;

  DBG("Start find SSE3 address\n");

  for (i=256; i<(Length-256); i++) {
    if (bytes[i] == 0x66 && bytes[i+1] == 0x0F && bytes[i+2] == 0x6F &&
        bytes[i+3] == 0x44 && bytes[i+4] == 0x0E && bytes[i+5] == 0xF1 &&
        bytes[i-1680-32] == 0x55) {
      patchLocation1 = i-1680-32;
      DBG("Found SSE3 data address at 0x%08x\n",patchLocation1);
    }

    // khasSSE2+..... title
    if (bytes[i] == 0xF3 && bytes[i+1] == 0x07 && bytes[i+2] == 0x00 &&
        bytes[i+3] == 0x00 && bytes[i+4] == 0x80 && bytes[i+5] == 0x07 &&
        bytes[i+6] == 0xFF && bytes[i+7] == 0xFF && bytes[i+8] == 0x24 &&
        bytes[i+9] == 0x01) {
      patchLocation2 = i;
      DBG("Found SSE3 Title address at 0x%08x\n",patchLocation2);
      break;
    }
  }

  if (!patchLocation1 || !patchLocation2) {
    DBG("Can't found SSE3 data addres or Title address at 0x%08x 0x%08x\n", patchLocation1, patchLocation2);
    return;
  }

  DBG("Found SSE3 last data addres Start\n");

  for (i=(patchLocation1+1500);i<Length;i++) {
    if (bytes[i] == 0x90 && bytes[i+1] == 0x90 && bytes[i+2] == 0x55) {
      patchlast = (i+1) - patchLocation1;
      DBG("Found SSE3 last data addres at 0x%08x\n", patchlast);
      break;
    }
  }

  if (!patchlast) {
    DBG("Can't found SSE3 data last addres at 0x%08x\n", patchlast);
    return;
  }

  // patech sse3_64 data

  for (i=0; i<patchlast; i++) {
    if (i<sizeof(sse3_5_patcher)) {
      bytes[patchLocation1 + i] = sse3_5_patcher[i];
    } else {
      bytes[patchLocation1 + i] = 0x90;
    }
  }

  // patch kHasSSE3 title
  bytes[patchLocation2 + 0] = 0x0C;
  bytes[patchLocation2 + 1] = 0x06;
  bytes[patchLocation2 + 8] = 0x2C;
  bytes[patchLocation2 + 9] = 0x00;

}

VOID Patcher_SSE3_7(VOID* kernelData)
{
     // not support yet
     return;
}

VOID Get_PreLink()
{
  UINT32  ncmds, cmdsize;
  UINT32  binaryIndex;
  UINTN   cnt;
  UINT8*  binary = (UINT8*)KernelData;
  struct  load_command        *loadCommand;
  struct  segment_command     *segCmd;
  struct  segment_command_64  *segCmd64;


  if (is64BitKernel) {
    binaryIndex = sizeof(struct mach_header_64);
  } else {
    binaryIndex = sizeof(struct mach_header);
  }

  ncmds = MACH_GET_NCMDS(binary);

  for (cnt = 0; cnt < ncmds; cnt++) {
    loadCommand = (struct load_command *)(binary + binaryIndex);
    cmdsize = loadCommand->cmdsize;

    switch (loadCommand->cmd) {
      case LC_SEGMENT_64:
        segCmd64 = (struct segment_command_64 *)loadCommand;
        //DBG("segCmd64->segname = %a\n",segCmd64->segname);
        //DBG("segCmd64->vmaddr = 0x%08x\n",segCmd64->vmaddr)
        //DBG("segCmd64->vmsize = 0x%08x\n",segCmd64->vmsize);
        if (AsciiStrCmp(segCmd64->segname, kPrelinkTextSegment) == 0) {
          DBG("Found PRELINK_TEXT, 64bit\n");
          if (segCmd64->vmsize > 0) {
            // 64bit segCmd64->vmaddr is 0xffffff80xxxxxxxx
            // PrelinkTextAddr = xxxxxxxx + KernelRelocBase
            PrelinkTextAddr = (UINT32)(segCmd64->vmaddr ? segCmd64->vmaddr + KernelRelocBase : 0);
            PrelinkTextSize = (UINT32)segCmd64->vmsize;
            PrelinkTextLoadCmdAddr = (UINT32)(UINTN)segCmd64;
          }
          DBG("at %p: vmaddr = 0x%lx, vmsize = 0x%lx\n", segCmd64, segCmd64->vmaddr, segCmd64->vmsize);
          DBG("PrelinkTextLoadCmdAddr = 0x%x, PrelinkTextAddr = 0x%x, PrelinkTextSize = 0x%x\n",
              PrelinkTextLoadCmdAddr, PrelinkTextAddr, PrelinkTextSize);
          //DBG("cmd = 0x%08x\n",segCmd64->cmd);
          //DBG("cmdsize = 0x%08x\n",segCmd64->cmdsize);
          //DBG("vmaddr = 0x%08x\n",segCmd64->vmaddr);
          //DBG("vmsize = 0x%08x\n",segCmd64->vmsize);
          //DBG("fileoff = 0x%08x\n",segCmd64->fileoff);
          //DBG("filesize = 0x%08x\n",segCmd64->filesize);
          //DBG("maxprot = 0x%08x\n",segCmd64->maxprot);
          //DBG("initprot = 0x%08x\n",segCmd64->initprot);
          //DBG("nsects = 0x%08x\n",segCmd64->nsects);
          //DBG("flags = 0x%08x\n",segCmd64->flags);
        }
        if (AsciiStrCmp(segCmd64->segname, kPrelinkInfoSegment) == 0) {
          UINT32 sectionIndex;
          struct section_64 *sect;

          DBG("Found PRELINK_INFO, 64bit\n");
          //DBG("cmd = 0x%08x\n",segCmd64->cmd);
          //DBG("cmdsize = 0x%08x\n",segCmd64->cmdsize);
          DBG("vmaddr = 0x%08x\n",segCmd64->vmaddr);
          DBG("vmsize = 0x%08x\n",segCmd64->vmsize);
          //DBG("fileoff = 0x%08x\n",segCmd64->fileoff);
          //DBG("filesize = 0x%08x\n",segCmd64->filesize);
          //DBG("maxprot = 0x%08x\n",segCmd64->maxprot);
          //DBG("initprot = 0x%08x\n",segCmd64->initprot);
          //DBG("nsects = 0x%08x\n",segCmd64->nsects);
          //DBG("flags = 0x%08x\n",segCmd64->flags);
          sectionIndex = sizeof(struct segment_command_64);

          while(sectionIndex < segCmd64->cmdsize) {
            sect = (struct section_64 *)((UINT8*)segCmd64 + sectionIndex);
            sectionIndex += sizeof(struct section_64);

            if(AsciiStrCmp(sect->sectname, kPrelinkInfoSection) == 0 && AsciiStrCmp(sect->segname, kPrelinkInfoSegment) == 0) {
              if (sect->size > 0) {
                // 64bit sect->addr is 0xffffff80xxxxxxxx
                // PrelinkInfoAddr = xxxxxxxx + KernelRelocBase
                PrelinkInfoLoadCmdAddr = (UINT32)(UINTN)sect;
                PrelinkInfoAddr = (UINT32)(sect->addr ? sect->addr + KernelRelocBase : 0);
                PrelinkInfoSize = (UINT32)sect->size;
              }
              DBG("__info found at %p: addr = 0x%lx, size = 0x%lx\n", sect, sect->addr, sect->size);
              DBG("PrelinkInfoLoadCmdAddr = 0x%x, PrelinkInfoAddr = 0x%x, PrelinkInfoSize = 0x%x\n",
                  PrelinkInfoLoadCmdAddr, PrelinkInfoAddr, PrelinkInfoSize);
            }
          }
        }
        break;

      case LC_SEGMENT:
        segCmd = (struct segment_command *)loadCommand;
        //DBG("segCmd->segname = %a\n",segCmd->segname);
        //DBG("segCmd->vmaddr = 0x%08x\n",segCmd->vmaddr)
        //DBG("segCmd->vmsize = 0x%08x\n",segCmd->vmsize);
        if (AsciiStrCmp(segCmd->segname, kPrelinkTextSegment) == 0) {
          DBG("Found PRELINK_TEXT, 32bit\n");
          if (segCmd->vmsize > 0) {
            // PrelinkTextAddr = vmaddr + KernelRelocBase
            PrelinkTextAddr = (UINT32)(segCmd->vmaddr ? segCmd->vmaddr + KernelRelocBase : 0);
            PrelinkTextSize = (UINT32)segCmd->vmsize;
            PrelinkTextLoadCmdAddr = (UINT32)(UINTN)segCmd;
          }
          DBG("at %p: vmaddr = 0x%lx, vmsize = 0x%lx\n", segCmd, segCmd->vmaddr, segCmd->vmsize);
          DBG("PrelinkTextLoadCmdAddr = 0x%x, PrelinkTextAddr = 0x%x, PrelinkTextSize = 0x%x\n",
              PrelinkTextLoadCmdAddr, PrelinkTextAddr, PrelinkTextSize);
          //gBS->Stall(30*1000000);
        }
        if (AsciiStrCmp(segCmd->segname, kPrelinkInfoSegment) == 0) {
          UINT32 sectionIndex;
          struct section *sect;

          DBG("Found PRELINK_INFO, 32bit\n");
          //DBG("cmd = 0x%08x\n",segCmd->cmd);
          //DBG("cmdsize = 0x%08x\n",segCmd->cmdsize);
          DBG("vmaddr = 0x%08x\n",segCmd->vmaddr);
          DBG("vmsize = 0x%08x\n",segCmd->vmsize);
          //DBG("fileoff = 0x%08x\n",segCmd->fileoff);
          //DBG("filesize = 0x%08x\n",segCmd->filesize);
          //DBG("maxprot = 0x%08x\n",segCmd->maxprot);
          //DBG("initprot = 0x%08x\n",segCmd->initprot);
          //DBG("nsects = 0x%08x\n",segCmd->nsects);
          //DBG("flags = 0x%08x\n",segCmd->flags);
          sectionIndex = sizeof(struct segment_command);

          while(sectionIndex < segCmd->cmdsize) {
            sect = (struct section *)((UINT8*)segCmd + sectionIndex);
            sectionIndex += sizeof(struct section);

            if(AsciiStrCmp(sect->sectname, kPrelinkInfoSection) == 0 && AsciiStrCmp(sect->segname, kPrelinkInfoSegment) == 0) {
              if (sect->size > 0) {
                // PrelinkInfoAddr = sect->addr + KernelRelocBase
                PrelinkInfoLoadCmdAddr = (UINT32)(UINTN)sect;
                PrelinkInfoAddr = (UINT32)(sect->addr ? sect->addr + KernelRelocBase : 0);
                PrelinkInfoSize = (UINT32)sect->size;
              }
              DBG("__info found at %p: addr = 0x%lx, size = 0x%lx\n", sect, sect->addr, sect->size);
              DBG("PrelinkInfoLoadCmdAddr = 0x%x, PrelinkInfoAddr = 0x%x, PrelinkInfoSize = 0x%x\n",
                  PrelinkInfoLoadCmdAddr, PrelinkInfoAddr, PrelinkInfoSize);
              //gBS->Stall(30*1000000);
            }
          }
        }
        break;

      default:
        break;
    }
    binaryIndex += cmdsize;
  }

  //gBS->Stall(20*1000000);
  return;
}

VOID
FindBootArgs(IN LOADER_ENTRY *Entry)
{
  UINT8           *ptr;
  UINT8           archMode = sizeof(UINTN) * 8;

  // start searching from 0x200000.
  ptr = (UINT8*)(UINTN)0x200000;


  while(TRUE) {

    // check bootargs for 10.7 and up
    bootArgs2 = (BootArgs2*)ptr;

    if (bootArgs2->Version==2 && bootArgs2->Revision==0
        // plus additional checks - some values are not inited by boot.efi yet
        && bootArgs2->efiMode == archMode
        && bootArgs2->kaddr == 0 && bootArgs2->ksize == 0
        && bootArgs2->efiSystemTable == 0
        ) {
      // set vars
      dtRoot = (CHAR8*)(UINTN)bootArgs2->deviceTreeP;
      dtLength = &bootArgs2->deviceTreeLength;
      KernelSlide = bootArgs2->kslide;

      DBG_RT(Entry, "Found bootArgs2 at 0x%08x, DevTree at %p\n", ptr, dtRoot);
      //DBG("bootArgs2->kaddr = 0x%08x and bootArgs2->ksize =  0x%08x\n", bootArgs2->kaddr, bootArgs2->ksize);
      //DBG("bootArgs2->efiMode = 0x%02x\n", bootArgs2->efiMode);
      DBG_RT(Entry, "bootArgs2->CommandLine = %a\n", bootArgs2->CommandLine);
      DBG_RT(Entry, "bootArgs2->flags = 0x%x\n", bootArgs2->flags);
      DBG_RT(Entry, "bootArgs2->kslide = 0x%x\n", bootArgs2->kslide);
      DBG_RT(Entry, "bootArgs2->bootMemStart = 0x%x\n", bootArgs2->bootMemStart);
      if (Entry && Entry->KernelAndKextPatches && Entry->KernelAndKextPatches->KPDebug)
      gBS->Stall(2000000);

      // disable other pointer
      bootArgs1 = NULL;
      break;
    }

    // check bootargs for 10.4 - 10.6.x
    bootArgs1 = (BootArgs1*)ptr;

    if (bootArgs1->Version==1
        && (bootArgs1->Revision==6 || bootArgs1->Revision==5 || bootArgs1->Revision==4)
        // plus additional checks - some values are not inited by boot.efi yet
        && bootArgs1->efiMode == archMode
        && bootArgs1->kaddr == 0 && bootArgs1->ksize == 0
        && bootArgs1->efiSystemTable == 0
        ) {
      // set vars
      dtRoot = (CHAR8*)(UINTN)bootArgs1->deviceTreeP;
      dtLength = &bootArgs1->deviceTreeLength;

      DBG_RT(Entry, "Found bootArgs1 at 0x%08x, DevTree at %p\n", ptr, dtRoot);
      //DBG("bootArgs1->kaddr = 0x%08x and bootArgs1->ksize =  0x%08x\n", bootArgs1->kaddr, bootArgs1->ksize);
      //DBG("bootArgs1->efiMode = 0x%02x\n", bootArgs1->efiMode);

      // disable other pointer
      bootArgs2 = NULL;
      break;
    }

    ptr += 0x1000;
  }
}

BOOLEAN
KernelUserPatch(IN UINT8 *UKernelData, LOADER_ENTRY *Entry)
{
  INTN Num, i = 0, y = 0;
  for (; i < Entry->KernelAndKextPatches->NrKernels; ++i) {
    DBG_RT(Entry, "Patch[%d]: %a\n", i, Entry->KernelAndKextPatches->KernelPatches[i].Label);
    if (!Entry->KernelAndKextPatches->KernelPatches[i].MenuItem.BValue) {
      //DBG_RT(Entry, "Patch[%d]: %a :: is not allowed for booted OS %a\n", i, Entry->KernelAndKextPatches->KernelPatches[i].Label, Entry->OSVersion);
      DBG_RT(Entry, "==> disabled\n");
      continue;
    }

    Num = SearchAndReplaceMask(
                               UKernelData,
                               KERNEL_MAX_SIZE,
                               Entry->KernelAndKextPatches->KernelPatches[i].Data,
                               Entry->KernelAndKextPatches->KernelPatches[i].MaskFind,
                               Entry->KernelAndKextPatches->KernelPatches[i].DataLen,
                               Entry->KernelAndKextPatches->KernelPatches[i].Patch,
                               Entry->KernelAndKextPatches->KernelPatches[i].MaskReplace,
                               Entry->KernelAndKextPatches->KernelPatches[i].Count
                               );

    if (Num) {
      y++;
    }

    DBG_RT(Entry, "==> %a : %d replaces done\n", Num ? "Success" : "Error", Num);
  }
  if (Entry->KernelAndKextPatches->KPDebug) {
    gBS->Stall(2000000);
  }

  return (y != 0);
}

BOOLEAN
BooterPatch(IN UINT8 *BooterData, IN UINT64 BooterSize, LOADER_ENTRY *Entry)
{
  INTN Num, i = 0, y = 0;
  for (; i < Entry->KernelAndKextPatches->NrBoots; ++i) {
    DBG_RT(Entry, "Patch[%d]: %a\n", i, Entry->KernelAndKextPatches->BootPatches[i].Label);
    if (!Entry->KernelAndKextPatches->BootPatches[i].MenuItem.BValue) {
      DBG_RT(Entry, "==> disabled\n");
      continue;
    }
    
    Num = SearchAndReplaceMask(
                               BooterData,
                               BooterSize,
                               Entry->KernelAndKextPatches->BootPatches[i].Data,
                               Entry->KernelAndKextPatches->BootPatches[i].MaskFind,
                               Entry->KernelAndKextPatches->BootPatches[i].DataLen,
                               Entry->KernelAndKextPatches->BootPatches[i].Patch,
                               Entry->KernelAndKextPatches->BootPatches[i].MaskReplace,
                               Entry->KernelAndKextPatches->BootPatches[i].Count
                               );
    
    if (Num) {
      y++;
    }
    
    DBG_RT(Entry, "==> %a : %d replaces done\n", Num ? "Success" : "Error", Num);
  }
  if (Entry->KernelAndKextPatches->KPDebug) {
    gBS->Stall(2000000);
  }
  
  return (y != 0);
}

VOID
KernelAndKextPatcherInit(IN LOADER_ENTRY *Entry)
{
  if (PatcherInited) {
    return;
  }

  PatcherInited = TRUE;

  // KernelRelocBase will normally be 0
  // but if OsxAptioFixDrv is used, then it will be > 0
  SetKernelRelocBase();
  DBG("KernelRelocBase = %lx\n", KernelRelocBase);

  // Find bootArgs - we need then for proper detection
  // of kernel Mach-O header
  FindBootArgs(Entry);
  if (bootArgs1 == NULL && bootArgs2 == NULL) {
    DBG("BootArgs not found - skipping patches!\n");
    return;
  }

  // Find kernel Mach-O header:
  // for 10.4 - 10.5: 0x00111000
  // for 10.6 - 10.7: 0x00200000
  // for ML: bootArgs2->kslide + 0x00200000
  // for AptioFix booting - it's always at KernelRelocBase + 0x00200000

  UINT64 os_version = AsciiOSVersionToUint64(Entry->OSVersion);
  if (os_version < AsciiOSVersionToUint64("10.6")) {
    KernelData = (UINT8*)(UINTN)(KernelSlide + KernelRelocBase + 0x00111000);
  } else {
    KernelData = (UINT8*)(UINTN)(KernelSlide + KernelRelocBase + 0x00200000);
  }

  // check that it is Mach-O header and detect architecture
  if(MACH_GET_MAGIC(KernelData) == MH_MAGIC || MACH_GET_MAGIC(KernelData) == MH_CIGAM) {
    DBG("Found 32 bit kernel at 0x%p\n", KernelData);
    is64BitKernel = FALSE;
  } else if (MACH_GET_MAGIC(KernelData) == MH_MAGIC_64 || MACH_GET_MAGIC(KernelData) == MH_CIGAM_64) {
    DBG_RT(Entry, "Found 64 bit kernel at 0x%p\n", KernelData);
    is64BitKernel = TRUE;
  } else {
    // not valid Mach-O header - exiting
    DBG_RT(Entry, "Kernel not found at 0x%p - skipping patches!", KernelData);
    KernelData = NULL;
    return;
  }

  // find __PRELINK_TEXT and __PRELINK_INFO
  Get_PreLink();

  isKernelcache = PrelinkTextSize > 0 && PrelinkInfoSize > 0;
  DBG_RT(Entry, "isKernelcache: %s\n", isKernelcache ? L"Yes" : L"No");
}

VOID
KernelAndKextsPatcherStart(IN LOADER_ENTRY *Entry)
{
  BOOLEAN KextPatchesNeeded, patchedOk;

  // we will call KernelAndKextPatcherInit() only if needed
  if ((Entry == NULL) || (Entry->KernelAndKextPatches == NULL)) return;

  KextPatchesNeeded = (
    Entry->KernelAndKextPatches->KPAppleIntelCPUPM ||
    Entry->KernelAndKextPatches->KPAppleRTC ||
    Entry->KernelAndKextPatches->KPDELLSMBIOS ||
    (Entry->KernelAndKextPatches->KPATIConnectorsPatch != NULL) ||
    ((Entry->KernelAndKextPatches->NrKexts > 0) && (Entry->KernelAndKextPatches->KextPatches != NULL))
  );

  DBG_RT(Entry, "\nKernelToPatch: ");
  if (gSettings.KernelPatchesAllowed && (Entry->KernelAndKextPatches->KernelPatches != NULL) && Entry->KernelAndKextPatches->NrKernels) {
    DBG_RT(Entry, "Enabled: ");
    KernelAndKextPatcherInit(Entry);
    if (KernelData == NULL) goto NoKernelData;
    patchedOk = KernelUserPatch(KernelData, Entry);
    DBG_RT(Entry, patchedOk ? " OK\n" : " FAILED!\n");
  } else {
    DBG_RT(Entry, "Disabled\n");
  }

  DBG_RT(Entry, "\nKernelCpu patch: ");
  if (Entry->KernelAndKextPatches->KPKernelCpu) {
    //
    // Kernel patches
    //
    DBG_RT(Entry, "Enabled: ");
    KernelAndKextPatcherInit(Entry);
    if (KernelData == NULL) goto NoKernelData;
    if(is64BitKernel) {
      DBG_RT(Entry, "64 bit patch ...");
      KernelPatcher_64(KernelData, Entry);
    } else {
      DBG_RT(Entry, "32 bit patch ...");
      KernelPatcher_32(KernelData, Entry->OSVersion);
    }
    DBG_RT(Entry, " OK\n");
  } else {
    DBG_RT(Entry, "Disabled\n");
  }

  //other method for KernelCPU patch is FakeCPUID
  DBG_RT(Entry, "\nFakeCPUID patch: ");
  if (Entry->KernelAndKextPatches->FakeCPUID) {
    DBG_RT(Entry, "Enabled: 0x%06x\n", Entry->KernelAndKextPatches->FakeCPUID);
    KernelAndKextPatcherInit(Entry);
    if (KernelData == NULL) goto NoKernelData;
    KernelCPUIDPatch((UINT8*)KernelData, Entry);
  } else {
    DBG_RT(Entry, "Disabled\n");
  }

  // CPU power management patch for haswell with locked msr
  DBG_RT(Entry, "\nKernelPm patch: ");
  if (Entry->KernelAndKextPatches->KPKernelPm) {
    DBG_RT(Entry, "Enabled: ");
    KernelAndKextPatcherInit(Entry);
    if (KernelData == NULL) goto NoKernelData;
    patchedOk = FALSE;
    if (is64BitKernel) {
      patchedOk = KernelPatchPm(KernelData, Entry);
    }
    DBG_RT(Entry, patchedOk ? " OK\n" : " FAILED!\n");
  } else {
    DBG_RT(Entry, "Disabled\n");
  }
  
  // Patch to not dump kext at panic (c)vit9696
  DBG_RT(Entry, "\nPanicNoKextDump patch: ");
  if (Entry->KernelAndKextPatches->KPPanicNoKextDump) {
    DBG_RT(Entry, "Enabled: ");
    KernelAndKextPatcherInit(Entry);
    if (KernelData == NULL) goto NoKernelData;
    patchedOk = KernelPanicNoKextDump(KernelData);
    DBG_RT(Entry, patchedOk ? " OK\n" : " FAILED!\n");
  } else {
    DBG_RT(Entry, "Disabled\n");
  }


  // Lapic Panic Kernel Patch
  DBG_RT(Entry, "\nKernelLapic patch: ");
  if (Entry->KernelAndKextPatches->KPKernelLapic) {
    KernelAndKextPatcherInit(Entry);
    if (KernelData == NULL) goto NoKernelData;
    if(is64BitKernel) {
      DBG_RT(Entry, "64-bit patch ...");
      patchedOk = KernelLapicPatch_64(KernelData);
    } else {
      DBG_RT(Entry, "32-bit patch ...");
      patchedOk = KernelLapicPatch_32(KernelData);
    }
    DBG_RT(Entry, patchedOk ? " OK\n" : " FAILED!\n");
  } else {
    DBG_RT(Entry, "Disabled\n");
  }

  if (Entry->KernelAndKextPatches->KPKernelXCPM) {
    //
    // syscl - EnableExtCpuXCPM: Enable unsupported CPU's PowerManagement
    //
    EnableExtCpuXCPM = NULL;
    patchedOk = FALSE;
    if (gCPUStructure.Vendor == CPU_VENDOR_INTEL) {
      switch (gCPUStructure.Model) {
          case CPU_MODEL_JAKETOWN:
            // SandyBridge-E LGA2011
            EnableExtCpuXCPM = SandyBridgeEPM;
            gSNBEAICPUFixRequire = TRUE;       // turn on SandyBridge-E AppleIntelCPUPowerManagement Fix
            break;
              
          case CPU_MODEL_IVY_BRIDGE:
            // IvyBridge
            EnableExtCpuXCPM = KernelIvyBridgeXCPM;
            break;
              
          case CPU_MODEL_IVY_BRIDGE_E5:
            // IvyBridge-E
            EnableExtCpuXCPM = KernelIvyE5XCPM;
            break;

          case CPU_MODEL_HASWELL_E:
            // Haswell-E
            EnableExtCpuXCPM = HaswellEXCPM;
            break;
              
          case CPU_MODEL_BROADWELL_E5:
          case CPU_MODEL_BROADWELL_DE:
            // Broadwell-E/EP
            EnableExtCpuXCPM = BroadwellEPM;
            gBDWEIOPCIFixRequire = TRUE;
            break;

          default:
            if (gCPUStructure.Model >= CPU_MODEL_HASWELL &&
               (AsciiStrStr(gCPUStructure.BrandString, "Celeron") || AsciiStrStr(gCPUStructure.BrandString, "Pentium"))) {
              // Haswell+ low-end CPU
              EnableExtCpuXCPM = HaswellLowEndXCPM;
            }
            break;
      }
        
      // syscl - now enable extra Cpu's PowerManagement
      // only Intel support this feature till now
      // move below code outside the if condition if AMD supports
      // XCPM later on
      if (EnableExtCpuXCPM) {
        BOOLEAN apply_idle_patch = (gCPUStructure.Model >= CPU_MODEL_SKYLAKE_U) && gSettings.HWP;
        KernelAndKextPatcherInit(Entry);
        if (KernelData == NULL) goto NoKernelData;
        patchedOk = EnableExtCpuXCPM(KernelData, Entry, apply_idle_patch);
      }
    }
    DBG_RT(Entry, "EnableExtCpuXCPM - %a!\n", patchedOk? "OK" : "FAILED");
  }

  if (Entry->KernelAndKextPatches->KPDebug) {
    gBS->Stall(2000000);
  }

  //
  // Kext patches
  //

  // we need to scan kexts if "InjectKexts true and CheckFakeSMC"
  if (/*OSFLAG_ISSET(Entry->Flags, OSFLAG_WITHKEXTS) || */
      OSFLAG_ISSET(Entry->Flags, OSFLAG_CHECKFAKESMC)) {
    DBG_RT(Entry, "\nAllowing kext patching to check if FakeSMC is present\n");
    gSettings.KextPatchesAllowed = TRUE;
    KextPatchesNeeded = TRUE;
  }

  DBG_RT(Entry, "\nKextPatches Needed: %c, Allowed: %c ... ",
         (KextPatchesNeeded ? L'Y' : L'n'),
         (gSettings.KextPatchesAllowed ? L'Y' : L'n')
         );

  if (KextPatchesNeeded && gSettings.KextPatchesAllowed) {
    KernelAndKextPatcherInit(Entry);
    if (KernelData == NULL) goto NoKernelData;
    DBG_RT(Entry, "\nKext patching STARTED\n");
    KextPatcherStart(Entry);  //is FakeSMC found in cache then inject will be disabled
    DBG_RT(Entry, "\nKext patching ENDED\n");
  } else {
    DBG_RT(Entry, "Disabled\n");
  }

  if (Entry->KernelAndKextPatches->KPDebug) {
    DBG_RT(Entry, "Pausing 10 secs ...\n\n");
    gBS->Stall(10000000);
  }

  //
  // Kext add
  //
  if (Entry->KernelAndKextPatches->KPDebug) {
    if (OSFLAG_ISSET(Entry->Flags, OSFLAG_CHECKFAKESMC) &&
        OSFLAG_ISUNSET(Entry->Flags, OSFLAG_WITHKEXTS)) {
    // disabled kext injection if FakeSMC is already present
 //   Entry->Flags = OSFLAG_UNSET(Entry->Flags, OSFLAG_WITHKEXTS); //Slice - we are already here
    
      DBG_RT(Entry, "\nInjectKexts: disabled because FakeSMC is already present and InjectKexts option set to Detect\n");
      gBS->Stall(500000);
    }
  }

  if (OSFLAG_ISSET(Entry->Flags, OSFLAG_WITHKEXTS)) {
    UINT32      deviceTreeP;
    UINT32      deviceTreeLength;
    EFI_STATUS  Status;
    UINTN       DataSize;

    // check if FSInject already injected kexts
    DataSize = 0;
    Status = gRT->GetVariable (L"FSInject.KextsInjected", &gEfiGlobalVariableGuid, NULL, &DataSize, NULL);
    if (Status == EFI_BUFFER_TOO_SMALL) {
      // var exists - just exit
      if (Entry->KernelAndKextPatches->KPDebug) {
        DBG_RT(Entry, "\nInjectKexts: skipping, FSInject already injected them\n");
        gBS->Stall(500000);
      }
      return;
    }

    KernelAndKextPatcherInit(Entry);
    if (KernelData == NULL) goto NoKernelData;
    if (bootArgs1 != NULL) {
      deviceTreeP = bootArgs1->deviceTreeP;
      deviceTreeLength = bootArgs1->deviceTreeLength;
    } else if (bootArgs2 != NULL) {
      deviceTreeP = bootArgs2->deviceTreeP;
      deviceTreeLength = bootArgs2->deviceTreeLength;
    } else return;

    Status = InjectKexts(deviceTreeP, &deviceTreeLength, Entry);

    if (!EFI_ERROR(Status)) KernelBooterExtensionsPatch(KernelData, Entry);
  }

  return;

NoKernelData:
  if (Entry->KernelAndKextPatches->KPDebug) {
    DBG_RT(Entry, "==> ERROR: Kernel not found\n");
    gBS->Stall(5000000);
  }
}
