//*************************************************************************************************
//*************************************************************************************************
//
//                                          STRING
//
//*************************************************************************************************
//*************************************************************************************************

#if !defined(__XSTRINGW_H__)
#define __XSTRINGW_H__

#include "XToolsCommon.h"
#include "unicode_conversions.h"

#define LPATH_SEPARATOR L'\\'

extern xsize XStringWGrowByDefault;
//extern void __GLOBAL__sub_I_XStringW();

class XStringW
{
protected:
 	wchar_t *m_data;
	xsize m_len;
	xsize m_allocatedSize;

	// convenience method. Did it this way to avoid #define in header. They can have an impact on other headers
	xsize min(xsize x1, xsize x2) const { if ( x1 < x2 ) return x1; return x2; }
	xsize max(xsize x1, xsize x2) const { if ( x1 > x2 ) return x1; return x2; }

// Next 2 methods are protected intentionally. They are const method returning non-const pointer. That's intentional, but dangerous. Do not expose to public.
// It's better practice, if you need a non-const pointer for low-level access, to use dataSized and ahev to specify the size
	wchar_t* _data(unsigned int ui) const { if ( ui >= m_allocatedSize ) panic("wchar_t* data(unsigned int ui=0) -> ui >= m_allocatedSize"); return m_data+ui; }
	wchar_t* _data(int i) const { if ( i<0 ) panic("wchar_t* data(int i) -> i < 0"); if ( (unsigned int)i >= m_allocatedSize ) panic("wchar_t* data(int i) -> i >= m_allocatedSize");  return m_data+i; }
	wchar_t* _data(unsigned long ui) const { if ( ui >= m_allocatedSize ) panic("wchar_t* data(unsigned long ui=0) -> ui >= m_allocatedSize"); return m_data+ui; }
	wchar_t* _data(long i) const { if ( i<0 ) panic("wchar_t* data(long i) -> i < 0"); if ( (unsigned long)i >= m_allocatedSize ) panic("wchar_t* data(long i) -> i >= m_allocatedSize");  return m_data+i; }
	wchar_t* _data(xsize ui) const { if ( ui >= m_allocatedSize ) panic("wchar_t* data(xsize ui=0) -> ui >= m_allocatedSize"); return m_data+ui; }
	wchar_t* _data(xisize i) const { if ( i<0 ) panic("wchar_t* data(xisize i) -> i < 0"); if ( (xsize)i >= m_allocatedSize ) panic("wchar_t* data(xisize i) -> i >= m_allocatedSize");  return m_data+i; }

public:
	void Init(xsize aSize=0);
	XStringW();
	XStringW(const XStringW &aString);
//	XStringW(const wchar_t *);
//	XStringW(const wchar_t* S, xsize count);
//	XStringW(const wchar_t);
//	XStringW(const char*);

	~XStringW();

protected:
	wchar_t *CheckSize(xsize nNewSize, xsize nGrowBy = XStringWGrowByDefault);

public:
	const wchar_t* wc_str() const { return m_data; } // equivalent as std::string
	const wchar_t *data(xsize ui=0) const { return m_data+ui; } // do not multiply by sizeof(wchar_t), it's done by the compiler.
	const wchar_t *data(xisize i) const { if ( i<0 ) panic("const wchar_t *data(INTN i=0) const -> i < 0"); return m_data+i; } // do not multiply by sizeof(wchar_t), it's done by the compiler.

	wchar_t* dataSized(xsize ui, xsize sizeMin, xsize nGrowBy=XStringWGrowByDefault) { CheckSize(ui+sizeMin, nGrowBy); return _data(ui); }
	wchar_t* dataSized(xisize i, xsize sizeMin, xsize nGrowBy=XStringWGrowByDefault) { if ( i<0 ) panic("wchar_t* dataSized(xisize i, xsize sizeMin, xsize nGrowBy) -> i < 0"); CheckSize((xsize)i+sizeMin, nGrowBy); return _data(i); }
	wchar_t* forgetDataWithoutFreeing();

//	xsize length() const { return m_len; }
	xsize size() const { return m_len; }
	xsize sizeInBytes() const { return m_len*sizeof(wchar_t); }
	xsize allocatedSize() const { return m_allocatedSize; }
	void SetLength(xsize len);
	const wchar_t* s() { return m_data; }

