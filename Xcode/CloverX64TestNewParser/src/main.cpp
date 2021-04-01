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
#include "../../../rEFIt_UEFI/Platform/ConfigPlist/ConfigPlistClass.h"
#include "../../../rEFIt_UEFI/Platform/ConfigPlist/CompareSettings.h"



int test1()
{
  char *source = NULL;
  size_t newLen = 0;
  FILE *fp = fopen("config-test1.plist", "r");
//  FILE *fp = fopen("/JiefLand/5.Devel/Clover/user config/eng_redaesm/config.plist", "r");
  if (fp == NULL) {
    fputs("Error fopen config plist", stderr);
    exit(-1);
  }
  /* Go to the end of the file. */
  if (fseek(fp, 0L, SEEK_END) == 0) {
    /* Get the size of the file. */
    long bufsize = ftell(fp);
    if (bufsize == -1) {
      fputs("Error ftell config plist", stderr);
      exit(-1);
    }

    /* Allocate our buffer to that size. */
    source = (char*)malloc(sizeof(char) * (bufsize + 1));

    /* Go back to the start of the file. */
    if (fseek(fp, 0L, SEEK_SET) != 0) {
      fputs("Error fseek config plist", stderr);
      exit(-1);
    }

    /* Read the entire file into memory. */
    newLen = fread(source, sizeof(char), bufsize, fp);
    if ( ferror( fp ) != 0 ) {
        fputs("Error reading config plist", stderr);
        exit(-1);
    } else {
        source[newLen++] = '\0'; /* Just to be safe. */
    }
  }
  fclose(fp);

  TagDict* dict = NULL;
  EFI_STATUS Status = ParseXML(source, &dict, (UINT32)newLen);
  printf("ParseXML returns %s\n", efiStrError(Status));
  if ( EFI_ERROR(Status) ) {
    return Status;
  }
//    XString8 s;
//    dict->sprintf(0, &s);
//    printf("%s\n", s.c_str());
    
  SETTINGS_DATA settings;
  Status = GetEarlyUserSettings(dict, settings);
//printf("settings=%llx, &ResetHDA=%llx %d\n", uintptr_t(&settings), uintptr_t(&settings.Devices.Audio.ResetHDA), settings.Devices.Audio.ResetHDA);
  printf("GetEarlyUserSettings returns %s\n", efiStrError(Status));
  Status = GetUserSettings(dict, settings);
  printf("GetUserSettings returns %s\n", efiStrError(Status));

  if ( !settings.Devices.compareOldAndCompatibleArb() ) {
    printf("!compareOldAndCompatibleArb");
  }

  //  if ( !compareOldAndNewArb(settings.Devices.newArbProperties, settings.Devices.oldArbProperties) ) {
  //    printf("aie");
  //  }

  bool b;
  ConfigPlistClass configPlist;
  
  XmlLiteParser xmlLiteParser;
  xmlLiteParser.init(source, newLen);

  printf("\n");
  printf("=== [ Parse ] ====================\n");
  b = configPlist.parse(&xmlLiteParser, LString8(""));
  for ( size_t idx = 0 ; idx < xmlLiteParser.getErrorsAndWarnings().size() ; idx++ ) {
    const XmlParserMessage& xmlMsg = xmlLiteParser.getErrorsAndWarnings()[idx];
    printf("%s: %s\n", xmlMsg.isError ? "Error" : "Warning", xmlMsg.msg.c_str());
  }
  if ( b ) {
    if ( xmlLiteParser.getErrorsAndWarnings().size() == 0 ) {
      printf("Your plist looks so wonderful. Well done!\n");
    }
  }

  printf("\n");
  printf("=== [ CompareOldNewSettings ] ====================\n");
  return CompareOldNewSettings(settings, configPlist) ? 0 : -1;
}


class PlistTest : public XmlDict
{
public:
  XmlStringWArray DisableDrivers = XmlStringWArray();

  XmlDictField m_fields[1] = {
    {"DisableDrivers", DisableDrivers},
  };
  virtual void getFields(XmlDictField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
};

int test2()
{
  char *source = NULL;
  size_t newLen = 0;
  FILE *fp = fopen("config-test2.plist", "r");
  if (fp == NULL) {
    fputs("Error fopen config plist", stderr);
    exit(-1);
  }
  /* Go to the end of the file. */
  if (fseek(fp, 0L, SEEK_END) == 0) {
    /* Get the size of the file. */
    long bufsize = ftell(fp);
    if (bufsize == -1) {
      fputs("Error ftell config plist", stderr);
      exit(-1);
    }

    /* Allocate our buffer to that size. */
    source = (char*)malloc(sizeof(char) * (bufsize + 1));

    /* Go back to the start of the file. */
    if (fseek(fp, 0L, SEEK_SET) != 0) {
      fputs("Error fseek config plist", stderr);
      exit(-1);
    }

    /* Read the entire file into memory. */
    newLen = fread(source, sizeof(char), bufsize, fp);
    if ( ferror( fp ) != 0 ) {
        fputs("Error reading config plist", stderr);
        exit(-1);
    } else {
        source[newLen++] = '\0'; /* Just to be safe. */
    }
  }
  fclose(fp);


  bool b;
  PlistTest plistTest;
  
  XmlLiteParser xmlLiteParser;
  xmlLiteParser.init(source, newLen);
  xmlLiteParser.moveForwardUntilSignificant();
  xmlLiteParser.skipHeader();
  printf("\n");
  printf("=== [ Parse ] ====================\n");
  b = plistTest.parseFromXmlLite(&xmlLiteParser, ""_XS8, true);
  if ( b )
  {
    b = plistTest.validate(&xmlLiteParser, ""_XS8, XmlParserPosition(), true);
  }
  if ( xmlLiteParser.getErrorsAndWarnings().size() > 0 ) {
    for ( size_t idx = 0 ; idx < xmlLiteParser.getErrorsAndWarnings().size() ; idx++ ) {
      const XmlParserMessage& xmlMsg = xmlLiteParser.getErrorsAndWarnings()[idx];
      printf("%s: %s\n", xmlMsg.isError ? "Error" : "Warning", xmlMsg.msg.c_str());
    }
  }else{
    printf("Your plist looks so wonderful. Well done!\n");
  }
  printf("\n");
  return 0;
}

extern "C" void tmp();

extern "C" int main(int argc, const char * argv[])
{
//tmp();
	(void)argc;
	(void)argv;
	setlocale(LC_ALL, "en_US"); // to allow printf unicode char


char buf[] = { 'a', '\xef', '\xbb', '\xbf', 'b', 0};
XString8 s5 = S8Printf("01234567890123456789");
s5.S8Printf("%s", buf);
//const char* buf2 = s5.c_str();

  xcode_utf_fixed_tests();


  return test1();
//  return test2();

//	return all_tests() ? 0 : -1 ;
}
