//
//  Support.h
//  espfinder
//
//  Created by vector sigma on 25/07/2019.
//  Copyright © 2019 Clover. All rights reserved.
//

#ifndef Support_h
#define Support_h
#import <Foundation/Foundation.h>


#define fm [NSFileManager defaultManager]
#define kcmdname @"espfinder"
#define kcmdver @"0.3"

#define kUsage "Usage examples:\nespfinder -t disk2s4\nor\nespfinder -t /\nor\nespfinder -t /Volumes/VolName\nor\nespfinder -d -t /\n\n-t\tmount point or BSD Name, required option.\n\n-d\ta dump at ~/Desktop/espfinder.plist will be made.\n\n"

#define kdumpPath [NSHomeDirectory() stringByAppendingPathComponent:@"Desktop/espfinder.plist"]


#define kWholeDisks                       "WholeDisks"                        /* ( NSArray  ) */
#define kAllDisks                         "AllDisks"                          /* ( NSArray  ) */
#define kAllDisksAndPartitions            "AllDisksAndPartitions"             /* ( NSArray  ) */
#define kDeviceIdentifier                 "DeviceIdentifier"                  /* ( NSString ) */
#define kPartitions                       "Partitions"                        /* ( NSArray  ) */
#define kContent                          "Content"                           /* ( NSString ) */
#define kEFIContent                       "EFI"                               /* ( NSString ) */

#define kContainers                       "Containers"                        /* ( NSArray  ) */
#define kContainerReference               "ContainerReference"                /* ( NSString ) */
#define kDesignatedPhysicalStore          "DesignatedPhysicalStore"           /* ( NSString ) */

#define kRAIDSetMembers                   "RAIDSetMembers"                    /* ( NSArray  ) */

#define kAPFSPhysicalStores               "APFSPhysicalStores"                /* ( NSArray  ) */

#define kCoreStorageLogicalVolumeGroups   "CoreStorageLogicalVolumeGroups"    /* ( NSArray  ) */
#define kCoreStorageLogicalVolumeFamilies "CoreStorageLogicalVolumeFamilies"  /* ( NSArray  ) */
#define kCoreStorageLogicalVolumes        "CoreStorageLogicalVolumes"         /* ( NSArray  ) */
#define kCoreStoragePhysicalVolumes       "CoreStoragePhysicalVolumes"        /* ( NSArray  ) */
#define kCoreStorageUUID                  "CoreStorageUUID"                   /* ( NSString ) */

/*!
 * @function   usage
 * @abstract   Show usage with examples.
 */
static void usage() {
  printf(kUsage);
  exit(0);
}

/*!
 * @function   error
 * @abstract   Show usage with examples and exit the program.
 * @param      error The error c string to show.
 */
static void error(const char *error) {
  printf("Error: %s\n", error);
  exit(1);
}

/*!
 * @function   diskutil
 * @abstract   Execute disktutil and return an NSString object with the output of diskutil.
 * @param      args Any argument supported by diskutil, see man diskutil.
 */
static NSString *diskutil(NSString *args)  {
  NSTask *t = [NSTask new];
  t.launchPath = @"/bin/bash";
  
  t.arguments = [NSArray arrayWithObjects:@"-c",
                 [NSString stringWithFormat:@"diskutil %@", args],
                 nil];
  
  NSPipe * o = [NSPipe pipe];
  t.standardError = o;
  t.standardOutput = o;
  
  [t launch];
  [t waitUntilExit];
  
  NSFileHandle * fh = [o fileHandleForReading];
  NSData * data = [fh readDataToEndOfFile];
  return (data != nil) ? [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding] : nil;
}

/*!
 * @function   getAllDisks
 * @abstract   Execute disktutil list -plist.
 */
static NSDictionary *getAllDisks()  {
  NSDictionary* dict = nil;
  NSString *xml = diskutil(@"list -plist");
  
  if (xml != nil) {
    NSString *err = nil;
    NSPropertyListFormat format;
    dict = [NSPropertyListSerialization propertyListFromData:
            [xml dataUsingEncoding:NSUTF8StringEncoding]
                                            mutabilityOption:NSPropertyListImmutable
                                                      format:&format
                                            errorDescription:&err];
  }
  
  return dict;
}

