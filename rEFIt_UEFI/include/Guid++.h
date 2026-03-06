/*
 * guid.h
 *
 *  Created on: 16 Apr 2020
 *      Author: jief
 */

#ifndef GUID_PLUS_PLUS_H_
#define GUID_PLUS_PLUS_H_

extern "C++"
{

//typedef int GUID;
#define GUID_PLUSPLUS_DEFINED
class GUID;
typedef GUID         EFI_GUID;
#define CONST_EFI_GUID_PTR_T  const EFI_GUID&
#define JCONST_EFI_GUID_PTR_T  const EFI_GUID&

extern "C" {
#include <Uefi/UefiBaseType.h>
}

#include <stddef.h>
#include "../cpp_foundation/XToolsCommon.h"
#include "../cpp_foundation/XString.h"


constexpr EFI_GUID operator ""_guid(const char *str, size_t N);

/*
 * Class to replace struct EFI_GUID to bring some syntaxic sugar : initialisation at construction, assignment, == operator, etc.
 */

// This class is just to enable static assignment of form : EFI_GUID guid = {0xF913C2C2, 0x5351, 0x4FDB, {0x93, 0x44, 0x70, 0xFF, 0xED, 0xB8, 0x42, 0x25}}
class GUID_Data4
{
public:
 UINT8 i0;
 UINT8 i1;
 UINT8 i2;
 UINT8 i3;
 UINT8 i4;
 UINT8 i5;
 UINT8 i6;
 UINT8 i7;
};


class GUID
{
public:
  UINT32  Data1;
  UINT16  Data2;
  UINT16  Data3;
  UINT8   Data4[8];

  constexpr GUID() : Data1(0), Data2(0), Data3(0), Data4{0,0,0,0,0,0,0,0} {}

  // 2022-05 : defining a copy ctor force to define a copy assignment.
  //constexpr GUID(const GUID& other) : GUID{other.Data1, other.Data2, other.Data3, {other.Data4[0], other.Data4[1], other.Data4[2], other.Data4[3], other.Data4[4], other.Data4[5], other.Data4[6], other.Data4[7]}} { }
  constexpr GUID(UINT32 _data1, UINT16 _data2, UINT16 _data3, const GUID_Data4& _data4) : Data1(_data1), Data2(_data2), Data3(_data3), Data4{_data4.i0, _data4.i1, _data4.i2, _data4.i3, _data4.i4, _data4.i5, _data4.i6, _data4.i7} { }

  // 2022-05 : I don't know how to define a constexpr copy assignment. The compiler does it for me.
//  constexpr const GUID& operator = (const GUID& other) { return /*(void)(Data1 = other.Data1), static_cast<void>(Data2 = other.Data2), (void)(Data3 = other.Data3), (void)(Data4[0] = other.Data4[0]), (void)(Data4[1] = other.Data4[1]), (void)(Data4[2] = other.Data4[2]), (void)(Data4[3] = other.Data4[3]), (void)(Data4[4] = other.Data4[4]), (void)(Data4[5] = other.Data4[5]), (void)(Data4[6] = other.Data4[6]), (void)(Data4[7] = other.Data4[7]), */*this; };

  constexpr bool operator == (const GUID& other) const {
    return Data1 == other.Data1  &&  Data2 == other.Data2  &&  Data3 == other.Data3  && Data4[0] == other.Data4[0]  &&  Data4[1] == other.Data4[1]  &&  Data4[2] == other.Data4[2]  &&  Data4[3] == other.Data4[3]  &&  Data4[4] == other.Data4[4]  &&  Data4[5] == other.Data4[5]  &&  Data4[6] == other.Data4[6]  &&  Data4[7] == other.Data4[7];
  }
  constexpr bool operator != (const GUID& other) const { return ! (*this == other); }

  void setNull() { Data1 = 0; *this = GUID(); }
  constexpr bool isNull() const { return Data1 == 0  &&  Data2 == 0  &&  Data3 == 0  &&  Data4[0] == 0  &&  Data4[1] == 0  &&  Data4[2] == 0  &&  Data4[3] == 0  &&  Data4[4] == 0  &&  Data4[5] == 0  &&  Data4[6] == 0  &&  Data4[7] == 0; }
  constexpr bool notNull() const { return !isNull(); }
  
  int getVariant() const {
    if ( Data4[0] < 0x80 ) return 0;
    if ( Data4[0] < 0xC0 ) return 1;
    if ( Data4[0] < 0xE0 ) return 2;
    return 3;
  }

private:
  // Helper function for _guid litteral operator. They are private because they are not made to be called directly.
  // Conversion from litteral comes from https://github.com/tobias-loew/constexpr-GUID-cpp-11
  static constexpr const size_t short_guid_form_length = 36;  // XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX
  static constexpr const size_t long_guid_form_length = 38;  // {XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX}

  static int parse_guid_error() { panic("Incorrect format for guid operator."); return 0; } // IMPORTANT : not constexpr
  static EFI_GUID parse_guid_error2() { panic("Incorrect format for guid operator."); return GUID(); } // IMPORTANT : not constexpr

  
  static constexpr bool is_hex_digit(const char c)
  {
    return ('0' <= c && c <= '9') || ('a' <= c && c <= 'f') || ('A' <= c && c <= 'F');
  }

  template <typename T, enable_if( is_char(T) ) >
  static constexpr uint8_t parse_hex_digit(const T c)
  {
    return
      ('0' <= c && c <= '9')
      ? c - '0'
      : ('a' <= c && c <= 'f')
        ? 10 + c - 'a'
        : ('A' <= c && c <= 'F')
        ? 10 + c - 'A'
        : parse_guid_error();
      //   : throw "invalid character in GUID"; // throw doesn't compile with -fno-exception.
  }

  template <typename T, enable_if( is_char(T) ) >
  static constexpr uint8_t parse_hex_uint8_t(const T *ptr)
  {
    return (parse_hex_digit(ptr[0]) << 4) + parse_hex_digit(ptr[1]);
  }

  template <typename T, enable_if( is_char(T) ) >
  static constexpr uint16_t parse_hex_uint16_t(const T *ptr)
  {
    return (parse_hex_uint8_t(ptr) << 8) + parse_hex_uint8_t(ptr + 2);
  }

  template <typename T, enable_if( is_char(T) ) >
  static constexpr uint32_t parse_hex_uint32_t(const T* ptr)
  {
    return (parse_hex_uint16_t(ptr) << 16) + parse_hex_uint16_t(ptr + 4);
  }
  
  static constexpr uint16_t parse_hex_uint16_t_be(const char *ptr)
  {
    return (parse_hex_uint8_t(ptr + 2) << 8) + parse_hex_uint8_t(ptr);
  }

  static constexpr uint32_t parse_hex_uint32_t_be(const char *ptr)
  {
    return (parse_hex_uint16_t_be(ptr + 4) << 16) + parse_hex_uint16_t_be(ptr);
  }


  // This function call parse_guid_error2(). That's a way to get a compile error.
  // Therefore, if called fron non-constexpr, it panics.
  static constexpr GUID parse_guid(const char *begin)
  {
    return
      begin[8] != '-'
      || begin[13] != '-'
      || begin[18] != '-'
      || begin[23] != '-'
      ? parse_guid_error2()
      : GUID{
        parse_hex_uint32_t(begin),
        parse_hex_uint16_t(begin + 8 + 1),
        parse_hex_uint16_t(begin + 8 + 1 + 4 + 1),
        {
          parse_hex_uint8_t(begin + 8 + 1 + 4 + 1 + 4 + 1),
          parse_hex_uint8_t(begin + 8 + 1 + 4 + 1 + 4 + 1 + 2),
          parse_hex_uint8_t(begin + 8 + 1 + 4 + 1 + 4 + 1 + 2 + 2 + 1),
          parse_hex_uint8_t(begin + 8 + 1 + 4 + 1 + 4 + 1 + 2 + 2 + 1 + 2),
          parse_hex_uint8_t(begin + 8 + 1 + 4 + 1 + 4 + 1 + 2 + 2 + 1 + 2 + 2),
          parse_hex_uint8_t(begin + 8 + 1 + 4 + 1 + 4 + 1 + 2 + 2 + 1 + 2 + 2 + 2),
          parse_hex_uint8_t(begin + 8 + 1 + 4 + 1 + 4 + 1 + 2 + 2 + 1 + 2 + 2 + 2 + 2),
          parse_hex_uint8_t(begin + 8 + 1 + 4 + 1 + 4 + 1 + 2 + 2 + 1 + 2 + 2 + 2 + 2 + 2)
        }
      };
  }
  
  friend constexpr GUID operator ""_guid(const char *str, size_t N);

public:
  
  /** Returns true is Str is ascii Guid in format xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx */
  template <typename T, typename IntegralType, enable_if( is_char(T) && is_integral(IntegralType) ) >
  static constexpr bool IsValidGuidString(const T* s, IntegralType n)
  {
    return
    (
      ( n == 36
             && is_hex_digit(s[0])
             && is_hex_digit(s[1])
             && is_hex_digit(s[2])
             && is_hex_digit(s[3])
             && is_hex_digit(s[4])
             && is_hex_digit(s[5])
             && is_hex_digit(s[6])
             && is_hex_digit(s[7])
             && s[8] == '-'
             && is_hex_digit(s[9])
             && is_hex_digit(s[10])
             && is_hex_digit(s[11])
             && is_hex_digit(s[12])
             && s[13] == '-'
             && is_hex_digit(s[14])
             && is_hex_digit(s[15])
             && is_hex_digit(s[16])
             && is_hex_digit(s[17])
             && s[18] == '-'
             && is_hex_digit(s[19])
             && is_hex_digit(s[20])
             && is_hex_digit(s[21])
             && is_hex_digit(s[22])
             && s[23] == '-'
             && is_hex_digit(s[24])
             && is_hex_digit(s[25])
             && is_hex_digit(s[26])
             && is_hex_digit(s[27])
             && is_hex_digit(s[28])
             && is_hex_digit(s[29])
             && is_hex_digit(s[30])
             && is_hex_digit(s[31])
             && is_hex_digit(s[32])
             && is_hex_digit(s[33])
             && is_hex_digit(s[34])
             && is_hex_digit(s[35])
      )
      ||
      ( n == 38
             && s[0] == '{'
             && is_hex_digit(s[1])
             && is_hex_digit(s[2])
             && is_hex_digit(s[3])
             && is_hex_digit(s[4])
             && is_hex_digit(s[5])
             && is_hex_digit(s[6])
             && is_hex_digit(s[7])
             && is_hex_digit(s[8])
             && s[9] == '-'
             && is_hex_digit(s[10])
             && is_hex_digit(s[11])
             && is_hex_digit(s[12])
             && is_hex_digit(s[13])
             && s[14] == '-'
             && is_hex_digit(s[15])
             && is_hex_digit(s[16])
             && is_hex_digit(s[17])
             && is_hex_digit(s[18])
             && s[19] == '-'
             && is_hex_digit(s[20])
             && is_hex_digit(s[21])
             && is_hex_digit(s[22])
             && is_hex_digit(s[23])
             && s[24] == '-'
             && is_hex_digit(s[25])
             && is_hex_digit(s[26])
             && is_hex_digit(s[27])
             && is_hex_digit(s[28])
             && is_hex_digit(s[29])
             && is_hex_digit(s[30])
             && is_hex_digit(s[31])
             && is_hex_digit(s[32])
             && is_hex_digit(s[33])
             && is_hex_digit(s[34])
             && is_hex_digit(s[35])
             && is_hex_digit(s[36])
             && s[37] == '}'
      )
    );
  }

  template <typename T, typename IntegralType, enable_if( is_char(T) && is_integral(IntegralType) ) >
  GUID& takeValueFrom(const T* s, IntegralType n)
  {
    if ( !IsValidGuidString(s, n) ) { setNull(); return *this; }

    //  if ( parse_hex_digit(s[19]) == 0x0C || parse_hex_digit(s[19]) == 0x0D ) // Check variant. It's not clear in Clover if we follow that.
    //  That's why there is 2 methods : LE/BE selection is manual
    // LE binary storage
    if ( s[0] == '{' ) s += 1;
    *this =  GUID{
      parse_hex_uint32_t(s),
      parse_hex_uint16_t(s + 8 + 1),
      parse_hex_uint16_t(s + 8 + 1 + 4 + 1),
      {
        parse_hex_uint8_t(s + 8 + 1 + 4 + 1 + 4 + 1),
        parse_hex_uint8_t(s + 8 + 1 + 4 + 1 + 4 + 1 + 2),
        parse_hex_uint8_t(s + 8 + 1 + 4 + 1 + 4 + 1 + 2 + 2 + 1),
        parse_hex_uint8_t(s + 8 + 1 + 4 + 1 + 4 + 1 + 2 + 2 + 1 + 2),
        parse_hex_uint8_t(s + 8 + 1 + 4 + 1 + 4 + 1 + 2 + 2 + 1 + 2 + 2),
        parse_hex_uint8_t(s + 8 + 1 + 4 + 1 + 4 + 1 + 2 + 2 + 1 + 2 + 2 + 2),
        parse_hex_uint8_t(s + 8 + 1 + 4 + 1 + 4 + 1 + 2 + 2 + 1 + 2 + 2 + 2 + 2),
        parse_hex_uint8_t(s + 8 + 1 + 4 + 1 + 4 + 1 + 2 + 2 + 1 + 2 + 2 + 2 + 2 + 2)
      }
    };
    return *this;
  }

  template <typename T, typename IntegralType, enable_if( is_char(T) && is_integral(IntegralType) ) >
  GUID& takeValueFromBE(const T* s, IntegralType n)
  {
    if ( !IsValidGuidString(s, n) ) { setNull(); return *this; }

//  if ( parse_hex_digit(s[19]) == 0x0C || parse_hex_digit(s[19]) == 0x0D ) // Check variant. It's not clear in Clover if we follow that.
//  That's why there is 2 methods : LE/BE selection is manual
    if ( s[0] == '{' ) s += 1;
    *this =  GUID{
      parse_hex_uint32_t_be(s),
      parse_hex_uint16_t_be(s + 8 + 1),
      parse_hex_uint16_t_be(s + 8 + 1 + 4 + 1),
      {
        parse_hex_uint8_t(s + 8 + 1 + 4 + 1 + 4 + 1),
        parse_hex_uint8_t(s + 8 + 1 + 4 + 1 + 4 + 1 + 2),
        parse_hex_uint8_t(s + 8 + 1 + 4 + 1 + 4 + 1 + 2 + 2 + 1),
        parse_hex_uint8_t(s + 8 + 1 + 4 + 1 + 4 + 1 + 2 + 2 + 1 + 2),
        parse_hex_uint8_t(s + 8 + 1 + 4 + 1 + 4 + 1 + 2 + 2 + 1 + 2 + 2),
        parse_hex_uint8_t(s + 8 + 1 + 4 + 1 + 4 + 1 + 2 + 2 + 1 + 2 + 2 + 2),
        parse_hex_uint8_t(s + 8 + 1 + 4 + 1 + 4 + 1 + 2 + 2 + 1 + 2 + 2 + 2 + 2),
        parse_hex_uint8_t(s + 8 + 1 + 4 + 1 + 4 + 1 + 2 + 2 + 1 + 2 + 2 + 2 + 2 + 2)
      }
    };
    return *this;
  }
  
  template <typename T, enable_if( is___String(T) )>
  GUID& takeValueFrom(const T& Str)
  {
    if ( Str.isEmpty() ) { setNull(); return *this; }
    return takeValueFrom(Str.data(), Str.length());
  }
  
  template <typename T, enable_if( is___String(T) )>
  GUID& takeValueFromBE(const T& Str)
  {
    if ( Str.isEmpty() ) { setNull(); return *this; }
    return takeValueFromBE(Str.data(), Str.length());
  }


// Creating a ctor (which allow assignement) from a string is a bit dangerous.
// You might endup writing guid1 = guid2, expecting this assignement to always work.
// But if guid2 is a XString it can fails.
// Assignement is such a basic operator that no one can guess it could fail.
// By being forced to use takeValueFrom, it's clearer that it can fail and that you have to check.
//  template <typename T, enable_if( is___String(T) )>
//  GUID(const T& Str) {
//    takeValueFrom(Str);
//  }



  XString8 toXString8(bool be = false) const
  {
    XString8 returnValue;
//    if ( getVariant() == 2 ) {
    if ( !be ) {
      returnValue.S8Printf("%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
      Data1, Data2, Data3, Data4[0], Data4[1],
      Data4[2], Data4[3], Data4[4], Data4[5], Data4[6], Data4[7]);
    }else{
      UINT8 *GuidData = (UINT8 *)this;
      returnValue.S8Printf("%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
                              GuidData[0], GuidData[1], GuidData[2], GuidData[3],
                              GuidData[4], GuidData[5],
                              GuidData[6], GuidData[7],
                              GuidData[8], GuidData[9], GuidData[10], GuidData[11],
                              GuidData[12], GuidData[13], GuidData[14], GuidData[15]);
    }
    return returnValue;
  }
  
  XString8 toXStringW(bool be = false) const
  {
    XStringW returnValue;
//    if ( getVariant() == 2 ) {
    if ( !be ) {
      returnValue.SWPrintf("%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
      Data1, Data2, Data3, Data4[0], Data4[1],
      Data4[2], Data4[3], Data4[4], Data4[5], Data4[6], Data4[7]);
    }else{
      UINT8 *GuidData = (UINT8 *)this;
      returnValue.SWPrintf("%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
                              GuidData[0], GuidData[1], GuidData[2], GuidData[3],
                              GuidData[4], GuidData[5],
                              GuidData[6], GuidData[7],
                              GuidData[8], GuidData[9], GuidData[10], GuidData[11],
                              GuidData[12], GuidData[13], GuidData[14], GuidData[15]);
    }
    return returnValue;
  }
  
  
  /** Returns true is Str is ascii Guid in format xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx */
  template <typename T, enable_if( is___String(T) )>
  static bool IsValidGuidString(const T& Str)
  {
    return IsValidGuidString(Str.data(), Str.length());
  }

};


constexpr GUID operator ""_guid(const char *str, size_t N)
{
  return (!(N == GUID::long_guid_form_length || N == GUID::short_guid_form_length))
    ? GUID::parse_guid_error2()
    : (N == GUID::long_guid_form_length && (str[0] != '{' || str[GUID::long_guid_form_length - 1] != '}'))
    ? GUID::parse_guid_error2()
    : GUID::parse_guid(str + (N == GUID::long_guid_form_length ? 1 : 0));
}

constexpr const GUID nullGuid;

} // extern C++

#endif /* NEWGUID_H_ */
