/*
 * common.h
 *
 *  Created on: 5 Apr 2020
 *      Author: jief
 */

#ifndef ENTRY_SCAN_COMMON_H_
#define ENTRY_SCAN_COMMON_H_

#include "../cpp_foundation/XString.h"

//XString AddLoadOption(IN const XString& LoadOptions, IN const XString& LoadOption);
//XString RemoveLoadOption(IN const XString& LoadOptions, IN const XString& LoadOption);

INTN
StrniCmp (
  IN CONST CHAR16 *Str1,
  IN CONST CHAR16 *Str2,
  IN UINTN  Count
  );

CONST CHAR16
*StriStr(
  IN CONST CHAR16 *Str,
  IN CONST CHAR16 *SearchFor
  );

VOID
StrToLower (
  IN CHAR16 *Str
  );

VOID
AlertMessage (
  IN CONST CHAR16 *Title,
  IN CONST CHAR16 *Message
  );

BOOLEAN
YesNoMessage (
  IN CONST CHAR16 *Title,
  IN CONST CHAR16 *Message);


UINT64 TimeDiff(UINT64 t0, UINT64 t1); //double in Platform.h



#endif /* ENTRY_SCAN_COMMON_H_ */
