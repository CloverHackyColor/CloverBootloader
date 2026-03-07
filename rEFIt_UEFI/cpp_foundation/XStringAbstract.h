/*
 *
 * Created by jief in 1997.
 * Copyright (c) 2020 Jief
 * All rights reserved.
 *
 */

#if !defined(__XSTRINGABSTRACT_H__)
#define __XSTRINGABSTRACT_H__

#include "XToolsCommon.h"
#include "unicode_conversions.h"
#include <XToolsConf.h>

#ifndef DEBUG_ALL
#define DEBUG_XStringAbstract 0
#else
#define DEBUG_XStringAbstract DEBUG_ALL
#endif

#if DEBUG_XStringAbstract == 0
#define DBG_XSTRING(...)
#else
#define DBG_XSTRING(...) DebugLog(DEBUG_XStringAbstract, __VA_ARGS__)
#endif

// #define XSTRING_CACHING_OF_SIZE

#define asciiToLower(ch)                                                       \
  (((ch >= L'A') && (ch <= L'Z')) ? ((ch - L'A') + L'a') : ch)
#define asciiToUpper(ch)                                                       \
  (((ch >= L'a') && (ch <= L'z')) ? ((ch - L'a') + L'A') : ch)

template <typename S, typename O>
int XStringAbstract__startWith(const S *src, const O *other, bool ignoreCase) {
  //	size_t nb = 0;
  const S *src2 = src;
  const O *other2 = other;

  char32_t src_char32;
  char32_t other_char32;
  other2 = get_char32_from_string(other2, &other_char32);
  if (!other_char32)
    return true; // startWith with empty string is considered true
  src2 = get_char32_from_string(src2, &src_char32);
  while (other_char32) {
    if (ignoreCase) {
      src_char32 = asciiToLower(src_char32);
      other_char32 = asciiToLower(other_char32);
    }
    if (src_char32 != other_char32)
      return false;
    src2 = get_char32_from_string(src2, &src_char32);
    other2 = get_char32_from_string(other2, &other_char32);
    //		nb += 1;
  };
  return src_char32 != 0;
}

template <typename S, typename O>
int XStringAbstract__startWithOrEqualTo(const S *src, const O *other,
                                        bool ignoreCase) {
  //  size_t nb = 0;
  const S *src2 = src;
  const O *other2 = other;

  char32_t src_char32;
  char32_t other_char32;
  other2 = get_char32_from_string(other2, &other_char32);
  if (!other_char32)
    return true; // startWith with empty string is considered true
  src2 = get_char32_from_string(src2, &src_char32);
  while (other_char32) {
    if (ignoreCase) {
      src_char32 = asciiToLower(src_char32);
      other_char32 = asciiToLower(other_char32);
    }
    if (src_char32 != other_char32)
      return false;
    src2 = get_char32_from_string(src2, &src_char32);
    other2 = get_char32_from_string(other2, &other_char32);
    //    nb += 1;
  };
  return true;
}
/*
 * Returns 1 if src > other
 */
template <typename S, typename O>
int XStringAbstract__compare(const S *src, const O *other, bool ignoreCase) {
  if (src == NULL || *src == 0)
    return other == NULL || *other == 0 ? 0 : -1;
  if (other == NULL || *other == 0)
    return 1;
  //	size_t len_s = length_of_utf_string(src);
  //	size_t len_other = length_of_utf_string(other);
  //	size_t nb = 0;
  const S *src2 = src;
  const O *other2 = other;

  char32_t src_char32;
  char32_t other_char32;
  src2 = get_char32_from_string(src2, &src_char32);
  other2 = get_char32_from_string(other2, &other_char32);
  while (src_char32) {
    if (ignoreCase) {
      src_char32 = asciiToLower(src_char32);
      other_char32 = asciiToLower(other_char32);
    }
    if (src_char32 != other_char32)
      break;
    src2 = get_char32_from_string(src2, &src_char32);
    other2 = get_char32_from_string(other2, &other_char32);
    //		nb += 1;
  };
  if (src_char32 == other_char32)
    return 0;
  return src_char32 > other_char32 ? 1 : -1;
}

template <typename S, typename O>
int XStringAbstract__ncompare(const S *src, const O *other, size_t n,
                              bool ignoreCase) {
  if (n == 0)
    return 0; // string of 0 length are equal.
  const S *src2 = src;
  const O *other2 = other;

  char32_t src_char32;
  char32_t other_char32;
  src2 = get_char32_from_string(src2, &src_char32);
  other2 = get_char32_from_string(other2, &other_char32);
  size_t nb = 1;
  while (src_char32 && nb < n) {
    if (ignoreCase) {
      src_char32 = asciiToLower(src_char32);
      other_char32 = asciiToLower(other_char32);
    }
    if (src_char32 != other_char32)
      break;
    src2 = get_char32_from_string(src2, &src_char32);
    other2 = get_char32_from_string(other2, &other_char32);
    nb += 1;
  };
  if (src_char32 == other_char32)
    return 0;
  return src_char32 > other_char32 ? 1 : -1;
}

template <typename O, typename P>
size_t XStringAbstract__indexOf(const O **s, const P *other, size_t offsetRet,
                                bool toLower) {
  size_t Idx = 0;

  char32_t s_char32;
  char32_t other_char32;

  do {
    const O *s2 = *s;
    const P *other2 = other;
    do {
      s2 = get_char32_from_string(s2, &s_char32);
      other2 = get_char32_from_string(other2, &other_char32);
      if (toLower) {
        s_char32 = asciiToLower(s_char32);
        other_char32 = asciiToLower(other_char32);
      }
    } while (s_char32 && other_char32 && s_char32 == other_char32);
    if (other_char32 == 0)
      return Idx + offsetRet;
    *s = get_char32_from_string(*s, &s_char32);
    Idx++;
  } while (s_char32);
  return MAX_XSIZE;
}

template <typename O, typename P>
size_t XStringAbstract__indexOf(const O *s, size_t Pos, const P *other,
                                bool toLower) {
  if (*other == 0)
    return Pos;

  char32_t char32 = 1;
  for (size_t Idx = 0; Idx < Pos; Idx += 1) {
    s = get_char32_from_string(s, &char32);
  }
  if (!char32)
    return MAX_XSIZE;
  return XStringAbstract__indexOf(&s, other, Pos, toLower);
}

/*
 * Find the last occurence of other, until pos
 * NOTE : do not pass SIZE_T_MAX as pos. maximum value is SIZE_T_MAX-1
 */
template <typename O, typename P>
size_t XStringAbstract__rindexOf(const O *s, size_t Pos, const P *other,
                                 bool toLower) {
  if (*other == 0)
    return Pos > length_of_utf_string(s) ? length_of_utf_string(s) : Pos;

  size_t index = XStringAbstract__indexOf(&s, other, 0, toLower);
  size_t prev_index =
      index; // initialize to index in case of index is already == Pos

  char32_t char32;
  s = get_char32_from_string(s, &char32);
  while (char32 && index < Pos) {
    prev_index = index;
    index = XStringAbstract__indexOf(&s, other, index + 1, toLower);
    s = get_char32_from_string(s, &char32);
  };
  if (index == Pos)
    return index;
  if (prev_index <= Pos)
    return prev_index;
  return MAX_XSIZE;
}

template <class T, class ThisXStringClass, class ThisLStringClass>
class __String {
public:
  typedef T char_t;
  typedef ThisXStringClass xs_t;
  typedef ThisLStringClass ls_t;

protected:
  T *__m_data = nullptr;

protected:
#ifdef XSTRING_CACHING_OF_SIZE
  size_t __m_size;
#endif

  // convenience method. Did it this way to avoid #define in header. They can
  // have an impact on other headers
  size_t Xmin(size_t x1, size_t x2) const {
    if (x1 < x2)
      return x1;
    return x2;
  }
  size_t Xmax(size_t x1, size_t x2) const {
    if (x1 > x2)
      return x1;
    return x2;
  }

#ifdef XSTRING_CACHING_OF_SIZE
#if 1 // if 1, the size will be checked. For debug purpose.
  void checkSizeCached() const { // Debug method
    if (size_of_utf_string(__m_data) != __m_size) {
      panic("bug XString");
    }
  }
#define XSTRING_CHECK_SIZE __String<T, ThisXStringClass>::checkSizeCached()
#else
#define XSTRING_CHECK_SIZE
#endif
#else
#define XSTRING_CHECK_SIZE
#endif

