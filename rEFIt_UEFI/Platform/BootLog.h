/*
Headers collection for procedures
*/

#ifndef __BOOTLOG__H__
#define __BOOTLOG__H__

#ifdef __cplusplus
extern "C" {
#endif


#include <Uefi/UefiBaseType.h>
#include <Protocol/SimpleFileSystem.h> // for EFI_FILE*


//#define SAFE_LOG_SIZE  80
//#define MSG_LOG_SIZE (256 * 1024)
#define PREBOOT_LOG  L"misc\\preboot.log"
#define LEGBOOT_LOG  L"misc\\legacy_boot.log"
//#define BOOT_LOG_new     L"misc\\boot.log"
//#define SYSTEM_LOG_new   L"misc\\system.log"
#define DEBUG_LOG    L"misc\\debug.log"
//#define PREWAKE_LOG  L"misc\\prewake.log"


#ifndef DEBUG_ALL
#define MsgLog(...)  DebugLog(1, __VA_ARGS__)
#else
#define MsgLog(...)  DebugLog(DEBUG_ALL, __VA_ARGS__)
#endif


void
InitBooterLog (void);

void closeDebugLog(void);

EFI_STATUS
SetupBooterLog (
  BOOLEAN AllowGrownSize
  );

EFI_STATUS
SaveBooterLog (
  const EFI_FILE* BaseDir  OPTIONAL,
  const CHAR16 *FileName
  );

#ifdef _MSC_VER
#define __attribute__(x)
#endif

void
EFIAPI
DebugLog (
  IN        INTN  DebugMode,
  IN  CONST CHAR8 *FormatString, ...) __attribute__((format(printf, 2, 3)));


/** Prints series of bytes. */
void
PrintBytes (
  IN  void *Bytes,
  IN  UINTN Number
  );


void DbgHeader(CONST CHAR8 *str);

/*
 * OpenCore
 */
// This use the EDK format
void EFIAPI DebugLogForOC(IN INTN DebugLevel, IN CONST CHAR8 *FormatString, ...);



#ifdef __cplusplus
} // extern "C"
#endif


#endif
