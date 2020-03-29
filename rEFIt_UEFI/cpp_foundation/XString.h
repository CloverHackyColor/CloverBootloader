//*************************************************************************************************
//*************************************************************************************************
//
//                                          STRING
//
//*************************************************************************************************
//*************************************************************************************************

#if !defined(__XSTRING_H__)
#define __XSTRING_H__

#include "XToolsCommon.h"

#define PATH_SEPARATOR '\\'

extern UINTN XStringGrowByDefault;


//class XConstString;
class XStringW;

class XString
{
  protected:
  	char *m_data;
  	xsize m_allocatedSize;

	// convenience method. Did it this way to avoid #define in header. They can have an impact on other headers
	xsize min(xsize x1, xsize x2) const { if ( x1 < x2 ) return x1; return x2; }
	xsize max(xsize x1, xsize x2) const { if ( x1 > x2 ) return x1; return x2; }

// Next 2 methods are protected intentionally. They are const method returning non-const pointer. That's intentional, but dangerous. Do not expose to public.
// It's better practice, if you need a non-const pointer for low-level access, to use dataSized and ahev to specify the size
	char* _data(unsigned int ui) const { if ( ui >= m_allocatedSize ) panic("char* data(unsigned int ui=0) -> ui >= m_allocatedSize"); return m_data+ui; }
	char* _data(int i) const { if ( i<0 ) panic("char* data(int i) -> i < 0"); if ( (unsigned int)i >= m_allocatedSize ) panic("char* data(int i) -> i >= m_allocatedSize");  return m_data+i; }
	char* _data(unsigned long ui) const { if ( ui >= m_allocatedSize ) panic("char* data(unsigned long ui=0) -> ui >= m_allocatedSize"); return m_data+ui; }
	char* _data(long i) const { if ( i<0 ) panic("char* data(long i) -> i < 0"); if ( (unsigned long)i >= m_allocatedSize ) panic("char* data(long i) -> i >= m_allocatedSize");  return m_data+i; }
	char* _data(xsize ui) const { if ( ui >= m_allocatedSize ) panic("char* data(xsize ui=0) -> ui >= m_allocatedSize"); return m_data+ui; }
	char* _data(xisize i) const { if ( i<0 ) panic("char* data(xisize i) -> i < 0"); if ( (xsize)i >= m_allocatedSize ) panic("char* data(xisize i) -> i >= m_allocatedSize");  return m_data+i; }

  public:

	void Init(xsize aSize=0);
	XString();
	XString(const XString &aString);
    XString(XString&& aString); // Move constructor


//	XString(const XConstString &aConstString);
//	XString(const char *S);
//	XString(const char* S, xsize count);

//	XString(const wchar_t *S);

//	XString(uchar);
//	XString(char);
//	XString(int);
//	XString(unsigned long long);

	~XString();

  public:
	char *CheckSize(xsize nNewSize, xsize nGrowBy = XStringGrowByDefault);

  public:
	const char* c_str() const { return m_data; } // same as std::string
	const char* data(xsize ui=0) const { return _data(ui); }
	const char* data(xisize i) const { return _data(i); }

	char* dataSized(xsize ui, xsize sizeMin, xsize nGrowBy=XStringGrowByDefault) { CheckSize(ui+sizeMin, nGrowBy); return _data(ui); }

	xsize length() const { return strlen(m_data); }
	xsize size() const { return strlen(m_data); }
	xsize allocatedSize() const { return m_allocatedSize; }
	void setLength(xsize len);

	/* IsNull ? */
	void setEmpty() { setLength(0); }
	bool isEmpty() const { return length() == 0; }

	/* Cast */
	operator const char *() const { return data(); }
//	operator char *() { return data(); }
	
//	int ToInt() const;
//	UINTN ToUINTN() const;

	/* char [] */
	char operator [](int i) const { return *_data(i); }
	char operator [](unsigned int ui) const { return *_data(ui); }
	char operator [](long i) const { return *_data(i); }
	char operator [](unsigned long ui) const { return *_data(ui); }
	char operator [](xisize i) const { return *data(i); }
	char operator [](xsize ui) const { return *data(ui); }

