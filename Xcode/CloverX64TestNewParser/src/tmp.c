//
//  tmp.c
//  cpp_tests_compare_settings UTF16 signed char
//
//  Created by Jief on 06/02/2021.
//  Copyright © 2021 JF Knudsen. All rights reserved.
//

#include "tmp.h"
//
//int __vsnprintf_chk (char* buf, size_t len, int check, size_t size, const char* format, va_list va)
//{
//  return 0;
//}

extern "C" void tmp()
{
  wprintf(L"%ls\n", L"Hello world൧楔");

}


