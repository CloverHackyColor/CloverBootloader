//
//  XBool.hpp
//  CloverX64
//
//  Created by Jief on 27/09/2021.
//

#ifndef XBool_hpp
#define XBool_hpp

#include <XToolsConf.h>
#include "XToolsCommon.h"

class XBool;

template <class _Tp>  struct _xtools__is_xbool_st                        : public _xtools__false_type {};
template <>           struct _xtools__is_xbool_st<XBool>                 : public _xtools__true_type {};
#define is_xbool(x) _xtools__is_xbool_st<remove_const(x)>::value

class XBool
{
    bool value;
public:
    constexpr XBool() : value(false) {}
    //explicit XBool(const bool other) { value = other; }
    constexpr XBool(bool other) : value(other) {}
    constexpr XBool(const XBool& other) : value(other.value) {}
    template <typename T, enable_if( ! is_bool(T) && ! is_xbool(T) )>
    XBool(T other) = delete;

    ~XBool() {};

    XBool& operator= (const XBool& other) { value = other.value; return *this; }
    XBool& operator= (const bool other) { value = other; return *this; }
    template <typename T, enable_if( ! is_bool(T) && ! is_xbool(T) )>
    XBool& operator= (const T other) = delete;
    
    operator bool() const { return value; }

    bool operator == (bool other) const { return value == other; }
    bool operator == (XBool other) const { return value == other.value; }
    template <typename T, enable_if( ! is_bool(T) && ! is_xbool(T) )>
    bool operator == (T other) const = delete;
    bool operator != (bool other) const { return value != other; }
    bool operator != (XBool other) const { return value != other.value; }
    template <typename T, enable_if( ! is_bool(T) && ! is_xbool(T) )>
    bool operator != (XBool other) const = delete;

    XBool& operator |= (bool other) { value |= other; return *this; }
    XBool& operator |= (XBool other) { value |= other.value; return *this; }
    template <typename T, enable_if( ! is_bool(T) && ! is_xbool(T) )>
    XBool& operator |= (XBool other) = delete;
};

#endif /* XBool_hpp */
