/*
 * CompareSettings.cpp
 *
 *  Created on: Feb 2, 2021
 *      Author: jief
 */

#include "CompareSettings.h"
#include <Platform.h>
#include "../../include/OSFlags.h"

uint64_t compareField_nbError = 0;
XString8 compareField_firstErrorField;

bool CompareFieldFailed(const XString8& field)
{
  MsgLog("ERROR CONFIG PLIST : the field '%s' differs in new SETTINGS_DATA\n", field.c_str());
  compareField_nbError += 1;
  if ( compareField_firstErrorField.isEmpty() ) compareField_firstErrorField = field;
  return false;
}

bool CompareFieldFailed(const XString8& field, const XString8& oldValue, const XString8& newValue)
{
  MsgLog("ERROR CONFIG PLIST : the field '%s' differs in new SETTINGS_DATA. oldS value '%s', new value '%s'\n", field.c_str(), oldValue.c_str(), newValue.c_str());
  compareField_nbError += 1;
  if ( compareField_firstErrorField.isEmpty() ) compareField_firstErrorField = field;
  return false;
}


bool compareField(bool oldValue, bool newValue, const XString8& label)
{
  if ( oldValue != newValue ) return CompareFieldFailed(label, S8Printf("%d", oldValue), S8Printf("%d", newValue));
  return true;
}

bool compareField(undefinable_bool oldValue, undefinable_bool newValue, const XString8& label)
{
  if ( oldValue.isDefined() != newValue.isDefined() ) return CompareFieldFailed(S8Printf("%s.isDefined()", label.c_str()), S8Printf("%d", oldValue.isDefined()), S8Printf("%d", newValue.isDefined()));
  if ( !oldValue.isDefined() ) return true;
  if ( oldValue.value() != newValue.value() ) return CompareFieldFailed(label, S8Printf("%d", oldValue.value()), S8Printf("%d", newValue.value()));
  return true;
}

//bool compareField(unsigned char oldValue, unsigned char newValue, const XString8& label)
//{
//  if ( oldValue != newValue ) return CompareFieldFailed(label, S8Printf("%u", oldValue), S8Printf("%u", newValue));
//  return true;
//}

bool compareField(uint8_t oldValue, uint8_t newValue, const XString8& label)
{
  if ( oldValue != newValue ) return CompareFieldFailed(label, S8Printf("%u", oldValue), S8Printf("%u", newValue));
  return true;
}

bool compareField(int8_t oldValue, int8_t newValue, const XString8& label)
{
  if ( oldValue != newValue ) return CompareFieldFailed(label, S8Printf("%d", oldValue), S8Printf("%d", newValue));
  return true;
}

bool compareField(uint16_t oldValue, uint16_t newValue, const XString8& label)
{
  if ( oldValue != newValue ) return CompareFieldFailed(label, S8Printf("%u", oldValue), S8Printf("%u", newValue));
  return true;
}

bool compareField(undefinable_uint16 oldValue, undefinable_uint16 newValue, const XString8& label)
{
  if ( oldValue.isDefined() != newValue.isDefined() ) return CompareFieldFailed(S8Printf("%s.isDefined()", label.c_str()), S8Printf("%d", oldValue.isDefined()), S8Printf("%d", newValue.isDefined()));
  if ( !oldValue.isDefined() ) return true;
  if ( oldValue.value() != newValue.value() ) return CompareFieldFailed(label, S8Printf("%d", oldValue.value()), S8Printf("%d", newValue.value()));
  return true;
}

bool compareField(int16_t oldValue, int16_t newValue, const XString8& label)
{
  if ( oldValue != newValue ) return CompareFieldFailed(label, S8Printf("%d", oldValue), S8Printf("%d", newValue));
  return true;
}

bool compareField(uint32_t oldValue, uint32_t newValue, const XString8& label)
{
  if ( oldValue != newValue ) return CompareFieldFailed(label, S8Printf("%u", oldValue), S8Printf("%u", newValue));
  return true;
}

bool compareField(undefinable_uint32 oldValue, undefinable_uint32 newValue, const XString8& label)
{
  if ( oldValue.isDefined() != newValue.isDefined() ) return CompareFieldFailed(S8Printf("%s.isDefined()", label.c_str()), S8Printf("%d", oldValue.isDefined()), S8Printf("%d", newValue.isDefined()));
  if ( !oldValue.isDefined() ) return true;
  if ( oldValue.value() != newValue.value() ) return CompareFieldFailed(label, S8Printf("%d", oldValue.value()), S8Printf("%d", newValue.value()));
  return true;
}

bool compareField(int32_t oldValue, int32_t newValue, const XString8& label)
{
  if ( oldValue != newValue ) return CompareFieldFailed(label, S8Printf("%d", oldValue), S8Printf("%d", newValue));
  return true;
}


bool compareField(uint64_t oldValue, uint64_t newValue, const XString8& label)
{
  if ( oldValue != newValue ) return CompareFieldFailed(label, S8Printf("%llu", oldValue), S8Printf("%llu", newValue));
  return true;
}

bool compareField(int64_t oldValue, int64_t newValue, const XString8& label)
{
  if ( oldValue != newValue ) return CompareFieldFailed(label, S8Printf("%lld", oldValue), S8Printf("%lld", newValue));
  return true;
}

