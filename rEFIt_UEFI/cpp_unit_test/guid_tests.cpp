#include <Platform.h> // Only use angled for Platform, else, xcode project won't compile
#include "../cpp_foundation/XString.h"
#include "../cpp_foundation/unicode_conversions.h"
#include "../Platform/guid.h"


static int breakpoint(int i)
{
  return i;
}

int guid_tests()
{
  #ifdef JIEF_DEBUG
  //  printf("XStringW_tests -> Enter\n");
  #endif
  
  {
    // uncomment to check that it fails at compile time
    // empty string
    //constexpr const EFI_GUID guidTest1 = ""_guid; (void)guidTest1;
    // one non-hex letter in definition
    //constexpr const EFI_GUID guidTest2 = "{12345678-4321-87X5-1122-334455667788}"_guid; (void)guidTest2;
    // one char missing
    //constexpr const EFI_GUID guidTest3 = "{12345678-4321-8715-1122-33445566778}"_guid; (void)guidTest3;
    // one char too many
    //constexpr const EFI_GUID guidTest4 = "{12345678-4321-8715-1122-3344556677881}"_guid; (void)guidTest4;
  }
  {
    // uncomment to check that it's panicking. Not great that it's panicking, but silently fail would be worse. Anyway, only the constexpr version must be used, therefore catching errors at compile time.
    // empty string
    // const EFI_GUID guidTest1 = ""_guid; (void)guidTest1;
    // one non-hex letter in definition
    // const EFI_GUID guidTest2 = "{12345678-4321-87X5-1122-334455667788}"_guid; (void)guidTest2;
    // one char missing
    // const EFI_GUID guidTest3 = "{12345678-4321-8715-1122-33445566778}"_guid; (void)guidTest3;
    // one char too many
    // const EFI_GUID guidTest4 = "{12345678-4321-8715-1122-3344556677881}"_guid; (void)guidTest4;
  }

  {
    if ( !EFI_GUID::IsValidGuidString("00112233-4455-6677-C899-AABBCCDDEEFF"_XS8) ) return breakpoint(1);
    if ( EFI_GUID::IsValidGuidString("00112233-4455-6677-C899-AABBCCDDEEFZ"_XS8) ) return breakpoint(1);
    if ( !EFI_GUID::IsValidGuidString("{00112233-4455-6677-C899-AABBCCDDEEFF}"_XS8) ) return breakpoint(1);
    if ( EFI_GUID::IsValidGuidString("{00112233-4455-6677-C899-AABBCCDDEEFZ}"_XS8) ) return breakpoint(1);
  }
  
  #ifdef PANIC_CAN_RETURN // 2022-04 : PANIC_CAN_RETURN doesn't work yet.
  {
    // uncomment to check that it fails at compile time
    {
      DontStopAtPanic dontStopAtPanic;
      // empty string
      const EFI_GUID guidTest1 = ""_guid; (void)guidTest1;
      if ( !i_have_panicked ) return breakpoint(3);

    }
    // one non-hex letter in definition
    const EFI_GUID guidTest2 = "{12345678-4321-87X5-1122-334455667788}"_guid; (void)guidTest2;
    // one char missing
    const EFI_GUID guidTest3 = "{12345678-4321-8715-1122-33445566778}"_guid; (void)guidTest3;
    // one char too many
    const EFI_GUID guidTest4 = "{12345678-4321-8715-1122-3344556677881}"_guid; (void)guidTest4;
  }
  #endif
  
  {
    EFI_GUID guidTest;
    XString8 s = "00112233-4455-6677-8899-aabbccddeeff"_XS8; // Variant 1. Stored as BE. Means, in memory 00112233 will be 00112233, but will be seen in debugger as 33221100
    guidTest.takeValueFromBE(s);
    if ( guidTest.Data1 != 0x33221100 ) return breakpoint(1); // being on a LE machine, BE value appears to be swapped.
    if ( guidTest.Data2 != 0x5544 ) return breakpoint(2);
    if ( guidTest.Data3 != 0x7766 ) return breakpoint(3);
    if ( guidTest.Data4[1] != 0x99 ) return breakpoint(5);
    if ( guidTest.Data4[2] != 0xaa ) return breakpoint(6);
    if ( guidTest.Data4[3] != 0xbb ) return breakpoint(7);
    if ( guidTest.Data4[4] != 0xcc ) return breakpoint(8);
    if ( guidTest.Data4[5] != 0xdd ) return breakpoint(9);
    if ( guidTest.Data4[6] != 0xee ) return breakpoint(10);
    if ( guidTest.Data4[7] != 0xff ) return breakpoint(11);
    XString8 guidTestAsString = guidTest.toXString8(true);
    if ( guidTestAsString != "00112233-4455-6677-8899-AABBCCDDEEFF"_XS8 ) return breakpoint(10);
  }
  {
    EFI_GUID guidTest;
    XString8 s = "{00112233-4455-6677-8899-aabbccddeeff}"_XS8; // Variant 1. Stored as BE. Means, in memory 00112233 will be 00112233, but will be seen in debugger as 33221100
    guidTest.takeValueFromBE(s);
    if ( guidTest.Data1 != 0x33221100 ) return breakpoint(1); // being on a LE machine, BE value appears to be swapped.
    if ( guidTest.Data2 != 0x5544 ) return breakpoint(2);
    if ( guidTest.Data3 != 0x7766 ) return breakpoint(3);
    if ( guidTest.Data4[1] != 0x99 ) return breakpoint(5);
    if ( guidTest.Data4[2] != 0xaa ) return breakpoint(6);
    if ( guidTest.Data4[3] != 0xbb ) return breakpoint(7);
    if ( guidTest.Data4[4] != 0xcc ) return breakpoint(8);
    if ( guidTest.Data4[5] != 0xdd ) return breakpoint(9);
    if ( guidTest.Data4[6] != 0xee ) return breakpoint(10);
    if ( guidTest.Data4[7] != 0xff ) return breakpoint(11);
    XString8 guidTestAsString = guidTest.toXString8(true);
    if ( guidTestAsString != "00112233-4455-6677-8899-AABBCCDDEEFF"_XS8 ) return breakpoint(10);
  }
  {
    EFI_GUID guidTest;
    XString8 s = "00112233-4455-6677-C899-aabbccddeeff"_XS8; // Variant 2. Value store as LE.
    guidTest.takeValueFrom(s);
    if ( guidTest.Data1 != 0x00112233 ) return breakpoint(21); // being on a LE machine, BE value appears to be swapped.
    if ( guidTest.Data2 != 0x4455 ) return breakpoint(22);
    if ( guidTest.Data3 != 0x6677 ) return breakpoint(23);
    if ( guidTest.Data4[0] != 0xC8 ) return breakpoint(4);
    if ( guidTest.Data4[1] != 0x99 ) return breakpoint(5);
    if ( guidTest.Data4[2] != 0xaa ) return breakpoint(6);
    if ( guidTest.Data4[3] != 0xbb ) return breakpoint(7);
    if ( guidTest.Data4[4] != 0xcc ) return breakpoint(8);
    if ( guidTest.Data4[5] != 0xdd ) return breakpoint(9);
    if ( guidTest.Data4[6] != 0xee ) return breakpoint(10);
    if ( guidTest.Data4[7] != 0xff ) return breakpoint(11);
    XString8 guidTestAsString = guidTest.toXString8();
    if ( guidTestAsString != "00112233-4455-6677-C899-AABBCCDDEEFF"_XS8 ) return breakpoint(10);
  }
  {
    EFI_GUID guidTest;
    XString8 s = "{00112233-4455-6677-C899-aabbccddeeff}"_XS8; // Variant 2. Value store as LE.
    guidTest.takeValueFrom(s);
    if ( guidTest.Data1 != 0x00112233 ) return breakpoint(21); // being on a LE machine, BE value appears to be swapped.
    if ( guidTest.Data2 != 0x4455 ) return breakpoint(22);
    if ( guidTest.Data3 != 0x6677 ) return breakpoint(23);
    if ( guidTest.Data4[0] != 0xC8 ) return breakpoint(4);
    if ( guidTest.Data4[1] != 0x99 ) return breakpoint(5);
    if ( guidTest.Data4[2] != 0xaa ) return breakpoint(6);
    if ( guidTest.Data4[3] != 0xbb ) return breakpoint(7);
    if ( guidTest.Data4[4] != 0xcc ) return breakpoint(8);
    if ( guidTest.Data4[5] != 0xdd ) return breakpoint(9);
    if ( guidTest.Data4[6] != 0xee ) return breakpoint(10);
    if ( guidTest.Data4[7] != 0xff ) return breakpoint(11);
    XString8 guidTestAsString = guidTest.toXString8();
    if ( guidTestAsString != "00112233-4455-6677-C899-AABBCCDDEEFF"_XS8 ) return breakpoint(10);
  }

//  if ( sizeof(EFI_GUID) != sizeof(EFI_GUID) ) return 1;
//  {
//    constexpr EFI_GUID guidTest0 = {0x12345678, 0x4321, 0xC765, {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88}};
//    if ( guidTest0.Data1 != 0x12345678 ) return 1;
//    if ( guidTest0.Data2 != 0x4321 ) return 2;
//    if ( guidTest0.Data3 != 0xC765 ) return 3;
//    if ( guidTest0.Data4[0] != 0x11 ) return 4;
//    if ( guidTest0.Data4[1] != 0x22 ) return 4;
//    if ( guidTest0.Data4[2] != 0x33 ) return 4;
//    if ( guidTest0.Data4[3] != 0x44 ) return 4;
//    if ( guidTest0.Data4[4] != 0x55 ) return 4;
//    if ( guidTest0.Data4[5] != 0x66 ) return 4;
//    if ( guidTest0.Data4[6] != 0x77 ) return 4;
//    if ( guidTest0.Data4[7] != 0x88 ) return 4;
//
//    constexpr EFI_GUID guidTest1 = {0x12345678, 0x4321, 0xC765, {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88}};
//    if ( guidTest1.Data1 != 0x12345678 ) return 1;
//    if ( guidTest1.Data2 != 0x4321 ) return 2;
//    if ( guidTest1.Data3 != 0xC765 ) return 3;
//    if ( guidTest1.Data4[0] != 0x11 ) return 4;
//    if ( guidTest1.Data4[1] != 0x22 ) return 4;
//    if ( guidTest1.Data4[2] != 0x33 ) return 4;
//    if ( guidTest1.Data4[3] != 0x44 ) return 4;
//    if ( guidTest1.Data4[4] != 0x55 ) return 4;
//    if ( guidTest1.Data4[5] != 0x66 ) return 4;
//    if ( guidTest1.Data4[6] != 0x77 ) return 4;
//    if ( guidTest1.Data4[7] != 0x88 ) return 4;
//
//    if ( memcmp(&guidTest1, &guidTest0, sizeof(EFI_GUID)) != 0 ) return 1;
//  }
  
//  constexpr EFI_GUID efiguidTestRef1 = {0x12345678, 0x4321, 0x8765, {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88}};
  constexpr EFI_GUID guidTestRef1 = {0x12345678, 0x4321, 0x8765, {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88}};
//  if ( guidTestRef1 != efiguidTestRef1 ) return breakpoint(10);

  {
    EFI_GUID guidTest2 = {0x12345678, 0x4321, 0x8765, {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88}};
    if ( guidTest2 != guidTestRef1 ) return breakpoint(10);
  }
  {
    constexpr EFI_GUID guidTest3 = {0x12345678, 0x4321, 0x8765, {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x89}};
    if ( guidTest3 == guidTestRef1 ) return breakpoint(10);
  }
  {
    EFI_GUID guidTest4(guidTestRef1); // copy ctor
    if ( guidTest4 != guidTestRef1 ) return breakpoint(10);
  }
  {
    EFI_GUID guidTest4;
    guidTest4 = guidTestRef1; // assignment ctor
    if ( guidTest4 != guidTestRef1 ) return breakpoint(10);
  }
  {
    constexpr EFI_GUID guidTest4(guidTestRef1); // copy ctor
    if ( guidTest4 != guidTestRef1 ) return breakpoint(10);
  }

  {
    EFI_GUID guidTest5; // assignment
    guidTest5 = guidTestRef1;
    if ( guidTest5 != guidTestRef1 ) return breakpoint(10);
  }
  {
    EFI_GUID guidTestNull;
    if ( guidTestNull.Data1 != 0 ) return breakpoint(1);
    if ( guidTestNull.Data2 != 0 ) return breakpoint(2);
    if ( guidTestNull.Data3 != 0 ) return breakpoint(3);
    if ( guidTestNull.Data4[0] != 0 ) return breakpoint(4);
    if ( guidTestNull.Data4[1] != 0 ) return breakpoint(4);
    if ( guidTestNull.Data4[2] != 0 ) return breakpoint(4);
    if ( guidTestNull.Data4[3] != 0 ) return breakpoint(4);
    if ( guidTestNull.Data4[4] != 0 ) return breakpoint(4);
    if ( guidTestNull.Data4[5] != 0 ) return breakpoint(4);
    if ( guidTestNull.Data4[6] != 0 ) return breakpoint(4);
    if ( guidTestNull.Data4[7] != 0 ) return breakpoint(4);
  }
  {
    constexpr const EFI_GUID guidTest6 = "{12345678-4321-8765-1122-334455667788}"_guid;
    if ( guidTest6 != guidTestRef1 ) return breakpoint(10);
  }
  {
    constexpr const EFI_GUID guidTest7 = "12345678-4321-8765-1122-334455667788"_guid;
    if ( guidTest7 != guidTestRef1 ) return breakpoint(10);
  }
  {
    EFI_GUID guidTest8;
    XString8 s = "12345678-4321-8765-1122-334455667788"_XS8;
    guidTest8.takeValueFrom(s);
    if ( guidTest8 != guidTestRef1 ) return breakpoint(10);
  }

//  {
//    constexpr const EFI_GUID guidTest = "12345678-4321-8765-1122-334455667788"_guid;
//    XString8 s = GuidLEToXString8(guidTest);
//    if ( s != "12345678-4321-8765-1122-334455667788"_XS8 ) return breakpoint(10);
//    if ( guidTest.toXString8() != "12345678-4321-8765-1122-334455667788"_XS8 ) return breakpoint(10);
//    if ( guidTest.toXStringW() != L"12345678-4321-8765-1122-334455667788"_XSW ) return breakpoint(10);
//  }
//
//  {
//    constexpr const EFI_GUID guidTest = "12345678-4321-8765-1122-334455667788"_guid;
//    XString8 s = GuidBeToXString8(guidTest);
//    if ( s != "78563412-2143-6587-1122-334455667788"_XS8 ) return breakpoint(10);
//    if ( guidTest.toXString8(true) != "78563412-2143-6587-1122-334455667788"_XS8 ) return breakpoint(10);
//    if ( guidTest.toXStringW(true) != L"78563412-2143-6587-1122-334455667788"_XSW ) return breakpoint(10);
//  }
  
//  {
//    EFI_GUID guidTest1;
//    EFI_STATUS Status = StrToGuidBE("12345678-4321-8765-1122-334455667788", &guidTest1);
//    if ( Status != 0 ) return breakpoint(0);
//    EFI_GUID guidTest2;
//    guidTest2.takeValueFromBE("12345678-4321-8765-1122-334455667788"_XS8);
//    if ( guidTest1 != guidTest2 ) return breakpoint(10);
//  }

//  constexpr const EFI_GUID guidTest102 = "12345678 4321-8765-1122-334455667788"_guid; // no constexpr, so this will compile, but panic.
//  constexpr const EFI_GUID guidTest103 = "12345678-4321 8765-1122-334455667788"_guid; // no constexpr, so this will compile, but panic.
//  constexpr const EFI_GUID guidTest104 = "12345678-4321-8765 1122-334455667788"_guid; // no constexpr, so this will compile, but panic.
//  constexpr const EFI_GUID guidTest105 = "12345678-4321-8765-1122 334455667788"_guid; // no constexpr, so this will compile, but panic.
//  constexpr const EFI_GUID guidTest106 = "12345678-4321-8765-1122-334455667788-"_guid; // no constexpr, so this will compile, but panic.
//  const EFI_GUID guidTest100 = "xx12345678-4321-8765-1122-334455667788"_guid; // no constexpr, so this will compile, but panic.
//  constexpr const EFI_GUID guidTest101 = "xx12345678-4321-8765-1122-334455667788"_guid; // This will NOT compile. Clang says "Constexpr variable must be initialized by a constant expression"

	return 0;
}

