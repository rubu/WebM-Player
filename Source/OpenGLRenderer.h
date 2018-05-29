#pragma once

#include "Player.h"
#include "AbstractView.h"
#include "WebMPlayerOpenGLProgramVariables.h"
#include "YUVFrame.h"
#include "OpenGL/OpenGLContext.h"
#include "OpenGL/OpenGLBuffer.h"

#include <list>
class OpenGLRenderer : public Player::IEventListener
{
public:
    OpenGLRenderer(IAbstractView& view, IOpenGLContext& context, const char* fragment_shader_source, const char* vertex_shader_source, size_t frame_queue_size = 50);
    
    void initialize();
    void render_frame(uint64_t host_time);
    
    // Player::IEventListener;
    void on_video_frame_size_changed(unsigned int width, unsigned int height) override;
    bool on_i420_video_frame_decoded(unsigned char* yuv_planes[3], uint64_t pts /* nanoseconds */) override;
    void on_exception(const std::exception& exception) override;

private:
    IAbstractView& view_;
    IOpenGLContext& context_;
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
    std::list<std::pair<uint64_t, YUVFrame>> frames_;
    std::list<std::pair<uint64_t, YUVFrame>>::iterator free_frame_iterator_;
};
