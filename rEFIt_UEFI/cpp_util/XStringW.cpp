//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//                                      STRING
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

#if !defined(__XStringW_CPP__)
#define __XStringW_CPP__

#if 0
#define DBG(...) DebugLog(2, __VA_ARGS__)
#else
#define DBG(...)
#endif

#include "XStringW.h"

extern "C" {
  #include <Library/MemoryAllocationLib.h>
  #include <Library/BaseMemoryLib.h>
	#include "Platform/Platform.h"
  #include "refit/IO.h"
}

UINTN XStringWGrowByDefault = 1024;
//const XStringW NullXStringW;


void XStringW::Init(UINTN aSize)
{
//DBG("Init aSize=%d\n", aSize);
	m_data = (wchar_t*)AllocatePool( (aSize+1)*sizeof(wchar_t) ); /* le 0 terminal n'est pas compté dans mSize */
	if ( !m_data ) {
		DBG("XStringW::Init(%d) : AllocatePool returned NULL. Cpu halted\n", (aSize+1)*sizeof(wchar_t));
		CpuDeadLoop();
	}
	m_size = aSize;
	m_len = 0;
	m_data[0] = 0;
}

//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
// Constructor
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

XStringW::XStringW()
{
DBG("Construteur\n");
	Init();
}

XStringW::XStringW(const XStringW &aString)
{
DBG("Constructor(const XStringW &aString) : %s\n", aString.data());
	Init(aString.length());
	StrnCpy(aString.data(), aString.length());
}

XStringW::XStringW(const wchar_t *S)
{
DBG("Constructor(const wchar_t *S) : %s, StrLen(S)=%d\n", S, StrLen(S));
	Init(StrLen(S));
	if ( S ) StrCpy(S);
}

XStringW::XStringW(const wchar_t *S, UINTN count)
{
DBG("Constructor(const wchar_t *S, UINTN count) : %s, %d\n", S, count);
	Init(count);
	StrnCpy(S, count);
}

XStringW::XStringW(const wchar_t aChar)
{
DBG("Constructor(const wchar_t aChar)\n");
	Init(1);
	StrnCpy(&aChar, 1);
}

//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
// Destructor
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx


XStringW::~XStringW()
{
DBG("Destructor :%s\n", data());
	FreePool((void*)m_data);
}

//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

void XStringW::SetLength(UINTN len)
{
//DBG("SetLength(%d)\n", len);
	m_len = len;
	m_data[len] = 0;

	if ( StrLen(data()) != len ) {
		DBG("XStringW::SetLength(UINTN len) : StrLen(data()) != len (%d != %d). System halted\n", StrLen(data()), len);
		CpuDeadLoop();
	}
}

/* CheckSize() */
wchar_t *XStringW::CheckSize(UINTN nNewSize, UINTN nGrowBy)
{
//DBG("CheckSize: m_size=%d, nNewSize=%d\n", m_size, nNewSize);

	if ( m_size < nNewSize )
	{
		nNewSize += nGrowBy;
		m_data = (wchar_t*)ReallocatePool(m_size*sizeof(wchar_t), (nNewSize+1)*sizeof(wchar_t), m_data);
		if ( !m_data ) {
  		DBG("XStringW::CheckSize(%d, %d) : ReallocatePool(%d, %d, %d) returned NULL. System halted\n", nNewSize, nGrowBy, m_size, (nNewSize+1)*sizeof(wchar_t), m_data);
	  	CpuDeadLoop();
		}
		m_size = nNewSize;
	}
	return m_data;
}

void XStringW::StrnCpy(const wchar_t *buf, UINTN len)
{
	if ( buf && *buf && len > 0 ) {
		CheckSize(len, 0);
		CopyMem(data(), buf, len*sizeof(wchar_t));
	}
	SetLength(len); /* data()[len]=0 done in SetLength */
}

void XStringW::StrCpy(const wchar_t *buf)
{
	if ( buf && *buf ) {
		StrnCpy(buf, StrLen(buf));
	}else{
		SetLength(0); /* data()[0]=0 done in SetLength */
	}
}

//inline
void XStringW::StrnCat(const wchar_t *buf, UINTN len)
{
  UINTN NewLen;

	if ( buf && *buf && len > 0 ) {
		NewLen = length()+len;
		CheckSize(NewLen, 0);
		CopyMem(data(length()), buf, len*sizeof(wchar_t));
		SetLength(NewLen); /* data()[NewLen]=0 done in SetLength */
	}
}

