//
//  CloverUpdaterAppDelegate.m
//  CloverUpdater
//
//  Created by Sergey on 29.04.13.
//  Copyright 2013 Paritet. All rights reserved.
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
}

- (IBAction)NeverButton:(id)sender {
    printf("-1");
    exit(0);
}

- (IBAction)NotNow:(id)sender {
    printf("0");
    exit(0);
}

- (IBAction)UpdateButton:(id)sender {
    printf("1");
    exit(0);
}

@end