	/* Empty ? */
	void setEmpty() { m_len = 0; }
	bool isEmpty() const { return size() == 0; }
	bool notEmpty() const { return !isEmpty(); }

	/* Cast */
//	operator const wchar_t *() const { return data(); }

	#if defined(__APPLE__) && defined(__OBJC__)
		operator NSString*() const { return [[[NSString alloc] initWithBytes:data() length:length()*sizeof(wchar_t) encoding:NSUTF32LittleEndianStringEncoding] autorelease]; }
	#endif
	
	int ToInt() const;
	xsize ToUInt() const;
	
//	XString mbs() const;

//	/* wchar_t [] */
//	wchar_t operator [](int i) const { return *_data(i); }
//	wchar_t operator [](unsigned int ui) const { return *_data(ui); }
//	wchar_t operator [](long i) const { return *_data(i); }
//	wchar_t operator [](unsigned long ui) const { return *_data(ui); }
//	wchar_t operator [](xisize i) const { return *data(i); }
//	wchar_t operator [](xsize ui) const { return *data(ui); }
//
//	/* wchar_t& [] */
//	wchar_t& operator [](int i) { return *_data(i); }
//	wchar_t& operator [](unsigned int ui) { return *_data(ui); }
//	wchar_t& operator [](long i) { return *_data(i); }
//	wchar_t& operator [](unsigned long ui) { return *_data(ui); }
//	wchar_t& operator [](xisize i) { return *_data(i); }
//	wchar_t& operator [](xsize ui) { return *_data(ui); }

	wchar_t LastChar() const { if ( size() > 0 ) return data()[size()-1]; else return 0; }
	void RemoveLastEspCtrl();

	void SetNull() { SetLength(0); };

	void StrnCpy(const wchar_t *buf, xsize len);
	void StrCpy(const wchar_t *buf);
	void StrnCat(const wchar_t *buf, xsize len);
	void StrCat(const wchar_t *buf);
	void StrCat(const XStringW &uneXStringW);
	void Delete(xsize pos, xsize count=1);

	void Insert(xsize pos, const XStringW& Str);


	void vSWPrintf(const char* format, va_list va);
  void SWPrintf(const char* format, ...) __attribute__((__format__(__printf__, 2, 3)));

	const XStringW &operator =(const XStringW &aString);
//	const XStringW &operator =(const wchar_t* S) {fdsf};
//	const XStringW &operator =(wchar_t);

	XStringW& takeValueFrom(const wchar_t* S);
	XStringW& takeValueFrom(const wchar_t* S, xsize count);
	XStringW& takeValueFrom(const char* S);

	XStringW& operator += (const XStringW &);
	XStringW& operator += (const wchar_t* S);
	XStringW& operator += (wchar_t);

	XStringW SubString(xsize pos, xsize count) const;
	xsize IdxOf(wchar_t c, xsize Pos = 0) const;
	xsize IdxOf(const XStringW& S, xsize Pos = 0) const;
	xsize RIdxOf(const wchar_t c, xsize Pos = MAX_XSIZE) const;
	xsize RIdxOf(const XStringW& S, xsize Pos = MAX_XSIZE) const;

	void ToLower(bool FirstCharIsCap = false);
	bool IsLetters() const;
	bool IsLettersNoAccent() const;
	bool IsDigits() const;
	bool IsDigits(xsize pos, xsize count) const;

	bool ExistIn(const XStringW &S) const { return IdxOf(S) != MAX_XSIZE; }
	void Replace(wchar_t c1, wchar_t c2);
	XStringW SubStringReplace(wchar_t c1, wchar_t c2);

	int Compare(const wchar_t* S) const { return (int)memcmp(data(), S, min(wcslen(S), size())*sizeof(wchar_t)); }

	bool Equal(const wchar_t* S) const { return Compare(S) == 0; };
  bool BeginingEqual(const wchar_t* S) const { return (memcmp(data(), S, wcslen(S)) == 0); }
  bool SubStringEqual(xsize Pos, const wchar_t* S) const { return (memcmp(data(Pos), S, wcslen(S)) == 0); }

