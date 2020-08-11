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


#define XBuffer_Super XRBuffer
class XBuffer : public XBuffer_Super
{
  protected:
	unsigned char*_WData; // same as RData (see XRBuffer)
	size_t m_allocatedSize;

	void Initialize(const unsigned char* p, size_t count, size_t index);
  public:
	XBuffer();
	XBuffer(XRBuffer &aBuffer, size_t pos = 0, size_t count = MAX_XSIZE);
	XBuffer(XBuffer &aBuffer, size_t pos = 0, size_t count = MAX_XSIZE);
	XBuffer(void *p, size_t count);
	const XBuffer &operator =(const XRBuffer &aBuffer);
	const XBuffer &operator =(const XBuffer &aBuffer);

	~XBuffer();

  public:
	void CheckSize(size_t nNewSize, size_t nGrowBy = XBufferGrowByDefault);

	// La red�finition de la m�thode suivante ne devrait pas �tre n�cessaire (je crois !). Et pourtant, si on l'enleve, �a ne compile pas...
	const void *Data(size_t ui=0) const { return XBuffer_Super::Data(ui); }
	void *Data(size_t ui=0) { return _WData+ui; }
	void *DataWithSizeMin(size_t ui, size_t size, size_t nGrowBy=XBufferGrowByDefault) { CheckSize(size, nGrowBy); return Data(ui); }

	char *CData(size_t ui=0) { return (char *)(_WData+ui); }
	char *CDataWithSizeMin(size_t ui, size_t size, size_t nGrowBy=XBufferGrowByDefault) { CheckSize(size, nGrowBy); return CData(ui); }

	unsigned char *UCData(size_t ui=0) { return _WData+ui; }
	void *UCDataWithSizeMin(size_t ui, unsigned int size, size_t nGrowBy=XBufferGrowByDefault) { CheckSize(size, nGrowBy); return UCData(ui); }

	size_t Size() const { return m_allocatedSize; }
	void SetLength(size_t len) { _Len = len; };

	/* IsNull ? */
	void SetNull() { SetLength(0); };

	/* [] */
	unsigned char& operator [](int i) { return UCData()[i]; } // underflow ? overflow ?
	unsigned char& operator [](size_t i) { return UCData()[i]; }

	void Cpy(const void *buf, size_t len);
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

	const XBuffer &operator += (const XRBuffer &aBuffer);

	size_t Sizeof() const;
//	bool ReadFileName(const char* FileName, unsigned int AddZeros=0);
	bool ReadFromBuf(const char *buf, size_t *idx, size_t count);
	bool ReadFromXBuffer(XRBuffer &unXBuffer);
};

#endif
