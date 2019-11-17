//
//  PowerObserver.m
//  CloverDaemonNew
//
//  Created by vector sigma on 09/11/2019.
//  Copyright © 2019 CloverHackyColor. All rights reserved.
//

#import "PowerObserver.h"

void powerCallback(void *refCon, io_service_t service, natural_t type, void *argument)
{
  [(PowerObserver *)CFBridgingRelease(refCon)powerDidReceiveMessage: type argument: argument];
}


@implementation PowerObserver
- (id)init {
  self = [super init];
  if (self) {
    [self registerPowerNotifications];
  }
  return self;
}

- (void)registerPowerNotifications {
  printf("Registering for Power notifications..\n");
  IONotificationPortRef notificationPort;
  root_port = IORegisterForSystemPower((void *)CFBridgingRetain(self),
                                       &notificationPort,
                                       powerCallback,
                                       &notifier);
  if (root_port) {
    CFRunLoopAddSource(CFRunLoopGetCurrent(),
                       IONotificationPortGetRunLoopSource(notificationPort),
                       kCFRunLoopDefaultMode);
  } else {
    printf("Can't register for Power notification.\n");
  }
  
}

- (void)powerDidReceiveMessage:(natural_t)type argument:(void *)argument {
  NSDateFormatter * formatter =  [[NSDateFormatter alloc] init];
  [formatter setDateFormat:@"yyyy-MMM-dd HH:mm:ss"];
  NSString *date = [formatter stringFromDate:[NSDate new]];
  
  switch (type) {
    case kIOMessageSystemWillSleep:{
      printf("System Sleep called at %s\n", [date UTF8String]);
      break;
    }
    case kIOMessageSystemHasPoweredOn:{
      printf("System awaken at %s\n", [date UTF8String]);
      [self performSelector:@selector(readAndCleanConflictingsKeys) withObject:nil afterDelay:3];
      break;
    }
  }
}

- (void)readAndCleanConflictingsKeys {
  if ([[NSFileManager defaultManager] fileExistsAtPath:@"/nvram.plist"]) {
    printf("Found /nvram.plist after waking from sleep, checking fro Apple GUIDs..\n");
    NSMutableDictionary *nvram = [NSMutableDictionary dictionaryWithContentsOfFile:@"/nvram.plist"];
    if (nvram != nil) {
      NSArray *keys = [nvram allKeys];
      int removed = 0;
      for (int i = 0; i < [keys count]; i++) {
        NSString *key = [keys objectAtIndex:i];
        if ([key hasPrefix:@"8BE4DF61-93CA-11D2-AA0D-00E098032B8C:"]) {
          printf("/nvram.plist contains %s\n", [key UTF8String]);
        }
        
        if ([key isEqualToString: @"efi-backup-boot-device"]) {
          printf("removing %s.\n", [key UTF8String]);
          [nvram removeObjectForKey:@"efi-backup-boot-device"];
        }
        if ([key isEqualToString: @"efi-backup-boot-device-data"]) {
          printf("removing %s.\n", [key UTF8String]);
          [nvram removeObjectForKey:@"efi-backup-boot-device-data"];
        }
        if ([key isEqualToString: @"install-product-url"]) {
          printf("removing %s.\n", [key UTF8String]);
          [nvram removeObjectForKey:@"install-product-url"];
        }
        if ([key isEqualToString: @"previous-system-uuid"]) {
          printf("removing %s.\n", [key UTF8String]);
          [nvram removeObjectForKey:@"previous-system-uuid"];
        }
      }
      
      if (removed > 0) {
        if (![nvram writeToFile:@"/nvram.plist" atomically:YES]) {
          printf("Error: cannot write back to /nvram.plist.\n");
        } else {
          system("/usr/bin/chflags hidden /nvram.plist");
        }
      }
    }
    
    
  }
}
@end
