#ifndef __REFIT_STRING_H__
#define __REFIT_STRING_H__


extern XString8 NonDetected;

/**
  Convert a Null-terminated ASCII string representing version number (separate by dots)
  to a UINT64 value.

  If Version is NULL, then result is 0.

  @param  Version         The pointer to a Null-terminated ASCII version string. Like 10.9.4
  @param  MaxDigitByPart  Is the maximum number of digits between the dot separators
  @param  MaxParts        Is the maximum number of parts (blocks between dot separators)
                          the version is composed. For a string like 143.5.77.26 the
                          MaxParts is 4

  @return Result

**/
UINT64 AsciiStrVersionToUint64(const XString8& Version, UINT8 MaxDigitByPart, UINT8 MaxParts);


/* Macro to use the AsciiStrVersionToUint64 for OSX Version strings */
#define AsciiOSVersionToUint64(version) AsciiStrVersionToUint64(version, 2, 3)

#endif
