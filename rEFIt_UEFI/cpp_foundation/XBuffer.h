/*
 *
 * Created by jief in 1997.
 * Copyright (c) 2020 Jief
 * All rights reserved.
 *
 */

#if !defined(__XBUFFER_H__)
#define __XBUFFER_H__

#include <XToolsConf.h>
#include "XToolsCommon.h"

#include "XRBuffer.h"
#include "XString.h"


#define XBuffer_Super XRBuffer<T>
template <typename T>
class XBuffer : public XBuffer_Super
{
public:
  static XBuffer<T> NullXBuffer;

protected:
	T*_WData; // same as RData (see XRBuffer)
	size_t m_allocatedSize;

	void Initialize(const T* p, size_t count, size_t index);
  public:
	XBuffer() : _WData(NULL), m_allocatedSize(0) { Initialize(NULL, 0, 0); } // ": _WData(NULL), m_allocatedSize(0)" to avoid effc++ warning

  XBuffer(const XBuffer<T>& aBuffer) : _WData(NULL), m_allocatedSize(0) { Initialize(aBuffer.data(), aBuffer.size(), aBuffer.index()); }
  XBuffer(XRBuffer<T> &aBuffer, size_t pos = 0, size_t count = MAX_XSIZE);
//	XBuffer(XBuffer &aBuffer, size_t pos = 0, size_t count = MAX_XSIZE);
	XBuffer(const void *p, size_t count);
	const XBuffer &operator =(const XRBuffer<T> &aBuffer);
	const XBuffer &operator =(const XBuffer &aBuffer);

  template<typename IntegralType, enable_if(is_integral(IntegralType))>
  void stealValueFrom(T* p, IntegralType count) {
    if ( count < 0 ) {
#ifdef DEBUG
      panic("XBuffer::stealValueFrom : count < 0. System halted\n");
#else
      return;
#endif
    }
    if( _WData ) free(_WData);
    m_allocatedSize = count;
    XRBuffer<T>::_RData = _WData = p;
    XRBuffer<T>::m_size = count;
    XRBuffer<T>::_Index = 0;
  }

  template<typename IntegralType, enable_if(is_integral(IntegralType))>
  void stealValueFrom(T* p, IntegralType count, IntegralType allocatedSize) {
    if ( count < 0 ) {
      panic("XBuffer::stealValueFrom : count < 0. System halted\n");
    }
    if ( allocatedSize < count ) {
      panic("XBuffer::stealValueFrom : allocatedSize < count. System halted\n");
    }
    if( _WData ) free(_WData);
    m_allocatedSize = allocatedSize;
    XRBuffer<T>::_RData = _WData = p;
    XRBuffer<T>::m_size = count;
    XRBuffer<T>::_Index = 0;
  }
	~XBuffer();

public:

	void CheckAllocatedSize(size_t nNewSize, size_t nGrowBy = XBufferGrowByDefault);

  void* vdata() const { return (void*)XBuffer_Super::data(); }
  const T* data() const { return _WData; }
  T* data() { return _WData; }

  template<typename IntegralType, enable_if(is_integral(IntegralType))>
  const T* data(IntegralType i) const { return XBuffer_Super::data(i); }
  template<typename IntegralType, enable_if(is_integral(IntegralType))>
  T* data(IntegralType i) { return _WData + i; }

  template<typename IntegralType, enable_if(is_integral(IntegralType))>
  T* dataSized(IntegralType size) {
    if ( size < 0 ) {
#ifdef DEBUG
      panic("XBuffer::dataSized : size < 0. System halted\n");
#else
      return NULL;
#endif
    }
    CheckAllocatedSize(size, 0);
    return data();
  }

//	void *Data(size_t ui=0) { return _WData+ui; }
//	void *DataWithSizeMin(size_t ui, size_t size, size_t nGrowBy=XBufferGrowByDefault) { CheckAllocatedSize(size, nGrowBy); return Data(ui); }
//
//	char *CData(size_t ui=0) { return (char *)(_WData+ui); }
//	char *CDataWithSizeMin(size_t ui, size_t size, size_t nGrowBy=XBufferGrowByDefault) { CheckAllocatedSize(size, nGrowBy); return CData(ui); }
//
//	unsigned char *UCData(size_t ui=0) { return _WData+ui; }
//	void *UCDataWithSizeMin(size_t ui, unsigned int size, size_t nGrowBy=XBufferGrowByDefault) { CheckAllocatedSize(size, nGrowBy); return UCData(ui); }

