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
  IN      CHAR16   *Str,
     OUT  EFI_GUID *Guid);

CHAR16 * GuidBeToStr(EFI_GUID *Guid);
CHAR16 * GuidLEToStr(EFI_GUID *Guid);



#endif /* PLATFORM_GUID_H_ */
