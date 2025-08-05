/*
 * Copyright (c) 2000-2020 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * The contents of this file constitute Original Code as defined in and
 * are subject to the Apple Public Source License Version 1.1 (the
 * "License").  You may not use this file except in compliance with the
 * License.  Please obtain a copy of the License at
 * http://www.apple.com/publicsource and read it before using this file.
 *
 * This Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */
/*
clang nvram2.c -o nvram3 -framework CoreFoundation -framework IOKit -mmacosx-version-min=10.13
*/
// https://github.com/apple-oss-distributions/system_cmds/blob/13d383ddb305dc1672243e5384ba18b58949cc2c/nvram.tproj/nvram.c


//#include "assumes.h"
#include <stdio.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOKitKeys.h>
//#include <IOKit/IOKitKeysPrivate.h>
#include <CoreFoundation/CoreFoundation.h>
#include <err.h>
#include <mach/mach_error.h>
#include <sys/stat.h>

#define TARGET_OS_BRIDGE 0

// properties found in the registry root
#define kIOConsoleUsersKey		"IOConsoleUsers"		/* value is OSArray */
#define kIOMaximumMappedIOByteCountKey  "IOMaximumMappedIOByteCount"    /* value is OSNumber */

// properties found in the console user dict

#define kIOConsoleSessionIDKey		"kCGSSessionIDKey"		/* value is OSNumber */

#define kIOConsoleSessionUserNameKey	"kCGSSessionUserNameKey"	/* value is OSString */
#define kIOConsoleSessionUIDKey		    "kCGSSessionUserIDKey"		/* value is OSNumber */
#define kIOConsoleSessionConsoleSetKey	"kCGSSessionConsoleSetKey"	/* value is OSNumber */
#define kIOConsoleSessionOnConsoleKey	"kCGSSessionOnConsoleKey"	/* value is OSBoolean */

// IOResources property
#define kIOConsoleUsersSeedKey		"IOConsoleUsersSeed"		/* value is OSNumber */

#define kIOKernelHasSafeSleep		1
#define kIONVRAMForceSyncNowPropertyKey		"IONVRAM-FORCESYNCNOW-PROPERTY"

#define  gEfiGlobalVariableGuid "8BE4DF61-93CA-11D2-AA0D-00E098032B8C"
#define  gAppleNvramGuid  "4D1EDE05-38C7-4A6A-9CC6-4BCCA8B38C14"
#define  gAppleSecureBootGuid  "94B73556-2197-4702-82A8-3E1337DAFBFB"
#define  gAppleBootGuid  "7C436110-AB2A-4BBB-A880-FE41995C9F82"
#define  gAppleNetworkGuid  "36C28AB5-6566-4C50-9EBD-CBB920F83843"

// Prototypes
static void UsageMessage(const char *message);
static void ParseFile(const char *fileName);
static void ParseXMLFile(const char *fileName);
static void SetOrGetOFVariable(char *str);
static kern_return_t GetOFVariable(const char *name, CFStringRef *nameRef,
                                   CFTypeRef *valueRef);
static kern_return_t SetOFVariable(const char *name, const char *value);
static void DeleteOFVariable(const char *name);
static void PrintOFVariables(void);
static void PrintOFLargestVariables(void);
static void PrintOFVariable(const void *key,const void *value,void *context);
static void SetOFVariableFromFile(const void *key, const void *value, void *context);
static int ClearOFVariables(void);
static void ClearOFVariable(const void *key,const void *value,void *context);
static CFTypeRef ConvertValueToCFTypeRef(CFTypeID typeID, const char *value);

static void NVRamSyncNow(void);

// Global Variables
static io_registry_entry_t gOptionsRef;
static io_registry_entry_t gSystemOptionsRef;
static io_registry_entry_t gSelectedOptionsRef;
static bool                gUseXML;
static bool                gPrintInHex;
static bool                gUseForceSync;

#if TARGET_OS_BRIDGE /* Stuff for nvram bridge -> intel */
#include <dlfcn.h>
#include <libMacEFIManager/MacEFIHostInterfaceAPI.h>

static kern_return_t LinkMacNVRAMSymbols(void);
static kern_return_t GetMacOFVariable(char *name, char **value);
static kern_return_t SetMacOFVariable(char *name, char *value);
static kern_return_t DeleteMacOFVariable(char *name);

static bool gBridgeToIntel;
static void *gDL_handle;
static void *gNvramInterface;

static void (*hostInterfaceInitialize_fptr)(void);
static void *(*createNvramHostInterface_fptr)(const char *handle);
static kern_return_t (*destroyNvramHostInterface_fptr)(void *interface);
static kern_return_t (*getNVRAMVariable_fptr)(void *interface, char *name, char **buffer, uint32_t *size);
static kern_return_t (*setNVRAMVariable_fptr)(void *interface, char *name, char *buffer);
static kern_return_t (*deleteNVRAMVariable_fptr)(void *interface, char *name);
static void (*hostInterfaceDeinitialize_fptr)(void); /* may not need? */

#endif /* TARGET_OS_BRIDGE */

