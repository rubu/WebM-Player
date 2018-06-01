#pragma once

#import <Cocoa/Cocoa.h>

@class EbmlTree;

@interface WebMPLayerEbmlTreeViewController : NSViewController<NSMenuDelegate>

-(BOOL)validateMenuItem:(NSMenuItem *)menuItem;
-(IBAction)show:(id)sender;
-(void)setEbmlTree:(EbmlTree*)ebmlTree;

@end
