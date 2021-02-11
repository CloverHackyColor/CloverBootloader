//
//  main.cpp
//  cpp_tests
//
//  Created by jief on 23.02.20.
//  Copyright © 2020 JF Knudsen. All rights reserved.
//

#include <iostream>
#include <locale.h>

#include "../../../rEFIt_UEFI/cpp_unit_test/all_tests.h"


extern "C" int main(int argc, const char * argv[])
{
	(void)argc;
	(void)argv;
	setlocale(LC_ALL, "en_US"); // to allow printf unicode char

//  xcode_utf_fixed_tests();
  
  
	return all_tests() ? 0 : -1 ;
}