int main(int argc, char **argv)
{
  long                cnt;
  char                *str, errorMessage[256];
  kern_return_t       result;
  mach_port_t         mainPort;
  int                 argcount = 0;


//#if defined(MAC_OS_VERSION_12_0)
//    result = IOMainPort(bootstrap_port, &mainPort);
//#else
	result = IOMasterPort(bootstrap_port, &mainPort);
//#endif
  if (result != KERN_SUCCESS) {
    errx(1, "Error getting the IOMainPort: %s",
        mach_error_string(result));
  }

  gOptionsRef = IORegistryEntryFromPath(mainPort, "IODeviceTree:/options");
  if (gOptionsRef == 0) {
    errx(1, "nvram is not supported on this system");
  }

  gSystemOptionsRef = IORegistryEntryFromPath(mainPort, "IOService:/options/options-system");

  gSelectedOptionsRef = gOptionsRef;

  for (cnt = 1; cnt < argc; cnt++) {
    str = argv[cnt];
    if (str[0] == '-' && str[1] != 0) {
      // Parse the options.
      for (str += 1 ; *str; str++) {
        switch (*str) {
          case 'p' :
#if TARGET_OS_BRIDGE
            if (gBridgeToIntel) {
              fprintf(stderr, "-p not supported for Mac NVRAM store.\n");
              return 1;
            }
#endif
            PrintOFVariables();
            break;

          case 'h' :
            UsageMessage("");
            break;

          case 'x' :
            if (gPrintInHex) {
                fprintf(stderr, "-x not supported with -X hex mode.\n");
                return 1;
            }
            gUseXML = true;
            break;
                
          case 'X' :
            if (gUseXML) {
                fprintf(stderr, "-X hex mode not supported with -x XMLmode.\n");
                return 1;
            }
            gPrintInHex = true;
            break;

          case 'f':
#if TARGET_OS_BRIDGE
            if (gBridgeToIntel) {
              fprintf(stderr, "-f not supported for Mac NVRAM store.\n");
              return 1;
            }
#endif
            cnt++;
            if (cnt < argc && *argv[cnt] != '-') {
              ParseFile(argv[cnt]);
            } else {
              UsageMessage("missing filename");
            }
            break;

          case 'd':
            cnt++;
            if (cnt < argc && *argv[cnt] != '-') {
#if TARGET_OS_BRIDGE
              if (gBridgeToIntel) {
                if ((result = DeleteMacOFVariable(argv[cnt])) != KERN_SUCCESS) {
                  errx(1, "Error deleting variable - '%s': %s (0x%08x)", argv[cnt],
                          mach_error_string(result), result);
                }
              }
              else
#endif
              {
                DeleteOFVariable(argv[cnt]);
              }
            } else {
                UsageMessage("missing name");
            }
            break;

          case 'c':
#if TARGET_OS_BRIDGE
            if (gBridgeToIntel) {
              fprintf(stderr, "-c not supported for Mac NVRAM store.\n");
              return 1;
            }
#endif
            result = ClearOFVariables();
            break;
          case 's':
            // -s option is unadvertised -- advises the kernel more forcibly to
            // commit the variable to nonvolatile storage
            gUseForceSync = true;
            break;
#if TARGET_OS_BRIDGE
          case 'm':
            // used to set nvram variables on the Intel side
            // from the ARM side (Bridge -> Mac)
            fprintf(stdout, "Using Mac NVRAM store.\n");

            LinkMacNVRAMSymbols();
            gBridgeToIntel = true;
            break;
#endif

          case 'z':
            // -z option is unadvertised -- attempts to use the options-system node
            // to write to the system NVRAM region if available
            if (gSystemOptionsRef) {
              fprintf(stderr, "Selecting options-system node.\n");
              gSelectedOptionsRef = gSystemOptionsRef;
            } else {
              fprintf(stderr, "No options-system node, using options.\n");
            }
            break;

          default:
            strcpy(errorMessage, "no such option as --");
            errorMessage[strlen(errorMessage)-1] = *str;
            UsageMessage(errorMessage);
          }
        }
      } else {
        // Other arguments will be firmware variable requests.
        argcount++;
        SetOrGetOFVariable(str);
      }
  }

  // radar:25206371
  if (argcount == 0 && gUseForceSync == true) {
      NVRamSyncNow();
  }

  if (argc == 1) {
      UsageMessage("no arguments specified");
  }

  IOObjectRelease(gOptionsRef);

  if (gSystemOptionsRef) {
    IOObjectRelease(gSystemOptionsRef);
  }

  return result;
}

// UsageMessage(message)
//
//   Print the usage information and exit.
//
static void UsageMessage(const char *message)
{
  warnx("(usage: %s)", message);

  printf("nvram [-x|-X] [-p] [-f filename] [-d name] [-c] name[=value] ...\n");
  printf("\t-x         use XML format for printing or reading variables\n");
  printf("\t           (must appear before -p or -f)\n");
  printf("\t-X         use HEX format for printing or reading variables\n");
  printf("\t           (must appear before -p or -f)\n");
  printf("\t-p         print all firmware variables\n");
  printf("\t-f         set firmware variables from a text file\n");
  printf("\t-d         delete the named variable\n");
  printf("\t-c         delete all variables\n");
#if TARGET_OS_BRIDGE
  printf("\t-m         set nvram variables on macOS from bridgeOS\n");
#endif
  printf("\tname=value set named variable\n");
  printf("\tname       print variable\n");
  printf("Note that arguments and options are executed in order.\n");

  exit(1);
}


// States for ParseFile.
enum {
  kFirstColumn = 0,
  kScanComment,
  kFindName,
  kCollectName,
  kFindValue,
  kCollectValue,
  kContinueValue,
  kSetenv,

  kMaxStringSize = 0x800,
  kMaxNameSize = 0x100
};