inline void XStringW::StrCat(const wchar_t *buf)
{
	if ( buf && *buf ) {
		StrnCat(buf, StrLen(buf));
	}
}

void XStringW::StrCat(const XStringW &uneXStringWW)
{
	StrnCat(uneXStringWW.data(), uneXStringWW.length());
}

void XStringW::Delete(UINTN pos, UINTN count)
{
	if ( pos < length() ) {
		if ( count != MAX_UINTN  &&  pos + count < length() ) {
			CopyMem( data(pos), data(pos+count), (length()-pos-count)*sizeof(wchar_t)); // CopyMem handles overlapping memory move
			SetLength(length()-count);/* data()[length()-count]=0 done in SetLength */
		}else{
			SetLength(pos);/* data()[pos]=0 done in SetLength */
		}
	}
}

void XStringW::Insert(UINTN pos, const XStringW& Str)
{
	if ( pos < length() ) {
		CheckSize(length()+Str.length());
		CopyMem(data(pos + Str.length()),  data(pos),  (length()-pos)*sizeof(wchar_t));
		CopyMem(data(pos), Str.data(), Str.length()*sizeof(wchar_t));
		SetLength(length()+Str.length());
	}else{
		StrCat(Str);
	}
}

void XStringW::Replace(wchar_t c1, wchar_t c2)
{
  wchar_t* p;

	p = data();
	while ( *p ) {
		if ( *p == c1 ) *p = c2;
		p += 1;
	}
}

XStringW XStringW::SubStringReplace(wchar_t c1, wchar_t c2)
{
  wchar_t* p;
  XStringW Result;

	p = data();
	while ( *p  ) {
		if ( *p == c1 ) Result += c2;
		else Result += *p;
		p++;
	}
	return Result;
}

void XStringW::vSPrintf(const wchar_t *format, VA_LIST va)
{
  POOL_PRINT  spc;
  PRINT_STATE ps;

  ZeroMem(&spc, sizeof (spc));
  spc.Str = data();
  SetLength(0);
  spc.Len = 0;
  spc.Maxlen = m_size;
  ZeroMem(&ps, sizeof (ps));
  ps.Output   = (IN EFI_STATUS (EFIAPI *)(VOID *context, CONST CHAR16 *str))_PoolPrint;
  ps.Context  = (void*)&spc;
  ps.fmt.u.pw = format;

  VA_COPY(ps.args, va);
  _PPrint (&ps);
  VA_END(ps.args);
}

void XStringW::SPrintf(const wchar_t *format, ...)
{
  VA_LIST     va;

  VA_START (va, format);
	vSPrintf(format, va);
	VA_END(va);
}

XStringW XStringW::basename() const
{
	UINTN idx = RIdxOf(LPATH_SEPARATOR);
	if ( idx == MAX_UINTN ) return XStringW();
	return SubString(idx+1, length()-idx-1);
}

XStringW XStringW::dirname() const
{
	UINTN idx = RIdxOf(LPATH_SEPARATOR);
	if ( idx == MAX_UINTN ) return XStringW();
	return SubString(0, idx);
}

XStringW XStringW::SubString(UINTN pos, UINTN count) const
{
	if ( count > length()-pos ) count = length()-pos;
	return XStringW( &(data()[pos]), count);
}

UINTN XStringW::IdxOf(wchar_t aChar, UINTN Pos) const
{
  UINTN Idx;

	for ( Idx=Pos ; Idx<length() ; Idx+=1 ) {
        if ( data()[Idx] == aChar ) return Idx;
	}
	return MAX_UINTN;
}

UINTN XStringW::IdxOf(const XStringW &S, UINTN Pos) const
{
  UINTN i;
  UINTN Idx;

	if ( length() < S.length() ) return MAX_UINTN;
	for ( Idx=Pos ; Idx<=length()-S.length() ; Idx+=1 ) {
		i = 0;
	    while( i<S.length()  &&  ( data()[Idx+i] - S[i] ) == 0 ) i += 1;
		if ( i == S.length() ) return Idx;
	}
	return MAX_UINTN;
}

UINTN XStringW::RIdxOf(const wchar_t charToSearch, UINTN Pos) const
{
  UINTN Idx;

	if ( Pos > length() ) Pos = length();
	if ( Pos < 1 ) return MAX_UINTN;
	for ( Idx=Pos ; Idx-- > 0 ; ) {
		if ( m_data[Idx] == charToSearch ) return Idx;
	}
	return MAX_UINTN;
}

