/*
 * MemoryTracker.h
 *
 *  Created on: Nov 9, 2023
 *      Author: jief
 */

#ifndef CPP_LIB_MEMORYTRACKER_H_
#define CPP_LIB_MEMORYTRACKER_H_

#ifdef JIEF_DEBUG
#define MEMORY_TRACKER_ENABLED
#endif



extern "C" {
void MemoryTrackerInit();
void MemoryTrackerInstallHook();
}
void MT_outputDanglingPtr();
uint64_t MT_getAllocCount();
uint64_t MT_getDanglingPtrCount();
void MemoryTrackerCheck();

class MemoryStopRecord
{
protected:
  bool recording;
public:
  MemoryStopRecord();
  ~MemoryStopRecord();
};



#ifdef MEMORY_TRACKER_ENABLED
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//
//                                          MTArray
//
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx


template<class TYPE>
class MTArray
{
  protected:
	TYPE*  m_data;
	size_t m_len;
	size_t m_allocatedSize;

  public:

	MTArray();

	size_t length() const { return m_len; }

////--------------------------------------------------

  TYPE &operator[](size_t index)
  {
    #ifdef JIEF_DEBUG
      if ( index < 0 ) {
        panic("MTArray::ElementAt(int) -> Operator [] : index < 0");
      }
      if ( (unsigned int)index >= m_len ) { // cast safe, index > 0
        panic("MTArray::ElementAt(int) -> Operator [] : index > m_len");
      }
    #endif
    return  m_data[index];
  }

	void CheckSize(size_t nNewSize);
	void CheckSize(size_t nNewSize, size_t nGrowBy);

	size_t Add(const TYPE newElement, size_t count = 1);
	void RemoveAtIndex(size_t nIndex);
	void FreeAndRemoveAtIndex(size_t nIndex);
  size_t indexOf(TYPE& e) const;
};


//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//
//
//
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

extern MTArray<uintptr_t> allocatedPtrArray;
extern MTArray<uint64_t> allocatedPtrInfoArray;


#endif



#endif /* CPP_LIB_MEMORYTRACKER_H_ */
