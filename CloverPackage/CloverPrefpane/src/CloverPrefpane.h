//
//  CloverPrefpane.h
//  CloverPrefpane
//
//  Created by JrCs on 03/05/13.
//  Copyright (c) 2013 ProjectOSX. All rights reserved.
//

#import <PreferencePanes/PreferencePanes.h>
#import <ServiceManagement/ServiceManagement.h>
#import <SecurityInterface/SFAuthorizationView.h>

@interface CloverPrefpane : NSPreferencePane {
    CFStringRef bundleID;
    NSFileManager *fileManager;
    NSString* plistPath;

    NSString* cloverTheme;
    NSString* mountEFI;
    NSString* nvRamDisk;
    NSString* logLineCount;
    NSString* logEveryBoot;

    IBOutlet NSPopUpButton *popUpCheckInterval;
    IBOutlet NSTextField *LastRunDate;
    IBOutlet NSDateFormatter *dateFormatter;
    IBOutlet NSButton *checkNowButton;
    IBOutlet SFAuthorizationView *authView;
    IBOutlet NSTextField *lastBootedRevision;
    IBOutlet NSTextField *lastInstalledRevision;
}

@property (retain) NSFileManager *fileManager;
@property (retain) NSString* plistPath;
@property (assign) NSString* cloverTheme;
@property (assign) NSString* mountEFI;
@property (assign) NSString* nvRamDisk;
@property (assign) NSString* logLineCount;
@property (assign) NSString* logEveryBoot;

- (id) initWithBundle:(NSBundle *)bundle;
- (void) mainViewDidLoad;
- (IBAction) checkNow:(id)sender;
- (IBAction) configureAutomaticUpdates:(id)sender;
- (IBAction) updateTheme:(id)sender;
- (IBAction) updateMountEFI:(id)sender;
- (IBAction) updateNVRamDisk:(id)sender;
- (IBAction) updateLogLineCount:(id)sender;
- (IBAction) updateLogEveryBoot:(id)sender;

- (BOOL)isUnlocked;

- (NSString*) getNVRamKey:(const char *)key;
- (OSErr) setNVRamKey:(const char *)key withValue:(const char *)value;
- (unsigned int) getUIntPreferenceKey:(CFStringRef)key
                             forAppID:(CFStringRef)appID
                          withDefault:(unsigned int)defaultValue;
- (void) setPreferenceKey:(CFStringRef)key
                 forAppID:(CFStringRef)appID
                  fromInt:(int)value;
@end
