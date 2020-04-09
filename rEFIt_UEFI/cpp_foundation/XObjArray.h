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

#include "XToolsCommon.h"


#if 1
#define XObjArray_DBG(...) DebugLog(2, __VA_ARGS__)
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
	xsize _Len;
	xsize m_allocatedSize;

  public:
	void Init();
	XObjArrayNC() { Init(); }
	virtual ~XObjArrayNC();

  protected:
	XObjArrayNC(const XObjArrayNC<TYPE> &anObjArrayNC) { (void)anObjArrayNC; DebugLog(2, "Intentionally not defined"); panic(); }
	const XObjArrayNC<TYPE> &operator =(const XObjArrayNC<TYPE> &anObjArrayNC) { (void)anObjArrayNC; DebugLog(2, "Intentionally not defined"); panic(); }
	xsize _getLen() const { return _Len; }

  public:
	xsize AllocatedSize() const { return m_allocatedSize; }
	xsize size() const { return _Len; }
	xsize length() const { return _Len; }

	bool NotNull() const { return size() > 0; }
	bool IsNull() const { return size() == 0; }

	const TYPE &ElementAt(xsize nIndex) const;
	TYPE &ElementAt(xsize nIndex);
	
	// This was useful for realtime debugging with a debugger that do not recognise references. That was years and years ago. Probably not needed anymore.
	#ifdef _DEBUG_iufasdfsfk
		const TYPE *DbgAt(int i) const { if ( i >= 0 && (xsize)i < _Len ) return &ElementAt ((xsize) i); else return NULL; }
	#endif

	const TYPE &operator[](xsize nIndex) const { return ElementAt(nIndex); }
	TYPE &operator[](xsize nIndex) { return ElementAt(nIndex); }

	xsize AddReference(TYPE *newElement, bool FreeIt);

//	xsize InsertRef(TYPE *newElement, xsize pos, bool FreeIt = false);
	xsize InsertRef(TYPE *newElement, xsize pos, bool FreeIt);

	void SetFreeIt(xsize nIndex, bool Flag);
	void SetFreeIt(const TYPE *Element, bool Flag);

	void Remove(const TYPE *Element);
	void RemoveWithoutFreeing(const TYPE *Element);
	void Remove(const TYPE &Element);
	void RemoveAtIndex(xsize nIndex);
	void RemoveAtIndex(int nIndex);
	void RemoveWithoutFreeing(xsize nIndex); // If you use this, there might be a design problem somewhere ???
	//void Remove(int nIndex);
	void RemoveAllBut(const TYPE *Element);

	void Empty();

  public:
	void CheckSize(xsize nNewSize, xsize nGrowBy = XArrayGrowByDefault);

};

template<class TYPE>
class XObjArray : public XObjArrayNC<TYPE>
{
  public:
	XObjArray() : XObjArrayNC<TYPE>() {}
	XObjArray(const XObjArray<TYPE> &anObjArray);
	const XObjArray<TYPE> &operator =(const XObjArray<TYPE> &anObjArray);

	xsize AddCopy(const TYPE &newElement, bool FreeIt = true);
	xsize AddCopies(const TYPE &n1, bool FreeIt = true);
	xsize AddCopies(const TYPE &n1, const TYPE &n2, bool FreeThem = true);
	xsize AddCopies(const TYPE &n1, const TYPE &n2, const TYPE &n3, bool FreeThem = true);
	xsize AddCopies(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, bool FreeThem = true);
	xsize AddCopies(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, const TYPE &n5, bool FreeThem = true);
	xsize AddCopies(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, const TYPE &n5, const TYPE &n6, bool FreeThem = true);
	xsize AddCopies(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, const TYPE &n5, const TYPE &n6, const TYPE &n7, bool FreeThem = true);
	xsize AddCopies(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, const TYPE &n5, const TYPE &n6, const TYPE &n7, const TYPE &n8, bool FreeThem = true);
	xsize AddCopies(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, const TYPE &n5, const TYPE &n6, const TYPE &n7, const TYPE &n8, const TYPE &n9, bool FreeThem = true);
	xsize AddCopies(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, const TYPE &n5, const TYPE &n6, const TYPE &n7, const TYPE &n8, const TYPE &n9, const TYPE &n10, bool FreeThem = true);
	xsize AddCopies(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, const TYPE &n5, const TYPE &n6, const TYPE &n7, const TYPE &n8, const TYPE &n9, const TYPE &n10, const TYPE &n11, bool FreeThem = true);
	xsize AddCopies(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, const TYPE &n5, const TYPE &n6, const TYPE &n7, const TYPE &n8, const TYPE &n9, const TYPE &n10, const TYPE &n11, const TYPE &n12, bool FreeThem = true);
	xsize AddCopies(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, const TYPE &n5, const TYPE &n6, const TYPE &n7, const TYPE &n8, const TYPE &n9, const TYPE &n10, const TYPE &n11, const TYPE &n12, const TYPE &n13, bool FreeThem = true);
	xsize AddCopies(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, const TYPE &n5, const TYPE &n6, const TYPE &n7, const TYPE &n8, const TYPE &n9, const TYPE &n10, const TYPE &n11, const TYPE &n12, const TYPE &n13, const TYPE &n14, bool FreeThem = true);
	//TYPE &       AddNew(bool FreeIt = true);