  // Method _data is protected intentionally. It's a const method returning
  // non-const pointer. That's intentional, but dangerous. Do not expose to
  // public. If you need a non-const pointer for low-level access, use dataSized
  // and specify the size pos is counted in logical char (UTF32 char)
  template <typename IntegralType, enable_if(is_integral(IntegralType))>
  T *_data(IntegralType pos) const {
    XSTRING_CHECK_SIZE;
    if (pos < 0) {
      log_technical_bug("T* data(int i) -> i < 0");
      return __m_data;
    }
    size_t offset = size_of_utf_string_len(
        __m_data, (unsigned_type(IntegralType))
                      pos); // If pos is too big, size_of_utf_string_len returns
                            // the end of the string
    return __m_data + offset;
  }

public:
#ifdef XSTRING_CACHING_OF_SIZE
  constexpr __String(const T *s, size_t size)
      : __m_data((T *)s), __m_size(size) {
  } // Do NOT call with size != strlen(s). This is for litteral operator, not
    // for public usage.
#else
  constexpr __String(const T *s) : __m_data((T *)s) {}
#endif

  //	constexpr __String() : m_data(&nullChar) {  }
  constexpr __String(const __String &) = delete;
  constexpr __String() = delete;

  // no assignement, no destructor

  constexpr const T *s() const {
    XSTRING_CHECK_SIZE;
    return __m_data;
  }
  constexpr const T *data() const {
    XSTRING_CHECK_SIZE;
    return __m_data;
  }

  template <typename IntegralType, enable_if(is_integral(IntegralType))>
  constexpr const T *data(IntegralType pos) const {
    return _data(pos);
  }

  size_t length() const {
    return length_of_utf_string(data());
  } // TODO: caching length

#ifdef XSTRING_CACHING_OF_SIZE
  size_t size() const {
    XSTRING_CHECK_SIZE;
    return __m_size;
  }
  size_t sizeInNativeChars() const { return size(); }
  size_t sizeInBytes() const { return size() * sizeof(T); }
  size_t sizeInBytesIncludingTerminator() const {
    return (size() + 1) * sizeof(T);
  } // usefull for unit tests
#else
  size_t size() const {
    XSTRING_CHECK_SIZE;
    return size_of_utf_string(data());
  }
  size_t sizeInNativeChars() const { return size_of_utf_string(data()); }
  size_t sizeInBytes() const { return size_of_utf_string(data()) * sizeof(T); }
  size_t sizeInBytesIncludingTerminator() const {
    return (size_of_utf_string(data()) + 1) * sizeof(T);
  } // usefull for unit tests
#endif

  /* Empty ? */
  bool isEmpty() const { return data() == nullptr || *data() == 0; }
  bool notEmpty() const { return !isEmpty(); }

  //--------------------------------------------------------------------- cast

  //	int ToInt() const;
  //	size_t ToUInt() const;

  //---------------------------------------------------------------------
  //charAt, []

  template <typename IntegralType, enable_if(is_integral(IntegralType))>
  char32_t char32At(IntegralType i) const {
    if (i < 0) {
#ifdef JIEF_DEBUG
      panic("__String<T>::char32At(size_t i) : i < 0. System halted\n");
#else
      return 0;
#endif
    }
    size_t nb = 0;
    const T *p = data();
    char32_t char32 = 0;
    do {
      p = get_char32_from_string(p, &char32);
      if (!char32) {
#ifdef JIEF_DEBUG
        if ((unsigned_type(IntegralType))i == nb)
          return 0; // no panic if we want to access the null terminator
        panic("__String::char32At(size_t i) : i >= length(). System halted\n");
#else
        return 0;
#endif
      }
      nb += 1;
    } while (nb <= (unsigned_type(IntegralType))i);
    return char32;
  }

  template <typename IntegralType, enable_if(is_integral(IntegralType))>
  char16_t char16At(IntegralType i) const {
    char32_t char32 = char32At(i);
    if (char32 >= 0x10000)
      return 0xFFFD; // � REPLACEMENT CHARACTER used to replace an unknown,
                     // unrecognized or unrepresentable character
    return (char16_t)char32;
  }

  /* [] */
  template <typename IntegralType, enable_if(is_integral(IntegralType))>
  char32_t operator[](IntegralType i) const {
    return char32At(i);
  }

  char32_t lastChar() const {
    if (length() > 0)
      return char32At(length() - 1);
    else
      return 0;
  }
  //	/* copy ctor */
  //	__String<T, ThisXStringClass>(const __String<T, ThisXStringClass> &S)
  //{ Init(0); takeValueFrom(S); }
  //	/* ctor */
  //	template<typename O, class OtherXStringClass>
  //	explicit __String<T, ThisXStringClass>(const __String<O,
  //OtherXStringClass>& S) { Init(0); takeValueFrom(S); }
  ////	template<typename O>
  ////	explicit __String<T, ThisXStringClass>(const O* S) { Init(0);
  ///takeValueFrom(S); }

  /* Copy Assign */ // Only other XString, no litteral at the moment.
  //	__String<T, ThisXStringClass>& operator =(const __String<T,
  //ThisXStringClass>& S) { strcpy(S.s()); return *this; }
  //	/* Assign */
  //	template<typename O, class OtherXStringClass>
  //	ThisXStringClass& operator =(const __String<O, OtherXStringClass>& S)
  //{ strcpy(S.s()); return *((ThisXStringClass*)this); }
  ////	template<class O>
  ////	ThisXStringClass& operator =(const O* S)	{ strcpy(S); return
  ///*this; }

  //---------------------------------------------------------------------
  //indexOf, rindexOf, contains, subString, startWith

  /* indexOf */
  size_t indexOf(char32_t char32Searched, size_t Pos = 0) const {
    char32_t buf[2] = {char32Searched, 0};
    return XStringAbstract__indexOf(data(), Pos, buf, false);
  }
  template <typename O> size_t indexOf(const O *S, size_t Pos = 0) const {
    return XStringAbstract__indexOf(data(), Pos, S, false);
  }
  template <typename O, class OtherXStringClass>
  size_t indexOf(
      const __String<O, OtherXStringClass, typename OtherXStringClass::ls_t> &S,
      size_t Pos = 0) const {
    return indexOf(S.s(), Pos);
  }
  /* IC */
  size_t indexOfIC(char32_t char32Searched, size_t Pos = 0) const {
    char32_t buf[2] = {char32Searched, 0};
    return XStringAbstract__indexOf(data(), Pos, buf, true);
  }
  template <typename O> size_t indexOfIC(const O *S, size_t Pos = 0) const {
    return XStringAbstract__indexOf(data(), Pos, S, true);
  }
  template <typename O, class OtherXStringClass>
  size_t indexOfIC(
      const __String<O, OtherXStringClass, typename OtherXStringClass::ls_t> &S,
      size_t Pos = 0) const {
    return indexOfIC(S.s(), Pos);
  }

  /* rindexOf */
  size_t rindexOf(const char32_t char32Searched,
                  size_t Pos = MAX_XSIZE - 1) const {
    char32_t buf[2] = {char32Searched, 0};
    return XStringAbstract__rindexOf(data(), Pos, buf, false);
  }
  template <typename O>
  size_t rindexOf(const O *S, size_t Pos = MAX_XSIZE - 1) const {
    return XStringAbstract__rindexOf(data(), Pos, S, false);
  }
  template <typename O, class OtherXStringClass>
  size_t rindexOf(
      const __String<O, OtherXStringClass, typename OtherXStringClass::ls_t> &S,
      size_t Pos = MAX_XSIZE - 1) const {
    return rindexOf(S.s(), Pos);
  }
  /* IC */
  size_t rindexOfIC(const char32_t char32Searched,
                    size_t Pos = MAX_XSIZE - 1) const {
    char32_t buf[2] = {char32Searched, 0};
    return XStringAbstract__rindexOf(data(), Pos, buf, true);
  }
  template <typename O>
  size_t rindexOfIC(const O *S, size_t Pos = MAX_XSIZE - 1) const {
    return XStringAbstract__rindexOf(data(), Pos, S, true);
  }
  template <typename O, class OtherXStringClass>
  size_t rindexOfIC(
      const __String<O, OtherXStringClass, typename OtherXStringClass::ls_t> &S,
      size_t Pos = MAX_XSIZE - 1) const {
    return rindexOf(S.s(), Pos);
  }

