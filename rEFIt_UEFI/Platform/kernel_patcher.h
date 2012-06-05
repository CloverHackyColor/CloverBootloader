/*
 * Copyright (c) 2009-2010 Frank peng. All rights reserved.
 *
 */
#include <Platform.h>
#include <loader.h>
#include <nlist.h>

#ifndef __LIBSAIO_KERNEL_PATCHER_H
#define __LIBSAIO_KERNEL_PATCHER_H

#define CPUFAMILY_INTEL_6_13		0xaa33392b
#define CPUFAMILY_INTEL_YONAH		0x73d67300
#define CPUFAMILY_INTEL_MEROM		0x426f69ef
#define CPUFAMILY_INTEL_PENRYN		0x78ea4fbc
#define CPUFAMILY_INTEL_NEHALEM		0x6b5a4cd2
#define CPUFAMILY_INTEL_WESTMERE	0x573b5eec

#define CPUIDFAMILY_DEFAULT 6

#define CPUID_MODEL_6_13	 	    13
#define CPUID_MODEL_YONAH			14
#define CPUID_MODEL_MEROM			15
#define CPUID_MODEL_PENRYN			35

#define MACH_GET_MAGIC(hdr)        (((struct mach_header_64*)(hdr))->magic)
#define MACH_GET_NCMDS(hdr)        (((struct mach_header_64*)(hdr))->ncmds)
#define SC_GET_CMD(hdr)            (((struct segment_command_64*)(hdr))->cmd)

extern UINT32 DisplayVendor[2];
//extern UINT32     KextAddr;
extern UINT32     KextLength;
//VOID findCPUfamily();

CHAR8* dtRoot;

//UINT64 kernelsize;

VOID KernelPatcher_64(VOID* kernelData);
VOID KernelPatcher_32(VOID* kernelData);

VOID Patcher_SSE3_5(VOID* kernelData);
VOID Patcher_SSE3_6(VOID* kernelData);
VOID Patcher_SSE3_7(VOID* kernelData);

//VOID register_kernel_symbol(CONST CHAR8* name);
//UINT64 symbol_handler(CHAR8* symbolName, UINT64 addr);
//INTN locate_symbols(VOID* kernelData);

VOID KextPatcher_Start();
VOID Get_PreLink(VOID* binary);
VOID KextPatcher_driver_ATI();
VOID KextPatcher_ATI(UINT32 driverAddr, UINT32 driverSize);
VOID InjectKernelCache(VOID* binary);


#endif /* !__BOOT2_KERNEL_PATCHER_H */
