/** @file

  Common declarations and definitions.

  By dmazar, 26/09/2012

**/

#ifndef __DMP_COMMON_H__
#define __DMP_COMMON_H__


//
// LOG_TO_SCREEN:
// 1 - will enable log output to screen
// 0 - will disable log output to screen
//
#define LOG_TO_SCREEN			0

//
// LOG_TO_SERIAL:
// 2 - will enable log output to serial, and allow sending console output to serial as well (when CAPTURE_CONSOLE_OUTPUT below is enabled)
// 1 - will enable serial output, but prevent sending console output to serial
// 0 - will disable log output to serial
//
// Options 1 and 2 differ only when CAPTURE_CONSOLE_OUTPUT is enabled:
//  Some UEFI implementations always send console output to serial (even when we send nothing), while others don't do it.
//  So, we may want to print console output to file, but prevent it from being printed to serial (on firmwares where it is already being printed there).
//  Or, we may want to print console output to both file and serial (on firmwares where it is not being printed there).
//
// LOG_TO_SERIAL=1/2 requires
//  Clover/DumpUefiCalls/DumpUefiCalls.inf {
//    <LibraryClasses>
//      DebugLib|MdePkg/Library/BaseDebugLibSerialPort/BaseDebugLibSerialPort.inf
//      SerialPortLib|MdeModulePkg/Library/BaseSerialPortLib16550/BaseSerialPortLib16550.inf
//    <PcdsFixedAtBuild>
//      gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x07
//      gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0xFFFFFFFF
//  }
// in package DSC file (Clover.dsc)
// Alternatively, they can be defined directly in global [LibraryClasses] and [PcdsFixedAtBuild] of DSC file (as in DumpUefiCalls.dsc)
//
#ifdef DEBUG_ON_SERIAL_PORT
#define LOG_TO_SERIAL			2
#else
#define LOG_TO_SERIAL			0
#endif

//
// LOG_TO_FILE:
// 4 - will enable file output with append while logging, reopening file with every write (this is how it was implemented before, not sure if needed)
// 3 - will enable file output with append while logging, keeping file open, and flush after every write (good for debugging if it hangs)
// 2 - will enable file output, appending to existing file (save is done only at the end)
// 1 - will enable file output, overwriting old file (save is done only at the end)
// 0 - will disable file output
// Note: append while logging (options 3,4) works fine on CloverEFI, but varies between different UEFI firmwares, use only if needed for debugging.
//
#ifdef DEBUG_ON_SERIAL_PORT
#define LOG_TO_FILE			0
#else
#define LOG_TO_FILE			4
#endif

//
// LOG_TO_FILE_PATH
//
// Note: some firmwares don't support writing to long filenames, so it's better to keep names as 8.3.
#ifdef CLOVER_BUILD
#define LOG_TO_FILE_PATH L"\\EFI\\CLOVER\\misc\\EfiCalls.log"
#else
#define LOG_TO_FILE_PATH L"\\EFI\\EfiCalls.log"
#endif

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
#define WORK_DURING_RUNTIME		2

//
// PRINT_DUMPS:
// 2 - will print dumps for ST, RT, and DataHub vars when ExitBootServices() is called
// 1 - will print dumps for ST, RT vars when ExitBootServices() is called
// 0 - will skip printing dumps
//
#define PRINT_DUMPS			0

//
// PRINT_SHELL_VARS:
// 1 - will print shell vars in PrintRTVariables(), takes place only if above is enabled
// 0 - will skip shell vars
//
#define PRINT_SHELL_VARS		0

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
// CAPTURE_FILESYSTEM_ACCESS		1
// 1 - will capture and display all file system access
// 0 - will skip capturing file system access
//
#define CAPTURE_FILESYSTEM_ACCESS	1

//
// CAPTURE_CONSOLE_OUTPUT
// 2 - will capture conout, and prevent it from being displayed to screen (useful when saving to file where there is a lot of screen output)
// 1 - will capture conout, allowing it to be displayed to screen also (useful when saving to file)
// 0 - will skip capturing conout
//
#define CAPTURE_CONSOLE_OUTPUT		1

//
// CLEANER_LOG
// 1 - will prevent printing CalculateCrc32() and GetVariable("EfiTime",...), as they are called very often on some firmwares
// 0 - will allow printing the above
//
#define CLEANER_LOG			1

// 
// BOOT_LOADERS
// specify here as shown below (in a CHAR16* array) all boot loaders for which overrides will start
// array must be terminated by NULL
//  
#define BOOT_LOADERS { L"boot.efi", L"bootmgfw.efi", L"grub.efi", L"grubx64.efi", L"bootx64.efi", NULL }

#define HANDLE_PROTOCOL 0
#define LOCATE_PROTOCOL 0
#define OPEN_PROTOCOL 0
#define SET_KEY_STROKE 0

#include "Lib.h"
#include "Log.h"
#include "FileLib.h"
#include "BootServices.h"
#include "RuntimeServices.h"
#include "DataHub.h"
#include "Fs.h"
#include "AppleProtocols.h"

extern EFI_GUID mEfiSimplePointerProtocolGuid;

#endif // __DMP_COMMON_H__

