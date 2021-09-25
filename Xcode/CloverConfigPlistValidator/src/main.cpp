//
//  main.cpp
//  cpp_tests
//
//  Created by jief on 23.02.20.
//  Copyright © 2020 Jief_Machak. All rights reserved.
//

#include <iostream>
#include <locale.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include "../../../rEFIt_UEFI/Platform/CloverVersion.h"

#ifndef MIN
#define MIN(a,b)    ((a) < (b) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a,b)    ((a) > (b) ? (a) : (b))
#endif

ssize_t read_all(int fd, void* buf, size_t size)
{
  size_t nbluTotal = 0;
  while ( nbluTotal < size )
  {
    ssize_t nblu = read(fd, ((uint8_t*)buf)+nbluTotal, MIN(65536, size-nbluTotal));
    if ( nblu < 0 ) return -1;
    if ( nblu == 0 ) {
      if ( nbluTotal != size ) {
        // Read only nbluTotal bytes instead of size
        return -1;
      }
      return nbluTotal;
    }
    nbluTotal += nblu;
  }
  return size;
}

#include "../../../rEFIt_UEFI/Settings/ConfigPlist/ConfigPlistClass.h"

extern "C" int main(int argc, const char * argv[])
{
	(void)argc;
	(void)argv;
	setlocale(LC_ALL, "en_US"); // to allow printf unicode char

//AsciiStrHexToUint64("dwf");

  const char* path = NULL;
  #ifdef JIEF_DEBUG
      path = "config-nowarning-noerror.plist";
      path = "config-test2.plist";
      path = "/Volumes/CL_EFI_VMDK/EFI/CLOVER/config.plist";
  #endif
  
  if ( !path ) {
      if ( argc == 2 ) {
          path = argv[1];
      }else{
          fprintf(stderr, "ConfigPlistValidator for '%s'\n", gRevisionStr);
          fprintf(stderr, "Build id is '%s'\n", gBuildId.c_str());
          fprintf(stderr, "Usage ConfigPlistValidator path_to_config.plist\n");
          return -1;
      }
  }
  struct stat st;
  int ret = stat(path, &st);
  if ( ret != 0 ) {
    fprintf(stderr, "Cannot stat file '%s'\n", path);
    return 1;
  }

  char* buf = (char*)malloc(st.st_size+1);

  int fd = open(path, O_RDONLY);
  if ( fd < 0 ) {
    fprintf(stderr, "Cannot open file '%s'. Errno %s\n", path, strerror(errno));
    return 1;
  }
  ssize_t nblu = read_all(fd, buf, st.st_size);
  if ( nblu != st.st_size ) {
    fprintf(stderr, "Cannot read file '%s'. Errno %s\n", path, strerror(errno));
    return 1;
  }
  buf[st.st_size] = 0; // should not be needed.
  
  bool b;
  ConfigPlistClass configPlistTest;
  
  XmlLiteParser xmlLiteParser;
  xmlLiteParser.init(buf, st.st_size);

  b = configPlistTest.parse(&xmlLiteParser, LString8(""));
  for ( size_t idx = 0 ; idx < xmlLiteParser.getErrorsAndWarnings().size() ; idx++ ) {
    const XmlParserMessage& xmlMsg = xmlLiteParser.getErrorsAndWarnings()[idx];
    printf("%s: %s\n", xmlMsg.isError ? "Error" : "Warning", xmlMsg.msg.c_str());
  }
  if ( b ) {
//    if ( xmlLiteParser.getErrorsAndWarnings().size() > 0 ) {
//      printf("parse return true, but there is error and warnings! BUG !!");
//    }
    if ( xmlLiteParser.getErrorsAndWarnings().size() == 0 ) {
      printf("Your plist looks so wonderful. Well done!\n");
    }
    return 0;
  }else{
    return 1;
  }
}