	/* char& [] */
	char& operator [](int i) { return *_data(i); }
	char& operator [](unsigned int ui) { return *_data(ui); }
	char& operator [](long i) { return *_data(i); }
	char& operator [](unsigned long ui) { return *_data(ui); }
	char& operator [](xisize i) { return *_data(i); }
	char& operator [](xsize ui) { return *_data(ui); }

	char lastChar() const { if ( length() > 0 ) return data()[length()-1]; else return 0; }
	void removeLastEspCtrl();

	void StrCpy(const char *buf);
	void StrnCpy(const char *buf, xsize len);
	void StrnCat(const char *buf, xsize len);
	void StrCat(const char *buf);
	void Delete(xsize pos, xsize count=1);

	void Insert(xsize pos, const XString& Str);

	void Cat(const XString &uneXString);

	XString& vSPrintf(const char *Format, va_list va);
	XString& SPrintf(const char *format, ...)
		#ifndef _MSC_VER
			__attribute__((format (printf, 2, 3))) // 2 and 3 because of hidden parameter 'this'.
		#endif
		;


	const XString& takeValueFrom(const char* S) { StrCpy(S); return *this; }
	const XString& takeValueFrom(const char* S, xsize count) { StrnCpy(S, count); return *this; }
	const XString& takeValueFrom(const wchar_t* S) { SPrintf("%ls", S); return *this; }

	const XString& operator =(const XString& aString);
	XString& operator =(XString&& aString);
//	const XString &operator =(const XConstString &aConstString);

// Deactivate assignment during refactoring to avoid confusion
//	const XString &operator =(const char* S);
//	const XString &operator =(char);
//	const XString &operator =(int);
//	const XString &operator =(unsigned int);
//	const XString &operator =(long);
//	const XString &operator =(unsigned long long);

	const XString &operator += (const XString &);
	const XString &operator += (const char* S);
//	const XString &operator += (const XConstString &aConstString);

// Deactivate assignment during refactoring to avoid confusion
//	const XString &operator += (char);
//	const XString &operator += (int);
//	const XString &operator += (unsigned int);
//	const XString &operator += (long);
//	const XString &operator += (unsigned long long);

	XString SubString(xsize pos, xsize count) const;
	xsize IdxOf(char c, xsize Pos = 0) const;
	xsize IdxOf(const XString &S, xsize Pos = 0) const;
#ifdef TODO_skqdjfhksqjhfksjqdf
	xsize IdxOfIC(const XString &S, xsize Pos = 0) const;
	xsize IdxOfIAC(const XString &S, xsize Pos = 0) const;
#endif
	xsize RIdxOf(const XString &S, xsize Pos = MAX_XSIZE) const;

	void ToLower(bool FirstCharIsCap = false);
	bool IsLetters() const;
	bool IsLettersNoAccent() const;
	bool IsDigits() const;
	bool IsDigits(xsize pos, xsize count) const;

	bool ExistIn(const XString &S) const { return IdxOf(S) != MAX_XSIZE; }
#ifdef TODO_skqdjfhksqjhfksjqdf
	bool ExistInIC(const XString &S) const { return IdxOfIC(S) != MAX_XSIZE; }
	bool ExistInIAC(const XString &S) const { return IdxOfIC(S) != MAX_XSIZE; }
	bool DeleteIC(const XString &S);
#endif
	void Replace(char c1, char c2);
	XString SubStringReplace(char c1, char c2);

	int Compare(const char* S) const { return strcmp(data(), (S ? S : "")); }// AsciiStrCmp return 0 or !0, not usual strcmp
#ifdef TODO_skqdjfhksqjhfksjqdf
	//IC
	int CompareIC(const char* S) const { return StringCompareIC(data(), (S ? S : "")); }
	// IA
	int CompareIA(const char* S) const { return StringCompareIA(data(), (S ? S : "")); }
	// IAC
	int CompareIAC(const char* S) const { return StringCompareIAC(data(), (S ? S : "")); }
/*
	int CompareIACSubString(const char* S, size_t LenS) const { return SubStringCompareIAC(data(), length(), S, LenS); } //SubStringCompareIC renvoi 0 (ÈgalitÈ) si Len1 ou Len2 == 0
	int CompareIACSubString(xsize Pos, const char* S, size_t LenS) const { return SubStringCompareIAC(data(Pos), length()-Pos, S, LenS); } //SubStringCompareIC renvoi 0 (ÈgalitÈ) si Len1 ou Len2 == 0
	int CompareIACSubString(xsize Pos, xsize Len, const char* S, size_t LenS) const { return SubStringCompareIAC(data(Pos), Len, S, LenS); }
*/
#endif