  template <typename O, class OtherXStringClass>
  bool contains(const __String<O, OtherXStringClass,
                               typename OtherXStringClass::ls_t> &S) const {
    return indexOf(S) != MAX_XSIZE;
  }
  template <typename O> bool contains(const O *S) const {
    return indexOf(S) != MAX_XSIZE;
  }
  template <typename O, class OtherXStringClass>
  size_t containsIC(const __String<O, OtherXStringClass,
                                   typename OtherXStringClass::ls_t> &S) const {
    return indexOfIC(S) != MAX_XSIZE;
  }
  template <typename O> size_t containsIC(const O *S) const {
    return indexOfIC(S) != MAX_XSIZE;
  }

  ThisXStringClass subString(size_t pos, size_t count) const {
    size_t len = length();
    if (pos >= len || count == 0) return ThisXStringClass();
    if (count > len - pos) count = len - pos;

    ThisXStringClass ret;

    const T *src = data();
    char32_t char32 = 1;
    while (char32 && pos > 0) {
      src = get_char32_from_string(src, &char32);
      pos -= 1;
    };
    ret.strncat(src, count);
    return ret;
  }

  template <typename O, enable_if(is_char(O))>
  bool startWith(O otherChar) const {
    O other[2] = {otherChar, 0};
    return XStringAbstract__startWith(data(), other, false);
  }
  template <typename O, class OtherXStringClass>
  bool
  startWith(const __String<O, OtherXStringClass,
                           typename OtherXStringClass::ls_t> &otherS) const {
    return XStringAbstract__startWith(data(), otherS.data(), false);
  }
  template <typename O> bool startWith(const O *other) const {
    return XStringAbstract__startWith(data(), other, false);
  }
  template <typename O, class OtherXStringClass>
  bool
  startWithIC(const __String<O, OtherXStringClass,
                             typename OtherXStringClass::ls_t> &otherS) const {
    return XStringAbstract__startWith(data(), otherS.data(), true);
  }
  template <typename O> bool startWithIC(const O *other) const {
    return XStringAbstract__startWith(data(), other, true);
  }

  template <typename O, enable_if(is_char(O))>
  bool startWithOrEqualTo(O otherChar) const {
    O other[2] = {otherChar, 0};
    return XStringAbstract__startWithOrEqualTo(data(), other, false);
  }
  template <typename O, class OtherXStringClass>
  bool startWithOrEqualTo(
      const __String<O, OtherXStringClass, typename OtherXStringClass::ls_t>
          &otherS) const {
    return XStringAbstract__startWithOrEqualTo(data(), otherS.data(), false);
  }
  template <typename O> bool startWithOrEqualTo(const O *other) const {
    return XStringAbstract__startWithOrEqualTo(data(), other, false);
  }
  template <typename O, class OtherXStringClass>
  bool startWithOrEqualToIC(
      const __String<O, OtherXStringClass, typename OtherXStringClass::ls_t>
          &otherS) const {
    return XStringAbstract__startWithOrEqualTo(data(), otherS.data(), true);
  }
  template <typename O> bool startWithOrEqualToIC(const O *other) const {
    return XStringAbstract__startWithOrEqualTo(data(), other, true);
  }

  template <typename O, class OtherXStringClass>
  bool endWithOrEqualToIC(
      const __String<O, OtherXStringClass, typename OtherXStringClass::ls_t>
          &otherS) const {
    if (length() < otherS.length())
      return false;
    return XStringAbstract__rindexOf(data(), SIZE_T_MAX - 1, otherS.data(),
                                     true) == length() - otherS.length();
  }

  //---------------------------------------------------------------------

  ThisXStringClass basename() const {
    size_t lastSepPos = MAX_XSIZE;
    size_t pos = 0;
    const T *p = data();
    char32_t char32 = 0;
    p = get_char32_from_string(p, &char32);
    while (char32) {
      if (char32 == U'/' || char32 == U'\\')
        lastSepPos = pos;
      pos += 1;
      p = get_char32_from_string(p, &char32);
    };
    if (lastSepPos == MAX_XSIZE) {
      if (p == data())
        return ThisXStringClass().takeValueFrom(".");
    }
    return subString(lastSepPos + 1, MAX_XSIZE);
  }
  ThisXStringClass dirname() const {
    size_t idx;
    idx = rindexOf('\\');
    if (idx != MAX_XSIZE)
      return subString(0, idx);
    idx = rindexOf('/');
    if (idx != MAX_XSIZE)
      return subString(0, idx);
    return ThisXStringClass();
  }

  //	bool IsDigits(size_t pos, size_t count) const;
  //{
  //  const T *p;
  //  const T *q;
  //
  //	if ( pos >= size() ) {
  //		return false;
  //	}
  //	if ( pos+count > size() ) {
  //		return false;
  //	}
  //	p = data() + pos;
  //	q = p + count;
  //	for ( ; p < q ; p+=1 ) {
  //		if ( *p < '0' ) return false;
  //		if ( *p > '9' ) return false;
  //	}
  //	return true;
  //}

  //---------------------------------------------------------------------
  //strcmp, equal, comparison operator

  template <typename O> int strcmp(const O *S) const {
    return XStringAbstract__compare(data(), S, false);
  }
  //	int Compare(const char* S) const { return ::Compare<T, char>(data(), S);
  //} 	int Compare(const char16_t* S) const { return ::Compare<T,
  //char16_t>(data(), S); }; 	int Compare(const char32_t* S) const { return
  //::Compare<T, char32_t>(data(), S); }; 	int Compare(const wchar_t* S) const {
  //return ::Compare<T, wchar_t>(data(), S); };
  //

  template <typename O> int strncmp(const O *S, size_t n) const {
    return XStringAbstract__ncompare(data(), S, n, false);
  }

  template <typename O, class OtherXStringClass>
  bool isEqual(const __String<O, OtherXStringClass,
                              typename OtherXStringClass::ls_t> &S) const {
    return XStringAbstract__compare(data(), S.s(), false) == 0;
  }
  template <typename O> bool isEqual(const O *S) const {
    return XStringAbstract__compare(data(), S, false) == 0;
  }

  template <typename O, class OtherXStringClass>
  bool isEqualIC(const __String<O, OtherXStringClass,
                                typename OtherXStringClass::ls_t> &S) const {
    return XStringAbstract__compare(data(), S.s(), true) == 0;
  }
  template <typename O> bool isEqualIC(const O *S) const {
    return XStringAbstract__compare(data(), S, true) == 0;
  }

  //	bool SubStringEqual(size_t Pos, const T* S) const { return
  //(memcmp(data(Pos), S, wcslen(S)) == 0); }

  template <typename IntegralType, typename O, class OtherXStringClass>
  bool isEqualAtIC(IntegralType pos,
                   const __String<O, OtherXStringClass,
                                  typename OtherXStringClass::ls_t> &S) const {

#ifdef JIEF_DEBUG
    if (pos < 0)
      panic("XString::equalAtIC -> i < 0");
#else
    if (pos < 0)
      return false;
#endif

    if ((unsigned_type(IntegralType))pos > length() - S.length())
      return false;
    return XStringAbstract__ncompare(data() + (unsigned_type(IntegralType))pos,
                                     S.s(), S.length(), true) == 0;
  }

public:
  // == operator
  template <typename O, class OtherXStringClass>
  bool operator==(const __String<O, OtherXStringClass,
                                 typename OtherXStringClass::ls_t> &s2) const {
    return (*this).strcmp(s2.s()) == 0;
  }
  //	template<typename O>
  //	bool operator == (const O* s2) const { return (*this).strcmp(s2) == 0; }
  //	template<typename O>
  //	friend bool operator == (const O* s1, ThisXStringClass& s2) { return
  //s2.strcmp(s1) == 0; }

