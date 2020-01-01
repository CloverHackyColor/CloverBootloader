//
//  main.m
//  Cloverhelper
//
//  Created by vector sigma on 26/10/2019.
//  Copyright © 2019 CloverHackyColor. All rights reserved.
//


/*
 This command line must be run as root and it is responsible for creating the Clover tree
 and to install boot sectors and drivers.
 */

// TODO: redone in swift as swift command line are now small in size, with no hurry...
#import <Foundation/Foundation.h>

#define ktempLogPath "/tmp/cltmplog"
#define kfm [NSFileManager defaultManager]

#pragma mark -
#pragma mark copy and replace files
#pragma mark -

void cleanUp() {
  NSTask *task = [[NSTask alloc] init];
  
  [task setEnvironment:[[NSProcessInfo new] environment]];
  [task setLaunchPath:@"/bin/bash"];
  [task setArguments:@[ @"-c", @"rm -f /tmp/Clover* && rm -f /tmp/boot0* && rm -f /tmp/boot1*" ]];

  [task launch];
}

NSString *gTargetVolume;

void saveLog() {
  if (gTargetVolume != nil
      && [kfm fileExistsAtPath:gTargetVolume]
      && [kfm fileExistsAtPath:@ktempLogPath]) {
    NSString *finalLogPath =
    [gTargetVolume stringByAppendingPathComponent:@"EFI/CLOVER/Clover.app_install.log"];
    if ([kfm fileExistsAtPath: finalLogPath]) {
      [kfm removeItemAtPath:finalLogPath error:nil];
    }
    NSData *log = [NSData dataWithContentsOfFile:@ktempLogPath];
    if (log != nil) {
      [log writeToFile:finalLogPath atomically:NO];
    }
  }
}

void addToLog(NSString *str) {
  if (![kfm fileExistsAtPath:@ktempLogPath]) {
    system([[NSString stringWithFormat:@"echo > %s", ktempLogPath] UTF8String]);
  }
  NSFileHandle *fh = [NSFileHandle fileHandleForWritingAtPath:@ktempLogPath];
  [fh seekToEndOfFile];
  [fh writeData:[str dataUsingEncoding:NSUTF8StringEncoding]];
  [fh closeFile];
}

void post(NSString *format, ...) {
  va_list arguments;
  va_start(arguments, format);
  NSString *str = [[NSString alloc] initWithFormat:format arguments:arguments];
  addToLog(str);
  va_end(arguments);
  printf("%s", [str UTF8String]);
  
}

void exitWithMessage(NSString *format, ...) {
  va_list arguments;
  va_start(arguments, format);
  NSString *str = [[NSString alloc] initWithFormat:format arguments:arguments];
  
  va_end(arguments);
  printf("%s", [str UTF8String]);
  addToLog(str);
  saveLog();
  cleanUp();
  exit(EXIT_FAILURE);
}

BOOL copyReplace(NSString *source, NSString *dest, NSDictionary *attributes) {
  NSError *err = nil;
  NSString *upperDir = [dest stringByDeletingLastPathComponent];
  
  if ([upperDir isEqualToString:@"/"]) {
    post(@"+ ../%@\n", [dest lastPathComponent]);
  } else {
    post(@"+ ../%@/%@\n",
           [[dest stringByDeletingLastPathComponent] lastPathComponent], [dest lastPathComponent]);
  }
  
  if (![kfm fileExistsAtPath:source]) {
    post(@"Error: %@ doesn't exist\n", source);
    return NO;
  }
  // remove destination if already exist
  if ([kfm fileExistsAtPath:dest]) {
    [kfm removeItemAtPath:dest error:&err];
  }
  
  // create upper directory if needed
  if (err == nil) {
    if (![kfm fileExistsAtPath:upperDir]) {
      [kfm createDirectoryAtPath:upperDir
     withIntermediateDirectories:YES
                      attributes:attributes
                           error:&err];
    }
  }
  
  // copy the new item
  if (err == nil) {
    if ([kfm copyItemAtPath:source toPath:dest error:&err] && attributes) {
      [kfm setAttributes:attributes ofItemAtPath:dest error:nil];
    }
  }
  
  // print any errors if any
  if (err != nil) {
    post(@"%@\n", err.description);
  }
  
  return (err == nil);
}

#pragma mark -
#pragma mark Main
#pragma mark -

