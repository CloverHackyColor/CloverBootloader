//*************************************************************************************************
//*************************************************************************************************
//
//                                          XObjArray
//
// Developed by jief666, from 1997.
//
//*************************************************************************************************
//*************************************************************************************************

#if !defined(__XOBJARRAY_H__)
#define __XOBJARRAY_H__

#include <XToolsConf.h>
#include "XToolsCommon.h"


#if 1
#define XObjArray_DBG(...) printf__VA_ARGS__)
#else
#define XObjArray_DBG(...)
#endif



template <class TYPE>
class XObjArrayEntry
{
  public:
	TYPE* Object;
	bool  FreeIt;
};

template<class TYPE>
class XObjArrayNC
{
  public:
	XObjArrayEntry<TYPE> *_Data;
	size_t _Len;
	size_t m_allocatedSize;

  public:
	void Init();
	XObjArrayNC() : _Data(0), _Len(0), m_allocatedSize(0) { Init(); }
	virtual ~XObjArrayNC();

  protected:
//	XObjArrayNC(const XObjArrayNC<TYPE> &anObjArrayNC) { (void)anObjArrayNC; panic("Intentionally not defined"); }
//	const XObjArrayNC<TYPE> &operator =(const XObjArrayNC<TYPE> &anObjArrayNC) { (void)anObjArrayNC; panic("Intentionally not defined"); }
	XObjArrayNC(const XObjArrayNC<TYPE> &anObjArrayNC) = delete;
	const XObjArrayNC<TYPE> &operator =(const XObjArrayNC<TYPE> &anObjArrayNC) = delete;
	size_t _getLen() const { return _Len; }

  public:
	size_t AllocatedSize() const { return m_allocatedSize; }
	size_t size() const { return _Len; }
	size_t length() const { return _Len; }

	bool notEmpty() const { return size() > 0; }
	bool isEmpty() const { return size() == 0; }

	template<typename IntegralType, enable_if(is_integral(IntegralType))>
	const TYPE &ElementAt(IntegralType nIndex) const
	{
		if (nIndex < 0) {
			panic("XObjArrayNC::ElementAt() : i < 0. System halted\n");
		}
		if ( (unsigned_type(IntegralType))nIndex >= _Len ) {
			panic("XObjArrayNC::ElementAt() -> operator []  -  index (%zu) greater than length (%zu)\n", (size_t)nIndex, _Len);
		}
		return  *((TYPE *)(_Data[nIndex].Object));
	}

	template<typename IntegralType, enable_if(is_integral(IntegralType))>
	TYPE &ElementAt(IntegralType nIndex)
	{
		if (nIndex < 0) {
			panic("XObjArrayNC::ElementAt() : i < 0. System halted\n");
		}
		if ( (unsigned_type(IntegralType))nIndex >= _Len ) {
			panic("XObjArrayNC::ElementAt() const -> operator []  -  index (%zu) greater than length (%zu)\n", (size_t)nIndex, _Len);
		}
		return  *((TYPE *)(_Data[nIndex].Object));
	}

	// This was useful for realtime debugging with a debugger that do not recognise references. That was years and years ago. Probably not needed anymore.
//	#ifdef _DEBUG_iufasdfsfk
		const TYPE *DbgAt(int i) const { if ( i >= 0 && (size_t)i < _Len ) return &ElementAt ((size_t) i); else return NULL; }
//	#endif

	template<typename IntegralType, enable_if(is_integral(IntegralType))>
	const TYPE &operator[](IntegralType nIndex) const { return ElementAt(nIndex); }
	
	template<typename IntegralType, enable_if(is_integral(IntegralType))>
	TYPE &operator[](IntegralType nIndex) { return ElementAt(nIndex); }

	size_t AddReference(TYPE *newElement, bool FreeIt);

//	size_t InsertRef(TYPE *newElement, size_t pos, bool FreeIt = false);
	size_t InsertRef(TYPE *newElement, size_t pos, bool FreeIt);

	void SetFreeIt(size_t nIndex, bool Flag);
	void SetFreeIt(const TYPE *Element, bool Flag);

	void Remove(const TYPE *Element);
	void RemoveWithoutFreeing(const TYPE *Element);
	void Remove(const TYPE &Element);
	void RemoveAtIndex(size_t nIndex);
	void RemoveAtIndex(int nIndex);
	void RemoveWithoutFreeingAtIndex(size_t nIndex); // If you use this, there might be a design problem somewhere ???
	//void Remove(int nIndex);
	void RemoveAllBut(const TYPE *Element);

	void setEmpty();

