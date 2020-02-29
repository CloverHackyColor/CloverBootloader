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
#include "utf8Conversion.h"

#define LPATH_SEPARATOR L'\\'

extern UINTN XStringWGrowByDefault;
extern void __GLOBAL__sub_I_XStringW();

class XStringW
{
protected:
 	wchar_t *m_data;
	UINTN m_len;
	UINTN m_size;

public:
	void Init(UINTN aSize=0);
	XStringW();
	XStringW(const XStringW &aString);
	XStringW(const wchar_t *);
	XStringW(const wchar_t* S, UINTN count);
	XStringW(const wchar_t);

	XStringW(const char*);

	~XStringW();

protected:
	wchar_t *CheckSize(UINTN nNewSize, UINTN nGrowBy = XStringWGrowByDefault);

public:
	const wchar_t *data(UINTN ui=0) const { return m_data+ui; } // do not multiply by sizeof(wchar_t), it's done by the compiler.
	wchar_t *data(UINTN ui=0) { return m_data+ui; } // do not multiply by sizeof(wchar_t), it's done by the compiler.
	wchar_t *dataWithSizeMin(UINTN pos, UINTN sizeMin, UINTN nGrowBy=XStringWGrowByDefault) { CheckSize(sizeMin, nGrowBy); return data(pos); }

	UINTN length() const { return m_len; }
	UINTN size() const { return m_size; }
	void SetLength(UINTN len);

	/* IsNull ? */
	bool IsNull() const { return length() == 0 ; }
	bool NotNull() const { return length() > 0 ; }

	/* Cast */
	operator const wchar_t *() const { return data(); }

	#if defined(__APPLE__) && defined(__OBJC__)
		operator NSString*() const { return [[[NSString alloc] initWithBytes:data() length:length()*sizeof(wchar_t) encoding:NSUTF32LittleEndianStringEncoding] autorelease]; }
	#endif
	
	int ToInt() const;
	UINTN ToUInt() const;
	
//	XString mbs() const;

	/* [] */
	wchar_t operator [](int i) const { return *data(i); }
	wchar_t operator [](UINTN i) const { return *data(i); }

	wchar_t& operator [](int i) { return *data(i); }
	wchar_t& operator [](UINTN i) { return *data(i); }

	wchar_t LastChar() const { if ( length() > 0 ) return data()[length()-1]; else return 0; }
	void RemoveLastEspCtrl();

	void SetNull() { SetLength(0); };

	void StrnCpy(const wchar_t *buf, UINTN len);
	void StrCpy(const wchar_t *buf);
	void StrnCat(const wchar_t *buf, UINTN len);
	void StrCat(const wchar_t *buf);
	void StrCat(const XStringW &uneXStringW);
	void Delete(UINTN pos, UINTN count=1);

	void Insert(UINTN pos, const XStringW& Str);


	void vSPrintf(const char* format, VA_LIST va);
#ifndef _MSC_VER
  void SPrintf(const char* format, ...) __attribute__((__format__(__printf__, 2, 3)));
#else
  void SPrintf(const char* format, ...);
#endif // !__MSC_VER


	const XStringW &operator =(const XStringW &aString);
	const XStringW &operator =(const wchar_t* S);
	const XStringW &operator =(wchar_t);

	const XStringW &operator += (const XStringW &);
	const XStringW &operator += (const wchar_t* S);
	const XStringW &operator += (wchar_t);

	XStringW SubString(UINTN pos, UINTN count) const;
	UINTN IdxOf(wchar_t c, UINTN Pos = 0) const;
	UINTN IdxOf(const XStringW& S, UINTN Pos = 0) const;
	UINTN RIdxOf(const wchar_t c, UINTN Pos = MAX_XSIZE) const;
	UINTN RIdxOf(const XStringW& S, UINTN Pos = MAX_XSIZE) const;

	void ToLower(bool FirstCharIsCap = false);
	bool IsLetters() const;
	bool IsLettersNoAccent() const;
	bool IsDigits() const;
	bool IsDigits(UINTN pos, UINTN count) const;

	bool ExistIn(const XStringW &S) const { return IdxOf(S) != MAX_XSIZE; }
	void Replace(wchar_t c1, wchar_t c2);
	XStringW SubStringReplace(wchar_t c1, wchar_t c2);

	int Compare(const wchar_t* S) const { return (int)StrCmp(data(), S) ; }

