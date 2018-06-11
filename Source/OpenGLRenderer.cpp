#include "OpenGLRenderer.h"
#include "OpenGL/OpenGLShader.h"
#include "OpenGL/OpenGLContextLock.h"

#define DEBUG_PTS

void attribute_load_error_handler(const char* error, void* user_data)
{
    fprintf(stderr, "%s", error);
}

static const GLushort indexes_[6] =
{
    0, 1, 2,
    0, 3, 2,
};

static void create_orthographic_matrix(float matrix[16], float left, float right, float bottom, float top, float nearZ, float farZ)
{
    float ral = right + left;
    float rsl = right - left;
    float tab = top + bottom;
    float tsb = top - bottom;
    float fan = farZ + nearZ;
    float fsn = farZ - nearZ;
    memset(matrix, 0, 16 * sizeof(float));
    matrix[0] = 2.0f / rsl;
    matrix[5] = 2.0f / tsb;
    matrix[10] = -2.0f / fsn;
    matrix[12] = -ral / rsl;
    matrix[13] = -tab / tsb;
    matrix[14] = -fan / fsn;
    matrix[15] = 1.0f;
}

static GLuint create_program(const char* fragment_shader_source, const char* vertex_shader_source)
{
    OpenGLShader fragment_shader(compile_shader(GL_FRAGMENT_SHADER, fragment_shader_source)), vertex_shader(compile_shader(GL_VERTEX_SHADER, vertex_shader_source));
    GLuint program = glCreateProgram();
    CHECK_OPENGL("glCreateProgram() failed");
    glAttachShader(program, fragment_shader);
    CHECK_OPENGL("glAttachShader(%u, %u) failed", program, static_cast<unsigned int>(fragment_shader));
    glAttachShader(program, vertex_shader);
    CHECK_OPENGL("glAttachShader(%u, %u) failed", program, static_cast<unsigned int>(vertex_shader));
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


OpenGLRenderer::OpenGLRenderer(IOpenGLContext& context, Player& player, const char* fragment_shader_source, const char* vertex_shader_source, size_t frame_queue_size) : context_(context),
    player_(player),
    program_(create_program(fragment_shader_source, vertex_shader_source)),
    vertexes_{},
    frame_queue_size_(frame_queue_size),
    first_frame_(true),
    first_frame_host_time_(0),
    first_frame_pts_(0),
    timescale_(0),
    free_frame_iterator_(frames_.end())
{
    if (variables_.initialize(program_, attribute_load_error_handler, this) == false)
    {
        throw std::runtime_error("failed to initialize shader program variables");
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_);
    CHECK_OPENGL("glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, %u) failed", static_cast<unsigned int>(index_buffer_));
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexes_), indexes_, GL_STATIC_DRAW);
    CHECK_OPENGL("glBufferData() failed");
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    CHECK_OPENGL("glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0) failed");
    OpenGLContextLock lock(context_);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
}

bool OpenGLRenderer::on_render_aread_size_changed(unsigned int width, unsigned int height)
{
    return true;
}

bool OpenGLRenderer::on_video_frame_size_changed(unsigned int width, unsigned int height)
{
    try
    {
        {
            OpenGLContextLock lock(context_);
            variables_.y_texture_.value().initialize(width, height);
            variables_.u_texture_.value().initialize(width / 2, height / 2);
            variables_.v_texture_.value().initialize(width / 2, height / 2);
            variables_.chroma_div_h_.value() = variables_.chroma_div_w_.value() = 1.0f;
            glViewport(0, 0, width, height);

            create_orthographic_matrix(variables_.projection_matrix_.value(), width / -2.0f, width / 2.0f, height / -2.0f, height / 2.0f, 1.0f, -1.0f);
            
            GLfloat (&model_view_matrix)[16] = variables_.model_view_matrix_.value();
            memset(model_view_matrix, 0, 16 * sizeof(float));
            model_view_matrix[0] = model_view_matrix[5] = model_view_matrix[10] = model_view_matrix[15] = 1.0f;
            

            vertexes_[0].x = vertexes_[1].x = - (float)width/ 2.0f;
            vertexes_[2].x = vertexes_[3].x =   (float)width / 2.0f;
            vertexes_[1].y = vertexes_[2].y = - (float)width / 2.0f;
            vertexes_[0].y = vertexes_[3].y =   (float)width / 2.0f;
            
            vertexes_[0].s0 = 0.0f;
            vertexes_[0].t0 = 0.0f;
            vertexes_[1].s0 = 0.0f;
            vertexes_[1].t0 = 1.0f;
            vertexes_[2].s0 = 1.0f;
            vertexes_[2].t0 = 1.0f;
            vertexes_[3].s0 = 1.0f;
            vertexes_[3].t0 = 0.0f;

            glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
            CHECK_OPENGL("glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, %u) failed", static_cast<unsigned int>(vertex_buffer_));
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertexes_), vertexes_, GL_STATIC_DRAW);
            CHECK_OPENGL("glBufferData() failed");
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            CHECK_OPENGL("glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0) failed");
        }
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        frames_.clear();
        for (size_t frame_index = 0; frame_index < frame_queue_size_; ++frame_index)
        {
            frames_.emplace_back(0, YUVFrame(height, width, width / 2, width / 2));
        }
        free_frame_iterator_ = frames_.begin();
    }
    catch (const ExceptionBase& exception)
    {
        fprintf(stderr, "%s(%d): caught exception \n\t%s", __FILE__, __LINE__, exception.error_with_location());
        return false;
    }
    catch (const std::exception& exception)
    {
        fprintf(stderr, "%s(%d): caught exception \n\t%s", __FILE__, __LINE__, exception.what());
        return false;
    }
    return true;
}