  public:
	void CheckSize(size_t nNewSize, size_t nGrowBy = XArrayGrowByDefault);

};

template<class TYPE>
class XObjArray : public XObjArrayNC<TYPE>
{
  public:
	XObjArray() : XObjArrayNC<TYPE>() {}

  XObjArray(bool FreeThem, TYPE* n1, ...);

  XObjArray(const TYPE &n1, bool FreeIt = true);
  XObjArray(const TYPE &n1, const TYPE &n2, bool FreeThem = true);
  XObjArray(const TYPE &n1, const TYPE &n2, const TYPE &n3, bool FreeThem = true);
  XObjArray(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, bool FreeThem = true);
  XObjArray(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, const TYPE &n5, bool FreeThem = true);
  XObjArray(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, const TYPE &n5, const TYPE &n6, bool FreeThem = true);
  XObjArray(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, const TYPE &n5, const TYPE &n6, const TYPE &n7, bool FreeThem = true);
  XObjArray(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, const TYPE &n5, const TYPE &n6, const TYPE &n7, const TYPE &n8, bool FreeThem = true);
  XObjArray(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, const TYPE &n5, const TYPE &n6, const TYPE &n7, const TYPE &n8, const TYPE &n9, bool FreeThem = true);
  XObjArray(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, const TYPE &n5, const TYPE &n6, const TYPE &n7, const TYPE &n8, const TYPE &n9, const TYPE &n10, bool FreeThem = true);
  XObjArray(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, const TYPE &n5, const TYPE &n6, const TYPE &n7, const TYPE &n8, const TYPE &n9, const TYPE &n10, const TYPE &n11, bool FreeThem = true);
  XObjArray(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, const TYPE &n5, const TYPE &n6, const TYPE &n7, const TYPE &n8, const TYPE &n9, const TYPE &n10, const TYPE &n11, const TYPE &n12, bool FreeThem = true);
  XObjArray(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, const TYPE &n5, const TYPE &n6, const TYPE &n7, const TYPE &n8, const TYPE &n9, const TYPE &n10, const TYPE &n11, const TYPE &n12, const TYPE &n13, bool FreeThem = true);
  XObjArray(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, const TYPE &n5, const TYPE &n6, const TYPE &n7, const TYPE &n8, const TYPE &n9, const TYPE &n10, const TYPE &n11, const TYPE &n12, const TYPE &n13, const TYPE &n14, bool FreeThem = true);

	XObjArray(const XObjArray<TYPE> &anObjArray);

	const XObjArray<TYPE> &operator =(const XObjArray<TYPE> &anObjArray);

	size_t AddCopy(const TYPE &newElement, bool FreeIt = true);
	size_t AddCopies(const TYPE &n1, bool FreeIt = true);
	size_t AddCopies(const TYPE &n1, const TYPE &n2, bool FreeThem = true);
	size_t AddCopies(const TYPE &n1, const TYPE &n2, const TYPE &n3, bool FreeThem = true);
	size_t AddCopies(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, bool FreeThem = true);
	size_t AddCopies(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, const TYPE &n5, bool FreeThem = true);
	size_t AddCopies(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, const TYPE &n5, const TYPE &n6, bool FreeThem = true);
	size_t AddCopies(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, const TYPE &n5, const TYPE &n6, const TYPE &n7, bool FreeThem = true);
	size_t AddCopies(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, const TYPE &n5, const TYPE &n6, const TYPE &n7, const TYPE &n8, bool FreeThem = true);
	size_t AddCopies(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, const TYPE &n5, const TYPE &n6, const TYPE &n7, const TYPE &n8, const TYPE &n9, bool FreeThem = true);
	size_t AddCopies(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, const TYPE &n5, const TYPE &n6, const TYPE &n7, const TYPE &n8, const TYPE &n9, const TYPE &n10, bool FreeThem = true);
	size_t AddCopies(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, const TYPE &n5, const TYPE &n6, const TYPE &n7, const TYPE &n8, const TYPE &n9, const TYPE &n10, const TYPE &n11, bool FreeThem = true);
	size_t AddCopies(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, const TYPE &n5, const TYPE &n6, const TYPE &n7, const TYPE &n8, const TYPE &n9, const TYPE &n10, const TYPE &n11, const TYPE &n12, bool FreeThem = true);
	size_t AddCopies(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, const TYPE &n5, const TYPE &n6, const TYPE &n7, const TYPE &n8, const TYPE &n9, const TYPE &n10, const TYPE &n11, const TYPE &n12, const TYPE &n13, bool FreeThem = true);
	size_t AddCopies(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, const TYPE &n5, const TYPE &n6, const TYPE &n7, const TYPE &n8, const TYPE &n9, const TYPE &n10, const TYPE &n11, const TYPE &n12, const TYPE &n13, const TYPE &n14, bool FreeThem = true);
	//TYPE &       AddNew(bool FreeIt = true);

