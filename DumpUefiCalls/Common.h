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
//  Clover/DumpUefiCalls/DumpUefiCalls.inf {
//    <PcdsFixedAtBuild>
//      gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x07
//      gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0xFFFFFFFF
//    <LibraryClasses>
//      DebugLib|MdePkg/Library/BaseDebugLibSerialPort/BaseDebugLibSerialPort.inf
//      SerialPortLib|MdeModulePkg/Library/BaseSerialPortLib16550/BaseSerialPortLib16550.inf
//  }
// in package DSC file (Clover.dsc)
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
#define PRINT(...) LogPrint(__VA_ARGS__);

//
// WORK_DURING_RUNTIME:
// 2 - will continue to print even after ExitBootServices() - using "new style" callback event
// 1 - will continue to print even after ExitBootServices() - using "old style" callback event
//        usefull only when printing to serial in VBox for example
// 0 - will stop printing when ExitBootServices() are called
//
#define WORK_DURING_RUNTIME		0

//
// PRINT_DUMPS:
// 1 - will print dumps (ST, RT vars) when ExitBootServices() is called
// 0 - will skip printing dumps
//
#define PRINT_DUMPS			1

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

// 
// BOOT_LOADERS
// specify here as shown below (in a CHAR16* array) all boot loaders for which overrides will start
// array must be terminated by NULL
//  
#define BOOT_LOADERS { L"boot.efi", L"bootmgfw.efi", L"grub.efi", L"grubx64.efi", NULL }


#include "Lib.h"
#include "Log.h"
#include "FileLib.h"
#include "BootServices.h"
#include "RuntimeServices.h"

#endif // __DMP_COMMON_H__

