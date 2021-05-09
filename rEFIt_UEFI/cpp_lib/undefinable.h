/*
 *
 * Copyright (c) 2020 Jief
 * All rights reserved.
 *
 */

#ifndef CPP_LIB_DEF_TYPES_H_
#define CPP_LIB_DEF_TYPES_H_

template <class T>
class undefinable
{
protected:
  bool m_defined = false;
  T m_value = T();

public:
  bool isDefined() const { return m_defined; }
  void setDefined(bool b) { m_defined = b; }
  
//  T& value() { return m_value; } // never allow to return a modifiable value. We need to hook ever assignment possible.
  const T& value() const {
	  if ( !isDefined() ) {
      panic("get value of an undefined undefinable type");
  }
  	return m_value;
  }
  const T& dgetValue() const { return m_value; } // if !m_defined, m_value = T()

  explicit operator const T&() const {
    if ( !isDefined() ) panic("get value of an undefined undefinable type");
    return m_value;
  }
  undefinable<T>& operator = (T value) {
    setDefined(true);
    m_value = value;
    return *this;
  }
  
  bool operator ==(const undefinable<T>& other) const
  {
    if ( !(m_defined == other.m_defined ) ) return false;
    if ( m_defined && !(m_value == other.m_value ) ) return false; // we don't test value if this is not defined.
    return true;
  }
  bool operator !=(const undefinable<T>& other) const { return !(*this == other); }
};

class undefinable_bool : public undefinable<bool>
{
  using super = undefinable<bool>;
public:
    undefinable_bool() { }
    explicit undefinable_bool(bool newValue) { super::operator=(newValue); }
    undefinable_bool& operator = (bool newValue) { super::operator=(newValue); return *this; }
};

class undefinable_uint8 : public undefinable<uint8_t>
{
  using super = undefinable<uint8_t>;
public:
    undefinable_uint8() { }
    explicit undefinable_uint8(uint8_t newValue) { super::operator=(newValue); }
    undefinable_uint8& operator = (uint8_t newValue) { super::operator=(newValue); return *this; }
};

class undefinable_uint16 : public undefinable<uint16_t>
{
  using super = undefinable<uint16_t>;
public:
    undefinable_uint16() { }
    explicit undefinable_uint16(uint16_t newValue) { super::operator=(newValue); }
    undefinable_uint16& operator = (uint16_t newValue) { super::operator=(newValue); return *this; }
};

class undefinable_uint32 : public undefinable<uint32_t>
{
  using super = undefinable<uint32_t>;
public:
    undefinable_uint32() { }
    explicit undefinable_uint32(uint32_t newValue) { super::operator=(newValue); }
    undefinable_uint32& operator = (uint32_t newValue) { super::operator=(newValue); return *this; }
};

class undefinable_XString8 : public undefinable<XString8>
{
  using super = undefinable<XString8>;
public:
    undefinable_XString8() { }
    explicit undefinable_XString8(const XString8& newValue) { super::operator=(newValue); }
    undefinable_XString8& operator = (XString8 newValue) { super::operator=(newValue); return *this; }
};


#endif /* CPP_LIB_DEF_TYPES_H_ */