/*!
 * @function   getAllAPFSDisks
 * @abstract   Execute disktutil apfs list -plist.
 */
static NSDictionary *getAllAPFSDisks()  {
  NSDictionary* dict = nil;
  NSString *xml = diskutil(@"apfs list -plist");
  
  if (xml != nil) {
    NSString *err = nil;
    NSPropertyListFormat format;
    dict = [NSPropertyListSerialization propertyListFromData:
            [xml dataUsingEncoding:NSUTF8StringEncoding]
                                            mutabilityOption:NSPropertyListImmutable
                                                      format:&format
                                            errorDescription:&err];
  }
  return dict;
}

/*!
 * @function   getAllCSDisks
 * @abstract   Execute disktutil cs list -plist.
 */
static NSDictionary *getAllCSDisks()  {
  NSDictionary* dict = nil;
  NSString *xml = diskutil(@"cs list -plist");
  
  if (xml != nil) {
    NSString *err = nil;
    NSPropertyListFormat format;
    dict = [NSPropertyListSerialization propertyListFromData:
            [xml dataUsingEncoding:NSUTF8StringEncoding]
                                            mutabilityOption:NSPropertyListImmutable
                                                      format:&format
                                            errorDescription:&err];
  }
  return dict;
}

/*!
 * @function   getDiskInfo
 * @abstract   Execute disktutil info -plist.
 * @param      disk A BSD Name, mount point path or UUID.
 */
static NSDictionary *getDiskInfo(NSString *disk)  {
  NSDictionary* dict = nil;
  NSString *xml = diskutil([NSString stringWithFormat:@"info -plist %@", disk]);
  
  if (xml != nil) {
    NSString *err = nil;
    NSPropertyListFormat format;
    dict = [NSPropertyListSerialization propertyListFromData:
            [xml dataUsingEncoding:NSUTF8StringEncoding]
                                            mutabilityOption:NSPropertyListImmutable
                                                      format:&format
                                            errorDescription:&err];
  }
  return dict;
}

/*!
 * @function   getBSDNameAndUUIDFrom
 * @abstract   Return the disk object for the given device or mount point.
 * @param      diskORmtp A BSD Name or a mount point.
 */
static NSString *getBSDNameAndUUIDFrom(const char *diskORmtp) {
  NSString *result = nil;
  NSString *object = [NSString stringWithFormat:@"%s", diskORmtp];
  DADiskRef           diskRef = NULL;
  DASessionRef        session;
  CFURLRef            volURL;
  
  session = DASessionCreate(NULL);
  if (session == NULL) {
    return nil;
  }
  
  if ([object hasPrefix:@"/"] && ![object hasPrefix:@"/dev/disk"]
      && ![object hasPrefix:@"disk"] && [fm fileExistsAtPath:object]) {
    volURL = (CFURLRef)CFBridgingRetain([NSURL fileURLWithPath:object]);
    diskRef = DADiskCreateFromVolumePath(NULL, session, volURL);
  } else if (([object hasPrefix:@"/dev/disk"] || [object hasPrefix:@"disk"])) {
    if ([object hasPrefix:@"/dev/disk"]) {
      object = [object stringByReplacingOccurrencesOfString:@"/dev/" withString:@""];
    }
    diskRef = DADiskCreateFromBSDName(NULL, session, [object UTF8String]);
  } else {
    return nil;
  }
  
  if (session) {
    if (diskRef) {
      CFDictionaryRef dict = DADiskCopyDescription(diskRef);
      
      if (dict) {
        CFUUIDRef bsdname = CFDictionaryGetValue(dict, kDADiskDescriptionMediaBSDNameKey);
        CFUUIDRef uuid = CFDictionaryGetValue(dict, kDADiskDescriptionMediaUUIDKey);
        
        if (!uuid) {
          uuid = CFUUIDCreateFromString(kCFAllocatorDefault, CFSTR("00000000-0000-0000-0000-000000000000"));
        }
        
        result = [NSString stringWithFormat:@"%@ %@",
                  bsdname,
                  CFUUIDCreateString(kCFAllocatorDefault,
                                     uuid)];
        
        CFRelease(uuid);
        CFRelease(bsdname);
      }
      
      CFRelease(diskRef);
    }
    
  }
  return result;
}

#endif /* Support_h */
