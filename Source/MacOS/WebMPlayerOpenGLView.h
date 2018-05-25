#pragma once

#import <Cocoa/Cocoa.h>

@interface WebMPlayerOpenGLView : NSOpenGLView

-(void)playFile:(NSURL*)fileURL;
-(void)renderFrameForTime:(const CVTimeStamp*)timestamp;

@end
