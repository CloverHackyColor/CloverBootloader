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

#include "../../../PosixCompilation/xcode_utf_fixed.h"
#include "../../../rEFIt_UEFI/Platform/ConfigPlist/ConfigPlistClass.h"
#include "../../../rEFIt_UEFI/Platform/ConfigPlist/SMBIOSPlist.h"

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

static EFI_STATUS oldParse(const char *source, size_t newLen, bool enableCloverLog, bool enableLog, TagDict** dict, SETTINGS_DATA* settings)
{
  EFI_STATUS Status = ParseXML(source, dict, (UINT32)newLen);
  if ( enableLog ) printf("ParseXML returns %s\n", efiStrError(Status));
  if ( EFI_ERROR(Status) ) {
    return Status;
  }
  //    XString8 s;
  //    dict->sprintf(0, &s);
  //    printf("%s\n", s.c_str());
  
  Status = GetEarlyUserSettings(*dict, *settings);
  //printf("settings=%llx, &ResetHDA=%llx %d\n", uintptr_t(&settings), uintptr_t(&settings.Devices.Audio.ResetHDA), settings.Devices.Audio.ResetHDA);
  if ( enableLog ) printf("GetEarlyUserSettings returns %s\n", efiStrError(Status));
  
  Status = GetUserSettings(*dict, *settings);
  if ( enableLog ) printf("GetUserSettings returns %s\n", efiStrError(Status));
  return Status;
}

static EFI_STATUS test1(const char *source, size_t newLen, bool enableCloverLog, bool enableLog)
{
  EFI_STATUS Status;
  TagDict* dict = NULL;
  SETTINGS_DATA settings;

  gEnableCloverLog = enableCloverLog;
  if ( enableCloverLog ) enableLog = true;

  Status = oldParse(source, newLen, enableCloverLog, enableLog, &dict, &settings);
  
  bool compareOldAndCompatibleArbOK = settings.Devices.compareOldAndCompatibleArb();
  if ( !compareOldAndCompatibleArbOK ) {
    if ( enableLog ) printf("!compareOldAndCompatibleArb");
  }
  
  //  if ( !compareOldAndNewArb(settings.Devices.newArbProperties, settings.Devices.oldArbProperties) ) {
  //    printf("aie");
  //  }
  
  ConfigPlistClass configPlist;
  
  XmlLiteParser xmlLiteParser;
  xmlLiteParser.init(source, newLen);
  
  gEnableCloverLog = enableLog;

  if ( enableLog ) printf("\n");
  if ( enableLog ) printf("=== [ Parse ] ====================\n");
  configPlist.parse(&xmlLiteParser, ""_XS8);
  for ( size_t idx = 0 ; idx < xmlLiteParser.getErrorsAndWarnings().size() ; idx++ ) {
    const XmlParserMessage& xmlMsg = xmlLiteParser.getErrorsAndWarnings()[idx];
    if ( enableLog ) printf("%s: %s\n", xmlMsg.isError ? "Error" : "Warning", xmlMsg.msg.c_str());
  }
  
  if ( enableLog ) printf("\n");
  if ( enableLog ) printf("=== [ CompareOldNewSettings ] ====================\n");
  
  uint64_t nbError = 0;
  
//  nbError = CompareOldNewSettings(settings, configPlist);
//  if ( nbError == 0 )
//  {
//    SETTINGS_DATA settings2;
//    AssignOldNewSettings(settings2, configPlist, SmbiosPlistClass());
//    if ( !settings.isEqual(settings2) ) {
//      nbError++;
//      if ( enableLog ) printf("Binary comparison failed\n");
//bool b_tmp = settings.isEqual(settings2);
//(void)b_tmp;
//      return EFI_COMPROMISED_DATA;
//    }
//  }
//
//#if __cplusplus > 201703L
//  if ( nbError == 0 )
//  {
//    SETTINGS_DATA settings2;
//    AssignOldNewSettings(settings2, configPlist);
//    if ( !(settings == settings2) ) {
//      nbError++;
//      if ( enableLog ) printf("Binary comparison(2) failed\n");
//      return EFI_COMPROMISED_DATA;
//    }
//  }
//#endif

  if ( nbError == 0 )
  {
    SETTINGS_DATA settings2;
    AssignOldNewSettings(settings2, configPlist, SmbiosPlistClass());
//    if ( !(settings == settings2) ) {
//      nbError++;
//      if ( enableLog ) printf("Binary comparison(2) failed\n");
//      return EFI_COMPROMISED_DATA;
//    }
    SETTINGS_DATA settings3;
    settings3.takeValueFrom(configPlist);
    if ( !settings3.isEqual(settings2) ) {
      bool b_tmp = settings3.isEqual(settings2);
      (void)b_tmp;
      nbError++;
    }
  }

  if ( nbError == 0 ) {
		if ( enableLog ) printf("Comparison OK\n");
  }else{
    if ( enableLog ) printf("Nb errors : %lld\n", nbError);
  }

  return 0;
}

