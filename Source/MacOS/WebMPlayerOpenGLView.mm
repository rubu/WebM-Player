#import "WebMPlayerOpenGLView.h"
#import "WebMPlayerEbmlTreeViewController.h"
#import "EbmlTree.h"

#include "../OpenGLRenderer.h"

#define kFailedToLoadShader @"failed to load OpenGL shader source from bundled resources"
#define kFailedToInitializeOpenGL @"failed to initialize OpenGL renderer"

@interface WebMPlayerOpenGLView ()

@property (weak) IBOutlet WebMPLayerEbmlTreeViewController *ebmlTreeViewController;

@end

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

class OpenGLContext : public IOpenGLContext
{
public:
    OpenGLContext(WebMPlayerOpenGLView* view) : view_(view)
    {
    }
    
    // IOpenGLContext
    void lock() override
    {
        NSOpenGLContext* openGLContext = [view_ openGLContext];
        [openGLContext makeCurrentContext];
    }
    
    void unlock() override
    {
        [[view_ openGLContext] flushBuffer];
        [NSOpenGLContext clearCurrentContext];
    }

private:
    WebMPlayerOpenGLView* const view_;
};

class PlayerEventListener : public Player::IEventListener
{
public:
    PlayerEventListener(OpenGLRenderer& opengl_renderer, WebMPlayerOpenGLView* view) : opengl_renderer_(opengl_renderer),
        view_(view)
    {
    
    }

    // Player::IEventListener
    void set_timescale(unsigned int timescale_numerator, unsigned int timescale_denominator) override
    {
        opengl_renderer_.set_timescale(timescale_numerator, timescale_denominator);
    }

    bool on_video_frame_size_changed(unsigned int width, unsigned int height) override
    {
        dispatch_async(dispatch_get_main_queue(), ^
                       {
                           NSRect windowFrame = view_.window.frame;
                           windowFrame.size.width = width;
                           windowFrame.size.height = height;
                           [view_.window setFrame:windowFrame display:YES];
                       });
        return opengl_renderer_.on_video_frame_size_changed(width, height);
    }
   
    bool on_i420_video_frame_decoded(unsigned char* yuv_planes[3], size_t strides[3], uint64_t pts /* nanoseconds */) override
    {
        return opengl_renderer_.on_i420_video_frame_decoded(yuv_planes, strides, pts);
    }

    void on_ebml_document_ready(const EbmlDocument& ebml_document) override
    {
        [view_.ebmlTreeViewController setEbmlTree:[[EbmlTree alloc] initWithEbmlDocument:&ebml_document]];
    }

    void on_exception(const std::exception& exception) override
    {

    }

private:
    OpenGLRenderer& opengl_renderer_;
    WebMPlayerOpenGLView* const view_;
};

@implementation WebMPlayerOpenGLView
{
    CVDisplayLinkRef _displayLink;
    std::unique_ptr<OpenGLRenderer> _openglRenderer;
    std::unique_ptr<OpenGLContext> _openglContext;
    std::unique_ptr<PlayerEventListener> _playerEventListener;
    Player _player;
}

-(void)awakeFromNib
{
    [super awakeFromNib];
    NSOpenGLPixelFormatAttribute pixelFormatAttributes[] =
    {
      NSOpenGLPFADoubleBuffer,
      NSOpenGLPFADepthSize, 24,
      0
    };
    NSOpenGLPixelFormat* pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:pixelFormatAttributes];
    NSOpenGLContext* openGLContext = [[NSOpenGLContext alloc] initWithFormat:pixelFormat shareContext:nil];
    [self setPixelFormat:pixelFormat];
    [self setOpenGLContext:openGLContext];
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
    [openGLContext makeCurrentContext];
    try
    {
        _openglContext = std::make_unique<OpenGLContext>(self);
        _openglRenderer = std::make_unique<OpenGLRenderer>(*_openglContext.get(), _player, i420FragmentShaderSource, defaultVertexShaderSource);
    }
    catch (const std::exception& exception)
    {
        [NSException raise:kFailedToInitializeOpenGL format:@"%s", exception.what()];
    }
    _playerEventListener = std::make_unique<PlayerEventListener>(*_openglRenderer.get(), self);
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

-(void)resizeRenderArea:(NSSize)renderAreaSize
{
}

-(void)dealloc
{
    CVDisplayLinkStop(_displayLink);
}

-(void)drawRect:(NSRect)dirtyRect
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

-(void)playFile:(NSURL*)fileURL
{
    _openglRenderer->reset();
    _player.start(fileURL.path.UTF8String, _playerEventListener.get());
}

-(void)renderFrameForTime:(const CVTimeStamp*)timestamp
{
    _openglRenderer->render_frame(timestamp->hostTime);
}
@end
