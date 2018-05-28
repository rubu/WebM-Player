#import "WebMPlayerOpenGLView.h"
#include "../OpenGLRenderer.h"

#define kFailedToLoadShader @"failed to load OpenGL shader source from bundled resources"
#define kFailedToInitializeOpenGL @"failed to initialize OpenGL renderer"

CVReturn DisplayLinkOutputCallback(CVDisplayLinkRef displayLink,
                                        const CVTimeStamp *inNow,
                                        const CVTimeStamp *inOutputTime,
                                        CVOptionFlags flagsIn,
                                        CVOptionFlags *flagsOut,
                                        void *displayLinkContext)
{
    WebMPlayerOpenGLView* view = (__bridge WebMPlayerOpenGLView*)(displayLinkContext);
    [view renderFrameForTime:inOutputTime];
    return kCVReturnSuccess;
}

class OpenGLRendererDelegate : public IAbstractView, public IOpenGLContext
{
public:
    OpenGLRendererDelegate(WebMPlayerOpenGLView* view) : view_(view)
    {
    
    }

    // IAbstractView
    void resize(unsigned int width, unsigned int height) override
    {
        dispatch_async(dispatch_get_main_queue(), ^
                       {
                           NSRect windowFrame = view_.window.frame;
                           windowFrame.size.width = width;
                           windowFrame.size.height = height;
                           [view_.window setFrame:windowFrame display:YES];
                       });
    }
    
    // public IOpenGLContext
    void lock() override
    {
        NSOpenGLContext* openGLContext = [view_ openGLContext];
        [openGLContext makeCurrentContext];
    }
    
    void unlock() override
    {
        [NSOpenGLContext clearCurrentContext];
    }

private:
    WebMPlayerOpenGLView* const view_;
};

@implementation WebMPlayerOpenGLView
{
    CVDisplayLinkRef _displayLink;
    std::unique_ptr<OpenGLRendererDelegate> _delegate;
    std::unique_ptr<OpenGLRenderer> _renderer;
    std::unique_ptr<Player> _player;
}

- (void)awakeFromNib
{
    [super awakeFromNib];
    NSString* i420FragmentShaderPath = [[NSBundle mainBundle] pathForResource:@"I420toRGBA" ofType:@"frag"];
    const GLchar* i420FragmentShaderSource = (GLchar *)[[NSString stringWithContentsOfFile:i420FragmentShaderPath encoding:NSASCIIStringEncoding error:nil] cStringUsingEncoding:NSASCIIStringEncoding];
    if (i420FragmentShaderSource == nil)
    {
        [NSException raise:kFailedToLoadShader format:@"failed to read shader file %@", i420FragmentShaderPath];
    }
    NSString* defaultVertexShaderPath = [[NSBundle mainBundle] pathForResource:@"default" ofType:@"vert"];
    const GLchar* defaultVertexShaderSource = (GLchar *)[[NSString stringWithContentsOfFile:defaultVertexShaderPath encoding:NSASCIIStringEncoding error:nil] cStringUsingEncoding:NSASCIIStringEncoding];
    if (defaultVertexShaderSource == nil)
    {
        [NSException raise:kFailedToLoadShader format:@"failed to read shader file %@", defaultVertexShaderPath];
    }
    NSOpenGLContext* openGLContext = [self openGLContext];
    [openGLContext makeCurrentContext];
    try
    {
        _delegate = std::make_unique<OpenGLRendererDelegate>(self);
        _renderer = std::make_unique<OpenGLRenderer>(*_delegate.get(), *_delegate.get(), i420FragmentShaderSource, defaultVertexShaderSource);
    }
    catch (const std::exception& exception)
    {
        [NSException raise:kFailedToInitializeOpenGL format:@"%s", exception.what()];
    }
    [NSOpenGLContext clearCurrentContext];
    CVReturn result = CVDisplayLinkCreateWithCGDisplay(CGMainDisplayID(), &_displayLink);
    if(result == kCVReturnSuccess)
    {
        result = CVDisplayLinkSetOutputCallback(_displayLink, DisplayLinkOutputCallback, (__bridge void*)self);
        if (result == kCVReturnSuccess)
        {
            CVDisplayLinkStart(_displayLink);
        }
    }
}

-(void) dealloc
{
    CVDisplayLinkStop(_displayLink);
}

-(void) drawRect:(NSRect)dirtyRect
{
    [[NSColor blackColor] setFill];
    NSRectFill(dirtyRect);
}

-(void) playFile:(NSURL*)fileURL
{
    _player = std::make_unique<Player>(fileURL.path.UTF8String, _renderer.get());
    _player->start();
}

-(void)renderFrameForTime:(const CVTimeStamp*)timestamp
{
    _renderer->render_frame(timestamp->hostTime);
}
@end
