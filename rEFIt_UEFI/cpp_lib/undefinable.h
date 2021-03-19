/*
 * def_types.h
 *
 *  Created on: Mar 19, 2021
 *      Author: jief
 */

#ifndef CPP_LIB_DEF_TYPES_H_
#define CPP_LIB_DEF_TYPES_H_

template <class T>
class undefinable
{
protected:
  bool m_defined = false;
  T m_value = false;

public:
  bool isDefined() const { return m_defined; }
  void setDefined(bool b) { m_defined = b; }
  
  operator T() const {
    if ( !isDefined() ) panic("def_type is not defined");
    return m_value;
  }
  undefinable<T>& operator = (T value) {
    setDefined(true);
    m_value = value;
    return *this;
  }
};

class undefinable_bool : public undefinable<bool>
{
  using super = undefinable<bool>;
public:
    undefinable_bool& operator = (bool b) { super::operator=(b); return *this; }
};

class undefinable_uint16 : public undefinable<uint16_t>
{
  using super = undefinable<uint16_t>;
public:
    undefinable_uint16& operator = (uint16_t b) { super::operator=(b); return *this; }
};

class undefinable_uint32 : public undefinable<uint32_t>
{
  using super = undefinable<uint32_t>;
public:
    undefinable_uint32& operator = (uint32_t b) { super::operator=(b); return *this; }
};



#endif /* CPP_LIB_DEF_TYPES_H_ */
