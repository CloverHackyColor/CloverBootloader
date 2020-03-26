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


#endif
