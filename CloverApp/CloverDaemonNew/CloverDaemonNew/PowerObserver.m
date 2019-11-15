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
      // who will come first? ..I don't know so do that after 3 seconds
      [self performSelector:@selector(removeNVRAMPlist) withObject:nil afterDelay:3];
      break;
    }
  }
}

- (void)removeNVRAMPlist {
  if ([[NSFileManager defaultManager] fileExistsAtPath:@"/nvram.plist"]) {
    printf("Found /nvram.plist after waking from sleep, checking fro Apple GUIDs..\n");
    NSDictionary *nvram = [NSDictionary dictionaryWithContentsOfFile:@"/nvram.plist"];
    if (nvram != nil) {
      NSArray *keys = [nvram allKeys];
      for (int i = 0; i < [keys count]; i++) {
        NSString *key = [keys objectAtIndex:i];
        if ([key hasPrefix:@"8BE4DF61-93CA-11D2-AA0D-00E098032B8C:"]) {
          printf("/nvram.plist contains %s\n", [key UTF8String]);
        }
      }
    }
    
    
  }
}
@end