	xsize InsertCopy(const TYPE &newElement, xsize pos);

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
	#ifdef _DEBUG_iufasdfsfk
	{
		const TYPE *tmp;
		tmp = DbgAt(0);
	}
	#endif
}

/* Constructeur */
template<class TYPE>
XObjArray<TYPE>::XObjArray(const XObjArray<TYPE> &anObjArray) : XObjArrayNC<TYPE>()
{
  xsize ui;

  	XObjArrayNC<TYPE>::Init();
	this->CheckSize(anObjArray.size(), (xsize)0);
	for ( ui=0 ; ui<anObjArray.size() ; ui+=1 ) AddCopy(anObjArray.ElementAt(ui));
}

/* operator = */
template<class TYPE>
const XObjArray<TYPE> &XObjArray<TYPE>::operator =(const XObjArray<TYPE> &anObjArray)
{
  xsize ui;

  	XObjArrayNC<TYPE>::Empty();
	CheckSize(anObjArray.length(), 0);
	for ( ui=0 ; ui<anObjArray.size() ; ui+=1 ) AddCopy(anObjArray.ElementAt(ui));
	return *this;
}

/* Destructeur */
template<class TYPE>
XObjArrayNC<TYPE>::~XObjArrayNC()
{
//printf("XObjArray Destructor\n");
	Empty();
	if ( _Data ) free(_Data);
}

/* CheckSize() */
template<class TYPE>
void XObjArrayNC<TYPE>::CheckSize(xsize nNewSize, xsize nGrowBy)
{
	if ( m_allocatedSize < nNewSize ) {
		nNewSize += nGrowBy + 1;
		_Data = (XObjArrayEntry<TYPE> *)realloc((void *)_Data, sizeof(XObjArrayEntry<TYPE>) * nNewSize, sizeof(XObjArrayEntry<TYPE>) * m_allocatedSize);
		if ( !_Data ) {
			DebugLog(2, "XObjArrayNC<TYPE>::CheckSize(nNewSize=%llu, nGrowBy=%llu) : Xrealloc(%llu, %llu, %" PRIuPTR ") returned NULL. System halted\n", nNewSize, nGrowBy, m_allocatedSize, sizeof(XObjArrayEntry<TYPE>) * nNewSize, (uintptr_t)_Data);
			panic();
		}
//		memset(&_Data[m_allocatedSize], 0, (nNewSize-m_allocatedSize) * sizeof(XObjArrayEntry<TYPE>));
		m_allocatedSize = nNewSize;
	}
}

/* ElementAt() */
template<class TYPE>
TYPE &XObjArrayNC<TYPE>::ElementAt(xsize index)
{
		if ( index >= _Len ) {
			DebugLog(2, "XObjArray<TYPE>::ElementAt(xsize) -> operator []  -  index (%llu) greater than length (%llu)\n", index, _Len);
			panic();
		}
		return  *((TYPE *)(_Data[index].Object));
}

