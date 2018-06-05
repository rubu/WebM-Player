#pragma once

#include "Player.h"
#include "AbstractView.h"
#include "WebMPlayerOpenGLProgramVariables.h"
#include "YUVFrame.h"
#include "OpenGL/OpenGLContext.h"
#include "OpenGL/OpenGLBuffer.h"

#include <list>
class OpenGLRenderer
{
public:
    OpenGLRenderer(IOpenGLContext& context, Player& player, const char* fragment_shader_source, const char* vertex_shader_source, size_t frame_queue_size = 50);
    
    void initialize();
    bool on_video_frame_size_changed(unsigned int width, unsigned int height);
    bool on_render_aread_size_changed(unsigned int width, unsigned int height);
    bool on_i420_video_frame_decoded(unsigned char* yuv_planes[3], size_t strides[3], uint64_t pts /* nanoseconds */);
    void render_frame(uint64_t host_time);
    void reset();
    void set_timescale(unsigned int timescale_numerator, unsigned int timescale_denominator);

private:
    IOpenGLContext& context_;
    Player& player_;
    const GLuint program_;
    OpenGLBuffer vertex_buffer_;
    OpenGLBuffer index_buffer_;
    OpenGLVertexInfo vertexes_[4];
    WebMPlayerOpenGLProgramVariables variables_;
    const size_t frame_queue_size_;
    std::recursive_mutex mutex_;
    bool first_frame_;
    uint64_t first_frame_host_time_;
    uint64_t first_frame_pts_;
    unsigned int timescale_;
    std::list<std::pair<uint64_t, YUVFrame>> frames_;
    std::list<std::pair<uint64_t, YUVFrame>>::iterator free_frame_iterator_;
};
