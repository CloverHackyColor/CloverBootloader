//
//  CloverUpdaterAppDelegate.h
//  CloverUpdater
//
//  Created by Sergey on 29.04.13.
//  Copyright 2013 Paritet. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Arguments.h"

@interface CloverUpdaterAppDelegate : NSObject <NSApplicationDelegate> {
    NSWindow *window;
  IBOutlet NSTextField *OldRevision;
  
  IBOutlet NSTextField *NewRevision;
  
}

@property (assign) IBOutlet NSWindow *window;
- (IBAction)NeverButton:(id)sender;
- (IBAction)NotNow:(id)sender;
- (IBAction)OKButton:(id)sender;

@end