	size_t InsertCopy(const TYPE &newElement, size_t pos);

};

//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//
//                                          XObjArray
//
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx


/* Constructeur */
template<class TYPE>
void XObjArrayNC<TYPE>::Init()
{
	_Data = nullptr;
	m_allocatedSize = 0;
	_Len = 0;
	// THis was useful for realtime debugging with a debugger that do not recognise references.
//	#ifdef _DEBUG_iufasdfsfk
//	{
//		const TYPE *tmp;
//		tmp = DbgAt(0);
//	}
//	#endif
}
/* Constructeur */
template<class TYPE>
XObjArray<TYPE>::XObjArray(bool FreeThem, TYPE* n1, ...)
{
  va_list     va;

  va_start (va, n1);
  size_t count = 0;
  TYPE* t = va_arg(va, TYPE*);
  while ( t != nullptr ) {
    count++;
    t = va_arg(va, TYPE*);
  }
  va_end(va);

  XObjArrayNC<TYPE>::Init();
  this->CheckSize(count+1, (size_t)0);
  XObjArrayNC<TYPE>::AddReference(n1, FreeThem);

  va_start (va, n1);
  t = va_arg(va, TYPE*);
  while ( t != nullptr ) {
    XObjArrayNC<TYPE>::AddReference(t, FreeThem);
    t = va_arg(va, TYPE*);
  }
  va_end(va);
}



