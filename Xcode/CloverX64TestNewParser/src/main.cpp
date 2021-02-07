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
#include "../../../rEFIt_UEFI/Platform/ConfigPlist/ConfigPlist.h"
#include "../../../rEFIt_UEFI/Platform/ConfigPlist/CompareSettings.h"


int test1()
{
  TagDict* dict = NULL;
  EFI_STATUS Status = ParseXML(configSample1, &dict, (UINT32)strlen(configSample1));
  printf("ParseXML returns %s\n", efiStrError(Status));
  if ( EFI_ERROR(Status) ) {
    return Status;
  }
//    XString8 s;
//    dict->sprintf(0, &s);
//    printf("%s\n", s.c_str());
    
  SETTINGS_DATA settings;
  Status = GetEarlyUserSettings(dict, settings);
  printf("GetEarlyUserSettings returns %s\n", efiStrError(Status));
  Status = GetUserSettings(dict, settings);
  printf("GetUserSettings returns %s\n", efiStrError(Status));

  bool b;
  ConfigPlist configPlist;
  
  XmlLiteParser xmlLiteParser;
  xmlLiteParser.init(configSample1, strlen(configSample1));

  b = configPlist.parse(&xmlLiteParser, LString8(""));
//  for ( size_t idx = 0 ; idx < xmlLiteParser.getErrorsAndWarnings().size() ; idx++ ) {
//    const XmlParserMessage& xmlMsg = xmlLiteParser.getErrorsAndWarnings()[idx];
//    printf("%s: %s\n", xmlMsg.isError ? "Error" : "Warning", xmlMsg.msg.c_str());
//  }
  if ( b ) {
    if ( xmlLiteParser.getErrorsAndWarnings().size() == 0 ) {
      printf("Your plist looks so wonderful. Well done!\n");
    }
  }

  return CompareEarlyUserSettingsWithConfigPlist(settings, configPlist) ? 0 : -1;
}






extern "C" void tmp();

extern "C" int main(int argc, const char * argv[])
{
//tmp();
	(void)argc;
	(void)argv;
	setlocale(LC_ALL, "en_US"); // to allow printf unicode char

  xcode_utf_fixed_tests();


  return test1();
  
//	return all_tests() ? 0 : -1 ;
}
