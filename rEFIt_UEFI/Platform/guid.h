/*
 * guid.h
 *
 *  Created on: 16 Apr 2020
 *      Author: jief
 */

#ifndef PLATFORM_GUID_H_
#define PLATFORM_GUID_H_



BOOLEAN
IsValidGuidAsciiString (
  IN CHAR8 *Str
  );


EFI_STATUS
StrToGuidLE (
  IN      CONST CHAR16   *Str,
     OUT  EFI_GUID *Guid);

XStringW GuidBeToStr(EFI_GUID *Guid);
XString8 GuidLEToXString8(EFI_GUID *Guid);
XStringW GuidLEToXStringW(EFI_GUID *Guid);



#endif /* PLATFORM_GUID_H_ */
