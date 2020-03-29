//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//                                      STRING
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

#if !defined(__XSTRING_CPP__)
#define __XSTRING_CPP__

#if 0
#define DBG(...) DebugLog(2, __VA_ARGS__)
#else
#define DBG(...)
#endif

#include "XToolsCommon.h"
#include "XString.h"
#include "XStringW.h"

#include "../../Include/Library/printf_lite.h"

xsize XStringGrowByDefault = 2;
const XString NullXString;


void XString::Init(xsize aSize)
{
	m_data = (char*)malloc( (aSize+1)*sizeof(char) ); /* le 0 terminal n'est pas compté dans m_allocatedSize */
	if ( !m_data ) {
		DebugLog(2, "XString::Init(%llu) : Xalloc returned NULL. Cpu halted\n", (aSize+1)*sizeof(char));
		panic();
	}
	m_allocatedSize = aSize;
	m_data[0] = 0;
}

XString::XString()
{
//Debugf("Construteur\n");
	Init();
}

XString::~XString()
{
//Debugf("Destructeur :%s\n", c);
	delete m_data; // delete nullptr do nothing
}

void XString::setLength(xsize len)
{
	CheckSize(len);
	m_data[len] = 0;
}

/* CheckSize() */
char *XString::CheckSize(xsize nNewSize, xsize nGrowBy)
{
	if ( m_allocatedSize < nNewSize )
	{

		nNewSize += nGrowBy;
		m_data = (char*)realloc(m_data, (nNewSize+1)*sizeof(char), m_allocatedSize*sizeof(wchar_t)); // realloc is identical to malloc if m_data is NULL
		if ( !m_data ) {
			DBG("XString::CheckSize(%d, %d) : Xrealloc(%d, %d, %d) returned NULL. System halted\n", nNewSize, nGrowBy, m_size, (nNewSize+1)*sizeof(char), c);
			panic();
		}
		m_allocatedSize = nNewSize;
	}
	return m_data;
}

void XString::StrnCpy(const char *buf, xsize len)
{
	if ( buf && *buf && len > 0 ) {
		CheckSize(len, 0);
		xsize idx = 0;
		char* p = _data(0);
		while ( idx++ < len  &&  (*p++ = *buf++) != 0 );
		setLength(idx-1); /* SetLength fait _Data[len]=0 */
	}else{
		setLength(0); /* SetLength fait _Data[len]=0 */
	}
}

void XString::StrCpy(const char *buf)
{
	if ( buf && *buf ) {
		StrnCpy(buf, (xsize)strlen(buf)); // overflow ?
	}else{
		setLength(0); /* SetLength fait _Data[len]=0 */
	}
}

//inline
void XString::StrnCat(const char *buf, xsize len)
{
  xsize NewLen;

	if ( buf && *buf && len > 0 ) {
		NewLen = length()+len;
		CheckSize(NewLen, 0);
		memcpy(_data(0)+length(), buf, len);
		setLength(NewLen); /* SetLength fait data()[len]=0 */
	}
}

void XString::Cat(const XString &uneXString)
{
	CheckSize(length()+uneXString.length());
	memcpy(_data(0)+length(), uneXString.m_data, uneXString.length());
	setLength(length() + uneXString.length());
}

void XString::StrCat(const char *buf)
{
	if ( buf && *buf ) {
		StrnCat(buf, (xsize)strlen(buf)); // overflow ?
	}
}

void XString::Delete(xsize pos, xsize count)
{
	if ( pos < length() ) {
		if ( count != MAX_XSIZE  &&  pos + count < length() ) {
			memmove(_data(0)+pos, data()+pos+count, length()-pos-count);
			setLength(length()-count);
//			data()[length()] = 0; fait dans setlength();
		}else{
			setLength(pos);
//			data()[length()] = 0; fait dans setlength();
		}
	}
}

void XString::Insert(xsize pos, const XString& Str)
{
	if ( pos < length() ) {
		CheckSize(length()+Str.length());
		memmove(_data(0)+pos+Str.length(), data()+pos, length()-pos+1); // +1 to copy the NULL terminator
		memcpy(_data(0)+pos, Str.data(), Str.length());
	}else{
		StrCat(Str);
	}
}

