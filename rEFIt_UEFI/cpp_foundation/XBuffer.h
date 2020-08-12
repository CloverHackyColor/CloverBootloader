//*************************************************************************************************
//*************************************************************************************************
//
//                                          BUFFER
//
//*************************************************************************************************
//*************************************************************************************************
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
  protected:
	T*_WData; // same as RData (see XRBuffer)
	size_t m_allocatedSize;

	void Initialize(T* p, size_t count, size_t index);
  public:
	XBuffer() : _WData(NULL), m_allocatedSize(0) { Initialize(NULL, 0, 0); } // ": _WData(NULL), m_allocatedSize(0)" to avoid effc++ warning

  XBuffer(const XBuffer<T>& aBuffer) : _WData(NULL), m_allocatedSize(0) { Initialize(aBuffer.data(), aBuffer.size(), aBuffer.index()); }
  XBuffer(XRBuffer<T> &aBuffer, size_t pos = 0, size_t count = MAX_XSIZE);
//	XBuffer(XBuffer &aBuffer, size_t pos = 0, size_t count = MAX_XSIZE);
	XBuffer(void *p, size_t count);
	const XBuffer &operator =(const XRBuffer<T> &aBuffer);
	const XBuffer &operator =(const XBuffer &aBuffer);

  template<typename IntegralType, enable_if(is_integral(IntegralType))>
	void stealValueFrom(T* p, IntegralType count) {
    if ( count < 0 ) {
      panic("XBuffer::stealValueFrom : count < 0. System halted\n");
    }
    if( _WData ) free(_WData);
    Initialize(p, count, 0);
  }


	~XBuffer();

  public:
	void CheckSize(size_t nNewSize, size_t nGrowBy = XBufferGrowByDefault);

  void* vdata() const { return XBuffer_Super::data(); }
  T* data() const { return _WData; }

  template<typename IntegralType, enable_if(is_integral(IntegralType))>
  const T* data(IntegralType i) const { return XBuffer_Super::data(i); }

  template<typename IntegralType, enable_if(is_integral(IntegralType))>
  T* dataSized(IntegralType size) {
    if ( size < 0 ) {
      panic("XBuffer::dataSized : size < 0. System halted\n");
    }
    CheckSize(size, 0);
    return data();
  }

//	void *Data(size_t ui=0) { return _WData+ui; }
//	void *DataWithSizeMin(size_t ui, size_t size, size_t nGrowBy=XBufferGrowByDefault) { CheckSize(size, nGrowBy); return Data(ui); }
//
//	char *CData(size_t ui=0) { return (char *)(_WData+ui); }
//	char *CDataWithSizeMin(size_t ui, size_t size, size_t nGrowBy=XBufferGrowByDefault) { CheckSize(size, nGrowBy); return CData(ui); }
//
//	unsigned char *UCData(size_t ui=0) { return _WData+ui; }
//	void *UCDataWithSizeMin(size_t ui, unsigned int size, size_t nGrowBy=XBufferGrowByDefault) { CheckSize(size, nGrowBy); return UCData(ui); }

	size_t size() const { return XRBuffer<T>::size(); }

  template<typename IntegralType, enable_if(is_integral(IntegralType))>
	void setSize(IntegralType size) { CheckSize(size); XBuffer_Super::m_size = size; };
	void setEmpty() { setSize(0); };

  /* [] */
  template<typename IntegralType, enable_if(is_integral(IntegralType))>
  T& operator [](IntegralType i)
  {
    if (i < 0) panic("XBuffer::[] : i < 0. System halted\n");
    if ( (unsigned_type(IntegralType))i >= size() ) panic("XBuffer::[] : i > _Len. System halted\n");
    return _WData[(unsigned_type(IntegralType))i];
  }

