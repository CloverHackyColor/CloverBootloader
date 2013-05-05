//
//  CloverPrefpane.m
//  CloverPrefpane
//
//  Created by JrCs on 03/05/13.
//  Copyright (c) 2013 ProjectOSX. All rights reserved.
//

#import "CloverPrefpane.h"

CFStringRef agentIdentifier=CFSTR("com.projectosx.Clover.Updater");
CFStringRef agentExecutable=CFSTR("/Library/Application Support/Clover/CloverUpdaterUtility");
CFStringRef checkIntervalKey=CFSTR("ScheduledCheckInterval");
CFStringRef lastCheckTimestampKey=CFSTR("LastCheckTimestamp");

@implementation CloverPrefpane

@synthesize fileManager = fileManager;
@synthesize plistPath = plistPath;

// System Preferences calls this method when the pane is initialized.
- (id)initWithBundle:(NSBundle *)bundle {
    if ( ( self = [super initWithBundle:bundle] ) != nil ) {
        // Get the bundle identifier -> com.projectosx.CloverPrefpane
        const char* const appID = [[[NSBundle bundleForClass:[self class]] bundleIdentifier]
                                   cStringUsingEncoding:NSASCIIStringEncoding];
        bundleID = CFStringCreateWithCString(kCFAllocatorDefault, appID, kCFStringEncodingASCII); // Need a CFString
        
        self.fileManager = [NSFileManager defaultManager];
        NSArray *searchPaths = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES);
        NSString *agentsFolder = [[searchPaths objectAtIndex:0] stringByAppendingPathComponent:@"LaunchAgents"];
        [fileManager createDirectoryAtPath:agentsFolder withIntermediateDirectories:YES attributes:nil error:nil];
        self.plistPath = [[agentsFolder stringByAppendingPathComponent:(NSString *)agentIdentifier] stringByAppendingPathExtension:@"plist"];
    }
    return self;
}

- (void)mainViewDidLoad {
    // Initialize popUpCheckInterval
    unsigned int checkInterval =
        [self getUIntPreferenceKey:checkIntervalKey forAppID:agentIdentifier withDefault:0];
    [popUpCheckInterval selectItemWithTag:checkInterval];
    
    // Initialize LastRunDate
    unsigned int lastCheckTimestamp =
        [self getUIntPreferenceKey:lastCheckTimestampKey forAppID:agentIdentifier withDefault:0];
    if (lastCheckTimestamp == 0) {
        [LastRunDate setStringValue:(NSString*)CFSTR("-")];
    } else {
        NSDate *date = [NSDate dateWithTimeIntervalSince1970:lastCheckTimestamp];
        NSString *formattedDateString = [dateFormatter stringFromDate:date];
        [LastRunDate setStringValue:formattedDateString];
    }
    // Enable the checkNowButton if executable is present
    [checkNowButton setEnabled:[fileManager fileExistsAtPath:(NSString*)agentExecutable]];
    
    BOOL plistExists = [fileManager fileExistsAtPath:plistPath];
    if (plistExists && checkInterval == 0) {
        [fileManager removeItemAtPath:plistPath error:nil];
    }
}

- (IBAction) configureAutomaticUpdates:(id)sender {
    CFDictionaryRef launchInfo = SMJobCopyDictionary(kSMDomainUserLaunchd, agentIdentifier);
    if (launchInfo != NULL) {
        CFRelease(launchInfo);
        CFErrorRef error = NULL;
        if (!SMJobRemove(kSMDomainUserLaunchd, agentIdentifier, NULL, YES, &error))
            NSLog(@"Error in SMJobRemove: %@", error);
        if (error)
            CFRelease(error);
    }

	NSInteger checkInterval = [sender tag];
	[self setPreferenceKey:checkIntervalKey forAppID:agentIdentifier fromInt:(int)checkInterval];

    if (checkInterval > 0 && [fileManager fileExistsAtPath:(NSString*)agentExecutable]) {
        // Create a new plist
         NSArray* call = [NSArray arrayWithObjects:
                         (NSString *)agentExecutable,
                         @"startup",
                         nil];
        NSMutableDictionary *plist = [NSMutableDictionary dictionaryWithObjectsAndKeys:
                                      (NSString *)agentIdentifier, @"Label",
                                      [NSNumber numberWithInteger:checkInterval], @"StartInterval",
                                      [NSNumber numberWithBool:YES], @"RunAtLoad",
                                      (NSString *)agentExecutable, @"Program",
                                      call, @"ProgramArguments",
                                      nil];
        [plist writeToFile:plistPath atomically:YES];

		CFErrorRef error = NULL;
		if (!SMJobSubmit(kSMDomainUserLaunchd, (CFDictionaryRef)plist, NULL, &error)) {
			if (error) {
				NSLog(@"Error in SMJobSubmit: %@", error);
			} else
				NSLog(@"Error in SMJobSubmit without details. Check /var/db/launchd.db/com.apple.launchd.peruser.NNN/overrides.plist for %@ set to disabled.", agentIdentifier);
		}
		if (error)
			CFRelease(error);
    } else {
        // Remove the plist
        [fileManager removeItemAtPath:plistPath error:nil];
    }
    CFPreferencesAppSynchronize(agentIdentifier); // Force the preferences to be save to disk
}

- (IBAction)checkNow:(id)sender {
    [[NSWorkspace sharedWorkspace] launchApplication:(NSString*)agentExecutable];
}

// get and set preference keys functions idea taken from:
// http://svn.perian.org/branches/perian-1.1/CPFPerianPrefPaneController.m
- (unsigned int)getUIntPreferenceKey:(CFStringRef)key
                            forAppID:(CFStringRef)appID
                         withDefault:(unsigned int)defaultValue
{
	CFPropertyListRef value;
	unsigned int ret = defaultValue;
	
	value = CFPreferencesCopyAppValue(key, appID);
	if (value && CFGetTypeID(value) == CFNumberGetTypeID())
		CFNumberGetValue(value, kCFNumberIntType, &ret);
	
	if (value)
		CFRelease(value);
	
	return ret;
}

- (void)setPreferenceKey:(CFStringRef)key
                forAppID:(CFStringRef)appID
                 fromInt:(int)value
{
	CFNumberRef numRef = CFNumberCreate(NULL, kCFNumberIntType, &value);
	CFPreferencesSetAppValue(key, numRef, appID);
	CFRelease(numRef);
}

@end