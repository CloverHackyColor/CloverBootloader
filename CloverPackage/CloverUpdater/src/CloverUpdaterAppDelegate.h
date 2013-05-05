//
//  CloverUpdaterAppDelegate.h
//  CloverUpdater
//
//  Created by Slice on 29.04.13.
//

#import <Cocoa/Cocoa.h>
#import "Arguments.h"

@interface CloverUpdaterAppDelegate : NSObject <NSApplicationDelegate> {
    NSWindow *window;
    IBOutlet NSTextField *OldRevision;
    IBOutlet NSTextField *NewRevision;
    IBOutlet NSButton *updateButton;
}

@property (assign) IBOutlet NSWindow *window;
- (IBAction)NotNow:(id)sender;
- (IBAction)Update:(id)sender;

@end
