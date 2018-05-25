#import "WebMPlayerOpenGLView.h"
#include "../Player.h"

#import <OpenGL/gl.h>
#include <mach/mach_time.h>

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

class PlayerEventListener : public Player::IEventListener
{
public:
    PlayerEventListener(WebMPlayerOpenGLView* view) : view_(view),
        first_frame_(true),
        first_frame_host_time_(0),
        first_frame_pts_(0)
    {
    }

    void on_video_frame_size_changed(unsigned int width, unsigned int height) override
    {
        dispatch_async(dispatch_get_main_queue(), ^
        {
            NSRect windowFrame = view_.window.frame;
            windowFrame.size.width = width;
            windowFrame.size.height = height;
            [view_.window setFrame:windowFrame display:YES];
        });
    }
    
    void on_i420_video_frame_decoded(unsigned char* (&yuv_planes)[3], unsigned int pts) override
    {
        if (first_frame_)
        {
            first_frame_host_time_ = mach_absolute_time();
            first_frame_pts_ = pts;
            first_frame_ = false;
        }
    }
    
    void on_exception(const std::exception& exception) override
    {
        
    }
    
private:
    WebMPlayerOpenGLView* view_;
    bool first_frame_;
    uint64_t first_frame_host_time_;
    unsigned int first_frame_pts_;
};

@implementation WebMPlayerOpenGLView
{
    CVDisplayLinkRef _displayLink;
    std::unique_ptr<PlayerEventListener> _playerEventListener;
    std::unique_ptr<Player> _player;
}

- (void)awakeFromNib
{
    [super awakeFromNib];
    CVReturn result = CVDisplayLinkCreateWithCGDisplay(CGMainDisplayID(), &_displayLink);
    if(result == kCVReturnSuccess)
    {
        result = CVDisplayLinkSetOutputCallback(_displayLink, DisplayLinkOutputCallback, (__bridge void*)self);
        if (result == kCVReturnSuccess)
        {
            CVDisplayLinkStart(_displayLink);
        }
    }
    glClearColor(0, 0, 0, 1);
}

-(void) dealloc
{
    CVDisplayLinkStop(_displayLink);
}

-(void) drawRect:(NSRect)dirtyRect
{
    glClear(GL_COLOR_BUFFER_BIT);
    glFlush();
}

-(void) playFile:(NSURL*)fileURL
{
    _playerEventListener = std::make_unique<PlayerEventListener>(self);
    _player = std::make_unique<Player>(fileURL.path.UTF8String, _playerEventListener.get());
    _player->start();
}

-(void)renderFrameForTime:(const CVTimeStamp*)timestamp
{
    
}

@end
