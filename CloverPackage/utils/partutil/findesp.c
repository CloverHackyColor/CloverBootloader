//
//  findesp.c
//  partutil
//
//  Created by Micky1979 on 04/02/18 (https://github.com/Micky1979/findesp).
//  Copyright © 2018 Micky1979. All rights reserved.
//

#include "partutil.h"

CFDictionaryRef getDescriptionsFrom(char const* diskOrMountPoint) {
  DADiskRef           disk = NULL;
  DASessionRef        session;
  CFDictionaryRef     descDict = NULL;
  CFURLRef            volURL = NULL;
  struct stat sb;

  session = DASessionCreate(NULL);

  if (session != NULL) {
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
      CFRelease(session);
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
                                            0) != kIOReturnSuccess) {
        continue;
      }

      if (serviceDictionary != NULL) {
        CFStringRef v = CFDictionaryGetValue(serviceDictionary,
                                             CFSTR(kIOBSDNameKey));

        CFIndex length = CFStringGetLength(v);
        CFIndex size = CFStringGetMaximumSizeForEncoding(length, kCFStringEncodingUTF8) + 1;
        char *bsd = (char *)malloc(size);

        if (CFStringGetCString(v, bsd, size, kCFStringEncodingUTF8)) {
          CFDictionaryRef desc = getDescriptionsFrom(bsd);
          if (desc) {
            const void *mediaName = CFStringGetCStringPtr(CFDictionaryGetValue(desc,
                                                                               kDADiskDescriptionMediaNameKey),
                                                          kCFStringEncodingUTF8);

            if(mediaName && strcmp(mediaName, "EFI System Partition") == 0) {
              CFStringRef esp = CFStringCreateWithCString(kCFAllocatorDefault, bsd, kCFStringEncodingUTF8);
              if (esp) {
                CFArrayAppendValue(esps, esp);
              }
            }
            CFRelease(desc);
          }
          free(bsd);
        }
        CFRelease(serviceDictionary);
      }
    }
  }
  return esps;
}

char * getESPFor(char const* diskOrMountPoint) {
  CFDictionaryRef desc = getDescriptionsFrom(diskOrMountPoint);
  if (desc) {
    const void *busPath = CFStringGetCStringPtr(CFDictionaryGetValue(desc,
                                                                     kDADiskDescriptionBusPathKey),
                                                kCFStringEncodingUTF8);

    if (busPath) {
      CFArrayRef esps = findEFIDisks();
      if (esps) {
        CFIndex count = CFArrayGetCount(esps);
        for (CFIndex i = 0; i < count; i++) {
          CFStringRef c = CFArrayGetValueAtIndex(esps, i);
          CFIndex length = CFStringGetLength(c);
          CFIndex size = CFStringGetMaximumSizeForEncoding(length, kCFStringEncodingUTF8) + 1;
          char *bsd = (char *)malloc(size);

          if (CFStringGetCString(c, bsd, size, kCFStringEncodingUTF8)) {
            CFDictionaryRef espDesc = getDescriptionsFrom(bsd);
            if (espDesc) {
              const void *espBusPath = CFStringGetCStringPtr(CFDictionaryGetValue(espDesc,
                                                                                  kDADiskDescriptionBusPathKey),
                                                             kCFStringEncodingUTF8);
              if (espBusPath) {
                if(strcmp(busPath, espBusPath) == 0) {
                  releaseESPArray(esps);
                  CFRelease(espDesc);
                  CFRelease(desc);
                  return bsd;
                }
              }
            }
            CFRelease(espDesc);
          }
          if (bsd) {
            free(bsd);
          }
        }
        releaseESPArray(esps);
      }
    }
    CFRelease(desc);
  }
  return NULL;
}

void releaseESPArray(CFArrayRef esps)
{
  CFIndex count = CFArrayGetCount(esps);
  for (CFIndex i = 0; i < count; i++) {
    CFStringRef s = CFArrayGetValueAtIndex(esps, i);
    if (s) {
      CFRelease(s);
    }
  }

  if (esps) {
    CFRelease(esps);
  }
}
