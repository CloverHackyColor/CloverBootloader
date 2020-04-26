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


#define XStringArraySuper XObjArray<XStringClass>

template<class XStringClass_>
class XStringArray_/* : public XStringArraySuper*/
{
  protected:
	XObjArray<XStringClass_> array;
	
  public:
	typedef XStringClass_ XStringClass;

	XStringArray_() {};

	size_t size() const { return array.size(); }
	void setEmpty() { array.Empty(); }
	bool isEmpty() const { return this->size() == 0 ; }
	bool notEmpty() const { return this->size() > 0 ; }
	
//	#define enable_if XStringAbstract__enable_if_t
	/* [] */
	template<typename IntegralType, enable_if(is_integral(IntegralType))>
	const XStringClass& operator [](IntegralType i) const { return array[i]; }
	/* ElementAt */
	template<typename IntegralType, enable_if(is_integral(IntegralType))>
	const XStringClass& elementAt(IntegralType i) const { return array[i]; }
	template<typename IntegralType, enable_if(is_integral(IntegralType))>
	XStringClass& elementAt(IntegralType i) { return array[i]; }


//	const XStringClass& dbg(size_t i) const { return array[i]; }


	XStringClass ConcatAll(const XStringClass& Separator = ", "_XS, const XStringClass& Prefix = NullXString, const XStringClass& Suffix = NullXString) const
	{
		xsize i;
		XString s;
		
		if ( array.size() > 0 ) {
			s = Prefix;
			s += array.ElementAt(0);
			for ( i=1 ; i<array.size() ; i+=1 ) {
				s += Separator;
				s += array.ElementAt(i);
			}
			s += Suffix;
		}
		return s;
	}


	template<class OtherXStringArrayClass>
	bool Equal(const OtherXStringArrayClass &aStrings) const
	{
		xsize ui;
		
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
		xsize i;
		
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

	void Add(const XStringClass &aString) { array.AddCopy(aString); }
	
	void AddReference(XStringClass *newElement, bool FreeIt) { array.AddReference(newElement, FreeIt); }
	template<class OtherXStringClass>
	void import(const XStringArray_<OtherXStringClass> &aStrings)
	{
		xsize i;
		
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
		xsize i;
		
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

class XStringArray : public XStringArray_<XString>
{
};
extern const XStringArray NullXStringArray;

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
//	return Split<XStringArrayClass>(S, ", "_XS);
//};



template<class XStringArrayClass, typename CharType1, typename CharType2, enable_if(is_char(CharType1) && is_char(CharType2))>
XStringArrayClass Split(const CharType1* S, const CharType2* Separator)
{
  XStringArrayClass xsArray;

	size_t separatorLength = length_of_utf_string(Separator);
	
	if ( separatorLength == 0 ) {
		typename XStringArrayClass::XStringClass* xstr;
		xstr = new typename XStringArrayClass::XStringClass;
		xstr->takeValueFrom(S);
		xsArray.AddReference(xstr, true);
		return xsArray;
	}

	const CharType1* s = S;
	char32_t char32 = 1;
	
	do
	{
		while ( XStringAbstract__ncompare(s, Separator, separatorLength, false) == 0 ) {
			// I have to implement a move_forward_one_char in unicode_conversions, as we don't care about char32
			for ( size_t i = 0 ; i < separatorLength ; i++ ) s = get_char32_from_string(s, &char32);
		}
		const CharType1* t = s;
		size_t nb = 0;
		while ( char32 && XStringAbstract__ncompare(t, Separator, separatorLength, false) != 0 ) {
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

template<class XStringArrayClass, class XStringClass1, class XStringClass2, enable_if(!is_char(XStringClass1) && !is_char_ptr(XStringClass1) && !is_char(XStringClass2))>
XStringArrayClass Split(const XStringClass1& S, const XStringClass2& Separator)
{
	return Split<XStringArrayClass>(S.s(), Separator.s());
//
//  XStringArrayClass Ss;
//  size_t idxB, idxE;
//
//	if ( Separator.length() == 0 ) {
//		Ss.Add(S);
//		return Ss;
//	}
//	idxB = 0;
//	idxE = S.indexOf(Separator, idxB);
//	while ( idxE != MAX_XSIZE ) {
//		Ss.Add(S.subString(idxB, idxE-idxB));
//		idxB = idxE + Separator.length();
//		idxE = S.indexOf(Separator, idxB);
//	}
//	if ( idxB < S.length() ) Ss.Add(S.subString(idxB, S.length()-idxB));
//	return Ss;
};


template<class XStringArrayClass, class XStringClass1, enable_if(!is_char(XStringClass1) && !is_char_ptr(XStringClass1))>
XStringArrayClass Split(const XStringClass1& S)
{
	return Split<XStringArrayClass>(S, ", "_XS);
};

#endif