bool compareField(size_t oldValue, size_t newValue, const XString8& label)
{
  if ( oldValue != newValue ) return CompareFieldFailed(label, S8Printf("%zu", oldValue), S8Printf("%zu", newValue));
  return true;
}

bool compareField(wchar_t oldValue, wchar_t newValue, const XString8& label)
{
  if ( oldValue != newValue ) return CompareFieldFailed(label, S8Printf("%d", oldValue), S8Printf("%d", newValue));
  return true;
}

bool compareField(const XString8& oldValue, const XString8& newValue, const XString8& label)
{
  if ( oldValue != newValue ) return CompareFieldFailed(label, oldValue, newValue);
  return true;
}

bool compareField(const undefinable_XString8& oldValue, const undefinable_XString8& newValue, const XString8& label)
{
  if ( oldValue.isDefined() != newValue.isDefined() ) return CompareFieldFailed(S8Printf("%s.isDefined()", label.c_str()), S8Printf("%u", oldValue.isDefined()), S8Printf("%u", newValue.isDefined()));
  if ( !oldValue.isDefined() ) return true;
  if ( oldValue.value() != newValue.value() ) return CompareFieldFailed(label, oldValue.value(), newValue.value());
  return true;
}

bool compareField(const XBuffer<UINT8>& oldValue, const XBuffer<UINT8>& newValue, const XString8& label)
{
  if ( oldValue != newValue ) {
    (void)(oldValue != newValue);
    return CompareFieldFailed(label);
  }
  return true;
}

bool compareField(const XBuffer<UINT8>& oldValue, const XmlData& newValue, const XString8& label)
{
  if ( !newValue.isDefined() ) {
    if ( oldValue.notEmpty() ) return CompareFieldFailed(S8Printf("%s.isDefined()", label.c_str()), "oldValue.notEmpty()"_XS8, "!newValue.isDefined()"_XS8);
    return true;
  }
  if ( oldValue != newValue.value() ) return CompareFieldFailed(label);
  return true;
}

bool compareField(const XString8Array& oldValue, const XString8Array& newValue, const XString8& label)
{
  if ( oldValue != newValue ) return CompareFieldFailed(label, S8Printf("%s", oldValue.ConcatAll().c_str()), S8Printf("%s", newValue.ConcatAll().c_str()));
  return true;
}

bool compareField(const XString8Array& oldValue, const ConstXString8Array& newValue, const XString8& label)
{
  if ( oldValue != newValue ) return CompareFieldFailed(label, S8Printf("%s", oldValue.ConcatAll().c_str()), S8Printf("%s", newValue.ConcatAll().c_str()));
  return true;
}

bool compareField(const XStringWArray& oldValue, const XStringWArray& newValue, const XString8& label)
{
  if ( oldValue != newValue ) return CompareFieldFailed(label, S8Printf("%ls", oldValue.ConcatAll().wc_str()), S8Printf("%ls", newValue.ConcatAll().wc_str()));
  return true;
}

bool compareField(const XStringWArray& oldValue, const ConstXStringWArray& newValue, const XString8& label)
{
  if ( oldValue != newValue ) return CompareFieldFailed(label, S8Printf("%ls", oldValue.ConcatAll().wc_str()), S8Printf("%ls", newValue.ConcatAll().wc_str()));
  return true;
}

bool compareField(const ConstXStringWArray& oldValue, const ConstXStringWArray& newValue, const XString8& label)
{
  if ( oldValue != newValue ) return CompareFieldFailed(label, S8Printf("%ls", oldValue.ConcatAll().wc_str()), S8Printf("%ls", newValue.ConcatAll().wc_str()));
  return true;
}

bool compareField(const EFI_GRAPHICS_OUTPUT_BLT_PIXEL& oldValue, const EFI_GRAPHICS_OUTPUT_BLT_PIXEL& newValue, const XString8& label)
{
  if ( memcmp(&oldValue, &newValue, sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL)) != 0 ) return CompareFieldFailed(label);
  return true;
}

bool compareField(const void* oldValue, size_t oldSize, const void* newValue, size_t newSize, const XString8& label)
{
  if ( oldSize != newSize ) return CompareFieldFailed(S8Printf("%s size", label.c_str()), S8Printf("%zu", oldSize), S8Printf("%zu", newSize));
  if ( memcmp(oldValue, newValue, oldSize) != 0 ) return CompareFieldFailed(S8Printf("%s", label.c_str()));
  return true;
}

bool compareField(const void* oldValue, size_t oldSize, XBuffer<uint8_t> newValue, const XString8& label)
{
  if ( oldSize != newValue.size() ) return CompareFieldFailed(S8Printf("%s size", label.c_str()), S8Printf("%zu", oldSize), S8Printf("%zu", newValue.size()));
  if ( memcmp(oldValue, newValue.data(), oldSize) != 0 ) return CompareFieldFailed(S8Printf("%s", label.c_str()));
  return true;
}


bool compareField(const uint8_t* oldValue, size_t oldSize, const uint8_t* newValue, size_t newSize, const XString8& label)
{
  return compareField((void*)oldValue, oldSize, (void*)newValue,newSize, label);
}

