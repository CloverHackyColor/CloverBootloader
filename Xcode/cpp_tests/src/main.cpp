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

printf("sizeof(wchar_t)=%lu\n", sizeof(wchar_t));
printf("%lc\n", L'Ľ');
printf("sizeof(size_t)=%lu\n", sizeof(size_t));
printf("sizeof(long)=%lu\n", sizeof(long));
printf("sizeof(long long)=%lu\n", sizeof(long long));
printf("sizeof(size_t)=%lu\n", sizeof(size_t));
printf("%zu\n", (size_t)MAX_UINT64);
printf("%zd\n", (size_t)MAX_UINT64);

	return all_tests();
}
