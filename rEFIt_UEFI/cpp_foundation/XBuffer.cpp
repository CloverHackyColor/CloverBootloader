////*************************************************************************************************
////
////                                          Buffer
////
////*************************************************************************************************
//
//#if !defined(__XBUFFER_CPP__)
//#define __XBUFFER_CPP__
//
//#include <XToolsConf.h>
//#include "XToolsCommon.h"
//
//#include "XRBuffer.h"
//#include "XBuffer.h"
//
////#include "../JiefDevTools/XConstString.h"
//#include "XString.h"
//
////const XBuffer NullXBuffer;
//
////*************************************************************************************************
//template <typename T>
//void XBuffer<T>::Initialize(T* p, size_t count, size_t index)
//{
//	if ( p!=NULL && count>0 )
//	{
//		m_allocatedSize = count;
//		_WData = (unsigned char*)malloc(m_allocatedSize);
//		if ( !_WData ) {
//		  panic("XBuffer<T>::Initialize(%zu) : malloc returned NULL. System halted\n", count);
//		}
//		memcpy(_WData, p, count);
//		XRBuffer<T>::_RData = _WData;
//		XRBuffer<T>::_Len = count;
//		XRBuffer<T>::_Index = index;
//	}
//	else{
//		m_allocatedSize = 0;
//		_WData = NULL;
//		XRBuffer<T>::_RData = NULL;
//		XRBuffer<T>::_Len = 0;
//		XRBuffer<T>::_Index = 0;
//	}
//}
//
////-------------------------------------------------------------------------------------------------
////                                               CheckSize
////-------------------------------------------------------------------------------------------------
//template <typename T>
//void XBuffer<T>::CheckSize(size_t nNewSize, size_t nGrowBy)
//{
//  if ( m_allocatedSize < nNewSize )
//  {
//    nNewSize += nGrowBy;
//    _WData = (unsigned char*)Xrealloc(_WData, nNewSize, m_allocatedSize);
//    if ( !_WData ) {
//      panic("XBuffer<T>::CheckSize(%zu, %zu) : Xrealloc(%" PRIuPTR " %zu, %zu) returned NULL. System halted\n", nNewSize, nGrowBy, uintptr_t(_WData), nNewSize, m_allocatedSize);
//    }
//    XRBuffer<T>::_RData = _WData;
//    m_allocatedSize = nNewSize;
//  }
//}
//
////-------------------------------------------------------------------------------------------------
////                                               ctor
////-------------------------------------------------------------------------------------------------
//
//template <typename T>
//XBuffer<T>::XBuffer() : _WData(NULL), m_allocatedSize(0) // initialisation to avoid effc++ warning
//{
//	Initialize(NULL, 0, 0);
//}
//
//template <typename T>
//XBuffer<T>::XBuffer(XRBuffer<T> &aXRBuffer, size_t pos, size_t count) : _WData(NULL), m_allocatedSize(0) // initialisation to avoid effc++ warning
//{
//	if ( pos < aXRBuffer.Length() ) {
//		Initialize(NULL, 0, 0);
//	}else{
//		if ( count > aXRBuffer.Length()-pos ) count = aXRBuffer.Length()-pos;
//		Initialize(aXRBuffer.UCData(pos), count, aXRBuffer.Index());
//	}
//}
//
//template <typename T>
//XBuffer<T>::XBuffer(XBuffer &aXBuffer, size_t pos, size_t count) : _WData(NULL), m_allocatedSize(0) // initialisation to avoid effc++ warning
//{
//	if ( pos >= aXBuffer.Length() ) {
//		Initialize(NULL, 0, 0);
//	}else{
//		if ( count > aXBuffer.Length()-pos ) count = aXBuffer.Length()-pos;
//		Initialize(aXBuffer.UCData(pos), count, aXBuffer.Index());
//	}
//}
//
//template <typename T>
//XBuffer<T>::XBuffer(void *p, size_t count) : _WData(NULL), m_allocatedSize(0) // initialisation to avoid effc++ warning
//{
//	Initialize((const unsigned char*)p, count, 0);
//}
//
//template <typename T>
//XBuffer<T>::~XBuffer()
//{
//	free(_WData);
//}
//
////-------------------------------------------------------------------------------------------------
////                                               operator =, +=
////-------------------------------------------------------------------------------------------------
//template <typename T>
//const XBuffer<T>& XBuffer<T>::operator =(const XRBuffer<T> &aBuffer)
//{
////TRACE("Operator =const XBuffer&\n");
//	Cpy(aBuffer.Data(), aBuffer.Length());
//	SetIndex(aBuffer.Index());
//	return *this;
//}
//
//template <typename T>
//const XBuffer<T>& XBuffer<T>::operator =(const XBuffer<T> &aBuffer)
//{
////TRACE("Operator =const XBuffer&\n");
//	Cpy(aBuffer.Data(), aBuffer.Length());
//	SetIndex(aBuffer.Index());
//	return *this;
//}
//
//template <typename T>
//const XBuffer<T> &XBuffer<T>::operator +=(const XRBuffer<T> &aBuffer)
//{
////TRACE("Operator +=const XBuffer&\n");
//	Cat(aBuffer.Data(), aBuffer.Length());
//	return *this;
//}
//
////-------------------------------------------------------------------------------------------------
////                                               Cpy, Cat, Delete
////-------------------------------------------------------------------------------------------------
//template <typename T>
//void XBuffer<T>::ncpy(const void *buf, size_t len)
//{
//	if ( buf && len > 0 ) {
//		CheckSize(len, 0);         // GrowBy 0
//		memcpy(data(), buf, len);
//		SetLength(len);
//	}
//}
//
//template <typename T>
//void XBuffer<T>::Cat(const void *buf, size_t len)
//{
//	if ( buf && len > 0 ) {
//		CheckSize(size()+len);
//		memcpy(Data(size()), buf, len);
//		SetLength(size()+len);
//	}
//}
//
//template <typename T>
//void XBuffer<T>::Cat(const XString8 &aXString8)
//{
//	Cat(aXString8.sizeInBytes());
//	Cat(aXString8.data(),aXString8.sizeInBytes());
//};
//
//template <typename T>
//void XBuffer<T>::Delete(unsigned int pos, unsigned int count)
//{
//	if ( pos < size() ) {
//		if ( pos + count < size() ) {
//			memmove( _WData+pos, _WData+pos+count, size()-pos-count);
//			SetLength(size()-count);
//		}else{
//			SetLength(pos);
//		}
//	}
//}
//
////-------------------------------------------------------------------------------------------------
////                                              ReadFrom
////-------------------------------------------------------------------------------------------------
////bool XBuffer<T>::ReadFileName(const char* FileName, unsigned int AddZeros)
////{
////  FILE *fp;
////  long int ulen;
////  unsigned int len;
////
////	fp = fopen(FileName, "rb");
////	if ( fp == NULL ) {
////		SetLastErrorMsgf("Impossible d'ouvrir le fichier en lecture '%s'", FileName);
////		goto finally;
////	}
////
////	fseek(fp, 0, 2); // Seek to end of file
////	ulen = ftell(fp);
////	if ( ulen <= 0 ) {
////		SetLastErrorMsgf("Impossible d'avoir la longueur du fichier '%s'", FileName);
////		goto finally;
////	}
////
////	len = (unsigned int)ulen;
////	fseek(fp, 0, 0);
////
////	if ( fread(CDataWithSizeMin(0, len+AddZeros), 1, len, fp) != len ) {
////		SetLastErrorMsgf("Impossible de lire %d octets du fichier '%s'", len, FileName);
////		goto finally;
////	}
////	SetLength(len);
////	SetIndex(0);
////	if ( fclose(fp) != 0 ) {
////		SetLastErrorMsgf("Impossible de fermer les fichier '%s'", FileName);
////		goto finally;
////	}
////	{for ( unsigned int ui=len ; ui<len+AddZeros ; ui+=1 ) *CData(ui) = 0;}
////	return YES;
////  finally:
////	if ( fp != NULL ) fclose(fp);
////	SetLength(0);
////	return false;
////
////}
//
////bool XBuffer<T>::ReadFromFILE(FILE *fp)
////{
////  unsigned int longueur;
////
////  if ( fread(&longueur, sizeof(longueur), 1, fp) != 1 ) goto fin;
////  if ( longueur > 0  &&  fread(DataWithSizeMin(0, longueur, 0), longueur, 1, fp) != 1 ) goto fin;
////  SetLength(longueur);
////  SetIndex(0);
////  return OUI;
////  fin:
////  SetNull();
////  return NON;
////}
//
//template <typename T>
//bool XBuffer<T>::ReadFromXRBuffer(XRBuffer<T> &unXBuffer)
//{
//  size_t longueur;
//	
//	if ( !unXBuffer.GetSize_t(&longueur) ) return false;
//	if ( !unXBuffer.Get(DataWithSizeMin(0, longueur, 0), longueur) ) return false; // GrowBy (param 3 of DataWithSizeMin is 0 to avoid memory waste
//	SetLength(longueur);
//	SetIndex(0);
//	return true;
//}
//
//template <typename T>
//bool XBuffer<T>::ReadFromBuf(const char *buf, size_t *idx, size_t count)
//{
//  size_t longueur;
//
//	if ( count-*idx >= sizeof(longueur) ) {
//		longueur = *((size_t *)(buf+*idx));
//		*idx += sizeof(longueur);
//		if (  longueur > 0  &&  count-*idx>=longueur  ) memcpy(DataWithSizeMin(0, longueur, 0), buf+*idx, longueur);
//		*idx += longueur;
//		SetLength(longueur);
//		SetIndex(0);
//		return true;
//	}else{
//		SetNull();
//		return false;
//	}
//}
//
//#endif
