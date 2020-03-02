//*************************************************************************************************
//*************************************************************************************************
//
//                                          XArray
//
// Developed by jief666, from 1997.
//
//*************************************************************************************************
//*************************************************************************************************

#if !defined(__XARRAY_H__)
#define __XARRAY_H__

#include "XToolsCommon.h"


#if 0
#define XArray_DBG(...) DebugLog(2, __VA_ARGS__)
#else
#define XArray_DBG(...)
#endif

template<class TYPE>
class XArray
{
  protected:
	TYPE *m_data;
	xsize m_len;
	xsize m_size;
	xsize _GrowBy;

  public:
	void Init();
	XArray() { Init(); }
	XArray(const XArray<TYPE> &anArray);
	const XArray<TYPE> &operator =(const XArray<TYPE> &anArray);
	virtual ~XArray();

  public:
	const TYPE *Data() const { return m_data; }
	TYPE *Data() { return m_data; }

  public:
	xsize Size() const { return m_size; }
	xsize Length() const { return m_len; }
	void SetLength(xsize l);

  //low case functions like in std::vector
  xsize size() const { return m_len; }
  const TYPE& begin() const { return ElementAt(0); }
        TYPE& begin()       { return ElementAt(0); }

  const TYPE& end() const { return ElementAt(m_len - 1); }
        TYPE& end()       { return ElementAt(m_len - 1); }

  xsize insert(const TYPE newElement, xsize pos, xsize count = 1) { return Insert(newElement, pos, count); }
 
//--------------------------------------------------

	const TYPE& ElementAt(xsize nIndex) const;
	TYPE& ElementAt(xsize nIndex);

	const TYPE& operator[](xsize nIndex) const { return ElementAt(nIndex); }
	      TYPE& operator[](xsize nIndex)       { return ElementAt(nIndex); }
	const TYPE& operator[]( int nIndex)  const { return ElementAt(nIndex); }
	      TYPE& operator[]( int nIndex)        { return ElementAt(nIndex); }

	operator const void *() const { return m_data; };
	operator       void *()       { return m_data; };
	operator const TYPE *() const { return m_data; };
	operator       TYPE *()       { return m_data; };

	const TYPE * operator +( int i) const { return m_data+i; };
	      TYPE * operator +( int i)       { return m_data+i; };
	const TYPE * operator +(xsize i) const { return m_data+i; };
	      TYPE * operator +(xsize i)       { return m_data+i; };
	const TYPE * operator -( int i) const { return m_data-i; };
	      TYPE * operator -( int i)       { return m_data-i; };
	const TYPE * operator -(xsize i) const { return m_data-i; };
	      TYPE * operator -(xsize i)       { return m_data-i; };

//	xsize Add(const TYPE newElement);
//	TYPE         AddNew();
//	xsize Inserts(const TYPE &newElement, xsize pos, xsize count);

	void CheckSize(xsize nNewSize);
	void CheckSize(xsize nNewSize, xsize nGrowBy);

	xsize AddUninitialized(xsize count); // add count uninitialzed elements
	xsize Add(const TYPE newElement, xsize count = 1);
	xsize AddArray(const TYPE *newElements, xsize count = 1);
	xsize Insert(const TYPE newElement, xsize pos, xsize count = 1);

	void Remove(const TYPE *Element);
	void Remove(const TYPE &Element);
	void RemoveAtIndex(xsize nIndex);
	void RemoveAtIndex(int nIndex);

	void setEmpty();
  bool isEmpty() const { return size() == 0; }
    
  xsize IdxOf(TYPE& e) const;
	bool ExistIn(TYPE& e) const { return IdxOf(e) != MAX_XSIZE; }
};

//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//
//                                          XArray
//
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

template<class TYPE>
xsize XArray<TYPE>::IdxOf(TYPE& e) const
{
  xsize i;

	for ( i=0 ; i<Length() ; i+=1 ) {
		if ( ElementAt(i) == e ) return i;
	}
	return MAX_XSIZE;
}

/* Constructeur */
template<class TYPE>
void XArray<TYPE>::Init()
{
	m_data = nullptr;
	m_size = 0;
	m_len = 0;
	_GrowBy = XArrayGrowByDefault;
}

/* Constructeur */
template<class TYPE>
XArray<TYPE>::XArray(const XArray<TYPE> &anArray)
{
	Init();
	Add(anArray.Data(), anArray.Length());
}

/* operator = */
template<class TYPE>
const XArray<TYPE> &XArray<TYPE>::operator =(const XArray<TYPE> &anArray)
{
  xsize ui;

	setEmpty();
	for ( ui=0 ; ui<anArray.Length() ; ui+=1 ) Add(anArray[ui] );
	return *this;
}

/* Destructeur */
template<class TYPE>
XArray<TYPE>::~XArray()
{
//printf("XArray Destructor\n");
	if ( m_data ) Xfree(m_data);
}

