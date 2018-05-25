#pragma once

#import <Cocoa/Cocoa.h>

@interface WebMPlayerOpenGLView : NSOpenGLView

-(void) playFile:(NSURL*)fileURL;

@end