  template <typename O, class OtherXStringClass>
  bool operator!=(const __String<O, OtherXStringClass,
                                 typename OtherXStringClass::ls_t> &s2) const {
    return !(*this == s2);
  }
  //	template<typename O>
  //	bool operator != (const O* s2) const { return !(*this == s2); }
  //	template<typename O>
  //	friend bool operator != (const O* s1, const ThisXStringClass& s2) {
  //return s2.strcmp(s1) != 0; }

  template <typename O, class OtherXStringClass>
  bool operator<(const __String<O, OtherXStringClass,
                                typename OtherXStringClass::ls_t> &s2) const {
    return (*this).strcmp(s2.s()) < 0;
  }
  //	template<typename O>
  //	bool operator <  (const O* s2) const { return (*this).strcmp(s2) < 0; }
  //	template<typename O>
  //	friend bool operator <  (const O* s1, const ThisXStringClass& s2) {
  //return s2.strcmp(s1) > 0; }

  template <typename O, class OtherXStringClass>
  bool operator>(const __String<O, OtherXStringClass,
                                typename OtherXStringClass::ls_t> &s2) const {
    return (*this).strcmp(s2.s()) > 0;
  }
  //	template<typename O>
  //	bool operator >  (const O* s2) const { return  (*this).strcmp(s2) > 0; }
  //	template<typename O>
  //	friend bool operator >  (const O* s1, const ThisXStringClass& s2) {
  //return s2.strcmp(s1) < 0; }

  template <typename O, class OtherXStringClass>
  bool operator<=(const __String<O, OtherXStringClass,
                                 typename OtherXStringClass::ls_t> &s2) const {
    return (*this).strcmp(s2.s()) <= 0;
  }
  //	template<typename O>
  //	bool operator <= (const O* s2) const { return  (*this).strcmp(s2) <= 0;
  //} 	template<typename O> 	friend bool operator <= (const O* s1, const
  //ThisXStringClass& s2) { return s2.strcmp(s1) >= 0; }

  template <typename O, class OtherXStringClass>
  bool operator>=(const __String<O, OtherXStringClass,
                                 typename OtherXStringClass::ls_t> &s2) const {
    return (*this).strcmp(s2.s()) >= 0;
  }
  //	template<typename O>
  //	bool operator >= (const O* s2) const { return  (*this).strcmp(s2) >= 0;
  //} 	template<typename O> 	friend bool operator >= (const O* s1, const
  //ThisXStringClass& s2) { return s2.strcmp(s1) <= 0; }
};

// xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx  LString
// xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

template <class T, class ThisXStringClass, class ThisLStringClass>
class LString : public __String<T, ThisXStringClass, ThisLStringClass> {
public:
protected:
#ifdef XSTRING_CACHING_OF_SIZE
  constexpr LString(const T *s) : __String<T, ThisXStringClass>(s, 0) {};
  constexpr LString(const T *s, size_t size)
      : __String<T, ThisXStringClass>(s, size) {};
  constexpr LString(const LString &L)
      : __String<T, ThisXStringClass>(L.data(), L.size()) {};
#else
  constexpr LString(const T *s)
      : __String<T, ThisXStringClass, ThisLStringClass>(s) {};
  constexpr LString(const T *s, size_t size)
      : __String<T, ThisXStringClass, ThisLStringClass>(s, size) {};
  constexpr LString(const LString &L)
      : __String<T, ThisXStringClass, ThisLStringClass>(L.data()) {};
#endif

  constexpr LString() = delete;

  // no assignement, no destructor
};

// xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

///* __String + char32_t */
// template<typename CharType1, class XStringClass1>
// XStringClass1 operator + (const __String<CharType1, XStringClass1>& p1,
// char32_t p2) { XStringClass1 s; s.takeValueFrom(p1); s.strcat(p2); return s;
// }
//
///* __String + __String */
// template<typename CharType1, class XStringClass1, typename CharType2, class
// XStringClass2> XStringClass1 operator + (const __String<CharType1,
// XStringClass1>& p1, const __String<CharType2, XStringClass2>& p2) {
// XStringClass1 s; s.takeValueFrom(p1); s.strcat(p2); return s; }
//
///* char* + __String */
// template<typename CharType1, typename CharType2, class XStringClass2>
// XStringClass2 operator + (const CharType1* p1, const __String<CharType2,
// XStringClass2>& p2) { XStringClass2 s; s.takeValueFrom(p1); s.strcat(p2);
// return s; }
//
///* __String + char* */
// template<typename T1, class XStringClass1, typename CharType2>
// XStringClass1 operator + (const __String<T1, XStringClass1>& p1, const
// CharType2* p2) { XStringClass1 s; s.takeValueFrom(p1); s.strcat(p2); return
// s; }

template <typename Base> _xtools__true_type is_base_of_test_func(Base *);
template <typename Base> _xtools__false_type is_base_of_test_func(void *);
template <typename B, typename D>
auto test_pre_is_base_of(int)
    -> decltype(is_base_of_test_func<B>(static_cast<D *>(nullptr)));

template <class, class = _xtools__void_t<>, class = _xtools__void_t<>,
          class = _xtools__void_t<>>
struct __string_type {
  typedef void type;
};
template <typename T>
// struct __string_type<T, _xtools__void_t<typename T::xs_t>,
// _xtools__void_t<typename T::char_t>> { typedef __String<typename T::char_t,
// typename T::xs_t> type; }; struct __string_type<T, _xtools__void_t<typename
// T::char_t>, _xtools__void_t<typename T::xs_t>, _xtools__void_t<typename
// T::ls_t>> { typedef __String<typename T::char_t, typename T::xs_t, typename
// T::ls_t> type; };
struct __string_type<T, _xtools__void_t<typename T::char_t>> {
  typedef __String<typename T::char_t, typename T::xs_t, typename T::ls_t> type;
};

#define is___String_t(x)                                                       \
  decltype(test_pre_is_base_of<typename __string_type<x>::type, x>(0))
#define is___String(x) is___String_t(x)::value

// template< class, class = _xtools__void_t<>, class = _xtools__void_t<> >
// struct __lstring_type { typedef void type; };
// template< typename T >
// struct __lstring_type<T, _xtools__void_t<typename T::xs_t>,
// _xtools__void_t<typename T::char_t>> { typedef LString<typename T::char_t,
// typename T::xs_t> type; };
template <class, class = _xtools__void_t<>, class = _xtools__void_t<>,
          class = _xtools__void_t<>>
struct __lstring_type {
  typedef void type;
};
template <typename T>
struct __lstring_type<T, _xtools__void_t<typename T::xs_t>,
                      _xtools__void_t<typename T::char_t>,
                      _xtools__void_t<typename T::ls_t>> {
  typedef LString<typename T::char_t, typename T::xs_t, typename T::ls_t> type;
};
// struct __lstring_type<T, _xtools__void_t<typename T::char_t>,
// _xtools__void_t<typename T::xs_t>, xtools__void_t<typename T::ls_t>> {
// typedef LString<typename T::char_t, typename T::xs_t, typename T::ls_t> type;
// };

#define is___LString_t(x)                                                      \
  decltype(test_pre_is_base_of<typename __lstring_type<x>::type, x>(0))
#define is___LString(x) is___LString_t(x)::value

/* __string_class_or<T1, T2>::type is T1 is T1 is a subclass of __String. If T1
 * is not a subclass of __String, returns T2 if it's a subclass of __String */
template <typename T1, typename T2, typename Tdummy = void>
struct __string_class_or;
template <typename T1, typename T2>
struct __string_class_or<T1, T2,
                         enable_if_t(!is___String(T1) &&
                                     !is___String(T2))> { /*typedef double
                                                             type;*/
};
template <typename T1, typename T2>
struct __string_class_or<T1, T2, enable_if_t(is___String(T1))> {
  typedef typename T1::xs_t type;
};
template <typename T1, typename T2>
struct __string_class_or<T1, T2,
                         enable_if_t(!is___String(T1) && is___String(T2))> {
  typedef typename T2::xs_t type;
};

