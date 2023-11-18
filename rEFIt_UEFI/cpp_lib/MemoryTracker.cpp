/*
 * MemoryTracker.cpp
 *
 *  Created on: Nov 9, 2023
 *      Author: jief
 */

#include <Platform.h>

#include "MemoryTracker.h"
#include "../cpp_foundation/XString.h"

#ifndef DEBUG_ALL
#define DEBUG_MT 1
#else
#define DEBUG_MT DEBUG_ALL
#endif

#if DEBUG_MT == 0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_MT, __VA_ARGS__)
#endif

#ifdef MEMORY_TRACKER_ENABLED


uint64_t MT_alloc_count = 0;
bool MT_recording = false;

MTArray<uintptr_t> allocatedPtrArray;
MTArray<uint64_t> allocatedPtrInfoArray;

void MT_import(const void* p)
{
//  printf("MT_import ptr %llx\n", uintptr_t(p));
  MT_recording = false;
  auto count = MT_alloc_count;
  allocatedPtrArray.Add(uintptr_t(p)); // this can allocate memory, so don't record that.
  allocatedPtrInfoArray.Add(count); // this can allocate memory, so don't record that.
  MT_recording = true;
}
//
//void MT_delete(const void* p)
//{
//  if ( p ) {
//    uintptr_t ref2 = uintptr_t(p);
//    auto idx = allocatedPtrArray.indexOf(ref2);
//    if ( idx == MAX_XSIZE ) {
////      printf("MT_delete unknown ptr %llx\n", uintptr_t(p));
//    }else{
////if ( allocatedPtrInfoArray[idx]->alloc_idx == 15579 ) printf("MT_delete(0x%llx) - %lld\n", uintptr_t(p), allocatedPtrInfoArray[idx]->alloc_idx);
//      allocatedPtrArray.RemoveAtIndex(idx);
//      allocatedPtrInfoArray.RemoveAtIndex(idx);
//    }
//  }
//}


extern "C" {

void* real_malloc(size_t size);
void* real_type_malloc(EFI_MEMORY_TYPE MemoryType, size_t size);
void* my_malloc(size_t size);
void real_free(void*);
void my_free(void*);

//#define MAGIC_BEGINNING 0xDEADBEEF32659821
//#define MAGIC_END 0x1245788956231973

#define MAGIC_BEGINNING 0xDEADBEEFBBBBBBBB
#define MAGIC_END 0xEEEEEEEEDEADBEEF

uint64_t MT_count_to_break_into = 0;

#if defined(IS_UEFI_MODULE)

VOID *
InternalAllocatePool (
  IN EFI_MEMORY_TYPE  MemoryType,
  IN UINTN            AllocationSize
  )
#else

void* my_malloc(size_t AllocationSize)

#endif
{
  VOID        *Memory = NULL;

#ifdef JIEF_DEBUG
if ( MT_alloc_count == 4028 || MT_alloc_count == MT_count_to_break_into ) {
NOP;
}
#endif
  if ( MT_recording )
  {
      Memory = real_type_malloc(MemoryType, AllocationSize+24);
      if ( !Memory ) return NULL;

      *(uint64_t*)Memory = AllocationSize;

    //  void* p = ((uint64_t*)Memory + 1) ;

      *((uint64_t*)Memory + 1) = MAGIC_BEGINNING;
      *(uint64_t*)( ((uint8_t*)Memory) + 16 + AllocationSize ) = MAGIC_END;
      if ( MT_recording ) {
        MT_import( ((uint8_t*)Memory) + 16 );
        ++MT_alloc_count; // do this only if !MT_recording because MT_import is doing an allocation.
      }
      return ((uint8_t*)Memory) + 16;

  }else{
      Memory = real_type_malloc(MemoryType, AllocationSize);
      return Memory;
  }
}


#if defined(IS_UEFI_MODULE)

void (EFIAPI FreePool)(IN void *MemoryPlus16)

#else

void my_free(IN void *MemoryPlus16)

#endif
{
  if ( !MemoryPlus16 ) return;

  uintptr_t ref2 = uintptr_t(MemoryPlus16);
  auto idx = allocatedPtrArray.indexOf(ref2);
  if ( idx == MAX_XSIZE ) {
    //DBG("Delete non recorded ptr %llx\n", uintptr_t(MemoryPlus16));
    real_free(MemoryPlus16);
    return;
  }

  uint8_t* Memory = ((uint8_t*)MemoryPlus16) - 16;
  uint64_t AllocationSize = *(uint64_t*)Memory;

  if ( *((uint64_t*)Memory + 1) != MAGIC_BEGINNING ) {
    DBG("Buffer underrun for ptr %llx count %lld\n", uintptr_t(MemoryPlus16), allocatedPtrInfoArray[idx]);
  }
  if ( *(uint64_t*)( ((uint8_t*)Memory) + 16 + AllocationSize ) != MAGIC_END ) {
    DBG("Buffer overrun for ptr %llx count %lld\n", uintptr_t(MemoryPlus16), allocatedPtrInfoArray[idx]);
  }
  allocatedPtrArray.RemoveAtIndex(idx);
  allocatedPtrInfoArray.RemoveAtIndex(idx);
  real_free( Memory );
}

} // extern "C"