EFI_STATUS test1(const char *source, bool enableCloverLog, bool enableLog)
{
  return test1(source, strlen(source), enableCloverLog, enableLog);
}

EFI_STATUS test1_file(const char* filename, bool disableCloverLog, bool noLog)
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

  return test1(source, newLen, disableCloverLog, noLog);
}


#include <chrono>


EFI_STATUS test_speed(const char* filename)
{
  EFI_STATUS Status;
  char * source;
  size_t newLen;
  TagDict* dict = NULL;
  SETTINGS_DATA settings;

  printf("Speed test with file:%s\n", filename);
  Status = readFile(filename, &source, &newLen);
  printf("readFile returns %s\n", efiStrError(Status));
  if ( EFI_ERROR(Status) ) {
    return Status;
  }

  auto t1 = std::chrono::high_resolution_clock::now();

  for ( size_t i = 0 ; i < 1000 ; ++i ) {
    Status = oldParse(source, newLen, false, false, &dict, &settings);
//    dict->FreeTag();
  }

  auto t2 = std::chrono::high_resolution_clock::now();

  XmlLiteParser xmlLiteParser;
  ConfigPlistClass configPlist;
  for ( size_t i = 0 ; i < 1000 ; ++i ) {
    xmlLiteParser.init(source, newLen);
    configPlist.parse(&xmlLiteParser, ""_XS8);
  }
  
  auto t3 = std::chrono::high_resolution_clock::now();

  /* Getting number of milliseconds as an integer. */
  auto s_int = std::chrono::duration_cast<std::chrono::milliseconds>( (t2 - t1) / 1 );
  printf("Elpased time %lld ms\n", s_int.count());
  
  auto s_int3 = std::chrono::duration_cast<std::chrono::milliseconds>( (t3 - t2) / 1 );
  printf("Elpased time %lld ms\n", s_int3.count());

  return EFI_SUCCESS;
}







int nbStructuralProblem = 0;
int nbConfigPlistRead = 0;

bool printFileBefore = true;

EFI_STATUS test4testers(const char* filename)
{
  EFI_STATUS Status;
  char * source;
  size_t newLen;

  if ( printFileBefore ) printf("file:%s\n", filename);

  gEnableCloverLog = false;
  
  Status = readFile(filename, &source, &newLen);
  if ( EFI_ERROR(Status) ) {
    fprintf(stderr, "Cannot read the file '%s'. Status = %s\n", filename, efiStrError(Status));
    return Status;
  }
  LString8 source8 = LString8(source);
  if ( source8.containsIC("ShowPicker"_XS8) ) {
    // it's OC
    return EFI_SUCCESS;
  }
  if ( source8.startWithOrEqualToIC("bplist"_XS8) ) {
    // it's compressed
    return EFI_SUCCESS;
  }
  if ( !source8.startWithOrEqualToIC("<?xml"_XS8) ) {
    // it's not xml
    return EFI_SUCCESS;
  }

  TagDict* dict = NULL;
  Status = ParseXML(source, &dict, (UINT32)newLen);
  nbConfigPlistRead += 1;
  if ( EFI_ERROR(Status) ) {
//    printf("ParseXML returns %s\n", efiStrError(Status));
    // it's probably not a well formed plist
//printf("file:%s\n", filename);
//    return Status;
  }
    
  SETTINGS_DATA settings;
  Status = GetEarlyUserSettings(dict, settings);
  if ( EFI_ERROR(Status) ) {
//    printf("GetEarlyUserSettings returns %s\n", efiStrError(Status));
    if ( !printFileBefore ) printf("file:%s\n", filename);
    return Status;
  }
  Status = GetUserSettings(dict, settings);
  if ( EFI_ERROR(Status) ) {
//    printf("GetUserSettings returns %s\n", efiStrError(Status));
    if ( !printFileBefore ) printf("file:%s\n", filename);
    return Status;
  }

  if ( !settings.Devices.compareOldAndCompatibleArb() ) {
//    printf("!compareOldAndCompatibleArb");
    if ( !printFileBefore ) printf("file:%s\n", filename);
    return EFI_MEDIA_CHANGED;
  }

  ConfigPlistClass configPlist;
  
  XmlLiteParser xmlLiteParser;
  xmlLiteParser.init(source, newLen);

//  printf("\n");
//  printf("=== [ Parse ] ====================\n");
  configPlist.parse(&xmlLiteParser, LString8(""));
//  for ( size_t idx = 0 ; idx < xmlLiteParser.getErrorsAndWarnings().size() ; idx++ ) {
//    const XmlParserMessage& xmlMsg = xmlLiteParser.getErrorsAndWarnings()[idx];
//    printf("%s: %s\n", xmlMsg.isError ? "Error" : "Warning", xmlMsg.msg.c_str());
//  }
//  if ( b ) {
//    if ( xmlLiteParser.getErrorsAndWarnings().size() == 0 ) {
//      printf("Your plist looks so wonderful. Well done!\n");
//    }
//  }
//
//  printf("\n");
//  printf("=== [ CompareOldNewSettings ] ====================\n");

  if ( xmlLiteParser.xmlParsingError ) {
    printf("This file '%s' has an xml structural problem. Consider fixing.\n", filename);
    nbStructuralProblem += 1;
    return RETURN_HTTP_ERROR;
  }
//  uint64_t nbError = CompareOldNewSettings(settings, configPlist);
//  if ( nbError > 0 ) {
//    //    printf("nbError > 0");
//    if ( !printFileBefore ) printf("file:%s\n", filename);
//    return EFI_COMPROMISED_DATA;
//  }

  {
		SETTINGS_DATA settings2;
		AssignOldNewSettings(settings2, configPlist, SmbiosPlistClass());
//		if ( !settings.isEqual(settings2) ) {
//			if ( !printFileBefore ) printf("file:%s\n", filename);
//			bool b_tmp = settings.isEqual(settings2);
//			(void)b_tmp;
//			return EFI_COMPROMISED_DATA;
//		}
    SETTINGS_DATA settings3;
    settings3.takeValueFrom(configPlist);
    if ( !settings3.isEqual(settings2) ) {
      if ( !printFileBefore ) printf("file:%s\n", filename);
      bool b_tmp = settings3.isEqual(settings2);
      (void)b_tmp;
      return EFI_COMPROMISED_DATA;
    }
	}

	#if __cplusplus > 201703L
		{
			SETTINGS_DATA settings2;
			AssignOldNewSettings(settings2, configPlist);
			if ( !(settings == settings2) ) {
				if ( !printFileBefore ) printf("file:%s\n", filename);
				return EFI_COMPROMISED_DATA;
			}
		}
	#endif

  if ( source8.contains("<key>Boot</key>"_XS8)  ||  source8.contains("<key>KernelAndKextPatches</key>"_XS8)  ||  source8.contains("<key>GUI</key>"_XS8) ) {
    if ( !printFileBefore ) printf("file:%s\n", filename);
  }
//  printf("ZZZZZZfile:%s\n", filename);
  return EFI_SUCCESS;
}