// ParseFile(fileName)
//
//   Open and parse the specified file.
//
static void ParseFile(const char *fileName)
{
  long state, ni = 0, vi = 0;
  int tc;
  char name[kMaxNameSize];
  char value[kMaxStringSize];
  FILE *patches;
  kern_return_t kret;

  if (gUseXML) {
    ParseXMLFile(fileName);
    return;
  }

  patches = fopen(fileName, "r");
  if (patches == 0) {
    err(1, "Couldn't open patch file - '%s'", fileName);
  }

  state = kFirstColumn;
  while ((tc = getc(patches)) != EOF) {
    if(ni==(kMaxNameSize-1))
      errx(1, "Name exceeded max length of %d", kMaxNameSize);
    if(vi==(kMaxStringSize-1))
      errx(1, "Value exceeded max length of %d", kMaxStringSize);
    switch (state) {
    case kFirstColumn :
      ni = 0;
      vi = 0;
      if (tc == '#') {
        state = kScanComment;
      } else if (tc == '\n') {
        // state stays kFirstColumn.
      } else if (isspace(tc)) {
        state = kFindName;
      } else {
        state = kCollectName;
        name[ni++] = tc;
      }
      break;

    case kScanComment :
      if (tc == '\n') {
        state = kFirstColumn;
      } else {
        // state stays kScanComment.
      }
      break;

    case kFindName :
      if (tc == '\n') {
        state = kFirstColumn;
      } else if (isspace(tc)) {
        // state stays kFindName.
      } else {
        state = kCollectName;
        name[ni++] = tc;
      }
      break;

    case kCollectName :
      if (tc == '\n') {
        name[ni] = 0;
        warnx("Name must be followed by white space - '%s'", name);
        state = kFirstColumn;
      } else if (isspace(tc)) {
        state = kFindValue;
      } else {
        name[ni++] = tc;
        // state staus kCollectName.
      }
      break;

    case kFindValue :
    case kContinueValue :
      if (tc == '\n') {
        state = kSetenv;
      } else if (isspace(tc)) {
        // state stays kFindValue or kContinueValue.
      } else {
        state = kCollectValue;
        value[vi++] = tc;
      }
      break;

    case kCollectValue :
      if (tc == '\n') {
        if (value[vi-1] == '\\') {
          value[vi-1] = '\r';
          state = kContinueValue;
        } else {
          state = kSetenv;
        }
      } else {
        // state stays kCollectValue.
        value[vi++] = tc;
      }
      break;
    }

    if (state == kSetenv) {
      name[ni] = 0;
      value[vi] = 0;
      if ((kret = SetOFVariable(name, value)) != KERN_SUCCESS) {
        errx(1, "Error setting variable - '%s': %s", name,
             mach_error_string(kret));
      }
      state = kFirstColumn;
    }
  }

  if (state != kFirstColumn) {
    errx(1, "Last line ended abruptly");
  }
}

// ParseXMLFile(fileName)
//
//   Open and parse the specified file in XML format,
//   and set variables appropriately.
//
static void ParseXMLFile(const char *fileName)
{
  CFPropertyListRef plist;
  int fd;
  struct stat sb;
  char *buffer;
  CFReadStreamRef stream;
  CFPropertyListFormat format = kCFPropertyListBinaryFormat_v1_0;

  fd = open(fileName, O_RDONLY | O_NOFOLLOW, S_IFREG);
  if (fd == -1) {
      errx(1, "Could not open %s: %s", fileName, strerror(errno));
  }

  if (fstat(fd, &sb) == -1) {
      errx(1, "Could not fstat %s: %s", fileName, strerror(errno));
  }

  if (sb.st_size > UINT32_MAX) {
      errx(1, "too big for our purposes");
  }

  buffer = malloc((size_t)sb.st_size);
  if (buffer == NULL) {
      errx(1, "Could not allocate buffer");
  }

  if (read(fd, buffer, (size_t)sb.st_size) != sb.st_size) {
      errx(1, "Could not read %s: %s", fileName, strerror(errno));
  }

  close(fd);

  stream = CFReadStreamCreateWithBytesNoCopy(kCFAllocatorDefault,
          (const UInt8 *)buffer,
          (CFIndex)sb.st_size,
          kCFAllocatorNull);
  if (stream == NULL) {
      errx(1, "Could not create stream from serialized data");
  }

  if (!CFReadStreamOpen(stream)) {
      errx(1, "Could not open the stream");
  }

  plist = CFPropertyListCreateWithStream(kCFAllocatorDefault,
          stream,
          (CFIndex)sb.st_size,
          kCFPropertyListImmutable,
          &format,
          NULL);

  if (plist == NULL) {
      errx(1, "Error parsing XML file");
  }

  CFReadStreamClose(stream);

  CFRelease(stream);

  free(buffer);

  CFDictionaryApplyFunction(plist, &SetOFVariableFromFile, 0);

  CFRelease(plist);
}

