/*
 * AssignSettings.h
 *
 *  Created on: Feb 2, 2021
 *      Author: jief
 */

#ifndef _CONFIGPLIST_AssignFIELD_H_
#define _CONFIGPLIST_AssignFIELD_H_

//#include "../../"

extern "C" {
  #include <Protocol/GraphicsOutput.h>
}

extern uint64_t AssignField_nbError;
extern XString8 AssignField_firstErrorField;

bool AssignFieldFailed(const XString8& msg);
bool AssignFieldFailed(const XString8& field, const XString8& SETTINGS_DATA_value, const XString8& newValue);

bool AssignField(bool& SETTINGS_DATA_value, bool newValue, const XString8& label);
bool AssignField(undefinable_bool& SETTINGS_DATA_value, undefinable_bool newValue, const XString8& label);
bool AssignField(uint8_t& SETTINGS_DATA_value, uint8_t newValue, const XString8& label);
bool AssignField(int8_t& SETTINGS_DATA_value, int8_t newValue, const XString8& label);
bool AssignField(uint16_t& SETTINGS_DATA_value, uint16_t newValue, const XString8& label);
bool AssignField(undefinable_uint16& SETTINGS_DATA_value, undefinable_uint16 newValue, const XString8& label);
bool AssignField(int16_t& SETTINGS_DATA_value, int16_t newValue, const XString8& label);
bool AssignField(uint32_t& SETTINGS_DATA_value, uint32_t newValue, const XString8& label);
bool AssignField(undefinable_uint32& SETTINGS_DATA_value, undefinable_uint32 newValue, const XString8& label);
bool AssignField(int32_t& SETTINGS_DATA_value, int32_t newValue, const XString8& label);
bool AssignField(uint64_t& SETTINGS_DATA_value, uint64_t newValue, const XString8& label);
bool AssignField(int64_t& SETTINGS_DATA_value, int64_t newValue, const XString8& label);
bool AssignField(size_t& SETTINGS_DATA_value, size_t newValue, const XString8& label);
bool AssignField(wchar_t& SETTINGS_DATA_value, wchar_t newValue, const XString8& label);
bool AssignField(XString8& SETTINGS_DATA_value, const XString8& newValue, const XString8& label);
bool AssignField(XStringW& SETTINGS_DATA_value, const XStringW& newValue, const XString8& label);
bool AssignField(undefinable_XString8& SETTINGS_DATA_value, const undefinable_XString8& newValue, const XString8& label);
bool AssignField(XBuffer<UINT8>& SETTINGS_DATA_value, const XBuffer<UINT8>& newValue, const XString8& label);
bool AssignField(XBuffer<UINT8>& SETTINGS_DATA_value, const XmlData& newValue, const XString8& label);
bool AssignField(XString8Array& SETTINGS_DATA_value, const XString8Array& newValue, const XString8& label);
bool AssignField(XString8Array& SETTINGS_DATA_value, const ConstXString8Array& newValue, const XString8& label);
bool AssignField(XStringWArray& SETTINGS_DATA_value, const XStringWArray& newValue, const XString8& label);
bool AssignField(XStringWArray& SETTINGS_DATA_value, const ConstXStringWArray& newValue, const XString8& label);
bool AssignField(EFI_GRAPHICS_OUTPUT_BLT_PIXEL& SETTINGS_DATA_value, const EFI_GRAPHICS_OUTPUT_BLT_PIXEL& newValue, const XString8& label);

bool AssignField(MISC_SLOT_TYPE& SETTINGS_DATA_value, MISC_SLOT_TYPE newValue, const XString8& label);
bool AssignField(TAG_TYPE& SETTINGS_DATA_value, TAG_TYPE newValue, const XString8& label);
bool AssignField(LanguageCode& SETTINGS_DATA_value, LanguageCode newValue, const XString8& label);

bool AssignField(void*& SETTINGS_DATA_value, size_t& oldSize, const void* newValue, size_t newSize, const XString8& label);
//bool AssignField(void*& SETTINGS_DATA_value, size_t oldSize, const void* newValue, size_t newSize, const XString8& label);
bool AssignField(uint8_t*& SETTINGS_DATA_value, size_t& oldSize, const uint8_t* newValue, size_t newSize, const XString8& label);
bool AssignField(uint8_t SETTINGS_DATA_value[], size_t oldSize, const uint8_t* newValue, size_t newSize, const XString8& label);


#define Assign(x) AssignField(oldS.x, newS.dget##x(), S8Printf("%s.%s", label.c_str(), STRINGIZE(x)))
#define sizeAssign(x) true

#endif /* _CONFIGPLIST_AssignFIELD_H_ */