static bool iterate_test_file(const XString8 &fileFullPath)
{
  //printf("fileZZZ:%s %s\n", fileFullPath.c_str(), fileFullPath.basename().c_str());
  if ( fileFullPath.basename().containsIC("config") && fileFullPath.basename().endWithOrEqualToIC(".plist"_XS8)  ) {
    EFI_STATUS Status = test4testers(fileFullPath.c_str());
    if ( EFI_ERROR(Status) ) {
      if ( Status == EFI_NOT_FOUND ) {
      }else if ( Status == RETURN_HTTP_ERROR ) {
      }else{
        //            fprintf(stderr, "There is a problem is my new parser with the field '%s'. Do NOT change your config.plist.\n", compareField_firstErrorField.c_str());
        fprintf(stderr, "Please send the file '%s' as is to me. DO NOT modify the file please, or I won't catch that bug in my new parser. Thanks.\n", fileFullPath.c_str());
        //            fprintf(stderr, "Press any key :");
        //            getchar();
        //            fprintf(stderr, "\n");
        return false;
      }
    }
  }
  return true;
}

class TestEq
{
public:
	uint8_t foo[2] = {1, 2};
	#if __cplusplus > 201703L
		bool operator == (const TestEq&) const = default;
	#endif
};

bool iterateFileSystem(const XString8& path)
{
//  printf("folder:%s\n", path.c_str());
// 	TestEq t1;
//	TestEq t2;
//	printf("t1 == t2 %d\n", t1 == t2);


  XString8 correctedPath = path;
  while ( correctedPath.contains("//") ) correctedPath.replaceAll("//"_XS8, "/"_XS8);
  if ( correctedPath == "/"_XS8 ) correctedPath.setEmpty();

  DIR *dir;
  struct dirent *entry;

  if ( !(dir = opendir(path.c_str())) )
      return true;

  XString8 file;
  XString8 subPath;
  XString8 fullpath;
  XString8Array folderNameArray;

  while ((entry = readdir(dir)) != NULL)
  {
    if (entry->d_type == DT_DIR) {
      if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
          continue;
      file.takeValueFrom(entry->d_name);
      fullpath = S8Printf("%s/%s", correctedPath.c_str(), file.c_str());
      if ( fullpath == "/System"_XS8 ) continue;
      if ( fullpath == "/Library"_XS8 ) continue;
      if ( fullpath == "/private"_XS8 ) continue;
      if ( fullpath == "/usr"_XS8 ) continue;
      if ( fullpath.containsIC(".Spotlight-V100"_XS8) ) continue;
      if ( fullpath.startWithOrEqualToIC("/Users") && file == "Library"_XS8 ) continue;
      if ( file.endWithOrEqualToIC(".app"_XS8) ) continue;
      if ( file == "DerivedData"_XS8 ) continue;
//          printf("%*s[%s]\n", indent, "", entry->d_name);
      folderNameArray.Add(fullpath);
    } else {
      file.takeValueFrom(entry->d_name);
      fullpath = S8Printf("%s/%s", path.c_str(), file.c_str());
      if ( ! iterate_test_file(fullpath) ) return false;
    }
  }
  closedir(dir);

  for ( size_t idx = 0 ; idx < folderNameArray.size() ; ++idx )
  {
    if( !iterateFileSystem(folderNameArray[idx]) ) return false;
  }
  return true;
}

