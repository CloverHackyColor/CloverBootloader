//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//
//                                           STRINGS
//
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

#include "XToolsCommon.h"
#include "XStringArray.h"


const XStringArray NullXStringArray;


XStringArray::XStringArray() : XStringArraySuper()
{
}

//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------
void XStringArray::AddStrings(const wchar_t *Val1, ...)
{
  va_list va;
  const wchar_t *p;

	{
		XString* newS = new XString;
		newS->takeValueFrom(Val1);
		AddReference(newS, true);
	}
	va_start(va, Val1);
	p = VA_ARG(va, const wchar_t *);
	while ( p != nullptr ) {
		XString* newS = new XString;
		newS->takeValueFrom(Val1);
		AddReference(newS, true);
		p = VA_ARG(va, const wchar_t *);
	}
	va_end(va);
}

XString XStringArray::ConcatAll(const XString& Separator, const XString& Prefix, const XString& Suffix) const
{
  xsize i;
  XString s;

	if ( size() > 0 ) {
		s = Prefix;
		s += ElementAt(0);
		for ( i=1 ; i<size() ; i+=1 ) {
			s += Separator;
			s += ElementAt(i);
		}
		s += Suffix;
	}
	return s;
}

bool XStringArray::Equal(const XStringArray &aStrings) const
{
  xsize ui;

	if ( size() != aStrings.size() ) return false;
	for ( ui=0 ; ui<size() ; ui+=1 ) {
		if ( ElementAt(ui) != aStrings[ui] ) return false;
	}
	return true;
}

bool XStringArray::Same(const XStringArray &aStrings) const
{
  xsize i;

	for ( i=0 ; i<size() ; i+=1 ) {
		if ( !aStrings.Contains(ElementAt(i)) ) return false;
	}
	for ( i=0 ; i<aStrings.size() ; i+=1 ) {
		if ( !Contains(aStrings.ElementAt(i)) ) return false;
	}
	return true;
}

bool XStringArray::Contains(const XString &S) const
{
  xsize i;

	for ( i=0 ; i<size() ; i+=1 ) {
		if ( ElementAt(i) == S ) return true;
	}
	return false;
}

//-------------------------------------------------------------------------------------------------
// Add
//-------------------------------------------------------------------------------------------------
void XStringArray::Add(const XStringArray &aStrings)
{
  xsize i;

	for ( i=0 ; i<aStrings.size() ; i+=1 ) {
		AddCopy(aStrings[i]);
	}
}

void XStringArray::AddID(const XString &aString)
{
	if ( !Contains(aString) ) AddCopy(aString);
}

void XStringArray::AddID(const XStringArray &aStrings)
{
  xsize i;

	for ( i=0 ; i<aStrings.size() ; i+=1 ) {
		if ( !Contains(aStrings[i]) ) AddCopy(aStrings[i]);
	}
}


//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
// Divers
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

XStringArray Split(const XString &S, const XString &Separator)
{
  XStringArray Ss;
  xsize idxB, idxE;

	idxB = 0;
	idxE = S.IdxOf(Separator, idxB);
	while ( idxE != MAX_XSIZE ) {
		Ss.AddCopy(S.SubString(idxB, idxE-idxB));
		idxB = idxE + Separator.length();
		idxE = S.IdxOf(Separator, idxB);
	}
	if ( idxB < S.length() ) Ss.AddCopy(S.SubString(idxB, S.length()-idxB));
	return Ss;
}

