/*
 * Copyright (c) 2011-2012 Frank Peng. All rights reserved.
 *
 */

#include "kernel_patcher.h"
#include "sse3_patcher.h"
#include "sse3_5_patcher.h"

#define KERNEL_DEBUG 0

#if KERNEL_DEBUG
#define DBG(x...)	Print(x);
#else
#define DBG(x...)
#endif

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

VOID KernelPatcher_64(VOID* kernelData)
{
    BOOLEAN check = TRUE;
     
    UINT8* bytes = (UINT8*)kernelData;
    UINT32 patchLocation=0, patchLocation1=0;
    UINT32 i;
    
    if (AsciiStrnCmp(OSVersion,"10.7",4)==0) return;
        
    DBG(L"Found _cpuid_set_info _panic Start\n");
    // _cpuid_set_info _panic address
    for (i=0; i<0x1000000; i++)	 
    {   
        if (bytes[i] == 0xC7 && bytes[i+1] == 0x05 && bytes[i+6] == 0x07 && bytes[i+7] == 0x00 &&
            bytes[i+8] == 0x00 && bytes[i+9] == 0x00 && bytes[i+10] == 0xC7 && bytes[i+11] == 0x05 &&
            bytes[i-5] == 0xE8)
        {
            patchLocation = i-5;
            DBG(L"Found _cpuid_set_info _panic address at 0x%08x\n",patchLocation);
            break;
        }
    }
	 
	if (!patchLocation)
	{
	     DBG(L"Can't find _cpuid_set_info _panic address, patch kernel abort.\n",i);
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
            DBG(L"Found _tsc_init _panic address at 0x%08x\n",patchLocation1);
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
	 	 
    UINT32 jumpaddr = patchLocation;
		 
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
        DBG(L"Can't Found jumpaddr address.\n");
        return;  //can't find jump location
    }
    
    UINT32 cpuid_family_addr;
    
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

	bytes[patchLocation -  5] = (cpuid_family_addr & 0x000000FF) >>  0;	
	bytes[patchLocation -  4] = (cpuid_family_addr & 0x0000FF00) >>  8;	
	bytes[patchLocation -  3] = (cpuid_family_addr & 0x00FF0000) >> 16;
	bytes[patchLocation -  2] = (cpuid_family_addr & 0xFF000000) >> 24;
    
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
        
    DBG(L"Found _cpuid_set_info _panic Start\n");
    // _cpuid_set_info _panic address
    for (i=0; i<0x1000000; i++)	 
    {   
        if (bytes[i] == 0xC7 && bytes[i+1] == 0x05 && bytes[i+6] == 0x07 && bytes[i+7] == 0x00 &&
            bytes[i+8] == 0x00 && bytes[i+9] == 0x00 && bytes[i+10] == 0xC7 && bytes[i+11] == 0x05 &&
            bytes[i-5] == 0xE8)
        {
            patchLocation = i-5;
            DBG(L"Found _cpuid_set_info _panic address at 0x%08x\n",patchLocation);
            break;
        }
    }
	 
	if (!patchLocation)
	{
	     DBG(L"Can't find _cpuid_set_info _panic address, patch kernel abort.\n",i);
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
            DBG(L"Found _tsc_init _panic address at 0x%08x\n",patchLocation1);
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
	 
	UINT32 jumpaddr = patchLocation;
	 
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
        DBG(L"Can't Found jumpaddr address.\n");
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
     
    DBG(L"Start find SSE3 address\n");
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
            DBG(L"Found SSE3 data address at 0x%08x\n",patchLocation1);
        }

         // khasSSE2+..... title
        if (bytes[i] == 0xE3 && bytes[i+1] == 0x07 && bytes[i+2] == 0x00 &&
            bytes[i+3] == 0x00 && bytes[i+4] == 0x80 && bytes[i+5] == 0x07 &&
            bytes[i+6] == 0xFF && bytes[i+7] == 0xFF && bytes[i+8] == 0x24 &&
            bytes[i+9] == 0x01)
        {
           patchLocation2 = i;
           DBG(L"Found SSE3 Title address at 0x%08x\n",patchLocation2);
           break;
        }
        i++;
    }            
	
    if (!patchLocation1 || !patchLocation2) 
    {
        DBG(L"Can't found SSE3 data addres or Title address at 0x%08x 0x%08x\n", patchLocation1, patchLocation2);
        return;
    }
     
    DBG(L"Found SSE3 last data addres Start\n");
    i = patchLocation1 + 1500;
    //for (i=(patchLocation1+1500); i<(patchLocation1+3000); i++)
    while(TRUE)
    {
        if (bytes[i] == 0x90 && bytes[i+1] == 0x90 && bytes[i+2] == 0x55 ) 
        {
            patchlast = (i+1) - patchLocation1;
            DBG(L"Found SSE3 last data addres at 0x%08x\n", patchlast);
            break;
        }
        i++;
    }
     
    if (!patchlast)
    {
        DBG(L"Can't found SSE3 data last addres at 0x%08x\n", patchlast);
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

    DBG(L"Start find SSE3 address\n");
    
    for (i=256; i<(Length-256); i++) 
    {
        if (bytes[i] == 0x66 && bytes[i+1] == 0x0F && bytes[i+2] == 0x6F && 
            bytes[i+3] == 0x44 && bytes[i+4] == 0x0E && bytes[i+5] == 0xF1 &&
            bytes[i-1680-32] == 0x55)
        {
            patchLocation1 = i-1680-32;
            DBG(L"Found SSE3 data address at 0x%08x\n",patchLocation1);
        }

        // khasSSE2+..... title
        if (bytes[i] == 0xF3 && bytes[i+1] == 0x07 && bytes[i+2] == 0x00 &&
            bytes[i+3] == 0x00 && bytes[i+4] == 0x80 && bytes[i+5] == 0x07 &&
            bytes[i+6] == 0xFF && bytes[i+7] == 0xFF && bytes[i+8] == 0x24 &&
            bytes[i+9] == 0x01)
        {
           patchLocation2 = i;
           DBG(L"Found SSE3 Title address at 0x%08x\n",patchLocation2);
           break;
        }
    }            
	
    if (!patchLocation1 || !patchLocation2) 
    {
        DBG(L"Can't found SSE3 data addres or Title address at 0x%08x 0x%08x\n", patchLocation1, patchLocation2);
        return;
    }
     
    DBG(L"Found SSE3 last data addres Start\n");
          
    for (i=(patchLocation1+1500);i<Length;i++)
    {
       if (bytes[i] == 0x90 && bytes[i+1] == 0x90 && bytes[i+2] == 0x55) 
       {
           patchlast = (i+1) - patchLocation1;
            DBG(L"Found SSE3 last data addres at 0x%08x\n", patchlast);
           break;
       }
    }

    if (!patchlast)
    {
        DBG(L"Can't found SSE3 data last addres at 0x%08x\n", patchlast);
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