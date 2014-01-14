/** @file

  Common declarations and definitions.

  By dmazar, 26/09/2012             

**/

#ifndef __DMP_COMMON_H__
#define __DMP_COMMON_H__


//
// Enable/disable log outputs: 1 or 0
//
// LOG_TO_SERIAL=1 requires
// [PcdsFixedAtBuild]
//  gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x07
//  gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0xFFFFFFFF
// in package DSC file
//
#define LOG_TO_SCREEN			1
#define LOG_TO_SERIAL			0

//
// LOG_TO_FILE:
// 4 - will enable file output with append while logging, reopening file with every write (this is how it was implemented before, not sure if needed)
// 3 - will enable file output with append while logging, keeping file open, and flush after every write (good for debugging if it hangs)
// 2 - will enable file output, appending to existing file (save is done only at the end)
// 1 - will enable file output, overwriting old file (save is done only at the end)
// 0 - will disable file output
// Note: append while logging (options 3,4) works fine on CloverEFI, but varies between different UEFI firmwares, use only if needed for debugging.
//
#define LOG_TO_FILE			1

//
// LOG_TO_FILE_PATH
//
#define LOG_TO_FILE_PATH L"\\EFI\\UefiCalls.log"

//
// PRINT calls our main logger.
//
// the following ensures that we don't end up calling print from another print, which was observed in some cases and could cause reboot/hang
extern BOOLEAN InPrint;
#define PRINT(...) if (!InPrint) { InPrint=TRUE; LogPrint(__VA_ARGS__); InPrint=FALSE; }

//
// WORK_DURING_RUNTIME:
// 1 - will continue to print even after ExitBootServices()
//        usefull only when printing to serial in VBox for example
// 0 - will stop printing when ExitBootServices() are called
//
#define WORK_DURING_RUNTIME		0

//
// PRINT_RT_VARS:
// 1 - will print RT vars before exiting bootservices
// 0 - will skip RT vars printing
//
#define PRINT_RT_VARS			1

//
// PRINT_SHELL_VARS:
// 1 - will print shell vars in PrintRTVariables(), takes place only if above is enabled
// 0 - will skip shell vars
//
#define PRINT_SHELL_VARS		1

//
// PRINT_MEMORY_MAP:
// 1 - will print memmap when GetMemoryMap() is called
// 0 - will skip printing memmap
//
#define PRINT_MEMORY_MAP		0

//
// PRINT_ALLOCATE_POOL:
// 1 - will print AllocatePool() calls, note that there are a lot of them
// 0 - will skip printing AllocatePool() calls.
//
#define PRINT_ALLOCATE_POOL		0

//
// CAPTURE_CONSOLE_OUTPUT
// 2 - will capture conout, and prevent it from being displayed to screen (useful when saving to file)
// 1 - will capture conout, allowing it to be displayed to screen also (useful when saving to file)
// 0 - will skip capturing conout
//
#define CAPTURE_CONSOLE_OUTPUT		1


#include "Lib.h"
#include "Log.h"
#include "FileLib.h"
#include "BootServices.h"
#include "RuntimeServices.h"

#endif // __DMP_COMMON_H__

