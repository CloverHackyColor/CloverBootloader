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
    io_registry_entry_t _ioRegEntryRef;
    NSString* agentPlistPath;
    NSDictionary *_diskutilList;
    NSArray *_efiPartitions;
    NSString *_cloverMountEfiPartition;
    NSArray *_nvRamPartitions;
    NSString *_cloverNvRamDisk;

    IBOutlet NSPopUpButton *popUpCheckInterval;
    IBOutlet NSTextField *LastRunDate;
    IBOutlet NSButton *checkNowButton;
    IBOutlet SFAuthorizationView *authView;
    IBOutlet NSTextField *lastBootedRevision;
    IBOutlet NSTextField *lastInstalledRevision;

    IBOutlet NSTextField *_logLineCountTextField;
    IBOutlet NSTextField *_logEveryBootTextField;

    IBOutlet NSPathControl *_EFIPathControl;
    IBOutlet NSComboBox *_cloverThemeComboBox;
    IBOutlet NSImageView *_themePreview;
    IBOutlet NSTextField *_noPreviewLabel;
    IBOutlet NSTextField *_themeAuthor;
    IBOutlet NSTextField *_themeYear;
    IBOutlet NSTextField *_themeDescription;
    IBOutlet NSTextField *_themeWarning;
}

@property (readonly) IBOutlet NSDictionary* diskutilList;
@property (readonly) IBOutlet NSArray* efiPartitions;
@property (nonatomic, copy) IBOutlet NSString* cloverMountEfiPartition;
@property (readonly) IBOutlet NSArray* nvRamPartitions;
@property (nonatomic, copy) IBOutlet NSString* cloverNvRamDisk;


- (id)        initWithBundle:(NSBundle *)bundle;
- (void)      mainViewDidLoad;
- (IBAction)  checkNow:(id)sender;
- (void)      initNVRamVariableFields; //:(id)sender;
- (IBAction)  configureAutomaticUpdates:(id)sender;
- (IBAction)  simpleNvramVariableChanged:(id)sender;
- (IBAction)  showPathOpenPanel:(id)sender;
- (IBAction)  themeComboBox:(NSComboBox*)sender;
- (void)      initThemeTab:(NSString*)efiDir;
- (NSString*) getNVRamKey:(const char *)key;
- (OSErr)     setNVRamKey:(NSString*)key Value:(NSString*)value;
- (void)      updateThemeTab:(NSString*) themeName;

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
