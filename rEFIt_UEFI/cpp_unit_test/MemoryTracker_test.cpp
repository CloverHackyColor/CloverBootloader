#include <Platform.h> // Only use angled for Platform, else, xcode project won't compile
#include "../cpp_lib/MemoryTracker.h"
#include "../cpp_foundation/XStringArray.h"


static int breakpoint(int v)
{
  return v;
}

//
//class TestAlloc {
//public:
////  MemoryAllocation4Object objma;
//  void* ptr1 = 0;
//  void* ptr2 = 0;
//
//  TestAlloc() {};
//  ~TestAlloc() {
////    objma.free(ptr1, "ptr1"_XS8);
//    MT_delete(ptr1, ""_XS8);
//  };
//
//  void* testAlloc2()
//  {
//    printf("--TestAlloc.testAlloc2 - begin\n");
////    MemoryAllocation ma(__FILE_NAME__, __PRETTY_FUNCTION__, __LINE__);
//    ptr1 = MT_alloc(10, "ptr1  (supposed to be a memory leak)"_XS8); (void)ptr1;
////    objma.takeOwnership(ptr1, "ptr1"_XS8);
//    void* ptrl1 = MT_alloc(10, "ptrl1"_XS8); (void)ptrl1;
//    MT_delete(ptrl1, "ptrl1"_XS8);
//    void* ptrl2 = MT_alloc(10, "ptrl2 bis"_XS8);
//    printf("--TestAlloc.testAlloc2 - end\n");
////    MT_export(q2);
////    return ma.return_(ptrl2);
//    return ptrl2;
//  }
//};
//
//void* testAlloc2()
//{
//  printf("--testAlloc2 - begin\n");
//
//  MemoryAllocation ma(__FILE__, __PRETTY_FUNCTION__, __LINE__);
//  void* p2 = MT_alloc(10, "p2 (supposed to be a memory leak)"_XS8); (void)p2;
//  void* q2 = MT_alloc(10, "q2"_XS8); (void)q2;
//  MT_delete(q2, "q2"_XS8);
//  q2 = MT_alloc(10, "q2 bis"_XS8);
//
//
//  printf("--testAlloc2 - end\n");
//  return ma.return_(q2);
//}
//
//void testAlloc1()
//{
//  printf("--testAlloc1 - begin\n");
//  MemoryAllocation ma(__FILE__, __PRETTY_FUNCTION__, __LINE__);
//  void* p = MT_alloc(10, "p (supposed to be a memory leak)"_XS8); (void)p;
//  void* q = MT_alloc(10, "q"_XS8); (void)q;
//  void* t = testAlloc2(); (void)t;
//  MT_delete(q, "q"_XS8);
//  q = MT_alloc(10, "q bis (supposed to be a memory leak)"_XS8);
//  printf("--testAlloc1 - end\n");
//}
//
//
//TestAlloc g_ta;
//

void f_p1void(void* p)
{
  void* q = p;
  (void)q;
}

int MemoryTracker_tests()
{
#ifdef MEMORY_TRACKER_ENABLED
  void* p; (void)p;
  
//  int t1_v = 3;
////  apd<int*> apdTest1 = &t1_v; // this would crash because this ptr cannot be freed
//  apd<int*> apdTest1 = new int(t1_v); // this is ok
//  int* q = &(*apdTest1);
////  int* qq = *apdTest1;
//  int* const* r = &apdTest1;
//  **r = 7;
//
//  void* u = apdTest1;
//  auto uuu = &apdTest1.get();
//
//  f_p1void(apdTest1);
//
//  auto apdTest1_get = apdTest1.get();
//  auto apdTest1_adr_of_get = &apdTest1.get();
//  auto apdTest1_getAdrOfPtr = &apdTest1;
  
  
  MemoryTrackerInstallHook();

  {
    new char();
    char* pp = (char*)malloc(2);
    *(pp-1) = 0;
    free(pp);
  }
  {
    new char();
    char* pp = (char*)malloc(2);
    *(pp+3) = 0;
    free(pp);
  }
  MT_outputDanglingPtr();
  if ( MT_getDanglingPtrCount() != 2 ) return breakpoint(1);
  
//  testAlloc1();
//  printf("------------------\n");
//  {
//    TestAlloc ta;
//    p = ta.testAlloc2();
//    MT_delete(p, "return ta.testAlloc2()"_XS8);
//  }
//  printf("------------------\n");
//  p = g_ta.testAlloc2();
//  MT_delete(p, "return g_ta.testAlloc2()"_XS8);
//  MT_outputDanglingPtr();
//  printf("------------------\n");
#endif
	return 0;
}

