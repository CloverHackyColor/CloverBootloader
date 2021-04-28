/*
 * CompareSettings.h
 *
 *  Created on: Feb 2, 2021
 *      Author: jief
 */

#ifndef _CONFIGPLIST_COMPAREFIELD_H_
#define _CONFIGPLIST_COMPAREFIELD_H_

//#include "../../"

extern "C" {
  #include <Protocol/GraphicsOutput.h>
}

extern uint64_t compareField_nbError;
extern XString8 compareField_firstErrorField;

bool CompareFieldFailed(const XString8& msg);
bool CompareFieldFailed(const XString8& field, const XString8& oldValue, const XString8& newValue);

bool compareField(bool oldValue, bool newValue, const XString8& label);
bool compareField(undefinable_bool oldValue, undefinable_bool newValue, const XString8& label);
bool compareField(uint8_t oldValue, uint8_t newValue, const XString8& label);
bool compareField(int8_t oldValue, int8_t newValue, const XString8& label);
bool compareField(uint16_t oldValue, uint16_t newValue, const XString8& label);
bool compareField(undefinable_uint16 oldValue, undefinable_uint16 newValue, const XString8& label);
bool compareField(int16_t oldValue, int16_t newValue, const XString8& label);
bool compareField(uint32_t oldValue, uint32_t newValue, const XString8& label);
bool compareField(undefinable_uint32 oldValue, undefinable_uint32 newValue, const XString8& label);
bool compareField(int32_t oldValue, int32_t newValue, const XString8& label);
bool compareField(uint64_t oldValue, uint64_t newValue, const XString8& label);
bool compareField(int64_t oldValue, int64_t newValue, const XString8& label);
bool compareField(size_t oldValue, size_t newValue, const XString8& label);
bool compareField(wchar_t oldValue, wchar_t newValue, const XString8& label);
bool compareField(const XString8& oldValue, const XString8& newValue, const XString8& label);
bool compareField(const undefinable_XString8& oldValue, const undefinable_XString8& newValue, const XString8& label);
bool compareField(const XBuffer<UINT8>& oldValue, const XBuffer<UINT8>& newValue, const XString8& label);
bool compareField(const XBuffer<UINT8>& oldValue, const XmlData& newValue, const XString8& label);
bool compareField(const XString8Array& oldValue, const XString8Array& newValue, const XString8& label);
bool compareField(const XString8Array& oldValue, const ConstXString8Array& newValue, const XString8& label);
bool compareField(const XStringWArray& oldValue, const XStringWArray& newValue, const XString8& label);
bool compareField(const XStringWArray& oldValue, const ConstXStringWArray& newValue, const XString8& label);
bool compareField(const EFI_GRAPHICS_OUTPUT_BLT_PIXEL& oldValue, const EFI_GRAPHICS_OUTPUT_BLT_PIXEL& newValue, const XString8& label);

bool compareField(const void* oldValue, size_t oldSize, const void* newValue, size_t newSize, const XString8& label);
bool compareField(const uint8_t* oldValue, size_t oldSize, const uint8_t* newValue, size_t newSize, const XString8& label);


#define compare(x) compareField(oldS.x, newS.dget##x(), S8Printf("%s.%s", label.c_str(), STRINGIZE(x)))
#define fcompare(x) compareField(oldS.x, newS.x, S8Printf("%s.%s", label.c_str(), STRINGIZE(x)))
//#define compare2(x, y) compareField(oldS.x.y, newS.x.dget##y(), CONCAT(STRINGIZE(x) STRINGIZE(.) STRINGIZE(y), _XS8 ))
//#define compare3(x, y, z) compareField(oldS.x.y.z, newS.x.y.dget##z(), S8Printf("%s.%s.%s", STRINGIZE(x), STRINGIZE(y), STRINGIZE(z)))
//#define compare3array(x, y, idx, z) compareField(oldS.x.y[idx].z, newS.x.y[idx].dget##z(), S8Printf("%s.%s[%zu].%s", STRINGIZE(x), STRINGIZE(y), idx, STRINGIZE(z)))
//#define fcompare2(x, y) compareField(oldS.x.y, newS.x.y, CONCAT(STRINGIZE(x) STRINGIZE(.) STRINGIZE(y), _XS8 ))

#endif /* _CONFIGPLIST_COMPAREFIELD_H_ */
