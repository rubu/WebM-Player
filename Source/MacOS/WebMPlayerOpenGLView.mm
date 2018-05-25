#import "WebMPlayerOpenGLView.h"
#include "../Player.h"

#import <OpenGL/gl.h>

CVReturn DisplayLinkOutputCallback(CVDisplayLinkRef displayLink,
                                        const CVTimeStamp *inNow,
                                        const CVTimeStamp *inOutputTime,
                                        CVOptionFlags flagsIn,
                                        CVOptionFlags *flagsOut,
                                        void *displayLinkContext)
{
    return kCVReturnSuccess;
}

class PlayerEventListener : public Player::IEventListener
{
public:
    PlayerEventListener(WebMPlayerOpenGLView* view) : view_(view)
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
    
    void on_video_frame_decoded(unsigned char* (&yuv_planes)[3]) override
    {
    }
    
    void on_exception(const std::exception& exception) override
    {
        
    }
    
private:
    WebMPlayerOpenGLView* view_;
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
    _playerEventListener = std::make_unique<PlayerEventListener>(self);
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
    _player = std::make_unique<Player>(fileURL.path.UTF8String, _playerEventListener.get());
    _player->start();
}

@end