UINTN XStringW::RIdxOf(const XStringW &S, UINTN Pos) const
{
  UINTN i;
  UINTN Idx;

	if ( S.length() == 0 ) return MAX_UINTN;
	if ( Pos > length() ) Pos = length();
	if ( Pos < S.length() ) return MAX_UINTN;
	Pos -= S.length();
	for ( Idx=Pos+1 ; Idx-- > 0 ; ) {
		i = 0;
		while( i<S.length()  &&  data()[Idx+i] == S[i] ) i += 1;
		if ( i == S.length() ) return Idx;
	}
	return MAX_UINTN;
}


bool XStringW::IsDigits() const
{
  const wchar_t *p;

	p = data();
	if ( !*p ) return false;
	for ( ; *p ; p+=1 ) {
		if ( *p < '0' ) return false;
		if ( *p > '9' ) return false;
	}
	return true;
}

bool XStringW::IsDigits(UINTN pos, UINTN count) const
{
  const wchar_t *p;
  const wchar_t *q;

	if ( pos >= length() ) {
		return false;
	}
	if ( pos+count > length() ) {
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


void XStringW::RemoveLastEspCtrl()
{
  wchar_t *p;

	if ( length() > 0 ) {
		p = data() + length() - 1;
		if ( *p >= 0 && *p <= ' ' ) {
			p -= 1;
			while ( p>data() && *p >= 0 && *p <= ' ' ) p -= 1;
			if ( p>data() ) {
				SetLength( (UINTN)(p-data())+1);
			}else{
				if ( *p >= 0 && *p <= ' ' ) SetLength(0);
				else SetLength(1);
			}
		}
	}
}

//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------
//bool XStringW::ReadFromFILE(FILE *fp)
//{
//  UINTN longueur;
//
//	if ( fread(&longueur, sizeof(longueur), 1, fp) != 1 ) goto fin;
//	if ( longueur > 0  &&  fread(dataWithSizeMin(0, longueur), sizeof(wchar_t), longueur, fp) != 1 ) goto fin;
//	SetLength(longueur);
//	return true;
//  fin:
//	SetNull();
//	return false;
//}
//
//bool XStringW::WriteToFILE(FILE *fp) const
//{
//  UINTN longueur;
//
//	longueur = length();
//	if ( fwrite(&longueur, sizeof(longueur), 1, fp) != 1 ) return false;
//	if ( longueur > 0  &&  fwrite(data(), sizeof(wchar_t), longueur, fp) != 1 ) return false;
//	return true;
//}


//*************************************************************************************************
//
//                                       Operators =
//
//*************************************************************************************************

const XStringW &XStringW::operator =(wchar_t aChar)
{
//TRACE("Operator =wchar_t \n");
	StrnCpy(&aChar, 1);
	return *this;
}

const XStringW &XStringW::operator =(const XStringW &aString)
{
//TRACE("Operator =const XStringW&\n");
	StrnCpy(aString.data(), aString.length());
	return *this;
}

const XStringW &XStringW::operator =(const wchar_t *S)
{
//TRACE("Operator =const wchar_t *\n");
	StrCpy(S);
	return *this;
}



//*************************************************************************************************
//
//                                       Operators +=
//
//*************************************************************************************************

const XStringW &XStringW::operator +=(wchar_t aChar)
{
//TRACE("Operator +=wchar_t \n");
	StrnCat(&aChar, 1);
	return *this;
}

const XStringW &XStringW::operator +=(const XStringW &aString)
{
//TRACE("Operator +=const XStringW&\n");
	StrnCat(aString.data(), aString.length());
	return *this;
}

const XStringW &XStringW::operator +=(const wchar_t *S)
{
//TRACE("operator +=const wchar_t *\n");
	StrCat(S);
	return *this;
}


//-----------------------------------------------------------------------------
//                                 Functions
//-----------------------------------------------------------------------------

XStringW SPrintf(const wchar_t *format, ...)
{
  VA_LIST     va;
  XStringW str;

  VA_START (va, format);
  str.vSPrintf(format, va);
	VA_END(va);

  return str;
}

XStringW SubString(const wchar_t *S, UINTN pos, UINTN count)
{
	if ( StrLen(S)-pos < count ) count = StrLen(S)-pos;
	return ( XStringW(S+pos, count) );
}


XStringW CleanCtrl(const XStringW &S)
{
  XStringW ReturnValue;
  UINTN i;

	for ( i=0 ; i<S.length() ; i+=1 ) {
		if ( S[i] >=0  &&  S[i] < ' ' ) ReturnValue += 'x'; /* wchar_t are signed !!! */
		else ReturnValue += S[i];
	}
	return ReturnValue;
}


#endif
