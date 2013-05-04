//
//  CloverUpdaterAppDelegate.m
//  CloverUpdater
//
//  Created by Slice on 29.04.13.
//

#import "CloverUpdaterAppDelegate.h"

@implementation CloverUpdaterAppDelegate

@synthesize window;

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
	// Insert code here to initialize your application 
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)theApplication {
    return YES;
}

- (void) awakeFromNib {
    [OldRevision setStringValue: [NSString stringWithFormat: @"%s", arg1]];
    [NewRevision setStringValue: [NSString stringWithFormat: @"%s", arg2]];
    if ([OldRevision intValue] >= [NewRevision intValue]) {
        [updateButton setEnabled:FALSE];
    }
}

- (IBAction)NotNow:(id)sender {
    printf("0");
    exit(0);
}

- (IBAction)Update:(id)sender {
    printf("1");
    exit(0);
}

@end
