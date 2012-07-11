//
//  AppDelegate.m
//  RiackExample
//
//  Created by Kaspar Pedersen on 06/07/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "AppDelegate.h"
#include "riack.h"

@implementation AppDelegate

@synthesize bucketListView = _bucketListView;
@synthesize bucketsArrayController = _bucketsArrayController;
@synthesize window = _window;
@synthesize host = _host;
@synthesize port = _port;
@synthesize mainTabView = _mainTabView;
@synthesize labelConnectionState = _labelConnectionState;
@synthesize labelServerName = _labelServerName;
@synthesize labelServerVersion = _labelServerVersion;

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    // Insert code here to initialize your application
    riack_init();
    client = riack_new_client(0);
}


- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
{
    if (client) {
        riack_free(client);
    }
    riack_cleanup();
    return NSTerminateNow;
}

-(void) updateServerInfo {
    RIACK_STRING node, version;
    if (riack_server_info(client, &node, &version) == RIACK_SUCCESS) {
        if (node.len > 0) {
            NSString *serverName = [[NSString alloc] initWithBytes:node.value length:node.len encoding:NSASCIIStringEncoding];
            [self.labelServerName setStringValue:serverName];
        } else {
            [self.labelServerName setStringValue:@"Unknown"];
        }
        if (version.len > 0) {
            NSString *serverVersion = [[NSString alloc] initWithBytes:version.value length:version.len encoding:NSASCIIStringEncoding];
            [self.labelServerVersion setStringValue:serverVersion];
        } else {
            [self.labelServerVersion setStringValue:@"Unknown"];
        }
        riack_free_string(client, &node);
        riack_free_string(client, &version);
    }
}

-(void) updateBucketList {
    RIACK_STRING_LIST bucketList;
    [[self.bucketsArrayController content] removeAllObjects];
    if (riack_list_buckets(client, &bucketList) == RIACK_SUCCESS) {
        for (size_t i=0; i<bucketList.string_count; ++i) {
            NSString *currentBucket = [[NSString alloc] initWithBytes:bucketList.strings[i].value 
                                                               length:bucketList.strings[i].len 
                                                             encoding:NSASCIIStringEncoding];
            [self.bucketsArrayController addObject:currentBucket];
        }
        riack_free_string_list(client, &bucketList);
        [self.bucketListView reloadData];
    }
}

- (IBAction)connectClicked:(id)sender {
    NSString *host = [self.host stringValue];
    const char* szHost = [host cStringUsingEncoding:NSASCIIStringEncoding];
    if (riack_connect(client, szHost, [self.port intValue]) == RIACK_SUCCESS) {
        self.labelConnectionState.stringValue = @"Connected";
        [self updateServerInfo];
    } else {
        self.labelConnectionState.stringValue = @"Disconnected";
    }
}

/// NSTabViewDelegate

- (void)tabView:(NSTabView *)tabView didSelectTabViewItem:(NSTabViewItem *)tabViewItem {
    if ([tabViewItem.label compare:@"Buckets"] == NSOrderedSame) {
        [self updateBucketList];
    }
}

@end