void MemoryTrackerCheck()
{
  size_t nb_ptr = allocatedPtrArray.length();
  DBG("-- %zu pointers :\n", nb_ptr);
  for( size_t idx=0 ; idx < nb_ptr ; ++idx )
  {
    uint8_t* Memory = ((uint8_t*)allocatedPtrArray[idx]) - 16;
    uint64_t AllocationSize = *(uint64_t*)Memory;

    if ( *((uint64_t*)Memory + 1) != MAGIC_BEGINNING ) {
      DBG("Buffer underrun for ptr %llx count %lld\n", uintptr_t(Memory + 16), allocatedPtrInfoArray[idx]);
    }
    if ( *(uint64_t*)( ((uint8_t*)Memory) + 16 + AllocationSize ) != MAGIC_END ) {
      DBG("Buffer overrun for ptr %llx count %lld\n", uintptr_t(Memory + 16), allocatedPtrInfoArray[idx]);
    }
  }
}

void MemoryTrackerInit()
{
  MT_alloc_count = 0;
  MT_recording = false;
}


/*
 * In fact, the hook is already in place. We just have to activate it now.
 */
void MemoryTrackerInstallHook()
{
  MT_recording = true;
}

void MT_outputDanglingPtr()
{
  size_t nb_ptr = allocatedPtrArray.length();
  DBG("-- %zu lost pointer :\n", nb_ptr);
  for( size_t idx=0 ; idx < nb_ptr ; ++idx ) {
    DBG("    Dangling ptr %llx count=%lld\n", allocatedPtrArray[idx], allocatedPtrInfoArray[idx]);
  }
}
uint64_t MT_getAllocCount() { return MT_alloc_count; };
uint64_t MT_getDanglingPtrCount() { return allocatedPtrArray.length(); };



MemoryStopRecord::MemoryStopRecord() : recording(MT_recording) {  MT_recording = false; };
MemoryStopRecord::~MemoryStopRecord() { MT_recording = recording; };






//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//
//                                          MTArray
//
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

template<class TYPE>
MTArray<TYPE>::MTArray() : m_data(0), m_len(0), m_allocatedSize(4096)
{
  m_data = (TYPE*)real_malloc(4096 * sizeof(TYPE));
  if ( !m_data ) {
#ifdef JIEF_DEBUG
    panic("MTArray<TYPE>:: MTArray() : OldAllocatePool(%zu) returned NULL. System halted\n", 4096 * sizeof(TYPE));
#endif
  }
}


template<class TYPE>
size_t MTArray<TYPE>::indexOf(TYPE& e) const
{
  size_t i;

	for ( i=0 ; i<length() ; i+=1 ) {
		if ( m_data[i] == e ) return i;
	}
	return MAX_XSIZE;
}

/* CheckSize()  // nNewSize is number of TYPE, not in bytes */
template<class TYPE>
void MTArray<TYPE>::CheckSize(size_t nNewSize, size_t nGrowBy)
{
//MTArray_DBG("CheckSize: m_len=%d, m_size=%d, nGrowBy=%d, nNewSize=%d\n", m_len, m_size, nGrowBy, nNewSize);
	if ( nNewSize > m_allocatedSize ) {
		nNewSize += nGrowBy;
    TYPE* new_data;
      new_data = (TYPE*)real_malloc(nNewSize * sizeof(TYPE));
		if ( !new_data ) {
#ifdef JIEF_DEBUG
			panic("MTArray<TYPE>::CheckSize(nNewSize=%zu, nGrowBy=%zu) : OldAllocatePool(%zu, %lu, %" PRIuPTR ") returned NULL. System halted\n", nNewSize, nGrowBy, m_allocatedSize, nNewSize*sizeof(TYPE), (uintptr_t)m_data);
#endif
		}
    memcpy(new_data, m_data, m_allocatedSize * sizeof(TYPE));
    real_free(m_data);
    m_data = new_data;
//		memset(&_Data[_Size], 0, (nNewSize-_Size) * sizeof(TYPE)); // Could help for debugging, but zeroing in not needed.
		m_allocatedSize = nNewSize;
	}
}