extern "C" int main(int argc, const char * argv[])
{
//tmp();
	(void)argc;
	(void)argv;
	setlocale(LC_ALL, "en_US"); // to allow printf unicode char

//  xcode_utf_fixed_tests();

  gEnableCloverLog = false;

#if defined(JIEF_DEBUG)

//  EFI_STATUS Status;

//  const char* emptyConfigPlist = "<dict></dict>";
//  Status = test1(emptyConfigPlist, strlen(emptyConfigPlist), true, true);
//  if ( EFI_ERROR(Status) ) {
//    printf("***************************************************  ERROR **********************************\n");
//    exit(1);
//  }

//  test1("<dict></dict>", false, true);
//  test1("<dict><key>SMBIOS</key><dict></dict></dict>", false, true);


//  test1_file("config-test1.plist", true, true);
//    test1_file("/JiefLand/5.Devel/Clover/Clover-projects/Clover--CloverHackyColor--master.3/Xcode/CloverConfigPlistValidator/config-test2.plist", true, true);
//  test1_file("config-small.plist", true, true);
//  test1_file("/System/Library/CoreServices/Certificate Assistant.app/Contents/Resources/3_CreateAndConfigure.bundle/Contents/Resources/CertificateAssistantTrustedApps.plist", true, true);
  //  test1_file("/JiefLand/3.Infos/3.Infos-Hackintosh/3.Infos-Dell M4300/Precision M4300 --1D-00-00-00-B1-C2/config.plist", true, true);
//  test_speed("/JiefLand/5.Devel/Clover/Clover-projects/Clover--CloverHackyColor--master.3/Xcode/CloverConfigPlistValidator/config-nowarning-noerror.plist");
//  iterateFileSystem("/JiefLand/5.Devel/Clover/user config"_XS8);

#else
  printf("Version 13\n");
  if ( argc == 0 ) {
    printf("Usage %s [path to file or folder to scan]\n", argv[0]);
    return -1;
  }
  size_t nb = 0;
  if ( argc == 1 ) {
    if ( !iterateFileSystem("/"_XS8) ) nb += 1;
  }else{
    for ( int idx = 1 ; nb == 0  &&  idx < argc ; ++idx ) { // nb == 0 because I want to stop at the first problem.
      XString8 file;
      file.takeValueFrom(argv[idx]);
      struct stat st_stat;
      if ( stat(file.c_str(), &st_stat) != 0 ) {
        fprintf(stderr, "Cannot access '%s'. Skipped.\n", file.c_str());
        continue;
      }
      if ( S_ISDIR(st_stat.st_mode) ) {
        printf("Look for config.plist in folder '%s'\n", file.c_str());
        if ( !iterateFileSystem(file) ) nb += 1;
      }else{
//        if ( !iterate_test(file) ) nb += 1;
        if ( EFI_ERROR(test1_file(file.c_str(), true, true)) ) nb += 1;
      }
    }
  }
  if ( nb == 0 ) {
    if ( nbStructuralProblem > 0 ) {
      printf("%d file(s) ignored because of xml structural problem (missing closing tag or like).\n", nbStructuralProblem);
    }
    printf("%d file(s) read. No parsing bug detected. Great. Let me know\n", nbConfigPlistRead);
  }
#endif

  return 0;
}






//
//
//bool b;
//b =  "aa"_XS8.endWithOrEqualToIC("config.plist"_XS8);
//b =  "config.plist"_XS8.endWithOrEqualToIC("config.plist"_XS8);
//b =  "Config.plist"_XS8.endWithOrEqualToIC("config.plist"_XS8);
//b =  "AAConfig.plist"_XS8.endWithOrEqualToIC("config.plist"_XS8);
//b =  "AAConfig.plista"_XS8.endWithOrEqualToIC("config.plist"_XS8);
