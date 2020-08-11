//*************************************************************************************************
//
//                                          Buffer
//
//*************************************************************************************************

#if !defined(__XBUFFER_CPP__)
#define __XBUFFER_CPP__

#include <XToolsConf.h>
#include "XToolsCommon.h"

#include "XBuffer.h"

//#include "../JiefDevTools/XConstString.h"
#include "XString.h"

const XBuffer NullXBuffer;

//*************************************************************************************************
void XBuffer::Initialize(const unsigned char* p, size_t count, size_t index)
{
	if ( p!=NULL && count>0 )
	{
		m_allocatedSize = count;
		_WData = (unsigned char*)malloc(m_allocatedSize);
		if ( !_WData ) {
		  panic("XBuffer::Initialize(%zu) : malloc returned NULL. System halted\n", count);
		}
		memcpy(_WData, p, count);
		_RData = _WData;
		_Len = count;
		_Index = index;
	}
	else{
		m_allocatedSize = 0;
		_WData = NULL;
		_RData = NULL;
		_Len = 0;
		_Index = 0;
	}
}

//-------------------------------------------------------------------------------------------------
//                                               CheckSize
//-------------------------------------------------------------------------------------------------
void XBuffer::CheckSize(size_t nNewSize, size_t nGrowBy)
{
  if ( m_allocatedSize < nNewSize )
  {
    nNewSize += nGrowBy;
    _WData = (unsigned char*)Xrealloc(_WData, nNewSize, m_allocatedSize);
    if ( !_WData ) {
      panic("XBuffer::CheckSize(%zu, %zu) : Xrealloc(%" PRIuPTR " %zu, %zu) returned NULL. System halted\n", nNewSize, nGrowBy, uintptr_t(_WData), nNewSize, m_allocatedSize);
    }
    _RData = _WData;
    m_allocatedSize = nNewSize;
  }
}

//-------------------------------------------------------------------------------------------------
//                                               ctor
//-------------------------------------------------------------------------------------------------

XBuffer::XBuffer() : _WData(NULL), m_allocatedSize(0) // initialisation to avoid effc++ warning
{
	Initialize(NULL, 0, 0);
}

XBuffer::XBuffer(XRBuffer &aXRBuffer, size_t pos, size_t count) : _WData(NULL), m_allocatedSize(0) // initialisation to avoid effc++ warning
{
	if ( pos < aXRBuffer.Length() ) {
		Initialize(NULL, 0, 0);
	}else{
		if ( count > aXRBuffer.Length()-pos ) count = aXRBuffer.Length()-pos;
		Initialize(aXRBuffer.UCData(pos), count, aXRBuffer.Index());
	}
}

XBuffer::XBuffer(XBuffer &aXBuffer, size_t pos, size_t count) : _WData(NULL), m_allocatedSize(0) // initialisation to avoid effc++ warning
{
	if ( pos >= aXBuffer.Length() ) {
		Initialize(NULL, 0, 0);
	}else{
		if ( count > aXBuffer.Length()-pos ) count = aXBuffer.Length()-pos;
		Initialize(aXBuffer.UCData(pos), count, aXBuffer.Index());
	}
}

XBuffer::XBuffer(void *p, size_t count) : _WData(NULL), m_allocatedSize(0) // initialisation to avoid effc++ warning
{
	Initialize((const unsigned char*)p, count, 0);
}

XBuffer::~XBuffer()
{
	free(_WData);
}

//-------------------------------------------------------------------------------------------------
//                                               operator =, +=
//-------------------------------------------------------------------------------------------------
const XBuffer &XBuffer::operator =(const XRBuffer &aBuffer)
{
//TRACE("Operator =const XBuffer&\n");
	Cpy(aBuffer.Data(), aBuffer.Length());
	SetIndex(aBuffer.Index());
	return *this;
}

const XBuffer &XBuffer::operator =(const XBuffer &aBuffer)
{
//TRACE("Operator =const XBuffer&\n");
	Cpy(aBuffer.Data(), aBuffer.Length());
	SetIndex(aBuffer.Index());
	return *this;
}

const XBuffer &XBuffer::operator +=(const XRBuffer &aBuffer)
{
//TRACE("Operator +=const XBuffer&\n");
	Cat(aBuffer.Data(), aBuffer.Length());
	return *this;
}

//-------------------------------------------------------------------------------------------------
//                                               Cpy, Cat, Delete
//-------------------------------------------------------------------------------------------------
void XBuffer::Cpy(const void *buf, size_t len)
{
	if ( buf && len > 0 ) {
		CheckSize(len, 0);         // GrowBy � 0
		memcpy(Data(), buf, len);
		SetLength(len);
	}
}

void XBuffer::Cat(const void *buf, size_t len)
{
	if ( buf && len > 0 ) {
		CheckSize(Length()+len);
		memcpy(Data(Length()), buf, len);
		SetLength(Length()+len);
	}
}

