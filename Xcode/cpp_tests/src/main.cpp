//
//  main.cpp
//  cpp_tests
//
//  Created by jief on 23.02.20.
//  Copyright © 2020 Jief_Machak. All rights reserved.
//

#include <iostream>
#include <locale.h>

#include "../../../PosixCompilation/xcode_utf_fixed.h"

#include "../../../rEFIt_UEFI/cpp_unit_test/all_tests.h"

//class Boolean
//{
//    bool flag;
//public:
//    explicit Boolean() : flag(false) {}
//    explicit Boolean(const bool other) { flag = other; }
//    explicit Boolean(const Boolean& other) { flag = other.flag; }
////    template <typename T>
////    Boolean(T other) = delete;// { return *this; }
//
//    Boolean& operator= (const Boolean& other) { return *this; }
//    Boolean& operator= (const bool other) { return *this; }
//    template <typename T>
//    Boolean& operator= (const T other) = delete;// { return *this; }
//
//    bool getValue() const {return flag;}
//    void setValue(bool a) {flag = a;}
//};


extern "C" int main(int argc, const char * argv[])
{
	(void)argc;
	(void)argv;
	setlocale(LC_ALL, "en_US"); // to allow printf unicode char

//  xcode_utf_fixed_tests();
  const int i = 2;
  (void)i;
  XBool b;
  b = true;
  b = false;
//  b = XBool(i);
//  b = (char*)NULL;
//  b = (float)1.0;
//  b = i;
  
	return all_tests() ? 0 : -1 ;
}