bool OpenGLRenderer::on_i420_video_frame_decoded(unsigned char* yuv_planes[3], size_t strides[3], uint64_t pts)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (first_frame_)
    {
        first_frame_host_time_ = get_host_time();
        first_frame_pts_ = pts;
        first_frame_ = false;
#if defined(DEBUG_PTS)
        printf("%s(%d): first frame host time - %llu\n" ,__FILE__, __LINE__, first_frame_host_time_);
#endif
    }
    if (free_frame_iterator_ != frames_.end())
    {
        free_frame_iterator_->first = (pts - first_frame_pts_) * timescale_ + first_frame_host_time_;
#if defined(DEBUG_PTS)
        printf("%s(%d): host time for frame pts %llu is  %llu\n" ,__FILE__, __LINE__, pts, free_frame_iterator_->first);
#endif
        free_frame_iterator_->second.load_planes(yuv_planes, strides);
        ++free_frame_iterator_;
    }
    return free_frame_iterator_ != frames_.end();
}

void OpenGLRenderer::reset()
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    first_frame_ = true;
    first_frame_host_time_ = 0;
    first_frame_pts_ = 0;
    free_frame_iterator_ = frames_.end();
}

void OpenGLRenderer::set_timescale(unsigned int timescale_numerator, unsigned int timescale_denominator)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (host_timescale % timescale_denominator == 0)
    {
        timescale_ = timescale_numerator * (host_timescale / timescale_denominator);
    }
    else
    {
        timescale_ = host_timescale * (timescale_numerator * 1.0f / timescale_denominator);
    }
}

void OpenGLRenderer::render_frame(uint64_t host_time)
{
    YUVFrame frame;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        auto& first_frame_entry = frames_.front();
        if (first_frame_entry.first != 0 && first_frame_entry.first < host_time)
        {
#if defined(DEBUG_PTS)
            printf("%s(%d): rendering frame with pts %llu at host time %llu\n" ,__FILE__, __LINE__, first_frame_entry.first, host_time);
#endif
            frame = std::move(first_frame_entry.second);
            frames_.pop_front();
        }
        else
        {
            return;
        }
    }
    {
        OpenGLContextLock opengl_context_lock(context_);
        std::swap(frame, current_frame_);
        render_frame(current_frame_);
    }
    if (frame.is_empty())
    {
        return;
    }
    bool resume_player = false;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        if (free_frame_iterator_ == frames_.end())
        {
            resume_player = true;
        }
        frames_.emplace_back(0, std::move(frame));
        if (resume_player)
        {
            --free_frame_iterator_;
        }
    }
    if (resume_player)
    {
        player_.resume();
    }
}

void OpenGLRenderer::render_current_frame()
{
    OpenGLContextLock opengl_context_lock(context_);
    render_frame(current_frame_);
}