/* ------------  get_char_ptr(x) --------------*/
template <typename T, typename Tdummy = void> struct _xstringarray__char_type;

template <typename T>
struct _xstringarray__char_type<T, enable_if_t(is___String(T))> {
  static const typename T::char_t *getCharPtr(const T &t) { return t.s(); }
};

template <typename T>
struct _xstringarray__char_type<T *, enable_if_t(is_char(T))> {
  static const T *getCharPtr(T *t) { return t; }
};

// template<typename T>
// struct _xstringarray__char_type<const T*, enable_if_t(is_char(T))>
//{
//     static const T* getCharPtr(const T* t) { return t; }
// };

template <typename T> struct _xstringarray__char_type<const T[]> {
  static const T *getCharPtr(T *t) { return t; }
};

template <typename T, size_t _Np> struct _xstringarray__char_type<T[_Np]> {
  static const T *getCharPtr(const T *t) { return t; }
};

#ifdef _MSC_VER
// I don't know why it's needed with VS.

template <typename T>
struct _xstringarray__char_type<T, enable_if_t(is___LString(T))> {
  static const typename T::char_t *getCharPtr(const T &t) { return t.s(); }
};

#endif

#define get_char_ptr(x) _xstringarray__char_type<typeof(x)>::getCharPtr(x)

// xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

// #define data() super::data()

template <class T, class ThisXStringClass, class ThisLStringClass>
class XStringAbstract : public __String<T, ThisXStringClass, ThisLStringClass> {
public:
  using super = __String<T, ThisXStringClass, ThisLStringClass>;
  using ls_t = ThisLStringClass;

protected:
  static T nullChar;
  size_t m_allocatedSize; // Must include null terminator. Real memory allocated
                          // is only m_allocatedSize (not m_allocatedSize+1)

  // xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
  //  Init , Alloc
  // xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

  /*
   * nNewSize must include null terminator.
   */
  void Alloc(size_t nNewAllocatedSize) {
    if (m_allocatedSize == 0) {
      super::__m_data = (T *)AllocatePool(nNewAllocatedSize * sizeof(T));  //Slice: was malloc
    } else {
//      super::__m_data =
//          (T *)Xrealloc(super::__m_data, nNewAllocatedSize * sizeof(T),
//                        m_allocatedSize * sizeof(T));
		super::__m_data = (T*)ReallocatePool( m_allocatedSize * sizeof(T),
			 nNewAllocatedSize * sizeof(T), super::__m_data);
    }
    if (!super::__m_data) {
      log_technical_bug("XStringAbstract::Alloc(%zu) : Xrealloc(%" PRIuPTR
                        ", %lu, %zd) returned NULL. System halted\n",
                        nNewAllocatedSize, uintptr_t(super::__m_data),
                        nNewAllocatedSize * sizeof(T),
                        m_allocatedSize * sizeof(T));
      m_allocatedSize = 0;
      return;
    }
    m_allocatedSize = nNewAllocatedSize;
  }

  //  public:
  /*
   * Make sure this string has allocated size of at least nNewSize+1.
   */
  bool CheckSize(size_t nNewAllocatedSize,
                 size_t nGrowBy = XStringGrowByDefault) // nNewSize is in number
                                                        // of chars, NOT bytes
  {
    // DBG_XSTRING("CheckSize: m_size=%d, nNewSize=%d\n", m_size, nNewSize);
    if (m_allocatedSize < nNewAllocatedSize + 1) {
      nNewAllocatedSize += nGrowBy;
      if (m_allocatedSize == 0) { // if ( *data() ) {
        // Even if m_allocatedSize == 0, data() might not be NULL because it can
        // points to a litteral. So we need to alloc and cpy the litteral in the
        // newly allocated buffer.
        size_t size = super::size();
        if (nNewAllocatedSize < size)
          nNewAllocatedSize = size;
        const T *m_dataSav = super::data();
        // super::__m_data = NULL; // No need, Alloc will reassign it.
        Alloc(nNewAllocatedSize + 1);
        utf_string_from_utf_string(super::__m_data, m_allocatedSize, m_dataSav);
        return true;
      } else {
        Alloc(nNewAllocatedSize + 1);
        return true;
      }
    }
    return false;
  }

public:
  /* default ctor */
#ifdef XSTRING_CACHING_OF_SIZE
  XStringAbstract()
      : __String<T, ThisXStringClass, ThisLStringClass>(&nullChar, 0),
        m_allocatedSize(0) {}
#else
  XStringAbstract()
      : __String<T, ThisXStringClass, ThisLStringClass>(&nullChar),
        m_allocatedSize(0) {}
#endif

  /* copy ctor */
#ifdef XSTRING_CACHING_OF_SIZE
  XStringAbstract(const XStringAbstract &S)
      : __String<T, ThisXStringClass, ThisLStringClass>(&nullChar, 0),
        m_allocatedSize(0)
#else
  XStringAbstract(const XStringAbstract &S)
      : __String<T, ThisXStringClass, ThisLStringClass>(&nullChar),
        m_allocatedSize(0)
#endif
  {
    *this = S;
  }

  ~XStringAbstract() {
    // DBG_XSTRING("Destructor :%ls\n", data());
    if (m_allocatedSize > 0)
      delete[] super::__m_data;
  }

#ifdef XSTRING_CACHING_OF_SIZE
  /* ctor */
  template <class OtherLStringClass>
  explicit XStringAbstract(const LString<T, OtherLStringClass> &S)
      : __String<T, ThisXStringClass>(S.s(), S.size()), m_allocatedSize(0) {}

  template <typename O, class OtherXStringClass>
  explicit XStringAbstract<T, ThisXStringClass>(
      const XStringAbstract<O, OtherXStringClass> &S)
      : __String<T, ThisXStringClass>(&nullChar, 0), m_allocatedSize(0) {
    takeValueFrom(S);
  }
  template <typename O, class OtherXStringClass>
  explicit XStringAbstract<T, ThisXStringClass>(
      const LString<O, OtherXStringClass> &S)
      : __String<T, ThisXStringClass>(&nullChar), m_allocatedSize(0) {
    takeValueFrom(S);
  }
// TEMPORARILY DISABLED
//	template<typename O>
//	explicit __String<T, ThisXStringClass>(const O* S) { Init(0);
//takeValueFrom(S); }
//
#else
  /* ctor */
  template <class OtherLStringClass>
  explicit XStringAbstract(
      const LString<T, OtherLStringClass, typename OtherLStringClass::ls_t> &S)
      : __String<T, ThisXStringClass, typename ThisXStringClass::ls_t>(S.s()),
        m_allocatedSize(0) {}

  template <typename O, class OtherXStringClass>
  // changed for gcc-14
  //	explicit XStringAbstract<T, ThisXStringClass, ThisLStringClass>(const
  //XStringAbstract<O, OtherXStringClass, typename OtherXStringClass::ls_t>& S)
  //: __String<T, ThisXStringClass, ThisLStringClass>(&nullChar),
  //m_allocatedSize(0) { takeValueFrom(S); } 	template<typename O, class
  //OtherXStringClass> 	explicit XStringAbstract<T, ThisXStringClass,
  //ThisLStringClass>(const LString<O, OtherXStringClass, typename
  //OtherXStringClass::ls_t>& S) : __String<T, ThisXStringClass, typename
  //ThisXStringClass::ls_t>(&nullChar), m_allocatedSize(0) { takeValueFrom(S); }
  explicit XStringAbstract(
      const XStringAbstract<O, OtherXStringClass,
                            typename OtherXStringClass::ls_t> &S)
      : __String<T, ThisXStringClass, ThisLStringClass>(&nullChar),
        m_allocatedSize(0) {
    takeValueFrom(S);
  }
  template <typename O, class OtherXStringClass>
  explicit XStringAbstract(
      const LString<O, OtherXStringClass, typename OtherXStringClass::ls_t> &S)
      : __String<T, ThisXStringClass, typename ThisXStringClass::ls_t>(
            &nullChar),
        m_allocatedSize(0) {
    takeValueFrom(S);
  }

//  TEMPORARILY DISABLED
//	template<typename O>
//	explicit __String<T, ThisXStringClass>(const O* S) { Init(0);
//takeValueFrom(S); }
//
#endif
  /* Copy Assign */
  XStringAbstract &operator=(const XStringAbstract &S) {
    if (this == &S) {
      return *this;
    }
    if (S.data() && S.m_allocatedSize == 0) {
      // S points to a litteral
      if (m_allocatedSize > 0) {
        delete[] super::__m_data;
        m_allocatedSize = 0;
      }
      super::__m_data =
          (T *)S.data(); // because it's a litteral, we don't copy. We need to
                         // cast, but we won't modify.
#ifdef XSTRING_CACHING_OF_SIZE
      super::__m_size = S.size();
#endif
    } else {
      strsicpy(S.s(), S.size());
    }
    return *this;
  }