// SetOrGetOFVariable(str)
//
//   Parse the input string, then set, append or get
//   the specified firmware variable.
//
static void SetOrGetOFVariable(char *str)
{
  long               set             = 0;
  long               append          = 0;
  long               remove          = 0;
  const char         *name           = NULL;
  char               *value          = NULL;
  char               *substring      = NULL;
  char               *tmp            = NULL;
  CFStringRef        nameRef         = NULL;
  CFTypeRef          valueRef        = NULL;
  size_t             len             = 0;
  CFMutableDataRef   mutableValueRef = NULL;
  kern_return_t      result;

  // OF variable name is first.
  name = str;

  // Find the equal sign for set, += for append, -= for trim
  while (*str) {
    if (*str == '+' && *(str+1) == '=') {
      append = 1;
      *str++ = '\0';
      *str++ = '\0';
      break;
    }
      
    if (*str == '-' && *(str+1) == '=') {
      remove = 1;
      *str++ = '\0';
      *str++ = '\0';
      break;
    }

    if (*str == '=') {
      set = 1;
      *str++ = '\0';
      break;
    }
    str++;
  }

  // Read the current value if appending or if no =/+=
  if (append == 1 || remove == 1 || (set == 0 && append == 0 && remove == 0)) {
#if TARGET_OS_BRIDGE
    if (gBridgeToIntel) {
      result = GetMacOFVariable(name, &value);
      if (result != KERN_SUCCESS) {
        errx(1, "Error getting variable - '%s': %s", name,
             mach_error_string(result));
      }
      nameRef = CFStringCreateWithCString(kCFAllocatorDefault, name, kCFStringEncodingUTF8);
      valueRef = CFStringCreateWithCString(kCFAllocatorDefault, value, kCFStringEncodingUTF8);
      free(value);
    }
    else
#endif
    {
      result = GetOFVariable(name, &nameRef, &valueRef);
      if (result != KERN_SUCCESS) {
        errx(1, "Error getting variable - '%s': %s", name,
             mach_error_string(result));
      }
    }
  }

  if (set == 1) {
    // On sets, the OF variable's value follows the equal sign.
    value = str;
  }

  if (append == 1) {
    // On append, the value to append follows the += substring
    if(CFGetTypeID(valueRef) == CFStringGetTypeID()) {
      len  = strlen(str);
      len += CFStringGetMaximumSizeForEncoding(CFStringGetLength(valueRef),
                                               kCFStringEncodingUTF8);
      tmp = calloc(len + 1, 1);
      if ( (tmp == NULL)
        || (CFStringGetCString(valueRef, tmp, len, kCFStringEncodingUTF8) == false)) {
        errx(1, "allocation failed");
      }
      
      value = tmp;
      strcpy(value + strlen(value), str);
    } else if(CFGetTypeID(valueRef) == CFDataGetTypeID()) {
      mutableValueRef = CFDataCreateMutableCopy(kCFAllocatorDefault, 
                                                CFDataGetLength(valueRef) + 
                                                strlen(str) + 1, 
                                                valueRef);
      CFDataAppendBytes(mutableValueRef, (const UInt8 *)str, strlen(str) + 1);
      value = (char *)CFDataGetBytePtr(mutableValueRef);
    }
    else {
      errx(1, "Unsupported named variable CFTypeID");
    }
  }

  if (remove == 1) {
    // On remove, the value to remove follows the -= substring
    if(CFGetTypeID(valueRef) == CFStringGetTypeID()) {
      len = CFStringGetMaximumSizeForEncoding(CFStringGetLength(valueRef),
                                               kCFStringEncodingUTF8);
      tmp = calloc(len + 1, 1);
      if(  (tmp == NULL)
        || (CFStringGetCString(valueRef, tmp, len, kCFStringEncodingUTF8) == false)) {
        errx(1, "failed to allocate string");
      }
    
      value = tmp;
    } else if(CFGetTypeID(valueRef) == CFDataGetTypeID()) {
      value = (char *)CFDataGetBytePtr(valueRef);
    } else {
      errx(1, "Unsupported named variable CFTypeID");
    }

    substring = strstr(value, str);
    if (substring == NULL) {
      errx(1, "substring %s not found in %s\n", str, value);
    }

    len = strlen(str);
    memmove(substring, substring + len, strlen(substring + len) + 1);
  }

  if (set == 1 || append == 1 || remove == 1) {
#if TARGET_OS_BRIDGE
    if (gBridgeToIntel) {
      result = SetMacOFVariable(name, value);
        if (result != KERN_SUCCESS) {
          errx(1, "Error setting variable - '%s': %s", name,
               mach_error_string(result));
        }
    }
    else
#endif
    {
      result = SetOFVariable(name, value);
      // Try syncing the new data to device, best effort!
      NVRamSyncNow();
      if (result != KERN_SUCCESS) {
        fprintf(stderr, "Error setting variable - '%s': %s.\n", name,
           mach_error_string(result));
        if (result == kIOReturnNoMemory) {
          PrintOFLargestVariables();
        }
        exit(1);
      }
    }
  } else {
    PrintOFVariable(nameRef, valueRef, 0);
  }
  if (nameRef) CFRelease(nameRef);
  if (valueRef) CFRelease(valueRef);
  if (tmp) free(tmp);
  if (mutableValueRef) CFRelease(mutableValueRef);
}

#if TARGET_OS_BRIDGE
static kern_return_t LinkMacNVRAMSymbols()
{
  gDL_handle = dlopen("libMacEFIHostInterface.dylib", RTLD_LAZY);
  if (gDL_handle == NULL) {
    errx(errno, "Failed to dlopen libMacEFIHostInterface.dylib");
    return KERN_FAILURE; /* NOTREACHED */
  }

  hostInterfaceInitialize_fptr = dlsym(gDL_handle, "hostInterfaceInitialize");
  if (hostInterfaceInitialize_fptr == NULL) {
    errx(errno, "failed to link hostInterfaceInitialize");
  }
  createNvramHostInterface_fptr = dlsym(gDL_handle, "createNvramHostInterface");
  if (createNvramHostInterface_fptr == NULL) {
    errx(errno, "failed to link createNvramHostInterface");
  }
  destroyNvramHostInterface_fptr = dlsym(gDL_handle, "destroyNvramHostInterface");
  if (destroyNvramHostInterface_fptr == NULL) {
    errx(errno, "failed to link destroyNvramHostInterface");
  }
  getNVRAMVariable_fptr = dlsym(gDL_handle, "getNVRAMVariable");
  if (getNVRAMVariable_fptr == NULL) {
    errx(errno, "failed to link getNVRAMVariable");
  }
  setNVRAMVariable_fptr = dlsym(gDL_handle, "setNVRAMVariable");
  if (setNVRAMVariable_fptr == NULL) {
    errx(errno, "failed to link setNVRAMVariable");
  }
  deleteNVRAMVariable_fptr = dlsym(gDL_handle, "deleteNVRAMVariable");
  if (deleteNVRAMVariable_fptr == NULL) {
      errx(errno, "failed to link deleteNVRAMVariable");
  }
  hostInterfaceDeinitialize_fptr = dlsym(gDL_handle, "hostInterfaceDeinitialize");
  if (hostInterfaceDeinitialize_fptr == NULL) {
    errx(errno, "failed to link hostInterfaceDeinitialize");
  }

  /* also do the initialization */
  hostInterfaceInitialize_fptr();
  gNvramInterface = createNvramHostInterface_fptr(NULL);

  return KERN_SUCCESS;
}
#endif