/* CheckSize() */
template<class TYPE>
void MTArray<TYPE>::CheckSize(size_t nNewSize)
{
	CheckSize(nNewSize, 4096);
}

/* Add(TYPE, size_t) */
template<class TYPE>
size_t MTArray<TYPE>::Add(const TYPE newElement, size_t count)
{
//  MTArray_DBG("size_t MTArray<TYPE>::Add(const TYPE newElement, size_t count) -> Enter. count=%d _Len=%d _Size=%d\n", count, m_len, m_size);
  size_t i;

	CheckSize(m_len+count);
	for ( i=0 ; i<count ; i++ ) {
		m_data[m_len+i] = newElement;
	}
	m_len += count;
	return m_len-count;
}


/* RemoveAtIndex(size_t) */
template<class TYPE>
void MTArray<TYPE>::RemoveAtIndex(size_t nIndex)
{
  if ( nIndex  < m_len ) {
    if ( nIndex<m_len-1 ) memmove(&m_data[nIndex], &m_data[nIndex+1], (m_len-nIndex-1)*sizeof(TYPE));
    m_len -= 1;
    return;
  }
  #if defined(_DEBUG) && defined(TRACE)
    TRACE("MTArray::Remove(size_t) -> nIndex > m_len\n");
  #endif
}


/* FreeAndRemoveAtIndex(size_t) */
template<class TYPE>
void MTArray<TYPE>::FreeAndRemoveAtIndex(size_t nIndex)
{
  if ( nIndex  < m_len ) {
#ifdef UNIT_TESTS_MACOS // won't be needed soon as I'll improve the EFI mock
    free(m_data[nIndex]);
#else
    OldFreePool(m_data[nIndex]);
#endif
    if ( nIndex<m_len-1 ) memmove(&m_data[nIndex], &m_data[nIndex+1], (m_len-nIndex-1)*sizeof(TYPE));
    m_len -= 1;
    return;
  }
  #if defined(_DEBUG) && defined(TRACE)
    TRACE("MTArray::FreeAndRemoveAtIndex(size_t) -> nIndex > m_len\n");
  #endif
}


#undef malloc
#undef free


void* real_type_malloc(EFI_MEMORY_TYPE MemoryType, size_t size)
{
  #if defined(IS_UEFI_MODULE)
    void* Memory;
    EFI_STATUS Status = gBS->AllocatePool(MemoryType, size, &Memory); // AllocatePool in MemoryAllocationLib uses EfiBootServicesData, not EfiConventionalMemory
    if (EFI_ERROR(Status)) return NULL;
    return Memory;
  #else
    return malloc(AllocationSize);
  #endif
}
void* real_malloc(size_t size) { return real_type_malloc(EfiBootServicesData, size); }

void real_free(void* p)
{
  #if defined(IS_UEFI_MODULE)
    EFI_STATUS Status = gBS->FreePool(p);
    if (EFI_ERROR(Status)) {
        // What to do ?
    }
  #else
    free(p);
  #endif
}



#if !defined(IS_UEFI_MODULE)

extern "C" {

#if !defined(IS_UEFI_MODULE)
void* malloc(size_t size);
void free(void* p);
#endif

} //extern C

#endif // !defined(IS_UEFI_MODULE)


#else // MEMORY_TRACKER_ENABLED

void MemoryTrackerInit() {};
void MemoryTrackerInstallHook() {};
void MT_outputDanglingPtr() {};
uint64_t MT_getAllocCount() { return 0; };
uint64_t MT_getDanglingPtrCount() { return 0; };
MemoryStopRecord::MemoryStopRecord() {};
MemoryStopRecord::~MemoryStopRecord() {};
void MemoryTrackerCheck() {};


#undef malloc
#undef free

void* my_malloc(size_t size)
{
  return malloc(size);
}

void my_free(void* p)
{
  free(p);
}


#endif// MEMORY_TRACKER_ENABLED