/* ElementAt() */
template<class TYPE>
const TYPE &XObjArrayNC<TYPE>::ElementAt(xsize index) const
{
		if ( index >= _Len ) {
			DebugLog(2, "XObjArray<TYPE>::ElementAt(xsize) const -> operator []  -  index (%llu) greater than length (%llu)\n", index, _Len);
			panic();
		}
		return  *((TYPE *)(_Data[index].Object));
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

/* Add(TYPE &, xsize) */
template<class TYPE>
xsize XObjArray<TYPE>::AddCopy(const TYPE &newElement, bool FreeIt)
{
	XObjArrayNC<TYPE>::CheckSize(XObjArray<TYPE>::_Len+1);
	XObjArray<TYPE>::_Data[XObjArray<TYPE>::_Len].Object = new TYPE(newElement);
	XObjArray<TYPE>::_Data[XObjArray<TYPE>::_Len].FreeIt = FreeIt;
	XObjArray<TYPE>::_Len += 1;
	return XObjArray<TYPE>::_Len-1;
}

template<class TYPE>
xsize XObjArray<TYPE>::AddCopies(const TYPE &n1, bool FreeIt)
{
	return AddCopy(n1, FreeIt);
}

template<class TYPE>
xsize XObjArray<TYPE>::AddCopies(const TYPE &n1, const TYPE &n2, bool FreeThem)
{
	xsize ui = AddCopies(n1, FreeThem);
	AddCopy(n2, FreeThem);
	return ui;
}

template<class TYPE>
xsize XObjArray<TYPE>::AddCopies(const TYPE &n1, const TYPE &n2, const TYPE &n3, bool FreeThem)
{
	xsize ui = AddCopies(n1, n2, FreeThem);
	AddCopy(n3, FreeThem);
	return ui;
}

template<class TYPE>
xsize XObjArray<TYPE>::AddCopies(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, bool FreeThem)
{
	xsize ui = AddCopies(n1, n2, n3, FreeThem);
	AddCopy(n4, FreeThem);
	return ui;
}

template<class TYPE>
xsize XObjArray<TYPE>::AddCopies(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, const TYPE &n5, bool FreeThem)
{
	xsize ui = AddCopies(n1, n2, n3, n4, FreeThem);
	AddCopy(n5, FreeThem);
	return ui;
}

template<class TYPE>
xsize XObjArray<TYPE>::AddCopies(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, const TYPE &n5, const TYPE &n6, bool FreeThem)
{
	xsize ui = AddCopies(n1, n2, n3, n4, n5, FreeThem);
	AddCopy(n6, FreeThem);
	return ui;
}

template<class TYPE>
xsize XObjArray<TYPE>::AddCopies(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, const TYPE &n5, const TYPE &n6, const TYPE &n7, bool FreeThem)
{
	xsize ui = AddCopies(n1, n2, n3, n4, n5, n6, FreeThem);
	AddCopy(n7, FreeThem);
	return ui;
}

template<class TYPE>
xsize XObjArray<TYPE>::AddCopies(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, const TYPE &n5, const TYPE &n6, const TYPE &n7, const TYPE &n8, bool FreeThem)
{
	xsize ui = AddCopies(n1, n2, n3, n4, n5, n6, n7, FreeThem);
	AddCopy(n8, FreeThem);
	return ui;
}

template<class TYPE>
xsize XObjArray<TYPE>::AddCopies(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, const TYPE &n5, const TYPE &n6, const TYPE &n7, const TYPE &n8, const TYPE &n9, bool FreeThem)
{
	xsize ui = AddCopies(n1, n2, n3, n4, n5, n6, n7, n8, FreeThem);
	AddCopy(n9, FreeThem);
	return ui;
}

template<class TYPE>
xsize XObjArray<TYPE>::AddCopies(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, const TYPE &n5, const TYPE &n6, const TYPE &n7, const TYPE &n8, const TYPE &n9, const TYPE &n10, bool FreeThem)
{
	xsize ui = AddCopies(n1, n2, n3, n4, n5, n6, n7, n8, n9, FreeThem);
	AddCopy(n10, FreeThem);
	return ui;
}

template<class TYPE>
xsize XObjArray<TYPE>::AddCopies(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, const TYPE &n5, const TYPE &n6, const TYPE &n7, const TYPE &n8, const TYPE &n9, const TYPE &n10, const TYPE &n11, bool FreeThem)
{
	xsize ui = AddCopies(n1, n2, n3, n4, n5, n6, n7, n8, n9, n10, FreeThem);
	AddCopy(n11, FreeThem);
	return ui;
}

template<class TYPE>
xsize XObjArray<TYPE>::AddCopies(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, const TYPE &n5, const TYPE &n6, const TYPE &n7, const TYPE &n8, const TYPE &n9, const TYPE &n10, const TYPE &n11, const TYPE &n12, bool FreeThem)
{
	xsize ui = AddCopies(n1, n2, n3, n4, n5, n6, n7, n8, n9, n10, n11, FreeThem);
	AddCopy(n12, FreeThem);
	return ui;
}

template<class TYPE>
xsize XObjArray<TYPE>::AddCopies(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, const TYPE &n5, const TYPE &n6, const TYPE &n7, const TYPE &n8, const TYPE &n9, const TYPE &n10, const TYPE &n11, const TYPE &n12, const TYPE &n13, bool FreeThem)
{
	xsize ui = AddCopies(n1, n2, n3, n4, n5, n6, n7, n8, n9, n10, n11, n12, FreeThem);
	AddCopy(n13, FreeThem);
	return ui;
}

template<class TYPE>
xsize XObjArray<TYPE>::AddCopies(const TYPE &n1, const TYPE &n2, const TYPE &n3, const TYPE &n4, const TYPE &n5, const TYPE &n6, const TYPE &n7, const TYPE &n8, const TYPE &n9, const TYPE &n10, const TYPE &n11, const TYPE &n12, const TYPE &n13, const TYPE &n14, bool FreeThem)
{
	xsize ui = AddCopies(n1, n2, n3, n4, n5, n6, n7, n8, n9, n10, n11, n12, n13, FreeThem);
	AddCopy(n14, FreeThem);
	return ui;
}

/* Add(TYPE *, xsize) */
template<class TYPE>
xsize XObjArrayNC<TYPE>::AddReference(TYPE *newElement, bool FreeIt)
{
	XObjArrayNC<TYPE>::CheckSize(XObjArrayNC<TYPE>::_Len+1);
	XObjArrayNC<TYPE>::_Data[XObjArrayNC<TYPE>::_Len].Object = newElement;
	XObjArrayNC<TYPE>::_Data[XObjArrayNC<TYPE>::_Len].FreeIt = FreeIt;
	XObjArrayNC<TYPE>::_Len += 1;
	return XObjArrayNC<TYPE>::_Len-1;
}

/* Insert(TYPE &, xsize) */
template<class TYPE>
xsize XObjArray<TYPE>::InsertCopy(const TYPE &newElement, xsize pos)
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

/* Insert(TYPE &, xsize) */
template<class TYPE>
xsize XObjArrayNC<TYPE>::InsertRef(TYPE *newElement, xsize pos, bool FreeIt)
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

/* SetFreeIt(xsize, bool) */
template<class TYPE>
void XObjArrayNC<TYPE>::SetFreeIt(xsize nIndex, bool Flag)
{
	if ( nIndex  < XObjArrayNC<TYPE>::_Len )
	{
		XObjArrayNC<TYPE>::_Data[nIndex].FreeIt = Flag;
	}
	else{
		#if defined(_DEBUG)
			throw "XObjArray::SetFreeIt(xsize) -> nIndex >= _Len\n";
		#endif
	}
}

/* SetFreeIt(const TYPE *Element, bool) */
template<class TYPE>
void XObjArrayNC<TYPE>::SetFreeIt(const TYPE *Element, bool Flag)
{
 xsize i;

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

/* Remove(xsize) */
template<class TYPE>
void XObjArrayNC<TYPE>::RemoveAtIndex(xsize nIndex)
{
	if ( nIndex  < XObjArrayNC<TYPE>::_Len )
	{
  	if ( nIndex >= XObjArrayNC<TYPE>::_Len ) {
		DebugLog(2, "void XObjArrayNC<TYPE>::RemoveAtIndex(xsize nIndex) : BUG nIndex (%llu) is > length(). System halted\n", nIndex);
	  	panic();
	  }
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
/* RemoveWithoutFreeing(xsize) */
template<class TYPE>
void XObjArrayNC<TYPE>::RemoveWithoutFreeing(xsize nIndex)
{
	if ( nIndex < _Len )
	{
		if ( nIndex<_Len-1 ) memcpy(&_Data[nIndex], &_Data[nIndex+1], (_Len-nIndex-1)*sizeof(XObjArrayEntry<TYPE>));
		_Len -= 1;
		return;
	}
	#if defined(_DEBUG)
		throw "XObjArray::RemoveWithoutFreeing(xsize) -> nIndex > _Len\n";
	#endif
}

//-------------------------------------------------------------------------------------------------
//                                               
//-------------------------------------------------------------------------------------------------
/* Remove(int) */
template<class TYPE>
void XObjArrayNC<TYPE>::RemoveAtIndex(int nIndex)
{
  #if defined(__XTOOLS_INT_CHECK__)
  	if ( nIndex < 0 ) {
  	  DebugLog(2, "XArray<TYPE>::RemoveAtIndex(int nIndex) : BUG nIndex (%d) is < 0. System halted\n", nIndex);
	  	panic();
	  }
	#endif
	RemoveAtIndex( (xsize)nIndex ); // Remove(xsize) will check that index is < _Len
}

/* Remove(const TYPE &) */
template<class TYPE>
void XObjArrayNC<TYPE>::Remove(const TYPE &Element)
{
  xsize i;

	for ( i=0 ; i<_Len ; i+= 1) {
		if ( *((TYPE *)(_Data[i].Object)) == Element ) {
			RemoveAtIndex(i);
			return ;
		}
	}
	#if defined(_DEBUG)
		DebugLog(2, "XObjArray::Remove(TYPE &) -> Not found\n");
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
  xsize i;

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
  xsize i;

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
  xsize i;

	for ( i=_Len ; i-- ; ) {
		if ( ((TYPE *)_Data[i].Object) != Element ) {
			Remove(i);
		}
	}
}

/* Empty() */
template<class TYPE>
void XObjArrayNC<TYPE>::Empty()
{
  xsize i;

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