// Deactivate assignment during refactoring to avoid confusion
//void XString::Replace(char c1, char c2)
//{
//  char* p;
//
//	p = data();
//	while ( *p ) {
//		if ( *p == c1 ) *p = c2;
//		p += 1;
//	}
//}
//
//XString XString::SubStringReplace(char c1, char c2)
//{
//  char* p;
//  XString Result;
//
//	p = data();
//	while ( *p  ) {
//		if ( *p == c1 ) Result += c2;
//		else Result += *p;
//		p++;
//	}
//	return Result;
//}

/* this is used when printf can only output unicode, so we need a conversion back to utf8 */
//static XString* XString_sprintfBuf;
//static xsize XString_sprintfBuf_len;
//static wchar_t XString_char_wait;
//
//static unsigned int XString_transmitSPrintf_utf32(const wchar_t wchar1, const wchar_t wchar2)
//{
//    unsigned int ret = 0;
//	UINTN utf32_char;
//
//	if ((wchar1 & 0xFC00) == 0xD800) { /* surrogates */
//		if ((wchar2 & 0xFC00) == 0xDC00) {
//			utf32_char = wchar1;
//			utf32_char &= 0x03FF;
//			utf32_char <<= 10;
//			utf32_char |= ((UINTN)wchar2) & 0x03FF;
//			utf32_char += 0x10000;
//			ret = 2;
//		}else{
//			// error
//			return 1; // Ignore wchar1. Tell the caller we used wchar1
//		}
//	}else{
//		utf32_char = wchar1;
//		ret = 1;
//	}
//
//	/* assertion: utf32_char is a single UTF-4 value */
//	int bits;
//
//	if (utf32_char < 0x80) {
//		(*XString_sprintfBuf) += (char)utf32_char;
//		bits = -6;
//	}
//	else if (utf32_char < 0x800) {
//		(*XString_sprintfBuf) += (char)(((utf32_char >> 6) & 0x1F) | 0xC0);
//		bits = 0;
//	}
//	else if (utf32_char < 0x10000) {
//		(*XString_sprintfBuf) += (char)(((utf32_char >> 12) & 0x0F) | 0xE0);
//		bits = 6;
//	}
//	else {
//		(*XString_sprintfBuf) += (char)(((utf32_char >> 18) & 0x07) | 0xF0);
//		bits = 12;
//	}
//	for (; bits >= 0; bits -= 6) {
//		(*XString_sprintfBuf) += (char)(((utf32_char >> bits) & 0x3F) | 0x80);
//	}
//	return ret;
//}
//
//
//static void XString_transmitSPrintf(const wchar_t* buf, size_t nbchar)
//{
//
//	#if __WCHAR_MAX__ <= 0xFFFF
//	// wchar_t is UTF16
//
//	unsigned int ret = 1;
//	if ( XString_char_wait ) {
//		ret = XString_transmitSPrintf_utf32(XString_char_wait, buf[0]);
//		XString_char_wait = 0;
//	}
//	xsize i;
//	for ( i = ret-1 ; i < nbchar-1 ; ) // cast ok, ret >
//	{
//		ret = XString_transmitSPrintf_utf32(buf[i], buf[i+1]);
//		i += ret;
//	}
//	if ( i < nbchar ) XString_char_wait = buf[i];
//	#else
//	#warning TODO
//	#endif
//}
//
//void XString::vSPrintf(const char* format, va_list va)
//{
//	SetLength(0);
//
//	XString_sprintfBuf = this;
//	XString_sprintfBuf_len = 0;
//	XString_char_wait = 0;
//	vprintf_with_callback(format, va, XString_transmitSPrintf);
//	if ( XString_char_wait ) XString_transmitSPrintf_utf32(XString_char_wait, 0);
//}

//static XString* XString_sprintfBuf;

