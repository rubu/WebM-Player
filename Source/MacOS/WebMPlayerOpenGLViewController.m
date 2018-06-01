#import "WebMPlayerOpenGLViewController.h"
#import "WebMPlayerOpenGLView.h"

@implementation WebMPlayerOpenGLViewController

-(NSSize)windowWillResize:(NSWindow *)sender toSize:(NSSize)frameSize
{
    [(WebMPlayerOpenGLView*)self.view resizeRenderArea:frameSize];
    return frameSize;
}

@end