  /* Copy Assign */
  XStringAbstract &operator=(const ls_t &S) {
    if (m_allocatedSize > 0) {
      delete[] super::__m_data;
      m_allocatedSize = 0;
    }
    super::__m_data = (T *)S.data(); // because it's a litteral, we don't copy.
                                     // We need to cast, but we won't modify.
#ifdef XSTRING_CACHING_OF_SIZE
    super::__m_size = S.size();
#endif
    return *this;
  }

/* Assign */
#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#endif
  template <typename O, class OtherXStringClass>
  ThisXStringClass &
  operator=(const __String<O, OtherXStringClass,
                           typename OtherXStringClass::ls_t> &S) {
    strcpy(S.s());
    return *((ThisXStringClass *)this);
  }
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif

  // TEMPORARILY DISABLED
  //	template<class O>
  //	ThisXStringClass& operator =(const O* S)	{ strcpy(S); return
  //*this; }

protected:
  //	ThisXStringClass& takeValueFromLiteral(const T* s)
  //	{
  //		if ( m_allocatedSize > 0 ) {
  //      panic_ask("XStringAbstract::takeValueFromLiteral -> m_allocatedSize >
  //      0");
  //    }
  //		super::__m_data = (T*)s;
  // #ifdef XSTRING_CACHING_OF_SIZE
  //  super::__m_size = strlen(s);
  // #endif
  //		return *((ThisXStringClass*)this);
  //	}

public:
  size_t allocatedSize() const { return m_allocatedSize; }

  void setEmpty() {
    if (m_allocatedSize <= 0)
      super::__m_data = &nullChar;
    else
      super::__m_data[0] = 0;
#ifdef XSTRING_CACHING_OF_SIZE
    super::__m_size = 0;
#endif
  }

  //  T* data() { return data(); }
  const T *data() const { return super::data(); }

  template <typename IntegralType, enable_if(is_integral(IntegralType))>
  const T *data(IntegralType pos) const {
    return super::_data(pos);
  }
  //  template<typename IntegralType, enable_if(is_integral(IntegralType))>
  //  T* data(IntegralType pos) { return super::_data(pos); }

  template <typename IntegralType, enable_if(is_integral(IntegralType))>
  T *dataSized(IntegralType size) {
    if (size < 0) {
      log_technical_bug("T* dataSized() -> i < 0");
      return NULL;
    }
    if ((unsigned_type(IntegralType))size > MAX_XSIZE) {
      log_technical_bug("T* dataSized() -> i > MAX_XSIZE");
      return NULL;
    }
    CheckSize((unsigned_type(IntegralType))size, 0);
    return super::_data(0);
  }
  void updateSize() {
#ifdef XSTRING_CACHING_OF_SIZE
    super::__m_size = 0; // Jief, TODO
#endif
  }
  //
  //	// Pos is counted in logical char but size is counted in physical char
  //(char, char16_t, char32_t or wchar_t) 	template<typename IntegralType1,
  //typename IntegralType2, enable_if(is_integral(IntegralType1) &&
  //is_integral(IntegralType2))> 	T* dataSized(IntegralType1 pos, IntegralType2
  //size)
  //	{
  //		if ( pos<0 ) panic("T* dataSized(xisize i, size_t sizeMin,
  //size_t nGrowBy) -> i < 0"); 		if ( size<0 ) panic("T* dataSized(xisize i,
  //size_t sizeMin, size_t nGrowBy) -> i < 0");
  // 		size_t offset = size_of_utf_string_len(data(), (typename
  // _xtools__make_unsigned<IntegralType1>::type)pos); // If pos is too big,
  // size_of_utf_string_len returns the end of the string
  //		CheckSize(offset + (typename
  //_xtools__make_unsigned<IntegralType2>::type)size); 		return _data(pos);
  //	}

  T *forgetDataWithoutFreeing() {
    if (m_allocatedSize == 0) {
      // this is a litteral. We have to copy it before returning. If not, it'll
      // crash when the user will free the pointer returned
      if (super::size() == 0)
        return NULL;
      CheckSize(super::size());
    }
    T *ret = super::__m_data;
    super::__m_data = &nullChar;
#ifdef XSTRING_CACHING_OF_SIZE
    super::__m_size = 0;
#endif
    m_allocatedSize = 0;
    return ret;
  }

  //---------------------------------------------------------------------
  //strcat, strcpy, operator =

  /* strcpy char */
  template <typename O, enable_if(is_char(O))> void strcpy(O otherChar) {
    if (otherChar != 0) {
      size_t newSize = utf_size_of_utf_string_len(data(), &otherChar, 1);
      CheckSize(newSize, 0);
      utf_string_from_utf_string_len(super::__m_data, m_allocatedSize,
                                     &otherChar, 1);
      super::__m_data[newSize] = 0;
#ifdef XSTRING_CACHING_OF_SIZE
      super::__m_size = newSize;
#endif
    } else {
      setEmpty();
    }
  }
  /* strcpy */
  template <typename O> void strcpy(const O *other) {
    if (other && *other) {
      size_t newSize = utf_size_of_utf_string(data(), other);
      CheckSize(newSize, 0);
      utf_string_from_utf_string(super::__m_data, m_allocatedSize, other);
      super::__m_data[newSize] = 0;
#ifdef XSTRING_CACHING_OF_SIZE
      super::__m_size = newSize;
      XSTRING_CHECK_SIZE;
#endif
    } else {
      setEmpty();
    }
  }
  /* strncpy */
  template <typename O> void strncpy(const O *other, size_t other_len) {
    if (other && *other && other_len > 0) {
      size_t newSize = utf_size_of_utf_string_len(data(), other, other_len);
      CheckSize(newSize, 0);
      utf_string_from_utf_string_len(super::__m_data, m_allocatedSize, other,
                                     other_len);
      super::__m_data[newSize] = 0;
#ifdef XSTRING_CACHING_OF_SIZE
      super::__m_size = newSize;
      XSTRING_CHECK_SIZE;
#endif
    } else {
      setEmpty();
    }
  }
  template <typename O> void strsicpy(const O *other, size_t other_size) {
    if (other && *other && other_size > 0) {
      CheckSize(other_size, 0);
      utf_string_from_utf_string_size(
          super::__m_data, m_allocatedSize, other,
          other_size); // TODO:utf_string_from_utf_string_SIZE
      super::__m_data[other_size] = 0;
#ifdef XSTRING_CACHING_OF_SIZE
      super::__m_size = other_size;
#endif
    } else {
      setEmpty();
    }
  }

