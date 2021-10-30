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
#include <getopt.h>

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

void usage()
{
  fprintf(stderr, "Usage ConfigPlistValidator [-h|--help] [-v|--version] [--info]  [-p|--productname=] path_to_config.plist\n");
  exit(1);
}

extern "C" int main(int argc, char * const argv[])
{
	(void)argc;
	(void)argv;
	setlocale(LC_ALL, "en_US"); // to allow printf unicode char

  int c;
  int info_flag = 0;
  XString8 ProductName;

  while (1)
  {
    static struct option long_options[] =
      {
        /* These options set a flag. */
        {"info", no_argument,       &info_flag, 1},
        {"productname", required_argument, 0, 'p'},
        {"help", no_argument,       0, 'h'},
        {"version", no_argument,       0, 'v'},
        {0, 0, 0, 0}
      };
    /* getopt_long stores the option index here. */
    int option_index = 0;

    c = getopt_long (argc, argv, "productname:info:hv", long_options, &option_index);

    /* Detect the end of the options. */
    if (c == -1)
      break;

    switch (c)
    {
      case 0:
        break;

      case 'p':
        ProductName.takeValueFrom(optarg);
        break;

      case 'h':
        usage();
        
      case 'v':
        fprintf(stderr, "ConfigPlistValidator for '%s'\n", gRevisionStr);
        fprintf(stderr, "Build id is '%s'\n", gBuildId.c_str());
        break;

      case '?':
        /* getopt_long already printed an error message. */
        break;

      default:
        fprintf(stderr, "Bug in argument parsing.\n");
        return 2;
    }
  }
  
  if (optind == argc) return 1;
  if (optind < argc-1 ) {
    usage();
  }


  const char* path = argv[optind];
  
  #ifdef JIEF_DEBUG
      path = "config-nowarning-noerror.plist";
      path = "config-test2.plist";
      path = "/JiefLand/5.Devel/Clover/user config/mifjpn/2021-10-22/config.plist";
      path = "/Volumes/CL_EFI_VMDK/EFI/CLOVER/config.plist";
      //path = "/Volumes/CL_EFI_VMDK/EFI/CLOVER/smbios.plist";
  #endif
  

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
  
  ConfigPlistClass configPlistTest;
  
  XmlLiteParser xmlLiteParser;
  xmlLiteParser.init(buf, st.st_size);

  if ( ProductName.notEmpty() ) {
    // if a ProductName is specified in plist, this will be ignored.
    configPlistTest.SMBIOS.defaultMacModel = GetModelFromString(ProductName);
  }

  configPlistTest.parse(&xmlLiteParser, LString8(""));
  
  if ( ProductName.notEmpty()  &&  configPlistTest.getSMBIOS().getProductName().isDefined() ) {
    if ( ProductName != configPlistTest.getSMBIOS().getProductName().value() ) {
      printf("Warning: ProductName is specified in command line AND in plist, to a different value. Command line option ignored.\n");
    }
  }
  
  bool b = true;
  for ( size_t idx = 0 ; idx < xmlLiteParser.getXmlParserMessageArray().size() ; idx++ ) {
    const XmlParserMessage& xmlMsg = xmlLiteParser.getXmlParserMessageArray()[idx];
    if ( xmlMsg.type != XmlParserMessageType::info ) {
      printf("%s\n", xmlMsg.getFormattedMsg().c_str());
      b = false;
    }else
    if ( info_flag ) {
      printf("%s\n", xmlMsg.getFormattedMsg().c_str());
    }
  }
  if ( b ) {
    if ( xmlLiteParser.getXmlParserInfoMessageCount() > 0 ) {
      printf("Your plist looks good. Well done!\n");
    }else{
      printf("Your plist looks so wonderful. Well done!\n");
    }
    return 0;
  }else{
    return 1;
  }
}
