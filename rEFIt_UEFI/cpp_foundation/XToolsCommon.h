//
//  XToolsCommon.h
//  cpp_tests
//
//  Created by jief on 25.04.20.
//  Copyright © 2020 JF Knudsen. All rights reserved.
//

#ifndef XToolsCommon_h
#define XToolsCommon_h


struct XStringAbstract__false_type {
    static constexpr bool value = false;
    bool v() const { return false; }
};

struct XStringAbstract__true_type {
    static constexpr bool value = true;
    bool v() const { return true; }
};

/* make unsigned */
template <class _Tp>
struct XStringAbstract__make_unsigned {};

template <> struct XStringAbstract__make_unsigned<         char>      {typedef unsigned char      type;};
template <> struct XStringAbstract__make_unsigned<  signed char>      {typedef unsigned char      type;};
template <> struct XStringAbstract__make_unsigned<unsigned char>      {typedef unsigned char      type;};
template <> struct XStringAbstract__make_unsigned<     char16_t>      {typedef char16_t           type;};
template <> struct XStringAbstract__make_unsigned<     char32_t>      {typedef char32_t           type;};
template <> struct XStringAbstract__make_unsigned<      wchar_t>      {typedef wchar_t            type;};
template <> struct XStringAbstract__make_unsigned<  signed short>     {typedef unsigned short     type;};
template <> struct XStringAbstract__make_unsigned<unsigned short>     {typedef unsigned short     type;};
template <> struct XStringAbstract__make_unsigned<  signed int>       {typedef unsigned int       type;};
template <> struct XStringAbstract__make_unsigned<unsigned int>       {typedef unsigned int       type;};
template <> struct XStringAbstract__make_unsigned<  signed long>      {typedef unsigned long      type;};
template <> struct XStringAbstract__make_unsigned<unsigned long>      {typedef unsigned long      type;};
template <> struct XStringAbstract__make_unsigned<  signed long long> {typedef unsigned long long type;};
template <> struct XStringAbstract__make_unsigned<unsigned long long> {typedef unsigned long long type;};
#define unsigned_type(x) typename XStringAbstract__make_unsigned<x>::type

/* enable_if */
template <bool, typename T = void>
struct XStringAbstract__enable_if_t
{};

template <typename T>
struct XStringAbstract__enable_if_t<true, T> {
  typedef T type;
};
//#define enable_if(x) XStringAbstract__enable_if_t(x, void)::type
#define enable_if(x) typename enable_if_type = typename XStringAbstract__enable_if_t<x>::type

//
//template< bool B, class T = void >
//using XStringAbstract__enable_if_t = typename XStringAbstract__enable_if<B,T>::type;


// is_integral
template <class _Tp> struct XStringAbstract__is_integral_st                     : public XStringAbstract__false_type {};
template <>          struct XStringAbstract__is_integral_st<bool>               : public XStringAbstract__true_type {};
template <>          struct XStringAbstract__is_integral_st<char>               : public XStringAbstract__true_type {};
template <>          struct XStringAbstract__is_integral_st<signed char>        : public XStringAbstract__true_type {};
template <>          struct XStringAbstract__is_integral_st<unsigned char>      : public XStringAbstract__true_type {};
template <>          struct XStringAbstract__is_integral_st<wchar_t>            : public XStringAbstract__true_type {};
template <>          struct XStringAbstract__is_integral_st<short>              : public XStringAbstract__true_type {};
template <>          struct XStringAbstract__is_integral_st<unsigned short>     : public XStringAbstract__true_type {};
template <>          struct XStringAbstract__is_integral_st<int>                : public XStringAbstract__true_type {};
template <>          struct XStringAbstract__is_integral_st<unsigned int>       : public XStringAbstract__true_type {};
template <>          struct XStringAbstract__is_integral_st<long>               : public XStringAbstract__true_type {};
template <>          struct XStringAbstract__is_integral_st<unsigned long>      : public XStringAbstract__true_type {};
template <>          struct XStringAbstract__is_integral_st<long long>          : public XStringAbstract__true_type {};
template <>          struct XStringAbstract__is_integral_st<unsigned long long> : public XStringAbstract__true_type {};
#define is_integral(x) XStringAbstract__is_integral_st<x>::value


// is_char
template <class _Tp>  struct XStringAbstract__is_char_st                        : public XStringAbstract__false_type {};
template <>           struct XStringAbstract__is_char_st<char>                  : public XStringAbstract__true_type {};
template <>           struct XStringAbstract__is_char_st<signed char>           : public XStringAbstract__true_type {};
template <>           struct XStringAbstract__is_char_st<unsigned char>         : public XStringAbstract__true_type {};
template <>           struct XStringAbstract__is_char_st<char16_t>              : public XStringAbstract__true_type {};
template <>           struct XStringAbstract__is_char_st<char32_t>              : public XStringAbstract__true_type {};
template <>           struct XStringAbstract__is_char_st<wchar_t>               : public XStringAbstract__true_type {};
#define is_char(x) XStringAbstract__is_char_st<x>::value

//
//// STRUCT TEMPLATE remove_reference
//template<class _Ty>
//	struct remove_ref
//	{	// remove reference
//	using type = _Ty;
//	};
//
//template<class _Ty>
//	struct remove_ref<_Ty&>
//	{	// remove reference
//	using type = _Ty;
//	};

// STRUCT TEMPLATE remove_const
template<class _Ty>
	struct remove_const
	{	// remove const
	using type = _Ty;
	};

template<class _Ty>
	struct remove_const<const _Ty>
	{	// remove const
	using type = _Ty;
	};



// is_char_ptr
template <class _Tp>  struct XStringAbstract__is_char_ptr_st                        : public XStringAbstract__false_type {};
template <>           struct XStringAbstract__is_char_ptr_st<char*>                  : public XStringAbstract__true_type {};
template <>           struct XStringAbstract__is_char_ptr_st<char[]>                : public XStringAbstract__true_type {};
template <size_t _Np> struct XStringAbstract__is_char_ptr_st<char[_Np]>             : public XStringAbstract__true_type {};
//template <>           struct XStringAbstract__is_char_ptr_st<signed char>           : public XStringAbstract__true_type {};
//template <>           struct XStringAbstract__is_char_ptr_st<unsigned char>         : public XStringAbstract__true_type {};
template <>           struct XStringAbstract__is_char_ptr_st<char16_t*>              : public XStringAbstract__true_type {};
template <>           struct XStringAbstract__is_char_ptr_st<char16_t[]>            : public XStringAbstract__true_type {};
template <size_t _Np> struct XStringAbstract__is_char_ptr_st<char16_t[_Np]>         : public XStringAbstract__true_type {};
template <>           struct XStringAbstract__is_char_ptr_st<char32_t*>              : public XStringAbstract__true_type {};
template <>           struct XStringAbstract__is_char_ptr_st<char32_t[]>            : public XStringAbstract__true_type {};
template <size_t _Np> struct XStringAbstract__is_char_ptr_st<char32_t[_Np]>         : public XStringAbstract__true_type {};
template <>           struct XStringAbstract__is_char_ptr_st<wchar_t*>               : public XStringAbstract__true_type {};
template <>           struct XStringAbstract__is_char_ptr_st<wchar_t[]>             : public XStringAbstract__true_type {};
template <size_t _Np> struct XStringAbstract__is_char_ptr_st<wchar_t[_Np]>          : public XStringAbstract__true_type {};
#define is_char_ptr(x) XStringAbstract__is_char_ptr_st<typename remove_const<x>::type>::value



#endif /* XToolsCommon_h */