	size_t size() const { return XRBuffer<T>::size(); }

  template<typename IntegralType, enable_if(is_integral(IntegralType))>
	void __setSize(IntegralType size) {
    if ( size<0 ) { log_technical_bug("XBuffer::setSize() -> i < 0"); return; }
    if ( (unsigned_type(IntegralType))size > MAX_XSIZE ) { log_technical_bug("XBuffer::setSize() -> i > MAX_XSIZE"); return; }
    CheckAllocatedSize((unsigned_type(IntegralType))size);
    XBuffer_Super::m_size = (unsigned_type(IntegralType))size;
  };

  /* add value 0 until size is reached */
  template<typename IntegralType, enable_if(is_integral(IntegralType))>
  void setSize(IntegralType size, const T& elementToCopy) {
    if ( size<0 ) { log_technical_bug("XBuffer::setSize() -> i < 0"); return; }
    unsigned_type(IntegralType) usize = (unsigned_type(IntegralType))size;
    if ( usize > MAX_XSIZE ) { log_technical_bug("XBuffer::setSize() -> i > MAX_XSIZE"); return; }
    CheckAllocatedSize(usize);
    for ( size_t idx = XBuffer_Super::m_size ; idx < usize ; ++idx ) {
      _WData[idx] = elementToCopy;
    }
    XBuffer_Super::m_size = usize;
  };


	void setEmpty() { __setSize(0); };

  bool operator == (const XBuffer<T>& other) const {
    if ( size() != other.size() ) return false;
    if ( size() == 0 ) return true;
    if ( memcmp(_WData, other._WData, size()) != 0 ) return false;
    return true;
  }
  bool operator != (const XBuffer<T>& other) const { return !(*this == other); }

  /* [] */
  template<typename IntegralType, enable_if(is_integral(IntegralType))>
  T& operator [](IntegralType i)
  {
//#ifdef DEBUG
    if (i < 0) panic("XBuffer::[] : i < 0. System halted\n");
    if ( (unsigned_type(IntegralType))i >= size() ) panic("XBuffer::[] : i > _Len. System halted\n");
//#else
//    // Cannot return 0, return value type is T, unknown at this stage.
//    if (i < 0) return 0;
//    if ( (unsigned_type(IntegralType))i >= size() ) return 0;
//#endif
    return _WData[(unsigned_type(IntegralType))i];
  }
  template<typename IntegralType, enable_if(is_integral(IntegralType))>
  const T& operator [](IntegralType i) const
  {
//#ifdef DEBUG
    if (i < 0) panic("XBuffer::[] : i < 0. System halted\n");
    if ( (unsigned_type(IntegralType))i >= size() ) panic("XBuffer::[] : i > _Len. System halted\n");
//#else
//    // Cannot return 0, return value type is T, unknown at this stage.
//    if (i < 0) return 0;
//    if ( (unsigned_type(IntegralType))i >= size() ) return 0;
//#endif
    return _WData[(unsigned_type(IntegralType))i];
  }

//  unsigned char& operator [](int i) { return UCData()[i]; } // underflow ? overflow ?
//	unsigned char& operator [](size_t i) { return UCData()[i]; }

  void memset(unsigned char c) {
    memset(_WData, c, size());
  }

  template<typename IntegralType, enable_if(is_integral(IntegralType))>
  void memset(unsigned char c, IntegralType count) {
#ifdef DEBUG
    if (count < 0) panic("XBuffer::memset : count < 0. System halted\n");
#else
    if (count < 0) return;
#endif
    if ( (unsigned_type(IntegralType))count >= size() ) __setSize(count);
    ::memset(_WData, c, count);
  }

  template<typename IntegralType1, typename IntegralType2, enable_if( is_integral(IntegralType1) && is_integral(IntegralType2) )>
  void memsetAtPos(IntegralType1 pos, unsigned char c, IntegralType2 count) {
#ifdef DEBUG
    if (pos < 0) panic("XBuffer::memset : pos < 0. System halted\n");
    if (count < 0) panic("XBuffer::memset : count < 0. System halted\n");
#else
    if (pos < 0) return;
    if (count < 0) return;
#endif
    if ( (size_t)pos + (unsigned_type(IntegralType2))count >= size() ) setSize((size_t)pos + (unsigned_type(IntegralType2))count);

    ::memset(_WData, c, count);
  }

