//
//  CloverPrefpane.m
//  CloverPrefpane
//
//  Created by JrCs on 03/05/13.
//  Copyright (c) 2013 ProjectOSX. All rights reserved.
//

#import "CloverPrefpane.h"
#include <mach/mach_error.h>

#define kCloverInstaller @"com.projectosx.clover.installer"

// Global Variables
static const CFStringRef agentIdentifier=CFSTR("com.projectosx.Clover.Updater");
static const CFStringRef agentExecutable=CFSTR("/Library/Application Support/Clover/CloverUpdaterUtility");
static const CFStringRef checkIntervalKey=CFSTR("ScheduledCheckInterval");
static const CFStringRef lastCheckTimestampKey=CFSTR("LastCheckTimestamp");

@implementation CloverPrefpane

// System Preferences calls this method when the pane is initialized.
- (id)initWithBundle:(NSBundle *)bundle {
    if ( ( self = [super initWithBundle:bundle] ) != nil ) {
        // Get a ressource from bundle
        // NSString* path = [self.bundle pathForResource:@"CloverPrefpane" ofType:@"tiff"];
        
        NSArray *searchPaths = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES);
        NSString *agentsFolder = [[searchPaths objectAtIndex:0] stringByAppendingPathComponent:@"LaunchAgents"];
        [[NSFileManager defaultManager] createDirectoryAtPath:agentsFolder withIntermediateDirectories:YES attributes:nil error:nil];
        agentPlistPath = [[agentsFolder stringByAppendingPathComponent:(NSString *)agentIdentifier] stringByAppendingPathExtension:@"plist"];

        // Allocate object for accessing IORegistry
        mach_port_t   masterPort;
        kern_return_t result = IOMasterPort(bootstrap_port, &masterPort);
        if (result != KERN_SUCCESS) {
            NSLog(@"Error getting the IOMaster port: %s", mach_error_string(result));
            exit(1);
        }

        _ioRegEntryRef = IORegistryEntryFromPath(masterPort, "IODeviceTree:/options");
        if (_ioRegEntryRef == 0) {
            NSLog(@"nvram is not supported on this system");
        }
    }
    return self;
}

- (void)mainViewDidLoad {
    BOOL plistExists;

    // Initialize revision fields
    NSArray *searchPaths = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSLocalDomainMask, YES);
    NSString *preferenceFolder = [[searchPaths objectAtIndex:0] stringByAppendingPathComponent:@"Preferences"];
    NSString *cloverInstallerPlist = [[preferenceFolder stringByAppendingPathComponent:kCloverInstaller] stringByAppendingPathExtension:@"plist"];
    plistExists = [[NSFileManager defaultManager] fileExistsAtPath:cloverInstallerPlist];
    NSString* installedRevision = @"-";
    if (plistExists) {
        NSDictionary *dict = [[NSDictionary alloc]
                              initWithContentsOfFile:cloverInstallerPlist];
        NSNumber* revision = [dict objectForKey:@"CloverRevision"];
        [dict release];
        if (revision)
            installedRevision = [revision stringValue];
    }
    [lastInstalledRevision setStringValue:installedRevision];

    NSString* bootedRevision = @"-";
    io_registry_entry_t ioRegistryEFI = IORegistryEntryFromPath(kIOMasterPortDefault, "IODeviceTree:/efi/platform");
    if (ioRegistryEFI) {
        CFStringRef nameRef = CFStringCreateWithCString(kCFAllocatorDefault, "clovergui-revision",
                                                        kCFStringEncodingUTF8);
        if (nameRef) {
            CFTypeRef valueRef = IORegistryEntryCreateCFProperty(ioRegistryEFI, nameRef, 0, 0);
            CFRelease(nameRef);
            if (valueRef) {
                // Get the OF variable's type.
                CFTypeID typeID = CFGetTypeID(valueRef);
                if (typeID == CFDataGetTypeID())
                    bootedRevision = [NSString stringWithFormat:@"%u",*((uint32_t*)CFDataGetBytePtr(valueRef))];
                CFRelease(valueRef);
            }
        }
        IOObjectRelease(ioRegistryEFI);
    }
    [lastBootedRevision setStringValue:bootedRevision];

    // Initialize popUpCheckInterval
    unsigned int checkInterval =
        [self getUIntPreferenceKey:checkIntervalKey forAppID:agentIdentifier withDefault:0];
    [popUpCheckInterval selectItemWithTag:checkInterval];
    
    // Initialize LastRunDate
    unsigned int lastCheckTimestamp =
        [self getUIntPreferenceKey:lastCheckTimestampKey forAppID:agentIdentifier withDefault:0];
    if (lastCheckTimestamp == 0) {
        [LastRunDate setStringValue:@"-"];
    } else {
        NSDate *date = [NSDate dateWithTimeIntervalSince1970:lastCheckTimestamp];
        [LastRunDate setStringValue:[LastRunDate.formatter stringFromDate:date]];

    }
    // Enable the checkNowButton if executable is present
    [checkNowButton setEnabled:[[NSFileManager defaultManager] fileExistsAtPath:(NSString*)agentExecutable]];
    
    // Init NVRam variables fields
    [self initNVRamVariableFields];

    // Setup security.
	AuthorizationItem items = {kAuthorizationRightExecute, 0, NULL, 0};
	AuthorizationRights rights = {1, &items};
	[authView setAuthorizationRights:&rights];
	authView.delegate = self;
	[authView updateStatus:nil];

    plistExists = [[NSFileManager defaultManager] fileExistsAtPath:agentPlistPath];
    if (plistExists && checkInterval == 0) {
        [[NSFileManager defaultManager] removeItemAtPath:agentPlistPath error:nil];
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

    if (checkInterval > 0 && [[NSFileManager defaultManager] fileExistsAtPath:(NSString*)agentExecutable]) {
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
        [plist writeToFile:agentPlistPath atomically:YES];

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
        [[NSFileManager defaultManager] removeItemAtPath:agentPlistPath error:nil];
    }
    CFPreferencesAppSynchronize(agentIdentifier); // Force the preferences to be save to disk
}