static void XString_transmitSPrintf(const char* buf, unsigned int nbchar, void* context)
{
//	(*XString_sprintfBuf).StrnCat(buf, nbchar);
	((XString*)(context))->StrnCat(buf, (xsize)nbchar); // nbchar cannot be negative
}

XString& XString::vSPrintf(const char* format, va_list va)
{
	setLength(0);

//	XString_sprintfBuf = this;
	vprintf_with_callback(format, va, XString_transmitSPrintf, this);
	return *this;
}

//void XString::vSPrintf(const char* format, va_list va)
//{
	// This is an attempt to use _PPrint from IO.c. Problem is : you have to allocate the memory BEFORE calling it.
//  POOL_PRINT  spc;
//  PRINT_STATE ps;
//
//  ZeroMem(&spc, sizeof (spc));
//  spc.Str = data();
//  SetLength(0);
//  spc.Len = 0;
//  spc.Maxlen = m_size;
//  ZeroMem(&ps, sizeof (ps));
//  ps.Output   = (IN EFI_STATUS (EFIAPI *)(VOID *context, CONST CHAR16 *str))_PoolPrint;
//  ps.Context  = (void*)&spc;
//  ps.fmt.u.pw = format;
//
//  VA_COPY(ps.args, va);
//  _PPrint (&ps);
//  va_end(ps.args);
//}

XString& XString::SPrintf(const char *Format, ...)
{
  va_list va;

	va_start(va, Format);
	vSPrintf(Format, va);
	va_end(va);
	return *this;
}


// Deactivate assignment during refactoring to avoid confusion
XString XString::basename() const
{
	if ( lastChar() == PATH_SEPARATOR ) {
		DebugLog(2, "XString::basename() -> LastChar() == PATH_SEPARATOR");
		panic();
	}
	xsize idx = RIdxOf(XString().SPrintf("%c",PATH_SEPARATOR)); // ctor char disabled during refactoring to avoid confusion
	if ( idx == MAX_XSIZE ) return NullXString;
	return SubString(idx+1, length()-idx-1);
}

XString XString::dirname() const
{
	xsize idx = RIdxOf(XString().SPrintf("%c",PATH_SEPARATOR)); // ctor char disabled during refactoring to avoid confusion
	if ( idx == MAX_XSIZE ) return NullXString;
	#ifdef __DEV_WIN32__
		if ( idx == 1  &&  *data(0) == PATH_SEPARATOR ) {
			// this string is an icomplete UNC name : \\server
			return NullXString;
		}
	#endif
	return SubString(0, idx);
}

//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
// Constructeurs Chaines
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

XString::XString(const XString &aString)
{
//Debugf("Construteur const String & : %s\n", aString);
	Init(aString.length());
	StrnCpy(aString.data(), aString.length());
}

XString::XString(XString&& aString) // Move constructor
{
	m_data = aString.m_data;
	m_allocatedSize = aString.m_allocatedSize;
	aString.m_data = 0;
	aString.m_allocatedSize = 0;
}

//// Deactivate assignment during refactoring to avoid confusion
//XString::XString(const wchar_t *S)
//{
//	Init();
//	SPrintf("%ls", S);
//}

/*
XString::XString(const NSString* const aNSString, NSStringEncoding encoding)
{
	char buf[ [aNSString length]*MB_CUR_MAX+MB_CUR_MAX ];
	if ( [aNSString getCString:buf maxLength:[aNSString length]*MB_CUR_MAX+MB_CUR_MAX encoding:encoding] )
	{
		Init(strlen(buf));
		StrnCpy(buf);
	}
	else {
		Init(0);
	}
}
*/

//
//XString::XString(const XConstString &aConstString)
//{
////Debugf("Construteur const ConstString & : %s\n", aConstString);
//	Init( aConstString.length() );
//	StrnCpy( aConstString.data() );
//}

// Deactivate assignment during refactoring to avoid confusion
//XString::XString(const char *S)
//{
////Debugf("Construteur const char * : %s\n", S);
//	Init((xsize)strlen(S)); // overflow ?
//	if ( S ) StrCpy(S);
//}
//
//XString::XString(const char *S, xsize count)
//{
////Debugf("Construteur const char *, unsigned int :%s   %d\n", S, count);
//	Init(count);
//	StrnCpy(S, count);
//}

