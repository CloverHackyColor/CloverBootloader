/** @file

  Common declarations and definitions.

  By dmazar, 26/09/2012             

**/

#ifndef __DMP_COMMON_H__
#define __DMP_COMMON_H__


#include "Lib.h"
#include "Log.h"
#include "FileLib.h"
#include "BootServices.h"
#include "RuntimeServices.h"

//
// Enable/disable log outputs: 1 or 0
//
// LOG_TO_SERIAL=1 requires
// [PcdsFixedAtBuild]
//  gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x07
//  gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0xFFFFFFFF
// in package DSC file
//
#define LOG_TO_SCREEN		1
#define LOG_TO_SERIAL		0
#define LOG_TO_FILE			1

//
// PRINT calls our main logger.
//
#define PRINT(...) LogPrint(__VA_ARGS__);

//
// WORK_DURING_RUNTIME:
// 1 - will continue to print even after ExitBootServices()
//        usefull only when printing to serial in VBox for example
// 0 - will stop printing when ExitBootServices() are called
//
#define WORK_DURING_RUNTIME		0

//
// PRINT_SHELL_VARS:
// 1 - will print shell vars in PrintRTVariables()
// 0 - will skipp shell vars
//
#define PRINT_SHELL_VARS		0


#endif // __DMP_COMMON_H__

