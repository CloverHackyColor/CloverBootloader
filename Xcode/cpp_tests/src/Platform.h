//
//  Platform.h.h
//  cpp_tests
//
//  Created by jief on 23.02.20.
//  Copyright © 2020 JF Knudsen. All rights reserved.
//

#ifndef Platform_h_h
#define Platform_h_h

#ifdef _MSC_VER
#include <Windows.h>
#endif

#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include <wchar.h>
#include "posix.h"

#ifndef __cplusplus
//typedef uint16_t wchar_t;
typedef uint32_t char32_t;
typedef uint16_t char16_t;
#endif


#define IN
#define OUT

#ifndef TRUE
#define TRUE true
#endif
#ifndef FALSE
#define FALSE false
#endif
#define VA_LIST va_list
#define VA_START va_start
#define VA_END va_end
#define VA_ARG va_arg
#define VA_COPY va_copy

#define VOID void
#define EFIAPI
#define CONST const

typedef UINTN RETURN_STATUS;
typedef RETURN_STATUS             EFI_STATUS;
#define MAX_BIT     0x8000000000000000ULL
#define ENCODE_ERROR(StatusCode)     ((RETURN_STATUS)(MAX_BIT | (StatusCode)))

#define RETURN_OUT_OF_RESOURCES      ENCODE_ERROR (9)


#define RETURN_SUCCESS               0
#define RETURN_LOAD_ERROR            ENCODE_ERROR (1)
#define RETURN_INVALID_PARAMETER     ENCODE_ERROR (2)
#define RETURN_UNSUPPORTED           ENCODE_ERROR (3)
#define RETURN_BAD_BUFFER_SIZE       ENCODE_ERROR (4)
#define RETURN_BUFFER_TOO_SMALL      ENCODE_ERROR (5)
#define RETURN_NOT_READY             ENCODE_ERROR (6)
#define RETURN_DEVICE_ERROR          ENCODE_ERROR (7)
#define RETURN_WRITE_PROTECTED       ENCODE_ERROR (8)
#define RETURN_OUT_OF_RESOURCES      ENCODE_ERROR (9)
#define RETURN_VOLUME_CORRUPTED      ENCODE_ERROR (10)
#define RETURN_VOLUME_FULL           ENCODE_ERROR (11)
#define RETURN_NO_MEDIA              ENCODE_ERROR (12)
#define RETURN_MEDIA_CHANGED         ENCODE_ERROR (13)
#define RETURN_NOT_FOUND             ENCODE_ERROR (14)
#define RETURN_ACCESS_DENIED         ENCODE_ERROR (15)
#define RETURN_NO_RESPONSE           ENCODE_ERROR (16)
#define RETURN_NO_MAPPING            ENCODE_ERROR (17)
#define RETURN_TIMEOUT               ENCODE_ERROR (18)
#define RETURN_NOT_STARTED           ENCODE_ERROR (19)
#define RETURN_ALREADY_STARTED       ENCODE_ERROR (20)
#define RETURN_ABORTED               ENCODE_ERROR (21)
#define RETURN_ICMP_ERROR            ENCODE_ERROR (22)
#define RETURN_TFTP_ERROR            ENCODE_ERROR (23)
#define RETURN_PROTOCOL_ERROR        ENCODE_ERROR (24)
#define RETURN_INCOMPATIBLE_VERSION  ENCODE_ERROR (25)
#define RETURN_SECURITY_VIOLATION    ENCODE_ERROR (26)
#define RETURN_CRC_ERROR             ENCODE_ERROR (27)
#define RETURN_END_OF_MEDIA          ENCODE_ERROR (28)
#define RETURN_END_OF_FILE           ENCODE_ERROR (31)

#define RETURN_WARN_UNKNOWN_GLYPH    ENCODE_WARNING (1)
#define RETURN_WARN_DELETE_FAILURE   ENCODE_WARNING (2)
#define RETURN_WARN_WRITE_FAILURE    ENCODE_WARNING (3)
#define RETURN_WARN_BUFFER_TOO_SMALL ENCODE_WARNING (4)

