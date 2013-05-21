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
static const CFStringRef efiDirPathKey=CFSTR("EFI Directory Path");

@implementation CloverPrefpane


// System Preferences calls this method when the pane is initialized.
- (id)initWithBundle:(NSBundle *)bundle {
    if ( ( self = [super initWithBundle:bundle] ) != nil ) {
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
        NSDictionary *dict = [[[NSDictionary alloc]
                              initWithContentsOfFile:cloverInstallerPlist] autorelease];
        NSNumber* revision = [dict objectForKey:@"CloverRevision"];
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

    // Get EFI Path
    NSString* efiDir=[self getStringPreferenceKey:efiDirPathKey
                                         forAppID:(CFStringRef)[self.bundle bundleIdentifier]
                                      withDefault:CFSTR("/")];
    [_EFIPathControl setURL:[NSURL fileURLWithPath:efiDir]];
    [self initThemeTab:efiDir];

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

-(void) initNVRamVariableFields /*:(id)sender*/ {
    NSString *value;

//    value = [self getNVRamKey:[[_logLineCountTextField identifier] UTF8String]];
  value = [self getNVRamKey:"Clover.LogLineCount"];
    [_logLineCountTextField setStringValue:value];

//    value = [self getNVRamKey:[[_logEveryBootTextField identifier] UTF8String]];
  value = [self getNVRamKey:"Clover.LogEveryBoot"];
    [_logEveryBootTextField setStringValue:value];

//    value = [self getNVRamKey:[[_mountEFITextField identifier] UTF8String]];
  value = [self getNVRamKey:"Clover.MountEFI"];
    [_mountEFITextField setStringValue:value];

//    value = [self getNVRamKey:[[_nvRamDiskTextField identifier] UTF8String]];
  value = [self getNVRamKey:"Clover.nvRamDisk"];
    [_nvRamDiskTextField setStringValue:value];
}


#pragma mark -
#pragma mark General Tab Methods
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


- (BOOL)isUnlocked {
	return [authView authorizationState] == SFAuthorizationViewUnlockedState;
}

#pragma mark -
#pragma mark NVRam Tab Methods

- (IBAction)simpleNvramVariableChanged:(id)sender {
    NSString *oldValue = [self getNVRamKey:[[sender identifier] UTF8String]];
    NSString *newValue = [sender stringValue];
    if ((oldValue.length != 0 || newValue.length != 0) &&
        ![oldValue isEqualToString:newValue]) {
        [self setNVRamKey:[[sender identifier] UTF8String] Value:[newValue UTF8String]];
    }
}

#pragma mark -
#pragma mark Theme Tab Methods

- (void) initThemeTab:(NSString*) efiDir {

    NSFileManager *fileManager = [NSFileManager defaultManager];
    BOOL isDir = NO;

    [_cloverThemeComboBox removeAllItems]; // Remove all theme names if exists
    [_themePreview setImage:nil];

    NSString *themesDir = [efiDir stringByAppendingPathComponent:@"CLOVER/Themes"];
    [fileManager fileExistsAtPath:themesDir isDirectory:(&isDir)];
    [_themeWarning setHidden:isDir];

    // get the list of all files and directories
    NSMutableArray *themes = [[[NSMutableArray alloc] init] autorelease];

    NSArray *fileList = [fileManager contentsOfDirectoryAtPath:themesDir error:nil];
    if (fileList) {
        for(NSString *file in fileList) {
            NSString *path = [[themesDir stringByAppendingPathComponent:file]
                               stringByAppendingPathComponent:@"theme.plist"];
            if ([fileManager fileExistsAtPath:path]) {
                [themes addObject:file];
            }
        }
    }
    else {
        // Try to get installed themes
        NSArray *searchPaths = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSLocalDomainMask, YES);
        NSString *preferenceFolder = [[searchPaths objectAtIndex:0] stringByAppendingPathComponent:@"Preferences"];
        NSString *cloverInstallerPlist = [[preferenceFolder stringByAppendingPathComponent:kCloverInstaller] stringByAppendingPathExtension:@"plist"];
        NSDictionary *dict = [[[NSDictionary alloc]
                              initWithContentsOfFile:cloverInstallerPlist] autorelease];
        if (dict) {
            NSArray *installedThemes = [dict objectForKey:@"InstalledThemes"];
            [themes addObjectsFromArray:installedThemes];
        }
        else {
            // Get default themes from bundle
            NSString* defaultThemePlistPath = [self.bundle pathForResource:@"DefaultThemes.plist" ofType:@"plist"];
            NSDictionary *dict = [[[NSDictionary alloc]
                                  initWithContentsOfFile:defaultThemePlistPath] autorelease];
            if (dict) {
                NSArray *defaultThemes = [dict objectForKey:@"Default Themes"];
                if (defaultThemes)
                    [themes addObjectsFromArray:defaultThemes];
            }
        }
    }

    NSString* currentTheme = [self getNVRamKey:"Clover.Theme"];
    if (currentTheme && [themes indexOfObject:currentTheme] == NSNotFound)
        [themes addObject:currentTheme];

    NSArray *sortedThemes = [themes sortedArrayUsingSelector:@selector(localizedCaseInsensitiveCompare:)];
    [_cloverThemeComboBox addItemsWithObjectValues:sortedThemes];
    if (currentTheme) {
        [_cloverThemeComboBox selectItemWithObjectValue:currentTheme];
        [self updateThemeTab:currentTheme];
    }
}

- (void) updateThemeTab:(NSString*) themeName {
    NSString *efiDir    = [[_EFIPathControl URL] path];
    NSString *themeDir  = [[efiDir stringByAppendingPathComponent:@"CLOVER/Themes"]
                           stringByAppendingPathComponent:themeName];
    NSString *imagePath = [themeDir stringByAppendingPathComponent:@"screenshot.png"];
    if (![[NSFileManager defaultManager] fileExistsAtPath:imagePath]) {
        imagePath = [self.bundle pathForResource:@"NoPreview.png" ofType:@"png"];
        [_noPreviewLabel setHidden:NO];
    }
    else
        [_noPreviewLabel setHidden:YES];

    NSImage *image = [[[NSImage alloc] initWithContentsOfFile:imagePath] autorelease];
    [_themePreview setImage:image];

    // Load the theme.plist file
    NSString *themePlistPath = [themeDir stringByAppendingPathComponent:@"theme.plist"];
    NSDictionary *dict = [[[NSDictionary alloc]
                           initWithContentsOfFile:themePlistPath] autorelease];
    NSString* author = [dict objectForKey:@"Author"];
    NSString* Year   = [dict objectForKey:@"Year"];
    NSString* Description = [dict objectForKey:@"Description"];
    [_themeAuthor setStringValue: author ? author : @"Unknown"];
    [_themeYear   setStringValue: Year ? Year : @"Unknown"];
    [_themeDescription setStringValue: Description ? Description : @"No description"];

    NSString *oldValue = [self getNVRamKey:"Clover.Theme"];
    if ((oldValue.length != 0 || themeName.length != 0) &&
        ![oldValue isEqualToString:themeName]) {
        if ([self setNVRamKey:"Clover.Theme" Value:[themeName UTF8String]] != 0) {
            [_cloverThemeComboBox setStringValue:oldValue];
            [self updateThemeTab:oldValue];
        }
    }
}

- (IBAction)showPathOpenPanel:(id)sender {
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    [panel setAllowsMultipleSelection:NO];
    [panel setCanChooseDirectories:YES];
    [panel setCanChooseFiles:NO];
    [panel setResolvesAliases:YES];

    NSString *panelTitle = NSLocalizedString(@"Select the EFI directory", @"Title for the open panel");
    [panel setTitle:panelTitle];

    NSString *promptString = NSLocalizedString(@"Set EFI directory", @"Prompt for the open panel prompt");
    [panel setPrompt:promptString];

    [panel beginSheetModalForWindow:[sender window] completionHandler:^(NSInteger result) {

        // Hide the open panel.
        [panel orderOut:self];

        // If the return code wasn't OK, don't do anything.
        if (result != NSOKButton) {
            return;
        }
        // Get the first URL returned from the Open Panel and set it at the first path component of the control.
        NSURL *url = [[panel URLs] objectAtIndex:0];
        [_EFIPathControl setURL:url];

        NSString *efiDir = [url path];
        [self setPreferenceKey:efiDirPathKey
                      forAppID:(CFStringRef)[self.bundle bundleIdentifier]
                    fromString:(CFStringRef)efiDir];
        CFPreferencesAppSynchronize((CFStringRef)[self.bundle bundleIdentifier]); // Force the preferences to be save to disk

        [self initThemeTab:efiDir];
    }];
}

- (IBAction)themeComboBox:(NSComboBox*)sender {
    NSString *themeName = [sender stringValue];
    [self updateThemeTab:themeName];
}

#pragma mark -
#pragma mark Authorization delegates

//
// SFAuthorization delegates
//
- (void)authorizationViewDidAuthorize:(SFAuthorizationView *)view {
}

- (void)authorizationViewDidDeauthorize:(SFAuthorizationView *)view {
}


//
// NVRAM methods
//
#pragma mark -
#pragma mark NVRam methods

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

//
// Preferences methods
//
#pragma mark -
#pragma mark Preferences methods

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

- (NSString *)getStringPreferenceKey:(CFStringRef)key
                            forAppID:(CFStringRef)appID
                         withDefault:(CFStringRef)defaultValue
{
	CFPropertyListRef value;
	NSString *ret = nil;

	value = CFPreferencesCopyAppValue(key, appID);
	if(value && CFGetTypeID(value) == CFStringGetTypeID())
		ret = [NSString stringWithString:(NSString *)value];
    else
        ret = [NSString stringWithString:(NSString *)defaultValue];

	if(value)
		CFRelease(value);

	return ret;
}

- (void)setPreferenceKey:(CFStringRef)key
                forAppID:(CFStringRef)appID
              fromString:(CFStringRef)value
{
	CFPreferencesSetAppValue(key, value, appID);
}


@end