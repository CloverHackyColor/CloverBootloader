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
#include "../../../rEFIt_UEFI/Platform/plist/plist.h"
#include "../../../rEFIt_UEFI/Platform/Settings.h"
#include "../../../rEFIt_UEFI/cpp_unit_test/all_tests.h"

#include <xcode_utf_fixed.h>
#include "../../../rEFIt_UEFI/Settings/ConfigPlist/ConfigPlistClass.h"
#include "../../../rEFIt_UEFI/Settings/ConfigPlist/SMBIOSPlist.h"

#include "ConfigSample1.h"
#include "Compare/CompareField.h"
#include "Compare/CompareSettings.h"
#include "Assign/AssignSettings.h"

#include "OldSettings/Settings.h"

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

extern UINT32 ArptADR1;
extern UINT32 FIXAirport (UINT8 *dsdt, UINT32 len);

extern "C" int main_fixbiosdsdt(int argc, const char * argv[])
{
	(void)argc;
	(void)argv;
	setlocale(LC_ALL, "en_US"); // to allow printf unicode char

//  xcode_utf_fixed_tests();

  gEnableCloverLog = false;

  char* buf;
  size_t size;
  readFile("DSDT_before_AIRPORT.bin", &buf, &size);

  ArptADR1=0x1C0000;
  FIXAirport((UINT8 *)buf, (UINT32)size);
  
  return 0;
}

