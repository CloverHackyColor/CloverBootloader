/*
  Some helper string functions
  JrCs 2014
*/

#include "Platform.h"

XString8 NonDetected = "10.10.10"_XS8;  //longer string

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

UINT64 AsciiStrVersionToUint64(const XString8& Version_, UINT8 MaxDigitByPart, UINT8 MaxParts)
{
  UINT64 result = 0;
  UINT16 part_value = 0;
  UINT16 part_mult  = 1;
  UINT16 max_part_value;
  
  const XString8* VersionPtr = &Version_;
  if (VersionPtr->isEmpty()) {
    VersionPtr = &NonDetected; //pointer to non-NULL string
  }
  const XString8& Version = *VersionPtr;

  while (MaxDigitByPart--) {
    part_mult = part_mult * 10;
  }
  max_part_value = part_mult - 1;
  size_t idx = 0;
  while (idx < Version.length() && MaxParts > 0) {  //Slice - NULL pointer dereferencing
    if (Version[idx] >= '0' && Version[idx] <= '9') {
      part_value = part_value * 10 + (Version[idx] - '0');
      if (part_value > max_part_value)
        part_value = max_part_value;
    }
    else if (Version[idx] == '.') {
      result = MultU64x64(result, part_mult) + part_value;
      part_value = 0;
      MaxParts--;
    }
    idx++;
  }

  while (MaxParts--) {
    result = MultU64x64(result, part_mult) + part_value;
    part_value = 0; // part_value is only used at first pass
  }

  return result;
}
