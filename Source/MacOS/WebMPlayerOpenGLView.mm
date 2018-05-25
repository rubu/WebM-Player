#import "WebMPlayerOpenGLView.h"
#include "../WebMPlayerOpenGLProgramVariables.h"
#include "../Player.h"
#include "../YUVFrame.h"

#import <OpenGL/gl3.h>
#include <mach/mach_time.h>

#include <list>

#define kFailedToLoadShader @"failed to load OpenGL shader"

void attribute_load_error_handler(const char* error, void* user_data)
{
    
}

static uint64_t mach_absolute_time_to_nanoseconds(uint64_t absolute_time = mach_absolute_time())
{
    static mach_timebase_info_data_t timebase_info {0, 0};
    if (timebase_info.denom == 0)
    {
        mach_timebase_info(&timebase_info);
    }
    return absolute_time * timebase_info.numer / timebase_info.denom;
}

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
    PlayerEventListener(WebMPlayerOpenGLView* view, size_t frame_queue_size = 50) : view_(view),
        frame_queue_size_(frame_queue_size),
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
        NSOpenGLContext* openGLContext = view_.openGLContext;
        [openGLContext makeCurrentContext];
        glViewport(0, 0, width, height);
        [NSOpenGLContext clearCurrentContext];
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        frames_.clear();
        for (size_t frame_index = 0; frame_index < frame_queue_size_; ++frame_index)
        {
            frames_.emplace_back(0, YUVFrame(height, width, width / 2, width / 2));
        }
        free_frame_iterator_ = frames_.begin();
    }
    
    bool on_i420_video_frame_decoded(unsigned char* yuv_planes[3], uint64_t pts /* nanoseconds */) override
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        if (first_frame_)
        {
            first_frame_host_time_ = mach_absolute_time_to_nanoseconds();
            first_frame_pts_ = pts;
            first_frame_ = false;
        }
        if (free_frame_iterator_ != frames_.end())
        {
            free_frame_iterator_->first = pts - first_frame_pts_ + first_frame_host_time_;
            free_frame_iterator_->second.load_planes(yuv_planes);
            ++free_frame_iterator_;
        }
        return free_frame_iterator_ != frames_.end();
    }
    
    void on_exception(const std::exception& exception) override
    {
        
    }
    
private:
    WebMPlayerOpenGLView* const view_;
    const size_t frame_queue_size_;
    std::recursive_mutex mutex_;
    bool first_frame_;
    uint64_t first_frame_host_time_;
    uint64_t first_frame_pts_;
    std::list<std::pair<uint64_t, YUVFrame>> frames_;
    std::list<std::pair<uint64_t, YUVFrame>>::iterator free_frame_iterator_;
};

@implementation WebMPlayerOpenGLView
{
    WebMPlayerOpenGLProgramVariables _openGLProgramVariables;
    CVDisplayLinkRef _displayLink;
    std::unique_ptr<PlayerEventListener> _playerEventListener;
    std::unique_ptr<Player> _player;
}

- (void)awakeFromNib
{
    [super awakeFromNib];
    NSOpenGLContext* openGLContext = [self openGLContext];
    [openGLContext makeCurrentContext];
    [self loadShaders];
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

- (GLuint)compileShaderOfType:(GLenum)type file:(NSString *)file
{
    const GLchar* source = (GLchar *)[[NSString stringWithContentsOfFile:file encoding:NSASCIIStringEncoding error:nil] cStringUsingEncoding:NSASCIIStringEncoding];
    if (source == nil)
    {
        [NSException raise:kFailedToLoadShader format:@"failed to read shader file %@", file];
    }
    GLuint shader = glCreateShader(type);
    GLenum error;
    if (shader == 0)
    {
        error = glGetError();
        [NSException raise:kFailedToLoadShader format:@"failed to create a shader of type %d", type];
    }
    glShaderSource(shader, 1, &source, NULL);
    error = glGetError();
    glCompileShader(shader);
    error = glGetError();
#if defined(DEBUG)
    GLint logLength = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
    error = glGetError();
    if (logLength > 0)
    {
        GLchar *log = reinterpret_cast<GLchar*>(malloc((size_t)logLength));
        glGetShaderInfoLog(shader, logLength, &logLength, log);
        error = glGetError();
        NSLog(@"shader compilation failed with error:\n%s", log);
        free(log);
    }
#endif
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    error = glGetError();
    if (status != GL_TRUE)
    {
        glDeleteShader(shader);
        error = glGetError();
        [NSException raise:kFailedToLoadShader format:@"shader compilation failed for file %@", file];
    }
    return shader;
}

-(void)loadShaders
{
    NSString* i420FragmentShaderPath = [[NSBundle mainBundle] pathForResource:@"I420toRGBA" ofType:@"frag"];
    NSString* defaultVertexShaderPath = [[NSBundle mainBundle] pathForResource:@"default" ofType:@"vert"];
    GLuint fragmentShader = [self compileShaderOfType:GL_FRAGMENT_SHADER file:i420FragmentShaderPath];
    GLuint vertexShader = [self compileShaderOfType:GL_VERTEX_SHADER file:defaultVertexShaderPath];
    if (0 != vertexShader && 0 != fragmentShader)
    {
        GLuint shaderProgram = glCreateProgram();
        GLenum error = glGetError();
        glAttachShader(shaderProgram, vertexShader  );
        error = glGetError();
        glAttachShader(shaderProgram, fragmentShader);
        error = glGetError();
        [self linkProgram:shaderProgram];
        if (_openGLProgramVariables.initialize(shaderProgram, attribute_load_error_handler, (__bridge void*)self) == false)
        {
            [NSException raise:kFailedToLoadShader format:@"could not bind shader program parameters"];
        }
        glDeleteShader(vertexShader  );
        error = glGetError();
        glDeleteShader(fragmentShader);
        error = glGetError();
    }
    else
    {
        [NSException raise:kFailedToLoadShader format:@"shader compilation failed"];
    }
}

- (void)linkProgram:(GLuint)program
{
    glLinkProgram(program);
    GLenum error = glGetError();
#if defined(DEBUG)
    GLint logLength;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
    error = glGetError();
    if (logLength > 0)
    {
        GLchar *log = reinterpret_cast<GLchar*>(malloc((size_t)logLength));
        glGetProgramInfoLog(program, logLength, &logLength, log);
        error = glGetError();
        NSLog(@"shader program linking failed with error:\n%s", log);
        free(log);
    }
#endif
    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    error = glGetError();
    if (0 == status)
    {
        [NSException raise:kFailedToLoadShader format:@"failed to link shader program"];
    }
}
@end