//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
// Constructeurs CaractËres
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

// Deactivate assignment during refactoring to avoid confusion
//XString::XString(char aChar)
//{
////TRACE("Construteur char \n");
//	Init(1);
//	StrnCpy(&aChar, 1);
//}

//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
// Constructeurs numériques
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

//XString::XString(int i)
//{
//  char buf[1024];
//  unsigned int len;
//
////TRACE("Copy Constructor(int) -> (%lu) %d\n", this, i);
//	len = sprintf(buf,"%d", i);
//	Init(len);
//	StrnCpy(buf, len);
//}

//XString::XString(unsigned int ui)
//{
//  char buf[1024];
//  unsigned int len;
//
////TRACE("Copy Constructor(unsigned int) -> (%lu) %u\n", this, ui);
//	len = sprintf(buf,"%u", ui);
//	Init(len);
//	StrnCpy(buf, len);
//}
//
//XString::XString(long l)
//{
//  char buf[1024];
//  unsigned int len;
//
////TRACE("Copy Constructor(long) -> (%lu) %ld\n", this, l);
//	len = sprintf(buf,"%ld", l);
//	Init(len);
//	StrnCpy(buf, len);
//}
//
//XString::XString(unsigned long ul)
//{
//  char buf[1024];
//  unsigned int len;
//
////TRACE("Copy Constructor(unsigned long long) -> (%lu) %lu\n", this, ul);
//	len = sprintf(buf,"%lu", ul);
//	Init(len);
//	StrnCpy(buf, len);
//}
//

//-------------------------------------------------------------------------------------------------
// StringCompare
//-------------------------------------------------------------------------------------------------
// -1 si buf1 est plus grand
//  0 si égal
//  1 si buf2 est plus grand
//-------------------------------------------------------------------------------------------------


XString XString::SubString(xsize pos, xsize count) const
{
	if ( count > length()-pos ) count = length()-pos;
	return XString().takeValueFrom( &(data()[pos]), count);
}

xsize XString::IdxOf(char aChar, xsize Pos) const
{
  xsize Idx;

	for ( Idx=Pos ; Idx<length() ; Idx+=1 ) {
        if ( data()[Idx] == aChar ) return Idx;
	}
	return MAX_XSIZE;
}

xsize XString::IdxOf(const XString &S, xsize Pos) const
{
  xsize i;
  xsize Idx;

	if ( length() < S.length() ) return MAX_XSIZE;
	for ( Idx=Pos ; Idx<=length()-S.length() ; Idx+=1 ) {
		i = 0;
	    while( i<S.length()  &&  ( data()[Idx+i] - S[i] ) == 0 ) i += 1;
		if ( i == S.length() ) return Idx;
	}
	return MAX_XSIZE;
}

#ifdef NOT_USED_ANYMORE_skqdjfhksqjhfksjqdf
unsigned int XString::IdxOfIC(const XString &S, xsize Pos) const
{
  xsize i;
  xsize Idx;

	if ( length() < S.length() ) return MAX_XSIZE;
	for ( Idx=Pos ; Idx<=length()-S.length() ; Idx+=1 ) {
		i = 0;
	    while( i<S.length()  &&  ( Minuscule(data()[Idx+i]) - Minuscule(S[i]) ) == 0 ) i += 1;
		if ( i == S.length() ) return Idx;
	}
	return MAX_XSIZE;
}

unsigned int XString::IdxOfIAC(const XString &S, xsize Pos) const
{
  xsize i;
  xsize Idx;

	if ( length() < S.length() ) return MAX_XSIZE;
	for ( Idx=Pos ; Idx<=length()-S.length() ; Idx+=1 ) {
		i = 0;
		while( i<S.length()  &&  ( MinusculeSansAccent(data()[Idx+i]) - MinusculeSansAccent(S[i]) ) == 0 ) i += 1;
		if ( i == S.length() ) return Idx;
	}
	return MAX_XSIZE;
}
#endif