- (IBAction)checkNow:(id)sender {
    [[NSWorkspace sharedWorkspace] launchApplication:(NSString*)agentExecutable];
}

- (IBAction)simpleNvramVariableChanged:(id)sender {
    NSString *oldValue = [self getNVRamKey:[[sender identifier] UTF8String]];
    NSString *newValue = [sender stringValue];
    if ((oldValue.length != 0 || newValue.length != 0) &&
        ![oldValue isEqualToString:newValue]) {
        [self setNVRamKey:[[sender identifier] UTF8String] Value:[newValue UTF8String]];
    }
}

- (BOOL)isUnlocked {
	return [authView authorizationState] == SFAuthorizationViewUnlockedState;
}

//
// SFAuthorization delegates
//
- (void)authorizationViewDidAuthorize:(SFAuthorizationView *)view {
}

- (void)authorizationViewDidDeauthorize:(SFAuthorizationView *)view {
}


-(void) initNVRamVariableFields {
    NSString *value;
    value = [self getNVRamKey:[[_themeTextField identifier] UTF8String]];
    [_themeTextField setStringValue:value];

    value = [self getNVRamKey:[[_logLineCountTextField identifier] UTF8String]];
    [_logLineCountTextField setStringValue:value];

    value = [self getNVRamKey:[[_logEveryBootTextField identifier] UTF8String]];
    [_logEveryBootTextField setStringValue:value];

    value = [self getNVRamKey:[[_mountEFITextField identifier] UTF8String]];
    [_mountEFITextField setStringValue:value];

    value = [self getNVRamKey:[[_nvRamDiskTextField identifier] UTF8String]];
    [_nvRamDiskTextField setStringValue:value];
}

//
// NVRAM methods
//

// Get NVRAM value
-(NSString*) getNVRamKey:(const char *)key {
    NSString*   result = @"";

    CFStringRef nameRef = CFStringCreateWithCString(kCFAllocatorDefault, key,
                                                    kCFStringEncodingUTF8);
    if (nameRef == 0) {
        NSLog(@"Error creating CFString for key %s", key);
        return result;
    }

    CFTypeRef valueRef = IORegistryEntryCreateCFProperty(_ioRegEntryRef, nameRef, 0, 0);
    CFRelease(nameRef);
    if (valueRef == 0) return result;

    // Get the OF variable's type.
    CFTypeID typeID = CFGetTypeID(valueRef);

    if (typeID == CFDataGetTypeID())
        result = [NSString stringWithUTF8String:(const char*)CFDataGetBytePtr(valueRef)];

    CFRelease(valueRef);

    return result;
}

// Set NVRAM key/value pair
-(OSErr) setNVRamKey:(const char *)key Value:(const char *)value {

    OSErr processError = 0;

    if (key) {
        if (!value)
            value="";
        // Size for key=value + null terminal char
        size_t len=strlen(key) + 1 + strlen(value) + 1;
        char* nvram_arg=(char*) malloc(sizeof(char) * len);
        snprintf(nvram_arg, len, "%s=%s", key, value);

        // Need 2 parameters: key=value and NULL
        const char **argv = (const char **)malloc(sizeof(char *) * 2);
        argv[0] = nvram_arg;
        argv[1] = NULL;

        processError = AuthorizationExecuteWithPrivileges([[authView authorization] authorizationRef], [@"/usr/sbin/nvram" UTF8String],
                                                          kAuthorizationFlagDefaults, (char *const *)argv, nil);

        if (processError != errAuthorizationSuccess)
            NSLog(@"Error trying to set nvram %s:%d", nvram_arg, processError);

        free(argv);
        free(nvram_arg);
    }
    return processError;
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