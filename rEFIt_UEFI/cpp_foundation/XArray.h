/*
 *
 * Created by jief in 1997.
 * Copyright (c) 2020 Jief
 * All rights reserved.
 *
 */

#if !defined(__XARRAY_H__)
#define __XARRAY_H__

#include <XToolsConf.h>
#include "XToolsCommon.h"


#if 0
#define XArray_DBG(...) printf(__VA_ARGS__)
#else
#define XArray_DBG(...)
#endif

template<class TYPE>
class XArray
{
  protected:
	TYPE*  m_data;
	size_t m_len;
	size_t m_allocatedSize;

  public:
//	void Init();
	XArray() : m_data(0), m_len(0), m_allocatedSize(0) { }
	XArray(const XArray<TYPE> &anArray);
	const XArray<TYPE> &operator =(const XArray<TYPE> &anArray);
	virtual ~XArray();

  public:
	const TYPE *data() const { return m_data; }
	TYPE *data() { return m_data; }

  public:
	size_t allocatedSize() const { return m_allocatedSize; }
	size_t length() const { return m_len; }
	size_t size() const { return m_len; }
	void  setSize(size_t l);

  //low case functions like in std::vector

  const TYPE& begin() const { if ( m_len == 0 ) panic("m_len == 0"); return ElementAt(0); }
        TYPE& begin()       { if ( m_len == 0 ) panic("m_len == 0"); return ElementAt(0); }

  const TYPE& end() const { if ( m_len == 0 ) panic("m_len == 0"); return ElementAt(m_len - 1); }
        TYPE& end()       { if ( m_len == 0 ) panic("m_len == 0"); return ElementAt(m_len - 1); }

  size_t insert(const TYPE newElement, size_t pos, size_t count = 1) { return Insert(newElement, pos, count); }
 
//--------------------------------------------------

  template<typename IntegralType, enable_if(is_integral(IntegralType))>
	const TYPE& ElementAt(IntegralType index) const
  {
    #ifdef DEBUG
      if ( index < 0 ) {
        panic("XArray::ElementAt(int) -> Operator [] : index < 0");
      }
      if ( (unsigned int)index >= m_len ) { // cast safe, index > 0
        panic("XArray::ElementAt(int) -> Operator [] : index > m_len");
      }
    #endif
    return  m_data[index];
  }
  template<typename IntegralType, enable_if(is_integral(IntegralType))>
	TYPE& ElementAt(IntegralType index) { return const_cast<TYPE&>(const_cast<const XArray<TYPE>*>(this)->ElementAt(index)); }

  template<typename IntegralType, enable_if(is_integral(IntegralType))>
  const TYPE &operator[](IntegralType nIndex) const { return ElementAt(nIndex); }
  template<typename IntegralType, enable_if(is_integral(IntegralType))>
  TYPE &operator[](IntegralType nIndex) { return ElementAt(nIndex); }

  bool operator==(const XArray<TYPE>& other) const
  {
    if ( size() != other.size() ) return false;
    for ( size_t idx = 0 ; idx < other.size() ; ++idx ) {
      if ( !( ElementAt(idx) == other.ElementAt(idx) ) ) return false;
    }
    return true;
  }
  bool isEqual(const XArray<TYPE>& other) const
  {
    if ( size() != other.size() ) return false;
    for ( size_t idx = 0 ; idx < other.size() ; ++idx ) {
      if ( !( ElementAt(idx).isEqual(other.ElementAt(idx)) ) ) return false;
    }
    return true;
  }



	operator const void *() const { return m_data; };
	operator       void *()       { return m_data; };
	operator const TYPE *() const { return m_data; };
	operator       TYPE *()       { return m_data; };

	const TYPE * operator +( int i) const { return m_data+i; };
	      TYPE * operator +( int i)       { return m_data+i; };
	const TYPE * operator +(size_t i) const { return m_data+i; };
	      TYPE * operator +(size_t i)       { return m_data+i; };
	const TYPE * operator -( int i) const { return m_data-i; };
	      TYPE * operator -( int i)       { return m_data-i; };
	const TYPE * operator -(size_t i) const { return m_data-i; };
	      TYPE * operator -(size_t i)       { return m_data-i; };

//	size_t Add(const TYPE newElement);
//	TYPE         AddNew();
//	size_t Inserts(const TYPE &newElement, size_t pos, size_t count);

	void CheckSize(size_t nNewSize);
	void CheckSize(size_t nNewSize, size_t nGrowBy);

	size_t AddUninitialized(size_t count); // add count uninitialzed elements
	size_t Add(const TYPE newElement, size_t count = 1);
	size_t AddArray(const TYPE *newElements, size_t count = 1);
	size_t Insert(const TYPE newElement, size_t pos, size_t count = 1);

	void Remove(const TYPE *Element);
	void Remove(const TYPE &Element);
	void RemoveAtIndex(size_t nIndex);
	void RemoveAtIndex(int nIndex);

	void setEmpty();
	bool isEmpty() const { return size() == 0; }
    
  size_t indexOf(TYPE& e) const;
	bool contains(TYPE& e) const { return indexOf(e) != MAX_XSIZE; } //logically it should be named as Contains(e)
};

//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//
//                                          XArray
//
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

template<class TYPE>
size_t XArray<TYPE>::indexOf(TYPE& e) const
{
  size_t i;

	for ( i=0 ; i<size() ; i+=1 ) {
		if ( ElementAt(i) == e ) return i;
	}
	return MAX_XSIZE;
}