	bool Equal(const char* S) const { return strcmp(data(), (S ? S : "")) == 0; };
//	bool BeginEqual(const char* S) const { return StringBeginEqual(data(), S); }
//	bool SubStringEqual(xsize Pos, const char* S) const { return StringBeginEqual(data(Pos), S); }
#ifdef TODO_skqdjfhksqjhfksjqdf
	// IC
	bool EqualIC(const char* S) const { return StringEqualIC(data(), S); }
	bool BeginEqualIC(const char* S) const { return StringBeginEqualIC(data(), S); }
	bool SubStringEqualIC(xsize Pos, const char* S) const { return StringBeginEqualIC(data(Pos), S); }
	bool EqualSubStringIC(const char* S) const { return StringBeginEqualIC(S, data()); }
	// IA
	bool EqualIA(const char* S) const { return CompareIA(S) == 0; };
	// IAC
	bool EqualIAC(const char* S) const { return CompareIAC(S) == 0; };
	bool BeginEqualIAC(const char* S) const { return StringBeginEqualIAC(data(), S); }
	bool SubStringEqualIAC(xsize Pos, const char* S) const { return StringBeginEqualIAC(data(Pos), S); }
	bool EqualSubStringIAC(const char* S) const { return StringBeginEqualIAC(S, data()); }


/*	bool EqualIACSubString(const char* S, size_t LenS) const { return CompareIACSubString(S, LenS) == 0; }
	bool EqualIACSubString(xsize Pos, const char* S, size_t LenS) const { return CompareIACSubString(Pos, S, LenS) == 0; }
	bool EqualIACSubString(xsize Pos, xsize Len, const char* S, size_t LenS) const { return CompareIACSubString(Pos, Len, S, LenS) == 0; }
*/
#endif

	XString basename() const;
	XString dirname() const;

//	size_t Sizeof() const { return size_t(sizeof(xsize)+length()); } // overflow ? underflow ?
//	bool ReadFromBuf(const char *buf, size_t *idx, size_t count);
//	bool WriteToBuf(char *buf, size_t *idx, size_t count) const;
//	bool ReadFromFILE(FILE *fp);
//	bool WriteToFILE(FILE *fp) const;
//	//
//	bool ReadFromXBuffer(XRBuffer &unXBuffer); // Impossible de mettre le XBuffer en const car il y a une variable d'instance de XBuffer incrÈmentÈe par ReadFromXBuffer
//	void CatToXBuffer(XBuffer *unXBuffer) const;
////	void WriteToXBuffer(XBuffer *unXBuffer, xsize *idx) const;

  public:
	// OpÈrateur +
	// Chaines
	friend XString operator + (const XString& p1, const XString& p2) { XString s; s=p1; s+=p2; return s; }
//	friend XString operator + (const XString& p1, const char *p2  ) { XString s; s=p1; s+=p2; return s; }
	XString operator + (const char *p2 ) { XString s(*this); s+=p2; return s; }

	friend XString operator + (const char *p1,   const XString& p2) { XString s; s.takeValueFrom(p1); s+=p2; return s; }
//	friend XString operator + (const XConstString& p1,   const XString& p2) { XString s; s=p1; s+=p2; return s; }
//	friend XString operator + (const XString& p1,   const XConstString& p2) { XString s; s=p1; s+=p2; return s; }
//	friend XString operator + (const XConstString& p1,   const XConstString& p2) { XString s; s=p1; s+=p2; return s; }
//	friend XString operator + (const XConstString &p1, const char *p2  ) { XString s; s=p1; s+=p2; return s; }
//	friend XString operator + (const char *p1,   const XConstString &p2) { XString s; s=p1; s+=p2; return s; }

// deactivate during refactoring
//	// Char
//	friend XString operator + (const XString& p1, char p2         ) { XString s; s=p1; s+=p2; return s; }
//	friend XString operator + (char p1,   const XString& p2       ) { XString s; s=p1; s+=p2; return s; }
//	// NumÈrique
//	friend XString operator + (const XString& p1, int p2          ) { XString s; s=p1; s+=p2; return s; }
//	friend XString operator + (int p1,   const XString& p2        ) { XString s; s=p1; s+=p2; return s; }
//	friend XString operator + (const XString& p1, unsigned int p2         ) { XString s; s=p1; s+=p2; return s; }
//	friend XString operator + (unsigned int p1,   const XString& p2       ) { XString s; s=p1; s+=p2; return s; }
//	friend XString operator + (const XString& p1, long p2         ) { XString s; s=p1; s+=p2; return s; }
//	friend XString operator + (long p1,   const XString& p2       ) { XString s; s=p1; s+=p2; return s; }
//	friend XString operator + (const XString& p1, unsigned long long p2        ) { XString s; s=p1; s+=p2; return s; }
//	friend XString operator + (unsigned long long p1,   const XString& p2      ) { XString s; s=p1; s+=p2; return s; }

