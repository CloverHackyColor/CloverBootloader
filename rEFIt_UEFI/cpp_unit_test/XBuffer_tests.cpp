#include <Platform.h> // Only use angled for Platform, else, xcode project won't compile
#include "../cpp_foundation/XBuffer.h"

int XBuffer_tests()
{

#ifdef JIEF_DEBUG
//	printf("XBuffer_tests -> Enter\n");
#endif

  XBuffer<UINT8> xb_uint8;
  
  void* p = (void*)1;
  char* p2 = (char*)2;
  xb_uint8.cat(p);
  xb_uint8.cat(p2);
  xb_uint8.cat(uintptr_t(0));

  return 0;
}