void OpenGLRenderer::render_frame(YUVFrame& frame)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (frame.is_empty())
        return;
    try
    {
        variables_.y_texture_.value().load(frame.y_plane());
        variables_.u_texture_.value().load(frame.u_plane());
        variables_.v_texture_.value().load(frame.v_plane());
        glUseProgram(program_);
        CHECK_OPENGL_DEBUG("glUseProgram(%d) failed", program_);
        glUniformMatrix4fv(variables_.projection_matrix_.location(), 1, GL_FALSE, variables_.projection_matrix_.value());
        CHECK_OPENGL_DEBUG("glUniformMatrix4fv(%d, ...) failed", variables_.projection_matrix_.location());
        glUniformMatrix4fv(variables_.model_view_matrix_.location(), 1, GL_FALSE, variables_.model_view_matrix_.value());
        CHECK_OPENGL_DEBUG("glUniformMatrix4fv(%d, ...) failed", variables_.model_view_matrix_.location());
        glActiveTexture(GL_TEXTURE0);
        CHECK_OPENGL_DEBUG("glActiveTexture() failed");
        glBindTexture(GL_TEXTURE_2D, variables_.y_texture_.value().id());
        CHECK_OPENGL_DEBUG("glBindTexture() failed");
        glUniform1i(variables_.y_texture_.location(), 0);
        CHECK_OPENGL_DEBUG("glUniform1i(%d, 0) failed", variables_.y_texture_.location());
        glActiveTexture(GL_TEXTURE0 + 1);
        CHECK_OPENGL_DEBUG("glActiveTexture() failed");
        glBindTexture(GL_TEXTURE_2D, variables_.u_texture_.value().id());
        CHECK_OPENGL_DEBUG("glBindTexture() failed");
        glUniform1i(variables_.u_texture_.location(), 1);
        CHECK_OPENGL_DEBUG("glUniform1i(%d, 1) failed", variables_.u_texture_.location());
        glActiveTexture(GL_TEXTURE0 + 2);
        CHECK_OPENGL_DEBUG("glActiveTexture() failed");
        glBindTexture(GL_TEXTURE_2D, variables_.v_texture_.value().id());
        CHECK_OPENGL_DEBUG("glBindTexture() failed");
        glUniform1i(variables_.v_texture_.location(), 2);
        CHECK_OPENGL_DEBUG("glUniform1i(%d, 2) failed", variables_.v_texture_.location());
        glUniform1f(variables_.chroma_div_w_.location(), variables_.chroma_div_w_.value());
        CHECK_OPENGL_DEBUG("glUniform1f(%d, %u) failed", variables_.chroma_div_w_.location(), variables_.chroma_div_w_.value());
        glUniform1f(variables_.chroma_div_h_.location(), variables_.chroma_div_h_.value());
        CHECK_OPENGL_DEBUG("glUniform1f(%d, %u) failed", variables_.chroma_div_h_.location(), variables_.chroma_div_h_.value());
        glClear(GL_COLOR_BUFFER_BIT);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
        CHECK_OPENGL_DEBUG("glBindBuffer(GL_ARRAY_BUFFER, %u) failed", static_cast<unsigned int>(vertex_buffer_));
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_);
        CHECK_OPENGL_DEBUG("glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, %u) failed", static_cast<unsigned int>(index_buffer_));
        glVertexAttribPointer(variables_.position_location_, 3, GL_FLOAT, GL_FALSE, sizeof(OpenGLVertexInfo), 0);
        CHECK_OPENGL_DEBUG("glVertexAttribPointer() failed");
        glEnableVertexAttribArray(variables_.position_location_);
        CHECK_OPENGL_DEBUG("glEnableVertexAttribArray() failed");
        glVertexAttribPointer(variables_.texture_coordinates_location_, 2, GL_FLOAT, GL_FALSE, sizeof(OpenGLVertexInfo), reinterpret_cast<const GLvoid*>(12));
        CHECK_OPENGL_DEBUG("glVertexAttribPointer() failed");
        glEnableVertexAttribArray(variables_.texture_coordinates_location_);
        CHECK_OPENGL_DEBUG("glEnableVertexAttribArray() failed");
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
        CHECK_OPENGL_DEBUG("glDrawElements() failed");
        CHECK_OPENGL("failed to render frame");
    }
    catch (const ExceptionBase& exception)
    {
        fprintf(stderr, "%s(%d): caught exception \n\t%s", __FILE__, __LINE__, exception.error_with_location());
    }
    catch (const std::exception& exception)
    {
        fprintf(stderr, "%s(%d): caught exception \n\t%s", __FILE__, __LINE__, exception.what());
    }
}
