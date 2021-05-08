/*
 *
 * Created by jief in 1997.
 * Copyright (c) 2020 Jief
 * All rights reserved.
 *
 */

#ifndef XToolsCommon_h
#define XToolsCommon_h

#ifdef __cplusplus

#include <stddef.h> // size_t

struct _xtools__false_type {
    static constexpr bool value = false;
    bool v() const { return false; }
};

struct _xtools__true_type {
    static constexpr bool value = true;
    bool v() const { return true; }
};

/* make unsigned */
template <class _Tp>
struct _xtools__make_unsigned {};

template <> struct _xtools__make_unsigned<              char> {typedef unsigned char      type;};
template <> struct _xtools__make_unsigned<       signed char> {typedef unsigned char      type;};
template <> struct _xtools__make_unsigned<     unsigned char> {typedef unsigned char      type;};
template <> struct _xtools__make_unsigned<          char16_t> {typedef char16_t           type;};
template <> struct _xtools__make_unsigned<          char32_t> {typedef char32_t           type;};
#if WCHAR_MAX <= 0xFFFF
template <> struct _xtools__make_unsigned<           wchar_t> {typedef char16_t           type;};
#else
template <> struct _xtools__make_unsigned<           wchar_t> {typedef char32_t           type;};
#endif
template <> struct _xtools__make_unsigned<             short> {typedef unsigned short     type;};
template <> struct _xtools__make_unsigned<    unsigned short> {typedef unsigned short     type;};
template <> struct _xtools__make_unsigned<               int> {typedef unsigned int       type;};
template <> struct _xtools__make_unsigned<      unsigned int> {typedef unsigned int       type;};
template <> struct _xtools__make_unsigned<              long> {typedef unsigned long      type;};
template <> struct _xtools__make_unsigned<     unsigned long> {typedef unsigned long      type;};
template <> struct _xtools__make_unsigned<         long long> {typedef unsigned long long type;};
template <> struct _xtools__make_unsigned<unsigned long long> {typedef unsigned long long type;};


// is_integral
template <class _Tp> struct _xtools__is_integral_st                     : public _xtools__false_type {};
//template <>          struct _xtools__is_integral_st<bool>               : public _xtools__true_type {};
template <>          struct _xtools__is_integral_st<char>               : public _xtools__true_type {};
template <>          struct _xtools__is_integral_st<signed char>        : public _xtools__true_type {};
template <>          struct _xtools__is_integral_st<unsigned char>      : public _xtools__true_type {};
//template <>          struct _xtools__is_integral_st<wchar_t>            : public _xtools__true_type {};
template <>          struct _xtools__is_integral_st<short>              : public _xtools__true_type {};
template <>          struct _xtools__is_integral_st<unsigned short>     : public _xtools__true_type {};
template <>          struct _xtools__is_integral_st<int>                : public _xtools__true_type {};
template <>          struct _xtools__is_integral_st<unsigned int>       : public _xtools__true_type {};
template <>          struct _xtools__is_integral_st<long>               : public _xtools__true_type {};
template <>          struct _xtools__is_integral_st<unsigned long>      : public _xtools__true_type {};
template <>          struct _xtools__is_integral_st<long long>          : public _xtools__true_type {};
template <>          struct _xtools__is_integral_st<unsigned long long> : public _xtools__true_type {};


// is_char
// Looks like I don't need to remove reference.
// This :   template<typename O, enable_if(is_char(O))>
//          void strcpy(O otherChar)
//   is still enabled if parameter is a char&

template <class _Tp>  struct _xtools__is_char_st                        : public _xtools__false_type {};
template <>           struct _xtools__is_char_st<char>                  : public _xtools__true_type {};
template <>           struct _xtools__is_char_st<signed char>           : public _xtools__true_type {};
template <>           struct _xtools__is_char_st<unsigned char>         : public _xtools__true_type {};
template <>           struct _xtools__is_char_st<char16_t>              : public _xtools__true_type {};
template <>           struct _xtools__is_char_st<char32_t>              : public _xtools__true_type {};
template <>           struct _xtools__is_char_st<wchar_t>               : public _xtools__true_type {};

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
//template<class _Ty>
//	struct remove_const
//	{	// remove const
//		typedef _Ty type;
//	};
//
//template<class _Ty>
//	struct remove_const<const _Ty>
//	{	// remove const
//		typedef _Ty type;
//	};


