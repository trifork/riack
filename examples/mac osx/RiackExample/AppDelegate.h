//
//  AppDelegate.h
//  RiackExample
//
//  Created by Kaspar Pedersen on 06/07/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface AppDelegate : NSObject <NSApplicationDelegate, NSTabViewDelegate> {
    struct RIACK_CLIENT *client;
    
}

@property (assign) IBOutlet NSWindow *window;

@property (weak) IBOutlet NSTextField *host;
@property (weak) IBOutlet NSTextField *port;
@property (weak) IBOutlet NSTabView *mainTabView;
@property (weak) IBOutlet NSTextField *labelConnectionState;
@property (weak) IBOutlet NSTextField *labelServerName;
@property (weak) IBOutlet NSTextField *labelServerVersion;
@property (weak) IBOutlet NSTableView *bucketListView;
@property (weak) IBOutlet NSArrayController *bucketsArrayController;

@property (weak) IBOutlet NSTextField *bucketSet;
@property (weak) IBOutlet NSTextField *keySet;
@property (weak) IBOutlet NSComboBox *contentTypeSet;
@property (unsafe_unretained) IBOutlet NSTextView *valueSet;
- (IBAction)setTouched:(id)sender;
@property (weak) IBOutlet NSLevelIndicatorCell *progressSet;

@property (weak) IBOutlet NSTextField *bucketGet;
@property (weak) IBOutlet NSTextField *keyGet;
- (IBAction)getTouched:(id)sender;
@property (weak) IBOutlet NSTextField *vtagGet;
@property (weak) IBOutlet NSTextField *contentTypeGet;
@property (unsafe_unretained) IBOutlet NSTextView *valueGet;

@end