// GetOFVariable(name, nameRef, valueRef)
//
//   Get the named firmware variable.
//   Return it and it's symbol in valueRef and nameRef.
//
static kern_return_t GetOFVariable(const char *name, CFStringRef *nameRef,
                                   CFTypeRef *valueRef)
{
  *nameRef = CFStringCreateWithCString(kCFAllocatorDefault, name,
                                       kCFStringEncodingUTF8);
  if (*nameRef == 0) {
    errx(1, "Error creating CFString for key %s", name);
  }

  *valueRef = IORegistryEntryCreateCFProperty(gSelectedOptionsRef, *nameRef, 0, 0);
  if (*valueRef == 0) return kIOReturnNotFound;

  return KERN_SUCCESS;
}

#if TARGET_OS_BRIDGE
// GetMacOFVariable(name, value)
//
// Get the named firmware variable from the Intel side.
// Return the value in value
//
static kern_return_t GetMacOFVariable(char *name, char **value)
{
  uint32_t value_size;

  return getNVRAMVariable_fptr(gNvramInterface, name, value, &value_size);
}
#endif

// SetOFVariable(name, value)
//
//   Set or create an firmware variable with name and value.
//
static kern_return_t SetOFVariable(const char *name, const char *value)
{
  CFStringRef   nameRef;
  CFTypeRef     valueRef;
  CFTypeID      typeID;
  kern_return_t result = KERN_SUCCESS;

  nameRef = CFStringCreateWithCString(kCFAllocatorDefault, name,
                                        kCFStringEncodingUTF8);
  if (nameRef == 0) {
    errx(1, "Error creating CFString for key %s", name);
  }

  valueRef = IORegistryEntryCreateCFProperty(gSelectedOptionsRef, nameRef, 0, 0);
  if (valueRef) {
    typeID = CFGetTypeID(valueRef);
    CFRelease(valueRef);
    valueRef = ConvertValueToCFTypeRef(typeID, value);
    if (valueRef == 0) {
      errx(1, "Error creating CFTypeRef for value %s", value);
    }
    result = IORegistryEntrySetCFProperty(gSelectedOptionsRef, nameRef, valueRef);
  } else {
    // In the default case, try data, string, number, then boolean.
    CFTypeID types[] = {CFDataGetTypeID(),
    CFStringGetTypeID(), CFNumberGetTypeID(),CFBooleanGetTypeID() };
    for (int i = 0; i < sizeof(types)/sizeof(types[0]); i++) {
      valueRef = ConvertValueToCFTypeRef(types[i], value);
      if (valueRef != 0) {
        result = IORegistryEntrySetCFProperty(gSelectedOptionsRef, nameRef, valueRef);
        if (result == KERN_SUCCESS || result == kIOReturnNoMemory || result ==  kIOReturnNoSpace) {
          break;
        }
      }
    }
  }

  CFRelease(nameRef);

  return result;
}

#if TARGET_OS_BRIDGE
static kern_return_t SetMacOFVariable(char *name, char *value)
{
  return setNVRAMVariable_fptr(gNvramInterface, name, value);
}
#endif

// DeleteOFVariable(name)
//
//   Delete the named firmware variable.
//
static void DeleteOFVariable(const char *name)
{
  SetOFVariable(kIONVRAMDeletePropertyKey, name);
}

#if TARGET_OS_BRIDGE
static kern_return_t DeleteMacOFVariable(char *name)
{
  return deleteNVRAMVariable_fptr(gNvramInterface, name);
}
#endif

static void NVRamSyncNow(void)
{
  if (!gUseForceSync) {
    SetOFVariable(kIONVRAMSyncNowPropertyKey, kIONVRAMSyncNowPropertyKey);
  } else {
    SetOFVariable(kIONVRAMForceSyncNowPropertyKey, kIONVRAMForceSyncNowPropertyKey);
  }
}