xsize XString::RIdxOf(const XString &S, xsize Pos) const
{
  xsize i;
  xsize Idx;

	if ( S.length() == 0 ) return MAX_XSIZE;
	if ( Pos > length() ) Pos = length();
	if ( Pos < S.length() ) return MAX_XSIZE;
	Pos -= S.length();
	for ( Idx=Pos+1 ; Idx-- > 0 ; ) {
		i = 0;
		while( i<S.length()  &&  data()[Idx+i] == S[i] ) i += 1;
		if ( i == S.length() ) return Idx;
	}
	return MAX_XSIZE;
}

#ifdef NOT_USED_ANYMORE_skqdjfhksqjhfksjqdf
bool XString::DeleteIC(const XString &S)
{
  xsize Idx;

	Idx = IdxOfIC(S);
	if ( Idx == MAX_XSIZE ) return false;
	Delete(Idx, S.length());
	return YES;
}
#endif

#ifdef NOT_USED_ANYMORE_skqdjfhksqjhfksjqdf
void XString::ToLower(bool FirstCharIsCap)
{

	if ( length() > 0 )
	{
	  unsigned int ui;

		if ( FirstCharIsCap ) {
			data()[0] = Majuscule(data()[0]);
			ui = 1;
		}else{
			ui = 0;
		}
		for ( ; ui < length() ; ui+=1 ) {
			data()[ui] = Minuscule(data()[ui]);
		}
	}
}

bool XString::IsLetters() const
{
  const char *p;
  char aChar;

	p = data();
	if ( !*p ) return false;
	for ( ; *p ; p+=1 ) {
		aChar = MinusculeSansAccent(*p);  // toutes les lettres, avec accent ou pas, seront dans l'intervalle 'a'..'z'
		if ( aChar < 'a' ) return false;
		if ( aChar > 'z' ) return false;
	}
	return true;
}
#endif

bool XString::IsDigits() const
{
  const char *p;

	p = data();
	if ( !*p ) return false;
	for ( ; *p ; p+=1 ) {
		if ( *p < '0' ) return false;
		if ( *p > '9' ) return false;
	}
	return true;
}

bool XString::IsDigits(xsize pos, xsize count) const
{
  const char *p;
  const char *q;

	if ( pos >= length() ) {
		DebugLog(2, "XString::IsDigits pos >= length()"); // We should #ifdef this to keep that for debug
		return false;
	}
	if ( pos+count > length() ) {
		DebugLog(2, "XString::IsDigits pos+count > length()"); // We should #ifdef this to keep that for debug
		return false;
	}
	p = data() + pos;
	q = p + count;
	for ( ; p < q ; p+=1 ) {
		if ( *p < '0' ) return false;
		if ( *p > '9' ) return false;
	}
	return true;
}

#ifdef NOT_USED_ANYMORE_skqdjfhksqjhfksjqdf
bool XString::IsLettersNoAccent() const
{
  const char *p;
  char aChar;

	p = data();
	if ( !*p ) return false;
	for ( ; *p ; p+=1 ) {
		aChar = Minuscule(*p); // Uniquement les lettres maj et min sans accent seront dans l'intervalle 'a'..'z'
		if ( aChar < 'a' ) return false;
		if ( aChar > 'z' ) return false;
	}
	return true;
}
#endif

void XString::removeLastEspCtrl()
{
  char *p;

	if ( length() > 0 ) {
		p = _data(0) + length() - 1;
		if ( *p >= 0 && *p <= ' ' ) {
			p -= 1;
			while ( p>data() && *p >= 0 && *p <= ' ' ) p -= 1;
			if ( p>data() ) {
				setLength( (xsize)(p-data()+1) ); // safe (p-data()+1 < length()
			}else{
				if ( *p >= 0 && *p <= ' ' ) setLength(0);
				else setLength(1);
			}
		}
	}
}
//
//int XString::ToInt() const
//{
//  int i;
//
//	if ( sscanf(data(), "%d", &i) != 1 ) return NOTAINT;
//	return i;
//}
//
//unsigned int XString::TOUInt() const
//{
//  unsigned int u;
//
//	if ( sscanf(data(), "%u", &u) != 1 ) return MAXUINT;
//	return u;
//}