template<class TYPE>
XObjArray<TYPE>::XObjArray(const TYPE &n1, bool FreeIt)
{
  XObjArrayNC<TYPE>::Init();
  this->CheckSize(1, (size_t)0);
  AddCopy(n1, FreeIt);
}
template<class TYPE>
XObjArray<TYPE>::XObjArray(const TYPE &n1, const TYPE &n2, bool FreeThem)
{
  XObjArrayNC<TYPE>::Init();
  this->CheckSize(2, (size_t)0);
  AddCopy(n1, FreeThem);
  AddCopy(n2, FreeThem);
}
template<class TYPE>
XObjArray<TYPE>::XObjArray(const TYPE &n1, const TYPE &n2, const TYPE &n3, bool FreeThem)
{
  XObjArrayNC<TYPE>::Init();
  this->CheckSize(3, (size_t)0);
  AddCopy(n1, FreeThem);
  AddCopy(n2, FreeThem);
  AddCopy(n3, FreeThem);
}
template<class TYPE>
XObjArray<TYPE>::XObjArray(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, bool FreeThem)
{
  XObjArrayNC<TYPE>::Init();
  this->CheckSize(4, (size_t)0);
  AddCopy(n1, FreeThem);
  AddCopy(n2, FreeThem);
  AddCopy(n3, FreeThem);
  AddCopy(n4, FreeThem);
}
template<class TYPE>
XObjArray<TYPE>::XObjArray(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, const TYPE &n5, bool FreeThem)
{
  XObjArrayNC<TYPE>::Init();
  this->CheckSize(5, (size_t)0);
  AddCopy(n1, FreeThem);
  AddCopy(n2, FreeThem);
  AddCopy(n3, FreeThem);
  AddCopy(n4, FreeThem);
  AddCopy(n5, FreeThem);
}
template<class TYPE>
XObjArray<TYPE>::XObjArray(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, const TYPE &n5, const TYPE &n6, bool FreeThem)
{
  XObjArrayNC<TYPE>::Init();
  this->CheckSize(6, (size_t)0);
  AddCopy(n1, FreeThem);
  AddCopy(n2, FreeThem);
  AddCopy(n3, FreeThem);
  AddCopy(n4, FreeThem);
  AddCopy(n5, FreeThem);
  AddCopy(n6, FreeThem);
}
template<class TYPE>
XObjArray<TYPE>::XObjArray(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, const TYPE &n5, const TYPE &n6, const TYPE &n7, bool FreeThem)
{
  XObjArrayNC<TYPE>::Init();
  this->CheckSize(7, (size_t)0);
  AddCopy(n1, FreeThem);
  AddCopy(n2, FreeThem);
  AddCopy(n3, FreeThem);
  AddCopy(n4, FreeThem);
  AddCopy(n5, FreeThem);
  AddCopy(n6, FreeThem);
  AddCopy(n7, FreeThem);
}
template<class TYPE>
XObjArray<TYPE>::XObjArray(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, const TYPE &n5, const TYPE &n6, const TYPE &n7, const TYPE &n8, bool FreeThem)
{
  XObjArrayNC<TYPE>::Init();
  this->CheckSize(8, (size_t)0);
  AddCopy(n1, FreeThem);
  AddCopy(n2, FreeThem);
  AddCopy(n3, FreeThem);
  AddCopy(n4, FreeThem);
  AddCopy(n5, FreeThem);
  AddCopy(n6, FreeThem);
  AddCopy(n7, FreeThem);
  AddCopy(n8, FreeThem);
}
template<class TYPE>
XObjArray<TYPE>::XObjArray(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, const TYPE &n5, const TYPE &n6, const TYPE &n7, const TYPE &n8, const TYPE &n9, bool FreeThem)
{
  XObjArrayNC<TYPE>::Init();
  this->CheckSize(9, (size_t)0);
  AddCopy(n1, FreeThem);
  AddCopy(n2, FreeThem);
  AddCopy(n3, FreeThem);
  AddCopy(n4, FreeThem);
  AddCopy(n5, FreeThem);
  AddCopy(n6, FreeThem);
  AddCopy(n7, FreeThem);
  AddCopy(n8, FreeThem);
  AddCopy(n9, FreeThem);
}
template<class TYPE>
XObjArray<TYPE>::XObjArray(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, const TYPE &n5, const TYPE &n6, const TYPE &n7, const TYPE &n8, const TYPE &n9, const TYPE &n10, bool FreeThem)
{
  XObjArrayNC<TYPE>::Init();
  this->CheckSize(10, (size_t)0);
  AddCopy(n1, FreeThem);
  AddCopy(n2, FreeThem);
  AddCopy(n3, FreeThem);
  AddCopy(n4, FreeThem);
  AddCopy(n5, FreeThem);
  AddCopy(n6, FreeThem);
  AddCopy(n7, FreeThem);
  AddCopy(n8, FreeThem);
  AddCopy(n9, FreeThem);
  AddCopy(n10, FreeThem);
}
template<class TYPE>
XObjArray<TYPE>::XObjArray(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, const TYPE &n5, const TYPE &n6, const TYPE &n7, const TYPE &n8, const TYPE &n9, const TYPE &n10, const TYPE &n11, bool FreeThem)
{
  XObjArrayNC<TYPE>::Init();
  this->CheckSize(11, (size_t)0);
  AddCopy(n1, FreeThem);
  AddCopy(n2, FreeThem);
  AddCopy(n3, FreeThem);
  AddCopy(n4, FreeThem);
  AddCopy(n5, FreeThem);
  AddCopy(n6, FreeThem);
  AddCopy(n7, FreeThem);
  AddCopy(n8, FreeThem);
  AddCopy(n9, FreeThem);
  AddCopy(n10, FreeThem);
  AddCopy(n11, FreeThem);
}
template<class TYPE>
XObjArray<TYPE>::XObjArray(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, const TYPE &n5, const TYPE &n6, const TYPE &n7, const TYPE &n8, const TYPE &n9, const TYPE &n10, const TYPE &n11, const TYPE &n12, bool FreeThem)
{
  XObjArrayNC<TYPE>::Init();
  this->CheckSize(12, (size_t)0);
  AddCopy(n1, FreeThem);
  AddCopy(n2, FreeThem);
  AddCopy(n3, FreeThem);
  AddCopy(n4, FreeThem);
  AddCopy(n5, FreeThem);
  AddCopy(n6, FreeThem);
  AddCopy(n7, FreeThem);
  AddCopy(n8, FreeThem);
  AddCopy(n9, FreeThem);
  AddCopy(n10, FreeThem);
  AddCopy(n11, FreeThem);
  AddCopy(n12, FreeThem);
}
template<class TYPE>
XObjArray<TYPE>::XObjArray(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, const TYPE &n5, const TYPE &n6, const TYPE &n7, const TYPE &n8, const TYPE &n9, const TYPE &n10, const TYPE &n11, const TYPE &n12, const TYPE &n13, bool FreeThem)
{
  XObjArrayNC<TYPE>::Init();
  this->CheckSize(13, (size_t)0);
  AddCopy(n1, FreeThem);
  AddCopy(n2, FreeThem);
  AddCopy(n3, FreeThem);
  AddCopy(n4, FreeThem);
  AddCopy(n5, FreeThem);
  AddCopy(n6, FreeThem);
  AddCopy(n7, FreeThem);
  AddCopy(n8, FreeThem);
  AddCopy(n9, FreeThem);
  AddCopy(n10, FreeThem);
  AddCopy(n11, FreeThem);
  AddCopy(n12, FreeThem);
  AddCopy(n13, FreeThem);
}
template<class TYPE>
XObjArray<TYPE>::XObjArray(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, const TYPE &n5, const TYPE &n6, const TYPE &n7, const TYPE &n8, const TYPE &n9, const TYPE &n10, const TYPE &n11, const TYPE &n12, const TYPE &n13, const TYPE &n14, bool FreeThem)
{
  XObjArrayNC<TYPE>::Init();
  this->CheckSize(14, (size_t)0);
  AddCopy(n1, FreeThem);
  AddCopy(n2, FreeThem);
  AddCopy(n3, FreeThem);
  AddCopy(n4, FreeThem);
  AddCopy(n5, FreeThem);
  AddCopy(n6, FreeThem);
  AddCopy(n7, FreeThem);
  AddCopy(n8, FreeThem);
  AddCopy(n9, FreeThem);
  AddCopy(n10, FreeThem);
  AddCopy(n11, FreeThem);
  AddCopy(n12, FreeThem);
  AddCopy(n13, FreeThem);
  AddCopy(n14, FreeThem);
}