//
// Enumeration of EFI_STATUS.
//
#define EFI_SUCCESS               RETURN_SUCCESS
#define EFI_LOAD_ERROR            RETURN_LOAD_ERROR
#define EFI_INVALID_PARAMETER     RETURN_INVALID_PARAMETER
#define EFI_UNSUPPORTED           RETURN_UNSUPPORTED
#define EFI_BAD_BUFFER_SIZE       RETURN_BAD_BUFFER_SIZE
#define EFI_BUFFER_TOO_SMALL      RETURN_BUFFER_TOO_SMALL
#define EFI_NOT_READY             RETURN_NOT_READY
#define EFI_DEVICE_ERROR          RETURN_DEVICE_ERROR
#define EFI_WRITE_PROTECTED       RETURN_WRITE_PROTECTED
#define EFI_OUT_OF_RESOURCES      RETURN_OUT_OF_RESOURCES
#define EFI_VOLUME_CORRUPTED      RETURN_VOLUME_CORRUPTED
#define EFI_VOLUME_FULL           RETURN_VOLUME_FULL
#define EFI_NO_MEDIA              RETURN_NO_MEDIA
#define EFI_MEDIA_CHANGED         RETURN_MEDIA_CHANGED
#define EFI_NOT_FOUND             RETURN_NOT_FOUND
#define EFI_ACCESS_DENIED         RETURN_ACCESS_DENIED
#define EFI_NO_RESPONSE           RETURN_NO_RESPONSE
#define EFI_NO_MAPPING            RETURN_NO_MAPPING
#define EFI_TIMEOUT               RETURN_TIMEOUT
#define EFI_NOT_STARTED           RETURN_NOT_STARTED
#define EFI_ALREADY_STARTED       RETURN_ALREADY_STARTED
#define EFI_ABORTED               RETURN_ABORTED
#define EFI_ICMP_ERROR            RETURN_ICMP_ERROR
#define EFI_TFTP_ERROR            RETURN_TFTP_ERROR
#define EFI_PROTOCOL_ERROR        RETURN_PROTOCOL_ERROR
#define EFI_INCOMPATIBLE_VERSION  RETURN_INCOMPATIBLE_VERSION
#define EFI_SECURITY_VIOLATION    RETURN_SECURITY_VIOLATION
#define EFI_CRC_ERROR             RETURN_CRC_ERROR
#define EFI_END_OF_MEDIA          RETURN_END_OF_MEDIA
#define EFI_END_OF_FILE           RETURN_END_OF_FILE

#define EFI_WARN_UNKNOWN_GLYPH    RETURN_WARN_UNKNOWN_GLYPH
#define EFI_WARN_DELETE_FAILURE   RETURN_WARN_DELETE_FAILURE
#define EFI_WARN_WRITE_FAILURE    RETURN_WARN_WRITE_FAILURE
#define EFI_WARN_BUFFER_TOO_SMALL RETURN_WARN_BUFFER_TOO_SMALL

#define RETURN_ERROR(StatusCode)     (((INTN)(RETURN_STATUS)(StatusCode)) < 0)
#define EFI_ERROR(A)              RETURN_ERROR(A)


#define OPTIONAL
#define ASSERT(x)

#ifdef _MSC_VER
#define __typeof__(x) decltype(x)
#endif


#include "../../../rEFIt_UEFI/Platform/Posix/abort.h"
#include "../../../rEFIt_UEFI/cpp_foundation/unicode_conversions.h"
#include "../../../rEFIt_UEFI/cpp_foundation/XString.h"
#include "../../../rEFIt_UEFI/cpp_foundation/XObjArray.h"

#include "xcode_utf_fixed.h"


void CpuDeadLoop(void);
void DebugLog(INTN DebugMode, const char *FormatString, ...);
#define MsgLog ::printf

void PauseForKey(const wchar_t* msg);

const char* efiStrError(EFI_STATUS Status);

RETURN_STATUS
EFIAPI
AsciiStrDecimalToUintnS (
  IN  CONST CHAR8              *String,
  OUT       CHAR8              **EndPointer,  OPTIONAL
  OUT       UINTN              *Data
  );

UINTN EFIAPI AsciiStrHexToUintn (IN CONST CHAR8 *String);
inline
UINTN EFIAPI AsciiStrHexToUintn (const XString8& String)
{
  return AsciiStrHexToUintn(String.c_str());
}

UINTN
EFIAPI
AsciiStrDecimalToUintn (
  IN      CONST CHAR8               *String
  );






void* AllocatePool(UINTN  AllocationSize);
void* AllocateZeroPool(UINTN  AllocationSize);
void* ReallocatePool(UINTN  OldSize, UINTN  NewSize, void* OldBuffer);
void FreePool(const void* Buffer);

void ZeroMem(void *Destination, UINTN Length);
void SetMem(void *Destination, UINTN Length, char c);
void CopyMem(void *Destination, const void *Source, UINTN Length);
INTN CompareMem(const void* DestinationBuffer, const void* SourceBuffer, UINTN Length);

CHAR16* EfiStrDuplicate (IN CONST CHAR16 *Src);
CHAR16* StrStr (IN CONST CHAR16 *String, IN CONST CHAR16 *SearchString);


//UINTN StrLen(const char16_t* String);
UINTN StrLen(const wchar_t* String);
//int StrCmp(const wchar_t* FirstString, const wchar_t* SecondString);
//int StrnCmp(const wchar_t* FirstString, const wchar_t* SecondString, UINTN Length);
//UINTN StrLen(const wchar_t* String);
//UINTN AsciiStrLen(const char* String);
//INTN AsciiStrCmp (const char *FirstString,const char *SecondString);



#endif /* Platform_h_h */