//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------
//
//bool XString::ReadFromBuf(const char *buf, size_t *idx, size_t count)
//{
//  xsize longueur;
//
////mylog2(::SPrintf("XString::ReadFromBuf *idx=%d count=%d - %d %d %d %d\n", *idx, count, buf[*idx+0], buf[*idx+1], buf[*idx+2], buf[*idx+3]).c);
//	if ( count-*idx >= sizeof(longueur) ) {
//		longueur = *((xsize *)(buf+*idx));
////mylog2(::SPrintf("XString::ReadFromBuf *idx=%d count=%d longueur=%d\n", *idx, count, longueur).c);
//		*idx += sizeof(longueur);
//		if (  longueur > 0  &&  count-*idx>=longueur  ) memcpy(DataWithSizeMin(0, longueur), buf+*idx, longueur);
//		*idx += longueur;
//		SetLength(longueur);
//		return true;
//	}else{
//		SetNull();
//		return false;
//	}
//}
//
//bool XString::WriteToBuf(char *buf, size_t *idx, size_t count) const
//{
//  xsize longueur;
//
//	if ( count-*idx < sizeof(longueur) + length() ) return false;
//	longueur = length();
//	memcpy(buf+*idx, &longueur, sizeof(longueur));
//	*idx += sizeof(longueur);
//	memcpy(buf+*idx, data(), length());
//	*idx += length();
//	return true;
//}
//
//bool XString::ReadFromFILE(FILE *fp)
//{
//  unsigned int longueur;
//
//	if ( fread(&longueur, sizeof(longueur), 1, fp) != 1 ) goto fin;
//	if ( longueur > 0  &&  fread(DataWithSizeMin(0, longueur), longueur, 1, fp) != 1 ) goto fin;
//	SetLength(longueur);
//	return true;
//  fin:
//	SetNull();
//	return false;
//}
//
//bool XString::WriteToFILE(FILE *fp) const
//{
//  xsize longueur;
//
//	longueur = length();
//	if ( fwrite(&longueur, sizeof(longueur), 1, fp) != 1 ) return false;
//	if ( longueur > 0  &&  fwrite(data(), longueur, 1, fp) != 1 ) return false;
//	return true;
//}
//
//#ifdef __DEVTOOLS_SOCKETS__
//void XString::ReadFromSOCKETT(SOCKET Sock, unsigned int LenMax, unsigned int TO, const char *ErrMsg)
//{
//  unsigned int longueur;
//
//	SockReceiveT(Sock, &longueur, sizeof(longueur), TO, ErrMsg);
//	if ( longueur > LenMax ) Throw("Longueur reÁue (%d) supérieure ‡ la longueur max (%d)", longueur, LenMax);
//	if ( longueur > 0 ) SockReceiveT(Sock, DataWithSizeMin(0, longueur, 0), longueur, TO, ErrMsg);
//	SetLength(longueur);
//}
//
//bool XString::ReadFromSOCKET(SOCKET Sock, unsigned int LenMax, unsigned int TO, const char *ErrMsg)
//{
//	try
//	{
//		ReadFromSOCKETT(Sock, LenMax, TO, ErrMsg);
//		SetLastErrorFlag(false);
//	}
//	StdCatch();
//	return !LastErrorFlag();
//}
//
//void XString::WriteToSOCKETT(SOCKET Sock, unsigned int TO,const char *ErrMsg) const
//{
//  unsigned int longueur;
//
//	longueur = length();
//	SockSendT(Sock, &longueur, sizeof(longueur), TO, ErrMsg);
//	if ( longueur > 0 ) SockSendT(Sock, data(), longueur, TO, ErrMsg);
//}
//
//bool XString::WriteToSOCKET(SOCKET Sock, unsigned int TO,const char *ErrMsg) const
//{
//	try
//	{
//		WriteToSOCKET(Sock, TO, ErrMsg);
//		SetLastErrorFlag(false);
//	}
//	StdCatch();
//	return !LastErrorFlag();
//}
//#endif
//
//bool XString::ReadFromXBuffer(XRBuffer &unXBuffer)
//{
//  xsize longueur;
//
//	if ( !unXBuffer.GetXSize(&longueur) ) goto fin;
//	if ( longueur>0  &&  !unXBuffer.Get(DataWithSizeMin(0, longueur), longueur) ) goto fin;
//	SetLength(longueur);
//	return true;
//fin:
//	SetNull();
//	return false;
//}
//
//void XString::CatToXBuffer(XBuffer *unXBuffer) const
//{
//	(*unXBuffer).Cat(length());
//	(*unXBuffer).Cat(data(), length());
//}

