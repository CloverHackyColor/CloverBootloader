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


extern "C" int main(int argc, const char * argv[])
{
	(void)argc;
	(void)argv;
	setlocale(LC_ALL, "en_US"); // to allow printf unicode char

//  xcode_utf_fixed_tests();
  int i = 2;
  bool b;
  b = i;
  
	return all_tests() ? 0 : -1 ;
}
