//
//  main.m
//  daemonInstaller
//
//  Created by vector sigma on 24/11/2019.
//  Copyright © 2019 CloverHackyColor. All rights reserved.
//

#import <Foundation/Foundation.h>

#define kfm [NSFileManager defaultManager]

#define kNativeSwiftCorePath @"/usr/lib/swift/libswiftCore.dylib"
#define kDaemonName @"CloverDaemonNew"
#define kFrameworksName @"Frameworks"
#define kCloverSupportPath @"/Library/Application Support/Clover"

void run(NSString *cmd) {
  NSTask *task = [[NSTask alloc] init];
  NSPipe *pipe = [NSPipe new];
  
  task.standardOutput = pipe;
  task.standardError = pipe;
  
  NSFileHandle * fh = [pipe fileHandleForReading];
  
  [task setEnvironment:[[NSProcessInfo new] environment]];
  [task setLaunchPath:@"/bin/bash"];
  [task setArguments:@[ @"-c", cmd]];
  
  task.terminationHandler = ^(NSTask *theTask) {
    if (theTask.terminationStatus != 0) {
      exit(EXIT_FAILURE);
    }
  };
  [task launch];
  
  NSData *data = [fh readDataToEndOfFile];
  if (data) {
    NSString *output = [[NSString alloc] initWithData:data
                                             encoding:NSUTF8StringEncoding];
    printf("%s\n", [output UTF8String]);
  }
}


int main(int argc, const char * argv[]) {
  @autoreleasepool {
    NSString *myPath = [NSString stringWithFormat:@"%s", argv[0]];
    NSString *daemonSrc = [[myPath stringByDeletingLastPathComponent]
                            stringByAppendingPathComponent:kDaemonName];
    
    NSString *contentsPath = [[myPath stringByDeletingLastPathComponent]
                              stringByDeletingLastPathComponent];
    
    NSString *frameworksSrc = [contentsPath stringByAppendingPathComponent:kFrameworksName];
    
   
    if (![kfm fileExistsAtPath:kCloverSupportPath]) {
      if (![kfm createDirectoryAtPath:kCloverSupportPath withIntermediateDirectories:YES attributes:nil error:nil]) {
        printf("Error: can't create %s\n", [kCloverSupportPath UTF8String]);
        exit(EXIT_FAILURE);
      }
    }
    
    if (![kfm fileExistsAtPath:kNativeSwiftCorePath]) {
      run([NSString stringWithFormat:@"cp -R '%@' '%@/'", frameworksSrc, kCloverSupportPath]);
    }
    run([NSString stringWithFormat:@"'%@' --install", daemonSrc]);
  }
  return 0;
}
