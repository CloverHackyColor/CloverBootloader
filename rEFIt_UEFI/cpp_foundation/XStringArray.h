//*************************************************************************************************
//*************************************************************************************************
//
//                                          STRINGS
//
//*************************************************************************************************
//*************************************************************************************************

#if !defined(__XSTRINGARRAY_H__)
#define __XSTRINGARRAY_H__

#include <XToolsConf.h>
#include "XToolsCommon.h"
#include "XObjArray.h"
#include "XString.h"



template<typename T, typename Tdummy=void>
struct _xstringarray__char_type;

template<typename T>
struct _xstringarray__char_type<T*, enable_if_t(is_char(T))>
{
    typedef const T* type;
    static const T* getCharPtr(T* t) { return t; }
};
//
//template<typename T>
//struct _xstringarray__char_type<const T*, enable_if_t(is_char(T))>
//{
//    typedef const T* type;
//    static const T* getCharPtr(const T* t) { return t; }
//};
//
//template<typename T>
//struct _xstringarray__char_type<const T[]>
//{
//    typedef const T* type;
//    static const T* getCharPtr(const T* t) { return t; }
//};
//
//template<typename T, size_t _Np>
//struct _xstringarray__char_type<const T[_Np]>
//{
//    typedef const T* type;
//    static const T* getCharPtr(const T* t) { return t; }
//};

template<typename T>
struct _xstringarray__char_type<T, enable_if_t(is___String(T))>
{
    typedef const char* type;
    static const typename T::char_t* getCharPtr(const T& t) { return t.s(); }
};



#define XStringArraySuper XObjArray<XStringClass>

template<class XStringClass_>
class XStringArray_/* : public XStringArraySuper*/
{
  protected:
	XObjArray<XStringClass_> array;
	
  public:
	typedef XStringClass_ XStringClass;

	XStringArray_() : array() {};

	size_t size() const { return array.size(); }
	void setEmpty() { array.Empty(); }
	bool isEmpty() const { return this->size() == 0 ; }
	bool notEmpty() const { return this->size() > 0 ; }
	
//	#define enable_if _xtools_enable_if_t
	/* [] */
	template<typename IntegralType, enable_if(is_integral(IntegralType))>
	const XStringClass& operator [](IntegralType i) const { return array[i]; }
	/* ElementAt */
	template<typename IntegralType, enable_if(is_integral(IntegralType))>
	const XStringClass& elementAt(IntegralType i) const { return array[i]; }
	template<typename IntegralType, enable_if(is_integral(IntegralType))>
	XStringClass& elementAt(IntegralType i) { return array[i]; }

  template<class Type1, class Type2, class Type3,
      enable_if(
                (  is_char(Type1) || is_char_ptr(Type1) || is___String(Type1)  ) &&
                (  is_char(Type2) || is_char_ptr(Type2) || is___String(Type2)  ) &&
                (  is_char(Type3) || is_char_ptr(Type3) || is___String(Type3)  )
               )
          >
  XStringClass ConcatAll(const Type1& Separator, const Type2& Prefix, const Type3& Suffix) const
  {
      size_t i;
      XStringClass s;

      if ( array.size() > 0 ) {
          s.takeValueFrom(Prefix);
          s += array.ElementAt(0);
          for ( i=1 ; i<array.size() ; i+=1 ) {
              s += Separator;
              s += array.ElementAt(i);
          }
          s += Suffix;
      }
      return s;
  }

	XStringClass ConcatAll() const
	{
		return ConcatAll(", ", "", "");
	}

  template<class Type1, enable_if(is_char(Type1) || is_char_ptr(Type1) || is___String(Type1))>
  XStringClass ConcatAll(const Type1& Separator) const
  {
      return ConcatAll(Separator, "", "");
  }


	template<class OtherXStringArrayClass>
	bool Equal(const OtherXStringArrayClass &aStrings) const
	{
		size_t ui;
		
		if ( array.size() != aStrings.size() ) return false;
		for ( ui=0 ; ui<array.size() ; ui+=1 ) {
			if ( array.ElementAt(ui) != aStrings[ui] ) return false;
		}
		return true;
	}
	template<class OtherXStringArrayClass>
	bool operator ==(const OtherXStringArrayClass &aXStrings) const { return Equal(aXStrings); }
	template<class OtherXStringArrayClass>
	bool operator !=(const OtherXStringArrayClass& aXStrings) const { return !Equal(aXStrings); }
	
