/*
 * Copyright (c) 2011-2012 Frank Peng. All rights reserved.
 *
 */

#include "Platform.h"
#include "LoaderUefi.h"
#include "device_tree.h"

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
#define DBG_RT(...)    if (gSettings.KPDebug) { AsciiPrint(__VA_ARGS__); }


EFI_PHYSICAL_ADDRESS    KernelRelocBase = 0;
BootArgs1   *bootArgs1 = NULL;
BootArgs2   *bootArgs2 = NULL;
CHAR8       *dtRoot = NULL;
VOID        *KernelData = NULL;
UINT32      KernelSlide = 0;
BOOLEAN     isKernelcache = FALSE;
BOOLEAN     is64BitKernel = FALSE;
BOOLEAN     SSSE3;

BOOLEAN     PatcherInited = FALSE;

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


/*
typedef struct kernSymbols_t
{
    CHAR8* symbol;
    UINT64 addr;
    struct kernSymbols_t* next;
} kernSymbols_t;

kernSymbols_t* kernelSymbols = NULL;

VOID register_kernel_symbol(CONST CHAR8* name)
{
    if(kernelSymbols == NULL)
    {
        kernelSymbols = AllocateZeroPool(sizeof(kernSymbols_t));
        kernelSymbols->next = NULL;
        kernelSymbols->symbol = (CHAR8*)name;
        kernelSymbols->addr = 0;
    }
    else 
    {
        kernSymbols_t *symbol = kernelSymbols;
        while(symbol->next != NULL)
        {
            symbol = symbol->next;
        }
        
        symbol->next = AllocateZeroPool(sizeof(kernSymbols_t));
        symbol = symbol->next;

        symbol->next = NULL;
        symbol->symbol = (CHAR8*)name;
        symbol->addr = 0;
    }
}

kernSymbols_t* lookup_kernel_symbol(CONST CHAR8* name)
{
    kernSymbols_t *symbol = kernelSymbols;

    while(symbol && (AsciiStrCmp(symbol->symbol, name)!=0))
    {
        symbol = symbol->next;
    }
    
    if(!symbol)
    {
        return NULL;
    }
    else
    {
        return symbol;
    }

}

UINT64 symbol_handler(CHAR8* symbolName, UINT64 addr)
{
    // Locate the symbol in the list, if it exists, update it's address
    kernSymbols_t *symbol = lookup_kernel_symbol(symbolName);
    
    if(symbol)
    {
        symbol->addr = addr;
    }
    
    return 0x7FFFFFFF; // fixme
}

INTN locate_symbols(VOID* kernelData)
{
    //DecodeMachO(kernelData);
    return 1;
}

*/

VOID SetKernelRelocBase()
{
    EFI_STATUS      Status;
    UINTN           DataSize = sizeof(KernelRelocBase);
    
    KernelRelocBase = 0;
    // OsxAptioFixDrv will set this
    Status = gRT->GetVariable(L"OsxAptioFixDrv-RelocBase", &gEfiAppleBootGuid, NULL, &DataSize, &KernelRelocBase);
    // KernelRelocBase is now either read or 0
    return;
}



