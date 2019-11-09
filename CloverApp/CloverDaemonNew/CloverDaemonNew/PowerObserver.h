//
//  PowerObserver.h
//  CloverDaemonNew
//
//  Created by vector sigma on 09/11/2019.
//  Copyright © 2019 CloverHackyColor. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <IOKit/pwr_mgt/IOPMLib.h>
#import <IOKit/IOMessage.h>
NS_ASSUME_NONNULL_BEGIN

@interface PowerObserver : NSObject {
  io_connect_t root_port;
  io_object_t notifier;
}
- (void)registerPowerNotifications;
- (void)powerDidReceiveMessage:(natural_t)type argument:(void *)argument;

- (void)removeNVRAMPlist;
@end

NS_ASSUME_NONNULL_END