	void ncpy(const void *buf, size_t len);
  void cpy(bool b) { ncpy(&b, sizeof(b)); };
  void cpy(char c) { ncpy(&c, sizeof(c)); };
  void cpy(unsigned char c) { ncpy(&c, sizeof(c)); };
  void cpy(signed char c) { ncpy(&c, sizeof(c)); };
  void cpy(signed short s) { ncpy(&s, sizeof(s)); };
  void cpy(unsigned short us) { ncpy(&us, sizeof(us)); };
  void cpy(signed int i) { ncpy(&i, sizeof(i)); };
  void cpy(unsigned int ui) { ncpy(&ui, sizeof(ui)); };
  void cpy(signed long l) { ncpy(&l, sizeof(l)); };
  void cpy(unsigned long ul) { ncpy(&ul, sizeof(ul)); };
  void cpy(signed long long ull) { ncpy(&ull, sizeof(ull)); };
  void cpy(unsigned long long ull) { ncpy(&ull, sizeof(ull)); };
  void cpy(float f) { ncpy(&f, sizeof(f)); };
  void cpy(double d) { ncpy(&d, sizeof(d)); };
  void cpy(void* p) { ncpy(&p, sizeof(p)); };
	// Cat
	void ncat(const void *buf, size_t len);
  void cat(bool b) { ncat(&b, sizeof(b)); };
  void cat(char c) { ncat(&c, sizeof(c)); };
  void cat(unsigned char c) { ncat(&c, sizeof(c)); };
  void cat(signed char c) { ncat(&c, sizeof(c)); };
  void cat(signed short s) { ncat(&s, sizeof(s)); };
  void cat(unsigned short us) { ncat(&us, sizeof(us)); };
  void cat(signed int i) { ncat(&i, sizeof(i)); };
	void cat(unsigned int ui) { ncat(&ui, sizeof(ui)); };
  void cat(signed long l) { ncat(&l, sizeof(l)); };
	void cat(unsigned long ul) { ncat(&ul, sizeof(ul)); };
  void cat(signed long long ull) { ncat(&ull, sizeof(ull)); };
  void cat(unsigned long long ull) { ncat(&ull, sizeof(ull)); };
  void cat(float f) { ncat(&f, sizeof(f)); };
  void cat(double d) { ncat(&d, sizeof(d)); };
  void cat(void* p) { ncat(&p, sizeof(p)); };

protected:
  static void transmitS8Printf(const char* buf, unsigned int nbchar, void* context)
  {
    ((XBuffer<T>*)(context))->ncat(buf, nbchar);
  }
public:
  void vS8Catf(const char* format, XTOOLS_VA_LIST va)
  {
    vprintf_with_callback(format, va, transmitS8Printf, this);
  }
  void S8Catf(const char* format, ...) __attribute__((__format__(__printf__, 2, 3)))
  {
    XTOOLS_VA_LIST     va;

    XTOOLS_VA_START (va, format);
    vS8Catf(format, va);
    XTOOLS_VA_END(va);
  }

  T* forgetDataWithoutFreeing()
  {
    T* ret = data();
    m_allocatedSize = 0;
    XRBuffer<T>::_RData = _WData = NULL;
    XRBuffer<T>::m_size = 0;
    XRBuffer<T>::_Index = 0;
    return ret;
  }


//	void cat(const XString8 &aXString8);
	void cat(const XBuffer &unXBuffer) { ncat(unXBuffer.Length()); ncat(unXBuffer.Data(), unXBuffer.Length()); }
	void deleteAtPos(unsigned int pos, unsigned int count=1);

	const XBuffer &operator += (const XRBuffer<T> &aBuffer);

	size_t Sizeof() const;
//	bool ReadFileName(const char* FileName, unsigned int AddZeros=0);
	bool ReadFromBuf(const char *buf, size_t *idx, size_t count);
	bool ReadFromXRBuffer(XRBuffer<T> &unXBuffer);
};

