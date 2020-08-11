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
class XBuffer;

class XRBuffer
{
  protected:
	const unsigned char *_RData;
	size_t _Len;
	size_t _Index;

  public:
	void Initialize(const void *p, size_t count);
  XRBuffer() : _RData(0), _Len(0), _Index(0) { }
	XRBuffer(const void *p, size_t count) : _RData((const unsigned char *)p), _Len(count), _Index(0) { Initialize(p, count); }
	XRBuffer(const XRBuffer& aXRBuffer, size_t pos = 0, size_t count = MAX_XSIZE);


//	XRBuffer(const XBuffer &aBuffer, unsigned int pos = 0, unsigned int count = MAX_XSIZE);

  public:

	const void *Data(size_t ui=0) const { return _RData+ui; }
	const char *CData(size_t ui=0) const { return (char *)(_RData+ui); }
	const unsigned char *UCData(size_t ui=0) const { return _RData+ui; }

	size_t Sizeof() const;

	size_t Length() const { return _Len; }
	size_t Index() const { return _Index; }
	void SetIndex(size_t Idx) { _Index = Idx; };

	// IsNull ? //
	bool IsNull() const { return Length() == 0 ; }
	bool NotNull() const { return Length() > 0 ; }

	// Cast //
	operator const char *() const { return CData(); }

	// [] //
  template<typename IntegralType, enable_if(is_integral(IntegralType))>
	unsigned char operator [](IntegralType i) const {
    if (i < 0) {
      panic("XRBuffer::operator [] : i < 0. System halted\n");
    }
    if ( (unsigned_type(IntegralType))i >= _Len ) {
      panic("XRBuffer::operator [] : index > len. System halted\n");
    }
    return UCData()[i];
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