/* CheckSize() */
template<class TYPE>
void XArray<TYPE>::CheckSize(xsize nNewSize, xsize nGrowBy)
{
//XArray_DBG("CheckSize: m_len=%d, m_size=%d, nGrowBy=%d, nNewSize=%d\n", m_len, m_size, nGrowBy, nNewSize);
	if ( nNewSize > m_size ) {
		nNewSize += nGrowBy;
		m_data = (TYPE *)Xrealloc( m_size * sizeof(TYPE), nNewSize * sizeof(TYPE), (void *)m_data);
		if ( !m_data ) {
  		DebugLog(2, "XArray<TYPE>::CheckSize(nNewSize=%llu, nGrowBy=%llu) : Xrealloc(%d, %d, %d) returned NULL. System halted\n", nNewSize, nGrowBy, m_size, nNewSize*sizeof(TYPE), m_data);
	  	CpuDeadLoop();
		}
//		memset(&_Data[_Size], 0, (nNewSize-_Size) * sizeof(TYPE)); // Could help for debugging, but zeroing in not needed.
		m_size = nNewSize;
	}
}

/* CheckSize() */
template<class TYPE>
void XArray<TYPE>::CheckSize(xsize nNewSize)
{
	CheckSize(nNewSize, XArrayGrowByDefault);
}

/* SetLength (xsize i) */
template<class TYPE>
void XArray<TYPE>::SetLength(xsize l)
{
	m_len = l;
	#ifdef DEBUG
		if(m_len > m_size) {
			DebugLog(2, "XArray::SetLength(xsize) -> _Len > _Size");
			CpuDeadLoop();
		}
	#endif
}


/* ElementAt() */
template<class TYPE>
TYPE &XArray<TYPE>::ElementAt(xsize index)
{
	#ifdef _DEBUG
		if ( index >= m_len ) {
			DebugLog(2, "XArray::ElementAt(xsize) -> Operator [] : index > m_len");
			CpuDeadLoop();
		}
	#endif
	return  m_data[index];
}

/* ElementAt() */
template<class TYPE>
const TYPE& XArray<TYPE>::ElementAt(xsize index) const
{
	#ifdef _DEBUG
		if ( index >= m_len ) {
			DebugLog(2, "XArray::ElementAt(xsize) const -> Operator [] : index > m_len");
			CpuDeadLoop();
		}
	#endif
	return  m_data[index];
}

/* Add(xsize) */
template<class TYPE>
xsize XArray<TYPE>::AddUninitialized(xsize count)
{
	CheckSize(m_len+count);
	m_len += count;
	return m_len-count;
}

/* Add(TYPE, xsize) */
template<class TYPE>
xsize XArray<TYPE>::Add(const TYPE newElement, xsize count)
{
//  XArray_DBG("xsize XArray<TYPE>::Add(const TYPE newElement, xsize count) -> Enter. count=%d _Len=%d _Size=%d\n", count, m_len, m_size);
  xsize i;

	CheckSize(m_len+count);
	for ( i=0 ; i<count ; i++ ) {
		m_data[m_len+i] = newElement;
	}
	m_len += count;
	return m_len-count;
}

/* Add(TYPE *, xsize) */
template<class TYPE>
xsize XArray<TYPE>::AddArray(const TYPE *newElements, xsize count)
{
  xsize i;

	CheckSize(m_len+count);
	for ( i=0 ; i<count ; i++ ) {
		m_data[m_len+i] = newElements[i];
	}
	m_len += count;
	return m_len-count;
}

/* Insert(TYPE &, xsize, xsize) */
template<class TYPE>
xsize XArray<TYPE>::Insert(const TYPE newElement, xsize pos, xsize count)
{
  xsize i;

	if ( pos  < m_len ) {
		CheckSize(m_len+count);
		memmove(&m_data[pos], &m_data[pos+count], (m_len-pos)*sizeof(TYPE));
		for ( i=0 ; i<count ; i++ ) {
			m_data[pos+i] = newElement;
		}
		m_len += count;
		return pos;
	}else{
		return Add(newElement, count);
	}
}

/* Remove(xsize) */
template<class TYPE>
void XArray<TYPE>::RemoveAtIndex(xsize nIndex)
{
	if ( nIndex  < m_len ) {
		if ( nIndex<m_len-1 ) Xmemmove(&m_data[nIndex], &m_data[nIndex+1], (m_len-nIndex-1)*sizeof(TYPE));
		m_len -= 1;
		return;
	}
	#if defined(_DEBUG) && defined(TRACE)
		TRACE("XArray::Remove(xsize) -> nIndex > m_len\n");
	#endif
}

/* Remove(int) */
template<class TYPE>
void XArray<TYPE>::RemoveAtIndex(int nIndex)
{
  #if defined(__XTOOLS_INT_CHECK__)
  	if ( nIndex < 0 ) {
  	  DebugLog(2, "XArray<TYPE>::RemoveAtIndex(int nIndex) : BUG nIndex (%d) is < 0. System halted\n", nIndex);
	  	CpuDeadLoop();
	  }
	#endif

	RemoveAtIndex( (xsize)nIndex ); // Check of nIndex is made in Remove(xsize nIndex)
	return;
}

/* Remove(TYPE) */
template<class TYPE>
void XArray<TYPE>::Remove(const TYPE &Element)
{
  xsize i;

	for ( i=0 ; i<m_len ; i+= 1) {
		if ( m_data[i] == Element ) {
			Remove(i);
			return ;
		}
	}
	#if defined(_DEBUG) && defined(TRACE)
		TRACE("XArray::Remove(const TYPE &) -> Element not found\n");
	#endif
}

/* Remove(TYPE *) */
template<class TYPE>
void XArray<TYPE>::Remove(const TYPE *Element)
{
		Remove(*Element);
}

/* Empty() */
template<class TYPE>
void XArray<TYPE>::setEmpty()
{
//printf("XArray Empty\n");
	m_len = 0;
}

#endif