template <typename T>
XBuffer<T> XBuffer<T>::NullXBuffer = XBuffer<T>();



//*************************************************************************************************
template <typename T>
void XBuffer<T>::Initialize(const T* p, size_t count, size_t index)
{
  if ( p!=NULL && count>0 )
  {
    m_allocatedSize = count;
    _WData = (T*)malloc(m_allocatedSize);
    if ( !_WData ) {
#ifdef DEBUG
      panic("XBuffer<T>::Initialize(%zu) : malloc returned NULL. System halted\n", count);
#else
      return;
#endif
    }
    memcpy(_WData, p, count);
    XRBuffer<T>::_RData = _WData;
    XRBuffer<T>::m_size = count;
    XRBuffer<T>::_Index = index;
  }
  else{
    m_allocatedSize = 0;
    _WData = NULL;
    XRBuffer<T>::_RData = NULL;
    XRBuffer<T>::m_size = 0;
    XRBuffer<T>::_Index = 0;
  }
}

//-------------------------------------------------------------------------------------------------
//                                               CheckAllocatedSize
//-------------------------------------------------------------------------------------------------
template <typename T>
void XBuffer<T>::CheckAllocatedSize(size_t nNewSize, size_t nGrowBy)
{
  if ( m_allocatedSize < nNewSize )
  {
    nNewSize += nGrowBy;
    _WData = (T*)Xrealloc(_WData, nNewSize*sizeof(T), m_allocatedSize);
    if ( !_WData ) {
#ifdef DEBUG
      panic("XBuffer<T>::CheckAllocatedSize(%zu, %zu) : Xrealloc(%" PRIuPTR " %zu, %zu) returned NULL. System halted\n", nNewSize, nGrowBy, uintptr_t(_WData), nNewSize, m_allocatedSize);
#else
      m_allocatedSize = 0;
      return;
#endif
    }
    XRBuffer<T>::_RData = _WData;
    m_allocatedSize = nNewSize;
  }
}

//-------------------------------------------------------------------------------------------------
//                                               ctor
//-------------------------------------------------------------------------------------------------

template <typename T>
XBuffer<T>::XBuffer(XRBuffer<T> &aXRBuffer, size_t pos, size_t count) : _WData(NULL), m_allocatedSize(0) // initialisation to avoid effc++ warning
{
  if ( pos < aXRBuffer.Length() ) {
    Initialize(NULL, 0, 0);
  }else{
    if ( count > aXRBuffer.Length()-pos ) count = aXRBuffer.Length()-pos;
    Initialize(aXRBuffer.UCData(pos), count, aXRBuffer.Index());
  }
}
//
//template <typename T>
//XBuffer<T>::XBuffer(XBuffer &aXBuffer, size_t pos, size_t count) : _WData(NULL), m_allocatedSize(0) // initialisation to avoid effc++ warning
//{
//  if ( pos >= aXBuffer.Length() ) {
//    Initialize(NULL, 0, 0);
//  }else{
//    if ( count > aXBuffer.Length()-pos ) count = aXBuffer.Length()-pos;
//    Initialize(aXBuffer.UCData(pos), count, aXBuffer.Index());
//  }
//}

template <typename T>
XBuffer<T>::XBuffer(const void *p, size_t count) : _WData(NULL), m_allocatedSize(0) // initialisation to avoid effc++ warning
{
  Initialize((const unsigned char*)p, count, 0);
}

template <typename T>
XBuffer<T>::~XBuffer()
{
  free(_WData);
}

//-------------------------------------------------------------------------------------------------
//                                               operator =, +=
//-------------------------------------------------------------------------------------------------
template <typename T>
const XBuffer<T>& XBuffer<T>::operator =(const XRBuffer<T> &aBuffer)
{
//TRACE("Operator =const XBuffer&\n");
  Cpy(aBuffer.Data(), aBuffer.Length());
  SetIndex(aBuffer.Index());
  return *this;
}

template <typename T>
const XBuffer<T>& XBuffer<T>::operator =(const XBuffer<T> &aBuffer)
{
//TRACE("Operator =const XBuffer&\n");
  ncpy(aBuffer.data(), aBuffer.size());
  XBuffer_Super::setIndex(aBuffer.index());
  return *this;
}