/* Constructeur */
template<class TYPE>
XObjArray<TYPE>::XObjArray(const XObjArray<TYPE> &anObjArray) : XObjArrayNC<TYPE>()
{
  size_t ui;

  XObjArrayNC<TYPE>::Init();
	this->CheckSize(anObjArray.size(), (size_t)0);
	for ( ui=0 ; ui<anObjArray.size() ; ui+=1 ) AddCopy(anObjArray.ElementAt(ui));
}

/* operator = */
template<class TYPE>
const XObjArray<TYPE> &XObjArray<TYPE>::operator =(const XObjArray<TYPE> &anObjArray)
{
  size_t ui;

	if ( this == &anObjArray ) return *this; // self assignement
  	XObjArrayNC<TYPE>::setEmpty();
	this->CheckSize(anObjArray.length(), 0);
	for ( ui=0 ; ui<anObjArray.size() ; ui+=1 ) AddCopy(anObjArray.ElementAt(ui));
	return *this;
}

/* Destructeur */
template<class TYPE>
XObjArrayNC<TYPE>::~XObjArrayNC()
{
//printf("XObjArray Destructor\n");
	setEmpty();
	if ( _Data ) free(_Data);
}

/* CheckSize() */
template<class TYPE>
void XObjArrayNC<TYPE>::CheckSize(size_t nNewSize, size_t nGrowBy)
{
	if ( m_allocatedSize < nNewSize ) {
		nNewSize += nGrowBy + 1;
		_Data = (XObjArrayEntry<TYPE> *)Xrealloc((void *)_Data, sizeof(XObjArrayEntry<TYPE>) * nNewSize, sizeof(XObjArrayEntry<TYPE>) * m_allocatedSize);
		if ( !_Data ) {
			panic("XObjArrayNC<TYPE>::CheckSize(nNewSize=%zu, nGrowBy=%zu) : Xrealloc(%zu, %zu, %" PRIuPTR ") returned NULL. System halted\n", nNewSize, nGrowBy, m_allocatedSize, sizeof(XObjArrayEntry<TYPE>) * nNewSize, (uintptr_t)_Data);
		}
//		memset(&_Data[m_allocatedSize], 0, (nNewSize-m_allocatedSize) * sizeof(XObjArrayEntry<TYPE>));
		m_allocatedSize = nNewSize;
	}
}

///* Add() */
//template<class TYPE>
//TYPE &XObjArray<TYPE>::AddNew(bool FreeIt)
//{
//	XObjArrayNC<TYPE>::CheckSize(XObjArray<TYPE>::_getLen()+1);
//	XObjArray<TYPE>::_Data[XObjArray<TYPE>::_Len].Object = new TYPE;
//	XObjArray<TYPE>::_Data[XObjArray<TYPE>::_Len].FreeIt = FreeIt;
//	XObjArray<TYPE>::_Len += 1;
//	return *((TYPE *)(XObjArray<TYPE>::_Data[XObjArray<TYPE>::_Len-1].Object));
//}

/* Add(TYPE &, size_t) */
template<class TYPE>
size_t XObjArray<TYPE>::AddCopy(const TYPE &newElement, bool FreeIt)
{
	XObjArrayNC<TYPE>::CheckSize(XObjArray<TYPE>::_Len+1);
  XObjArray<TYPE>::_Data[XObjArray<TYPE>::_Len].Object = new TYPE(newElement);
	XObjArray<TYPE>::_Data[XObjArray<TYPE>::_Len].FreeIt = FreeIt;
	XObjArray<TYPE>::_Len += 1;
	return XObjArray<TYPE>::_Len-1;
}

