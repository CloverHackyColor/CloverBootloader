/*
 * common.h
 *
 *  Created on: 10 Apr 2020
 *      Author: jief
 */

#ifndef LOADER_H_
#define LOADER_H_

//#define DUMP_KERNEL_KEXT_PATCHES 1

// Kernel scan states
#define KERNEL_SCAN_ALL        (0)
#define KERNEL_SCAN_NEWEST     (1)
#define KERNEL_SCAN_OLDEST     (2)
#define KERNEL_SCAN_FIRST      (3)
#define KERNEL_SCAN_LAST       (4)
#define KERNEL_SCAN_MOSTRECENT (5)
#define KERNEL_SCAN_EARLIEST   (6)
#define KERNEL_SCAN_NONE       (100)


UINT8       GetOSTypeFromPath (IN CONST CHAR16 *Path);

#ifdef DUMP_KERNEL_KEXT_PATCHES
// Utils functions
VOID DumpKernelAndKextPatches(KERNEL_AND_KEXT_PATCHES *Patches);
#endif


#endif
