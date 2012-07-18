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
@synthesize vtagGet = _vtagGet;
@synthesize contentTypeGet = _contentTypeGet;
@synthesize valueGet = _valueGet;

@synthesize bucketListView = _bucketListView;
@synthesize bucketsArrayController = _bucketsArrayController;
@synthesize bucketSet = _bucketSet;
@synthesize keySet = _keySet;
@synthesize contentTypeSet = _contentTypeSet;
@synthesize valueSet = _valueSet;
@synthesize progressSet = _progressSet;
@synthesize bucketGet = _bucketGet;
@synthesize keyGet = _keyGet;
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
    if (riack_connect(client, szHost, [self.port intValue], 0) == RIACK_SUCCESS) {
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

// Get set

-(RIACK_STRING)nsstringToRiackString:(NSString*)str {
    RIACK_STRING result;
    result.len = [str lengthOfBytesUsingEncoding:NSUTF8StringEncoding];
    result.value = (char*)[str cStringUsingEncoding:NSUTF8StringEncoding];
    return result;
}

-(NSString*)riackStringToNSString:(RIACK_STRING)str {
    return [[NSString alloc] initWithBytes:str.value length:str.len encoding:NSUTF8StringEncoding];
}

- (IBAction)setTouched:(id)sender {
    struct RIACK_OBJECT object, *result_object;
    struct RIACK_CONTENT content;
    result_object = NULL;
    NSString *bucket = [self.bucketSet stringValue];
    NSString *key = [self.keySet stringValue];
    NSString *contentType = [self.contentTypeSet stringValue];
    NSString *value = [self.valueSet string];
    if ([bucket length] > 0 && [key length] > 0 && [contentType length] > 0 && [value length] > 0) {
        memset(&object, 0, sizeof(object));
        memset(&content, 0, sizeof(content));
        object.bucket = [self nsstringToRiackString:bucket];
        object.key = [self nsstringToRiackString:key];
        object.content_count = 1;
        object.content = &content;
        object.content[0].content_type = [self nsstringToRiackString:contentType];
        object.content[0].data_len = [[self.valueSet string] lengthOfBytesUsingEncoding:NSUTF8StringEncoding];
        object.content[0].data = (uint8_t*)[[self.valueSet string] cStringUsingEncoding:NSUTF8StringEncoding];
        [self.progressSet setIntValue:0];
        if (riack_put(client, object, result_object, NULL) == RIACK_SUCCESS) {
            
            [self.progressSet setIntValue:100];            
        } else {
            NSAlert *alert = [NSAlert alertWithMessageText:@"Error"
                                             defaultButton:@"OK" 
                                           alternateButton:nil
                                               otherButton:nil 
                                 informativeTextWithFormat:@"Failed to set value (for some reason)"];
            [alert runModal];
        }

    } else {
        NSAlert *alert = [NSAlert alertWithMessageText:@"Error"
                                         defaultButton:@"OK" 
                                       alternateButton:nil
                                           otherButton:nil 
                             informativeTextWithFormat:@"Missing bucket, key or value"];
        [alert runModal];
    }
}

- (IBAction)getTouched:(id)sender {
    NSString *bucket = [self.bucketGet stringValue];
    NSString *key = [self.keyGet stringValue];
    if ([bucket length] > 0 && [key length] > 0) {
        struct RIACK_GET_OBJECT gotten;
        RIACK_STRING rbucket = [self nsstringToRiackString:bucket];
        RIACK_STRING rkey = [self nsstringToRiackString:key];
        if (riack_get(client, rbucket, rkey, 0, &gotten) == RIACK_SUCCESS) {
            if (gotten.object.content_count == 1) {
                NSString *vtag = [self riackStringToNSString:gotten.object.content[0].vtag];
                [self.vtagGet setStringValue:vtag];
                NSString *contentType = [self riackStringToNSString:gotten.object.content[0].content_type];
                [self.contentTypeGet setStringValue:contentType];
                uint8_t* data = gotten.object.content[0].data;
                size_t datalen = gotten.object.content[0].data_len;
                NSString *value = [[NSString alloc] initWithBytes:data length:datalen encoding:NSUTF8StringEncoding];
                [self.valueGet setString:value];
            } else {
                // TODO
            }
            riack_free_get_object(client, &gotten);
        } else {
            // TODO
        }
    } else {
        NSAlert *alert = [NSAlert alertWithMessageText:@"Error"
                                         defaultButton:@"OK" 
                                       alternateButton:nil
                                           otherButton:nil 
                             informativeTextWithFormat:@"Missing bucket or key value"];
        [alert runModal];
    }
}
@end
