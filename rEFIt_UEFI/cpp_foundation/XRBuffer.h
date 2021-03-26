//*************************************************************************************************
//*************************************************************************************************
//
//                                          RBUFFER
//
//*************************************************************************************************
//*************************************************************************************************

#if !defined(__XRBUFFER_H__)
#define __XRBUFFER_H__

#include <XToolsConf.h>
#include "XToolsCommon.h"


class XStringW;
class XString8;
class XString8Array;

template <typename T>
class XRBuffer
{
  protected:
	const T* _RData;
	size_t m_size;
	size_t _Index;

  public:
	void Initialize(const void *p, size_t count);
  XRBuffer() : _RData(0), m_size(0), _Index(0) { }
	XRBuffer(const T* p, size_t count) : _RData(p), m_size(count), _Index(0) { }
	XRBuffer(const XRBuffer& aXRBuffer, size_t pos = 0, size_t count = MAX_XSIZE);
//  {
//    if ( pos < aXRBuffer.Length() ) {
//      _Len = count;
//      if ( _Len > aXRBuffer.Length()-pos ) _Len = aXRBuffer.Length()-pos;
//      _RData = (unsigned char*)aXRBuffer.Data(pos);
//    }
//  }


//	XRBuffer(const XBuffer &aBuffer, unsigned int pos = 0, unsigned int count = MAX_XSIZE);

  public:

	const T* data() const { return _RData; }

	const char *CData(size_t ui=0) const { return (char *)(_RData+ui); }
	const unsigned char *UCData(size_t ui=0) const { return (unsigned char*)(_RData+ui); }

	size_t Sizeof() const;

	size_t size() const { return m_size; }
	size_t index() const { return _Index; }
 
  bool operator == (const XRBuffer<T>& other) const {
    if ( m_size != other.m_size ) return false;
    if ( m_size == 0 ) return true;
    if ( memcmp(_RData, other._RData, m_size) != 0 ) return false;
    return true;
  }
  bool operator != (const XRBuffer<T>& other) const { return !(*this == other); }

  template<typename IntegralType, enable_if(is_integral(IntegralType))>
	void setIndex(IntegralType Idx)
  {
    if (Idx < 0) {
#ifdef DEBUG
      panic("XBuffer::setIndex : Idx < 0. System halted\n");
#else
      _Index = 0;
#endif
    }
    _Index = Idx;
  }

	bool isEmpty() const { return m_size == 0 ; }
	bool notEmpty() const { return m_size > 0 ; }

	// Cast //
  //  Automatic cast are a bit dangerous...
  //	operator const char *() const { return CData(); }

	// [] //
  template<typename IntegralType, enable_if(is_integral(IntegralType))>
	T& operator [](IntegralType i)
	{
    if (i < 0) {
#ifdef DEBUG
      panic("XRBuffer::operator [] : i < 0. System halted\n");
#else
      return 0;
#endif
    }
    if ( (unsigned_type(IntegralType))i >= m_size ) {
#ifdef DEBUG
      panic("XRBuffer::operator [] : index >= m_size. System halted\n");
#else
      return 0;
#endif
    }
    return _RData[i];
  } // underflow ? overflow ?
  template<typename IntegralType, enable_if(is_integral(IntegralType))>
  const T& operator [](IntegralType i) const
  {
    if (i < 0) {
#ifdef DEBUG
      panic("XRBuffer::operator [] : i < 0. System halted\n");
#else
      return 0;
#endif
   }
    if ( (unsigned_type(IntegralType))i >= m_size ) {
#ifdef DEBUG
      panic("XRBuffer::operator [] : index > len. System halted\n");
#else
      return 0;
#endif
    }
    return _RData[i];
  } // underflow ? overflow ?

	bool Get(void *buf, size_t count);

  bool Get(bool *b) { return Get(b, sizeof(*b)); }

	bool Get(char *c) { return Get(c, sizeof(*c)); }
	bool Get(unsigned char *b) { return Get(b, sizeof(*b)); }
	bool Get(short *s) { return Get(s, sizeof(*s)); }
	bool Get(unsigned short *us) { return Get(us, sizeof(*us)); }
	bool Get(int *i) { return Get(i, sizeof(*i)); };
	bool Get(unsigned int *ui) { return Get(ui, sizeof(*ui)); };
  bool Get(long *l) { return Get(l, sizeof(*l)); };
  bool Get(unsigned long *ul) { return Get(ul, sizeof(*ul)); };
  bool Get(long long *ll) { return Get(ll, sizeof(*ll)); };
  bool Get(unsigned long long *ull) { return Get(ull, sizeof(*ull)); };

  bool Get(float *f) { return Get(f, sizeof(*f)); };
  bool Get(double *d) { return Get(d, sizeof(*d)); };

  bool GetBool(bool *b) { return Get(b, sizeof(*b)); }
	bool GetChar(char *c) { return Get(c, sizeof(*c)); }
	bool GetByte(unsigned char *b) { return Get(b, sizeof(*b)); }
	bool GetUShort(unsigned short *us) { return Get(us, sizeof(*us)); }
	bool GetUInt(unsigned int *ui) { return Get(ui, sizeof(*ui)); };
  bool GetULong(unsigned long *ul) { return Get(ul, sizeof(*ul)); };
  bool GetULongLong(unsigned long long *ull) { return Get(ull, sizeof(*ull)); };

  bool GetSize_t(size_t *size) { return Get(size, sizeof(*size)); };

	size_t IdxOf(const XString8& aXString8, size_t Pos = 0) const;
	size_t IdxOf(const XString8Array& aXString8Array, size_t Pos = 0, size_t *n = NULL) const;

//	bool WriteToBuf(char *buf, size_t *idx, size_t count) const;
//	void WriteToFileNameT(const char* FileName) const;
//	bool WriteToFileName(const char* FileName) const;

//	bool WriteToXFILE(XFILE *f) const;
};

#endif
