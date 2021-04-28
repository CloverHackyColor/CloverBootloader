/*
 * AssignSettings.cpp
 *
 *  Created on: Feb 2, 2021
 *      Author: jief
 */

#include "AssignSettings.h"
#include <Platform.h>
#include "../../include/OSFlags.h"

uint64_t AssignField_nbError = 0;
XString8 AssignField_firstErrorField;


bool AssignField(bool& SETTINGS_DATA_value, bool newValue, const XString8& label)
{
  SETTINGS_DATA_value = newValue;
  return true;
}

bool AssignField(undefinable_bool& SETTINGS_DATA_value, undefinable_bool newValue, const XString8& label)
{
  SETTINGS_DATA_value = newValue;
  return true;
}

//bool AssignField(unsigned char& SETTINGS_DATA_value, unsigned char newValue, const XString8& label)
//{
//  SETTINGS_DATA_value = newValue;
//  return true;
//}

bool AssignField(uint8_t& SETTINGS_DATA_value, uint8_t newValue, const XString8& label)
{
  SETTINGS_DATA_value = newValue;
  return true;
}

bool AssignField(int8_t& SETTINGS_DATA_value, int8_t newValue, const XString8& label)
{
  SETTINGS_DATA_value = newValue;
  return true;
}

bool AssignField(uint16_t& SETTINGS_DATA_value, uint16_t newValue, const XString8& label)
{
  SETTINGS_DATA_value = newValue;
  return true;
}

bool AssignField(undefinable_uint16& SETTINGS_DATA_value, undefinable_uint16 newValue, const XString8& label)
{
  SETTINGS_DATA_value = newValue;
  return true;
}

bool AssignField(int16_t& SETTINGS_DATA_value, int16_t newValue, const XString8& label)
{
  SETTINGS_DATA_value = newValue;
  return true;
}

bool AssignField(uint32_t& SETTINGS_DATA_value, uint32_t newValue, const XString8& label)
{
  SETTINGS_DATA_value = newValue;
  return true;
}

bool AssignField(undefinable_uint32& SETTINGS_DATA_value, undefinable_uint32 newValue, const XString8& label)
{
  SETTINGS_DATA_value = newValue;
  return true;
}

bool AssignField(int32_t& SETTINGS_DATA_value, int32_t newValue, const XString8& label)
{
  SETTINGS_DATA_value = newValue;
  return true;
}


bool AssignField(uint64_t& SETTINGS_DATA_value, uint64_t newValue, const XString8& label)
{
  SETTINGS_DATA_value = newValue;
  return true;
}

bool AssignField(int64_t& SETTINGS_DATA_value, int64_t newValue, const XString8& label)
{
  SETTINGS_DATA_value = newValue;
  return true;
}

bool AssignField(size_t& SETTINGS_DATA_value, size_t newValue, const XString8& label)
{
  SETTINGS_DATA_value = newValue;
  return true;
}

bool AssignField(wchar_t& SETTINGS_DATA_value, wchar_t newValue, const XString8& label)
{
  SETTINGS_DATA_value = newValue;
  return true;
}

bool AssignField(XString8& SETTINGS_DATA_value, const XString8& newValue, const XString8& label)
{
  SETTINGS_DATA_value = newValue;
  return true;
}

bool AssignField(XStringW& SETTINGS_DATA_value, const XStringW& newValue, const XString8& label)
{
  SETTINGS_DATA_value = newValue;
  return true;
}

bool AssignField(undefinable_XString8& SETTINGS_DATA_value, const undefinable_XString8& newValue, const XString8& label)
{
  SETTINGS_DATA_value = newValue;
  return true;
}

bool AssignField(XBuffer<UINT8>& SETTINGS_DATA_value, const XBuffer<UINT8>& newValue, const XString8& label)
{
  SETTINGS_DATA_value = newValue;
  return true;
}

bool AssignField(XBuffer<UINT8>& SETTINGS_DATA_value, const XmlData& newValue, const XString8& label)
{
  SETTINGS_DATA_value = newValue.value();
  return true;
}

bool AssignField(XString8Array& SETTINGS_DATA_value, const XString8Array& newValue, const XString8&  label)
{
  SETTINGS_DATA_value = newValue;
  return true;
}

bool AssignField(XString8Array& SETTINGS_DATA_value, const ConstXString8Array& newValue, const XString8&  label)
{
  SETTINGS_DATA_value.import(newValue);
  return true;
}

bool AssignField(XStringWArray& SETTINGS_DATA_value, const XStringWArray& newValue, const XString8&  label)
{
  SETTINGS_DATA_value = newValue;
  return true;
}

bool AssignField(XStringWArray& SETTINGS_DATA_value, const ConstXStringWArray& newValue, const XString8&  label)
{
//  SETTINGS_DATA_value.import(newValue);
  SETTINGS_DATA_value = newValue;
  return true;
}

bool AssignField(ConstXStringWArray& SETTINGS_DATA_value, const ConstXStringWArray& newValue, const XString8&  label)
{
  SETTINGS_DATA_value = newValue;
  return true;
}

bool AssignField(EFI_GRAPHICS_OUTPUT_BLT_PIXEL& SETTINGS_DATA_value, const EFI_GRAPHICS_OUTPUT_BLT_PIXEL& newValue, const XString8&  label)
{
  SETTINGS_DATA_value = newValue;
  return true;
}

bool AssignField(MISC_SLOT_TYPE& SETTINGS_DATA_value, MISC_SLOT_TYPE newValue, const XString8& label)
{
  SETTINGS_DATA_value = newValue;
  return true;
}

bool AssignField(TAG_TYPE& SETTINGS_DATA_value, TAG_TYPE newValue, const XString8& label)
{
  SETTINGS_DATA_value = newValue;
  return true;
}

bool AssignField(LanguageCode& SETTINGS_DATA_value, LanguageCode newValue, const XString8& label)
{
  SETTINGS_DATA_value = newValue;
  return true;
}


//
//bool AssignField(void*& SETTINGS_DATA_value, size_t& oldSize, const void* newValue, size_t newSize, const XString8&  label)
//{
//  SETTINGS_DATA_value = malloc(newSize);
//  memcpy(SETTINGS_DATA_value, newValue, newSize);
//  oldSize = newSize;
//  return true;
//}
////
//
//bool AssignField(uint8_t*& SETTINGS_DATA_value, size_t& oldSize, const uint8_t* newValue, size_t newSize, const XString8&  label)
//{
//  return AssignField((void*&)SETTINGS_DATA_value, (size_t&)oldSize, (void*)newValue, newSize, label);
//}
//
//bool AssignField(uint8_t SETTINGS_DATA_value[], size_t oldSize, const uint8_t* newValue, size_t newSize, const XString8& label)
//{
//  if ( oldSize == newSize) {
//    memcpy(SETTINGS_DATA_value, newValue, newSize);
//  }else{
//    panic("------------> AssignField bug\n");
//  }
//  return true;
//}
//
