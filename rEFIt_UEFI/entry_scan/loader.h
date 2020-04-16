/*
 * common.h
 *
 *  Created on: 10 Apr 2020
 *      Author: jief
 */

#ifndef LOADER_H_
#define LOADER_H_

//#define DUMP_KERNEL_KEXT_PATCHES 1


UINT8       GetOSTypeFromPath (IN CONST CHAR16 *Path);

#ifdef DUMP_KERNEL_KEXT_PATCHES
// Utils functions
VOID DumpKernelAndKextPatches(KERNEL_AND_KEXT_PATCHES *Patches);
#endif


#endif