template<class TYPE>
size_t XObjArray<TYPE>::AddCopies(const TYPE &n1, bool FreeIt)
{
	return AddCopy(n1, FreeIt);
}

template<class TYPE>
size_t XObjArray<TYPE>::AddCopies(const TYPE &n1, const TYPE &n2, bool FreeThem)
{
	size_t ui = AddCopies(n1, FreeThem);
	AddCopy(n2, FreeThem);
	return ui;
}

template<class TYPE>
size_t XObjArray<TYPE>::AddCopies(const TYPE &n1, const TYPE &n2, const TYPE &n3, bool FreeThem)
{
	size_t ui = AddCopies(n1, n2, FreeThem);
	AddCopy(n3, FreeThem);
	return ui;
}

template<class TYPE>
size_t XObjArray<TYPE>::AddCopies(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, bool FreeThem)
{
	size_t ui = AddCopies(n1, n2, n3, FreeThem);
	AddCopy(n4, FreeThem);
	return ui;
}

template<class TYPE>
size_t XObjArray<TYPE>::AddCopies(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, const TYPE &n5, bool FreeThem)
{
	size_t ui = AddCopies(n1, n2, n3, n4, FreeThem);
	AddCopy(n5, FreeThem);
	return ui;
}

template<class TYPE>
size_t XObjArray<TYPE>::AddCopies(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, const TYPE &n5, const TYPE &n6, bool FreeThem)
{
	size_t ui = AddCopies(n1, n2, n3, n4, n5, FreeThem);
	AddCopy(n6, FreeThem);
	return ui;
}

template<class TYPE>
size_t XObjArray<TYPE>::AddCopies(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, const TYPE &n5, const TYPE &n6, const TYPE &n7, bool FreeThem)
{
	size_t ui = AddCopies(n1, n2, n3, n4, n5, n6, FreeThem);
	AddCopy(n7, FreeThem);
	return ui;
}

template<class TYPE>
size_t XObjArray<TYPE>::AddCopies(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, const TYPE &n5, const TYPE &n6, const TYPE &n7, const TYPE &n8, bool FreeThem)
{
	size_t ui = AddCopies(n1, n2, n3, n4, n5, n6, n7, FreeThem);
	AddCopy(n8, FreeThem);
	return ui;
}

template<class TYPE>
size_t XObjArray<TYPE>::AddCopies(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, const TYPE &n5, const TYPE &n6, const TYPE &n7, const TYPE &n8, const TYPE &n9, bool FreeThem)
{
	size_t ui = AddCopies(n1, n2, n3, n4, n5, n6, n7, n8, FreeThem);
	AddCopy(n9, FreeThem);
	return ui;
}

template<class TYPE>
size_t XObjArray<TYPE>::AddCopies(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, const TYPE &n5, const TYPE &n6, const TYPE &n7, const TYPE &n8, const TYPE &n9, const TYPE &n10, bool FreeThem)
{
	size_t ui = AddCopies(n1, n2, n3, n4, n5, n6, n7, n8, n9, FreeThem);
	AddCopy(n10, FreeThem);
	return ui;
}

template<class TYPE>
size_t XObjArray<TYPE>::AddCopies(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, const TYPE &n5, const TYPE &n6, const TYPE &n7, const TYPE &n8, const TYPE &n9, const TYPE &n10, const TYPE &n11, bool FreeThem)
{
	size_t ui = AddCopies(n1, n2, n3, n4, n5, n6, n7, n8, n9, n10, FreeThem);
	AddCopy(n11, FreeThem);
	return ui;
}

template<class TYPE>
size_t XObjArray<TYPE>::AddCopies(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, const TYPE &n5, const TYPE &n6, const TYPE &n7, const TYPE &n8, const TYPE &n9, const TYPE &n10, const TYPE &n11, const TYPE &n12, bool FreeThem)
{
	size_t ui = AddCopies(n1, n2, n3, n4, n5, n6, n7, n8, n9, n10, n11, FreeThem);
	AddCopy(n12, FreeThem);
	return ui;
}

template<class TYPE>
size_t XObjArray<TYPE>::AddCopies(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, const TYPE &n5, const TYPE &n6, const TYPE &n7, const TYPE &n8, const TYPE &n9, const TYPE &n10, const TYPE &n11, const TYPE &n12, const TYPE &n13, bool FreeThem)
{
	size_t ui = AddCopies(n1, n2, n3, n4, n5, n6, n7, n8, n9, n10, n11, n12, FreeThem);
	AddCopy(n13, FreeThem);
	return ui;
}