///* Constructeur */
//template<class TYPE>
//void XArray<TYPE>::Init()
//{
//	m_data = nullptr;
//	m_allocatedSize = 0;
//	m_len = 0;
//	_GrowBy = XArrayGrowByDefault;
//}

/* Constructeur */
template<class TYPE>
XArray<TYPE>::XArray(const XArray<TYPE> &anArray) : m_data(0), m_len(0), m_allocatedSize(0)
{
//	Init();
	AddArray(anArray.data(), anArray.size());
}

/* operator = */
template<class TYPE>
const XArray<TYPE> &XArray<TYPE>::operator =(const XArray<TYPE> &anArray)
{
  if ( anArray.length() > 0 )
  {
  	CheckSize(anArray.length(), 0);
  	memcpy(m_data, anArray.m_data, anArray.m_len * sizeof(TYPE));
  	m_len = anArray.m_len;
  }
  else
  {
  	setEmpty();
  }
//
//
//size_t ui;
//
//	setEmpty();
//	for ( ui=0 ; ui<anArray.length() ; ui+=1 ) Add(anArray[ui] );
	return *this;
}

/* Destructeur */
template<class TYPE>
XArray<TYPE>::~XArray()
{
//printf("XArray Destructor\n");
	if ( m_data ) free(m_data);
}

/* CheckSize()  // nNewSize is number of TYPE, not in bytes */
template<class TYPE>
void XArray<TYPE>::CheckSize(size_t nNewSize, size_t nGrowBy)
{
//XArray_DBG("CheckSize: m_len=%d, m_size=%d, nGrowBy=%d, nNewSize=%d\n", m_len, m_size, nGrowBy, nNewSize);
	if ( nNewSize > m_allocatedSize ) {
		nNewSize += nGrowBy;
		m_data = (TYPE *)Xrealloc((void *)m_data, nNewSize * sizeof(TYPE), m_allocatedSize * sizeof(TYPE) );
		if ( !m_data ) {
#ifdef DEBUG
			panic("XArray<TYPE>::CheckSize(nNewSize=%zu, nGrowBy=%zu) : Xrealloc(%zu, %lu, %" PRIuPTR ") returned NULL. System halted\n", nNewSize, nGrowBy, m_allocatedSize, nNewSize*sizeof(TYPE), (uintptr_t)m_data);
#endif
		}
//		memset(&_Data[_Size], 0, (nNewSize-_Size) * sizeof(TYPE)); // Could help for debugging, but zeroing in not needed.
		m_allocatedSize = nNewSize;
	}
}

/* CheckSize() */
template<class TYPE>
void XArray<TYPE>::CheckSize(size_t nNewSize)
{
	CheckSize(nNewSize, XArrayGrowByDefault);
}

/* SetLength (size_t i) */
template<class TYPE>
void XArray<TYPE>::setSize(size_t l)
{
	CheckSize(l, 0); // be sure the size is allocated
	m_len = l;
	#ifdef DEBUG
		if(m_len > m_allocatedSize) {
			panic("XArray::SetLength(size_t) -> _Len > _Size");
		}
	#endif
}


/* Add(size_t) */
template<class TYPE>
size_t XArray<TYPE>::AddUninitialized(size_t count)
{
	CheckSize(m_len+count);
	m_len += count;
	return m_len-count;
}

/* Add(TYPE, size_t) */
template<class TYPE>
size_t XArray<TYPE>::Add(const TYPE newElement, size_t count)
{
//  XArray_DBG("size_t XArray<TYPE>::Add(const TYPE newElement, size_t count) -> Enter. count=%d _Len=%d _Size=%d\n", count, m_len, m_size);
  size_t i;

	CheckSize(m_len+count);
	for ( i=0 ; i<count ; i++ ) {
		m_data[m_len+i] = newElement;
	}
	m_len += count;
	return m_len-count;
}

/* Add(TYPE *, size_t) */
template<class TYPE>
size_t XArray<TYPE>::AddArray(const TYPE *newElements, size_t count)
{
  size_t i;

	CheckSize(m_len+count);
	for ( i=0 ; i<count ; i++ ) {
		m_data[m_len+i] = newElements[i];
	}
	m_len += count;
	return m_len-count;
}

/* Insert(TYPE &, size_t, size_t) */
template<class TYPE>
size_t XArray<TYPE>::Insert(const TYPE newElement, size_t pos, size_t count)
{
  size_t i;

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

/* Remove(size_t) */
template<class TYPE>
void XArray<TYPE>::RemoveAtIndex(size_t nIndex)
{
	if ( nIndex  < m_len ) {
		if ( nIndex<m_len-1 ) memmove(&m_data[nIndex], &m_data[nIndex+1], (m_len-nIndex-1)*sizeof(TYPE));
		m_len -= 1;
		return;
	}
	#if defined(_DEBUG) && defined(TRACE)
		TRACE("XArray::Remove(size_t) -> nIndex > m_len\n");
	#endif
}

/* Remove(int) */
template<class TYPE>
void XArray<TYPE>::RemoveAtIndex(int nIndex)
{
  #if defined(__XTOOLS_CHECK_OVERFLOW__)
  	if ( nIndex < 0 ) {
  	  panic("XArray<TYPE>::RemoveAtIndex(int nIndex) : BUG nIndex (%d) is < 0. System halted\n", nIndex);
	}
	#endif

	RemoveAtIndex( (size_t)nIndex ); // Check of nIndex is made in Remove(size_t nIndex)
	return;
}

/* Remove(TYPE) */
template<class TYPE>
void XArray<TYPE>::Remove(const TYPE &Element)
{
  size_t i;

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