  /* strcat char */
  template <typename O, enable_if(is_char(O))> void strcat(O otherChar) {
    if (otherChar) {
      size_t currentSize = super::size();
      size_t newSize =
          currentSize + utf_size_of_utf_string_len(data(), &otherChar, 1);
      CheckSize(newSize);
      utf_string_from_utf_string_len(super::__m_data + currentSize,
                                     m_allocatedSize, &otherChar, 1);
      super::__m_data[newSize] = 0;
#ifdef XSTRING_CACHING_OF_SIZE
      super::__m_size = newSize;
#endif
    } else {
      // nothing to do
    }
  }
  /* strcat char* */
  template <typename O> void strcat(const O *other) {
    if (other && *other) {
      size_t currentSize = super::size(); // size is number of T, not in bytes
      size_t newSize =
          currentSize + utf_size_of_utf_string(
                            data(), other); // size is number of T, not in bytes
      CheckSize(newSize);
      utf_string_from_utf_string(super::__m_data + currentSize,
                                 m_allocatedSize - currentSize, other);
      super::__m_data[newSize] = 0;
#ifdef XSTRING_CACHING_OF_SIZE
      super::__m_size = newSize;
#endif
    } else {
      // nothing to do
    }
  }
  /* strcat __String */
  template <typename OtherCharType, class OtherXStringClass>
  void strcat(const __String<OtherCharType, OtherXStringClass,
                             typename OtherXStringClass::ls_t> &other) {
    size_t currentSize = super::size(); // size is number of T, not in bytes
    size_t newSize =
        currentSize +
        utf_size_of_utf_string(data(),
                               other.s()); // size is number of T, not in bytes
    CheckSize(newSize);
    utf_string_from_utf_string(super::__m_data + currentSize,
                               m_allocatedSize - currentSize, other.s());
    super::__m_data[newSize] = 0;
#ifdef XSTRING_CACHING_OF_SIZE
    super::__m_size = newSize;
    XSTRING_CHECK_SIZE;
#endif
  }
  /* strncat */
  template <typename O> void strncat(const O *other, size_t other_len) {
    if (other && *other && other_len > 0) {
      size_t currentSize = super::size();
      size_t other_size = utf_size_of_utf_string_len(data(), other, other_len);
      size_t newSize = currentSize + other_size;
      CheckSize(newSize);
      utf_string_from_utf_string_len(super::__m_data + currentSize,
                                     m_allocatedSize, other, other_len);
      super::__m_data[newSize] = 0;
#ifdef XSTRING_CACHING_OF_SIZE
      super::__m_size = newSize;
      XSTRING_CHECK_SIZE;
#endif
    } else {
      // nothing to do
    }
  }
  /* strsicat */
  template <typename O> void strsicat(const O *other, size_t other_size) {
    if (other && *other && other_size > 0) {
      size_t currentSize = super::size();
      size_t newSize = currentSize + other_size;
      CheckSize(newSize);
      utf_string_from_utf_string_size(super::__m_data + currentSize,
                                      m_allocatedSize, other, other_size);
      super::__m_data[newSize] = 0;
#ifdef XSTRING_CACHING_OF_SIZE
      super::__m_size = newSize;
      XSTRING_CHECK_SIZE;
#endif
    } else {
      // nothing to do
    }
  }

  /* insert char* */
  template <typename O>
  ThisXStringClass &insertAtPos(const O *other, size_t other_len, size_t pos) {
    if (!other || !*other)
      return *((ThisXStringClass *)this);

    size_t currentLength = super::length();
    if (pos >= currentLength) {
      strncat(other, other_len);
      return *((ThisXStringClass *)this);
    }

    size_t currentSize = super::size();
    size_t otherSize = utf_size_of_utf_string_len(data(), other, other_len);
    CheckSize(currentSize + otherSize);
    size_t start = size_of_utf_string_len(
        data(), pos); // size is number of T, not in bytes
    memmove(super::__m_data + start + otherSize, super::__m_data + start,
            (currentSize - start + 1) *
                sizeof(T)); // memmove handles overlapping memory move
    utf_stringnn_from_utf_string(super::__m_data + start, otherSize, other);
    //    data()[newSize] = 0;
#ifdef XSTRING_CACHING_OF_SIZE
    super::__m_size = currentSize + otherSize;
    XSTRING_CHECK_SIZE;
#endif
    return *((ThisXStringClass *)this);
  }

  /* insert char* */
  template <typename O>
  ThisXStringClass &insertAtPos(const O *other, size_t pos) {
    if (!other || !*other)
      return *((ThisXStringClass *)this);

    size_t currentLength = super::length();
    if (pos >= currentLength) {
      strcat(other);
      return *((ThisXStringClass *)this);
    }

    size_t currentSize = super::size();
    size_t otherSize = utf_size_of_utf_string(data(), other);
    CheckSize(currentSize + otherSize);
    size_t start = size_of_utf_string_len(
        data(), pos); // size is number of T, not in bytes
    memmove(data() + start + otherSize, data() + start,
            (currentSize - start + 1) *
                sizeof(T)); // memmove handles overlapping memory move
    utf_stringnn_from_utf_string(data() + start, otherSize, other);
    //    data()[newSize] = 0;
#ifdef XSTRING_CACHING_OF_SIZE
    super::__m_size = currentSize + otherSize;
    XSTRING_CHECK_SIZE;
#endif
    return *((ThisXStringClass *)this);
  }
  /* insert char */
  template <typename O, enable_if(is_char(O))>
  void insertAtPos(O otherChar, size_t pos) {
    insertAtPos(&otherChar, 1, pos);
  }

  ThisXStringClass &deleteCharsAtPos(size_t pos, size_t count = 1) {
    size_t currentLength = super::length();
    if (pos >= currentLength)
      return *((ThisXStringClass *)this);

    size_t currentSize = super::size(); // size is number of T, not in bytes
    CheckSize(currentSize, 0); // Although we only delete, we have to CheckSize
                               // in case this string point to a litteral.

    size_t start = size_of_utf_string_len(
        data(), pos); // size is number of T, not in bytes

    //    if ( pos+count >= currentLength ) count = currentLength - pos;
    if (pos + count >= currentLength) {
      super::__m_data[start] = 0;
#ifdef XSTRING_CACHING_OF_SIZE
      super::__m_size = start;
#endif
    } else {
      size_t end = start + size_of_utf_string_len(
                               data() + start,
                               count); // size is number of T, not in bytes
      memmove(super::__m_data + start, super::__m_data + end,
              (currentSize - end + 1) *
                  sizeof(T)); // memmove handles overlapping memory move
#ifdef XSTRING_CACHING_OF_SIZE
      super::__m_size -= end - start;
#endif
    }
// Handle length change when implementing caching length feature.
#ifdef XSTRING_CACHING_OF_SIZE
    XSTRING_CHECK_SIZE;
#endif
    return *((ThisXStringClass *)this);
  }

  ThisXStringClass &replaceAll(char32_t charToSearch,
                               char32_t charToReplaceBy) {
    size_t currentSize =
        super::sizeInNativeChars(); // size is number of T, not in bytes
    size_t charToSearchSize = utf_size_of_utf_string_len(
        data(), &charToSearch, 1); // size is number of T, not in bytes
    size_t charToReplaceBySize = utf_size_of_utf_string_len(
        data(), &charToReplaceBy, 1); // size is number of T, not in bytes
    // careful because 'charToReplaceBySize - charToSearchSize' overflows when
    // charToSearchSize > charToReplaceBySize, which happens.

    char32_t char32;
    T *previousData = super::__m_data;
    T *previousP = super::__m_data;
    T *p = get_char32_from_string(previousP, &char32);
    while (char32) {
      if (!char32)
        break;
      if (char32 == charToSearch) {
        if (CheckSize(currentSize + charToReplaceBySize - charToSearchSize)) {
          previousP =
              super::__m_data +
              (previousP - previousData); // CheckSize have reallocated. Correct
                                          // previousP, p and previousData.
          p = super::__m_data + (p - previousData);
          previousData = super::__m_data;
        }
        memmove(p + charToReplaceBySize - charToSearchSize, p,
                uintptr_t(super::__m_data + currentSize - p + 1) * sizeof(T));
        p += charToReplaceBySize;
        p -= charToSearchSize;
        currentSize += charToReplaceBySize;
        currentSize -= charToSearchSize;
        utf_stringnn_from_utf_string(previousP, charToReplaceBySize,
                                     &charToReplaceBy);
      }
      previousP = p;
      p = get_char32_from_string(previousP, &char32);
    }
#ifdef XSTRING_CACHING_OF_SIZE
    super::__m_size = currentSize;
    XSTRING_CHECK_SIZE;
#endif
    return *((ThisXStringClass *)this);
  }