template<class TYPE>
size_t XObjArray<TYPE>::AddCopies(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, const TYPE &n5, const TYPE &n6, const TYPE &n7, const TYPE &n8, const TYPE &n9, const TYPE &n10, const TYPE &n11, const TYPE &n12, const TYPE &n13, const TYPE &n14, bool FreeThem)
{
	size_t ui = AddCopies(n1, n2, n3, n4, n5, n6, n7, n8, n9, n10, n11, n12, n13, FreeThem);
	AddCopy(n14, FreeThem);
	return ui;
}

/* Add(TYPE *, size_t) */
template<class TYPE>
size_t XObjArrayNC<TYPE>::AddReference(TYPE *newElement, bool FreeIt)
{
	XObjArrayNC<TYPE>::CheckSize(XObjArrayNC<TYPE>::_Len+1);
	XObjArrayNC<TYPE>::_Data[XObjArrayNC<TYPE>::_Len].Object = newElement;
	XObjArrayNC<TYPE>::_Data[XObjArrayNC<TYPE>::_Len].FreeIt = FreeIt;
	XObjArrayNC<TYPE>::_Len += 1;
	return XObjArrayNC<TYPE>::_Len-1;
}

/* Insert(TYPE &, size_t) */
template<class TYPE>
size_t XObjArray<TYPE>::InsertCopy(const TYPE &newElement, size_t pos)
{
	if ( pos  < XObjArray<TYPE>::_Len ) {
		XObjArrayNC<TYPE>::CheckSize(XObjArray<TYPE>::_Len+1);
		memmove(&XObjArray<TYPE>::_Data[pos+1], &XObjArray<TYPE>::_Data[pos], (XObjArray<TYPE>::_Len-pos)*sizeof(XObjArrayEntry<TYPE>));
		XObjArray<TYPE>::_Data[pos].Object = new TYPE(newElement);
		XObjArray<TYPE>::_Data[pos].FreeIt = true;
		XObjArray<TYPE>::_Len += 1;
		return pos;
	}else{
		return AddCopy(newElement);
	}
}

/* Insert(TYPE &, size_t) */
template<class TYPE>
size_t XObjArrayNC<TYPE>::InsertRef(TYPE *newElement, size_t pos, bool FreeIt)
{
	if ( pos  < XObjArrayNC<TYPE>::_Len ) {
		CheckSize(XObjArrayNC<TYPE>::_Len+1);
		memmove(&XObjArrayNC<TYPE>::_Data[pos+1], &XObjArrayNC<TYPE>::_Data[pos], (XObjArrayNC<TYPE>::_Len-pos)*sizeof(XObjArrayEntry<TYPE>));
		_Data[pos].Object = newElement;
		_Data[pos].FreeIt = FreeIt;
		XObjArrayNC<TYPE>::_Len += 1;
		return pos;
	}else{
		return AddRef(newElement, FreeIt);
	}
}

/* SetFreeIt(size_t, bool) */
template<class TYPE>
void XObjArrayNC<TYPE>::SetFreeIt(size_t nIndex, bool Flag)
{
	if ( nIndex  < XObjArrayNC<TYPE>::_Len )
	{
		XObjArrayNC<TYPE>::_Data[nIndex].FreeIt = Flag;
	}
	else{
		#if defined(_DEBUG)
			throw "XObjArray::SetFreeIt(size_t) -> nIndex >= _Len\n";
		#endif
	}
}

/* SetFreeIt(const TYPE *Element, bool) */
template<class TYPE>
void XObjArrayNC<TYPE>::SetFreeIt(const TYPE *Element, bool Flag)
{
 size_t i;

	for ( i=0 ; i < XObjArrayNC<TYPE>::_Len ; i+= 1) {
		if ( ((TYPE *)XObjArrayNC<TYPE>::_Data[i].Object) == Element ) {
			SetFreeIt(i, Flag);
			return ;
		}
	}
	#if defined(_DEBUG)
		throw "XObjArray::SetFreeIt(const TYPE *) -> nIndex >= _Len\n";
	#endif
}

