//
//  findesp.c
//  partutil
//
//  Created by Micky1979 on 04/02/18 (https://github.com/Micky1979/findesp).
//  Copyright © 2018 Micky1979. All rights reserved.
//

#include "partutil.h"

CFDictionaryRef getDescriptionsFrom(char const* diskOrMountPoint) {
  int                 err = 0;
  DADiskRef           disk = NULL;
  DASessionRef        session;
  CFDictionaryRef     descDict = NULL;
  CFURLRef            volURL = NULL;
  struct stat sb;
  
  session = DASessionCreate(NULL);
  if (session == NULL) err = EINVAL;
    
    if (err == 0) {
      if (strncmp("disk", diskOrMountPoint, strlen("disk")) == 0) {
        disk = DADiskCreateFromBSDName(NULL,
                                       session,
                                       diskOrMountPoint);
      } else if (stat(diskOrMountPoint, &sb) == 0 && S_ISDIR(sb.st_mode)) {
        volURL = CFURLCreateFromFileSystemRepresentation(kCFAllocatorDefault,
                                                         (const UInt8 *)diskOrMountPoint,
                                                         strlen(diskOrMountPoint),
                                                         true);
        disk = DADiskCreateFromVolumePath(NULL,
                                          session,
                                          volURL);
        if (volURL) {
          CFRelease(volURL);
        }
      } else {
        usage (1);
      }
      
      if (session) {
        CFRelease(session);
      }
      
      if (disk != NULL) {
        descDict = DADiskCopyDescription(disk);
        CFRelease(disk);
        if (descDict != NULL) {
          return descDict;
        }
      }
    } else {
      fprintf(stderr, "DADiskCreateXXX(%s) returned NULL\n", diskOrMountPoint);
      exit(EXIT_FAILURE);
    }
  return NULL;
}

CFArrayRef findEFIDisks() {
  CFMutableDictionaryRef match_dictionary = IOServiceMatching(kIOMediaClass);
  io_iterator_t entry_iterator;
  CFMutableArrayRef esps = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
  if (IOServiceGetMatchingServices(kIOMasterPortDefault,
                                   match_dictionary,
                                   &entry_iterator) == kIOReturnSuccess) {
    io_registry_entry_t serviceObject;
    while ((serviceObject = IOIteratorNext(entry_iterator))) {
      CFMutableDictionaryRef serviceDictionary = NULL;
      if (IORegistryEntryCreateCFProperties(serviceObject,
                                            &serviceDictionary,
                                            kCFAllocatorDefault,
                                            kNilOptions) != kIOReturnSuccess) {
        continue;
      }

      if (serviceDictionary != NULL) {
        const char *bsd = CFStringGetCStringPtr(CFDictionaryGetValue(serviceDictionary,
                                                                     CFSTR(kIOBSDNameKey)),
                                                kCFStringEncodingUTF8);
        if (bsd) {
          CFDictionaryRef desc = getDescriptionsFrom(bsd);
          if (desc) {
            const void *mediaName = CFStringGetCStringPtr(CFDictionaryGetValue(desc,
                                                                               kDADiskDescriptionMediaNameKey),
                                                          kCFStringEncodingUTF8);
            if(strcmp(mediaName, "EFI System Partition") == 0) {
              CFArrayAppendValue(esps, CFStringCreateWithCString(NULL, bsd, kCFStringEncodingUTF8));
            }
            CFRelease(desc);
          }
        }
      }
      if (serviceDictionary) {
        CFRelease(serviceDictionary);
      }
    }
  }
  return esps;
}

char const* getESPFor(char const* diskOrMountPoint) {
  CFDictionaryRef desc = getDescriptionsFrom(diskOrMountPoint);
  if (desc != NULL) {
    const void *busPath = CFStringGetCStringPtr(CFDictionaryGetValue(desc,
                                                                     kDADiskDescriptionBusPathKey),
                                                kCFStringEncodingUTF8);
    CFArrayRef esps = findEFIDisks();
    if (esps) {
      CFIndex count = CFArrayGetCount(esps);
      for (CFIndex i = 0; i < count; i++) {
        const void *bsd = CFStringGetCStringPtr(CFArrayGetValueAtIndex(esps, i), kCFStringEncodingUTF8);
        
        CFDictionaryRef espDesc = getDescriptionsFrom(bsd);
        if (espDesc) {
          const void *espBusPath = CFStringGetCStringPtr(CFDictionaryGetValue(espDesc,
                                                                              kDADiskDescriptionBusPathKey),
                                                         kCFStringEncodingUTF8);
          if(strcmp((const char*)busPath, (const char*)espBusPath) == 0) {
            CFRelease(esps);
            CFRelease(espDesc);
            return bsd;
          } else {
            CFRelease(espDesc);
          }
        }
      }
      CFRelease(esps);
    }
  }
  return "";
}
