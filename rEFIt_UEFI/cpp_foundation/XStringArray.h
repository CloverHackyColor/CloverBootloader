//*************************************************************************************************
//*************************************************************************************************
//
//                                          STRINGS
//
//*************************************************************************************************
//*************************************************************************************************

#if !defined(__XSTRINGARRAY_H__)
#define __XSTRINGARRAY_H__

#include "XToolsCommon.h"
#include "XStringWArray.h"
#include "XString.h"


#define XStringArraySuper XObjArray<XString>
class XStringArray : public XStringArraySuper
{

  public:

	XStringArray();


	void SetNull() { Empty(); }
	bool IsNull() const { return size() == 0 ; }
	bool NotNull() const { return size() > 0 ; }

	XString ConcatAll(const XString& Separator = ", "_XS, const XString& Prefix = NullXString, const XString& Suffix = NullXString) const;

	bool Equal(const XStringArray &aStrings) const;
	bool operator ==(const XStringArray &aXStrings) const { return Equal(aXStrings); }
	bool operator !=(const XStringArray& aXStrings) const { return !Equal(aXStrings); }
	bool Contains(const XString &S) const;
	bool Same(const XStringArray &aStrings) const;

    // Add
    void AddStrings(const wchar_t *Val1, ...);

	void AddNoNull(const XString &aString) { if ( !aString.isEmpty() ) AddCopy(aString); }
	void AddEvenNull(const XString &aString) { AddCopy(aString); }

	void Add(const XString &aString) { AddCopy(aString); }
	void Add(const XStringArray &aStrings);

	void AddID(const XString &aString); /* ignore Duplicate */
	void AddID(const XStringArray &aStrings); /* ignore Duplicate */
};

extern const XStringArray NullXStringArray;
XStringArray Split(const XString &S, const XString &Separator = ", "_XS);

#endif
