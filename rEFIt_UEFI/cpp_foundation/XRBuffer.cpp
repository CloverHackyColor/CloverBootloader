//*************************************************************************************************
//
//                                          Buffer
//
//*************************************************************************************************

#if !defined(__XRBUFFER_CPP__)
#define __XRBUFFER_CPP__

#include <XToolsConf.h>
#include "XToolsCommon.h"

#include "XRBuffer.h"

#include "XString.h"
#include "XStringArray.h"

const XRBuffer NullXRBuffer;


//*************************************************************************************************
//
//                                          RBuffer (ConstBuffer)
//
//*************************************************************************************************
void XRBuffer::Initialize(const void *p, size_t count)
{
	_RData = (unsigned char*)p;
	_Len = count;
	_Index = 0;
}

XRBuffer::XRBuffer(const XRBuffer &aXRBuffer, size_t pos, size_t count) : _RData(0), _Len(0), _Index(0)
{
	if ( pos < aXRBuffer.Length() ) {
    _Len = count;
		if ( _Len > aXRBuffer.Length()-pos ) _Len = aXRBuffer.Length()-pos;
		_RData = (unsigned char*)aXRBuffer.Data(pos);
	}
}
/*
XRBuffer::XRBuffer(const XBuffer &aXBuffer, size_t pos, size_t count)
{
	if ( count > aXBuffer.Length() ) count = aXBuffer.Length();
	_Data = aXBuffer.UCData();
	_Len = count;
	_Index = 0;
}
*/

//-------------------------------------------------------------------------------------------------
//                                               
//-------------------------------------------------------------------------------------------------
bool XRBuffer::Get(void *buf, size_t count)
{
	if ( Length() - Index() >= count ) {
		memcpy(buf, Data(Index()), count);
		SetIndex(Index()+count);
		return true;
	}
	return false;
}

size_t XRBuffer::IdxOf(const XString8& aXString8, size_t Pos) const
{
	if ( aXString8.length() > Length()-Pos ) return MAX_XSIZE;
	size_t nb = Length()-aXString8.length()+1;
	for ( size_t ui=Pos ; ui<nb ; ui+=1 ) {
		if ( strncmp(CData(ui), aXString8.c_str(), aXString8.sizeInBytes()) == 0 ) return ui;
	}
	return  MAX_XSIZE;
}

size_t XRBuffer::IdxOf(const XString8Array& aXString8Array, size_t Pos, size_t *n) const
{
  size_t pos;

	for (size_t ui=0 ; ui<aXString8Array.size() ; ui+=1 ) {
		pos = IdxOf(aXString8Array[ui], Pos);
		if ( pos != MAX_XSIZE ) {
			if ( n != NULL ) *n = ui;
			return pos;
		}
	}
	return MAX_XSIZE;
}

size_t XRBuffer::Sizeof() const
{
	return sizeof(unsigned int)+Length();
}
/*
bool XRBuffer::WriteToBuf(char *buf, size_t *idx, size_t count) const
{
  unsigned int longueur;

	if ( count-*idx < sizeof(longueur) + Length() ) return NON;
	longueur = Length();
	memcpy(buf+*idx, &longueur, sizeof(longueur));
	*idx += sizeof(longueur);
	memcpy(buf+*idx, _Data, Length());
	*idx += Length();
	return OUI;
}
*/
//void XRBuffer::WriteToFileNameT(const char* FileName) const
//{
//  XFILE f;
//
//	f.OpenT(FileName, "wb");
//	f.WriteT(Data(), Length());
//	f.CloseT();
//}
//
//bool XRBuffer::WriteToFileName(const char* FileName) const
//{
//	try {
//		WriteToFileNameT(FileName);
//		SetLastErrorFlag(NON);
//	}
//	StdCatch();
//	return !LastErrorFlag();
//}
//
//bool XRBuffer::WriteToFILE(FILE *fp) const
//{
//  size_t longueur;
//
//	longueur = Length();
//	if ( fwrite(&longueur, sizeof(longueur), 1, fp) != 1 ) return NON;
//	if ( longueur > 0  &&  fwrite(Data(), longueur, 1, fp) != 1 ) return NON;
//	return OUI;
//}
//
//void XRBuffer::WriteToXFILET(XFILE *f) const
//{
//  size_t longueur;
//
//	longueur = Length();
//	f->WriteT(&longueur, sizeof(longueur));
//	if ( longueur > 0 ) f->WriteT(Data(), longueur);
//}
//
//bool XRBuffer::WriteToXFILE(XFILE *f) const
//{
//	try {
//		WriteToXFILET(f);
//		SetLastErrorFlag(NON);
//	}
//	StdCatch();
//	return !LastErrorFlag();
//}

#endif
