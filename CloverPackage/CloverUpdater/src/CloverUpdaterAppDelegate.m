//
//  CloverUpdaterAppDelegate.m
//  CloverUpdater
//
//  Created by Slice on 29.04.13.
//  Modified by JrCs
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
        [updateView setHidden:YES];
        [noUpdateView setHidden:NO];
    } else {
        [NewRevision setTextColor:[NSColor blueColor]];
        [updateView setHidden:NO];
        [noUpdateView setHidden:YES];
    }
}

- (IBAction)DontUpdate:(id)sender {
    printf("0");
    exit(0);
}

- (IBAction)Update:(id)sender {
    printf("1");
    exit(0);
}

@end
