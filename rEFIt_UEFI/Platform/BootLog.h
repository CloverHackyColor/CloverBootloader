/*
Headers collection for procedures
*/

#ifndef __BOOTLOG__H__
#define __BOOTLOG__H__

#ifdef __cplusplus
extern "C" {
#endif


#include <Protocol/SimpleFileSystem.h> // for EFI_FILE_HANDLE

#ifdef __cplusplus
} // extern "C"
#endif


//#define SAFE_LOG_SIZE  80
//#define MSG_LOG_SIZE (256 * 1024)
#define PREBOOT_LOG  L"EFI\\CLOVER\\misc\\preboot.log"
#define LEGBOOT_LOG  L"EFI\\CLOVER\\misc\\legacy_boot.log"
#define BOOT_LOG     L"EFI\\CLOVER\\misc\\boot.log"
#define SYSTEM_LOG   L"EFI\\CLOVER\\misc\\system.log"
#define DEBUG_LOG    L"EFI\\CLOVER\\misc\\debug.log"
//#define PREWAKE_LOG  L"EFI\\CLOVER\\misc\\prewake.log"



VOID
InitBooterLog (VOID);

EFI_STATUS
SetupBooterLog (
  BOOLEAN AllowGrownSize
  );

EFI_STATUS
SaveBooterLog (
  IN  EFI_FILE_HANDLE BaseDir  OPTIONAL,
  IN  CONST CHAR16 *FileName
  );

#ifdef _MSC_VER
#define __attribute__(x)
#endif

VOID
EFIAPI
DebugLog (
  IN        INTN  DebugMode,
  IN  CONST CHAR8 *FormatString, ...) __attribute__((format(printf, 2, 3)));


/** Prints series of bytes. */
VOID
PrintBytes (
  IN  VOID *Bytes,
  IN  UINTN Number
  );

#endif