/* Remove(size_t) */
template<class TYPE>
void XObjArrayNC<TYPE>::RemoveAtIndex(size_t nIndex)
{
  if ( nIndex >= XObjArrayNC<TYPE>::_Len ) {
    panic("void XObjArrayNC<TYPE>::RemoveAtIndex(size_t nIndex) : BUG nIndex (%zu) is > length(). System halted\n", nIndex);
  }
	if ( _Data[nIndex].FreeIt )
	{
		TYPE *TmpObject; // BCB 4 oblige me to use a tmp var for doing the delete.

		TmpObject = (TYPE *)(_Data[nIndex].Object);
		delete TmpObject;
	}
	if ( nIndex<XObjArrayNC<TYPE>::_Len-1 ) memmove(&_Data[nIndex], &_Data[nIndex+1], (_Len-nIndex-1)*sizeof(XObjArrayEntry<TYPE>));
	_Len -= 1;
	return;
}

//-------------------------------------------------------------------------------------------------
//                                               
//-------------------------------------------------------------------------------------------------
/* RemoveWithoutFreeing(size_t) */
template<class TYPE>
void XObjArrayNC<TYPE>::RemoveWithoutFreeingAtIndex(size_t nIndex)
{
	if ( nIndex < _Len )
	{
		if ( nIndex<_Len-1 ) memcpy(&_Data[nIndex], &_Data[nIndex+1], (_Len-nIndex-1)*sizeof(XObjArrayEntry<TYPE>));
		_Len -= 1;
		return;
	}
	#if defined(_DEBUG)
		panic("XObjArray::RemoveWithoutFreeing(size_t) -> nIndex > _Len");
	#endif
}

//-------------------------------------------------------------------------------------------------
//                                               
//-------------------------------------------------------------------------------------------------
/* Remove(int) */
template<class TYPE>
void XObjArrayNC<TYPE>::RemoveAtIndex(int nIndex)
{
  #if defined(__XTOOLS_CHECK_OVERFLOW__)
  	if ( nIndex < 0 ) {
  	  panic("XArray<TYPE>::RemoveAtIndex(int nIndex) : BUG nIndex (%d) is < 0. System halted\n", nIndex);
	}
	#endif
	RemoveAtIndex( (size_t)nIndex ); // Remove(size_t) will check that index is < _Len
}

/* Remove(const TYPE &) */
template<class TYPE>
void XObjArrayNC<TYPE>::Remove(const TYPE &Element)
{
  size_t i;

	for ( i=0 ; i<_Len ; i+= 1) {
		if ( *((TYPE *)(_Data[i].Object)) == Element ) {
			RemoveAtIndex(i);
			return ;
		}
	}
	#if defined(_DEBUG)
		printf("XObjArray::Remove(TYPE &) -> Not found\n");
		panic();
	#endif
}

//-------------------------------------------------------------------------------------------------
//                                               
//-------------------------------------------------------------------------------------------------
/* Remove(const TYPE *) */
template<class TYPE>
void XObjArrayNC<TYPE>::Remove(const TYPE *Element)
{
  size_t i;

	for ( i=0 ; i<_Len ; i+= 1) {
		if ( ((TYPE *)_Data[i].Object) == Element ) {
			Remove(i);
			return ;
		}
	}
	#if defined(_DEBUG)
		throw "XObjArray::Remove(TYPE *) -> not found\n";
	#endif
}

//-------------------------------------------------------------------------------------------------
//                                               
//-------------------------------------------------------------------------------------------------
/* RemoveWithoutFreeing(const TYPE *) */
template<class TYPE>
void XObjArrayNC<TYPE>::RemoveWithoutFreeing(const TYPE *Element)
{
  size_t i;

	for ( i=0 ; i<_Len ; i+= 1) {
		if ( ((TYPE *)_Data[i].Object) == Element ) {
			RemoveWithoutFreeing(i);
			return ;
		}
	}
	#if defined(_DEBUG)
		throw "XObjArray::RemoveWithoutFreeing(TYPE *) -> not found\n";
	#endif
}

//-------------------------------------------------------------------------------------------------
//                                               
//-------------------------------------------------------------------------------------------------
template<class TYPE>
void XObjArrayNC<TYPE>::RemoveAllBut(const TYPE *Element)
{
  size_t i;

	for ( i=_Len ; i-- ; ) {
		if ( ((TYPE *)_Data[i].Object) != Element ) {
			Remove(i);
		}
	}
}

/* Empty() */
template<class TYPE>
void XObjArrayNC<TYPE>::setEmpty()
{
  size_t i;

	if ( _Len > 0 ) {
		for ( i=0 ; i<_Len ; i+= 1) {
			if ( _Data[i].FreeIt )
			{
			  TYPE *TmpObject; // BCB 4 oblige me to use a tmp var for doing the delete.

				TmpObject = (TYPE *)(_Data[i].Object);
				delete TmpObject;
			}
		}
		_Len = 0;
	}
}

#endif
