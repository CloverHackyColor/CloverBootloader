//
//  CloverUpdaterAppDelegate.h
//  CloverUpdater
//
//  Created by Slice on 29.04.13.
//  Modified by JrCs
//

#import <Cocoa/Cocoa.h>
#import "Arguments.h"

@interface CloverUpdaterAppDelegate : NSObject <NSApplicationDelegate> {
    IBOutlet NSView *updateView;
    IBOutlet NSView *noUpdateView;

    IBOutlet NSTextField *OldRevision;
    IBOutlet NSTextField *NewRevision;
}

- (IBAction)DontUpdate:(id)sender;
- (IBAction)Update:(id)sender;

@end
