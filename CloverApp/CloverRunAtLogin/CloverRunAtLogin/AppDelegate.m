//
//  AppDelegate.m
//  CloverRunAtLogin
//
//  Created by vector sigma on 31/10/2019.
//  Copyright © 2019 CloverHackyColor. All rights reserved.
//

#import "AppDelegate.h"

@interface AppDelegate ()

@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
  NSString *appID = @"org.slice.CloverRunAtLogin";
  BOOL running = NO;
  NSString *appPath = [self myPath];
  
  NSArray *runnings = [[NSWorkspace sharedWorkspace] runningApplications];
  for (NSRunningApplication *app in runnings) {
    if ([app.bundleIdentifier isEqualToString:appID] && [app.bundleURL.path isEqualToString:appPath]) {
      running = YES;
    }
  }
  
  if (!running) {
    [[NSWorkspace sharedWorkspace] launchApplication:appPath];
  }
  [self performSelector:@selector(terminate) withObject:nil afterDelay:3];
}

- (NSString *)myPath {
  NSURL *myUrl = [[NSBundle mainBundle] bundleURL];
  int count = 4;
  while (count > 0) {
    myUrl = myUrl.URLByDeletingLastPathComponent;
    count--;
  }
  return myUrl.path;
}

- (void)terminate {
  [NSApp terminate:nil];
}

@end
