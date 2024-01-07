//
//  main.cpp
//  cpp_tests
//
//  Created by jief on 23.02.20.
//  Copyright © 2020 Jief_Machak. All rights reserved.
//

#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <locale.h>

//#include <iostream>
//#include <fstream>
//#include <iostream>
//#include <filesystem>

//#include <Cocoa/Cocoa.h>

#include <Platform.h>
#include <Efi.h>
#include "../../../rEFIt_UEFI/Platform/Settings.h"

#include "../../../rEFIt_UEFI/Settings/ConfigPlist/ConfigPlistClass.h"
#include "../../../rEFIt_UEFI/Settings/ConfigPlist/SMBIOSPlist.h"

#include "ConfigSample1.h"


extern bool gEnableCloverLog;


static EFI_STATUS readFile(const char* filename, char** sourcePtr, size_t* newLen)
{
  char*& source = *sourcePtr;
  *newLen = 0;
  FILE *fp = fopen(filename, "r");
  if (fp == NULL) {
//    fputs("Error fopen config plist", stderr);
    return EFI_NOT_FOUND;
  }
  /* Go to the end of the file. */
  if (fseek(fp, 0L, SEEK_END) == 0) {
    /* Get the size of the file. */
    long bufsize = ftell(fp);
    if (bufsize == -1) {
//      fputs("Error ftell config plist", stderr);
      return EFI_LOAD_ERROR;
    }

    /* Allocate our buffer to that size. */
    source = (char*)malloc(sizeof(char) * (bufsize + 1));

    /* Go back to the start of the file. */
    if (fseek(fp, 0L, SEEK_SET) != 0) {
//      fputs("Error fseek config plist", stderr);
      return EFI_LOAD_ERROR;
    }

    /* Read the entire file into memory. */
    *newLen = fread(source, sizeof(char), bufsize, fp);
    if ( ferror( fp ) != 0 ) {
//      fputs("Error reading config plist", stderr);
      return EFI_LOAD_ERROR;
    } else {
      source[*newLen] = '\0'; /* Just to be safe. */
    }
  }
  fclose(fp);
  return EFI_SUCCESS;
}


static EFI_STATUS test1(const char *source, size_t newLen)
{
  ConfigPlistClass configPlist;

  XmlLiteParser xmlLiteParser;
  xmlLiteParser.init(source, newLen);

  printf("\n");
  printf("=== [ Parse ] ====================\n");
  configPlist.parse(&xmlLiteParser, ""_XS8);
  for ( size_t idx = 0 ; idx < xmlLiteParser.getErrorsAndWarnings().size() ; idx++ ) {
    const XmlParserMessage& xmlMsg = xmlLiteParser.getErrorsAndWarnings()[idx];
    printf("%s: %s\n", xmlMsg.isError ? "Error" : "Warning", xmlMsg.msg.c_str());
  }

  SETTINGS_DATA settingsData;
  settingsData.takeValueFrom(configPlist);
  
  printf("\n");

  return 0;
}

EFI_STATUS test1(const char *source)
{
  return test1(source, strlen(source));
}

EFI_STATUS test1_file(const char* filename)
{
  EFI_STATUS Status;
  char * source;
  size_t newLen;

printf("Debug test file:%s\n", filename);
  Status = readFile(filename, &source, &newLen);
  printf("readFile returns %s\n", efiStrError(Status));
  if ( EFI_ERROR(Status) ) {
    return Status;
  }

  return test1(source, newLen);
}


extern "C" int main_read_configplist(int argc, const char * argv[])
{

  test1_file("/JiefLand/5.Devel/Clover/user config/kushwavez/2021-04-29/X1C6_config_and_logs/config.plist");
  
  return 0;
}

