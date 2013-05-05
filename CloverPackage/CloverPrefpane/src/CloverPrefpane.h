//
//  CloverPrefpane.h
//  CloverPrefpane
//
//  Created by JrCs on 03/05/13.
//  Copyright (c) 2013 ProjectOSX. All rights reserved.
//

#import <PreferencePanes/PreferencePanes.h>
#import <ServiceManagement/ServiceManagement.h>

@interface CloverPrefpane : NSPreferencePane {
    CFStringRef bundleID;
    NSFileManager *fileManager;
    NSString* plistPath;
    IBOutlet NSPopUpButton *popUpCheckInterval;
    IBOutlet NSTextField *LastRunDate;
    IBOutlet NSDateFormatter *dateFormatter;
    IBOutlet NSButton *checkNowButton;
}

@property (retain) NSFileManager *fileManager;
@property (retain) NSString* plistPath;

- (id) initWithBundle:(NSBundle *)bundle;
- (void) mainViewDidLoad;
- (IBAction) checkNow:(id)sender;
- (IBAction) configureAutomaticUpdates:(id)sender;

- (unsigned int) getUIntPreferenceKey:(CFStringRef)key
                             forAppID:(CFStringRef)appID
                          withDefault:(unsigned int)defaultValue;
- (void) setPreferenceKey:(CFStringRef)key
                 forAppID:(CFStringRef)appID
                  fromInt:(int)value;
@end
