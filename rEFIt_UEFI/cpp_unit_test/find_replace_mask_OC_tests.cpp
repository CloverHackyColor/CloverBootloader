#include <Platform.h> // Only use angled for Platform, else, xcode project won't compile

//#include "../../OpenCorePkg/Include/Acidanthera/Library/OcMiscLib.h"


INT32
FindPattern (
  IN CONST UINT8   *Pattern,
  IN CONST UINT8   *PatternMask OPTIONAL,
  IN CONST UINT32  PatternSize,
  IN CONST UINT8   *Data,
  IN UINT32        DataSize,
  IN INT32         DataOff
  );

UINT32
ApplyPatch (
  IN CONST UINT8   *Pattern,
  IN CONST UINT8   *PatternMask OPTIONAL,
  IN CONST UINT32  PatternSize,
  IN CONST UINT8   *Replace,
  IN CONST UINT8   *ReplaceMask OPTIONAL,
  IN UINT8         *Data,
  IN UINT32        DataSize,
  IN UINT32        Count,
  IN UINT32        Skip
  );

static int breakpoint(int i)
{
  return i;
}

int find_replace_mask_OC_tests()
{
//  int ret;
  INT32 int32;
  UINT32 uint32;

  int32 = FindPattern((UINT8*)"\x11", NULL, 1, (UINT8*)"\x01\x11\x02", 3, 0);
  if ( int32 != 1 ) breakpoint(1);

  int32 = FindPattern((UINT8*)"\x13\x14", NULL, 2, (UINT8*)"\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a", 10, 0);
  if ( int32 != 2 ) breakpoint(1);

  int32 = FindPattern((UINT8*)"\x14\x00\x16", (UINT8*)"\xFF\x00\xFF", 3, (UINT8*)"\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a", 10, 0);
  if ( int32 != 3 ) breakpoint(1);

  int32 = FindPattern((UINT8*)"\xC0", (UINT8*)"\xF0", 1, (UINT8*)"\x00\xCC\x00", 3, 0);
  if ( int32 != 1 ) breakpoint(1);

  // Simple patch of 3 bytes
  {
    UINT8 buf[] = { 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x20 };
    UINT8 expectedBuf[] = { 0x11, 0x12, 0x23, 0x24, 0x25, 0x16, 0x17, 0x18, 0x19, 0x20 };
    uint32 = ApplyPatch( (UINT8*)"\x13\x14\x15", NULL, 3,   (UINT8*)"\x23\x24\x25", NULL,   buf, 10, 0, 0);
    if ( uint32 != 1 ) breakpoint(1);
    if ( memcmp(buf, expectedBuf, 10) != 0 ) breakpoint(1);
  }
  // Patch 3 bytes with find mask
  {
    UINT8 buf[] = { 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x20 };
    UINT8 expectedBuf[] = { 0x11, 0x12, 0x23, 0x24, 0x25, 0x16, 0x17, 0x18, 0x19, 0x20 };
    uint32 = ApplyPatch( (UINT8*)"\x13\x00\x15", (UINT8*)"\xFF\x00\xFF", 3,   (UINT8*)"\x23\x24\x25", NULL,   buf, 10, 0, 0);
    if ( uint32 != 1 ) breakpoint(1);
    if ( memcmp(buf, expectedBuf, 10) != 0 ) breakpoint(1);
  }
  // Patch 3 bytes with find mask and replacemask
  {
    UINT8 buf[] = { 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x20 };
    UINT8 expectedBuf[] = { 0x11, 0x12, 0x23, 0x14, 0x25, 0x16, 0x17, 0x18, 0x19, 0x20 };
    uint32 = ApplyPatch( (UINT8*)"\x13\x00\x15", (UINT8*)"\xFF\x00\xFF", 3,   (UINT8*)"\x23\x24\x25", (UINT8*)"\xFF\x00\xFF",   buf, 10, 0, 0);
    if ( uint32 != 1 ) breakpoint(1);
    if ( memcmp(buf, expectedBuf, 10) != 0 ) breakpoint(1);
  }
  // Patch 3 bytes with find mask and replacemask
  {
    UINT8 buf[] = { 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x20 };
    UINT8 expectedBuf[] = { 0x11, 0x12, 0x23, 0x24, 0x15, 0x16, 0x17, 0x18, 0x19, 0x20 };
    uint32 = ApplyPatch(
                (UINT8*)"\x13\x00\x15", (UINT8*)"\xFF\x00\xFF", 3,
                (UINT8*)"\x23\x24\x25", (UINT8*)"\xFF\xFF\x00",
                buf, 10, 0, 0);
    if ( uint32 != 1 ) breakpoint(1);
    if ( memcmp(buf, expectedBuf, 10) != 0 ) breakpoint(1);
  }
  // Patch half a byte
  {
    UINT8 buf[] = { 0x11, 0xCC, 0x13 };
    UINT8 expectedBuf[] = { 0x11, 0xC2, 0x13 };
    uint32 = ApplyPatch((UINT8*)"\xC0", (UINT8*)"\xF0", 1,
                       (UINT8*)"\x22", (UINT8*)"\x0F",
                       buf, 3, 0, 0);
    if ( uint32 != 1 ) breakpoint(1);
    if ( memcmp(buf, expectedBuf, 3) != 0 ) breakpoint(1);
  }

  return 0;
}
