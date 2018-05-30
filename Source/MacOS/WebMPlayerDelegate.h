#pragma once

#import <Cocoa/Cocoa.h>

@interface WebMPlayerDelegate : NSObject <NSApplicationDelegate>

-(IBAction)openDocument:(id)sender;
-(BOOL)application:(NSApplication *)sender openFile:(NSString*)filename;

@end