	/* contains */
	template<typename CharType, enable_if(is_char(CharType))>
	bool contains(const CharType* s) const
	{
		for ( size_t i=0 ; i<array.size() ; i+=1 ) {
			if ( array.ElementAt(i).equal(s) ) return true;
		}
		return false;
	}
	template<class OtherXStringClass, enable_if(!is_char(OtherXStringClass) && !is_char_ptr(OtherXStringClass))>
	bool contains(const OtherXStringClass &S) const
	{
		return contains(S.s());
	}
	/* containsIC */
	template<typename CharType, enable_if(is_char(CharType))>
	bool containsIC(const CharType* s) const
	{
		for ( size_t i=0 ; i<array.size() ; i+=1 ) {
			if ( array.ElementAt(i).equalIC(s) ) return true;
		}
		return false;
	}
	template<class OtherXStringClass, enable_if(!is_char(OtherXStringClass) && !is_char_ptr(OtherXStringClass))>
	bool containsIC(const OtherXStringClass &S) const
	{
		return containsIC(S.s());
	}
	/* ContainsStartWithIC */
	template<typename CharType, enable_if(is_char(CharType))>
	bool containsStartWithIC(const CharType* s) const
	{
		for ( size_t i=0 ; i<array.size() ; i+=1 ) {
			if ( array.ElementAt(i).startWithIC(s) ) return true;
		}
		return false;
	}
	template<class OtherXStringClass, enable_if(!is_char(OtherXStringClass) && !is_char_ptr(OtherXStringClass))>
	bool containsStartWithIC(const OtherXStringClass &S) const
	{
		return ContainsStartWithIC(S.s());
	}


	template<class OtherXStringArrayClass>
	bool Same(const OtherXStringArrayClass &aStrings) const
	{
		size_t i;
		
		for ( i=0 ; i<this->size() ; i+=1 ) {
			if ( !aStrings.contains(array.ElementAt(i)) ) return false;
		}
		for ( i=0 ; i<aStrings.size() ; i+=1 ) {
			if ( !contains(aStrings[i]) ) return false;
		}
		return true;
	}

    // Add
	template<typename CharType>
    void AddStrings(const CharType* Val1, ...)
	{
		va_list va;
		const wchar_t *p;
		
		{
			XStringClass* newS = new XStringClass;
			newS->takeValueFrom(Val1);
			XStringArraySuper::AddReference(newS, true);
		}
		va_start(va, Val1);
		p = VA_ARG(va, const CharType*);
		while ( p != nullptr ) {
			XStringClass* newS = new XStringClass;
			newS->takeValueFrom(Val1);
			XStringArraySuper::AddReference(newS, true);
			p = VA_ARG(va, const CharType*);
		}
		va_end(va);
	}

	void AddNoNull(const XStringClass &aString) { if ( !aString.isEmpty() ) array.AddCopy(aString); }
	void AddEvenNull(const XStringClass &aString) { array.AddCopy(aString); }

	template<typename CharType, enable_if(is_char(CharType))>
	void Add(const CharType* s)
	{
		XStringClass* xstr = new XStringClass;
		xstr->strcpy(s);
		array.AddReference(xstr, true);
	}

//    void Add(const XStringClass &aString) { array.AddCopy(aString); }
    template<typename XStringClass1, enable_if(is___String(XStringClass1))>
    void Add(const XStringClass1 &aString) { Add(aString.s()); }

	void AddReference(XStringClass *newElement, bool FreeIt) { array.AddReference(newElement, FreeIt); }
	template<class OtherXStringClass>
	void import(const XStringArray_<OtherXStringClass> &aStrings)
	{
		size_t i;
		
		for ( i=0 ; i<aStrings.size() ; i+=1 ) {
			array.AddCopy(aStrings[i]);
		}
	}

	void AddID(const XStringClass &aString) /* ignore Duplicate */
	{
		if ( !contains(aString) ) array.AddCopy(aString);
	}
	void importID(const XStringArray_ &aStrings) /* ignore Duplicate */
	{
		size_t i;
		
		for ( i=0 ; i<aStrings.size() ; i+=1 ) {
			if ( !Contains(aStrings[i]) ) AddCopy(aStrings[i]);
		}
	}
	void remove(const XStringClass &aString)
	{
		if ( array.size() > 0 )
		{
			size_t i = array.size();
			do {
				i--;
				if ( array[i] == aString ) {
					array.RemoveAtIndex(i);
					break;
				}
			} while ( i > 0 );
		}
	}
	void removeIC(const XStringClass &aString)
	{
		if ( array.size() > 0 )
		{
			size_t i = array.size();
			do {
				i--;
				if ( array[i].equalIC(aString) ) {
					array.RemoveAtIndex(i);
					break;
				}
			} while ( i > 0 );
		}
	}

};

class XString8Array : public XStringArray_<XString8>
{
};
extern const XString8Array NullXString8Array;

class XString16Array : public XStringArray_<XString16>
{
};
extern const XString16Array NullXString16Array;

