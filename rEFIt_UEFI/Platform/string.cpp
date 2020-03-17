/*
  Some helper string functions
  JrCs 2014
*/

#include "Platform.h"

CHAR8 NonDetected[] = "10.10.10";  //longer string

/**
  Convert a Null-terminated ASCII string representing version number (separate by dots)
  to a UINT64 value.

  If Version is NULL, then result is 0. (Slice - no)

  @param  Version         The pointer to a Null-terminated ASCII version string. Like 10.9.4
  @param  MaxDigitByPart  Is the maximum number of digits between the dot separators
  @param  MaxParts        Is the maximum number of parts (blocks between dot separators)
                          the version is composed. For a string like 143.5.77.26 the
                          MaxParts is 4

  @return Result

**/

UINT64 AsciiStrVersionToUint64(const CHAR8 *Version, UINT8 MaxDigitByPart, UINT8 MaxParts) {
  UINT64 result = 0;
  UINT16 part_value = 0;
  UINT16 part_mult  = 1;
  UINT16 max_part_value;
  
  if (!Version) {
    Version = NonDetected; //pointer to non-NULL string
  }

  while (MaxDigitByPart--) {
    part_mult = part_mult * 10;
  }
  max_part_value = part_mult - 1;

  while (*Version && MaxParts > 0) {  //Slice - NULL pointer dereferencing
    if (*Version >= '0' && *Version <= '9') {
      part_value = part_value * 10 + (*Version - '0');
      if (part_value > max_part_value)
        part_value = max_part_value;
    }
    else if (*Version == '.') {
      result = MultU64x64(result, part_mult) + part_value;
      part_value = 0;
      MaxParts--;
    }
    Version++;
  }

  while (MaxParts--) {
    result = MultU64x64(result, part_mult) + part_value;
    part_value = 0; // part_value is only used at first pass
  }

  return result;
}
