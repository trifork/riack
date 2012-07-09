//
//  AppDelegate.h
//  RiackExample
//
//  Created by Kaspar Pedersen on 06/07/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface AppDelegate : NSObject <NSApplicationDelegate> {
    struct RIACK_CLIENT *client;
}

@property (assign) IBOutlet NSWindow *window;

@property (readonly, strong, nonatomic) NSPersistentStoreCoordinator *persistentStoreCoordinator;
@property (readonly, strong, nonatomic) NSManagedObjectModel *managedObjectModel;
@property (readonly, strong, nonatomic) NSManagedObjectContext *managedObjectContext;
@property (weak) IBOutlet NSTextField *host;
@property (weak) IBOutlet NSTextField *port;
@property (weak) IBOutlet NSTabView *mainTabView;
@property (weak) IBOutlet NSTextField *labelConnectionState;
@property (weak) IBOutlet NSTextField *labelServerName;
@property (weak) IBOutlet NSTextField *labelServerVersion;

- (IBAction)saveAction:(id)sender;

@end
