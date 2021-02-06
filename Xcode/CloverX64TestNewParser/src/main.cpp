//
//  main.cpp
//  cpp_tests
//
//  Created by jief on 23.02.20.
//  Copyright © 2020 JF Knudsen. All rights reserved.
//
#include <iostream>
#include <locale.h>

#include <Platform.h>
#include <Efi.h>
#include "../../../rEFIt_UEFI/Platform/plist/plist.h"
#include "../../../rEFIt_UEFI/Platform/Settings.h"
#include "../../../rEFIt_UEFI/cpp_unit_test/all_tests.h"

#include "../../../../../cpp_tests/Include/xcode_utf_fixed.h"
#include "ConfigSample1.h"


int test1()
{
  TagDict* dict = NULL;
  EFI_STATUS Status = ParseXML(configSample1, &dict, (UINT32)strlen(configSample1));
  printf("ParseXML returns %s\n", efiStrError(Status));
  if ( !EFI_ERROR(Status) ) {
//    XString8 s;
//    dict->sprintf(0, &s);
//    printf("%s\n", s.c_str());
    
    SETTINGS_DATA settings;
    Status = GetEarlyUserSettings(dict, settings);
    printf("GetEarlyUserSettings returns %s\n", efiStrError(Status));
    Status = GetUserSettings(dict, settings);
    printf("GetUserSettings returns %s\n", efiStrError(Status));
  }

  return 0;
}






extern "C" void tmp();

extern "C" int main(int argc, const char * argv[])
{
//tmp();
	(void)argc;
	(void)argv;
	setlocale(LC_ALL, "en_US"); // to allow printf unicode char

  xcode_utf_fixed_tests();

  test1();
  
	return all_tests() ? 0 : -1 ;
}