VOID KernelPatcher_64(VOID* kernelData)
{
    BOOLEAN     check = TRUE;
     
    UINT8       *bytes = (UINT8*)kernelData;
    UINT32      patchLocation=0, patchLocation1=0;
    UINT32      i;
    UINT32      jumpaddr;
    UINT32      cpuid_family_addr;
    
    //if (AsciiStrnCmp(OSVersion,"10.7",4)==0) return;
        
    DBG("Found _cpuid_set_info _panic Start\n");
    // _cpuid_set_info _panic address
    for (i=0; i<0x1000000; i++) 
    {   
        if (bytes[i] == 0xC7 && bytes[i+1] == 0x05 && bytes[i+6] == 0x07 && bytes[i+7] == 0x00 &&
            bytes[i+8] == 0x00 && bytes[i+9] == 0x00 && bytes[i+10] == 0xC7 && bytes[i+11] == 0x05 &&
            bytes[i-5] == 0xE8)
        {
            patchLocation = i-5;
            DBG("Found _cpuid_set_info _panic address at 0x%08x\n",patchLocation);
            break;
        }
    }
     
    if (!patchLocation)
    {
         DBG("Can't find _cpuid_set_info _panic address, patch kernel abort.\n",i);
         return;
    }
    
    // this for 10.6.0 and 10.6.1 kernel and remove tsc.c unknow cpufamily panic
    //  488d3df4632a00
    // find _tsc_init panic address
    for (i=0; i<0x1000000; i++) 
    {   // _cpuid_set_info _panic address
        if (bytes[i] == 0x48 && bytes[i+1] == 0x8D && bytes[i+2] == 0x3D && bytes[i+3] == 0xF4 &&
            bytes[i+4] == 0x63 && bytes[i+5] == 0x2A && bytes[i+6] == 0x00)
        {
            patchLocation1 = i+9;
            DBG("Found _tsc_init _panic address at 0x%08x\n",patchLocation1);
            break;
        }
    }
    
    // found _tsc_init panic addres and patch it
    if (patchLocation1) 
    {
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
         
    for (i=0;i<500;i++) 
    {
        if( bytes[jumpaddr-i-4] == 0x85 && bytes[jumpaddr-i-3] == 0xC0 &&
            bytes[jumpaddr-i-2] == 0x0f )
        {
            jumpaddr -= i;
            bytes[jumpaddr-1] = 0x87;
            bytes[jumpaddr] -= 10;
            break;
        }
    }
    
    if (jumpaddr == patchLocation) 
    {
        for(i=0;i<500;i++) 
        {
            if( bytes[jumpaddr-i-3] == 0x85 && bytes[jumpaddr-i-2] == 0xC0 &&
                bytes[jumpaddr-i-1] == 0x75 )
            {
                jumpaddr -= i;
                bytes[jumpaddr-1] = 0x77;
                check = FALSE;
                break;
            }
        }    
    }
    
    if (jumpaddr == patchLocation) 
    {
        DBG("Can't Found jumpaddr address.\n");
        return;  //can't find jump location
    }
    
    if (check) 
    {
        cpuid_family_addr = bytes[jumpaddr + 6] <<  0 |
                            bytes[jumpaddr + 7] <<  8 |
                            bytes[jumpaddr + 8] << 16 |
                            bytes[jumpaddr + 9] << 24;
    }
    else
    {
        cpuid_family_addr = bytes[jumpaddr + 3] <<  0 |
                            bytes[jumpaddr + 4] <<  8 |
                            bytes[jumpaddr + 5] << 16 |
                            bytes[jumpaddr + 6] << 24;
    }
       
    if (check) 
    {
        bytes[patchLocation - 13] = (CPUFAMILY_INTEL_YONAH & 0x000000FF) >>  0;
        bytes[patchLocation - 12] = (CPUFAMILY_INTEL_YONAH & 0x0000FF00) >>  8;
        bytes[patchLocation - 11] = (CPUFAMILY_INTEL_YONAH & 0x00FF0000) >> 16;
        bytes[patchLocation - 10] = (CPUFAMILY_INTEL_YONAH & 0xFF000000) >> 24;
    }
        
    if (check && (AsciiStrnCmp(OSVersion,"10.6.8",6)!=0) && (AsciiStrnCmp(OSVersion,"10.7",4)==0))
        cpuid_family_addr -= 255;
    
    if (!check) 
        cpuid_family_addr += 10;
    
    if (AsciiStrnCmp(OSVersion,"10.6.8",6)==0) goto SE3;
    
    //patch info->cpuid_cpufamily
    bytes[patchLocation -  9] = 0x90;
    bytes[patchLocation -  8] = 0x90;
    
    bytes[patchLocation -  7] = 0xC7;
    bytes[patchLocation -  6] = 0x05;

    bytes[patchLocation -  5] = (cpuid_family_addr & 0x000000FF);
    bytes[patchLocation -  4] = (UINT8)((cpuid_family_addr & 0x0000FF00) >>  8);
    bytes[patchLocation -  3] = (UINT8)((cpuid_family_addr & 0x00FF0000) >> 16);
    bytes[patchLocation -  2] = (UINT8)((cpuid_family_addr & 0xFF000000) >> 24);
    
    bytes[patchLocation -  1] = CPUIDFAMILY_DEFAULT; //cpuid_family need alway set 0x06
    bytes[patchLocation +  0] = CPUID_MODEL_YONAH;   //cpuid_model set CPUID_MODEL_MEROM
    bytes[patchLocation +  1] = 0x01;                //cpuid_extmodel alway set 0x01
    bytes[patchLocation +  2] = 0x00;                //cpuid_extfamily alway set 0x00
    bytes[patchLocation +  3] = 0x90;                
    bytes[patchLocation +  4] = 0x90;
SE3:
    // patch sse3
    if (!SSSE3 && (AsciiStrnCmp(OSVersion,"10.6",4)==0))
    {
        Patcher_SSE3_6((VOID*)bytes);
    }
    if (!SSSE3 && (AsciiStrnCmp(OSVersion,"10.7",4)==0))
    {
        Patcher_SSE3_7((VOID*)bytes);
    }

}

VOID KernelPatcher_32(VOID* kernelData)
{
    UINT8* bytes = (UINT8*)kernelData;
    UINT32 patchLocation=0, patchLocation1=0;
    UINT32 i;
    UINT32 jumpaddr;
        
    DBG("Found _cpuid_set_info _panic Start\n");
    // _cpuid_set_info _panic address
    for (i=0; i<0x1000000; i++) 
    {   
        if (bytes[i] == 0xC7 && bytes[i+1] == 0x05 && bytes[i+6] == 0x07 && bytes[i+7] == 0x00 &&
            bytes[i+8] == 0x00 && bytes[i+9] == 0x00 && bytes[i+10] == 0xC7 && bytes[i+11] == 0x05 &&
            bytes[i-5] == 0xE8)
        {
            patchLocation = i-5;
            DBG("Found _cpuid_set_info _panic address at 0x%08x\n",patchLocation);
            break;
        }
    }
     
    if (!patchLocation)
    {
         DBG("Can't find _cpuid_set_info _panic address, patch kernel abort.\n",i);
         return;
    }
    
    // this for 10.6.0 and 10.6.1 kernel and remove tsc.c unknow cpufamily panic
    //  c70424540e5900
    // find _tsc_init panic address
    for (i=0; i<0x1000000; i++) 
    {   // _cpuid_set_info _panic address
        if (bytes[i] == 0xC7 && bytes[i+1] == 0x04 && bytes[i+2] == 0x24 && bytes[i+3] == 0x54 &&
            bytes[i+4] == 0x0E && bytes[i+5] == 0x59 && bytes[i+6] == 0x00)
        {
            patchLocation1 = i+7;
            DBG("Found _tsc_init _panic address at 0x%08x\n",patchLocation1);
            break;
        }
    }
    
    // found _tsc_init panic addres and patch it
    if (patchLocation1) 
    {
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
     
    for (i=0;i<500;i++) 
    {
        if (bytes[jumpaddr-i-3] == 0x85 && bytes[jumpaddr-i-2] == 0xC0 &&
            bytes[jumpaddr-i-1] == 0x75 )
        {
             jumpaddr -= i;
             bytes[jumpaddr-1] = 0x77;
             if(bytes[patchLocation - 17] == 0xC7)
                 bytes[jumpaddr] -=10;
             
             break;
        } 
    }

    if (jumpaddr == patchLocation) 
    {
        DBG("Can't Found jumpaddr address.\n");
        return;  //can't find jump location
    }
    // patch info_p->cpufamily to CPUFAMILY_INTEL_MEROM

    if (bytes[patchLocation - 17] == 0xC7) 
    {
        bytes[patchLocation - 11] = (CPUFAMILY_INTEL_YONAH & 0x000000FF) >>  0;
        bytes[patchLocation - 10] = (CPUFAMILY_INTEL_YONAH & 0x0000FF00) >>  8;
        bytes[patchLocation -  9] = (CPUFAMILY_INTEL_YONAH & 0x00FF0000) >> 16;
        bytes[patchLocation -  8] = (CPUFAMILY_INTEL_YONAH & 0xFF000000) >> 24;
    } 
    
    //patch info->cpuid_cpufamily
    bytes[patchLocation -  7] = 0xC7;
    bytes[patchLocation -  6] = 0x05;
    bytes[patchLocation -  5] = bytes[jumpaddr + 3];
    bytes[patchLocation -  4] = bytes[jumpaddr + 4];
    bytes[patchLocation -  3] = bytes[jumpaddr + 5];
    bytes[patchLocation -  2] = bytes[jumpaddr + 6];
    
    bytes[patchLocation -  1] = CPUIDFAMILY_DEFAULT; //cpuid_family  need alway set 0x06
    bytes[patchLocation +  0] = CPUID_MODEL_YONAH;   //cpuid_model set CPUID_MODEL_MEROM
    bytes[patchLocation +  1] = 0x01;                //cpuid_extmodel alway set 0x01
    bytes[patchLocation +  2] = 0x00;                //cpuid_extfamily alway set 0x00
    bytes[patchLocation +  3] = 0x90;
    bytes[patchLocation +  4] = 0x90;
    
    if (AsciiStrnCmp(OSVersion,"10.7",4)==0) return;
    
    if (!SSSE3 && (AsciiStrnCmp(OSVersion,"10.6",4)==0))
    {
        Patcher_SSE3_6((VOID*)bytes);
    }
    if (!SSSE3 && (AsciiStrnCmp(OSVersion,"10.5",4)==0))
    {
        Patcher_SSE3_5((VOID*)bytes);
    } 
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
    while(TRUE)
    {
        if (bytes[i] == 0x66 && bytes[i+1] == 0x0F && bytes[i+2] == 0x6F && 
            bytes[i+3] == 0x44 && bytes[i+4] == 0x0E && bytes[i+5] == 0xF1 &&
            bytes[i-1664-32] == 0x55
            )
        {
            patchLocation1 = i-1664-32;
            DBG("Found SSE3 data address at 0x%08x\n",patchLocation1);
        }

         // khasSSE2+..... title
        if (bytes[i] == 0xE3 && bytes[i+1] == 0x07 && bytes[i+2] == 0x00 &&
            bytes[i+3] == 0x00 && bytes[i+4] == 0x80 && bytes[i+5] == 0x07 &&
            bytes[i+6] == 0xFF && bytes[i+7] == 0xFF && bytes[i+8] == 0x24 &&
            bytes[i+9] == 0x01)
        {
           patchLocation2 = i;
           DBG("Found SSE3 Title address at 0x%08x\n",patchLocation2);
           break;
        }
        i++;
    }            
    
    if (!patchLocation1 || !patchLocation2) 
    {
        DBG("Can't found SSE3 data addres or Title address at 0x%08x 0x%08x\n", patchLocation1, patchLocation2);
        return;
    }
     
    DBG("Found SSE3 last data addres Start\n");
    i = patchLocation1 + 1500;
    //for (i=(patchLocation1+1500); i<(patchLocation1+3000); i++)
    while(TRUE)
    {
        if (bytes[i] == 0x90 && bytes[i+1] == 0x90 && bytes[i+2] == 0x55 ) 
        {
            patchlast = (i+1) - patchLocation1;
            DBG("Found SSE3 last data addres at 0x%08x\n", patchlast);
            break;
        }
        i++;
    }
     
    if (!patchlast)
    {
        DBG("Can't found SSE3 data last addres at 0x%08x\n", patchlast);
        return;
    }
    // patch sse3_64 data

    for (i=0; i<patchlast; i++) 
    {
        if (i<sizeof(sse3_patcher)) 
        {
            bytes[patchLocation1 + i] = sse3_patcher[i];
        } 
        else 
        {
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
    
    for (i=256; i<(Length-256); i++) 
    {
        if (bytes[i] == 0x66 && bytes[i+1] == 0x0F && bytes[i+2] == 0x6F && 
            bytes[i+3] == 0x44 && bytes[i+4] == 0x0E && bytes[i+5] == 0xF1 &&
            bytes[i-1680-32] == 0x55)
        {
            patchLocation1 = i-1680-32;
            DBG("Found SSE3 data address at 0x%08x\n",patchLocation1);
        }

        // khasSSE2+..... title
        if (bytes[i] == 0xF3 && bytes[i+1] == 0x07 && bytes[i+2] == 0x00 &&
            bytes[i+3] == 0x00 && bytes[i+4] == 0x80 && bytes[i+5] == 0x07 &&
            bytes[i+6] == 0xFF && bytes[i+7] == 0xFF && bytes[i+8] == 0x24 &&
            bytes[i+9] == 0x01)
        {
           patchLocation2 = i;
           DBG("Found SSE3 Title address at 0x%08x\n",patchLocation2);
           break;
        }
    }            
    
    if (!patchLocation1 || !patchLocation2) 
    {
        DBG("Can't found SSE3 data addres or Title address at 0x%08x 0x%08x\n", patchLocation1, patchLocation2);
        return;
    }
     
    DBG("Found SSE3 last data addres Start\n");
          
    for (i=(patchLocation1+1500);i<Length;i++)
    {
       if (bytes[i] == 0x90 && bytes[i+1] == 0x90 && bytes[i+2] == 0x55) 
       {
           patchlast = (i+1) - patchLocation1;
            DBG("Found SSE3 last data addres at 0x%08x\n", patchlast);
           break;
       }
    }

    if (!patchlast)
    {
        DBG("Can't found SSE3 data last addres at 0x%08x\n", patchlast);
        return;
    }

    // patech sse3_64 data

    for (i=0; i<patchlast; i++) 
    {
        if (i<sizeof(sse3_5_patcher)) 
        {
            bytes[patchLocation1 + i] = sse3_5_patcher[i];
        } 
        else 
        {
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
  struct load_command         *loadCommand;
  struct  segment_command     *segCmd;
  struct segment_command_64   *segCmd64;
  
  
  if (is64BitKernel) {
    binaryIndex = sizeof(struct mach_header_64);
  } else {
    binaryIndex = sizeof(struct mach_header);
  }
  
  ncmds = MACH_GET_NCMDS(binary);
  
  for (cnt = 0; cnt < ncmds; cnt++) {
    loadCommand = (struct load_command *)(binary + binaryIndex);
    cmdsize = loadCommand->cmdsize;
    
    switch (loadCommand->cmd) 
    {
      case LC_SEGMENT_64: 
        segCmd64 = (struct segment_command_64 *)loadCommand;
        //DBG("segCmd64->segname = %a\n",segCmd64->segname);
        //DBG("segCmd64->vmaddr = 0x%08x\n",segCmd64->vmaddr)
        //DBG("segCmd64->vmsize = 0x%08x\n",segCmd64->vmsize); 
        if (AsciiStrCmp(segCmd64->segname, kPrelinkTextSegment) == 0)
        {
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
        if (AsciiStrCmp(segCmd64->segname, kPrelinkInfoSegment) == 0)
        {
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
          
          while(sectionIndex < segCmd64->cmdsize)
          {
            sect = (struct section_64 *)((UINT8*)segCmd64 + sectionIndex);
            sectionIndex += sizeof(struct section_64);
            
            if(AsciiStrCmp(sect->sectname, kPrelinkInfoSection) == 0 && AsciiStrCmp(sect->segname, kPrelinkInfoSegment) == 0)
            {
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
        if (AsciiStrCmp(segCmd->segname, kPrelinkTextSegment) == 0)
        {
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
        if (AsciiStrCmp(segCmd->segname, kPrelinkInfoSegment) == 0)
        {
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
          
          while(sectionIndex < segCmd->cmdsize)
          {
            sect = (struct section *)((UINT8*)segCmd + sectionIndex);
            sectionIndex += sizeof(struct section);
            
            if(AsciiStrCmp(sect->sectname, kPrelinkInfoSection) == 0 && AsciiStrCmp(sect->segname, kPrelinkInfoSegment) == 0)
            {
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
FindBootArgs(VOID)
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
        )
    {
      // set vars
      dtRoot = (CHAR8*)(UINTN)bootArgs2->deviceTreeP;
      KernelSlide = bootArgs2->kslide;
      
      DBG("Found bootArgs2 at 0x%08x, DevTree at %p\n", ptr, dtRoot);
      //DBG("bootArgs2->kaddr = 0x%08x and bootArgs2->ksize =  0x%08x\n", bootArgs2->kaddr, bootArgs2->ksize);
      //DBG("bootArgs2->efiMode = 0x%02x\n", bootArgs2->efiMode);
      DBG("bootArgs2->CommandLine = %a\n", bootArgs2->CommandLine);
      DBG("bootArgs2->flags = %x %x\n", bootArgs2->flags);
      DBG("bootArgs2->kslide = %x\n", bootArgs2->kslide);
      //gBS->Stall(5000000);
      
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
        )
    {
      // set vars
      dtRoot = (CHAR8*)(UINTN)bootArgs1->deviceTreeP;
      
      DBG("Found bootArgs1 at 0x%08x, DevTree at %p\n", ptr, dtRoot);
      //DBG("bootArgs1->kaddr = 0x%08x and bootArgs1->ksize =  0x%08x\n", bootArgs1->kaddr, bootArgs1->ksize);
      //DBG("bootArgs1->efiMode = 0x%02x\n", bootArgs1->efiMode);
      
      // disable other pointer
      bootArgs2 = NULL;
      break;
    }
    
    ptr += 0x1000;
  }
}

VOID
KernelAndKextPatcherInit(VOID)
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
  FindBootArgs();
  if (bootArgs1 == NULL && bootArgs2 == NULL) {
    DBG("BootArgs not found - skipping patches!\n");
    return;
  }
  
  // Find kernel Mach-O header:
  // for ML: bootArgs2->kslide + 0x00200000
  // for older versions: just 0x200000
  // for AptioFix booting - it's always at KernelRelocBase + 0x200000
  KernelData = (VOID*)(UINTN)(KernelSlide + KernelRelocBase + 0x00200000);
  
  // check that it is Mach-O header and detect architecture
  if(MACH_GET_MAGIC(KernelData) == MH_MAGIC || MACH_GET_MAGIC(KernelData) == MH_CIGAM)
  {
    DBG("Found 32 bit kernel at 0x%p\n", KernelData);
    is64BitKernel = FALSE;
  }
  else if(MACH_GET_MAGIC(KernelData) == MH_MAGIC_64 || MACH_GET_MAGIC(KernelData) == MH_CIGAM_64)
  {
    DBG("Found 64 bit kernel at 0x%p\n", KernelData);
    is64BitKernel = TRUE;
  }
  else {
    // not valid Mach-O header - exiting
    DBG("Kernel not found at 0x%p - skipping patches!", KernelData);
    KernelData = NULL;
    return;
  }
  
  // find __PRELINK_TEXT and __PRELINK_INFO
  Get_PreLink();
  
  
  isKernelcache = PrelinkTextSize > 0 && PrelinkInfoSize > 0;
  DBG("isKernelcache: %s\n", isKernelcache ? L"Yes" : L"No");
}

VOID
KernelAndKextsPatcherStart(VOID)
{
  
  // we will call KernelAndKextPatcherInit() only if needed
  
  DBG_RT("\nKernelCpu patch: ");
  if (gSettings.KPKernelCpu) {
    
    //
    // Kernel patches
    //
    DBG_RT("Enabled: ");
    if ((gCPUStructure.Family!=0x06 && AsciiStrStr(OSVersion,"10.7")!=0)||
        (gCPUStructure.Model==CPU_MODEL_ATOM &&
         ((AsciiStrStr(OSVersion,"10.7")!=0) || AsciiStrStr(OSVersion,"10.6")!=0)) ||
        (gCPUStructure.Model==CPU_MODEL_IVY_BRIDGE && AsciiStrStr(OSVersion,"10.7")!=0) ||
        (gCPUStructure.Model==CPU_MODEL_IVY_BRIDGE_E5 && AsciiStrStr(OSVersion,"10.7")!=0)
        )
    {
      KernelAndKextPatcherInit();
      if (KernelData == NULL) {
        if (gSettings.KPDebug) {
          DBG_RT("ERROR: Kernel not found\n");
          gBS->Stall(5000000);
        }
        return;
      }
      
      if(is64BitKernel) {
        DBG_RT("64 bit patch ...");
        KernelPatcher_64(KernelData);
      } else {
        DBG_RT("32 bit patch ...");
        KernelPatcher_32(KernelData);
      }
      DBG_RT(" OK\n");
    } else {
      DBG_RT(" Not executed!\n");
    }
  } else {
    DBG_RT("Not done - Disabled.\n");
  }
  if (gSettings.KPDebug) {
    gBS->Stall(5000000);
  }
  
  //
  // Kext patches
  //
  DBG_RT("\nKextPatches Needed: %c, Allowed: %c ... ",
         (gSettings.KPKextPatchesNeeded ? L'Y' : L'n'),
         (gSettings.KextPatchesAllowed ? L'Y' : L'n')
         );
  if (gSettings.KPKextPatchesNeeded && gSettings.KextPatchesAllowed) {
    KernelAndKextPatcherInit();
    if (KernelData == NULL) {
      if (gSettings.KPDebug) {
        DBG_RT("ERROR: Kernel not found\n");
        gBS->Stall(5000000);
      }
      return;
    }
    
    DBG_RT("\nKext patching STARTED\n");
    KextPatcherStart();
    DBG_RT("\nKext patching ENDED\n");
    
  } else {
    DBG_RT("Not needed or not allowed\n");
  }
  if (gSettings.KPDebug) {
    DBG_RT("Pausing 10 secs ...\n\n");
    gBS->Stall(10000000);
  }

  //
  // Kext add
  //
  if (gSettings.BootArgs != NULL
      && (AsciiStrStr(gSettings.BootArgs, "WithKexts") != NULL)
      )
  {
    UINT32      deviceTreeP;
    UINT32      deviceTreeLength;
    EFI_STATUS  Status;
    UINTN       DataSize;

    // check if FSInject already injected kexts
    DataSize = 0;
    Status = gRT->GetVariable (L"FSInject.KextsInjected", &gEfiGlobalVariableGuid, NULL, &DataSize, NULL);
    if (Status == EFI_BUFFER_TOO_SMALL) {
      // var exists - just exit
      if (gSettings.KPDebug) {
        DBG_RT("\nInjectKexts: skipping, FSInject already injected them\n");
        gBS->Stall(5000000);
      }
      return;
    }
    
    KernelAndKextPatcherInit();
    if (KernelData == NULL) {
      return;
    }

    if (bootArgs1 != NULL) {
        deviceTreeP = bootArgs1->deviceTreeP;
        deviceTreeLength = bootArgs1->deviceTreeLength;
    } else if (bootArgs2 != NULL) {
        deviceTreeP = bootArgs2->deviceTreeP;
        deviceTreeLength = bootArgs2->deviceTreeLength;
    } else return;

    Status = InjectKexts(deviceTreeP, &deviceTreeLength);

    if (!EFI_ERROR(Status)) KernelBooterExtensionsPatch(KernelData);
  }
  
}