//*************************************************************************************************
//
//                                       Opérateurs =
//
//*************************************************************************************************


//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
// Opérateur = CaractËres
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

// Deactivate assignment during refactoring to avoid confusion
//const XString &XString::operator =(char aChar)
//{
////TRACE("Operator =char \n");
//	StrnCpy(&aChar, 1);
//	return *this;
//}


//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
// Opérateur = Chaines
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

const XString &XString::operator =(const XString &aString)
{
//TRACE("Operator =const XString&\n");
	StrnCpy(aString.data(), aString.length());
	return *this;
}

XString& XString::operator =(XString&& aString)
{
	delete m_data; // delete does nothing if m_data is NULL
	m_data = aString.m_data;
	m_allocatedSize = aString.m_allocatedSize;
	aString.m_data = 0;
	aString.m_allocatedSize = 0;
	return *this;
}

//
//const XString &XString::operator =(const XConstString &aConstString)
//{
////TRACE("Operator =const XString&\n");
//	StrnCpy(aConstString.data());
//	return *this;
//}

// Deactivate assignment during refactoring to avoid confusion
//const XString &XString::operator =(const char *S)
//{
////TRACE("Operator =const char *\n");
//	StrCpy(S);
//	return *this;
//}


//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
// Opérateur = numériques
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//
//const XString &XString::operator =(int i)
//{
//  char buf[1024];
//  unsigned int len;
//
////TRACE("Opérateur =(int) -> (%lu) %d\n", this, i);
//	len = sprintf(buf,"%d", i);
//	StrnCpy(buf, len);
//	return *this;
//}
//
//const XString &XString::operator =(unsigned int ui)
//{
//  char buf[1024];
//  unsigned int len;
//
////TRACE("Opérateur =(long) -> (%lu) %u\n", this, ui);
//	len = sprintf(buf,"%u", ui);
//	StrnCpy(buf, len);
//	return *this;
//}
//
//const XString &XString::operator =(long l)
//{
//  char buf[1024];
//  unsigned int len;
//
////TRACE("Opérateur =(long) -> (%lu) %ld\n", this, l);
//	len = sprintf(buf,"%ld", l);
//	StrnCpy(buf, len);
//	return *this;
//}
//
//const XString &XString::operator =(unsigned long ul)
//{
//  char buf[1024];
//  unsigned int len;
//
////TRACE("Opérateur =(long) -> (%lu) %lu\n", this, ul);
//	len = sprintf(buf,"%lu", ul);
//	StrnCpy(buf, len);
//	return *this;
//}
//


//*************************************************************************************************
//
//                                       Opérateurs +=
//
//*************************************************************************************************


//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
// Opérateur = CaractËres
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

// Deactivate assignment during refactoring to avoid confusion
//const XString &XString::operator +=(char aChar)
//{
////TRACE("Operator +=char \n");
//	StrnCat(&aChar, 1);
//	return *this;
//}


//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
// Opérateur = Chaines
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

const XString &XString::operator +=(const XString &aString)
{
//TRACE("Operator +=const XString&\n");
	StrnCat(aString.data(), aString.length());
	return *this;
}
//
//const XString &XString::operator +=(const XConstString &aConstString)
//{
////TRACE("Operator +=const EConstString&\n");
//	StrnCat(aConstString.data(), aConstString.length());
//	return *this;
//}

