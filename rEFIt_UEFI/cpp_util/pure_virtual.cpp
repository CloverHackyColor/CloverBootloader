//
//  pure_virtual.cpp
//  CloverX64
//
//  Created by Jief on 24/08/2020.
//

#include "../Platform/Platform.h"

#ifdef _MSC_VER

extern "C" int _purecall()
{
  panic("Pure virtual function called");
}

#else

extern "C" void __cxa_pure_virtual()
{
  panic("Pure virtual function called");
}

#endif
