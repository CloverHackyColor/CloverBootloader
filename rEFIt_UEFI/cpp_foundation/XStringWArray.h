//*************************************************************************************************
//*************************************************************************************************
//
//                                          STRINGS
//
//*************************************************************************************************
//*************************************************************************************************

#if !defined(__XSTRINGWARRAY_H__)
#define __XSTRINGWARRAY_H__

#include "XToolsCommon.h"
#include "XObjArray.h"
#include "XString.h"


#define XStringWArraySuper XObjArray<XStringW>
class XStringWArray : public XStringWArraySuper
{

  public:

	XStringWArray();


	void SetNull() { Empty(); }
	bool IsNull() const { return size() == 0 ; }
	bool NotNull() const { return size() > 0 ; }

	XStringW ConcatAll(const XStringW& Separator = L", "_XSW, const XStringW& Prefix = NullXStringW, const XStringW& Suffix = NullXStringW) const;

	bool Equal(const XStringWArray &aStrings) const;
	bool operator ==(const XStringWArray &aXStrings) const { return Equal(aXStrings); }
	bool operator !=(const XStringWArray& aXStrings) const { return !Equal(aXStrings); }
	bool Contains(const XStringW &S) const;
	bool Same(const XStringWArray &aStrings) const;

    // Add
    void AddStrings(const wchar_t *Val1, ...);

	void AddNoNull(const XStringW &aString) { if ( !aString.isEmpty() ) AddCopy(aString); }
	void AddEvenNull(const XStringW &aString) { AddCopy(aString); }

	void Add(const XStringW &aString) { AddCopy(aString); }
	void Add(const XStringWArray &aStrings);

	void AddID(const XStringW &aString); /* ignore Duplicate */
	void AddID(const XStringWArray &aStrings); /* ignore Duplicate */
};

extern const XStringWArray NullXStringws;
XStringWArray Split(const XStringW &S, const XStringW &Separator = L", "_XSW);

#endif