template <typename T>
const XBuffer<T> &XBuffer<T>::operator +=(const XRBuffer<T> &aBuffer)
{
//TRACE("Operator +=const XBuffer&\n");
  Cat(aBuffer.Data(), aBuffer.Length());
  return *this;
}

//-------------------------------------------------------------------------------------------------
//                                               Cpy, Cat, Delete
//-------------------------------------------------------------------------------------------------
template <typename T>
void XBuffer<T>::ncpy(const void *buf, size_t len)
{
  if ( buf && len > 0 ) {
    __setSize(len);
    memcpy(data(), buf, len);
  }
}

template <typename T>
void XBuffer<T>::ncat(const void *buf, size_t len)
{
  if ( buf && len > 0 ) {
    CheckAllocatedSize(size()+len);
    memcpy(data(size()), buf, len);
    __setSize(size()+len);
  }
}
//
//template <typename T>
//void XBuffer<T>::cat(const XString8 &aXString8)
//{
//  cat(aXString8.sizeInBytes());
//  cat(aXString8.s(), aXString8.sizeInBytes());
//};

template <typename T>
void XBuffer<T>::deleteAtPos(unsigned int pos, unsigned int count)
{
  if ( pos < size() ) {
    if ( size() - pos <= count ) {
      memmove( _WData+pos, _WData+pos+count, size()-pos-count);
      __setSize(size()-count);
    }else{
      __setSize(pos);
    }
  }
}

//-------------------------------------------------------------------------------------------------
//                                              ReadFrom
//-------------------------------------------------------------------------------------------------
//bool XBuffer<T>::ReadFileName(const char* FileName, unsigned int AddZeros)
//{
//  FILE *fp;
//  long int ulen;
//  unsigned int len;
//
//  fp = fopen(FileName, "rb");
//  if ( fp == NULL ) {
//    SetLastErrorMsgf("Impossible d'ouvrir le fichier en lecture '%s'", FileName);
//    goto finally;
//  }
//
//  fseek(fp, 0, 2); // Seek to end of file
//  ulen = ftell(fp);
//  if ( ulen <= 0 ) {
//    SetLastErrorMsgf("Impossible d'avoir la longueur du fichier '%s'", FileName);
//    goto finally;
//  }
//
//  len = (unsigned int)ulen;
//  fseek(fp, 0, 0);
//
//  if ( fread(CDataWithSizeMin(0, len+AddZeros), 1, len, fp) != len ) {
//    SetLastErrorMsgf("Impossible de lire %d octets du fichier '%s'", len, FileName);
//    goto finally;
//  }
//  SetLength(len);
//  SetIndex(0);
//  if ( fclose(fp) != 0 ) {
//    SetLastErrorMsgf("Impossible de fermer les fichier '%s'", FileName);
//    goto finally;
//  }
//  {for ( unsigned int ui=len ; ui<len+AddZeros ; ui+=1 ) *CData(ui) = 0;}
//  return YES;
//  finally:
//  if ( fp != NULL ) fclose(fp);
//  SetLength(0);
//  return false;
//
//}

//bool XBuffer<T>::ReadFromFILE(FILE *fp)
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

template <typename T>
bool XBuffer<T>::ReadFromXRBuffer(XRBuffer<T> &unXBuffer)
{
  size_t longueur;
  
  if ( !unXBuffer.GetSize_t(&longueur) ) return false;
  if ( !unXBuffer.Get(dataSized(longueur), longueur) ) return false; // GrowBy (param 3 of DataWithSizeMin is 0 to avoid memory waste
  setSize(longueur);
  XBuffer_Super::setIndex(0);
  return true;
}

template <typename T>
bool XBuffer<T>::ReadFromBuf(const char *buf, size_t *idx, size_t count)
{
  size_t longueur;

  if ( count-*idx >= sizeof(longueur) ) {
    longueur = *((size_t *)(buf+*idx));
    *idx += sizeof(longueur);
    if (  longueur > 0  &&  count-*idx>=longueur  ) memcpy(dataSized(longueur), buf+*idx, longueur);
    *idx += longueur;
    setSize(longueur);
    XBuffer_Super::setIndex(0);
    return true;
  }else{
    setEmpty();
    return false;
  }
}

#endif
