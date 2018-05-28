#include "OpenGLRenderer.h"
#include "OpenGL/OpenGLShader.h"
#include "OpenGL/OpenGLContextLock.h"

void attribute_load_error_handler(const char* error, void* user_data)
{
}

static GLuint CreateProgram(const char* fragment_shader_source, const char* vertex_shader_source)
{
    OpenGLShader fragment_shader(CompileShader(GL_FRAGMENT_SHADER, fragment_shader_source)), vertex_shader(CompileShader(GL_VERTEX_SHADER, vertex_shader_source));
    GLuint program = glCreateProgram();
    CHECK_OPENGL_CALL("glCreateProgram() failed");
    glAttachShader(program, fragment_shader);
    CHECK_OPENGL_CALL("glAttachShader(%u, %u) failed", program, fragment_shader);
    glAttachShader(program, vertex_shader);
    CHECK_OPENGL_CALL("glAttachShader(%u, %u) failed", program, vertex_shader);
    glLinkProgram(program);
#if defined(DEBUG)
    GLint program_linking_log_length = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &program_linking_log_length);
    if (program_linking_log_length > 0)
    {
        std::unique_ptr<char[]> log(new char[program_linking_log_length]);
        glGetProgramInfoLog(program, program_linking_log_length, &program_linking_log_length, log.get());
        printf("shader program linking compilation log:\n%s", log.get());
    }
#endif
    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (0 == status)
    {
        throw std::runtime_error("shader program linking failed");
    }
    return program;
}


OpenGLRenderer::OpenGLRenderer(IAbstractView& view, IOpenGLContext& context, const char* fragment_shader_source, const char* vertex_shader_source, size_t frame_queue_size) : view_(view),
    context_(context),
    program_(CreateProgram(fragment_shader_source, vertex_shader_source)),
    frame_queue_size_(frame_queue_size),
    first_frame_(true),
    first_frame_host_time_(0),
    first_frame_pts_(0)
{
    if (variables_.initialize(program_, attribute_load_error_handler, this) == false)
    {
        throw std::runtime_error("failed to initialize shader program variables");
    }
    glClearColor(0, 0, 0, 1);
}

void OpenGLRenderer::on_video_frame_size_changed(unsigned int width, unsigned int height)
{
    view_.resize(width, height);
    {
        OpenGLContextLock lock(context_);
        glViewport(0, 0, width, height);
        variables_.y_texture_.value().initialize(width, height);
        variables_.u_texture_.value().initialize(width / 2, height);
        variables_.v_texture_.value().initialize(width / 2, height);
    }
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    frames_.clear();
    for (size_t frame_index = 0; frame_index < frame_queue_size_; ++frame_index)
    {
        frames_.emplace_back(0, YUVFrame(height, width, width / 2, width / 2));
    }
    free_frame_iterator_ = frames_.begin();
}

bool OpenGLRenderer::on_i420_video_frame_decoded(unsigned char* yuv_planes[3], uint64_t pts /* nanoseconds */)
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

void OpenGLRenderer::on_exception(const std::exception& exception)
{
}

void OpenGLRenderer::render_frame(uint64_t host_time)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
}