void XBuffer::Cat(const XString8 &aXString8)
{
	Cat(aXString8.sizeInBytes());
	Cat(aXString8.data(),aXString8.sizeInBytes());
};

void XBuffer::Delete(unsigned int pos, unsigned int count)
{
	if ( pos < Length() ) {
		if ( pos + count < Length() ) {
			memmove( _WData+pos, _WData+pos+count, Length()-pos-count);
			SetLength(Length()-count);
		}else{
			SetLength(pos);
		}
	}
}

//-------------------------------------------------------------------------------------------------
//                                              ReadFrom
//-------------------------------------------------------------------------------------------------
//bool XBuffer::ReadFileName(const char* FileName, unsigned int AddZeros)
//{
//  FILE *fp;
//  long int ulen;
//  unsigned int len;
//
//	fp = fopen(FileName, "rb");
//	if ( fp == NULL ) {
//		SetLastErrorMsgf("Impossible d'ouvrir le fichier en lecture '%s'", FileName);
//		goto finally;
//	}
//
//	fseek(fp, 0, 2); // Seek to end of file
//	ulen = ftell(fp);
//	if ( ulen <= 0 ) {
//		SetLastErrorMsgf("Impossible d'avoir la longueur du fichier '%s'", FileName);
//		goto finally;
//	}
//
//	len = (unsigned int)ulen;
//	fseek(fp, 0, 0);
//
//	if ( fread(CDataWithSizeMin(0, len+AddZeros), 1, len, fp) != len ) {
//		SetLastErrorMsgf("Impossible de lire %d octets du fichier '%s'", len, FileName);
//		goto finally;
//	}
//	SetLength(len);
//	SetIndex(0);
//	if ( fclose(fp) != 0 ) {
//		SetLastErrorMsgf("Impossible de fermer les fichier '%s'", FileName);
//		goto finally;
//	}
//	{for ( unsigned int ui=len ; ui<len+AddZeros ; ui+=1 ) *CData(ui) = 0;}
//	return YES;
//  finally:
//	if ( fp != NULL ) fclose(fp);
//	SetLength(0);
//	return false;
//
//}

//bool XBuffer::ReadFromFILE(FILE *fp)
//{
//  unsigned int longueur;
//
//  if ( fread(&longueur, sizeof(longueur), 1, fp) != 1 ) goto fin;
//  if ( longueur > 0  &&  fread(DataWithSizeMin(0, longueur, 0), longueur, 1, fp) != 1 ) goto fin;
//  SetLength(longueur);
//  SetIndex(0);
//  return OUI;
//  fin:
//  SetNull();
//  return NON;
//}

bool XBuffer::ReadFromXBuffer(XRBuffer &unXBuffer)
{
  size_t longueur;
	
	if ( !unXBuffer.GetSize_t(&longueur) ) return false;
	if ( !unXBuffer.Get(DataWithSizeMin(0, longueur, 0), longueur) ) return false; // GrowBy (param 3 de DataWithSizeMin est � 0 pour eviter du gachis m�moire
	SetLength(longueur);
	SetIndex(0);
	return true;
}

bool XBuffer::ReadFromBuf(const char *buf, size_t *idx, size_t count)
{
  size_t longueur;

	if ( count-*idx >= sizeof(longueur) ) {
		longueur = *((size_t *)(buf+*idx));
		*idx += sizeof(longueur);
		if (  longueur > 0  &&  count-*idx>=longueur  ) memcpy(DataWithSizeMin(0, longueur, 0), buf+*idx, longueur);
		*idx += longueur;
		SetLength(longueur);
		SetIndex(0);
		return true;
	}else{
		SetNull();
		return false;
	}
}

#ifdef __DEVTOOLS_SOCKETS__
void XBuffer::ReadFromSOCKETT(SOCKET Sock, unsigned int LenMax, unsigned int TO, unsigned int AddZeros, const char *ErrMsg)
{
  unsigned int longueur;

	SockReceiveT(Sock, &longueur, sizeof(longueur), TO, ErrMsg);
	if ( longueur > LenMax ) Throw("Longueur re�ue (%d) sup�rieure � la longueur max (%d)", longueur, LenMax);
	if ( longueur > 0 ) SockReceiveT(Sock, DataWithSizeMin(0, longueur+AddZeros, 0), longueur, TO, ErrMsg);
	SetLength(longueur);
	{for ( unsigned int ui=longueur ; ui<longueur+AddZeros ; ui+=1 ) *CData(ui) = 0;}
	SetIndex(0);
}

bool XBuffer::ReadFromSOCKET(SOCKET Sock, unsigned int LenMax, unsigned int TO, unsigned int AddZeros, const char *ErrMsg)
{
	try
	{
		ReadFromSOCKETT(Sock, LenMax, TO, AddZeros, ErrMsg);
		SetLastErrorFlag(NON);
	}
	StdCatch();
	return !LastErrorFlag();
}
#endif

#endif