//  unsigned char& operator [](int i) { return UCData()[i]; } // underflow ? overflow ?
//	unsigned char& operator [](size_t i) { return UCData()[i]; }

  void memset(unsigned char c) {
    memset(_WData, c, size());
  }

  template<typename IntegralType, enable_if(is_integral(IntegralType))>
  void memset(unsigned char c, IntegralType count) {
    if (count < 0) panic("XBuffer::memset : count < 0. System halted\n");
    if ( (unsigned_type(IntegralType))count >= size() ) setSize(count);
    ::memset(_WData, c, count);
  }

  template<typename IntegralType1, typename IntegralType2, enable_if( is_integral(IntegralType1) && is_integral(IntegralType2) )>
  void memsetAtPos(IntegralType1 pos, unsigned char c, IntegralType2 count) {
    if (pos < 0) panic("XBuffer::memset : pos < 0. System halted\n");
    if (count < 0) panic("XBuffer::memset : count < 0. System halted\n");
    if ( (size_t)pos + (unsigned_type(IntegralType2))count >= size() ) setSize((size_t)pos + (unsigned_type(IntegralType2))count);
    ::memset(_WData, c, count);
  }

	void ncpy(const void *buf, size_t len);
	// Cat
	void Cat(const void *buf, size_t len);

  void Cat(bool b) { Cat(&b, sizeof(b)); };

  void Cat(char c) { Cat(&c, sizeof(c)); };
	void Cat(unsigned char b) { Cat(&b, sizeof(b)); };
	void Cat(short s) { Cat(&s, sizeof(s)); };
	void Cat(unsigned short us) { Cat(&us, sizeof(us)); };
	void Cat(int i) { Cat(&i, sizeof(i)); };
	void Cat(unsigned int ui) { Cat(&ui, sizeof(ui)); };
	void Cat(long l) { Cat(&l, sizeof(l)); };
	void Cat(unsigned long ul) { Cat(&ul, sizeof(ul)); };
	void Cat(unsigned long long ull) { Cat(&ull, sizeof(ull)); };

  void Cat(float f) { Cat(&f, sizeof(f)); };
  void Cat(double d) { Cat(&d, sizeof(d)); };

	void Cat(const XString8 &aXString8);
	void Cat(const XBuffer &unXBuffer) { Cat(unXBuffer.Length()); Cat(unXBuffer.Data(), unXBuffer.Length()); }
	void Delete(unsigned int pos, unsigned int count=1);

	const XBuffer &operator += (const XRBuffer<T> &aBuffer);

	size_t Sizeof() const;
//	bool ReadFileName(const char* FileName, unsigned int AddZeros=0);
	bool ReadFromBuf(const char *buf, size_t *idx, size_t count);
	bool ReadFromXRBuffer(XRBuffer<T> &unXBuffer);
};




//*************************************************************************************************
template <typename T>
void XBuffer<T>::Initialize(T* p, size_t count, size_t index)
{
  if ( p!=NULL && count>0 )
  {
    m_allocatedSize = count;
    _WData = (unsigned char*)malloc(m_allocatedSize);
    if ( !_WData ) {
      panic("XBuffer<T>::Initialize(%zu) : malloc returned NULL. System halted\n", count);
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
//                                               CheckSize
//-------------------------------------------------------------------------------------------------
template <typename T>
void XBuffer<T>::CheckSize(size_t nNewSize, size_t nGrowBy)
{
  if ( m_allocatedSize < nNewSize )
  {
    nNewSize += nGrowBy;
    _WData = (unsigned char*)Xrealloc(_WData, nNewSize, m_allocatedSize);
    if ( !_WData ) {
      panic("XBuffer<T>::CheckSize(%zu, %zu) : Xrealloc(%" PRIuPTR " %zu, %zu) returned NULL. System halted\n", nNewSize, nGrowBy, uintptr_t(_WData), nNewSize, m_allocatedSize);
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
XBuffer<T>::XBuffer(void *p, size_t count) : _WData(NULL), m_allocatedSize(0) // initialisation to avoid effc++ warning
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
    setSize(len);
    memcpy(data(), buf, len);
  }
}

template <typename T>
void XBuffer<T>::Cat(const void *buf, size_t len)
{
  if ( buf && len > 0 ) {
    CheckSize(size()+len);
    memcpy(Data(size()), buf, len);
    SetLength(size()+len);
  }
}

template <typename T>
void XBuffer<T>::Cat(const XString8 &aXString8)
{
  Cat(aXString8.sizeInBytes());
  Cat(aXString8.data(),aXString8.sizeInBytes());
};

template <typename T>
void XBuffer<T>::Delete(unsigned int pos, unsigned int count)
{
  if ( pos < size() ) {
    if ( pos + count < size() ) {
      memmove( _WData+pos, _WData+pos+count, size()-pos-count);
      setSize(size()-count);
    }else{
      setSize(pos);
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