CFDictionaryRef CreateMyDictionary(void) {

    // root
    CFMutableDictionaryRef dict0 = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
                                                            &kCFTypeDictionaryKeyCallBacks,
                                                            &kCFTypeDictionaryValueCallBacks);

    //   APPLE_BOOT_VARIABLE_GUID
    //   Print all of the firmware variables.
    CFMutableDictionaryRef dict1;
    kern_return_t          result;
    result = IORegistryEntryCreateCFProperties(gOptionsRef, &dict1, 0, 0);
    if (result != KERN_SUCCESS) {
        errx(1, "Error getting the firmware variables: %s", mach_error_string(result));
    }
    CFDictionarySetValue(dict0, CFSTR(gAppleBootGuid), dict1);
    CFRelease(dict1);

    CFMutableDictionaryRef dict2 = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks,
                                                             &kCFTypeDictionaryValueCallBacks);
    CFDictionarySetValue(dict0, CFSTR(gEfiGlobalVariableGuid), dict2);

    //   EFI_GLOBAL_VARIABLE_GUID
    //   Print the given firmware variable.
    CFStringRef   nameRef;
    CFTypeRef     valueRef;

    const char *key[32];
    char name[64];
    CFStringRef   var;
    int i = 0;
    int n = 0; // num of keys in this GUID

    key[n++] = "Boot0080";
    key[n++] = "BootOrder";
    key[n++] = "BootNext";
    key[n++] = "Boot0081";
    key[n++] = "Boot0082";
    key[n++] = "Boot0083";
    key[n++] = "BootCurrent";

    for ( i = 0; i < n; i++ ) {
        snprintf(name, sizeof(name), "%s:%s", gEfiGlobalVariableGuid, key[i]);
        var = CFStringCreateWithCString(NULL, key[i], kCFStringEncodingUTF8);
        result = GetOFVariable(name, &nameRef, &valueRef);
        if (result == KERN_SUCCESS) {
            CFDictionaryAddValue (dict2, var, valueRef);
            CFRelease(valueRef);
        }
        CFRelease(nameRef);
    }
    CFRelease(dict2);

    //  APPLE_WIRELESS_NETWORK_VARIABLE_GUID
    CFMutableDictionaryRef dict3 = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks,
                                                             &kCFTypeDictionaryValueCallBacks);
    CFDictionarySetValue(dict0, CFSTR(gAppleNetworkGuid), dict3);
    n = 0; // num of keys in this GUID
    key[n++] = "current-network";
    key[n++] = "preferred-count";
    key[n++] = "preferred-networks";
    

    for ( i = 0; i < n; i++ ) {
        snprintf(name, sizeof(name), "%s:%s", gAppleNetworkGuid, key[i]);
        var = CFStringCreateWithCString(NULL, key[i], kCFStringEncodingUTF8);
        result = GetOFVariable(name, &nameRef, &valueRef);
        if (result == KERN_SUCCESS) {
            CFDictionaryAddValue (dict3, var, valueRef);
            CFRelease(valueRef);
        }
        CFRelease(nameRef);
    }
    CFRelease(dict3);
  
     //  APPLE_SECURE_BOOT_VARIABLE_GUID
    CFMutableDictionaryRef dict4 = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks,
                                                             &kCFTypeDictionaryValueCallBacks);
    CFDictionarySetValue(dict0, CFSTR(gAppleSecureBootGuid), dict4);
    n = 0; // num of keys in this GUID
    key[n++] = "ApECID";
    key[n++] = "ApChipID";
    key[n++] = "ApBoardID";
    key[n++] = "ApSecurityDomain";
    key[n++] = "ApProductionStatus";
    key[n++] = "ApSecurityMode";
    key[n++] = "EffectiveProductionStatus";
    key[n++] = "EffectiveSecurityMode";
    key[n++] = "CertificateEpoch";
    key[n++] = "MixNMatchPreventionStatus";
    key[n++] = "CryptoDigestMethod";
    key[n++] = "HardwareModel";
    key[n++] = "InternalUseOnlyUnit";
    

    for ( i = 0; i < n; i++ ) {
        snprintf(name, sizeof(name), "%s:%s", gAppleSecureBootGuid, key[i]);
        var = CFStringCreateWithCString(NULL, key[i], kCFStringEncodingUTF8);
        result = GetOFVariable(name, &nameRef, &valueRef);
        if (result == KERN_SUCCESS) {
            CFDictionaryAddValue (dict4, var, valueRef);
            CFRelease(valueRef);
        }
        CFRelease(nameRef);
    }
    CFRelease(dict4);   
    
    CFMutableDictionaryRef dict5 = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks,
                                                             &kCFTypeDictionaryValueCallBacks);                                                            
    CFDictionarySetValue(dict0, CFSTR(gAppleNvramGuid), dict5);
    n=0;
    key[n++] = "gfx-saved-config-restore-status";
    key[n++] = "SkipLogo";
    key[n++] = "IASCurrentInstallPhase";
    key[n++] = "IASInstallPhaseList";
    for ( i = 0; i < n; i++ ) {
        snprintf(name, sizeof(name), "%s:%s", gAppleNvramGuid, key[i]);
        var = CFStringCreateWithCString(NULL, key[i], kCFStringEncodingUTF8);
        result = GetOFVariable(name, &nameRef, &valueRef);
        if (result == KERN_SUCCESS) {
            CFDictionaryAddValue (dict5, var, valueRef);
            CFRelease(valueRef);
        }
        CFRelease(nameRef);
    }
    CFRelease(dict5);  

    return dict0;
}

// PrintOFVariables()
//
//   Print all of the firmware variables.
//
static void PrintOFVariables(void)
{
  kern_return_t          result;
  CFMutableDictionaryRef dict;

  result = IORegistryEntryCreateCFProperties(gSelectedOptionsRef, &dict, 0, 0);
  if (result != KERN_SUCCESS) {
    errx(1, "Error getting the firmware variables: %s", mach_error_string(result));
  }

  if (gUseXML) {
    CFDataRef data;

	CFPropertyListRef propertyList = CreateMyDictionary();
	data = CFPropertyListCreateData( kCFAllocatorDefault, propertyList, kCFPropertyListXMLFormat_v1_0, 0, NULL );
 //   data = CFPropertyListCreateData( kCFAllocatorDefault, dict, kCFPropertyListXMLFormat_v1_0, 0, NULL );
    if (data == NULL) {
      errx(1, "Error converting variables to xml");
    }

    fwrite(CFDataGetBytePtr(data), sizeof(UInt8), CFDataGetLength(data), stdout);

    CFRelease(data);

  } else {

    CFDictionaryApplyFunction(dict, &PrintOFVariable, 0);

  }

  CFRelease(dict);
}