	XStringW basename() const;
	XStringW dirname() const;
	
//	bool ReadFromBuf(const char *buf, xsize *idx, xsize count);
//	bool WriteToBuf(char *buf, xsize *idx, xsize count) const;
//	bool ReadFromFILE(FILE *fp);
//	bool WriteToFILE(FILE *fp) const;
	//
//	bool ReadFromXBuffer(XRBuffer &unXBuffer); // Impossible de mettre le XBuffer en const car il y a une variable d'instance de XBuffer incrémentée par ReadFromXBuffer
//	void CatToXBuffer(XBuffer *unXBuffer) const;
//	void WriteToXBuffer(XBuffer *unXBuffer, xsize *idx) const;

public:
	// + operator
	//    with XStringW
	friend XStringW operator + (const XStringW& p1, const XStringW& p2) { XStringW s; s=p1; s+=p2; return s; }
	//    with const wchar_t
	friend XStringW operator + (const XStringW& p1, const wchar_t *p2  ) { XStringW s; s=p1; s+=p2; return s; }
	friend XStringW operator + (const wchar_t *p1,   const XStringW& p2) { XStringW s; s.StrCat(p1); s.StrCat(p2); return s; }
//	//    with wchar_t
//	friend XStringW operator + (const XStringW& p1, wchar_t p2         ) { XStringW s; s=p1; s+=p2; return s; }
//	friend XStringW operator + (wchar_t p1,   const XStringW& p2       ) { XStringW s; s=p1; s+=p2; return s; }


	// == operator
	friend bool operator == (const XStringW& s1,        const XStringW& s2)      { return s1.Compare(s2._data(0)) == 0; }
	friend bool operator == (const XStringW& s1,        const wchar_t* s2  )        { return s1.Compare(s2) == 0; }
	friend bool operator == (const wchar_t* s1,            const XStringW& s2)      { return s2.Compare(s1) == 0; }

	friend bool operator != (const XStringW& s1,        const XStringW& s2)      { return s1.Compare(s2._data(0)) != 0; }
	friend bool operator != (const XStringW& s1,        const wchar_t* s2  )        { return s1.Compare(s2) != 0; }
	friend bool operator != (const wchar_t* s1,            const XStringW& s2)      { return s2.Compare(s1) != 0; }

	friend bool operator <  (const XStringW& s1, const XStringW& s2) { return s1.Compare(s2._data(0)) < 0; }
	friend bool operator <  (const XStringW& s1, const wchar_t* s2  ) { return s1.Compare(s2) < 0; }
	friend bool operator <  (const wchar_t* s1,   const XStringW& s2) { return s2.Compare(s1) > 0; }

	friend bool operator >  (const XStringW& s1, const XStringW& s2) { return s1.Compare(s2._data(0)) > 0; }
	friend bool operator >  (const XStringW& s1, const wchar_t* s2  ) { return s1.Compare(s2) > 0; }
	friend bool operator >  (const wchar_t* s1,   const XStringW& s2) { return s2.Compare(s1) < 0; }

	friend bool operator <= (const XStringW& s1, const XStringW& s2) { return s1.Compare(s2._data(0)) <= 0; }
	friend bool operator <= (const XStringW& s1, const wchar_t* s2  ) { return s1.Compare(s2) <= 0; }
	friend bool operator <= (const wchar_t* s1,   const XStringW& s2) { return s2.Compare(s1) >= 0; }

	friend bool operator >= (const XStringW& s1, const XStringW& s2) { return s1.Compare(s2._data(0)) >= 0; }
	friend bool operator >= (const XStringW& s1, const wchar_t* s2  ) { return s1.Compare(s2) >= 0; }
	friend bool operator >= (const wchar_t* s1,   const XStringW& s2) { return s2.Compare(s1) <= 0; }

};

extern const XStringW NullXStringW;

XStringW operator"" _XSW ( const wchar_t* s, size_t len);

XStringW SWPrintf(const char* format, ...) __attribute__((__format__(__printf__, 1, 2)));
XStringW SubString(const wchar_t *S, xsize pos, xsize count);
XStringW CleanCtrl(const XStringW &S);

#endif
