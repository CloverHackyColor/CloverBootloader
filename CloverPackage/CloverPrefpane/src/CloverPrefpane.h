//
//  CloverPrefpane.h
//  CloverPrefpane
//
//  Created by JrCs on 03/05/13.
//  Copyright (c) 2013 ProjectOSX. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#import <PreferencePanes/PreferencePanes.h>
#import <ServiceManagement/ServiceManagement.h>
#import <SecurityInterface/SFAuthorizationView.h>

@interface CloverPrefpane : NSPreferencePane {
    io_registry_entry_t _ioRegEntryRef;
    NSString* agentPlistPath;

    IBOutlet NSPopUpButton *popUpCheckInterval;
    IBOutlet NSTextField *LastRunDate;
    IBOutlet NSButton *checkNowButton;
    IBOutlet SFAuthorizationView *authView;
    IBOutlet NSTextField *lastBootedRevision;
    IBOutlet NSTextField *lastInstalledRevision;

    IBOutlet NSPathControl *_EFIPathControl;
    IBOutlet NSComboBox  *_cloverThemeComboBox;
    IBOutlet NSTextField *_themeWarning;
  //----------------------
  
  IBOutlet NSMutableDictionary *themeInfo;
  IBOutlet NSMutableDictionary* nvram;
  IBOutlet NSNumber* cloverLogEveryBootEnabled;
  IBOutlet NSNumber* cloverLogEveryBootLimit;
  IBOutlet NSDictionary* diskutilList;
  IBOutlet NSArray* efiPartitions;
  IBOutlet NSArray* nvRamPartitions;
}

@property IBOutlet NSMutableDictionary *themeInfo;

@property IBOutlet NSMutableDictionary* nvram;

@property IBOutlet NSNumber* cloverLogEveryBootEnabled;
@property IBOutlet NSNumber* cloverLogEveryBootLimit;

@property (nonatomic,readonly,copy) IBOutlet NSDictionary* diskutilList;
@property (nonatomic,readonly,copy) IBOutlet NSArray* efiPartitions;
@property (nonatomic,readonly,copy) IBOutlet NSArray* nvRamPartitions;


- (id)        initWithBundle:(NSBundle *)bundle;
- (void)      mainViewDidLoad;
- (IBAction)  checkNow:(id)sender;
- (void)      initNVRamVariableFields; //:(id)sender;
- (IBAction)  configureAutomaticUpdates:(id)sender;
- (IBAction)  showPathOpenPanel:(id)sender;
- (IBAction)  themeComboBox:(NSComboBox*)sender;
- (void)      initThemeTab:(NSString*)efiDir;
- (NSString*) getCloverNVRam:(const NSString*)key;
- (NSString*) getNVRamKey:(const NSString*)key;
- (OSErr)     setNVRamKey:(const NSString*)key Value:(NSString*)value;
- (void)      updateThemeTab:(NSString*) themeName;
- (IBAction)  updateCloverLogEveryBoot:(id)sender;
- (IBAction)  updateCloverNVRamVariables:(id)sender;

- (BOOL)isUnlocked;

- (NSDictionary*) getPartitionProperties:(NSString*)bsdName;
- (unsigned int) getUIntPreferenceKey:(CFStringRef)key
                             forAppID:(CFStringRef)appID
                          withDefault:(unsigned int)defaultValue;
- (void) setPreferenceKey:(CFStringRef)key
                 forAppID:(CFStringRef)appID
                  fromInt:(int)value;
- (NSString *)getStringPreferenceKey:(CFStringRef)key
                            forAppID:(CFStringRef)appID
                         withDefault:(CFStringRef)defaultValue;
- (void) setPreferenceKey:(CFStringRef)key
                 forAppID:(CFStringRef)appID
                fromString:(CFStringRef)value;

@end