int main(int argc, char * const * argv) {
  @autoreleasepool {
    if ([kfm fileExistsAtPath:@ktempLogPath]) {
      [kfm removeItemAtPath:@ktempLogPath error:nil];
    }
    
    NSDateFormatter * df =  [NSDateFormatter new];
    [df setDateFormat:@"yyyy-MMM-dd HH:mm:ss"];
    NSLocale *locale = [[NSLocale alloc]
                        initWithLocaleIdentifier:@"en_US"];
    [df setLocale:locale];
    NSString *dateString = [df stringFromDate:[NSDate new]];

    if (geteuid() != 0) {
      exitWithMessage(@"Error: you don't have root permissions\n");
    }
    
    NSDictionary *CloverappDict = [NSDictionary dictionaryWithContentsOfFile:@"/tmp/Cloverapp"];
    if (CloverappDict == nil) {
      exitWithMessage(@"Error: can't load Cloverapp dictionary\n");
    }
    cleanUp();
    
    NSString *bootSectorsInstall    = @"/tmp/bootsectors-install";
    
    NSString *targetVol             = [CloverappDict objectForKey:@"targetVol"];
    NSString *disk                  = [CloverappDict objectForKey:@"disk"];
    NSString *filesystem            = [CloverappDict objectForKey:@"filesystem"];
    NSString *shemeMap              = [CloverappDict objectForKey:@"shemeMap"];
    NSString *boot0                 = [CloverappDict objectForKey:@"boot0"];
    NSString *boot1                 = [CloverappDict objectForKey:@"boot1"];
    NSString *boot2                 = [CloverappDict objectForKey:@"boot2"];
    NSString *cloverv2              = [CloverappDict objectForKey:@"CloverV2"];
    NSString *boot1installPath      = [CloverappDict objectForKey:@"boot1install"];
    NSString *bootSectorsInstallSrc = [CloverappDict objectForKey:@"bootsectors-install"];
    NSString *backUpPath            = [CloverappDict objectForKey:@"BackUpPath"];
    NSString *version               = [CloverappDict objectForKey:@"version"];
    BOOL isESP                      = [[CloverappDict objectForKey:@"isESP"] boolValue];

    post(@"%@, %@\n\n", version, dateString);
    post(@"My Path = %s\n", argv[0]);
    if (backUpPath != nil) {
      post(@"Backup made at:\n%@.\n", backUpPath);
    }
    
    if ([kfm fileExistsAtPath:bootSectorsInstall]) {
      post(@"Note: found old bootsectors-install..removing it..\n");
      if (![kfm removeItemAtPath:bootSectorsInstall error:nil]) {
        exitWithMessage(@"Error: can't remove old bootsectors-install.");
      }
    }
    
    if (bootSectorsInstallSrc != nil) {
      post(@"bootSectorsInstallSrc = %@\n", bootSectorsInstallSrc);
    }
    
    BOOL alt = NO;
    if ([CloverappDict objectForKey:@"alt"] != nil) {
      alt = [[CloverappDict objectForKey:@"alt"] boolValue];
    }
    
#pragma mark Preferences dict init
    NSMutableDictionary *preferences = [NSMutableDictionary new];
    
#pragma mark Attributes init
    NSDictionary * attributes = [NSDictionary dictionaryWithObjectsAndKeys:
                                 [NSNumber numberWithShort:0777], NSFilePosixPermissions, nil];
#pragma mark Check Volume
    BOOL isDir = NO;
    
    if (targetVol == nil) {
      exitWithMessage(@"Error: option targetVol [path to volume]  not specified\n");
    }
    gTargetVolume = targetVol;
    
    if (![kfm fileExistsAtPath:targetVol]) {
      exitWithMessage(@"Error: target volume \"%@\" doesn't exist.\n", targetVol);
    }
    
    if (cloverv2 == nil || ![kfm fileExistsAtPath:cloverv2]) {
      exitWithMessage(@"Error: cannot found CloverV2 directory\n");
    }
    
    post(@"Target volume: %@\n", targetVol);
    /*
    // check if target volume is writable
    NSString *volEnds = targetVol;
    if (![volEnds hasSuffix:@"/"]) {
      volEnds = [targetVol stringByAppendingString:@"/"];
    }
    
    
    if (![kfm isWritableFileAtPath:volEnds]) {
      exitWithMessage(@"Error: target volume \"%@\" is not writable.\n", targetVol);
    }
    */
#pragma mark Check paths
    // check other options before proceed
    NSString *boot0Path = nil, *boot1Path = nil, *boot2Path = nil;
    if (boot0 != nil) {
      post(@"boot0: %@\n", boot0);
      [preferences setValue:boot0 forKey:@"boot0"];
      boot0Path = [[cloverv2 stringByAppendingPathComponent:@"BootSectors"] stringByAppendingPathComponent:boot0];
      if (![kfm fileExistsAtPath:boot0Path]) {
        exitWithMessage(@"Error: cannot found \"%@\".\n", boot0);
      }
    }
    
    if (boot1 != nil) {
      post(@"boot1: %@\n", boot1);
      boot1Path = [[cloverv2 stringByAppendingPathComponent:@"BootSectors"] stringByAppendingPathComponent:boot1];
      if (![kfm fileExistsAtPath:boot1Path]) {
        exitWithMessage(@"Error: cannot found \"%@\".\n", boot1);
      }
    }
    
    if (boot2 != nil) {
      post(@"boot2: %@\n", boot2);
      [preferences setValue:boot2 forKey:@"boot2"];
      boot2Path = [[cloverv2 stringByAppendingPathComponent:@"Bootloaders/x64"] stringByAppendingPathComponent:boot2];
      if (![kfm fileExistsAtPath:boot2Path]) {
        exitWithMessage(@"Error: cannot found \"%@\".\n", boot2);
      }
    }
    
    NSError *err = nil;
    if ([kfm fileExistsAtPath:
         [targetVol stringByAppendingPathComponent:@"EFI/CLOVER"] isDirectory:&isDir]) {
      if (!isDir) {
        [kfm removeItemAtPath:
         [targetVol stringByAppendingPathComponent:@"EFI/CLOVER"] error:&err];
      }
    }
    
    if (![kfm fileExistsAtPath:[targetVol stringByAppendingPathComponent:@"EFI/CLOVER"]]) {
      [kfm createDirectoryAtPath:[targetVol stringByAppendingPathComponent:@"EFI/CLOVER"]
     withIntermediateDirectories:YES
                      attributes:attributes
                           error:&err];
    }
    
    if (err != nil) {
      exitWithMessage(@"%@\n", err.description);
    }
    
#pragma mark Create directories
    NSArray *sub = [NSArray arrayWithObjects:
                    @"EFI/BOOT",
                    @"EFI/CLOVER/ACPI/origin",
                    @"EFI/CLOVER/ACPI/patched",
                    @"EFI/CLOVER/ACPI/WINDOWS",
                    @"EFI/CLOVER/kexts/10",
                    @"EFI/CLOVER/kexts/10_recovery",
                    @"EFI/CLOVER/kexts/10_installer",
                    @"EFI/CLOVER/kexts/10.11",
                    @"EFI/CLOVER/kexts/10.12",
                    @"EFI/CLOVER/kexts/10.13",
                    @"EFI/CLOVER/kexts/10.14",
                    @"EFI/CLOVER/kexts/10.15",
                    @"EFI/CLOVER/kexts/Other",
                    @"EFI/CLOVER/misc",
                    @"EFI/CLOVER/ROM",
                    nil];
    
    for (int i = 0; i < [sub count]; i++) {
      err = nil;
      NSString *fp = [targetVol stringByAppendingPathComponent:[sub objectAtIndex:i]];
      if (![kfm fileExistsAtPath:fp]) {
        [kfm createDirectoryAtPath:fp withIntermediateDirectories:YES attributes:attributes error:&err];
        if (err != nil) {
          exitWithMessage(@"%@\n", err.description);
        }
      }
    }
    
#pragma mark Install Clover and drivers
    post(@"\nInstalling/Updating Clover:\n");
    if (!copyReplace([cloverv2 stringByAppendingPathComponent:@"EFI/BOOT/BOOTX64.efi"],
                     [targetVol stringByAppendingPathComponent:@"EFI/BOOT/BOOTX64.efi"],
                     attributes)) {
      exitWithMessage(@"Error: cannot copy BOOTX64.efi to destination.\n");
    }
    
    if (!copyReplace([cloverv2 stringByAppendingPathComponent:@"EFI/CLOVER/CLOVERX64.efi"],
                     [targetVol stringByAppendingPathComponent:@"EFI/CLOVER/CLOVERX64.efi"],
                     attributes)) {
      exitWithMessage(@"Error: cannot copy CLOVERX64.efi to destination.\n");
    }
    
    post(@"\nInstalling/Updating drivers:\n");
    NSArray * toDelete = [CloverappDict objectForKey:@"toDelete"];
    NSArray * UEFI = [CloverappDict objectForKey:@"UEFI"];
    NSArray * BIOS = [CloverappDict objectForKey:@"BIOS"];
    NSString *UEFIdest = [targetVol stringByAppendingPathComponent:@"EFI/CLOVER/drivers/UEFI"];
    NSString *BIOSdest = [targetVol stringByAppendingPathComponent:@"EFI/CLOVER/drivers/BIOS"];
    
    if (UEFI != nil) {
      for (int i = 0; i < [UEFI count]; i++) {
        NSString *dpath = [UEFI objectAtIndex:i];
        NSString *dname = [dpath lastPathComponent];
        if (!copyReplace(dpath, [UEFIdest stringByAppendingPathComponent:dname], attributes)) {
          exitWithMessage(@"Error: cannot copy %@ to destination.\n", dpath);
        }
      }
    }
    
    if (BIOS != nil) {
      for (int i = 0; i < [BIOS count]; i++) {
        NSString *dpath = [BIOS objectAtIndex:i];
        NSString *dname = [dpath lastPathComponent];
        if (!copyReplace(dpath, [BIOSdest stringByAppendingPathComponent:dname], attributes)) {
          exitWithMessage(@"Error: cannot copy %@ to destination.\n", dpath);
        }
      }
    }
    
    if (toDelete != nil) {
      for (int i = 0; i < [toDelete count]; i++) {
        NSString *dpath = [toDelete objectAtIndex:i];
        err = nil;
        if ([kfm fileExistsAtPath:dpath]) {
          post(@"- ../%@/%@\n",
                 [[dpath stringByDeletingLastPathComponent] lastPathComponent],
                 [dpath lastPathComponent]);
          [kfm removeItemAtPath:dpath error:&err];
          if (err != nil) {
            exitWithMessage(@"%@\n", err.description);
          }
        }
      }
    }
    
#pragma mark Install tools
    NSString *cv2tools = [cloverv2 stringByAppendingPathComponent:@"EFI/CLOVER/tools"];
    if ([kfm fileExistsAtPath:cv2tools]) {
      post(@"\nInstalling/Updating tools:\n");
      err = nil;
      NSArray *tools = [kfm contentsOfDirectoryAtPath:cv2tools error:&err];
      if (err == nil && tools != nil) {
        for (int i = 0; i < [tools count]; i++) {
          NSString *t = [tools objectAtIndex:i];
          if ([[t pathExtension] isEqualToString:@"efi"]) {
            if (!copyReplace([cv2tools stringByAppendingPathComponent:t],
                             [[targetVol stringByAppendingPathComponent:@"EFI/CLOVER/tools"]
                              stringByAppendingPathComponent:t],
                             attributes)) {
              exitWithMessage(@"Error: cannot copy %@ to destination.\n", t);
            }
          }
        }
      }
    }
    err = nil;
    
#pragma mark Install docs
    NSString *cv2docs = [cloverv2 stringByAppendingPathComponent:@"EFI/CLOVER/doc"];
    if ([kfm fileExistsAtPath:cv2tools]) {
      post(@"\nInstalling/Updating doc:\n");
      err = nil;
      NSArray *docs = [kfm contentsOfDirectoryAtPath:cv2docs error:&err];
      if (err == nil && docs != nil) {
        for (int i = 0; i < [docs count]; i++) {
          NSString *d = [docs objectAtIndex:i];
          if (![d hasPrefix:@"."]) {
            NSString *docpath = [cv2docs stringByAppendingPathComponent:d];
            NSDictionary *docattr = [kfm attributesOfItemAtPath:docpath error:nil];
            if (!copyReplace(docpath,
                             [[targetVol stringByAppendingPathComponent:@"EFI/CLOVER/doc"]
                              stringByAppendingPathComponent:d],
                             docattr)) {
              
              //exitWithMessage(@"Error: cannot copy %@ to destination.\n", d);
              // ... will not fail for a doc..
              post(@"Error: cannot copy %@ to destination.\n", d);
            }
          }
        }
      }
    }
    err = nil;
    
#pragma mark Create config.plist
    NSString *configPath = [targetVol stringByAppendingPathComponent:@"EFI/CLOVER/config.plist"];
    NSString *configSamplePath = [cloverv2 stringByAppendingPathComponent:@"EFI/CLOVER/config-sample.plist"];
    if (![kfm fileExistsAtPath:configPath]) {
      post(@"\nInstalling  config.plist:\n");
      copyReplace(configSamplePath, configPath, attributes);
    }
    
#pragma mark Install theme
    // install the theme defined in config if doesn't exist
    NSDictionary *config = [NSDictionary dictionaryWithContentsOfFile:configPath];
    NSString *themesSourceDir = [cloverv2 stringByAppendingPathComponent:@"themespkg"];
    NSString *themesDestDir   = [targetVol stringByAppendingPathComponent:@"EFI/CLOVER/themes"];
    
    if (config != nil && ![kfm fileExistsAtPath:themesDestDir]) {
      NSString *theme = nil;
      NSDictionary *GUI = [config objectForKey:@"GUI"];
      if (GUI != nil) {
        theme = [GUI objectForKey:@"Theme"];
      }
      
      // don't overwrite the theme if already exist as one from CTM can be newer
      if (theme != nil && ![kfm fileExistsAtPath:
                            [themesDestDir stringByAppendingPathComponent:theme]]) {
        post(@"\nInstalling Theme \"%@\":\n", theme);
        copyReplace([themesSourceDir stringByAppendingPathComponent:theme],
                    [themesDestDir stringByAppendingPathComponent:theme],
                    attributes);
      }
    }
    
#pragma mark Stage 2 installation
    if (boot2Path != nil) {
      post(@"\nInstalling stage 2..\n");
      if (!copyReplace(boot2Path, [targetVol stringByAppendingPathComponent:@"boot"], attributes)) {
        exitWithMessage(@"Error: cannot copy bootloader to detination.\n");
      }
      
      if (alt) {
        [preferences setValue:[NSNumber numberWithBool:YES] forKey:@"boot2Alt"];
        NSString *altPath = [boot2Path stringByDeletingLastPathComponent];
        NSArray *files = [kfm contentsOfDirectoryAtPath:altPath error:nil];
        if (files) {
          for (int i = 0; i < [files count]; i++) {
            NSString *f = [files objectAtIndex:i];
            if ([f hasPrefix:@"boot"]) {
              post(@"\nInstalling stage 2 \"%@ (alt)\".\n", f);
              copyReplace([altPath stringByAppendingPathComponent: f],
                          [targetVol stringByAppendingPathComponent:f],
                          attributes);
            }
          }
        }
      }
    }
    
#pragma mark Write Preferences
    NSString *prefPath = [targetVol stringByAppendingPathComponent:@"EFI/CLOVER/pref.plist"];
    if ([preferences writeToFile:prefPath atomically:YES]) {
      [kfm setAttributes:attributes ofItemAtPath:prefPath error:nil];
    }
    
    saveLog();
#pragma mark Boot sectors installation
    if (boot0Path != nil && boot1Path != nil && bootSectorsInstallSrc != nil) {
      // copy bootsectors-install
      [kfm copyItemAtPath:bootSectorsInstallSrc toPath:bootSectorsInstall error:nil];
      NSDictionary *bsiAttr = [kfm attributesOfItemAtPath:bootSectorsInstallSrc error:nil];
      [kfm setAttributes:bsiAttr ofItemAtPath:bootSectorsInstall error:nil];
      
      // copy boot0
      [kfm copyItemAtPath:boot0Path toPath:[@"/tmp" stringByAppendingPathComponent:boot0] error:nil];
      // copy boot1
      [kfm copyItemAtPath:boot1Path toPath:[@"/tmp" stringByAppendingPathComponent:boot1] error:nil];
      
      [kfm copyItemAtPath:boot1installPath toPath:@"/tmp/boot1-install" error:nil];
      
      /*
       /tmp/bootsectors-install disk4s1 hfs FDisk_partition_scheme boot0af boot1h
       */
      NSTask *task = [[NSTask alloc] init];
      NSPipe *pipe = [NSPipe new];
      
      task.standardOutput = pipe;
      task.standardError = pipe;
      
      NSFileHandle * fh = [pipe fileHandleForReading];
      
      [task setEnvironment:[[NSProcessInfo new] environment]];
      [task setLaunchPath:bootSectorsInstall];
      
      NSString *esp = isESP ? @"ESP" : @"OTHER";
      [task setArguments:@[ disk, filesystem, shemeMap, boot0, boot1, esp ]];
      
      task.terminationHandler = ^(NSTask *theTask) {
        if (theTask.terminationStatus != 0) {
          exitWithMessage(@"Error: failed installing boot sectors.\n");
        }
      };
      [task launch];
      //[task waitUntilExit];
      NSData *data = [fh readDataToEndOfFile];
      if (data) {
        NSString *output = [[NSString alloc] initWithData:data
                                                 encoding:NSUTF8StringEncoding];
        post(@"%@\n", output);
      }
      
    } else {
      saveLog();
      if (isESP) {
        NSTask *task = [[NSTask alloc] init];
        [task setEnvironment:[[NSProcessInfo new] environment]];
        [task setLaunchPath:@"/usr/sbin/diskutil"];
        [task setArguments:@[ @"umount", @"force", disk ]];
        [task launch];
      }
    }
    cleanUp();
    exit(EXIT_SUCCESS);
    
  }
  return 0;
}