template< class T > struct _xtools__remove_ref                { typedef T type; };
template< class T > struct _xtools__remove_ref<T&>            { typedef T type; };

template< class T > struct _xtools__remove_const                { typedef T type; };
template< class T > struct _xtools__remove_const<const T>       { typedef T type; };
template< class T > struct _xtools__remove_const<const T&>       { typedef T& type; };
template< class T > struct _xtools__remove_const<const T*>       { typedef T* type; };

template< class T > struct _xtools__remove_const_ptr                { typedef T type; };
template< class T > struct _xtools__remove_const_ptr<const T>       { typedef T type; };
template< class T > struct _xtools__remove_const_ptr<T*>       { typedef T type; };
template< class T > struct _xtools__remove_const_ptr<const T*>       { typedef T type; };
template< class T, int n > struct _xtools__remove_const_ptr<T[n]>       { typedef T type; };
template< class T, int n > struct _xtools__remove_const_ptr<const T[n]>       { typedef T type; };


// is_char_ptr
template <class _Tp>  struct _xtools__is_char_ptr_st                        : public _xtools__false_type {};
template <>           struct _xtools__is_char_ptr_st<char*>                  : public _xtools__true_type {};
template <>           struct _xtools__is_char_ptr_st<char[]>                : public _xtools__true_type {};
template <size_t _Np> struct _xtools__is_char_ptr_st<char[_Np]>             : public _xtools__true_type {};
template <>           struct _xtools__is_char_ptr_st<signed char*>           : public _xtools__true_type {};
template <>           struct _xtools__is_char_ptr_st<signed char[]>                : public _xtools__true_type {};
template <size_t _Np> struct _xtools__is_char_ptr_st<signed char[_Np]>             : public _xtools__true_type {};
template <>           struct _xtools__is_char_ptr_st<unsigned char*>         : public _xtools__true_type {};
template <>           struct _xtools__is_char_ptr_st<unsigned char[]>                : public _xtools__true_type {};
template <size_t _Np> struct _xtools__is_char_ptr_st<unsigned char[_Np]>             : public _xtools__true_type {};
template <>           struct _xtools__is_char_ptr_st<char16_t*>              : public _xtools__true_type {};
template <>           struct _xtools__is_char_ptr_st<char16_t[]>            : public _xtools__true_type {};
template <size_t _Np> struct _xtools__is_char_ptr_st<char16_t[_Np]>         : public _xtools__true_type {};
template <>           struct _xtools__is_char_ptr_st<char32_t*>              : public _xtools__true_type {};
template <>           struct _xtools__is_char_ptr_st<char32_t[]>            : public _xtools__true_type {};
template <size_t _Np> struct _xtools__is_char_ptr_st<char32_t[_Np]>         : public _xtools__true_type {};
template <>           struct _xtools__is_char_ptr_st<wchar_t*>               : public _xtools__true_type {};
template <>           struct _xtools__is_char_ptr_st<wchar_t[]>             : public _xtools__true_type {};
template <size_t _Np> struct _xtools__is_char_ptr_st<wchar_t[_Np]>          : public _xtools__true_type {};


/* enable_if */
template <bool, typename T = void>
struct _xtools_enable_if_t
{};

template <typename T>
struct _xtools_enable_if_t<true, T> {
  typedef T type;
};



template< class... >
using _xtools__void_t = void;

/* has type member */
// primary template handles types that have no nested ::type member:
template< class, class = _xtools__void_t<> >
struct _xtools__has_type_member : _xtools__false_type { };

// specialization recognizes types that do have a nested ::type member:
template< class T >
struct _xtools__has_type_member<T, _xtools__void_t<typename T::char_t>> : _xtools__true_type { };




#define remove_const(x) typename _xtools__remove_const<x>::type
#define unsigned_type(x) typename _xtools__make_unsigned<remove_const(x)>::type
#define remove_ref(x) typename _xtools__remove_ref<x>::type
#define remove_const_ptr(x) typename _xtools__remove_const_ptr<x>::type

#define is_integral(x) _xtools__is_integral_st<remove_const(x)>::value
#define is_char(x) _xtools__is_char_st<remove_const(x)>::value
#define is_char_ptr(x) _xtools__is_char_ptr_st<typename _xtools__remove_const<x>::type>::value
#define enable_if_t(x) typename _xtools_enable_if_t<x>::type
#define enable_if(x) typename enable_if_type = typename _xtools_enable_if_t<x>::type

#endif // __cplusplus

#endif /* XToolsCommon_h */