static size_t
ElementSize(const void *value)
{
  CFTypeID typeId = CFGetTypeID(value);
  if (typeId == CFStringGetTypeID()) {
    return CFStringGetLength(value);
  }
  else if (typeId == CFDataGetTypeID()) {
    return CFDataGetLength(value);
  }
  else if (typeId == CFNumberGetTypeID()) {
    return 8;
  }
  return 0;
}

// PrintOFLargestVariables()
//
//   Print the LARGEST_COUNT largest
//   nvram variables.
//
void
PrintOFLargestVariables(void)
{
#define LARGEST_COUNT 5
  kern_return_t          result;
  CFMutableDictionaryRef dict;
  long                   count;
  void                   **keys, **values;
  size_t                 *indices;

  result = IORegistryEntryCreateCFProperties(gOptionsRef, &dict, 0, 0);
  if (result != KERN_SUCCESS) {
    errx(1, "Error getting the firmware variables: %s", mach_error_string(result));
  }

  count = CFDictionaryGetCount(dict);

  keys = calloc(count, sizeof(void *));
  //os_assert(keys != NULL);
  values = calloc(count, sizeof(void *));
  //os_assert(values != NULL);

  indices = calloc(count, sizeof(*indices));
  //os_assert(indices != NULL);
  for (size_t i = 0; i < count; i++) {
    indices[i] = i;
  }

  CFDictionaryGetKeysAndValues(dict, (const void **)keys, (const void **)values);

  qsort_b(indices, count, sizeof(*indices), ^(const void *a, const void *b) {
    return (int)(((int64_t)ElementSize(values[*(size_t*)b]) - (int64_t)(ElementSize(values[*(size_t*)a]))));
  });

  fprintf(stderr, "key\tbytes\n");
  for (long i = 0; i < count && i < LARGEST_COUNT; i++) {
    size_t size = ElementSize(values[indices[i]]);
    if (size > 0) {
      fprintf(stderr, "%s\t%zu\n", CFStringGetCStringPtr(keys[indices[i]], kCFStringEncodingUTF8
  ), size);
    }
  }

  free(keys);
  free(values);
  free(indices);
  CFRelease(dict);
}