const XString &XString::operator +=(const char *S)
{
//TRACE("operator +=const char *\n");
	StrCat(S);
	return *this;
}


//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
// Opérateur = numériques
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//
//const XString &XString::operator +=(int i)
//{
//  char buf[1024];
//  unsigned int len;
//
////TRACE("Opérateur =(int) -> (%lu) %d\n", this, i);
//	len = sprintf(buf,"%d", i);
//	StrnCat(buf, len);
//	return *this;
//}
//
//const XString &XString::operator +=(unsigned int ui)
//{
//  char buf[1024];
//  unsigned int len;
//
////TRACE("Opérateur =(unsigned int) -> (%lu) %u\n", this, ui);
//	len = sprintf(buf,"%u", ui);
//	StrnCat(buf, len);
//	return *this;
//}
//
//const XString &XString::operator +=(long l)
//{
//  char buf[1024];
//  unsigned int len;
//
////TRACE("Opérateur =(long) -> (%lu) %ld\n", this, l);
//	len = sprintf(buf,"%ld", l);
//	StrnCat(buf, len);
//	return *this;
//}
//
//const XString &XString::operator +=(unsigned long ul)
//{
//  char buf[1024];
//  unsigned int len;
//
////TRACE("Opérateur =(unsigned long long) -> (%lu) %lu\n", this, ul);
//	len = sprintf(buf,"%lu", ul);
//	StrnCat(buf, len);
//	return *this;
//}


//-----------------------------------------------------------------------------
//                                 Fonction
//-----------------------------------------------------------------------------

XString operator"" _XS ( const char* s, size_t len)
{
  XString returnValue;
	if ( len > MAX_XSIZE ) len = MAX_XSIZE;
	returnValue.takeValueFrom(s, len);
    return returnValue; // don't do "return returnValue.takeValueFrom(s, len)" because it break the return value optimization.
}

XString SPrintf(const char *format, ...)
{
	va_list     va;
	XString str;
	
	va_start (va, format);
	str.vSPrintf(format, va);
	va_end(va);
	
	return str;
}

XString SubString(const char *S, xsize pos, xsize count)
{
	if ( strlen(S)-pos < count ) count = (xsize)(strlen(S)-pos); // overflow ?
	return XString().takeValueFrom( S+pos, count );
}

#ifdef NOT_USED_ANYMORE_skqdjfhksqjhfksjqdf
XString ToAlpha(const char *S)
{
  XString ReturnValue;
  unsigned int i;

	for ( i=0 ; S[i] ; i+=1 ) {
		ReturnValue += ToAlpha(S[i]);
	}
	return ReturnValue;
}

XString ToAlpha(const XString &S)
{
  XString ReturnValue;
  unsigned int i;

	for ( i=0 ; i<S.length() ; i+=1 ) {
		ReturnValue += ToAlpha(S[i]);
	}
	return ReturnValue;
}

XString ToLower(const char *S, bool FirstCharIsCap)
{
  XString ReturnValue;
  unsigned int ui;

	if ( S && *S ) {
		if ( FirstCharIsCap ) ReturnValue = Majuscule(S[0]);
		else ReturnValue = Minuscule(S[0]);

		for ( ui=1 ; S[ui] ; ui+=1 ) {
			ReturnValue += Minuscule(S[ui]);
		}
	}
	return ReturnValue;
}

XString ToUpper(const char *S)
{
  XString ReturnValue;
  unsigned int ui;

	if ( S && *S ) {
		for ( ui=0 ; S[ui] ; ui+=1 ) {
			ReturnValue += Majuscule(S[ui]);
		}
	}
	return ReturnValue;
}
#endif

// Deactivate during refactoring to avoid confusion
//XString CleanCtrl(const XString &S)
//{
//  XString ReturnValue;
//  xsize i;
//
//	for ( i=0 ; i<S.length() ; i+=1 ) {
//		if ( S[i] >=0  &&  S[i] < ' ' ) ReturnValue += 'x'; /* Les char sont signés !!! */
//		else ReturnValue += S[i];
//	}
//	return ReturnValue;
//}

#endif