	bool Equal(const wchar_t* S) const { return Compare(S) == 0; };
  bool BeginingEqual(const wchar_t* S) const { return (StrnCmp(data(), S, StrLen(S)) == 0); }
  bool SubStringEqual(UINTN Pos, const wchar_t* S) const { return (StrCmp(data(Pos), S) == 0); }

	XStringW basename() const;
	XStringW dirname() const;
	
//	bool ReadFromBuf(const char *buf, UINTN *idx, UINTN count);
//	bool WriteToBuf(char *buf, UINTN *idx, UINTN count) const;
//	bool ReadFromFILE(FILE *fp);
//	bool WriteToFILE(FILE *fp) const;
	//
//	bool ReadFromXBuffer(XRBuffer &unXBuffer); // Impossible de mettre le XBuffer en const car il y a une variable d'instance de XBuffer incrémentée par ReadFromXBuffer
//	void CatToXBuffer(XBuffer *unXBuffer) const;
//	void WriteToXBuffer(XBuffer *unXBuffer, UINTN *idx) const;

public:
	// + operator
	//    with XStringW
	friend XStringW operator + (const XStringW& p1, const XStringW& p2) { XStringW s; s=p1; s+=p2; return s; }
	//    with const wchar_t
	friend XStringW operator + (const XStringW& p1, const wchar_t *p2  ) { XStringW s; s=p1; s+=p2; return s; }
	friend XStringW operator + (const wchar_t *p1,   const XStringW& p2) { XStringW s; s=p1; s+=p2; return s; }
//	//    with wchar_t
//	friend XStringW operator + (const XStringW& p1, wchar_t p2         ) { XStringW s; s=p1; s+=p2; return s; }
//	friend XStringW operator + (wchar_t p1,   const XStringW& p2       ) { XStringW s; s=p1; s+=p2; return s; }

	// == operator
	friend bool operator == (const XStringW& s1,        const XStringW& s2)      { return s1.Compare(s2) == 0; }
	friend bool operator == (const XStringW& s1,        const wchar_t* s2  )        { return s1.Compare(s2) == 0; }
	friend bool operator == (const wchar_t* s1,            const XStringW& s2)      { return s2.Compare(s1) == 0; }

	friend bool operator != (const XStringW& s1,        const XStringW& s2)      { return s1.Compare(s2) != 0; }
	friend bool operator != (const XStringW& s1,        const wchar_t* s2  )        { return s1.Compare(s2) != 0; }
	friend bool operator != (const wchar_t* s1,            const XStringW& s2)      { return s2.Compare(s1) != 0; }

	friend bool operator <  (const XStringW& s1, const XStringW& s2) { return s1.Compare(s2) < 0; }
	friend bool operator <  (const XStringW& s1, const wchar_t* s2  ) { return s1.Compare(s2) < 0; }
	friend bool operator <  (const wchar_t* s1,   const XStringW& s2) { return s2.Compare(s1) > 0; }

	friend bool operator >  (const XStringW& s1, const XStringW& s2) { return s1.Compare(s2) > 0; }
	friend bool operator >  (const XStringW& s1, const wchar_t* s2  ) { return s1.Compare(s2) > 0; }
	friend bool operator >  (const wchar_t* s1,   const XStringW& s2) { return s2.Compare(s1) < 0; }

	friend bool operator <= (const XStringW& s1, const XStringW& s2) { return s1.Compare(s2) <= 0; }
	friend bool operator <= (const XStringW& s1, const wchar_t* s2  ) { return s1.Compare(s2) <= 0; }
	friend bool operator <= (const wchar_t* s1,   const XStringW& s2) { return s2.Compare(s1) >= 0; }

	friend bool operator >= (const XStringW& s1, const XStringW& s2) { return s1.Compare(s2) >= 0; }
	friend bool operator >= (const XStringW& s1, const wchar_t* s2  ) { return s1.Compare(s2) >= 0; }
	friend bool operator >= (const wchar_t* s1,   const XStringW& s2) { return s2.Compare(s1) <= 0; }

};

//extern const XStringW NullXStringW;
#ifndef _MSC_VER
XStringW SPrintf(const char* format, ...) __attribute__((__format__(__printf__, 1, 2)));
#else
XStringW SPrintf(const char* format, ...);
#endif // !__MSC_VER


XStringW SubString(const wchar_t *S, UINTN pos, UINTN count);

XStringW CleanCtrl(const XStringW &S);

#endif