  template <typename OtherXStringClass1, class OtherXStringClass2,
            enable_if(is___String(OtherXStringClass1) &&
                      is___String(OtherXStringClass2))>
  ThisXStringClass &replaceAll(const OtherXStringClass1 &search,
                               const OtherXStringClass2 &replaceBy) {
    size_t currentSize =
        super::sizeInNativeChars(); // size is number of T, not in bytes
    size_t sizeLeft = currentSize;  // size is number of T, not in bytes
    size_t searchSize = utf_size_of_utf_string(
        data(), search.s()); // size is number of T, not in bytes
    size_t replaceBySize = utf_size_of_utf_string(
        data(), replaceBy.s()); // size is number of T, not in bytes
    // careful because 'charToReplaceBySize - charToSearchSize' overflows when
    // charToSearchSize > charToReplaceBySize, which happens.

    //    size_t pos = super::indexOf(search);
    T *previousData = super::__m_data;
    T *previousP = super::__m_data;
    T *p = super::__m_data;
    size_t pos = XStringAbstract__indexOf((const T **)&p, search.s(), 0, false);
    while (pos != MAX_XSIZE) {
      if (CheckSize(currentSize + replaceBySize - searchSize)) {
        previousP = super::__m_data + (previousP - previousData);
        p = super::__m_data + (p - previousData);
        previousData = super::__m_data;
      }
      sizeLeft -= uintptr_t(p - previousP);
      memmove(p + replaceBySize, p + searchSize,
              (sizeLeft - searchSize + 1) * sizeof(T));
      //      memmove(data()+pos+replaceBySize-searchSize, data()+pos,
      //      (currentSize - pos + 1)*sizeof(T));
      utf_stringnn_from_utf_string(p, replaceBySize, replaceBy.s());
      p += replaceBySize;
      currentSize += replaceBySize;
      currentSize -= searchSize;
      sizeLeft -= searchSize;
      // sizeLeft is equal to utf_size_of_utf_string(p, p);
      previousP = p;
      pos = XStringAbstract__indexOf((const T **)&p, search.s(), 0, false);
    }
#ifdef XSTRING_CACHING_OF_SIZE
    super::__m_size = currentSize;
    XSTRING_CHECK_SIZE;
#endif
    return *((ThisXStringClass *)this);
  }

  void trim() {
    size_t lengthInNativeBytes = super::sizeInNativeChars();
    if (lengthInNativeBytes == 0)
      return;
    T *start = 0;
    T *s = super::__m_data;
    while (*s && unsigned_type(T)(*s) <= 32)
      s++;
    if (!*s) {
      super::__m_data[0] = 0;
#ifdef XSTRING_CACHING_OF_SIZE
      super::__m_size = 0;
      XSTRING_CHECK_SIZE;
#endif
      return;
    }
    start = s;
    s = super::__m_data + lengthInNativeBytes - 1;
    while (*s && unsigned_type(T)(*s) <= 32)
      s--;
    size_t newSize = uintptr_t(s - start) + 1;
    CheckSize(
        newSize,
        0); // We have to CheckSize in case this string point to a litteral.
    memmove(super::__m_data, start, newSize * sizeof(T));
    super::__m_data[newSize] = 0;
#ifdef XSTRING_CACHING_OF_SIZE
    super::__m_size = newSize;
    XSTRING_CHECK_SIZE;
#endif
  }

  void lowerAscii() {
    size_t currentSize = super::size(); // size is number of T, not in bytes
    CheckSize(
        currentSize,
        0); // We have to CheckSize in case this string point to a litteral.
    T *s = super::__m_data;
    while (*s) {
      *s = asciiToLower(*s);
      s++;
    }
  }

  void upperAscii() {
    size_t currentSize = super::size(); // size is number of T, not in bytes
    CheckSize(
        currentSize,
        0); // We have to CheckSize in case this string point to a litteral.
    T *s = super::__m_data;
    while (*s) {
      *s = asciiToUpper(*s);
      s++;
    }
  }

  /* size is in number of technical chars, NOT in bytes */
  ThisXStringClass &stealValueFrom(T *S, size_t allocatedSize) {
    if (m_allocatedSize > 0)
      delete[] super::__m_data;
    super::__m_data = S;
#ifdef XSTRING_CACHING_OF_SIZE
    super::__m_size = utf_size_of_utf_string(super::__m_data, super::__m_data);
#endif
    m_allocatedSize = allocatedSize;
    return *((ThisXStringClass *)this);
  }

  // Not sure we should keep that. We cannot know the allocated size. Therefore,
  // a future realloc may fail as EDK want the old size.
  ThisXStringClass &stealValueFrom(T *S) {
    if (m_allocatedSize > 0)
      delete[] super::__m_data;
    super::__m_data = S;
#ifdef XSTRING_CACHING_OF_SIZE
    super::__m_size = utf_size_of_utf_string(super::__m_data, super::__m_data);
    m_allocatedSize = super::__m_size + 1;
#else
    m_allocatedSize = super::size() + 1;
#endif
    return *((ThisXStringClass *)this);
  }

  ThisXStringClass &stealValueFrom(ThisXStringClass *S) {
    if (m_allocatedSize > 0)
      delete[] super::__m_data;
#ifdef XSTRING_CACHING_OF_SIZE
    super::__m_size = S->size();
#endif
    m_allocatedSize = S->m_allocatedSize;
    // do not use forgetDataWithoutFreeing() : it will allocate m_data if m_data
    // points to a literal. We want to keep the literal and avoid an allocation
    super::__m_data = S->super::__m_data;
    return *((ThisXStringClass *)this);
  }

  /* takeValueFrom */
  template <typename O, class OtherXStringClass>
  ThisXStringClass &
  takeValueFrom(const __String<O, OtherXStringClass,
                               typename OtherXStringClass::ls_t> &S) {
    *this = S;
    return *((ThisXStringClass *)this);
  }
  template <typename O> ThisXStringClass &takeValueFrom(const O *S) {
    strcpy(S);
    return *((ThisXStringClass *)this);
  }
  template <typename O, enable_if(is_char(O))>
  ThisXStringClass &takeValueFrom(const O C) {
    strcpy(C);
    return *((ThisXStringClass *)this);
  }
  //	template<typename O, class OtherXStringClass>
  //	ThisXStringClass& takeValueFrom(const __String<O, OtherXStringClass>& S,
  //size_t len) { strncpy(S.s(), len); return *((ThisXStringClass*)this);
  //}
  template <typename O>
  ThisXStringClass &takeValueFrom(const O *S, size_t len) {
    strncpy(S, len);
    return *((ThisXStringClass *)this);
  }

  /* += */
  template <typename O, class OtherXStringClass>
  ThisXStringClass &
  operator+=(const __String<O, OtherXStringClass,
                            typename OtherXStringClass::ls_t> &S) {
    strcat(S.s());
    return *((ThisXStringClass *)this);
  }
  template <typename O, enable_if(is_char(O))>
  ThisXStringClass &operator+=(O S) {
    strcat(S);
    return *((ThisXStringClass *)this);
  }
  template <typename O> ThisXStringClass &operator+=(const O *S) {
    strcat(S);
    return *((ThisXStringClass *)this);
  }
};

template <class T, class ThisXStringClass, class ThisLStringClass>
T XStringAbstract<T, ThisXStringClass, ThisLStringClass>::nullChar = 0;

//------------------------------------------------------- + operator

template <typename T1, typename T2,
          enable_if(is___String(T1) || is___String(T2))>
typename __string_class_or<T1, T2>::type operator+(T1 p1, T2 p2) {
  typename __string_class_or<T1, T2>::type s;
  s.takeValueFrom(p1);
  s.strcat(p2);
  return s;
}

//-------------------------------------------------------

#undef DBG_XSTRING
#undef asciiToLower
// #undef data()

#endif // __XSTRINGABSTRACT_H__
