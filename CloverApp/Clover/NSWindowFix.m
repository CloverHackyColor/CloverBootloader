//
//  NSWindowFix.m
//  Clover
//
//  Created by vector sigma on 29/11/2019.
//  Copyright © 2019 CloverHackyColor. All rights reserved.
//

#import "NSWindowFix.h"

@implementation NSWindow (popoverFix)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wobjc-protocol-method-implementation"
- (BOOL)canBecomeKeyWindow {
  /*
  if (floor(kCFCoreFoundationVersionNumber) < kCFCoreFoundationVersionNumber10_11
      && [[self className] isEqualToString:@"NSStatusBarWindow"]) {
    return YES;
  }
  return [self canBecomeKeyWindow];
  */
  
  return YES;
}
#pragma clang diagnostic pop
@end
