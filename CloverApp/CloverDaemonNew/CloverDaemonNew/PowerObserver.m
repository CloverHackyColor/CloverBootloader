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
    printf("Found /nvram.plist after waking from sleep, removing it:\n");
    NSError *error = nil;
    [[NSFileManager defaultManager] removeItemAtPath:@"/nvram.plist" error:&error];
    if (error == nil) {
      printf("/nvram.plist correctly removed.\n");
    } else {
      printf("%s\n", [[error localizedFailureReason] UTF8String]);
    }
  }
}
@end