// PrintOFVariable(key, value, context)
//
//   Print the given firmware variable.
//
static void PrintOFVariable(const void *key, const void *value, void *context)
{
  long          cnt, cnt2;
  CFIndex       nameLen;
  char          *nameBuffer = 0;
  const char    *nameString;
  char          numberBuffer[10];
  const uint8_t *dataPtr;
  uint8_t       dataChar;
  char          *dataBuffer = 0;
  CFIndex       valueLen;
  char          *valueBuffer = 0;
  const char    *valueString = 0;
  uint32_t      number;
  long          length;
  CFTypeID      typeID;

  if (gUseXML) {
    CFDataRef data;
    CFDictionaryRef dict = CFDictionaryCreate(kCFAllocatorDefault, &key, &value, 1,
                                              &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    if (dict == NULL) {
      errx(1, "Error creating dictionary for variable value");
    }

    data = CFPropertyListCreateData( kCFAllocatorDefault, dict, kCFPropertyListXMLFormat_v1_0, 0, NULL );
    if (data == NULL) {
      errx(1, "Error creating xml plist for variable");
    }

    fwrite(CFDataGetBytePtr(data), sizeof(UInt8), CFDataGetLength(data), stdout);

    CFRelease(dict);
    CFRelease(data);
    return;
  }

  // Get the OF variable's name.
  nameLen = CFStringGetMaximumSizeForEncoding(CFStringGetLength(key),
      kCFStringEncodingUTF8) + 1;
  nameBuffer = malloc(nameLen);
  if( nameBuffer && CFStringGetCString(key, nameBuffer, nameLen, kCFStringEncodingUTF8) )
    nameString = nameBuffer;
  else {
    warnx("Unable to convert property name to C string");
    nameString = "<UNPRINTABLE>";
  }

  // Get the OF variable's type.
  typeID = CFGetTypeID(value);

  if (typeID == CFBooleanGetTypeID()) {
    if (CFBooleanGetValue(value)) valueString = "true";
    else valueString = "false";
  } else if (typeID == CFNumberGetTypeID()) {
    CFNumberGetValue(value, kCFNumberSInt32Type, &number);
    if (number == 0xFFFFFFFF) snprintf(numberBuffer,10, "-1");
    else if (!gPrintInHex && number < 1000) sprintf(numberBuffer, "%d", number);
    else snprintf(numberBuffer,10, "0x%x", number);
    valueString = numberBuffer;
  } else if (typeID == CFStringGetTypeID()) {
    valueLen = CFStringGetMaximumSizeForEncoding(CFStringGetLength(value),
        kCFStringEncodingUTF8) + 1;
    valueBuffer = malloc(valueLen + 1);
    if ( valueBuffer && CFStringGetCString(value, valueBuffer, valueLen, kCFStringEncodingUTF8) )
      valueString = valueBuffer;
    else {
      warnx("Unable to convert value to C string");
      valueString = "<UNPRINTABLE>";
    }
  } else if (typeID == CFDataGetTypeID()) {
    length = CFDataGetLength(value);
    if (length == 0) valueString = "";
    else {
      dataBuffer = malloc(length * 3 + 3);
      if (dataBuffer != 0) {
        dataPtr = CFDataGetBytePtr(value);
        cnt = cnt2 = 0;
        if (gPrintInHex) {
            snprintf(dataBuffer,length * 3 + 3, "0x");
            cnt2 += 2;
        }
        for (; cnt < length; cnt++) {
          dataChar = dataPtr[cnt];
          if (gPrintInHex) {
              snprintf(dataBuffer + cnt2, length * 3 + 3, "%02x", dataChar);
              cnt2 += 2;
          } else if (!gPrintInHex && isprint(dataChar) && dataChar != '%') {
            dataBuffer[cnt2++] = dataChar;
          } else {
            snprintf(dataBuffer + cnt2, length * 3 + 3,"%%%02x", dataChar);
            cnt2 += 3;
          }
        }
        dataBuffer[cnt2] = '\0';
        valueString = dataBuffer;
      }
    }
  } else {
    valueString="<INVALID>";
  }

  if ((nameString != 0) && (valueString != 0))
    printf("%s\t%s\n", nameString, valueString);

  if (dataBuffer != 0) free(dataBuffer);
  if (nameBuffer != 0) free(nameBuffer);
  if (valueBuffer != 0) free(valueBuffer);
}

// ClearOFVariables()
//
//   Deletes all OF variables
//
static int ClearOFVariables(void)
{
    kern_return_t          result;
    CFMutableDictionaryRef dict;

    result = IORegistryEntryCreateCFProperties(gSelectedOptionsRef, &dict, 0, 0);
    if (result != KERN_SUCCESS) {
      errx(1, "Error getting the firmware variables: %s", mach_error_string(result));
    }
    CFDictionaryApplyFunction(dict, &ClearOFVariable, &result);

    CFRelease(dict);

    return result;
}

static void ClearOFVariable(const void *key, const void *value, void *context)
{
  kern_return_t result;
  result = IORegistryEntrySetCFProperty(gSelectedOptionsRef,
                                        CFSTR(kIONVRAMDeletePropertyKey), key);
  if (result != KERN_SUCCESS) {
    assert(CFGetTypeID(key) == CFStringGetTypeID());
    const char *keyStr = CFStringGetCStringPtr(key, kCFStringEncodingUTF8);
    char *keyBuffer = NULL;
    size_t keyBufferLen = 0;
    if (!keyStr) {
      keyBufferLen = CFStringGetMaximumSizeForEncoding(CFStringGetLength(key), kCFStringEncodingUTF8) + 1;
      keyBuffer = (char *)malloc(keyBufferLen);
      if (keyBuffer != NULL && CFStringGetCString(key, keyBuffer, keyBufferLen, kCFStringEncodingUTF8)) {
        keyStr = keyBuffer;
      } else {
        warnx("Unable to convert property name to C string");
        keyStr = "<UNPRINTABLE>";
      }
    }

    warnx("Error clearing firmware variable %s: %s", keyStr, mach_error_string(result));
    if (keyBuffer) {
      free(keyBuffer);
    }

    if (context) {
      *(int *)context = result;
    }
  }
}

// ConvertValueToCFTypeRef(typeID, value)
//
//   Convert the value into a CFType given the typeID.
//
static CFTypeRef ConvertValueToCFTypeRef(CFTypeID typeID, const char *value)
{
    CFTypeRef     valueRef = 0;
    long          cnt, cnt2, length;
    unsigned long number, tmp;

    if (typeID == CFBooleanGetTypeID()) {
      if (!strcmp("true", value)) valueRef = kCFBooleanTrue;
      else if (!strcmp("false", value)) valueRef = kCFBooleanFalse;
    } else if (typeID == CFNumberGetTypeID()) {
      number = strtol(value, 0, 0);
      valueRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type,
                                &number);
    } else if (typeID == CFStringGetTypeID()) {
      valueRef = CFStringCreateWithCString(kCFAllocatorDefault, value,
                                           kCFStringEncodingUTF8);
    } else if (typeID == CFDataGetTypeID()) {
      length = strlen(value);
      char valueCopy[length + 1];

      for (cnt = cnt2 = 0; cnt < length; cnt++, cnt2++) {
        if (value[cnt] == '%') {
          if ((cnt + 2 > length) ||
              !ishexnumber(value[cnt + 1]) ||
              !ishexnumber(value[cnt + 2])) return 0;
          number = toupper(value[++cnt]) - '0';
          if (number > 9) number -= 7;
          tmp = toupper(value[++cnt]) - '0';
          if (tmp > 9) tmp -= 7;
          number = (number << 4) + tmp;
          valueCopy[cnt2] = number;
        } else valueCopy[cnt2] = value[cnt];
      }
      valueRef = CFDataCreate(kCFAllocatorDefault, (const UInt8 *)valueCopy, cnt2);
    } else return 0;

    return valueRef;
}

static void SetOFVariableFromFile(const void *key, const void *value, void *context)
{
  kern_return_t result;

  result = IORegistryEntrySetCFProperty(gSelectedOptionsRef, key, value);
  if ( result != KERN_SUCCESS ) {
    long nameLen;
    char *nameBuffer;
    char *nameString;

    // Get the variable's name.
    nameLen = CFStringGetMaximumSizeForEncoding(CFStringGetLength(key),
        kCFStringEncodingUTF8) + 1;
    nameBuffer = malloc(nameLen);
    if( nameBuffer && CFStringGetCString(key, nameBuffer, nameLen, kCFStringEncodingUTF8) )
            nameString = nameBuffer;
    else {
            warnx("Unable to convert property name to C string");
            nameString = "<UNPRINTABLE>";
    }
    errx(1, "Error setting variable - '%s': %s", nameString,
         mach_error_string(result));
  }
}
