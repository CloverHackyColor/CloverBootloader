/*
Headers collection for procedures
*/

#ifndef __BOOTLOG__H__
#define __BOOTLOG__H__

#define MsgLog ::printf

#ifdef __cplusplus
extern "C" {
#endif


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



#ifdef __cplusplus
} // extern "C"
#endif


#endif