class XString32Array : public XStringArray_<XString32>
{
};
extern const XString32Array NullXString32Array;

class XStringWArray : public XStringArray_<XStringW>
{
};
extern const XStringWArray NullXStringWArray;


//
//template<class XStringArrayClass, class XStringClass1, enable_if(!is_char(XStringClass1) && !is_char_ptr(XStringClass1))>
//XStringArrayClass Split(const XStringClass1& S)
//{
//	return Split<XStringArrayClass>(S, ", "_XS8);
//};




//template<class XStringArrayClass, typename CharType1, typename CharType2, enable_if(is_char(CharType1) && is_char(CharType2))>
template<class XStringArrayClass, typename Type1, typename Type2,
    enable_if(
              (  is_char_ptr(Type1) || is___String(Type1)  ) &&
              (  is_char_ptr(Type2) || is___String(Type2)  )
             )
        >
XStringArrayClass Split(Type1 S, const Type2 Separator)
{
  XStringArrayClass xsArray;

    auto s = _xstringarray__char_type<Type1>::getCharPtr(S);
    auto separator = _xstringarray__char_type<Type2>::getCharPtr(Separator);
//    typename _xstringarray__char_type<Type2>::type separator = Separator;

	size_t separatorLength = length_of_utf_string(separator);
	
	if ( separatorLength == 0 ) {
		typename XStringArrayClass::XStringClass* xstr;
		xstr = new typename XStringArrayClass::XStringClass;
		xstr->takeValueFrom(S);
		xsArray.AddReference(xstr, true);
		return xsArray;
	}

//    typename _xstringarray__char_type<Type1>::type s = S;
//	const CharType1* s = S;
	char32_t char32 = 1;
	
	do
	{
		while ( XStringAbstract__ncompare(s, separator, separatorLength, false) == 0 ) {
			// I have to implement a move_forward_one_char in unicode_conversions, as we don't care about char32
			for ( size_t i = 0 ; i < separatorLength ; i++ ) s = get_char32_from_string(s, &char32);
		}
        if ( !*s ) return xsArray;
        auto t = s;
//        typename _xstringarray__char_type<Type1>::type t = s;
//		const CharType1* t = s;
		size_t nb = 0;
		while ( char32 && XStringAbstract__ncompare(t, separator, separatorLength, false) != 0 ) {
			nb++;
			t = get_char32_from_string(t, &char32);
		}
		typename XStringArrayClass::XStringClass* xstr;
		xstr = new typename XStringArrayClass::XStringClass;
		xstr->strncpy(s, nb);
		xsArray.AddReference(xstr, true);
//		s = get_char32_from_string(t, &char32);
		s = t;
		// Consume the separator we found
		for ( size_t i = 0 ; i < separatorLength ; i++ ) s = get_char32_from_string(s, &char32);
	} while ( char32 );
	
	return xsArray;
//
//
//	// TODO : Allocating temporary strings could be avoided by using low level function from unicode_conversions
//	typename XStringArrayClass::XStringClass SS;
//	SS.takeValueFrom(S);
//	typename XStringArrayClass::XStringClass XSeparator;
//	SS.takeValueFrom(Separator);
//	return Split<XStringArrayClass>(SS, XSeparator);
};


template<class XStringArrayClass, class Type1, enable_if( is_char_ptr(Type1)  ||  is___String(Type1) ) >
XStringArrayClass Split(Type1 S)
{
    return Split<XStringArrayClass>(S, ", ");
};





//
//template<class XStringArrayClass, class XStringClass1, class XStringClass2, enable_if(is___String(XStringClass1) && is___String(XStringClass2))>
//XStringArrayClass Split(const XStringClass1& S, const XStringClass2& Separator)
//{
//	return Split<XStringArrayClass>(S.s(), Separator.s());
////
////  XStringArrayClass Ss;
////  size_t idxB, idxE;
////
////	if ( Separator.length() == 0 ) {
////		Ss.Add(S);
////		return Ss;
////	}
////	idxB = 0;
////	idxE = S.indexOf(Separator, idxB);
////	while ( idxE != MAX_XSIZE ) {
////		Ss.Add(S.subString(idxB, idxE-idxB));
////		idxB = idxE + Separator.length();
////		idxE = S.indexOf(Separator, idxB);
////	}
////	if ( idxB < S.length() ) Ss.Add(S.subString(idxB, S.length()-idxB));
////	return Ss;
//};
//
//
//template<class XStringArrayClass, class XStringClass1, enable_if(!is_char(XStringClass1) && !is_char_ptr(XStringClass1))>
//XStringArrayClass Split(const XStringClass1& S)
//{
//	return Split<XStringArrayClass>(S, ", "_XS8);
//};

#endif