	// OpÈrateur ==
	// Chaines
	friend bool operator == (const XString& s1,        const XString& s2)      { return s1.Equal(s2); }
	friend bool operator == (const XString& s1,        const char* s2  )        { return s1.Equal(s2); }
	friend bool operator == (const char* s1,            const XString& s2)      { return s2.Equal(s1); }
//	friend bool operator == (const XConstString &s1,   const XString& s2)      { return s1.Compare(s2) == 0; }
//	friend bool operator == (const XString &s1,        const XConstString& s2) { return s1.Compare(s2) == 0; }
//	friend bool operator == (const XConstString &s1,   const XConstString& s2) { return s1.Compare(s2) == 0; }

	friend bool operator != (const XString& s1,        const XString& s2)      { return !s1.Equal(s2); }
	friend bool operator != (const XString& s1,        const char* s2  )        { return !s1.Equal(s2); }
	friend bool operator != (const char* s1,            const XString& s2)      { return !s2.Equal(s1); }
//	friend bool operator != (const XConstString &s1,   const XString& s2)      { return s1.Compare(s2) != 0; }
//	friend bool operator != (const XString &s1,        const XConstString& s2) { return s1.Compare(s2) != 0; }
//	friend bool operator != (const XConstString &s1,   const XConstString& s2) { return s1.Compare(s2) != 0; }

//	friend bool operator <  (const XString& s1, const XString& s2) { return s1.Compare(s2) < 0; }
//	friend bool operator <  (const XString& s1, const char* s2  ) { return s1.Compare(s2) < 0; }
//	friend bool operator <  (const char* s1,   const XString& s2) { return s2.Compare(s1) > 0; }
//
//	friend bool operator >  (const XString& s1, const XString& s2) { return s1.Compare(s2) > 0; }
//	friend bool operator >  (const XString& s1, const char* s2  ) { return s1.Compare(s2) > 0; }
//	friend bool operator >  (const char* s1,   const XString& s2) { return s2.Compare(s1) < 0; }
//
//	friend bool operator <= (const XString& s1, const XString& s2) { return s1.Compare(s2) <= 0; }
//	friend bool operator <= (const XString& s1, const char* s2  ) { return s1.Compare(s2) <= 0; }
//	friend bool operator <= (const char* s1,   const XString& s2) { return s2.Compare(s1) >= 0; }
//
//	friend bool operator >= (const XString& s1, const XString& s2) { return s1.Compare(s2) >= 0; }
//	friend bool operator >= (const XString& s1, const char* s2  ) { return s1.Compare(s2) >= 0; }
//	friend bool operator >= (const char* s1,   const XString& s2) { return s2.Compare(s1) <= 0; }

};

extern const XString NullXString;

XString operator"" _XS ( const char* s, size_t len);

XString SPrintf(const char *format, ...)
	#ifndef _MSC_VER
		__attribute__((format(printf, 1, 2)))
	#endif
;
XString SubString(const char *S, xsize pos, xsize count);
#ifdef TODO_skqdjfhksqjhfksjqdf
XString ToAlpha(const char *S);
XString ToAlpha(const XString &S);
XString ToLower(const char *S, bool FirstCharIsCap = false);
XString ToUpper(const char *S);
#endif
// Deactivate assignment during refactoring to avoid confusion
//XString CleanCtrl(const XString &S);

#endif
