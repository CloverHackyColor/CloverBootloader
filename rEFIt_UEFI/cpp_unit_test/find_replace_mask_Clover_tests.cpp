#include <Platform.h> // Only use angled for Platform, else, xcode project won't compile
#include "../Platform/MemoryOperation.h"

static int breakpoint(int i)
{
  return i;
}

int find_replace_mask_Clover_tests()
{
//  int ret;
//  INT32 int32;
  UINTN uintn;

  uintn = FindMemMask((UINT8*)"\x01\x11\x02", 3, (UINT8*)"\x11", 1, NULL, 0);
  if ( uintn != 1 ) breakpoint(1);

  uintn = FindMemMask((UINT8*)"\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a", 10, (UINT8*)"\x13\x14", 2, NULL, 0);
  if ( uintn != 2 ) breakpoint(1);

  uintn = FindMemMask((UINT8*)"\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a", 10, (UINT8*)"\x14\x00\x16", 3, (UINT8*)"\xFF\x00\xFF", 3);
  if ( uintn != 3 ) breakpoint(1);

  uintn = FindMemMask((UINT8*)"\x00\xCC\x00", 3, (UINT8*)"\xC0", 1, (UINT8*)"\xF0", 1);
  if ( uintn != 1 ) breakpoint(1);

  // Simple patch of 3 bytes
  {
    UINT8 buf[] = { 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x20 };
    UINT8 expectedBuf[] = { 0x11, 0x12, 0x23, 0x24, 0x25, 0x16, 0x17, 0x18, 0x19, 0x20 };
    uintn = SearchAndReplaceMask(buf, 10,
                                  (UINT8*)"\x13\x14\x15", NULL, 3,
                                  (UINT8*)"\x23\x24\x25", NULL,
                                  0);
    if ( uintn != 1 ) breakpoint(1);
    if ( memcmp(buf, expectedBuf, 10) != 0 ) breakpoint(1);
  }
  // Patch 3 bytes with find mask
  {
    UINT8 buf[] = { 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x20 };
    UINT8 expectedBuf[] = { 0x11, 0x12, 0x23, 0x24, 0x25, 0x16, 0x17, 0x18, 0x19, 0x20 };
    uintn = SearchAndReplaceMask(buf, 10,
                                 (UINT8*)"\x13\x00\x15", (UINT8*)"\xFF\x00\xFF", 3,
                                 (UINT8*)"\x23\x24\x25", NULL,
                                 0);
    if ( uintn != 1 ) breakpoint(1);
    if ( memcmp(buf, expectedBuf, 10) != 0 ) breakpoint(1);
  }
  // Patch 3 bytes with find mask and replacemask. Mask replace bits same place as Mask find
  // Find x13x00x15/0xFFx00xFF Replace x23x24x25/xFFx00xFF
  {
    UINT8 buf[] = { 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x20 };
    UINT8 expectedBuf[] = { 0x11, 0x12, 0x23, 0x14, 0x25, 0x16, 0x17, 0x18, 0x19, 0x20 };
    uintn = SearchAndReplaceMask(buf, 10,
                                 (UINT8*)"\x13\x00\x15", (UINT8*)"\xFF\x00\xFF", 3,
                                 (UINT8*)"\x23\x24\x25", (UINT8*)"\xFF\x00\xFF",
                                 0);
    if ( uintn != 1 ) breakpoint(1);
    if ( memcmp(buf, expectedBuf, 10) != 0 ) breakpoint(1);
  }
  // Patch 3 bytes with find mask and replacemask. Mask replace bits NOT same place as Mask find
  // Find x13x00x15/0xFFx00xFF Replace x23x24x25/xFFxFFx00
  {
    UINT8 buf[] = { 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x20 };
    UINT8 expectedBuf[] = { 0x11, 0x12, 0x23, 0x24, 0x15, 0x16, 0x17, 0x18, 0x19, 0x20 };
    uintn = SearchAndReplaceMask(buf, 10,
                                 (UINT8*)"\x13\x00\x15", (UINT8*)"\xFF\x00\xFF", 3,
                                 (UINT8*)"\x23\x24\x25", (UINT8*)"\xFF\xFF\x00",
                                 0);
    if ( uintn != 1 ) breakpoint(1);
    if ( memcmp(buf, expectedBuf, 10) != 0 ) breakpoint(1);
  }
  // Patch half a byte
  {
    UINT8 buf[] = { 0x11, 0xCC, 0x13 };
    UINT8 expectedBuf[] = { 0x11, 0xC2, 0x13 };
    uintn = SearchAndReplaceMask(buf, 3,
                                 (UINT8*)"\xC0", (UINT8*)"\xF0", 1,
                                 (UINT8*)"\x22", (UINT8*)"\x0F",
                                 0);
    if ( uintn != 1 ) breakpoint(1);
    if ( memcmp(buf, expectedBuf, 3) != 0 ) breakpoint(1);
  }

  return 0;
}
